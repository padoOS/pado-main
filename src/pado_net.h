#ifndef PADO_NET_H
#define PADO_NET_H

/* Ethernet header */
typedef struct {
    unsigned char dest[6];
    unsigned char src[6];
    unsigned short type;
} __attribute__((packed)) ethernet_header_t;

/* IP header */
typedef struct {
    unsigned char version_ihl;
    unsigned char tos;
    unsigned short tot_len;
    unsigned short id;
    unsigned short frag_off;
    unsigned char ttl;
    unsigned char protocol;
    unsigned short check;
    unsigned int saddr;
    unsigned int daddr;
} __attribute__((packed)) ip_header_t;

/* TCP header */
typedef struct {
    unsigned short source;
    unsigned short dest;
    unsigned int seq;
    unsigned int ack_seq;
    unsigned char doff_res;
    unsigned char flags;
    unsigned short window;
    unsigned short check;
    unsigned short urg_ptr;
} __attribute__((packed)) tcp_header_t;

void pado_net_init(void);
int pado_net_send(const void *data, unsigned int len);
int pado_net_recv(void *buffer, unsigned int bufsize);

/* HTTP GET 및 POST 요청 */
int pado_http_get(const char* url, char* response, unsigned int response_size);
int pado_http_post(const char* url, const char* data, char* response, unsigned int response_size);

#endif
