/*
** This is software is intended to measure basic system functionnalities.
** This is an academic project
*/

#include <unistd.h>
#include <stdlib.h>
#include <thread.h>
#include <wait.h>
#include <strings.h>
#include "sysMeasureTool.h"

unsigned int	gl_loopcycle = 10000;
double		gl_lastavgres;

static const char*	gl_progname;

static void 	readClock(char **arg)
{
	MEASUREINLOOP(2000, )
}

void* uselessfunc(void *p)
{
	return (NULL);
}

static void 	kernelThread(char **arg)
{
	MEASUREINLOOP(300000,
	thr_create(NULL, 0, uselessfunc, NULL, 0, NULL);
	thr_join(0, NULL, NULL);
	);
}

static void 	processCreation(char **arg)
{
	MEASUREINLOOP(3000000,
		if (fork() == 0)
		{
			_exit(0);
		}
		waitid(P_ALL, 0, NULL, WEXITED);
	);
}

static void 	loop(char **arg)
{
	MEASUREOUTLOOP(1, asm("");)
}

static void	proc0(void) __attribute__((noinline));
static void	proc1(int a) __attribute__((noinline));
static void	proc2(int a, int b) __attribute__((noinline));
static void	proc3(int a, int b, int c) __attribute__((noinline));
static void	proc4(int a, int b, int c, int d) __attribute__((noinline));
static void	proc5(int a, int b, int c, int d, int f) __attribute__((noinline));
static void	proc6(int a, int b, int c, int d, int f, int g) __attribute__((noinline));
static void	proc7(int a, int b, int c, int d, int f, int g, int h) __attribute__((noinline));

static void	proc0(void) { asm (""); }
static void	proc1(int a) { asm (""); }
static void	proc2(int a, int b) { asm (""); }
static void	proc3(int a, int b, int c) { asm (""); }
static void	proc4(int a, int b, int c, int d) { asm (""); }
static void	proc5(int a, int b, int c, int d, int f) { asm (""); }
static void	proc6(int a, int b, int c, int d, int f, int g) { asm (""); }
static void	proc7(int a, int b, int c, int d, int f, int g, int h) { asm (""); }

static void 	procCall(char **arg)
{
	fprintf(stdout, "Procedure call cost with 0 argument :\n");
	MEASUREOUTLOOP(1, proc0())
	fprintf(stdout, "Procedure call cost with 1 argument :\n");
	MEASUREOUTLOOP(1, proc1(0))
	fprintf(stdout, "Procedure call cost with 2 argument :\n");
	MEASUREOUTLOOP(1, proc2(0, 42))
	fprintf(stdout, "Procedure call cost with 3 argument :\n");
	MEASUREOUTLOOP(1, proc3(0, 42, 84))
	fprintf(stdout, "Procedure call cost with 4 argument :\n");
	MEASUREOUTLOOP(1, proc4(0, 42, 84, 245))
	fprintf(stdout, "Procedure call cost with 5 argument :\n");
	MEASUREOUTLOOP(1, proc5(0, 42, 84, 245, 23))
	fprintf(stdout, "Procedure call cost with 6 argument :\n");
	MEASUREOUTLOOP(1, proc6(0, 42, 84, 245, 23, 453))
	fprintf(stdout, "Procedure call cost with 7 argument :\n");
	MEASUREOUTLOOP(1, proc7(0, 42, 84, 245, 23, 90, 51))
}

static void 	sysCall(char **arg)
{
	MEASUREOUTLOOP(1, getpid())
}

static void	createPipeTab(int *pipetable, int size)
{
	int i;

	for (i = 0; i < size; ++i)
	{
		if (pipe(pipetable + (i * 2)))
		{
			fprintf(stderr, "Pipe creation failed\n");
			exit(1);
		}
	}
}

static void	pipeOverhead(char **arg)
{
	int	pipetable[PIPEPROCNB * 2];
	int	i;
	char	tok = 42;

	createPipeTab(pipetable, PIPEPROCNB);
	MEASUREOUTLOOP(1, 
	for (i = 0; i < PIPEPROCNB; ++i) {
		if (write(pipetable[i * 2], &tok, 1) != 1) {
			fprintf(stderr, "Pipe write failed\n");
			exit(1);
		}
		if (read(pipetable[i * 2 + 1], &tok, 1) != 1) {
			fprintf(stderr, "Pipe read failed\n");
			exit(1);
		}
	}
	)
}

static void	*readAndWritePipe(void *p)
{
	char	tok = 42;
	int	*pipetable = p;

	while (tok)
	{
		if (read(*pipetable, &tok, 1) != 1)
		{
			fprintf(stderr, "Pipe read failed\n");
			exit(1);
		}
		if (write(pipetable[1], &tok, 1) != 1)
		{
			fprintf(stderr, "Pipe write failed\n");
			exit(1);
		}
	}
	return (NULL);
}

