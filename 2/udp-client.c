#include <sys/socket.h>   // socket, bind, listen, accept, connect
#include <sys/time.h>     // gettimeofday
#include <netinet/in.h>   // sockaddr_in 구조체
#include <arpa/inet.h>    // inet_addr, htons 같은 주소 변환
#include <unistd.h>       // close, read, write
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
    // TCP 클라이언트랑 똑같이 어디로 보낼지 주소 정보를 채움
    // TCP는 이걸 connect()에 넘겼지만 UDP는 sendto()에 직접 넘김
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));  // 쓰레기값 제거
    server_addr.sin_family = AF_INET;              // IPv4 방식으로 통신
    server_addr.sin_port = htons(53140);           // 서버 포트번호
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 서버 IP

    // 2. sendto() - 속도 조절하면서 데이터 전송
    // TCP와 달리 보낼 때마다 주소를 같이 넘겨야 함
    // connect()로 연결을 맺지 않아서 매번 주소를 알려줘야 하기 때문
    int speed = 2000;  // 500, 1000, 2000 으로 바꿔가며 테스트
    char buf[2000];
    memset(buf, 'A', sizeof(buf));
    int total_bytes = 0;

    struct timeval start, end;
    gettimeofday(&start, NULL);  // 전송 시작 시간

    for (int i = 0; i < 10; i++) {  // 10초 동안 전송
        int send_result = sendto(sock, buf, speed, 0,
                                (struct sockaddr*)&server_addr, sizeof(server_addr));
        if (send_result < 0) {
            printf("sendto 실패\n");
            return -2;
        }
        total_bytes += speed;
        sleep(1);  // 1초 대기 → 초당 speed bytes 전송
    }

    gettimeofday(&end, NULL);  // 전송 종료 시간
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("총 전송 바이트: %d\n", total_bytes);
    printf("경과 시간: %.2f초\n", elapsed);
    printf("throughput: %.2f bytes/s\n", total_bytes / elapsed);

    // 3. close() - 소켓 닫기
    // UDP는 client_sock이 없어서 sock 하나만 닫으면 됨
    close(sock);
    return 0;
}