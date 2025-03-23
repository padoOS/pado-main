#include "pado.h"

/* SandFS 관련 */
#define MAX_SAND_NODES 1024
static sand_node pado_sand_nodes[MAX_SAND_NODES];
static int pado_sand_node_index = 0;

sand_node* pado_alloc_node(void) {
    if(pado_sand_node_index < MAX_SAND_NODES) {
        sand_node* node = &pado_sand_nodes[pado_sand_node_index++];
        int i;
        for(i = 0; i < 32; i++) node->name[i] = 0;
        node->child = 0;
        node->sibling = 0;
        node->parent = 0;
        node->size = 0;
        for(i = 0; i < 1024; i++) node->data[i] = 0;
        return node;
    }
    return 0;
}

sand_node* pado_fs_root = 0;
sand_node* pado_current_dir = 0;

void pado_fs_init(void) {
    pado_sand_node_index = 0;
    pado_fs_root = pado_alloc_node();
    if(pado_fs_root) {
        pado_strcpy(pado_fs_root->name, "/");
        pado_fs_root->type = SAND_NODE_DIR;
        pado_fs_root->parent = 0;
        pado_current_dir = pado_fs_root;
    }
}

sand_node* pado_fs_find_child(sand_node* dir, const char* name) {
    if(!dir || dir->type != SAND_NODE_DIR) return 0;
    sand_node* child = dir->child;
    while(child) {
        if(pado_strcmp(child->name, name) == 0)
            return child;
        child = child->sibling;
    }
    return 0;
}

void pado_fs_md(const char* name) {
    if(pado_fs_find_child(pado_current_dir, name)) {
        pado_print("Directory already exists.\n");
        return;
    }
    sand_node* newdir = pado_alloc_node();
    if(!newdir) { pado_print("No memory for new directory.\n"); return; }
    pado_strcpy(newdir->name, name);
    newdir->type = SAND_NODE_DIR;
    newdir->parent = pado_current_dir;
    newdir->sibling = pado_current_dir->child;
    pado_current_dir->child = newdir;
    pado_print("Directory created.\n");
}

void pado_fs_rmd(const char* name) {
    sand_node* prev = 0;
    sand_node* cur = pado_current_dir->child;
    while(cur) {
        if(pado_strcmp(cur->name, name) == 0 && cur->type == SAND_NODE_DIR) {
            if(cur->child) { pado_print("Directory not empty.\n"); return; }
            if(prev) prev->sibling = cur->sibling;
            else pado_current_dir->child = cur->sibling;
            pado_print("Directory removed.\n");
            return;
        }
        prev = cur;
        cur = cur->sibling;
    }
    pado_print("Directory not found.\n");
}

void pado_fs_nf(const char* name, int binary) {
    (void)binary;  /* -binary 옵션은 플래그만 처리 */
    if(pado_fs_find_child(pado_current_dir, name)) {
        pado_print("File already exists.\n");
        return;
    }
    sand_node* newfile = pado_alloc_node();
    if(!newfile) { pado_print("No memory for new file.\n"); return; }
    pado_strcpy(newfile->name, name);
    newfile->type = SAND_NODE_FILE;
    newfile->parent = pado_current_dir;
    newfile->size = 0;
    newfile->data[0] = '\0';
    newfile->sibling = pado_current_dir->child;
    pado_current_dir->child = newfile;
    pado_print("File created.\n");
}

void pado_fs_rmf(const char* name) {
    sand_node* prev = 0;
    sand_node* cur = pado_current_dir->child;
    while(cur) {
        if(pado_strcmp(cur->name, name) == 0 && cur->type == SAND_NODE_FILE) {
            if(prev) prev->sibling = cur->sibling;
            else pado_current_dir->child = cur->sibling;
            pado_print("File removed.\n");
            return;
        }
        prev = cur;
        cur = cur->sibling;
    }
    pado_print("File not found.\n");
}

void pado_fs_cd(const char* name) {
    sand_node* target = pado_fs_find_child(pado_current_dir, name);
    if(target && target->type == SAND_NODE_DIR) {
        pado_current_dir = target;
        pado_print("Changed directory.\n");
    } else {
        pado_print("Directory not found.\n");
    }
}

void pado_fs_cd_up(void) {
    if(pado_current_dir->parent) { pado_current_dir = pado_current_dir->parent; pado_print("Changed to parent directory.\n"); }
    else { pado_print("Already at root.\n"); }
}

void pado_fs_pwd_helper(sand_node* dir, char* buffer) {
    if(dir->parent) {
        pado_fs_pwd_helper(dir->parent, buffer);
        if(pado_strcmp(dir->parent->name, "/") != 0)
            pado_strcat(buffer, "/");
        pado_strcat(buffer, dir->name);
    } else { pado_strcat(buffer, dir->name); }
}

void pado_fs_pwd(void) {
    char buffer[128];
    buffer[0] = '\0';
    pado_fs_pwd_helper(pado_current_dir, buffer);
    pado_print(buffer);
    pado_print("\n");
}

void pado_fs_edit(const char* name) {
    sand_node* file = pado_fs_find_child(pado_current_dir, name);
    if(file && file->type == SAND_NODE_FILE) {
        pado_strcpy(file->data, "Edited content");
        file->size = pado_strlen(file->data);
        pado_print("File edited.\n");
    } else { pado_print("File not found.\n"); }
}

void pado_fs_append(const char* name) {
    sand_node* file = pado_fs_find_child(pado_current_dir, name);
    if(file && file->type == SAND_NODE_FILE) {
        pado_strcat(file->data, " Appended");
        file->size = pado_strlen(file->data);
        pado_print("Appended to file.\n");
    } else { pado_print("File not found.\n"); }
}

