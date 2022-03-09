#include "mftp.h"


void change_dir(char* directory);
void list();
void rcd(int socketfd, char *second_token);
int data_connect(const char *port_num, const char *host_name);
void rls(int socketfd, const char *argv2);
void show(int socketfd, char *second_token, const char *argv2);
void get(int socketfd, char *second_token, const char *argv2);
void put(int socketfd, char *second_token, const char *argv2);
void exit(int socketfd);

void list() {
	int fd[2];
	int rdr, wtr;
	int fork_error;
	int fork_error2;
	int status;	
	if((fork_error = fork()) == -1) {
		printf("%s\n", strerror(errno));
	}	

	if(fork_error == 0) {
		int pipe_error = pipe(fd);
		if(pipe_error == -1) {
			printf("%s\n", strerror(errno));
		}
		rdr = fd[0];
		wtr = fd[1];
		if((fork_error = fork()) == -1) {
			printf("%s\n", strerror(errno));
		}

		if(fork_error == 0) {
			close(rdr);
			int dup_error = dup2(wtr, 1);
			if(dup_error == -1) {
				printf("%s\n", strerror(errno));
			}

			execlp("ls", "ls", "-l", (char*) NULL);
			printf("%s\n", strerror(errno));
		}
		else {	
			close(wtr);
			int dup_error2 = dup2(rdr, 0);
			if(dup_error2 == -1) {
				printf("%s\n", strerror(errno));
			}

			wait(&status);
			execlp("more", "more", "-20", (char*) NULL);
			printf("%s\n", strerror(errno));
		}
	}
	
	else {
		wait(&status);
	

	}
	return;
}



void change_dir(char* directory) {
	char temp[MAX_INPUT];
	printf("old cwd: %s\n", getcwd(temp, MAX_INPUT));
	int temp_error = chdir(directory);
	if(temp_error != 0) {
		printf("unable to change directory\n");
	}
	else{
		printf("new cwd: %s\n", getcwd(temp, MAX_INPUT));
	}
	return;
}

void rcd(int socketfd, char *second_token) {
	printf("ready to rcd\n");
	char c;
	char C_command[100];
	char temp_buf2[100];
	int temp;
	int index = 0;
	int temp2;
	strcpy(C_command, "C");
	strcat(C_command, second_token);
	strcat(C_command, "\n");

	write(socketfd, C_command, strlen(C_command));

	while((temp2 = read(socketfd, &c, 1))) {
		temp_buf2[index] = c;
		index++;
		if (c == '\n') {
			break;
		}
	}

	temp_buf2[index+1] = '\0';
	if (temp_buf2[0] == 'E') {
		char *x = (temp_buf2 + 1);
		printf("Error, error message from the server:%s", x);
	}

	else if (temp_buf2[0] = 'A') {
	}
	return;
}

void rls(int socketfd, const char *argv2) {
	printf("ready to rls\n");
	char temp_buf1[1000];
	char temp_buf2[100000];
	int temp;
	int temp2;
	char c;
	int fork_error;
	int status;
	int fd[2];
	int rdr, wtr;
	int index = 0;
	write(socketfd, "D\n", 2);
	read(socketfd, temp_buf1, 1000);

	if (temp_buf1[0] == 'E') {
		printf("was not able to build the data connection\n");
	}

	else {
		char *s = (temp_buf1 + 1);
		strtok(s, "\n");

		int connectfd = data_connect(s, argv2);

		write(socketfd, "L\n", 2);
		while((temp2 = read(socketfd, &c, 1))) {
			temp_buf2[index] = c;
			index++;
			if (c == '\n') {
				break;
			}
		}

		temp_buf2[index+1] = '\0';
		if (temp_buf2[0] == 'E') {
			char *x = (temp_buf2 + 1);
			printf("Error, error message from the server:%s", x);
		}

		else if (temp_buf2[0] = 'A') {
			printf("server is ready to rls\n");

			if ((fork_error = fork()) == -1) {
				printf("%s\n", strerror(errno));
			}

			if(fork_error == 0) {
				int dup_error2 = dup2(connectfd, 0);
				execlp("more", "more", "-20", (char*) NULL);
				printf("%s\n", strerror(errno));
			}

			else {
				wait(&status);
			}
		}
		
		close(connectfd);
	}
}

