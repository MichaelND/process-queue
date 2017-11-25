CSE.30341.FA17: Project 02
==========================

This is the documentation for [Project 02] of [CSE.30341.FA17].

Members
-------

1. Anthony Luc (aluc@nd.edu)
2. Donald Luc (dluc@nd.edu)
3. Michael Wang (mwang6@nd.edu)

Design
------

> 1. You will need at least two types of structures:
>
>   - **Process**: This should keep track of each job along with any accounting
>     information such as **command** and **PID**.
>
>   - **Scheduler**: This should keep track of the jobs in running and waiting
>     queues along with other information such as **policy**, **number of
>     CPUs**, and **time slice duration**.
>
>   With these in mind, consider the following questions:
>
>   - What sort of **accounting** will you need to keep track of for each
>     **Process**?
>     Each process will have a PID, COMMAND, STATE, USER, THRESHOLD, USAGE, ARRIVAL, and START.
>
>   - How will you compute the turnaround and response times for each
>     **Process**?
>     Turnaround time needs the time that the process was scheduled as well as the time that the process was completed all over the total number of processes.
>
>   - What information do you need to store in the **Scheduler**?  How will it
>     maintain a running and waiting queue?
>     We will need to store two lists: one for running processes and one for waiting processes. Processes can be pushed onto the list when added or popped off of the list when completed.
>
>   - How will you compute the average turnaround and response times for the
>     whole process queue?
>     In a variable we can store the total turnaround/response times for the processes queue and divide by the total number of processes.

Response.

> 2. Debugging this project will be difficult without good logging.  Because
>    timing is an important component of scheduling, it will be useful to have
>    a consistent logging mechanism that includes timestamps.
>
>   - How will you go about logging information in your program?
>   We will create a macro that will help us log information and print out to the console/store into a log file.
>
>   - What sort of information will you log?
>   We will log PID, FILE, and LINE number.

Response.

> 3. Jobs in the process queue eventually become processes which need to be
>    created, preempted, resumed, and terminated.
>
>   - How will you create a process and execute its command?
>
>       Note: you will not want to use `/bin/sh -c command` in this assignment
>       since that creates two processes and you would only have direct control
>       of `/bin/sh` rather than `command`
>       We will fork() to create a new process and exec() to carryout the command.
>
>   - How will you implement preemption?  That is, how will you **stop** or
>     **pause** a running process?
>     Sending a SIGTSTP to the process will stop/pause the process if the process has signal handler.
>
>   - How will you **resume** a process that has been preempted?
>     Sending a SIGCONT will resume the process.
>
>   - How will you **terminate** an active process?
>     Sending a SIGTERM will terminate the process.
>
>   - How will you gather statistics or accounting information about each
>     process?  What will you store?
>     By searching for the process information in /proc/[PID]/ we can store information such as the time the CPU spent in user code (utime), the time the CPU spent in kernel mode (stime), and the start time (starttime).
>
>       [Hint](https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat)

Response.

> 4. The scheduler will need to activated under two conditions: when a process
>    dies and after some period of time (ie. time slice has expired).
>
>   - How will you trigger your scheduler when a process dies?  What must
>     happen when a process dies (consider both the **Scheduler** and the
>     **Process**)?
>     The parent process will wait on the child. Once the child process terminates and dies, then the scheduler will be triggered/activated. When the process dies, the scheduler will remove the process from the running/waiting list. The process will be freed.
>
>   - How will you ensure your scheduler runs periodically after a time slice
>     has expired?
		We need to keep track of time and compare it to our time slice. When the time slice expires, we will start the scheduler right away and calculate a new time for the next time slice. 
>
>       Note: you may wish to consider how your response to question 6 can help
>       with this.

Response.

> 5. The client and server need to communicate via a request and response
>    model.
>
>   - Which IPC mechanism will you use: named pipes or unix domain sockets?
>   We will use unix domain sockets.
>
>   - How will you utilize this IPC mechanism?
	We will use sockets to communicate between client and server which are the scheduler and worksim. A few of the commands we may need are bind(), listen(), connect(), accept().  
>
>       Note: you may wish to consider this response in light of your answer in
>       question 6.

Response.

> 6. The server will need to perform its scheduling duties and process requests
>    from clients.
>
>   - How will you multiplex I/O and computation?
    We will multiplex I/O and computation by using the poll() function.

>   - How will you ensure that your I/O will not block indefinitely?
    To ensure that I/O will not block indefinitely, we'd use the timeout functionality of the poll() function.

>   - How will you allow events such as a child process dying interrupt your
>     I/O, but block such an event from interrupting your normal scheduling
>     functions?  Why would this be necessary?
>     To allow a dying child to interrupt I/O but not our normal scheduling function, we'd use sigprocmask(). This would be necessary to allow one process to stop so that another process could go through. 

Response.

> 7. Although FIFO is straightforward, both Round Robin and Multi-Level
>    Feedback Queue require preemption and some extract accounting.
>
>   - How will you perform preemption?  What happens to a process when it is
>     prempted?
	  We will use kill -flag [PID] to halt a process so that another process may run. A process that is premmpted is waiting for a signal so that it may run again. 
>
>   - How will MLFQ determine if a process needs to be lowered in priority?
>     What information must be tracked and how it be updated?
	  MLFQ will determine if a process needs to be lowered if a certain amount of time has passed or has not exceeded the time slice. We need to track the utime for MLFQ to see if it lasts longer than a given time slice. 
>
>   - How will MFLQ determine if a priority boost is required?
	  After a certain amount of time that is preset, then a priority boost will occur which will move all processes to the topmost queue. We will keep track of time using utime. 

Response.

Demonstration
-------------

> [Demo Slides]

Errata
------

> MLFQ doesn't properly pop off from any of the levels. Testing the workloads on rdrn causes a segfault after all the processes have finished. Adding more than 80 processes to our queues causes segfaults. Adding a different time between scheduling causes our rdrn to continuously start process. 

Extra Credit
------------

> Describe what extra credit (if any) that you implemented.




[Project 02]:       https://www3.nd.edu/~pbui/teaching/cse.30341.fa17/project02.html
[CSE.30341.FA17]:   https://www3.nd.edu/~pbui/teaching/cse.30341.fa17/
[Demo Slides]:      https://docs.google.com/a/nd.edu/presentation/d/1lFD3WmMtmFtoUcqz28EGE1yPf2vkKr-5TZcLri6Vbl0/edit?usp=sharing
