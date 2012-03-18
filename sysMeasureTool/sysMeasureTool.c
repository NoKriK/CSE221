/*
**
**
** This is software is intended to measure basic system functionnalities.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
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
#define MEASUREINLOOP(X) 	\
{				\
unsigned long long b;		\
unsigned long long c;		\
unsigned long long res = 0;	\
unsigned int loopcnt;		\
				\
for (loopcnt = 0; loopcnt < LOOPCYCLE; ++loopcnt)\
{						\
	b = rdtsc();				\
	X;					\
	c = rdtsc();				\
	printf("%llu\n", c - b);		\
	res += c - b;				\
}						\
printf("Total execution time is : %llu\nEach execution takes about %f clock cycles (averaging from %d iterations)\n", \
	res, (double)res / (double)LOOPCYCLE, LOOPCYCLE);\
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


void* uselessfunc(void *p)
{
	return (NULL);
}

static void 	readClock(void)
{
	MEASUREINLOOP()
}

static void 	kernelThread(void)
{
	MEASUREINLOOP(
	pthread_create(NULL, NULL, &uselessfunc, NULL)
	);
}

static void 	processCreation(void)
{
	MEASUREINLOOP(
		if (fork() == 0)
		{
			_exit(0);
		}
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
	
	MEASUREINLOOP(
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

