#include <sys/socket.h>   // socket, bind, listen, accept, connect
#include <sys/time.h>     // gettimeofday
#include <netinet/in.h>   // sockaddr_in 구조체
#include <arpa/inet.h>    // inet_addr, htons 같은 주소 변환
#include <unistd.h>       // close, read, write, usleep
#include <string.h>       // memset, strlen
#include <stdio.h>  

int main(void){
    // 1. socket() - 소켓 생성
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

    printf("서버 대기 중...\n");

    // 3. recvfrom() - 데이터 수신 후 삭제
    int total_bytes = 0;
    struct timeval start, end, timeout;  // timeout은 첫 데이터 수신 후 설정용
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);  // recvfrom()에 넘길 구조체 크기
    char buffer[2000];  // 루프 밖에서 선언 (매 반복마다 재선언 불필요)

    while (1) {
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

        if (recv_result <= 0) {
            printf("타임아웃 or 에러 -> 수신 종료\n");
            break;
        }

        // END 신호 받으면 종료
        if (strcmp(buffer, "END") == 0) {
            printf("종료 신호 수신\n");
            break;
        }

        total_bytes += recv_result;  // 수신 바이트 누적
    }

    gettimeofday(&end, NULL);  // 수신 종료 시간 기록
    double totaltime = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;  // 총 경과 시간 계산
    printf("수신 완료\n");
    printf("총 수신 바이트: %d\n", total_bytes);
    printf("경과 시간: %.4f초\n", totaltime);
    printf("throughput: %.4f bytes/s\n", total_bytes / totaltime);  // 총 수신 바이트 ÷ 경과 시간

    // 4. close() - 소켓 닫기
    close(sock);

    return 0;
}