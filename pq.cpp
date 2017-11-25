#include "scheduler.hpp"

/* Global Variables */
char       *PROGRAM_NAME  = NULL;
int         NCPUS         = 1;
int         MICROSECONDS  = 1000;
bool        SERVER        = false;
Scheduler   scheduler;

std::string POLICY        = "fifo";
std::string PATH          = "/tmp/pq.socket";
std::string COMMAND       = "";

/* Functions */
void usage(const char *program_name, int status) {
    fprintf(stderr, "Usage: %s [OPTIONS] \n", program_name);
    fprintf(stderr, "\nGeneral Options:\n");
    fprintf(stderr, "\t-h                 Print this help message\n");
    fprintf(stderr, "\t-f PATH            Path to IPC channel\n");
    fprintf(stderr, "\nClient Options:\n");
    fprintf(stderr, "\tadd COMMAND        Add COMMAND to queue\n");
    fprintf(stderr, "\tstatus             Query status of queue\n");
    fprintf(stderr, "\trunning            Query running jobs\n");
    fprintf(stderr, "\twaiting            Query waiting jobs\n");
    fprintf(stderr, "\tflush              Remove all jobs from queue\n");
    fprintf(stderr, "\nServer Options:\n");
    fprintf(stderr, "\t-n NCPUS           Number of CPUS\n");
    fprintf(stderr, "\t-p POLICY          Scheduling policy (fifo, rdrn, mlfq)\n");
    fprintf(stderr, "\t-t MICROSECONDS    Time between scheduling\n");
    exit(status);
}

void sig_handler(int signum) {
    scheduler.RunningPop();
}

void sig_handler_stop(int signum) {
}

void sig_handler_cont(int signum) {
}

int main(int argc, char *argv[]) {
    PROGRAM_NAME = argv[0];

    int argind = 1;

    if (argc == 1) {
        usage(PROGRAM_NAME, 1); // No command line arguments given
    }

    // Parse flags.
    while (argind < argc && strlen(argv[argind]) > 1) {
        std::string arg = argv[argind++];

        // Parse general and server flags.
        
        if (arg.compare("-h") == 0){
            usage(PROGRAM_NAME, 0);
        }
        else if (arg.compare("-n") == 0) {
            NCPUS = atoi(argv[argind]);
            SERVER = true;
        }
        else if (arg.compare("-f") == 0) {
            PATH = argv[argind];
        }
        else if (arg.compare("-p") == 0) {
            POLICY = argv[argind];
            SERVER = true;
        }
        else if (arg.compare("-t") == 0) {
            MICROSECONDS = atoi(argv[argind]);
            SERVER = true;
        }
        else if (arg.compare("add") == 0) {
            COMMAND = "ADD ";

            // TODO: Figure out a better way to capture entire command.
            for (int i = argind; i < argc; i++) {
                COMMAND += argv[i];
                COMMAND += " ";
            }
            COMMAND += "\n";
            break;
        }
        else if (arg.compare("status") == 0) {
            COMMAND = "STATUS\n";
            break;
        }
        else if (arg.compare("running") == 0) {
            COMMAND = "RUNNING\n";
            break;
        }
        else if (arg.compare("waiting") == 0) {
            COMMAND = "WAITING\n";
            break;
        }
        else if (arg.compare("flush") == 0) {
            COMMAND = "FLUSH\n";
            break;
        }
        else {
            usage(PROGRAM_NAME, 1);
        }

        argind++;
    }

    // Initialize scheduler member variables
    scheduler.SetMicroseconds(MICROSECONDS);
    scheduler.SetNCPUS(NCPUS);
    scheduler.SetSocketpath(PATH);
    scheduler.SetPolicy(POLICY);

    // Arm signal handler
    signal(SIGCHLD, sig_handler);
    signal(SIGSTOP, sig_handler_stop);
    signal(SIGCONT, sig_handler_cont);

    // block sigstp in sigchld
    struct sigaction sa;
    sa.sa_handler = &sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror(0);
        exit(1);
    }

    /* Decide whether to do server or client */
    if (SERVER) {
        log("INFO  Starting Process Queue Server...");
        if (POLICY.compare("fifo") == 0) {
            log("INFO  Running FIFO...");
            RunServer(scheduler, FIFO, MICROSECONDS);
        }
        else if (POLICY.compare("rdrn") == 0) {
            log("INFO  Running RDRN...");
            RunServer(scheduler, RDRN, MICROSECONDS);
        }
        else if (POLICY.compare("mlfq") == 0) {
            log("INFO  Running MLFQ...");
            RunServer(scheduler, MLFQ, MICROSECONDS);
        }
        else {
            log("ERROR  No policy specified");
        }
    }
    else {
        RunClient(COMMAND, PATH);
    }

    return EXIT_SUCCESS;
}