/* exec 명령어: SandFS에 저장된 파일을 실행합니다.
   ELF 매직(0x7F 'E' 'L' 'F')인 경우 ELF 로더 호출 */
void pado_fs_exec(const char* name) {
    sand_node* file = pado_fs_find_child(pado_current_dir, name);
    if(!file || file->type != SAND_NODE_FILE) { pado_print("File not found.\n"); return; }
    if(file->size <= 0) { pado_print("Empty file.\n"); return; }
    if((unsigned char)file->data[0] == 0x7F &&
       file->data[1] == 'E' &&
       file->data[2] == 'L' &&
       file->data[3] == 'F') {
         pado_print("Executing ELF binary...\n");
         pado_elf_exec_from_buffer(file->data, file->size);
    } else {
         pado_print("Executing flat binary...\n");
         void (*entry)() = (void (*)())(file->data);
         entry();
    }
}

/* 명령어 이력 관리 */
#define MAX_HISTORY 64
static char pado_history[MAX_HISTORY][128];
static int pado_history_count = 0;

void pado_history_add(const char* command) {
    if(pado_history_count < MAX_HISTORY) {
        int i = 0;
        while(command[i] && i < 127) { pado_history[pado_history_count][i] = command[i]; i++; }
        pado_history[pado_history_count][i] = '\0';
        pado_history_count++;
    }
}

void pado_history_print(void) {
    int i;
    for(i = 0; i < pado_history_count; i++) {
        pado_print(pado_history[i]);
        pado_print("\n");
    }
}

/* 명령어 분석 및 실행 */
void pado_execute_command(char* input) {
    pado_history_add(input);
    char* tokens[10];
    int count = 0, i = 0, in_token = 0;
    while(input[i]) {
        if(input[i] != ' ' && input[i] != '\n') {
            if(!in_token) { tokens[count++] = &input[i]; in_token = 1; }
        } else {
            if(in_token) { input[i] = '\0'; in_token = 0; }
        }
        i++;
    }
    if(count == 0) return;
    if(pado_strcmp(tokens[0], "cd") == 0) {
        if(count >= 2) {
            if(pado_strcmp(tokens[1], "..") == 0)
                pado_fs_cd_up();
            else
                pado_fs_cd(tokens[1]);
        } else { pado_print("Usage: cd <directory>\n"); }
    } else if(pado_strcmp(tokens[0], "md") == 0) {
        if(count >= 2) pado_fs_md(tokens[1]);
        else pado_print("Usage: md <directory>\n");
    } else if(pado_strcmp(tokens[0], "rmd") == 0) {
        if(count >= 2) pado_fs_rmd(tokens[1]);
        else pado_print("Usage: rmd <directory>\n");
    } else if(pado_strcmp(tokens[0], "rmf") == 0) {
        if(count >= 2) pado_fs_rmf(tokens[1]);
        else pado_print("Usage: rmf <filename>\n");
    } else if(pado_strcmp(tokens[0], "history") == 0) {
        pado_history_print();
    } else if(pado_strcmp(tokens[0], "edit") == 0) {
        if(count >= 2) pado_fs_edit(tokens[1]);
        else pado_print("Usage: edit <filename>\n");
    } else if(pado_strcmp(tokens[0], "nf") == 0) {
        int binary = 0;
        if(count >= 2) { if(count >= 3 && pado_strcmp(tokens[2], "-binary") == 0) binary = 1; pado_fs_nf(tokens[1], binary); }
        else pado_print("Usage: nf <filename> [-binary]\n");
    } else if(pado_strcmp(tokens[0], "append") == 0) {
        if(count >= 2) pado_fs_append(tokens[1]);
        else pado_print("Usage: append <filename>\n");
    } else if(pado_strcmp(tokens[0], "pwd") == 0) {
        pado_fs_pwd();
    } else if(pado_strcmp(tokens[0], "cls") == 0) {
        pado_clear_screen();
    } else if(pado_strcmp(tokens[0], "exec") == 0) {
        if(count >= 2) pado_fs_exec(tokens[1]);
        else pado_print("Usage: exec <filename>\n");
    } else if(pado_strcmp(tokens[0], "post") == 0) {
        if(count >= 3) {
            char data[256];
            data[0] = '\0';
            int j;
            for(j = 2; j < count; j++){
                pado_strcat(data, tokens[j]);
                if(j < count - 1)
                    pado_strcat(data, " ");
            }
            char response[512];
            int resp_len = pado_http_post(tokens[1], data, response, 512);
            if(resp_len > 0) {
                response[resp_len] = '\0';
                pado_print("Response:\n");
                pado_print(response);
                pado_print("\n");
            } else {
                pado_print("HTTP POST failed.\n");
            }
        } else {
            pado_print("Usage: post <url> <data>\n");
        }
    } else if(pado_strcmp(tokens[0], "version") == 0) {
        pado_print("pado OS version 1.0\n");
    } else if(pado_strcmp(tokens[0], "ppi") == 0) {
        pado_ppi_command(input);
    } else {
        pado_print("Unknown command.\n");
    }
}

/* 쉘 루프: 키보드 입력을 받아 명령어 실행 */
void pado_shell(void) {
    char line[128];
    int pos = 0;
    pado_print("pado> ");
    while(1) {
        char c = pado_kb_getchar();
        if(c == '\n') {
            line[pos] = '\0';
            pado_print("\n");
            pado_execute_command(line);
            pos = 0;
            pado_print("pado> ");
        } else {
            if(c == '\b') { if(pos > 0) pos--; }
            else {
                if(pos < 127) { line[pos++] = c; char echo[2] = { c, '\0' }; pado_print(echo); }
            }
        }
    }
}
