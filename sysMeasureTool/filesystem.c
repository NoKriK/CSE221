/*
** This is software is intended to measure basic system functionnalities.
** This is an academic project
*/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <wait.h>
#ifdef __linux
# include <time.h>
#endif
#include "sysMeasureTool.h"

#define FSMAXSIZE	((unsigned int)(1 << 30) * 2)
#define GAPSIZE		(1 << 27)
#define MAXFSIZE	(1 << 31)
#define MINFSIZE	(1 << 24)
#define CONTSIZE	(1 << 27)
#define CONTPROC	(1 << 5)
#define BUFSIZE		2048

#define FILEBUFSIZE	(128 * 1024)

void			fsCacheSize(char **arg)
{
	char		filename[] = "./fileCacheBenchXXXXXX";
	unsigned int	targetsize = 0;
	unsigned int	cursize = 0;
	int		fd;
	char		buf[BUFSIZE];
	int		res;

	memset(buf, 42, BUFSIZE);
	if ((fd = mkstemp(filename)) == -1)
	{
		perror("File creation failed");
		exit(1);
	}
	printf("Temporary file name is %s\n", filename);
	for (targetsize = GAPSIZE; targetsize < FSMAXSIZE; targetsize += GAPSIZE)
	{
		while (cursize < targetsize)
		{
			if ((res = write(fd, buf, ((targetsize - cursize) % BUFSIZE) + 1)) == -1)
			{
				perror("Write error");
				exit(1);
			}
			cursize += res;
		}
		MEASUREOUTLOOP(0,
			if (lseek(fd, 0, SEEK_SET) == -1)
			{
				perror("Lseek error");
				exit(1);
			}
			do
			{
				if ((res = read(fd, buf, BUFSIZE)) == -1)
				{
					perror("Read error");
					exit(1);
				}
			}
			while (res)
		)
		printf("Average cycles for file of %dMB = %f\n", cursize / (1024 * 1024),
			gl_lastavgres);
	}
	if (unlink(filename) == -1)
		perror("File remove failed");
}

void			readFile(char **arg)
{
	char		filename[] = "./fileCacheBenchXXXXXX";
	unsigned int	targetsize = 0;
	unsigned int	cursize = 0;
	int		fd;
	char		buf[FILEBUFSIZE];
	int		res;
	int		readed;

	srandom(time(NULL) + getpid());
	memset(buf, 42, BUFSIZE);
	if ((fd = mkstemp(filename)) == -1)
	{
		perror("File creation failed");
		exit(1);
	}
	printf("Temporary file name is %s\n", filename);
	for (targetsize = MINFSIZE; targetsize < MAXFSIZE; targetsize = targetsize << 1)
	{
		while (cursize < targetsize)
		{
			if ((res = write(fd, buf, ((targetsize - cursize) % FILEBUFSIZE) + 1)) == -1)
			{
				perror("Write error");
				exit(1);
			}
			cursize += res;
			printf("Wrote %d bytes of %d bytes\r", cursize, targetsize);
		}
		MEASUREOUTLOOP(0,
			readed = 0;
			do
			{
				res = random();
				res = (res - (res % FILEBUFSIZE)) % (cursize - FILEBUFSIZE);
				if (lseek(fd, res, SEEK_SET) == -1)
				{
					perror("Lseek error");
					exit(1);
				}
				if ((res = read(fd, buf, FILEBUFSIZE)) == -1 || res == 0)
				{
					perror("Read error");
					exit(1);
				}
				readed += res;
				printf("Readed %d bytes of %d bytes\r", readed, cursize);
			}
			while (readed < cursize);
		)
		printf("Random read of %dMB file = %f cycles\n", cursize / (1024 * 1024),
			gl_lastavgres);
		MEASUREOUTLOOP(0,
			if (lseek(fd, 0, SEEK_SET) == -1)
			{
				perror("Lseek error");
				exit(1);
			}
			do
			{
				if ((res = read(fd, buf, FILEBUFSIZE)) == -1)
				{
					perror("Read error");
					exit(1);
				}
			}
			while (res)
		)
		printf("Sequential read of %dMB file = %f cycles\n", cursize / (1024 * 1024),
			gl_lastavgres);
	}
	if (unlink(filename) == -1)
		perror("File remove failed");
}

void			readContention(char **arg)
{
	char		*filenametpl = "./tmpReadContentionBenchXXXXXX";
	char		*filename[CONTPROC];
	char		buf[FILEBUFSIZE];
	int		fd[CONTPROC];
	int		res;
	unsigned int	i;
	unsigned int	j;
	unsigned int	size;

	memset(buf, 42, FILEBUFSIZE);
	for (i = 0; i < CONTPROC; i++)
	{
		filename[i] = strdup(filenametpl);
		if ((fd[i] = mkstemp(filename[i])) == -1)
		{
			perror("File creation failed");
			exit(1);
		}
		for (size = 0; size < CONTSIZE; size += res)
			if ((res = write(fd[i], buf, FILEBUFSIZE)) == -1)
			{
				perror("Write error");
				exit(1);
			}
		printf("Temporary file %s created.\n", filename[i]);
	}
	for (i = 1; i <= CONTPROC; i = i << 1)
	{
		printf("Starting %d processes...\n", i);
		for (j = 0; j < i; j++)
		{
			if (!(res = fork()))
			{
				MEASUREOUTLOOP(0,
					if (lseek(fd[j], 0, SEEK_SET) == -1)
					{
						perror("Lseek error");
						exit(1);
					}
					do
					{
						if ((res = read(fd[j], buf, FILEBUFSIZE)) == -1)
						{
							perror("Read error");
							exit(1);
						}
					}
					while (res)
				)
				printf("Sequential read of %dMB file = %f cycles\n", CONTSIZE / (1024 * 1024),
					gl_lastavgres);
				free(filename[j]);
				return;
			}
			else if (res == -1)
				perror("fork");
		}
		for (j = 0; j < i; j++)
			waitid(P_ALL, 0, NULL, WEXITED);
	}
	for (i = 0; i < CONTPROC; i++)
	{
		close(fd[i]);
		if (unlink(filename[i]) == -1)
			perror("File remove failed");
		free(filename[j]);
	}
}
