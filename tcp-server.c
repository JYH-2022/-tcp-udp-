#include <sys/socket.h>   // socket, bind, listen, accept, connect
#include <netinet/in.h>   // sockaddr_in 구조체
#include <arpa/inet.h>    // inet_addr, htons 같은 주소 변환
#include <unistd.h>       // close, read, write
#include <string.h>       // memset, strlen
#include <stdio.h>     

int main(void){
    // 1. socket() - 소켓 생성
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        printf("socket 생성 실패\n");
        return -1;
    }

    // bind() 전 준비 - 서버 자신의 주소 구조체 설정
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));  // 쓰레기값 제거
    server_addr.sin_family = AF_INET;              // IPv4 방식으로 통신
    server_addr.sin_port = htons(53140);           
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 어떤 IP에서 오는 연결이든 다 받겠다는 뜻

    // 2. bind() - 소켓에 주소(IP+포트) 등록
    int bind_result = bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (bind_result < 0) {
        printf("bind 실패\n");
        return -2;
    }

    // 3. listen() - 연결 대기 상태로 전환
    // bind() 까지 하면 포트 등록만 된 거고, listen() 을 해야 실제로 연결 받을 준비 완료가 됨.
    int listen_result = listen(sock, 1);
    if (listen_result < 0) {
        printf("listen 실패\n");
        return -3;
    }
    printf("서버 대기 중...\n");  

    // 4. accept() - 클라이언트 연결 수락
    // 연결되면 실제 통신용 새 소켓(client_sock)을 반환
    struct sockaddr_in client_addr;       // 연결된 클라이언트 주소 저장용
    socklen_t client_addr_size = sizeof(client_addr);  // 구조체 크기

    int client_sock = accept(sock, (struct sockaddr*)&client_addr, &client_addr_size);
    if (client_sock < 0) {
        printf("accept 실패\n");
        return -4;
    }

    // 5. recv() - 데이터 수신
    // 이제 sock이 아니라 client_sock으로 통신해야 함
    char buffer[100];  // 받은 데이터 저장할 공간
    memset(buffer, 0, sizeof(buffer));  // 버퍼 초기화

    int recv_result = recv(client_sock, buffer, sizeof(buffer), 0);
    if (recv_result < 0) {
        printf("recv 실패\n");
        return -5;
    }
    printf("받은 데이터: %s\n", buffer);  // 받은 데이터 출력

    // 받은 데이터 그대로 돌려주기 (에코)
    int send_result = send(client_sock, buffer, recv_result, 0);
    if (send_result < 0) {
        printf("send 실패\n");
        return -6;
    }
    printf("에코 완료\n");

    // 6. close() - 소켓 닫기
    close(client_sock);
    close(sock);





    return 0;
}