/*
** This is software is intended to measure basic system functionnalities.
** This is an academic project
*/

#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include "sysMeasureTool.h"

#define MSGLEN 2048
#define RRMSG 1460
#define PORT	4242

static void	echoDataHandler(int cfd)
{
	char	buf[MSGLEN];
	int	readed;

	while ((readed = read(cfd, buf, sizeof(buf))) > 0)
	{
		if (write(cfd, buf, readed) != readed)
		{
			fprintf(stderr, "Write failed\n");
		}
	}
}

static void	discardDataHandler(int cfd)
{
	char	buf[MSGLEN];

	while (read(cfd, buf, sizeof(buf)) > 0)
		asm("");
}

static void			tcpServer(void (*dataHandler)(int))
{
	struct sockaddr_in	sin;
	int			sfd;
	int			cfd;
        socklen_t		sinlen;

        sinlen = sizeof(sin);
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(PORT);
	if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		perror("Socket creation failed");
		exit(1);
	}
	if (bind(sfd, (const struct sockaddr *)&sin, sizeof(sin)) == -1 ||
		listen(sfd, 10) == -1)
	{
		perror("Socket bind failed");
		exit(1);
	}
	puts("tcpechoServer started !");
	while (42)
	{
		if ((cfd = accept(sfd, (struct sockaddr *)&sin, &sinlen)) == -1)
		{
			fprintf(stderr, "Accept failed\n");
			exit(1);
		}
		dataHandler(cfd);
		close(cfd);
		puts("Client disconnected.");
	}
	close(sfd);
	puts("tcpechoServer shuted down.");
}

void	tcpechoServer(char **arg)
{
	tcpServer(&echoDataHandler);
}

void	tcpdiscardServer(char **arg)
{
	tcpServer(&discardDataHandler);
}

static int		initClient(char *host, struct sockaddr_in *sin)
{
        struct hostent	*h;

	if (!host)
		fprintf(stderr, "No remote hostname given.\n");
        else if (!(sin->sin_port = htons(PORT)))
                fputs("Bad port number.\n", stderr);
        else if (!(h = gethostbyname(host)))
                fprintf(stderr, "Error resolving \"%s\" : %s.\n",
                 host, strerror(h_errno));
        else
        {
		memcpy(&(sin->sin_addr), h->h_addr_list[0], sizeof(sin->sin_addr));
                sin->sin_family = AF_INET;
		return (0);
	}
	return (-1);
}

void				tcpechoClient(char **arg)
{
        struct sockaddr_in	sin;
        int			sfd;
	char			buf[MSGLEN];

	memset(buf, 42, RRMSG);
	if (initClient(*arg, &sin) == -1)
		exit(1);
	if ((sfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Socket creation failed.");
		exit(1);
	}
	if (connect(sfd, (struct sockaddr *)&sin, sizeof(sin)))
	{
		perror("Socket connect failed");
		exit(1);
	}
	MEASUREOUTLOOP(1,
	if (write(sfd, buf, RRMSG) != RRMSG || 
		read(sfd, buf, RRMSG) != RRMSG)
	{
		perror("Write or read failed");
		exit(1);
	}
	)
	close(sfd);
}

void				tcpbandwidthClient(char **arg)
{
        struct sockaddr_in	sin;
        int			sfd;
	char			buf[MSGLEN];
	int			res;
	int			written;

	memset(buf, 42, RRMSG);
	if (initClient(*arg, &sin) == -1)
		exit(1);
	if ((sfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Socket creation failed.");
		exit(1);
	}
	if (connect(sfd, (struct sockaddr *)&sin, sizeof(sin)))
	{
		perror("Socket connect failed");
		exit(1);
	}
	MEASUREOUTLOOP(1,
	res = 0;
	while (res < NETTRANSFERSIZE)
	{
		if ((written = write(sfd, buf, RRMSG)) == -1)
		{
			perror("Write failed");
			exit(1);
		}
		res += written;
	}
	)
	close(sfd);
}

void				tcpconnectClient(char **arg)
{
	struct timespec		pauseTime;
        struct sockaddr_in	sin;
	unsigned long long 	b, c, con = 0, clo = 0;
        int			sfd, i;

	pauseTime.tv_sec = 0;
	pauseTime.tv_nsec = 1000000;
	if (initClient(*arg, &sin) == -1)
		exit(1);
	for (i = 0; i < LOOPCYCLE; i++)
	{
		if ((sfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		{
			perror("Socket creation failed.");
			exit(1);
		}
		b = rdtsc();
		if (connect(sfd, (struct sockaddr *)&sin, sizeof(sin)))
		{
			perror("Socket connect failed");
			exit(1);
		}
		c = rdtsc();
		con += c - b;
		b = rdtsc();
		close(sfd);
		c = rdtsc();
		clo += c - b;
		nanosleep(&pauseTime, NULL);
	}
	printf("Connection avg time : %lld\n", con / LOOPCYCLE);
	printf("Close avg time : %lld\n", clo / LOOPCYCLE);
}
