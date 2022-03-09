#include "mftp.h"


void change_dir(char* directory, int socketfd);
int data_connect(int socketfd);
void list(int connectfd, int socketfd);
void quit(int socketfd);
void get(int socketfd, int connectfd, char *s);
void put(int socketfd, int connectfd, char *s);



void put(int socketfd, int connectfd, char *s) {
	printf("Child %d: ready to put\n", getpid());
	char response[100];
	int temp;
	char c;
	char *temp_path;
	temp_path = s;

	int exist = access(temp_path, F_OK);
	if (exist == 0) {
		strcpy(response, "E");
		strcat(response, "file already exists\n");
		write(socketfd, response, strlen(response));
		close(connectfd);
	}

	else {
		int fd = open(temp_path, O_RDWR | O_CREAT, S_IRWXU);
		if (fd < 0) {
			strcpy(response, "E");
			strcat(response, "cannot open or create the file\n");
			write(socketfd, response, strlen(response));
			close(connectfd);
		}

		else {
			strcpy(response, "A");
			strcat(response, "\n");
			write(socketfd, response, strlen(response));
			while((temp = read(connectfd, &c, 1)) > 0) {
				write(fd, &c, 1);
			}
			close(connectfd);
		}
		close(fd);
	}
}

void get(int socketfd, int connectfd, char *s) {
	printf("Child %d: ready to gte\n", getpid());
	char response[100];
	struct stat area, *file_area = &area;
	int temp;
	char c;
	lstat(s, file_area);
	int exist = access(s, F_OK);
	if (exist != 0) {
		strcpy(response, "E");
		strcat(response, "file doesn't exist\n");
		write(socketfd, response, strlen(response));
		close(connectfd);
	}

	else if (S_ISREG(file_area->st_mode) != true) {
		strcpy(response, "E");
		strcat(response, "not a regular file\n");
		write(socketfd, response, strlen(response));
		close(connectfd);
	}

	else {
		int fd = open(s, O_RDONLY, S_IRWXU);
		if (fd < 0) {
			strcpy(response, "E");
			strcat(response, "cannot open the file with read only permission\n");
			write(socketfd, response, strlen(response));
			close(connectfd);
		}

		else {
			strcpy(response, "A");
			strcat(response, "\n");
			write(socketfd, response, strlen(response));
			while((temp = read(fd, &c, 1)) >0) {
				write(connectfd, &c, 1);
			}

			close(connectfd);
		}
		close(fd);
	}
				

		

}

void quit(int socketfd) {
	printf("\nChild %d: ready to exit the child\n", getpid());
	write(socketfd, "A\n", 2);
	close(socketfd);
	printf("Child %d: Quitting\n", getpid());
	exit(0);
}

void change_dir(char* directory, int socketfd) {
	char temp[100];
	char response[100];
	int temp_error;
	printf("old cwd: %s\n", getcwd(temp, 100));

	temp_error = chdir(directory);

	if(temp_error != 0) {
		response[0] = 0;
		strcpy(response, "E");
		strcat(response, strerror(errno));
		strcat(response, "\n");
		write(socketfd, response, strlen(response));
		printf("Child %d: unable to change directory\n", getpid());
	}
	else{	
		strcpy(response, "A");
		strcat(response, "\n");
		write(socketfd, response, strlen(response));
		printf("Child %d: able to cd, the new cwd: %s\n", getpid(), getcwd(temp, 100));

	}
	response[0] = 0;
	return;
}

void list(int connectfd, int socketfd) {
	int status;
	int fork_error;
	char response[100];
	if((fork_error = fork()) == -1) {
		strcpy(response, "E");
		strcat(response, strerror(errno));
		strcat(response, "\n");
		write(socketfd, response, strlen(response));
		printf("Child %d: unable to ls\n", getpid());
	}
	if(fork_error == 0) {
		int dup_error = dup2(connectfd, 1);
		strcpy(response, "A");
		strcat(response, "\n");
		write(socketfd, response, strlen(response));
		execlp("ls", "ls", "-l", (char*) NULL);
		strcpy(response, "E");
		strcat(response, "ls err");
		strcat(response, "\n");
		write(socketfd, response, strlen(response));
		printf("Child %d: unable to execlp\n", getpid());
	}


	else {
		close(connectfd);
		wait(&status);
	}
	return;
}
		
	

int data_connect(int socketfd) {
	struct sockaddr_in servAddr;
	struct sockaddr_in clientAddr;
	int length = sizeof(servAddr);
	char A_buf[10];
	char port_num_buf[10];
	char *s;
	 
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);

	if(listenfd == -1) {
		fprintf(stderr, "%s\n", strerror(errno));
		return -1;
	}
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = 0;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(listenfd, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0) {
		fprintf(stderr, "%s\n", strerror(errno));
		return -1;
	}

	int name_error = getsockname(listenfd, (struct sockaddr*) &servAddr, &length);
	if(name_error == -1) {
		fprintf(stderr, "%s\n", strerror(errno));
		return -1;
	}
	int port_num = ntohs(servAddr.sin_port);
	int listen_error = listen(listenfd, 1);
	if(listen_error != 0) {
		fprintf(stderr, "%s\n", strerror(errno));
		return -1;
	}

	
	strcpy(A_buf, "A");
	sprintf(port_num_buf, "%d", port_num);
	strcat(A_buf, port_num_buf);
	strcat(A_buf, "\n");
	

	
	write(socketfd, A_buf, strlen(A_buf));
	int connectfd = accept(listenfd, (struct sockaddr*) &clientAddr, &length);

	if(connectfd == -1) {
		fprintf(stderr, "%s\n", strerror(errno));
		return -1;
	}
	else {
		//printf("server was able to accpet data connection\n");
		return connectfd;
	}
}


