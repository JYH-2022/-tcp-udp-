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
    int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        printf("socket 생성 실패\n");
        return -1;
    }

    // 서버 주소 구조체 채우기
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));  // 쓰레기값 제거
    server_addr.sin_family = AF_INET;              // IPv4 방식으로 통신
    server_addr.sin_port = htons(53140);           // 서버 포트번호
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 서버 IP

    // 2. sendto() - 속도 조절하면서 데이터 전송
    int speed;
    printf("전송 속도 입력 (500/1000/2000): ");
    scanf("%d", &speed); 

    char buf[2000];
    memset(buf, 'A', sizeof(buf));  // buf를 'A'로 채움 (임의의 데이터)
    int total_bytes = 0;  // 총 전송 바이트 누적용

    struct timeval start, now;
    gettimeofday(&start, NULL); 

    for (int i = 0; i < 100; i++) { 
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
        double pasttime = (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1e6;  // 현재까지 경과 시간 (초 단위)
        double targettime = (i + 1) * 1.0;  // 목표 시간 (1초, 2초, 3초...)
        if (targettime > pasttime) {
            usleep((targettime - pasttime) * 1000000); 
        }
    }

    // 마지막에 종료 신호 전송
    // TCP는 close()로 서버가 알 수 있지만 UDP는 연결이 없어서 직접 알려줘야 함

    // gettimeofday를 END 전송 전에 찍어야 실제 데이터 전송 시간만 측정
    gettimeofday(&now, NULL);  // 전송 종료 시간 기록
    double totaltime = (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1e6;  // 총 경과 시간 계산

    sendto(sock, "END", 3, 0,
           (struct sockaddr*)&server_addr, sizeof(server_addr));  // 서버에 종료 신호 전송

    printf("총 전송 바이트: %d\n", total_bytes);
    printf("경과 시간: %.4f초\n", totaltime);
    printf("throughput: %.4f bytes/s\n", total_bytes / totaltime); 

    // 3. close() - 소켓 닫기
    close(sock);
    return 0;
}