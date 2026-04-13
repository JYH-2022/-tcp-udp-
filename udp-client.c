#include <sys/socket.h>   // socket, bind, listen, accept, connect
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
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(53140);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 2. sendto() - 데이터 전송
    // TCP와 달리 보낼 때마다 주소를 같이 넘겨야 함
    char buffer[] = "let's go";
    int send_result = sendto(sock, buffer, strlen(buffer), 0, 
                        (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (send_result < 0) {
        printf("sendto 실패\n");
        return -2;
    }

    // 3. recvfrom() - 데이터 수신
    // 서버가 에코로 돌려준 데이터 받기
    char recv_buffer[1024];
    memset(recv_buffer, 0, sizeof(recv_buffer));
    socklen_t addr_size = sizeof(server_addr);
    int recv_result = recvfrom(sock, recv_buffer, sizeof(recv_buffer), 0,
                           (struct sockaddr*)&server_addr, &addr_size);
    if (recv_result < 0) {
        printf("recvfrom 실패\n");
        return -3;
    }
    printf("서버로부터 받은 데이터: %s\n", recv_buffer);

    // 4. close()
    close(sock);

    return 0;
}