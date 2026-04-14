#include <sys/socket.h>   // socket, bind, listen, accept, connect
#include <sys/time.h>     // gettimeofday
#include <netinet/in.h>   // sockaddr_in 구조체
#include <arpa/inet.h>    // inet_addr, htons 같은 주소 변환
#include <unistd.h>       // close, read, write, usleep
#include <string.h>       // memset, strlen
#include <stdio.h>  

int main(void){
    // 1. socket() - 소켓 생성
    // TCP랑 다르게 SOCK_DGRAM(데이터그램), IPPROTO_UDP 사용
    // UDP는 연결 없이 독립적인 덩어리 단위로 데이터를 주고받음
    int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        printf("socket 생성 실패\n");
        return -1;
    }

    // 서버 자신의 주소 구조체 설정
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));    // 쓰레기값 제거
    server_addr.sin_family = AF_INET;                // IPv4 방식으로 통신
    server_addr.sin_port = htons(53140);             // 클라이언트와 반드시 같은 포트번호
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 어떤 IP에서 오는 연결이든 다 받겠다는 뜻

    // 2. bind() - 소켓에 주소(IP+포트) 등록
    int bind_result = bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (bind_result < 0) {
        printf("bind 실패\n");
        return -2;
    }

    // 처음엔 타임아웃 없이 기다림
    // 첫 데이터 받기 전까지는 무한정 대기
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    printf("서버 대기 중...\n");

    // 3. recvfrom() - 데이터 수신 후 삭제
    int total_bytes = 0;
    struct timeval start, end;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    while (1) {
        char buffer[2000];
        memset(buffer, 0, sizeof(buffer));

        int recv_result = recvfrom(sock, buffer, sizeof(buffer), 0,
                                   (struct sockaddr*)&client_addr, &client_addr_size);

        // 첫 데이터 받은 시점부터 타임아웃 3초 설정 + 시간 측정 시작
        if (total_bytes == 0 && recv_result > 0) {
            gettimeofday(&start, NULL);  // 첫 데이터 받은 시점부터 측정
            timeout.tv_sec = 3;          // 이후부터 3초 타임아웃 적용
            timeout.tv_usec = 0;
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        }

        // 타임아웃 or 에러 → 종료
        if (recv_result <= 0) {
            printf("타임아웃 or 에러 → 수신 종료\n");
            break;
        }

        // END 신호 받으면 종료
        if (strcmp(buffer, "END") == 0) {
            printf("종료 신호 수신\n");
            break;
        }

        total_bytes += recv_result;  // 수신 바이트 누적
    }

    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("수신 완료\n");
    printf("경과 시간: %.2f초\n", elapsed);
    printf("총 수신 바이트: %d\n", total_bytes);
    printf("throughput: %.2f bytes/s\n", total_bytes / elapsed);

    // 4. close() - 소켓 닫기
    close(sock);
    return 0;
}