#include "scheduler.hpp"

Scheduler::Scheduler() { }

Scheduler::~Scheduler() { }

void Scheduler::SetSocketpath(std::string socket_path) {
    this->socket_path = socket_path;
}

void Scheduler::SetCommand(std::string command) {
    this->command = command;
}
std::string Scheduler::GetSocketpath() {
    return socket_path;
}

void Scheduler::SetPolicy(std::string policy) {
    this->policy = policy;
}

void Scheduler::SetSubsequent(std::string subsequent) {
    this->subsequent = subsequent;
}

void Scheduler::SetMicroseconds(int microseconds) {
    this->microseconds = microseconds;
}

void Scheduler::SetNCPUS(int cpus) {
    this->cpus = cpus;
}

void Scheduler::RunningPop() {
    Process p = running.back();

    if (p.pid != 0) {           // Avoid incorrect process pops

        p.finished = time(NULL);

        double pturnaround = p.finished - p.arrival;
        double presponse = p.start - p.arrival;
        turnaround += pturnaround;
        response += presponse;
        totalprocesses++;       // Increment total processes - used to calculate turnaround/response

        log("Reaped process: %d %s Turnaround = %.3f, Response = %.3f", p.pid, p.subsequent, pturnaround, presponse);

        running.pop_back();     // Pop off running queue

        if (policy.compare("rdrn") == 0) {
            // Move a process from waiting to running if policy is rdrn
            p = waiting.back();
            waiting.pop_back();
            running.push_back(p);
        }
    }
}

char *Scheduler::stateString (char *a) {
    return (strcmp(a, "R") == 0) ? (char*)"Running" : (char*)"Sleeping";
}

void Scheduler::UpdateProcess(Process &process, int pid) {
    char file_name[BUFSIZ];
    char file_uptime[BUFSIZ];   //  Uptime to calc cpu_usage
    char buffer[BUFSIZ];
    FILE *f;
    FILE *uf;                   // open /proc/uptime for file uptime
    int n = 0;                  // used to switch through /proc/[PID]/stat
    const char *delim = " ";
    char *token;
    time_t stime;
    time_t uptime;

    time_t cutime;
    time_t cstime;
    time_t starttime;

    process.pid = pid;

    // Get uptime
    sprintf(file_uptime, "/proc/uptime");
    uf = fopen(file_uptime, "r");
    fgets(buffer, BUFSIZ, uf);
    token = strtok(buffer, delim);
    token = strtok(buffer, delim);
    uptime = atoi(token);
    fclose(uf);

    sprintf(file_name, "/proc/%d/stat", process.pid);

    f = fopen(file_name, "r");
    if (f == NULL) { perror("fopen failed"); _exit(EXIT_FAILURE); }

    // Tokenizing /proc/[PID]/stat.
    while (fgets(buffer, BUFSIZ, f)) {
        token = strtok(buffer, delim);
        while (token != NULL) {  //  52 tokens in /proc/[PID]/stat
            switch (n) {
                case  2:  //  state
                    process.state = stateString(token); // Returns either running or sleeping.
                    break;

                case 13:  //  utime
                    process.user = atof(token);
                    break;

                case 14:  //  stime for usage
                    stime = atoi(token);
                    break;

                case 15: // cutime
                    cutime = atoi(token);
                    break;

                case 16: // cstime
                    cstime = atoi(token);
                    break;

                case 21: // starttime
                    starttime = atoi(token);
                    break;

                default:
                    break;
            }
            n++;
            token = strtok(NULL, delim);
        }
    }
    fclose(f);

    int totaltime = process.user + stime;
    int Hertz = sysconf(_SC_CLK_TCK);

    totaltime = totaltime + cutime + cstime;
    double seconds = uptime - ((starttime) / Hertz);
    process.usage = 100 * ((totaltime / Hertz) / seconds);

    process.started = true; // Switch started flag (used for RDRN + MLFQ)
}

/* Run the FIFO algorithm */
void Scheduler::ServerFIFO() {
    Process p;
    while ((!waiting.empty()) && (running.size() < (unsigned)cpus)){
        p = waiting.back();
        ProcessStart(p);
        running.push_back(p);               // Push onto Running queue
        waiting.pop_back();
    }
}

