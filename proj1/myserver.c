#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024
#define BACKLOG 10

void requestHandle(int client_socket, char* request) {
    char method[10], path[20];
    FILE* file;
    long fsize;
    int option;
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    size_t bytesRead;

    sscanf(request, "%s %s", method, path);

	if (!strcmp(path, "/")) {
	    file = fopen("index.html", "rb");
		option = 0;
	}
	else if (!strcmp(path, "/1")) {
		file = fopen("1.html", "rb");
		option = 1;
	}
    else if (!strcmp(path, "/2")) {
        file = fopen("2.gif", "rb");
        option = 2;
    }
    else if (!strcmp(path, "/3")) {
        file = fopen("3.jpeg", "rb");
        option = 3;
    }
    else if (!strcmp(path, "/4")) {
        file = fopen("4.mp3", "rb");
        option = 4;
    }
    else if (!strcmp(path, "/5")) {
        file = fopen("5.pdf", "rb");
        option = 5;
    }
    else {
        option = -1;
    }
	
	if (option >= 0) {
		if (file == NULL) {
			perror("Opening file failed");
			exit(EXIT_FAILURE);
		}
    
    	fseek(file, 0, SEEK_END);
    	fsize = ftell(file);
    	fseek(file, 0, SEEK_SET);
	}

	switch(option) {
        case -1:
            sprintf(response, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n");
            break;
		case 0:
			sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", fsize);
			break;
		case 1:
			sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", fsize);
			break;
        case 2:
            sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: image/gif\r\nContent-Length: %ld\r\n\r\n", fsize);
            break;
        case 3:
            sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: %ld\r\n\r\n", fsize);
            break;
        case 4:
            sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: audio/mpeg\r\nContent-Length: %ld\r\n\r\n", fsize);
            break;
        case 5:
            sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: application/pdf\r\nContent-Length: %ld\r\n\r\n", fsize);
            break;
	}

	send(client_socket, response, strlen(response), 0);
    if (option >= 0) {
	    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
		    send(client_socket, buffer, bytesRead, 0);
	    }
		fclose(file);
    }

}

int main(int argc, char* argv[]) {
    int server_port = atoi(argv[1]);

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);  
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Binding failed");
        return 1;
    }

    if (listen(server_socket, BACKLOG) == -1) {
        perror("Listening failed");
        return 1;
    }

    printf("Server listening on port %d\n", server_port);

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

        requestHandle(client_socket, request);

        close(client_socket);
    }

    close(server_socket);
    return 0;
}

