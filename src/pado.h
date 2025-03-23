#ifndef PADO_H
#define PADO_H

/* VGA 관련 상수 */
#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define WHITE_ON_BLUE 0x1F

/* 기본 함수 프로토타입 */
void pado_clear_screen(void);
void pado_print(const char* str);
int pado_strlen(const char* s);
int pado_strcmp(const char* s1, const char* s2);
void pado_strcpy(char* dest, const char* src);
void pado_strcat(char* dest, const char* src);

/* SandFS – 파일 및 디렉터리 트리 */
typedef enum {
    SAND_NODE_DIR,
    SAND_NODE_FILE
} sand_node_type;

typedef struct sand_node {
    char name[32];
    sand_node_type type;
    struct sand_node* parent;
    struct sand_node* child;
    struct sand_node* sibling;
    char data[1024];
    int size;
} sand_node;

void pado_fs_init(void);
void pado_fs_md(const char* name);
void pado_fs_rmd(const char* name);
void pado_fs_cd(const char* name);
void pado_fs_cd_up(void);
void pado_fs_pwd(void);
void pado_fs_nf(const char* name, int binary);
void pado_fs_rmf(const char* name);
void pado_fs_edit(const char* name);
void pado_fs_append(const char* name);
sand_node* pado_fs_find_child(sand_node* dir, const char* name);

/* ELF 실행 – 실제 ELF 헤더 파싱 */
void pado_elf_exec_from_buffer(char* buffer, int size);
void pado_fs_exec(const char* name);

/* 명령어 이력 및 쉘 */
void pado_history_add(const char* command);
void pado_history_print(void);
void pado_execute_command(char* input);
void pado_shell(void);

/* 키보드 및 포트 I/O */
unsigned char pado_inb(unsigned short port);
void pado_outb(unsigned short port, unsigned char data);
void pado_kb_init(void);
void pado_kb_handler(void);
char pado_kb_getchar(void);

/* 네트워크 – Ethernet, IP, TCP, HTTP */
#include "pado_net.h"  /* pado_net.h와 pado_tcp.h 포함 */

/* ppi (pado package installer) 관련 */
void pado_ppi_command(char* input);

#endif