/* Run the RDRN algorithm */
void Scheduler::ServerRDRN() {
    // Move a process from running queue to waiting queue.
    if (!running.empty()) {
        Process p = running.back();
        running.pop_back();                 // Pop from deque
        ProcessPause(p);                    // Send signal
        waiting.push_front(p);              // Push onto waiting deque
    }

    // Move processes from waiting queue to running queue.
    while ((!waiting.empty()) && (running.size() < (unsigned)cpus)) {
        Process p = waiting.back();
        waiting.pop_back();
        if (p.pid == 0) {
            ProcessStart(p);
        } else {
            ProcessResume(p);
        }
        running.push_back(p);               // Push onto Running queue
    }
}

/* Run the MLFQ algorithm */
void Scheduler::ServerMLFQ(int &boostCount) {
    Process p;

    // Preempt running process.
    if (!running.empty()) {
        p = running.back();
        running.pop_back();
        if (checkExceedThreshold(p)) {
            p.priority++;
        }
        ProcessPause(p);
        mlfq_levels[p.priority].push_front(p);
    }

    // Apply priority boost
    boostCount++;                               // Increment for every timeslice
    if (boostCount == 20) {                     // Do a boost every 20 timeslices
        doBoost();
        boostCount = 0;
    }

    // Move waiting to top priority level -- immediately add new processes
    while (!waiting.empty()) {
        p = waiting.back();
        waiting.pop_back();
        mlfq_levels[0].push_front(p);
    }

    // Start or resume new process and move to running
    for ( auto l : mlfq_levels ) {
        while ((!l.empty()) and (running.size() < (unsigned)cpus)) {
            p = l.back();
            l.pop_back();                       // Errata -- pop_back doesn't pop off anything, so it fills up level 0
            if (p.pid == 0) {
                ProcessStart(p);
            } else {
                ProcessResume(p);
            }
            running.push_back(p);
        }
    }
}

void Scheduler::ProcessStart(Process &process) {
    int arrsize = 0;
    char *token;

    if (!process.started) {   // If Process has not started
        /* Fork and Execute */
        pid_t pid = fork();
        if (pid < 0) {  //  Error
            fprintf(stderr, "Unable to fork: %s\n", strerror(errno));
        }
        else if (pid == 0) { //  Child
            //get execvp command
            char * temp = strdup(process.subsequent);

            token = strtok(temp, " ");

            //get the size of the array
            while (token != NULL) {
                token = strtok(NULL, " ");
                arrsize++;
            }
            char *v[arrsize + 1];

            free(temp);

            token = strtok(process.subsequent, " ");
            int count = 0;
            while (token != NULL) {
                v[count] = token;
                token = strtok(NULL, " ");
                count++;
            }
            v[arrsize + 1] = NULL;

            int devNull = open("/dev/null", 2);
            dup2(devNull, STDOUT_FILENO);   // Replace STDOUT with devNULL
            
            if ((execvp(v[0], v)) < 0) {    // Execute process
                perror("Exec failed!");
            }
            close(devNull);
        }

        else { //  Parent
            UpdateProcess(process, pid);
            process.start = time(NULL);

            log("Started process %d: %s", pid, process.subsequent);

            int status;
            if (waitpid(pid, &status, WNOHANG) < 0) {
                perror("Wait Error");
            }
        }
    }
}

void Scheduler::ProcessResume(Process &p) {
    if ((kill(p.pid, SIGCONT)) < 0) {
        perror("Kill failed");
    }
}

void Scheduler::ProcessPause(Process &p) {
    // Send SIGSTOP
    if ((kill(p.pid, SIGSTOP)) < 0) {
        perror("Kill failed");
    }

}

// HandleAddRequest() - Adds process to waiting and reports to client the added process.
void Scheduler::HandleAddRequest(char *command, FILE *client_stream, char *subsequent) {
    int num = running.size() + waiting.size();
    time_t arrival = time (NULL);

    Process process;

    process.num = num;
    process.pid = 0;
    process.command = strdup(command);
    process.subsequent = strdup(subsequent);
    process.state = (char*)"Sleeping";
    process.user = 0;
    process.threshold = 0;
    process.usage = 0.0;
    process.arrival = arrival;
    process.start = 0;
    process.priority = 0;
    process.started = false;
    process.finished = 0;

    waiting.push_front(process);
    totalprocesses++;       // Increment total processes - used to calculate turnaround/response

    // Write to client the added command.
    char buffer[BUFSIZ];
    sprintf(buffer, "Added process %d: %s\n", num, subsequent);
    char *logging = buffer;
    fputs(buffer, client_stream);
    fflush(client_stream);

    // Log the command.
    logging[strlen(logging)-1] = 0;
    log("%s", logging);
}

