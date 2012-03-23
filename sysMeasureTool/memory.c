/*
** This is software is intended to measure basic system functionnalities.
** This is an academic project
*/

#include <stdlib.h>
#include "sysMeasureTool.h"

void		memoryLatency(char **arg)
{
	int	**array;
	void	**p;
	int	size, stride, i;

	for (stride = 1; stride < 16; stride++)
		for (size = MINARRAYSIZE; size <= MAXARRAYSIZE; size++)
		{
			if (!(array = malloc(1 << size)))
			{
				fprintf(stderr, "Malloc failure\n");
				exit(1);
			}
			for (i = 0; i + 1 < (1 << size) / (stride * sizeof(*array)); i++)
				array[i * stride] = (int *)(array + ((i + 1) * stride));
			array[i * stride] = (int *)array;
			p = (void **)array;
			MEASUREOUTLOOP(0, p = *p; asm("");)
			printf("%p\r", p);
			printf("Walked array with stride %d and size %d = %f\n", stride, size,
				gl_lastavgres);
			free(array);
		}
}

void			memoryBandwidth(char **arg)
{
	int		*array, *end;
	int		res = 0;
	unsigned int	size = (1 << MAXARRAYSIZE) / sizeof(*array);

	if (!(array = malloc(1 << MAXARRAYSIZE)))
	{
		fprintf(stderr, "Malloc failure\n");
		exit(1);
	}
	end = array + size;
	for (; array < end; array++)
		*array = 42;
	
	MEASUREOUTLOOP(0, 
	for (array = end - size; array < end; array++)
		res += *array;
	)
	printf("res %d\r", res);
	printf("Average read of %d MB in %f cycles\n", (1 << MAXARRAYSIZE) / (1024 * 1024),
		gl_lastavgres);
	MEASUREOUTLOOP(0, 
	for (array = end - size; array < end; array++)
		*array = 2752512;
	)
	printf("Average write of %d MB in %f cycles\n", (1 << MAXARRAYSIZE) / (1024 * 1024),
		gl_lastavgres);
	free(end - size);
}

void				pageFault(char **arg)
{
	unsigned long long	time = 0;
	unsigned int		i;
	int			*ar1, *end1;
	int			*ar2, *end2;
	int			res;
	unsigned int		size = RAMSIZE / sizeof(*ar1);

	for (i = 0; i < 10; i++)
	{
		if (!(ar1 = malloc(RAMSIZE)))
		{
			fprintf(stderr, "Malloc failure\n");
			exit(1);
		}
		if (!(ar2 = malloc(RAMSIZE)))
		{
			fprintf(stderr, "Malloc failure\n");
			exit(1);
		}
		end1 = ar1 + size;
		for (; ar1 < end1; ar1++)
			*ar1 = 42;
		end2 = ar2 + size;
		for (; ar2 < end2; ar2++)
			*ar2 = 42;
		ar1 = end1 - size;
		{
		unsigned long long b;
		unsigned long long c;
		b = rdtsc();
		res = *ar1;
		c = rdtsc();
		printf("%d\r", res);
		printf("Page in fault time : %lld\n", c - b);
		time += c - b;
		}
		free(end1 - size);
		free(end2 - size);
	}
	printf("Page in fault time average : %lld\n", time / 10);
	printf("Page in fault time per byte average : %lld\n", time / 10 / getpagesize());
}
