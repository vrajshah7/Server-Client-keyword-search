#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define MSIZ 80
#define PORT 2022
#define SA struct sockaddr


void resetBuf(char* b)
{
	for (int i = 0; i < MSIZ; i++)
		b[i] = '\0';
}

void handleCli(int connect_fd)
{
	char pattern[MSIZ], serverFilePath[MSIZ];
	char grep[100];
	resetBuf(pattern);
	resetBuf(serverFilePath);
	read(connect_fd, serverFilePath, sizeof(serverFilePath));
	printf("client file: %s\n", serverFilePath);
	read(connect_fd, pattern, sizeof(pattern));
	printf("pattern: %s\n", pattern);

	int fd[2];
	int k=pipe(fd);
	if(k == -1)
		exit(1); 
	if(fork() == 0){
		close(fd[0]);

		dup2(fd[1], 1);
		resetBuf(grep);
		sprintf(grep, "grep --color=always --with-filename '%s' %s", pattern,serverFilePath);
		system(grep);

	}
	else {
		char output[MSIZ];
		close(fd[1]);
		dup2(fd[0], 0);
		resetBuf(output);
		read(fd[0], &output, MSIZ);
	
		write(connect_fd, output, sizeof(output));
	}

}

int main()
{
	int socket_fd, connect_fd, len;
	struct sockaddr_in servaddr, acc;

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1) {
		printf("failure in creating socket\n");
		exit(0);
	}
	else
		printf("creation of socket was success\n");
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	
	if ((bind(socket_fd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind was a failure\n");
		exit(0);
	}
	else
		printf("Socket bind was a success\n");

	if ((listen(socket_fd, 5)) != 0) {
		printf("Listen function failed\n");
		exit(0);
	}
	else
		printf("listen function success\n");
	len = sizeof(acc);

	Check:;
	printf("\nWaiting on Client input\n");
	connect_fd = accept(socket_fd, (SA*)&acc, &len);
	if (connect_fd < 0) {
		printf("accept function failed\n");
		exit(0);
	}
	else
		printf("accept function success\n");

	handleCli(connect_fd);
	goto Check;

	close(socket_fd);
}

