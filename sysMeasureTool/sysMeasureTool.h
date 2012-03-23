#ifndef SYSMEASURETOOL_H_
# define SYSMEASURETOOL_H_

# include <unistd.h>
# include <stdio.h>

# define LOOPCYCLE gl_loopcycle

/*
** This is used for the process/thread context switch time
** measurement
*/
# define PIPEPROCNB	20

/*
** This values are used to measure the memory latency
** They are used ase log2(value)
*/
# define MINARRAYSIZE	9
# define MAXARRAYSIZE	27

/*
** This value is used to measure the page fault time
*/
# define RAMSIZE	((unsigned int)(1 << 30) * 2)

/*
** This value is used to measure the network bandwidth
*/
# define NETTRANSFERSIZE	((unsigned int)(1 << 30) * 2)

extern double	gl_lastavgres;

/* Crazy macro to measure outside the loop */
# define MEASUREOUTLOOP(PRINT, X)	\
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
if (PRINT)				\
	printf("Total execution time is : %llu\nEach execution takes about %f clock cycles (averaging from %d iterations)\n", \
	c - b, (double)(c - b) / (double)LOOPCYCLE, LOOPCYCLE);	\
gl_lastavgres =  (double)(c - b) / (double)LOOPCYCLE;	\
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

extern unsigned int	gl_loopcycle;

typedef struct s_measures {
	char	*argname;
	void	(*func)(char **arg);
}	t_measures;

static inline uint64_t	rdtsc(void) __attribute__((always_inline));

static inline uint64_t	rdtsc(void)
{
  uint32_t		hi, lo;

  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( lo | ((uint64_t)hi << 32));
}


void		memoryLatency(char **arg);
void		memoryBandwidth(char **arg);
void		pageFault(char **arg);
void		tcpechoServer(char **arg);
void		tcpdiscardServer(char **arg);
void		tcpechoClient(char **arg);
void		tcpbandwidthClient(char **arg);
void		tcpconnectClient(char **arg);

#endif /* !SYSMEASURETOOL_H_ */
