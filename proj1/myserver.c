#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024
#define BACKLOG 10

void requestHandle(int client_socket, char* request) {
    char method[10];    //요청 메소드
	char path[20];      //파일 경로
	char* filename;     //파일명
	char* extention = NULL; //파일 확장자
    FILE* file;     //파일에 대한 포인터
    long fsize;     //파일의 크기
    int option;     //현재 어떤 파일을 열었는지 표시
    char buffer[BUFFER_SIZE];   //파일 전송을 위한 버퍼
    char response[BUFFER_SIZE]; //응답
    size_t bytesRead;   //파일 전송 추적을 위한 변수

    sscanf(request, "%s %s", method, path); //start-line에서 필요한 정보를 읽어온다.
    
    /*
     * 요청 path의 맨 앞 '/' 제거
     */
	filename = strdup(path);
	filename += strspn(filename, "/");

    /*
     * 요청받은 파일을 연다.
     * 파일이 있는 경우 크기를 계산하고 extention이 파일 확장자를 가리키게 한다.
     */
    file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Opening file failed");
    }
    else {
        fseek(file, 0, SEEK_END);
        fsize = ftell(file);
        fseek(file, 0, SEEK_SET);

		extention = strchr(filename, '.') + 1;
    }

	
    /*
     * 파일의 확장자에 따라 option 설정.
     * 없는 파일이거나(extention = NULL) html, gif, jpg, mp3, pdf 이외의 확장자면 0으로 설정
     */
	if (extention == NULL) {
		option = 0;
	}
	else if (!strcmp(extention, "html")) {
		option = 1;
	}
	else if (!strcmp(extention, "gif")) {
		option = 2;
	}
    else if (!strcmp(extention, "jpeg") || !strcmp(extention, "jpg")) {
        option = 3;
    }
    else if (!strcmp(extention, "mp3")) {
        option = 4;
    }
    else if (!strcmp(extention, "pdf")) {
        option = 5;
    }
    else {
        option = 0;
    }

    /* 
     * option 값에 따라 Content-Type을 다르게 응답 헤더 작성
     * 0인 경우 404 Not Found
     */
	switch(option) {
		case 0:
			sprintf(response, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n");
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
    /* 
     * 응답 헤더와 파일 전송
     */
	send(client_socket, response, strlen(response), 0);
    if (option > 0) {
	    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
		    send(client_socket, buffer, bytesRead, 0);
	    }
		fclose(file);
    }

}

int main(int argc, char* argv[]) {
    int server_port = atoi(argv[1]);    //첫번째 인자를 서버가 통신할 포트로 설정

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);    //IPv4, TSP 소켓 생성
    if (server_socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server_addr, client_addr;    //소켓의 정보를 담을 구조체
    server_addr.sin_family = AF_INET;       //IPv4
    server_addr.sin_port = htons(server_port);  //초기에 설정한 포트
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);    //가능한 모든 IP
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {   //소켓에 포트, IP주소를 바인딩
        perror("Binding failed");
        return 1;
    }

    if (listen(server_socket, BACKLOG) == -1) {     //소켓에 백로그 생성, BACKLOG=10
        perror("Listening failed");
        return 1;
    }

    printf("Server listening on port %d\n", server_port);

    while (1) {
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);    //백로그에서 요청을 꺼내와 통신을 위한 client_socket 생성
        if (client_socket == -1) {
            perror("Accepting client failed");
            continue;
        }

        printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	
        char request[1024]; //요청 메시지를 담을 버퍼
        recv(client_socket, request, sizeof(request), 0);   //요청 메시지를 읽어옴
        printf("Received request:\n%s\n", request);

        requestHandle(client_socket, request);  //응답을 보내기 위해 client_socket과 요청 메시지를 인자로 requestHandle 호출

        close(client_socket);
    }

    close(server_socket);
    return 0;
}