static void	measurePipe(int *pipetable)
{
	char	tok = 42;

	MEASUREOUTLOOP(1, 
		if (write(pipetable[0], &tok, 1) != 1)
		{
			fprintf(stderr, "Pipe write failed\n");
			exit(1);
		}
		if (read(pipetable[PIPEPROCNB * 2 - 1], &tok, 1) != 1)
		{
			fprintf(stderr, "Pipe read failed\n");
			exit(1);
		}
	)
	/* Kill all processes/thread by sending a magic token */
	tok = 0;
	if (write(pipetable[0], &tok, 1) != 1)
	{
		fprintf(stderr, "Pipe write failed\n");
		exit(1);
	}
	if (read(pipetable[PIPEPROCNB * 2 - 1], &tok, 1) != 1)
	{
		fprintf(stderr, "Pipe read failed\n");
		exit(1);
	}
}

static void		processContextSwitch(char **arg)
{
	int		pipetable[PIPEPROCNB * 2];
	int		i;

	createPipeTab(pipetable, PIPEPROCNB);
	for (i = 0; i < PIPEPROCNB - 1; ++i)
	{
		if (fork() == 0)
		{
			readAndWritePipe(pipetable + i * 2 + 1);
			_exit(0);
		}
	}
	measurePipe(pipetable);
	/* Avoid zombies */
	for (i = 0; i < PIPEPROCNB - 1; ++i)
		waitid(P_ALL, 0, NULL, WEXITED);
}

static void		threadContextSwitch(char **arg)
{
	static int	pipetable[PIPEPROCNB * 2];
	int	i;

	createPipeTab(pipetable, PIPEPROCNB);
	for (i = 0; i < PIPEPROCNB - 1; ++i)
	{
		if (thr_create(NULL, 0, readAndWritePipe, pipetable + i * 2 + 1, 0, NULL))
		{
			fprintf(stderr, "Thread creation failed\n");
			exit(1);
		}
	}
	measurePipe(pipetable);
	/* Avoid zombies */
	for (i = 0; i < PIPEPROCNB - 1; ++i)
		thr_join(0, NULL, NULL);
}

static const t_measures	gl_funcs[] = {
	{ "kernelThread", kernelThread },
	{ "processCreation", processCreation },
	{ "readClock", readClock },
	{ "loop", loop },
	{ "procCall", procCall },
	{ "sysCall", sysCall },
	{ "pipeOverhead", pipeOverhead },
	{ "processContextSwitch", processContextSwitch },
	{ "threadContextSwitch", threadContextSwitch },
	{ "memoryLatency", memoryLatency },
	{ "memoryBandwidth", memoryBandwidth },
	{ "pageFault", pageFault },
	{ "tcpechoServer", tcpechoServer },
	{ "tcpdiscardServer", tcpdiscardServer },
	{ "tcpechoClient", tcpechoClient },
	{ "tcpbandwidthClient", tcpbandwidthClient },
	{ "tcpconnectClient", tcpconnectClient },
};

static void 	displayUsage(void)
{
	unsigned int i;

	fprintf(stderr, "Usage : %s op loopcycle\n", gl_progname);
	for (i = 0; i < sizeof(gl_funcs) / sizeof(gl_funcs[0]); ++i)
	{
		fprintf(stderr, "%s\n", gl_funcs[i].argname);
	}
}

#include <errno.h>
#include <stdlib.h>
#include <limits.h>

int		xintstrtol(const char *str)
{
	long	ret;

	errno = 0;
	ret = strtol(str, NULL, 10);
	if (errno)
	{
		perror("Error getting numerical value ");
		return (-1);
	}
	else if (ret > INT_MAX || ret < INT_MIN)
	{
		fputs("Value out of range : Integer overflow or underflow.\n", stderr);
		return (-1);
	}
  return ((int)ret);
}

int main(int ac, char **av)
{
	unsigned int i;
	void	(*fmeasure)(char **arg) = NULL;

	gl_progname = av[0];
	if (ac < 2)
	{
		displayUsage();
		return (1);
	}
	for (i = 0; i < sizeof(gl_funcs) / sizeof(gl_funcs[0]); ++i)
	{
		if (!strcmp(av[1], gl_funcs[i].argname))
		{
			fmeasure = gl_funcs[i].func;
			break;
		}
	}
	if (fmeasure == NULL)
	{
		fprintf(stderr, "%s : Invalid operation name.\n", gl_progname);
		displayUsage();
		return (1);
	}
	if (ac >= 3)
	{
		gl_loopcycle = xintstrtol(av[2]);
		if (gl_loopcycle < 0)
		{
			fprintf(stderr, "%s : Invalid loopcycle value.\n", gl_progname);
			displayUsage();
			return (1);
		}
	}
	else
		fprintf(stderr, "%s : No loopcycle value given, falling back to %u.\n", gl_progname, gl_loopcycle);
	fmeasure((ac > 3) ? av + 3: av + ac);
        return (0);
}

