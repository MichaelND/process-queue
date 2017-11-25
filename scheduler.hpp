/* scheduler.hpp */

#ifndef SCHEDULER_HPP
#define	SCHEDULER_HPP

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <array>
#include <deque>
#include <iostream>
#include <string>

/* Macros */
#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...)   fprintf(stderr, "[%5d] DEBUG %10s:%-4d " M "\n", getpid(), __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define 	log(M, ...)     fprintf(stderr, "[%5d]  INFO %10s:%-4d " M "\n", getpid(), __FILE__, __LINE__, ##__VA_ARGS__)


enum Policy {
	FIFO,
	RDRN,
	MLFQ,
};

enum Request {
	ADD,
	STATUS,
	RUNNING,
	WAITING,
	FLUSH,
};

struct Process {
	int				num;
	int 			pid;
	char		   *command;
	char		   *subsequent;
	char 		   *state;
	int 			user;
	int 			threshold;
	double			usage;
	time_t			arrival;
	time_t 		 	start;
	time_t			finished;
	int 			priority;
	bool 			started;
};

class Scheduler
{
public:
	Scheduler();
	~Scheduler();

	void ServerFIFO();
	void ServerRDRN();
	void ServerMLFQ(int &bc);

	void ProcessResume(Process &p);
	void ProcessStart(Process &p);
	void ProcessPause(Process &p);
	void HandleAddRequest(char *buffer, FILE *client_stream, char *subsequent);
	void HandleStatusRequest(FILE *client_stream);
	void HandleProcessRequest(FILE *client_stream, bool flag);
	void HandleFlushRequest(FILE *client_stream);
	void makeProcessString(std::deque<Process> &d, char *buffer);
	void makeMLFQString(char *buffer);

	void SetCommand(std::string command);
	void SetMicroseconds(int n);
	void SetNCPUS(int n);
	void SetPolicy(std::string policy);
	void SetSocketpath(std::string path);
	void SetSubsequent(std::string subsequent);
	void UpdateProcess(Process &p, int pid);

	std::string GetSocketpath();

	void RunningPop();

	char *stateString(char *c);

	bool checkExceedThreshold(Process &p);
	void doBoost();

private:
	std::string socket_path;
	std::string command;
	std::string subsequent;
	std::string policy;
	int removeRunning = 0;
	int cpus = 0;
	int levels = 0;
	int microseconds = 0;
	double turnaround = 0; 
	double response = 0; 
	int totalprocesses = 1;
	int timeboost = 25000;

	std::deque<Process> running;
	std::deque<Process> waiting;
	std::array<std::deque<Process>, 8> mlfq_levels;
};


/* Function Prototypes */
void RunClient(std::string command, std::string socketpath);
void RunServer(Scheduler &s, Policy p, int microseconds);

#endif
