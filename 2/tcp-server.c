#include <sys/socket.h>   // socket, bind, listen, accept, connect
#include <sys/time.h>     // gettimeofday
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
    server_addr.sin_port = htons(53140);           // 포트번호
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 어떤 IP에서 오는 연결이든 다 받겠다는 뜻

    // 2. bind() - 소켓에 주소(IP+포트) 등록
    int bind_result = bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (bind_result < 0) {
        printf("bind 실패\n");
        return -2;
    }

    // 3. listen() - 연결 대기 상태로 전환
    int listen_result = listen(sock, 1);
    if (listen_result < 0) {
        printf("listen 실패\n");
        return -3;
    }
    printf("서버 대기 중...\n");

    // 4. accept() - 클라이언트 연결 수락
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);  // accept()에 넘길 구조체 크기

    int client_sock = accept(sock, (struct sockaddr*)&client_addr, &client_addr_size);  // 연결 수락, 통신용 소켓 반환
    if (client_sock < 0) {
        printf("accept 실패\n");
        return -4;
    }

    // 5. recv() - 데이터 수신 후 삭제
    // 첫 데이터 받은 시점부터 측정 시작
    int total_bytes = 0;
    struct timeval start, end;
    char buffer[2000];  // 루프 밖에서 선언 (매 반복마다 재선언 불필요)

    while (1) {
        memset(buffer, 0, sizeof(buffer));  // 버퍼 쓰레기값 제거

        int recv_result = recv(client_sock, buffer, sizeof(buffer), 0);

        // 클라이언트가 close()하면 recv()가 0 반환 ->탈출
        if (recv_result <= 0) break;

        // 첫 데이터 받은 시점부터 측정 시작
        if (total_bytes == 0) gettimeofday(&start, NULL);

        total_bytes += recv_result;  // 수신 바이트 누적
    }

    gettimeofday(&end, NULL);  // 수신 종료 시간 기록
    double totaltime = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;  // 총 경과 시간 계산
    printf("수신 완료\n");
    printf("총 수신 바이트: %d\n", total_bytes);
    printf("경과 시간: %.4f초\n", totaltime);
    printf("throughput: %.4f bytes/s\n", total_bytes / totaltime);  // 총 수신 바이트 ÷ 경과 시간

    // 6. close() - 소켓 닫기
    close(client_sock);
    close(sock);

    return 0;
}