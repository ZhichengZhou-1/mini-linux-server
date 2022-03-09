HEADER = mftp.h

all: server_executable client_executable

server_executable: mftpserve.c $(HEADER)
	gcc -o server_executable mftpserve.c

client_executable: mftp.c $(HEADER)
	gcc -o client_executable mftp.c


