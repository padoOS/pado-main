#include "pado_net.h"
#include "pado.h"

#define NE2000_BASE 0x300

/* 단순화를 위한 NE2000 레지스터 오프셋 */
#define CR      0x00
#define DCR     0x0E
#define RCR     0x0C
#define TCR     0x0D
#define RBSTART 0x01
#define RBSTOP  0x02

void pado_net_init(void) {
    pado_print("Initializing NE2000...\n");
    pado_outb(NE2000_BASE + CR, 0x21);      // Stop & abort
    pado_outb(NE2000_BASE + DCR, 0x49);       // 예제 값
    pado_outb(NE2000_BASE + RCR, 0x20);
    pado_outb(NE2000_BASE + TCR, 0x02);
    pado_outb(NE2000_BASE + RBSTART, 0x40);
    pado_outb(NE2000_BASE + RBSTOP, 0x80);
    pado_print("NE2000 initialized.\n");
}

int pado_net_send(const void *data, unsigned int len) {
    pado_print("NE2000: Sending packet...\n");
    return len;
}

int pado_net_recv(void *buffer, unsigned int bufsize) {
    return 0;
}

#include "pado_tcp.h"
int pado_http_get(const char* url, char* response, unsigned int response_size) {
    pado_print("pado_http: HTTP GET ");
    pado_print(url);
    pado_print("\n");
    int sock = pado_tcp_connect("127.0.0.1", 80);
    if(sock < 0) {
        pado_print("pado_http: TCP connect failed.\n");
        return -1;
    }
    char request[256];
    pado_strcpy(request, "GET / HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n");
    pado_tcp_send(sock, request, pado_strlen(request));
    int len = pado_tcp_recv(sock, response, response_size);
    pado_tcp_close(sock);
    return len;
}

int pado_http_post(const char* url, const char* data, char* response, unsigned int response_size) {
    pado_print("pado_http: HTTP POST ");
    pado_print(url);
    pado_print("\n");
    int sock = pado_tcp_connect("127.0.0.1", 80);
    if(sock < 0) {
        pado_print("pado_http: TCP connect failed.\n");
        return -1;
    }
    char request[512];
    pado_strcpy(request, "POST / HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: ");
    int data_len = pado_strlen(data);
    char len_str[16];
    int temp = data_len, idx = 0;
    char temp_buf[16];
    if(temp == 0) { temp_buf[idx++] = '0'; }
    else {
        while(temp > 0) {
            temp_buf[idx++] = '0' + (temp % 10);
            temp /= 10;
        }
    }
    int i;
    for(i = 0; i < idx; i++){
        len_str[i] = temp_buf[idx - 1 - i];
    }
    len_str[idx] = '\0';
    pado_strcat(request, len_str);
    pado_strcat(request, "\r\n\r\n");
    pado_strcat(request, data);
    pado_tcp_send(sock, request, pado_strlen(request));
    int len = pado_tcp_recv(sock, response, response_size);
    pado_tcp_close(sock);
    return len;
}
