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

    // 서버 주소 구조체 채우기
    // TCP 클라이언트랑 똑같이 어디로 보낼지 주소 정보를 채움
    // TCP는 이걸 connect()에 넘겼지만 UDP는 sendto()에 직접 넘김
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));  // 쓰레기값 제거
    server_addr.sin_family = AF_INET;              // IPv4 방식으로 통신
    server_addr.sin_port = htons(53140);           // 서버 포트번호
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 서버 IP

    // 2. sendto() - 속도 조절하면서 데이터 전송
    // TCP와 달리 보낼 때마다 주소를 같이 넘겨야 함
    int speed;
    printf("전송 속도 입력 (500/1000/2000): ");
    scanf("%d", &speed);  // 실행할 때마다 speed 입력받음

    char buf[2000];
    memset(buf, 'A', sizeof(buf));  // buf를 'A'로 채움 (임의의 데이터)
    int total_bytes = 0;

    struct timeval start, now;
    gettimeofday(&start, NULL);  // 측정 시작

    for (int i = 0; i < 10; i++) {  // 10초 동안 전송
        int send_result = sendto(sock, buf, speed, 0,
                                (struct sockaddr*)&server_addr, sizeof(server_addr));
        if (send_result < 0) {
            printf("sendto 실패\n");
            return -2;
        }
        total_bytes += speed;  // 전송한 바이트 누적

        // 목표 시간까지 정확하게 대기
        // sleep(1) 대신 usleep으로 오차 방지
        gettimeofday(&now, NULL);
        double elapsed = (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1e6;
        double target = (i + 1) * 1.0;  // 목표 시간 (1초, 2초, 3초...)
        if (target > elapsed) {
            usleep((target - elapsed) * 1000000);  // 남은 시간만큼 대기
        }
    }

    // 마지막에 종료 신호 전송
    // TCP는 close()로 서버가 알 수 있지만 UDP는 연결이 없어서 직접 알려줘야 함
    sendto(sock, "END", 3, 0,
           (struct sockaddr*)&server_addr, sizeof(server_addr));

    gettimeofday(&now, NULL);  // 전송 종료 시간 기록
    double elapsed = (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1e6;
    printf("총 전송 바이트: %d\n", total_bytes);
    printf("경과 시간: %.2f초\n", elapsed);
    printf("throughput: %.2f bytes/s\n", total_bytes / elapsed);

    // 3. close() - 소켓 닫기
    close(sock);
    return 0;
}