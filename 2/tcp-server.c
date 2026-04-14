#include <sys/socket.h>   // socket, bind, listen, accept, connect
#include <sys/time.h>     // gettimeofday
#include <netinet/in.h>   // sockaddr_in 구조체
#include <arpa/inet.h>    // inet_addr, htons 같은 주소 변환
#include <unistd.h>       // close, read, write
#include <string.h>       // memset, strlen
#include <stdio.h>     

int main(void){
    // 1. socket() - 소켓 생성
    // IPv4, TCP 방식의 소켓을 만들고 sock에 소켓 번호(파일 디스크립터)를 저장
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        printf("socket 생성 실패\n");
        return -1;
    }

    // bind() 전 준비 - 서버 자신의 주소 구조체 설정
    // 클라이언트는 서버 주소를 채웠지만, 서버는 자기 자신의 주소를 채운다.
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));  // 쓰레기값 제거
    server_addr.sin_family = AF_INET;              // IPv4 방식으로 통신
    server_addr.sin_port = htons(53140);           // 포트번호
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 어떤 IP에서 오는 연결이든 다 받겠다는 뜻

    // 2. bind() - 소켓에 주소(IP+포트) 등록
    // "나는 이 포트번호로 연결을 받을게" 라고 OS에 등록하는 것
    int bind_result = bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (bind_result < 0) {
        printf("bind 실패\n");
        return -2;
    }

    // 3. listen() - 연결 대기 상태로 전환
    // bind()까지 하면 포트 등록만 된 거고, listen()을 해야 실제로 연결 받을 준비 완료
    // 두 번째 인자 1은 동시에 대기할 수 있는 연결 요청 최대 개수
    int listen_result = listen(sock, 1);
    if (listen_result < 0) {
        printf("listen 실패\n");
        return -3;
    }
    printf("서버 대기 중...\n");

    // 4. accept() - 클라이언트 연결 수락
    // 클라이언트가 connect()할 때까지 여기서 멈추고 기다림
    // 연결되면 실제 통신용 새 소켓(c_sock)을 반환
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    int c_sock = accept(sock, (struct sockaddr*)&client_addr, &client_addr_size);
    if (c_sock < 0) {
        printf("accept 실패\n");
        return -4;
    }
    printf("클라이언트 연결됨\n");

    // 5. recv() - 데이터 수신 후 삭제
    // TCP는 클라이언트가 close()하면 recv()가 0을 반환해서 자동으로 종료됨
    // 첫 데이터 받은 시점부터 측정 시작
    // → 클라이언트가 scanf로 speed 입력하는 시간 제외
    int total_bytes = 0;
    struct timeval start, end;

    while (1) {
        char buffer[2000];
        memset(buffer, 0, sizeof(buffer));  // 버퍼 쓰레기값 제거

        int recv_result = recv(c_sock, buffer, sizeof(buffer), 0);

        // 클라이언트가 close()하면 recv()가 0 반환 → 탈출
        if (recv_result <= 0) break;

        // 첫 데이터 받은 시점부터 측정 시작
        if (total_bytes == 0) gettimeofday(&start, NULL);

        total_bytes += recv_result;  // 수신 바이트 누적
        // 데이터 삭제 (그냥 안 씀)
    }

    gettimeofday(&end, NULL);  // 수신 종료 시간 기록
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("수신 완료\n");
    printf("경과 시간: %.2f초\n", elapsed);
    printf("총 수신 바이트: %d\n", total_bytes);
    printf("throughput: %.2f bytes/s\n", total_bytes / elapsed);

    // 6. close() - 소켓 닫기
    // c_sock 먼저 닫고 (통신용 소켓) sock은 나중에 닫음 (대기용 소켓)
    close(c_sock);
    close(sock);

    return 0;
}