int main(int argc, char const *argv[]) {
		struct sockaddr_in servAddr;
		int socketfd, listenfd;
		int port_num = atoi(argv[2]);

		listenfd = socket(AF_INET, SOCK_STREAM, 0);
		if(listenfd == -1) {
			
			fprintf(stderr, "%s\n", strerror(errno));
			exit(-1);
		}
		int set_error = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(int){1}, sizeof(int));
		if(set_error != 0) {

			fprintf(stderr, "%s\n", strerror(errno));
			exit(-2);
		}
		memset(&servAddr, 0, sizeof(servAddr));
		servAddr.sin_family = AF_INET;
		servAddr.sin_port = htons(port_num);
		servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		if(bind(listenfd, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0) {
		
			fprintf(stderr, "%s\n", strerror(errno));
			exit(-3);
		}
		int listen_error = listen(listenfd, BACKLOG);
		if(listen_error != 0) {
			
			fprintf(stderr, "%s\n", strerror(errno));
			exit(-4);
		}

		int connectfd;
		int length = sizeof(struct sockaddr_in);
		struct sockaddr_in clientAddr;
		char hostName[NI_MAXHOST];
		int hostEntry;

		while(1) {
			int status;
			int fork_error;
			int temp;
			char c;
			char d;
			int first_char;
			char command_buf[100];
			char command_buf2[100];
			int command_index = 0;
			int connectfd;
			char data_connect_resp[100];
			socketfd = accept(listenfd, (struct sockaddr*) &clientAddr, &length);
			if(socketfd == -1) {
				printf("%s\n", strerror(errno));
				exit(-6);
			}
			if((fork_error = fork())== -1) {
				printf("cannot fork, exiting\n");
				exit(-1);
			}
			
			else if(fork_error == 0) {
				
				hostEntry = getnameinfo((struct sockaddr*)&clientAddr, 
						sizeof(clientAddr),
						hostName,
						sizeof(hostName),
						NULL,
						0,
						NI_NUMERICSERV);
				if(hostEntry != 0) {
					fprintf(stderr, "%s\n", gai_strerror(hostEntry));
				}
				else {
					printf("Child %d: Client IP address: %s\n", getpid(), inet_ntoa(servAddr.sin_addr));
					printf("Child %d: Accepted connection from: %s\n", getpid(), hostName);
				}

				command_buf[0] = 0;
				command_buf2[0] = 0;
				data_connect_resp[0] = 0;
				while(1) {
					command_buf[0] = 0;
					command_buf2[0] = 0;
					while((temp = read(socketfd, &c, 1))) {
						command_buf[command_index] = c;
						command_index++;
						if( c =='\n') {
							break;
						}
					}
					command_index = 0;
					if(temp == -1 || temp == 0) {
						printf("Child %d: fatal error, exiting\n", getpid());

						exit(0);
					}
					
					if(command_buf[0] == 'Q') {
						quit(socketfd);
						command_buf[0] = 0;
						command_buf[0] = 0;
					}

					if(command_buf[0] == 'C') {
						printf("Child %d: ready to change direcotry\n", getpid());
						char *s = command_buf;
						int index = 0;
						while(*s++ != '\n') index++;
						command_buf[index] = '\0';
						s = (command_buf + 1);
						command_buf[0] = 0;
						command_buf2[0] = 0;
						change_dir(s, socketfd);
					}

				



					while( command_buf[0] == 'D') {
						int connectfd = data_connect(socketfd);
						if (connectfd == -1) {
							strcpy(data_connect_resp, "E");
							strcat(data_connect_resp, "was not able to build data connection\n");
							write(socketfd, data_connect_resp, strlen(data_connect_resp));
						}

						else if(connectfd != -1) {
							char x;
							int index_2 = 0;
							while((temp = read(socketfd, &x, 1))) {
								command_buf2[index_2] = x;
								index_2++;
								if(x == '\n') {
									break;
								}
							}
							index_2 = 0;

								

							if(command_buf2[0] == 'L') {
								printf("Child %d: ready to ls\n", getpid());
								char *s = (command_buf + 1);
								strtok(s, "\n");
								list(connectfd, socketfd);
								command_buf[0] = 0;
								command_buf2[0] = 0;
								close(connectfd);
								break;
							}
						
							if(command_buf2[0] == 'G') {
								char *s = (command_buf2 + 1);
								strtok(s, "\n");
								get(socketfd, connectfd, s);
								break;
							}

							if(command_buf2[0] == 'P') {
								char *s = (command_buf2 + 1);
								strtok(s, "\n");
								put(socketfd, connectfd, s);
								break;

							}



					}
				       		
				}
			}
		}


	
		else{	
			close(socketfd);
			waitpid(-1, &status, WNOHANG);
		}

	}
	

		
	return 0;
}
