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

    // 서버 자신의 주소 구조체 설정
    // 클라이언트는 서버 주소를 채웠지만, 서버는 자기 자신의 주소를 채운다.
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));   // 쓰레기값 제거
    server_addr.sin_family = AF_INET;               // IPv4 방식으로 통신
    server_addr.sin_port = htons(53140);            // 클라이언트와 반드시 같은 포트번호
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 어떤 IP에서 오는 연결이든 다 받겠다는 뜻

    // 2. bind() - 소켓에 주소(IP+포트) 등록
    // "나는 이 포트번호로 데이터를 받을게" 라고 OS에 등록하는 것
    int bind_result = bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (bind_result < 0) {
        printf("bind 실패\n");
        return -2;
    }
    printf("서버 대기 중...\n");

    // 3. recvfrom() - 데이터 수신 후 삭제
    // UDP는 연결 개념이 없어서 클라이언트 종료를 모름
    // 그래서 while(1) 대신 클라이언트가 보내는 횟수(10번)만큼만 받음
    int total_bytes = 0;
    struct timeval start, end;
    struct sockaddr_in client_addr;        // 보낸 클라이언트 주소 저장용
    socklen_t client_addr_size = sizeof(client_addr);

    for (int i = 0; i < 10; i++) 
    {
        char buffer[2000];
        memset(buffer, 0, sizeof(buffer));
        int recv_result = recvfrom(sock, buffer, sizeof(buffer), 0,
                                (struct sockaddr*)&client_addr, &client_addr_size);
        if (i == 0) gettimeofday(&start, NULL); // 첫 번째 데이터 받은 시점부터 측정
        if (recv_result <= 0) break;
        total_bytes += recv_result;
    }

    gettimeofday(&end, NULL);  // 수신 종료 시간
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("수신 완료\n");
    printf("경과 시간: %.2f초\n", elapsed);
    printf("총 수신 바이트: %d\n", total_bytes);
    printf("throughput: %.2f bytes/s\n", total_bytes / elapsed);

    // 4. close() - 소켓 닫기
    close(sock);
    return 0;
}