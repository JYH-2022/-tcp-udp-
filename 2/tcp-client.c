#include <sys/socket.h>   // socket, bind, listen, accept, connect
#include <sys/time.h>     // gettimeofday
#include <netinet/in.h>   // sockaddr_in 구조체
#include <arpa/inet.h>    // inet_addr, htons 같은 주소 변환
#include <unistd.h>       // close, read, write, usleep
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
    // result에 결과값 저장 → 성공하면 0, 실패하면 -1 반환
    int result = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (result < 0) {
        printf("connect failed\n");
        return -2;
    }

    // 4. send() - 속도 조절하면서 데이터 전송
    int speed;
    printf("전송 속도 입력 (500/1000/2000): ");
    scanf("%d", &speed);

    char buf[2000];
    memset(buf, 'A', sizeof(buf));  // buf를 'A'로 채움 (임의의 데이터)
    int total_bytes = 0;

    struct timeval start, now;
    gettimeofday(&start, NULL);  // 전송 시작 시간 기록

    for (int i = 0; i < 10; i++) {  // 10초 동안 전송
        int send_result = send(sock, buf, speed, 0);
        if (send_result < 0) {
            printf("send 실패\n");
            return -3;
        }
        total_bytes += speed;

        // 목표 시간까지 정확하게 대기
        // sleep(1) 대신 usleep으로 오차 방지
        gettimeofday(&now, NULL);
        double elapsed = (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1e6;
        double target = (i + 1) * 1.0;  // 목표 시간 (1초, 2초, 3초...)
        if (target > elapsed) {
            usleep((target - elapsed) * 1000000);  // 남은 시간만큼 대기
        }
    }

    // TCP는 close()하면 서버가 자동으로 연결 종료를 알 수 있어서 END 신호 필요없음
    // close() 하면 서버의 recv()가 0을 반환해서 서버도 종료됨
    gettimeofday(&now, NULL);  // 전송 종료 시간 기록
    double elapsed = (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1e6;
    printf("총 전송 바이트: %d\n", total_bytes);
    printf("경과 시간: %.2f초\n", elapsed);
    printf("throughput: %.2f bytes/s\n", total_bytes / elapsed);

    // 5. close() - 소켓 닫기
    close(sock);
    return 0;
}