#include "pado.h"

volatile unsigned short* pado_video_memory = (volatile unsigned short*) VGA_ADDRESS;
static int cursor_x = 0, cursor_y = 0;

void pado_clear_screen(void) {
    int i;
    for(i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        pado_video_memory[i] = (WHITE_ON_BLUE << 8) | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
}

void pado_print(const char* str) {
    int i = 0;
    while (str[i]) {
        if (str[i] == '\n') {
            cursor_x = 0;
            cursor_y++;
        } else {
            int index = cursor_y * VGA_WIDTH + cursor_x;
            pado_video_memory[index] = (WHITE_ON_BLUE << 8) | str[i];
            cursor_x++;
            if(cursor_x >= VGA_WIDTH) { cursor_x = 0; cursor_y++; }
            if(cursor_y >= VGA_HEIGHT) { cursor_y = 0; }
        }
        i++;
    }
}

int pado_strlen(const char* s) {
    int len = 0;
    while(s[len]) len++;
    return len;
}

int pado_strcmp(const char* s1, const char* s2) {
    int i = 0;
    while(s1[i] && s2[i]) {
        if(s1[i] != s2[i])
            return s1[i] - s2[i];
        i++;
    }
    return s1[i] - s2[i];
}

void pado_strcpy(char* dest, const char* src) {
    int i = 0;
    while(src[i]) { dest[i] = src[i]; i++; }
    dest[i] = '\0';
}

void pado_strcat(char* dest, const char* src) {
    int i = pado_strlen(dest);
    int j = 0;
    while(src[j]) { dest[i++] = src[j++]; }
    dest[i] = '\0';
}
