#include <sys/socket.h>   // socket, bind, listen, accept, connect
#include <netinet/in.h>   // sockaddr_in 구조체
#include <arpa/inet.h>    // inet_addr, htons 같은 주소 변환
#include <unistd.h>       // close, read, write
#include <string.h>       // memset, strlen
#include <stdio.h>        
int main() {

    // 1. socket() - 소켓 생성
    // IPv4, TCP 방식의 소켓을 만들고 sock에 소켓 번호(파일 디스크립터)를 저장
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        printf("socket 생성 실패\n");
        return -1;
    }

    // 2. connect() 전 준비 - 서버 주소 구조체 설정
    // connect()를 호출하기 전에 어디로 연결할지 주소 정보를 미리 채워놓는 단계
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));  // 쓰레기값 제거
    server_addr.sin_family = AF_INET;              // IPv4 방식으로 통신
    server_addr.sin_port = htons(53140);           // 포트번호
    // 네트워크는 빅 엔디안을 사용하고, 맥/윈도우는 리틀 엔디안을 사용해서 htons로 변환해줌
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 서버 IP

    // 3. connect() - 서버에 연결 요청
    // 위에서 채운 주소로 서버에 연결 시도
    // result에 결과값 저장 → 성공하면 0, 실패하면 -1 반환
    int result = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (result < 0) 
    {
        printf("connect failed\n");
        return -2;
    }

    // 4. send() - 데이터 전송
    char buffer[] = "let's go";
    int send_result = send(sock, buffer, strlen(buffer), 0);
    if (send_result < 0)
    {
        printf("send 실패\n");
        return -3;
    }
    // 서버가 돌려준 데이터 받기
    char recv_buffer[1024];
    memset(recv_buffer, 0, sizeof(recv_buffer));
    int recv_result = recv(sock, recv_buffer, sizeof(recv_buffer), 0);
    if (recv_result < 0) {
    printf("recv 실패\n");
    return -4;
    }
    printf("서버로부터 받은 데이터: %s\n", recv_buffer);
    
    // 5. close()
    close(sock);

    return 0;
}