// HandleStatusRequest() - Summary of sizes of runnning and waiting queues, average turnaround and response times, and the jobs in all queues.
void Scheduler::HandleStatusRequest(FILE *client_stream) { 
    // Write to client the status command.
    char buffer[BUFSIZ];

    double totalturnaround = turnaround/totalprocesses;
    double totalresponse = response/totalprocesses;

    sprintf(buffer, "Running =   %ld, Waiting =   %ld, Levels =    %d, Turnaround = %.3f, Response = %.3f \n\n", 
        running.size(), waiting.size(), levels, totalturnaround, totalresponse);

    strcat(buffer, "Running:\n");
    makeProcessString(running, buffer);         // Running

    if (policy.compare("mlfq") != 0) {          // FIFO and RDRN
        strcat(buffer, "Waiting:\n");
        makeProcessString(waiting, buffer);     // Waiting        
    } else {
        makeMLFQString(buffer);                 // MLFQ
    }

    fputs(buffer, client_stream);
    fflush(client_stream);
}

// HandleProcessRequest() - Lists all jobs in either Running or Waiting queue.
void Scheduler::HandleProcessRequest(FILE *client_stream, bool flag) {
    char buffer[BUFSIZ];
    if (flag)                                   // if true, print running
        makeProcessString(running, buffer);
    else
        makeProcessString(waiting, buffer);

    fputs(buffer, client_stream);
    fflush(client_stream);
}

void Scheduler::makeProcessString(std::deque<Process> &deq, char *buffer) {

    strcat(buffer, "  PID  COMMAND               STATE     USER  THRESHOLD  USAGE       ARRIVAL         START\n\n"); // Add header

    char catCString[BUFSIZ];
    for (auto p : deq) {

        if (p.pid != 0) {
            UpdateProcess(p, p.pid);                        // Update Process information of running before print
        }

        sprintf(catCString, "%5d %-22s %3s %6d %10d %6g %13lu %13lu\n",
                            p.pid, p.subsequent, p.state, p.user, p.threshold, p.usage, p.arrival, p.start); // Add process to cstring

        strcat(buffer, catCString);                         // Concatenate string
    }

    strcat(buffer, "\n");                                   // Add newline
}

void Scheduler::makeMLFQString(char *buffer) {
    char catCString[BUFSIZ];
    for ( int i = 0; i < int(mlfq_levels.size()); ++i ) {  // Loop through levels beginning at level 1
        sprintf(catCString, "\nLevel %d:\n", i); // Add level header
        strcat(catCString, "  PID  COMMAND               STATE     USER  THRESHOLD  USAGE       ARRIVAL         START\n\n"); // Add process header
        strcat(buffer, catCString);
        for (auto p : mlfq_levels[i]) {
            if (p.pid != 0) {
                UpdateProcess(p, p.pid);                    // Update Process information of running before print
            }

            sprintf(catCString, "%5d %-22s %3s %6d %10d %6g %13lu %13lu\n",
                        p.pid, p.subsequent, p.state, p.user, p.threshold, p.usage, p.arrival, p.start); // Add process to cstring

            strcat(buffer, catCString);                     // Concatenate string
        }
    }

    strcat(buffer, "\n");                                   // Add newline
}

// HandleFlushRequest() - Removes all jobs from all queues (terminates any that are active or suspended).
void Scheduler::HandleFlushRequest(FILE *client_stream) {
    // Print flush.
    char buffer[BUFSIZ];
    sprintf(buffer, "Flushed %d running and %d waiting processes\n", int(running.size()), int(waiting.size()));

    for (auto p : running) {
        ProcessPause(p);
    }

    // Clear deques.
    running.clear();
    waiting.clear();

    fputs(buffer, client_stream);
    fflush(client_stream);
}


bool Scheduler::checkExceedThreshold(Process &p) {
    // Update threshold based on level.
    int arbitraryMult = 10;
    int arbitraryInt = 5 * p.priority;
    p.threshold = p.priority * arbitraryMult + arbitraryInt;
    return p.user > p.threshold;
} 

void Scheduler::doBoost() {
    Process p;
    for ( int i = 1; i < int(mlfq_levels.size()); ++i ) {        // Loop through levels beginning at level 1
        while (!mlfq_levels[i].empty()) {
            p.priority = 0;
            mlfq_levels[0].push_back(p);
            mlfq_levels[i].pop_back();
        }
    }
}
