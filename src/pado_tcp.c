#include "pado_tcp.h"
#include "pado.h"
#include "pado_net.h"

/* 네트워크/호스트 바이트순 변환 */
unsigned short htons(unsigned short x) {
    return (x << 8) | (x >> 8);
}

unsigned int htonl(unsigned int x) {
    return ((x & 0xFF) << 24) | ((x & 0xFF00) << 8) |
           ((x >> 8) & 0xFF00) | (x >> 24);
}

unsigned short ntohs(unsigned short x) {
    return htons(x);
}

unsigned int ntohl(unsigned int x) {
    return htonl(x);
}

/* TCP 체크섬 (간단한 16비트 워드 합산) */
unsigned short pado_checksum(unsigned short* buf, int nwords) {
    unsigned long sum = 0;
    for (int i = 0; i < nwords; i++) {
        sum += buf[i];
    }
    while(sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);
    return (unsigned short)(~sum);
}

/* TCP 상태 */
typedef enum {
    TCP_CLOSED,
    TCP_SYN_SENT,
    TCP_ESTABLISHED,
    TCP_FIN_WAIT_1,
    TCP_FIN_WAIT_2,
    TCP_CLOSED_WAIT,
    TCP_LAST_ACK,
    TCP_TIME_WAIT
} tcp_state_t;

/* TCP 연결 구조체 (단일 연결만 지원) */
typedef struct {
    tcp_state_t state;
    unsigned int seq; /* 우리 시퀀스 번호 */
    unsigned int ack; /* 기대하는 상대 시퀀스 번호 */
    unsigned short local_port;
    unsigned short remote_port;
    unsigned int remote_ip; /* 네트워크 바이트순 */
} tcp_connection_t;

static tcp_connection_t tcp_conn;

/* 내부: TCP 패킷 전송 – IP, TCP 헤더 생성 후 pado_net_send() 호출 */
static void send_tcp_packet(unsigned char flags, const char *payload, unsigned int payload_len) {
    char packet[1500];
    /* IP 헤더 구성 */
    ip_header_t *ip = (ip_header_t *)packet;
    ip->version_ihl = 0x45;
    ip->tos = 0;
    ip->tot_len = htons(20 + 20 + payload_len);
    ip->id = 0;
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = 6; /* TCP */
    ip->saddr = htonl(0x7F000001);  /* 127.0.0.1 */
    ip->daddr = tcp_conn.remote_ip;
    ip->check = 0;
    ip->check = pado_checksum((unsigned short*)ip, 10);
    
    /* TCP 헤더 구성 */
    tcp_header_t *tcp = (tcp_header_t *)(packet + 20);
    tcp->source = htons(tcp_conn.local_port);
    tcp->dest = htons(tcp_conn.remote_port);
    tcp->seq = htonl(tcp_conn.seq);
    tcp->ack_seq = htonl(tcp_conn.ack);
    tcp->doff_res = (20 / 4) << 4;
    tcp->flags = flags;
    tcp->window = htons(0xFFFF);
    tcp->check = 0;  /* 실제 TCP 체크섬 계산은 pseudo-header 필요 */
    tcp->urg_ptr = 0;
    
    /* 페이로드 복사 */
    char *tcp_payload = packet + 20 + 20;
    for (unsigned int i = 0; i < payload_len; i++) {
        tcp_payload[i] = payload[i];
    }
    int total_len = 20 + 20 + payload_len;
    pado_net_send(packet, total_len);
}

/* 단순 TCP 패킷 수신 – 블로킹, IP 패킷을 받아 TCP 패킷 필터링 */
static int receive_tcp_packet(char *buffer, unsigned int bufsize) {
    while(1) {
        int len = pado_net_recv(buffer, bufsize);
        if(len <= 0) continue;
        ip_header_t *ip = (ip_header_t *)buffer;
        if(ip->protocol != 6) continue;
        tcp_header_t *tcp = (tcp_header_t *)(buffer + 20);
        if(tcp->dest == htons(tcp_conn.local_port) && tcp->source == htons(tcp_conn.remote_port))
            return len;
    }
    return -1;
}

/* TCP 연결: 3-way handshake */
int pado_tcp_connect(const char* dest_ip, unsigned short dest_port) {
    pado_print("Establishing TCP connection...\n");
    /* 간단화를 위해 dest_ip는 "127.0.0.1"로 가정 */
    tcp_conn.remote_ip = htonl(0x7F000001);
    tcp_conn.remote_port = dest_port;
    tcp_conn.local_port = 0x5000;  /* 고정 소스 포트 */
    tcp_conn.seq = 0x1000;         /* 초기 시퀀스 번호 */
    tcp_conn.ack = 0;
    tcp_conn.state = TCP_SYN_SENT;
    
    /* SYN 전송 (SYN 플래그 0x02) */
    send_tcp_packet(0x02, "", 0);
    
    /* SYN+ACK 수신 */
    char recv_buffer[1500];
    int len = receive_tcp_packet(recv_buffer, 1500);
    if(len <= 0) {
        pado_print("TCP: Failed to receive SYN+ACK.\n");
        return -1;
    }
    tcp_header_t *tcp = (tcp_header_t *)(recv_buffer + 20);
    if((tcp->flags & 0x12) == 0x12) {  /* SYN (0x02)와 ACK (0x10) */
        tcp_conn.ack = ntohl(tcp->seq) + 1;
        tcp_conn.state = TCP_ESTABLISHED;
        /* ACK 전송 */
        send_tcp_packet(0x10, "", 0);
        pado_print("TCP connection established.\n");
        return 1; /* 가짜 소켓 번호 */
    } else {
        pado_print("TCP: Unexpected packet received.\n");
        return -1;
    }
}

/* 데이터 전송: PSH+ACK (0x18) 사용 */
int pado_tcp_send(int socket, const void* data, unsigned int len) {
    (void)socket;
    if(tcp_conn.state != TCP_ESTABLISHED) {
        pado_print("TCP: Connection not established.\n");
        return -1;
    }
    send_tcp_packet(0x18, data, len);
    tcp_conn.seq += len;
    return len;
}

/* 데이터 수신: TCP 페이로드를 읽어 ACK 전송 */
int pado_tcp_recv(int socket, void* buffer, unsigned int bufsize) {
    (void)socket;
    if(tcp_conn.state != TCP_ESTABLISHED) {
        pado_print("TCP: Connection not established.\n");
        return -1;
    }
    char packet[1500];
    int len = receive_tcp_packet(packet, 1500);
    if(len <= 0) return -1;
    int header_len = 20 + 20;  /* IP, TCP 헤더 길이 */
    int payload_len = len - header_len;
    if(payload_len > (int)bufsize) payload_len = bufsize;
    for (int i = 0; i < payload_len; i++) {
        ((char*)buffer)[i] = packet[header_len + i];
    }
    tcp_conn.ack += payload_len;
    send_tcp_packet(0x10, "", 0);
    return payload_len;
}

/* TCP 연결 종료: FIN 전송, FIN+ACK 수신, ACK 전송 */
void pado_tcp_close(int socket) {
    (void)socket;
    if(tcp_conn.state != TCP_ESTABLISHED) return;
    send_tcp_packet(0x11, "", 0);  /* FIN + ACK */
    tcp_conn.state = TCP_FIN_WAIT_1;
    char packet[1500];
    int len = receive_tcp_packet(packet, 1500);
    tcp_header_t *tcp = (tcp_header_t *)(packet + 20);
    if(tcp->flags & 0x11) {
        send_tcp_packet(0x10, "", 0);
    }
    tcp_conn.state = TCP_CLOSED;
    pado_print("TCP connection closed.\n");
}
