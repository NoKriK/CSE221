/*
**
**
** This is software is intended to measure basic system functionnalities.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <thread.h>
#include <wait.h>
#include <strings.h>

unsigned int	gl_loopcycle = 10000;

#define LOOPCYCLE gl_loopcycle

/* Crazy macro to measure outide the loop */
#define MEASUREOUTLOOP(X)	\
{				\
unsigned long long b;		\
unsigned long long c;		\
unsigned int measureloopcnt;		\
				\
b = rdtsc();				\
for (measureloopcnt = 0; measureloopcnt < LOOPCYCLE; ++measureloopcnt)\
{						\
	X;					\
}						\
c = rdtsc();				\
printf("Total execution time is : %llu\nEach execution takes about %f clock cycles (averaging from %d iterations)\n", \
	c - b, (double)(c - b) / (double)LOOPCYCLE, LOOPCYCLE);\
}

/* Crazy macro to measure inside the loop */
#define MEASUREINLOOP(MAX, X) 	\
{				\
unsigned long long b;		\
unsigned long long c;		\
unsigned long long res = 0;	\
unsigned int loopcnt;		\
unsigned int realloop = LOOPCYCLE;		\
				\
for (loopcnt = 0; loopcnt < LOOPCYCLE; ++loopcnt)\
{						\
	b = rdtsc();				\
	X;					\
	c = rdtsc();				\
	printf("%llu\n", c - b);		\
	if (c - b < MAX)			\
		res += c - b;			\
	else					\
		realloop--;			\
}						\
printf("Total execution time is : %llu\nEach execution takes about %f clock cycles (averaging from %d iterations)\n", \
	res, (double)res / (double)realloop, realloop);\
}

/* BEGIN Prototype of the functions to inline */
static inline uint64_t	rdtsc(void) __attribute__((always_inline));
/* END Prototype of the functions to inline */

typedef struct s_measures {
	char	*argname;
	void	(*func)();
}	t_measures;

static const char*	gl_progname;


static uint64_t	inline rdtsc(void)
{
  uint32_t hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( lo | ((uint64_t)hi << 32));
}


static void 	readClock(void)
{
	MEASUREINLOOP(2000, )
}

void* uselessfunc(void *p)
{
	return (NULL);
}

static void 	kernelThread(void)
{
	MEASUREINLOOP(300000,
	thr_create(NULL, 0, uselessfunc, NULL, 0, NULL);
	thr_join(0, NULL, NULL);
	);
}

static void 	processCreation(void)
{
	MEASUREINLOOP(3000000,
		if (fork() == 0)
		{
			_exit(0);
		}
		waitid(P_ALL, 0, NULL, WEXITED);
	);
}

static void 	loop(void)
{
	MEASUREOUTLOOP()
}

static void	proc0(void) { }
static void	proc1(int a) { }
static void	proc2(int a, int b) { }
static void	proc3(int a, int b, int c) { }
static void	proc4(int a, int b, int c, int d) { }
static void	proc5(int a, int b, int c, int d, int f) { }
static void	proc6(int a, int b, int c, int d, int f, int g) { }
static void	proc7(int a, int b, int c, int d, int f, int g, int h) { }

static void 	procCall(void)
{
	fprintf(stdout, "Procedure call cost with 0 argument :\n");
	MEASUREOUTLOOP(proc0())
	fprintf(stdout, "Procedure call cost with 1 argument :\n");
	MEASUREOUTLOOP(proc1(0))
	fprintf(stdout, "Procedure call cost with 2 argument :\n");
	MEASUREOUTLOOP(proc2(0, 42))
	fprintf(stdout, "Procedure call cost with 3 argument :\n");
	MEASUREOUTLOOP(proc3(0, 42, 84))
	fprintf(stdout, "Procedure call cost with 4 argument :\n");
	MEASUREOUTLOOP(proc4(0, 42, 84, 245))
	fprintf(stdout, "Procedure call cost with 5 argument :\n");
	MEASUREOUTLOOP(proc5(0, 42, 84, 245, 23))
	fprintf(stdout, "Procedure call cost with 6 argument :\n");
	MEASUREOUTLOOP(proc6(0, 42, 84, 245, 23, 453))
	fprintf(stdout, "Procedure call cost with 7 argument :\n");
	MEASUREOUTLOOP(proc7(0, 42, 84, 245, 23, 90, 51))
}

static void 	sysCall(void)
{
	MEASUREOUTLOOP(getpid())
}

#define PIPEPROCNB	20
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

static void	pipeOverhead(void)
{
	int	pipetable[PIPEPROCNB * 2];
	int	i;
	char	tok = 42;

	createPipeTab(pipetable, PIPEPROCNB);
	MEASUREOUTLOOP(
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

	MEASUREOUTLOOP(
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

static void		processContextSwitch(void)
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

static void		threadContextSwitch(void)
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


#define CACHETABSI 1024 * 1024 * 32
static void 	cache(void)
{
	int	*tab;
	int	*tab1;
	int	i;
	int	res;

	if (!(tab = malloc(CACHETABSI * sizeof(*tab))) ||
		!(tab1 = malloc(CACHETABSI * sizeof(*tab))))
	{
		fprintf(stderr, "Malloc failed\n");
		exit(1);
	}
	
	MEASUREINLOOP(100000000,
	for (i = 0; i < CACHETABSI; ++i)
	{
		res += tab[i];
	}
	/*bcopy(tab, tab1, CACHETABSI * sizeof(*tab));*/
	)
	free(tab);
	res = res;
}

static const t_measures	gl_funcs[] = {
	{ "kernelThread", kernelThread },
	{ "processCreation", processCreation },
	{ "readClock", readClock },
	{ "loop", loop },
	{ "procCall", procCall },
	{ "sysCall", sysCall },
	{ "cache", cache },
	{ "pipeOverhead", pipeOverhead },
	{ "processContextSwitch", processContextSwitch },
	{ "threadContextSwitch", threadContextSwitch },
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
#include <stdarg.h>
#include <stdio.h>

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
	void	(*fmeasure)(void) = NULL;

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
	if (ac == 3)
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
	fmeasure();
        return (0);
}

