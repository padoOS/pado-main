#include "pado.h"

unsigned char pado_inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void pado_outb(unsigned short port, unsigned char data) {
    __asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

#define KB_BUFFER_SIZE 128
static char kb_buffer[KB_BUFFER_SIZE];
static int kb_buffer_pos = 0;
static char scancode_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0
};

volatile int pado_kb_data_ready = 0;
volatile char pado_kb_last_char = 0;

void pado_kb_handler(void) {
    unsigned char scancode = pado_inb(0x60);
    if(scancode < 128) {
        char c = scancode_map[scancode];
        if(c) {
            if(kb_buffer_pos < KB_BUFFER_SIZE - 1) {
                kb_buffer[kb_buffer_pos++] = c;
                kb_buffer[kb_buffer_pos] = '\0';
            }
            pado_kb_last_char = c;
            pado_kb_data_ready = 1;
        }
    }
    pado_outb(0x20, 0x20);
}

void pado_kb_init(void) {
    extern void pado_idt_load(unsigned int);
    extern void pado_kb_isr_stub(void);
    struct {
        unsigned short limit;
        unsigned int base;
    } idtp;
    idtp.limit = 256 * 8 - 1;
    idtp.base = 0;  /* 실제 IDT 설정은 별도 초기화 필요 */
    pado_idt_load((unsigned int)&idtp);
}

char pado_kb_getchar(void) {
    while(!pado_kb_data_ready);
    pado_kb_data_ready = 0;
    return pado_kb_last_char;
}
