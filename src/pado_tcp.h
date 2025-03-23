#ifndef PADO_TCP_H
#define PADO_TCP_H

int pado_tcp_connect(const char* dest_ip, unsigned short dest_port);
int pado_tcp_send(int socket, const void* data, unsigned int len);
int pado_tcp_recv(int socket, void* buffer, unsigned int bufsize);
void pado_tcp_close(int socket);
unsigned short pado_checksum(unsigned short* buf, int nwords);

/* 간단한 네트워크/호스트 바이트순 변환 함수 */
unsigned short htons(unsigned short x);
unsigned int htonl(unsigned int x);
unsigned short ntohs(unsigned short x);
unsigned int ntohl(unsigned int x);

#endif
