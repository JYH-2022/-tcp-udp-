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

    // 서버 자신의 주소 구조체 설정
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));  
    server_addr.sin_family = AF_INET;              
    server_addr.sin_port = htons(53140);           
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 

    // 2. bind() - 소켓에 주소(IP+포트) 등록
    // "나는 이 포트번호로 데이터를 받을게" 라고 OS에 등록하는 것
    int bind_result = bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (bind_result < 0) {
        printf("bind 실패\n");
        return -2;
    }
    printf("서버 대기 중...\n");

    // 3. recvfrom() - 데이터 수신
    // 누가 보냈는지 모르니까 client_addr에 보낸 사람 주소가 채워짐
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    struct sockaddr_in client_addr;        // 보낸 클라이언트 주소 저장용
    socklen_t client_addr_size = sizeof(client_addr);

    int recv_result = recvfrom(sock, buffer, sizeof(buffer), 0,
                           (struct sockaddr*)&client_addr, &client_addr_size);
    if (recv_result < 0) {
        printf("recvfrom 실패\n");
        return -3;
    }
    printf("받은 데이터: %s\n", buffer);

    // 4. sendto() - 받은 데이터 그대로 돌려주기 (에코!)
    // recvfrom()에서 받은 client_addr로 돌려보냄
    // TCP와 달리 accept()가 없어서 client_addr에서 클라이언트 주소를 가져옴
        int send_result = sendto(sock, buffer, recv_result, 0,
                         (struct sockaddr*)&client_addr, client_addr_size);
    if (send_result < 0) {
        printf("sendto 실패\n");
        return -4;
    }
    printf("에코 완료\n");

    close(sock);


    return 0;
}