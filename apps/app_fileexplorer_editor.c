#include "pado.h"

#define KEY_ARROW_UP    1000
#define KEY_ARROW_DOWN  1001
#define KEY_ENTER       '\r'
#define KEY_ESC         27

/* get_key(): 확장 키(0xE0)를 처리하여 방향키 상수 반환 */
int get_key() {
    int k = pado_kb_getchar();
    if(k == 0xE0) {  // 확장 키 prefix
        int code = pado_kb_getchar();
        if(code == 72) return KEY_ARROW_UP;
        if(code == 80) return KEY_ARROW_DOWN;
        return code;
    }
    return k;
}

/* editor_mode(): 주어진 파일 노드의 내용을 편집하는 간단한 텍스트 에디터 */
void editor_mode(sand_node* file) {
    #define EDITOR_BUFFER_SIZE 1024
    char buffer[EDITOR_BUFFER_SIZE];
    int pos = 0;
    int i;
    /* 기존 파일 내용 로드 */
    for(i = 0; i < file->size && i < EDITOR_BUFFER_SIZE - 1; i++) {
         buffer[i] = file->data[i];
    }
    pos = i;
    buffer[pos] = '\0';

    pado_clear_screen();
    pado_print("Simple Text Editor\n");
    pado_print("Type your text. Press ESC to finish and save.\n\n");
    pado_print(buffer);
    
    while(1) {
         int key = get_key();
         if(key == KEY_ESC) { // ESC 키로 편집 종료
              break;
         } else if(key == '\b') { // 백스페이스 처리
              if(pos > 0) { pos--; buffer[pos] = '\0'; }
         } else {
              if(pos < EDITOR_BUFFER_SIZE - 1) {
                  buffer[pos++] = (char)key;
                  buffer[pos] = '\0';
              }
         }
         pado_clear_screen();
         pado_print("Simple Text Editor\n");
         pado_print("Type your text. Press ESC to finish and save.\n\n");
         pado_print(buffer);
    }
    
    pado_print("\nSaving file...\n");
    for(i = 0; i < pos && i < 1023; i++) {
         file->data[i] = buffer[i];
    }
    file->data[i] = '\0';
    file->size = pos;
    pado_print("File saved. Press any key to return.");
    get_key();
}

/* app_main(): 파일 탐색기와 에디터를 결합한 메인 앱 */
void app_main(void) {
    while(1) {
        pado_clear_screen();
        pado_print("== File Explorer & Editor ==\n");
        pado_print("Use ↑/↓ to navigate, Enter to open, 'e' to edit file, 'b' to go back, ESC to exit.\n\n");

        /* 현재 디렉터리의 항목 목록 읽기 (최대 64개) */
        sand_node* curr = pado_current_dir->child;
        sand_node* list[64];
        int count = 0;
        while(curr && count < 64) {
             list[count++] = curr;
             curr = curr->sibling;
        }
        
        int selected = 0;
        int exit_loop = 0;
        while(1) {
             pado_clear_screen();
             pado_print("== File Explorer & Editor ==\n");
             pado_print("Use ↑/↓ to navigate, Enter to open, 'e' to edit file, 'b' to go back, ESC to exit.\n\n");
             for (int i = 0; i < count; i++) {
                 if(i == selected)
                     pado_print("> ");
                 else
                     pado_print("  ");
                 pado_print(list[i]->name);
                 if(list[i]->type == SAND_NODE_DIR)
                     pado_print(" [DIR]");
                 pado_print("\n");
             }
             
             int key = get_key();
             if(key == KEY_ARROW_UP) {
                 if(selected > 0) selected--;
             } else if(key == KEY_ARROW_DOWN) {
                 if(selected < count - 1) selected++;
             } else if(key == KEY_ENTER) {
                 if(list[selected]->type == SAND_NODE_DIR) {
                     pado_fs_cd(list[selected]->name);
                     exit_loop = 1;
                     break;
                 } else {
                     /* 파일 실행: 텍스트 파일의 경우, 실행하지 않고 에디터 모드로 전환할 수 있음 */
                     pado_fs_exec(list[selected]->name);
                     exit_loop = 1;
                     break;
                 }
             } else if(key == 'e') {
                 if(list[selected]->type == SAND_NODE_FILE) {
                     editor_mode(list[selected]);
                     exit_loop = 1;
                     break;
                 }
             } else if(key == 'b') {
                 pado_fs_cd_up();
                 exit_loop = 1;
                 break;
             } else if(key == KEY_ESC) {
                 return;
             }
        }
        if(exit_loop)
            continue;
    }
}