void show(int socketfd, char *second_token, const char *argv2) {
	//printf("ready to show\n");
	char temp_buf[100];
	char temp_buf2[100];
	char G_command[100];
	char c;
	int temp;
	int index = 0;
	int temp2;
	int status;
	int fork_error;
	strcpy(G_command, "G");
	strcat(G_command, second_token);
	strcat(G_command, "\n");
	
	write(socketfd, "D\n", 2);
	read(socketfd, temp_buf, 100);
	if (temp_buf[0] == 'E') {
		printf("was not able to build the data connection\n");
	}
	else {
		char *s = (temp_buf + 1);
		strtok(s, "\n");

		int connectfd = data_connect(s, argv2);

		write(socketfd, G_command, strlen(G_command));
		while((temp2 = read(socketfd, &c, 1))) {
			temp_buf2[index] = c;
			index++;
			if (c == '\n') {
				break;
			}
		}

		temp_buf2[index+1] = '\0';
		if (temp_buf2[0] == 'E') {
			char *x = (temp_buf2 + 1);
			printf("Error, error message from the server: %s", x);
		}

		else if (temp_buf2[0] == 'A') {
			printf("server is ready to show\n");
		
			if ((fork_error = fork()) == -1) {
				printf("%s\n", strerror(errno));
			}

			if (fork_error == 0) {
				int dup_error = dup2(connectfd, 0);
				if (dup_error == -1) {
					printf("dup error\n");
					exit(0);
				}
				else {
					execlp("more", "more", "-20", (char*) NULL);
					printf("%s\n", strerror(errno));
				}
			}

			else {
				wait(&status);
			}
		}

		close(connectfd);
	}
}

void get(int socketfd, char *second_token, const char *argv2) {
	//printf("ready to get\n");
	char temp_buf[100];
	char temp_buf2[100];
	char G_command[100];
	char c;
	int temp;
	int temp2;
	int index = 0;
	char *temp_path;
	strcpy(G_command, "G");
	strcat(G_command, second_token);
	strcat(G_command, "\n");
	write(socketfd, "D\n", 2);
	read(socketfd, temp_buf, 100);
	if (temp_buf[0] == 'E') {
		printf("was not albe to buld data connection\n");
	}
	else {
		char *s = (temp_buf + 1);
		strtok(s, "\n");

		int connectfd = data_connect(s, argv2);
	
		write(socketfd, G_command, strlen(G_command));
		while((temp2 = read(socketfd, &c, 1))) {
			temp_buf2[index] = c;
			index++;
			if (c == '\n') {
				break;
			}
		}

		temp_buf2[index+1] = '\0';
		char *localpath = strtok(second_token, "/");
		while (localpath != NULL) {
			temp_path = localpath;
			localpath = strtok(NULL, "/");
		}
		int exist = access(temp_path, F_OK);
		if (exist == 0) {
			printf("file already exist\n");
			return;
		}

		if (temp_buf2[0] == 'E') {
			char *x = (temp_buf2 + 1);
			fflush(stdout);
			printf("Error, error message from the server: %s", x);
		}

		else if (temp_buf2[0] == 'A') {
			printf("server is ready to transfer data as in response to get\n");
			int fd = open(temp_path, O_RDWR | O_CREAT, S_IRWXU);
			if (fd < 0) {
				printf("%s\n", strerror(errno));
			}
			else {
				while((temp = read(connectfd, &c, 1)) > 0) {
					write(fd, &c, 1);
				}
			}
			close(fd);
		}	
		close(connectfd);
	}
}


void quit(int socketfd) {
	printf("ready to quit\n");
	char temp_buf1[1000];
	char c;
	int temp;
	char *s = (temp_buf1 + 1);
	strtok(s, "\n");
	
	write(socketfd, "Q\n", 2);
	while((temp = read(socketfd, &c, 1))) {
	//	printf("%c", c);
	}

	close(socketfd);
	exit(0);
}


void put(int socketfd, char *second_token, const char *argv2) {
	char temp_buf[100];
	char temp_buf2[100];
	char P_command[100];
	char c;
	int temp, temp2;
	int index = 0;
	char *localpath;
	char *origin = malloc(sizeof(char) * 254);
	strcpy(origin, second_token);
	struct stat area, *file_area = &area;
	strcpy(P_command, "P");
	int regular = lstat(second_token, file_area);
	int permission = access(second_token, R_OK | F_OK);
	if (permission != 0) {
		printf("Error, file doesn't exist or no permission\n");
	}
	else if (S_ISREG(file_area->st_mode) != true) {
		printf("Error, file is not a regular file\n");
	}

	else {
		write(socketfd, "D\n", 2);
		read(socketfd, temp_buf, 100);
		if (temp_buf[0] == 'E') {
			printf("was not able to build data connection\n");
		}
		else {
			char *s = (temp_buf + 1);
			strtok(s, "\n");

			int connectfd = data_connect(s, argv2);
			char *temp_path = strtok(second_token, "/");
			while (temp_path != NULL) {
				localpath = temp_path;
				temp_path = strtok(NULL, "/");
			}
			strcat(P_command, localpath);
			strcat(P_command, "\n");			
			write(socketfd, P_command, strlen(P_command));
			while((temp2 = read(socketfd, &c, 1))) {
				temp_buf2[index] = c;
				index++;
				if (c == '\n') {
					break;
				}
			}

	      		if (temp_buf2[0] == 'E') {
				char *x = (temp_buf2 + 1);
				printf("Error, error message from the server: %s", x);
			}

			else if (temp_buf2[0] == 'A') {
				printf("server is ready to receive data as in response to put\n");
				int fd = open(origin, O_RDWR, 0700);
				if (fd < 0) {
					printf("%s\n", strerror(errno));
				}
				else {
					while((temp = read(fd, &c, 1)) > 0) {
						write(connectfd, &c, 1);
					}
				}
				close(fd);
			}
		
			close(connectfd);
		}
	}
	free(origin);

}



