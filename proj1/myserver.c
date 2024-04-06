#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


#define BUFFER_SIZE 1024


char* getFileName(int i) {
	char* fileName;
	switch(i) {
		case 1:
			fileName = "1.jpg";
			break;
		default:
			fileName = "";
			break;
	}
	return fileName;
}


void sendFile(int client_socket, char* filename) {
	char buffer[BUFFER_SIZE];
	size_t bytesRead;
	
	FILE *file = fopen(filename, "rb");
	if (file == NULL) {
		perror("Opening file failed");
		exit(EXIT_FAILURE);
	}

	while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
		send(client_socket, buffer, bytesRead, 0);
	}
	fclose(file);
}


void sendResponse(int client_socket, int i) {
	char* response;
	char* fileName;
	long fsize;
	char buffer[BUFFER_SIZE];
	size_t bytesRead;
	printf("111");
	fileName = getFileName(i);

	FILE *file = fopen(fileName, "rb");
	if (file == NULL) {
		perror("Opening file failed");
		exit(EXIT_FAILURE);
	}

	printf("1");

	fseek(file, 0, SEEK_END);
	fsize = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	printf("2");

	sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: %ld\r\n\r\n", fsize);
	
	printf("3");

	send(client_socket, response, sizeof(response), 0);

	printf("4");

	while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
		send(client_socket, buffer, bytesRead, 0);
	}

	fclose(file);
}


int main(int argc, char* argv[]) {
    const char* server_ip = "192.168.56.106";
    int server_port = atoi(argv[1]);
	char* fileName;

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
//    server_addr.sin_addr.s_addr = inet_addr(server_ip);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Binding failed");
        return 1;
    }

    if (listen(server_socket, 5) == -1) {
        perror("Listening failed");
        return 1;
    }

    printf("Server listening on %s:%d\n", server_ip, server_port);

    while (1) {
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Accepting client failed");
            continue;
        }

        printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	
        char request[1024];
        recv(client_socket, request, sizeof(request), 0);
        printf("Received request:\n%s\n", request);
		
//        char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nHello, World!";
		
		char response[] = "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\n\r\n";
        send(client_socket, response, sizeof(response), 0);

		fileName = getFileName(1);
		sendFile(client_socket, fileName);
//		sendResponse(client_socket, 1);

        close(client_socket);
    }

    close(server_socket);
    return 0;
}

