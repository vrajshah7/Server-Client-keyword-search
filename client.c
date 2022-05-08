#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 2022
#define SA struct sockaddr
#define MSIZ 80
#define IP_PRO 0
#ifdef WIN32
#pragma warning (disable: #num of the warning)
#endif

void resetBuf(char* f2)
{
	for (int i = 0; i < MSIZ; i++)
		f2[i] = '\0';
}

void handleCli(int socket_fd, char *argv[])
{
	char temp_buffer[MSIZ];
	char grep[100];
	resetBuf(temp_buffer);
	write(socket_fd, argv[3], sizeof(temp_buffer));
	write(socket_fd, argv[1], sizeof(temp_buffer));
	resetBuf(temp_buffer);
	read(socket_fd, temp_buffer, sizeof(temp_buffer));
	
	int fd[2];
	int k=pipe(fd);
	if(k == -1)
		exit(1); 
	if(fork() == 0){
		close(fd[0]);
		resetBuf(grep);
		sprintf(grep, "grep --color=always --with-filename '%s' %s", argv[1],argv[2]);
		system(grep);
	}
	else {
		char output[MSIZ];
		close(fd[1]);
		resetBuf(output);
		read(fd[0], &output, MSIZ);
        for(int i=0 ; i<sizeof(temp_buffer) ; i++) {
            if(temp_buffer[i] == '\n')
                break;
            else
                printf("%c", temp_buffer[i]);
        }
		
	}
	
}

int fileC(char* path)
{
	FILE* file = fopen(path, "r");
  	if(file)
    {
  		fclose(file);
      	return 1;
    }
  	return 0;
}

int main(int argc, char *argv[])
{
	int socket_fd, connect_fd;
	int f1,f2;
	struct sockaddr_in servaddr, acc;
	
	if(argc==4) 
	{
		f1 = fileC(argv[2]);
		f2 = fileC(argv[3]);
		if(f1==0 && f2==0)
		{
			printf("\nboth files not found\n");
			exit(0);
		}
		else if(f1==0 || f2==0)
		{
			if(f1==0)
			{
				printf("\nfile1 not found\n");
				exit(0);
			}
			else if(f2==0)
			{
				printf("\nfile2 not found\n");
				exit(0);
			}
		}
		
	}
	else if(argc!=4)
	{
		printf("\nnot enough arguments: requires arguments (pattern file1 file2)\n");
		exit(0);
	}

	socket_fd = socket(AF_INET, SOCK_STREAM, IP_PRO);
	if (socket_fd == -1) 
	{
		printf("\nfailure in creating socket\n");
		exit(0);
	}
	else
	{
		printf("\ncreation of socket was success\n");
	}

	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("137.207.82.53");
	servaddr.sin_port = htons(PORT);


	if (connect(socket_fd, (SA*)&servaddr, sizeof(servaddr)) != 0) 
	{
		printf("\nconnecting to server failed\n");
		exit(0);
	}
	else
		printf("\nserver connected\n\n");

	handleCli(socket_fd, argv);

	close(socket_fd);
}