int data_connect(const char *port_num, const char *host_name) {
	struct addrinfo hints, *actualdata;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;

	int error = getaddrinfo(host_name, port_num, &hints, &actualdata);
	if(error != 0) {
		fprintf(stderr, "in data connection getaddr Error: %s\n", gai_strerror(errno));
		exit(-5);
	}
	int connectfd = socket(actualdata -> ai_family, actualdata -> ai_socktype, 0);
	if(connectfd == -1) {
		fprintf(stderr, "in data connection socket Error: %s\n", strerror(errno));
		exit(-1);
	}
	if(connect(connectfd, actualdata->ai_addr, actualdata->ai_addrlen) < 0 ) {
		fprintf(stderr, "in data connection connect error: %s\n", strerror(errno));
		exit(-1);
	}
	freeaddrinfo(actualdata);

	return connectfd;
}




int main(int argc, char const *argv[]) {
	int socketfd;
	const char *argv1 = argv[1];
	const char *argv2 = argv[2];
	
	char rcd_buf[MAX_INPUT];
	char input_buf[MAX_INPUT];
	char dict_buf[MAX_INPUT];
	const char delimiter[2] = " ";
	char *first_token, *second_token;
	char *remote_command;
	struct addrinfo hints, *actualdata;
	memset(&hints, 0, sizeof(hints));
	int err;
	char com_buf[100];
	
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;
	char buf[100] = "hello";

	err = getaddrinfo(argv[2], argv[1], &hints, &actualdata); 
	if(err != 0) {
		fprintf(stderr, "Error: %s\n", gai_strerror(err));
		exit(-5);
	}
	socketfd = socket(actualdata -> ai_family, actualdata -> ai_socktype, 0);
	if(socketfd == -1) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		exit(-8);
	}
	if(connect(socketfd, actualdata->ai_addr, actualdata->ai_addrlen) < 0 ){ 
		fprintf(stderr, "Error: %s\n", strerror(errno));
		exit(-7);
	}
	freeaddrinfo(actualdata);

	printf("connected to server %s\n", argv[2]);
	while(1) {
		write(0, "enter input:", 12);
		char input_buf[MAX_INPUT];
		int command_line_temp;
		int command_line_index = 0;
		char input_c;
		while((command_line_temp = read(0, &input_c, 1))) {
			input_buf[command_line_index] = input_c;
			command_line_index++;
			if (input_c == '\n') {
				break;
			}
		}
		input_buf[command_line_index++] = '\0';
		strncpy(rcd_buf, input_buf, (sizeof(input_buf)));
		remote_command = strtok(rcd_buf, "\n");
		first_token = strtok(input_buf, delimiter);
		second_token = strtok(NULL, delimiter);
		second_token = strtok(second_token, "\n");
		printf("\n");
		if(strcmp(first_token, "cd") == 0) {
			//printf("ready to cd\n");
			change_dir(second_token);
		}

		else if(strcmp(first_token, "ls\n") == 0) {
			//printf("ready to ls\n");
			list();
		}

		else if(strcmp(first_token, "rcd") == 0) {
			rcd(socketfd, second_token);
		}


		else if(strcmp(first_token, "rls\n") == 0) {
			rls(socketfd, argv2);

		}

		else if(strcmp(first_token, "show") == 0) {
			show(socketfd, second_token, argv2);

		}

		else if(strcmp(first_token, "get") == 0) {
			get(socketfd, second_token, argv2);


		}

		else if(strcmp(first_token, "put") == 0) {
			put(socketfd, second_token, argv2);
		}


		
		else if(strcmp(first_token, "exit\n") == 0) {
			quit(socketfd);

		}

		else{
			printf("Invalid command\n");
		}

		
	}
	
	return 0;
}
