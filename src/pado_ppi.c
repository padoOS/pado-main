#include "pado.h"
#include "pado_net.h"
#include "pado_tcp.h"

#define PPI_REPO_URL "http://padoOS.github.io/"

// 내부: 패키지 설치 함수
static void pado_ppi_install(const char *package_name) {
    char url[128];
    pado_strcpy(url, PPI_REPO_URL);
    pado_strcat(url, package_name);
    pado_strcat(url, ".pkg");
    
    pado_print("Downloading package descriptor from: ");
    pado_print(url);
    pado_print("\n");
    
    char pkg_response[4096];
    int pkg_len = pado_http_get(url, pkg_response, 4096);
    if(pkg_len <= 0) {
        pado_print("Failed to download package descriptor.\n");
        return;
    }
    pkg_response[pkg_len] = '\0';
    
    /* .pkg 파일 형식 예:
       pkgname:hello files:http://hello.com/hello.bin,http://hello.com/readme.txt */
    char *files_str = 0;
    int i = 0;
    while(pkg_response[i]) {
        if(pkg_response[i]=='f' && pkg_response[i+1]=='i' && pkg_response[i+2]=='l' &&
           pkg_response[i+3]=='e' && pkg_response[i+4]=='s' && pkg_response[i+5]==':') {
            files_str = &pkg_response[i+6];
            break;
        }
        i++;
    }
    if(!files_str) {
        pado_print("Invalid package descriptor format (files: not found).\n");
        return;
    }
    pado_print("Package descriptor parsed. Downloading files...\n");
    
    /* 파일 목록은 콤마로 구분 */
    char *current = files_str;
    while(*current) {
        char *comma = current;
        while(*comma && *comma != ',') comma++;
        char url_buf[256];
        int url_len = comma - current;
        if(url_len >= 256) url_len = 255;
        for(i = 0; i < url_len; i++) {
            url_buf[i] = current[i];
        }
        url_buf[url_len] = '\0';
        /* 선행 공백 제거 */
        while(url_buf[0]==' ') {
            for(i = 0; url_buf[i]; i++) {
                url_buf[i] = url_buf[i+1];
            }
        }
        /* 파일 다운로드 */
        char file_response[8192];
        int file_len = pado_http_get(url_buf, file_response, 8192);
        if(file_len <= 0) {
            pado_print("Failed to download file: ");
            pado_print(url_buf);
            pado_print("\n");
        } else {
            /* URL의 마지막 '/' 이후가 파일명 */
            char *filename = url_buf;
            char *p = url_buf;
            while(*p) {
                if(*p=='/') filename = p+1;
                p++;
            }
            pado_print("Downloaded file: ");
            pado_print(filename);
            pado_print("\n");
            /* SandFS에 파일 저장: 기존 파일 삭제 후 새 파일 생성 */
            pado_fs_rmf(filename);
            pado_fs_nf(filename, 0);
            sand_node* file_node = pado_fs_find_child(pado_current_dir, filename);
            if(file_node) {
                int copy_len = (file_len < 1024) ? file_len : 1024;
                for(i = 0; i < copy_len; i++) {
                    file_node->data[i] = file_response[i];
                }
                file_node->size = copy_len;
                pado_print("Installed file: ");
                pado_print(filename);
                pado_print("\n");
            } else {
                pado_print("Error: Could not create file node for ");
                pado_print(filename);
                pado_print("\n");
            }
        }
        if(*comma == ',') {
            current = comma + 1;
        } else {
            break;
        }
    }
    pado_print("Package installation complete.\n");
}

// ppi 명령어 처리: 사용법: "ppi install <package_name>"
void pado_ppi_command(char* input) {
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
    if(count < 2) {
        pado_print("Usage: ppi install <package_name>\n");
        return;
    }
    if(pado_strcmp(tokens[1], "install") == 0) {
        if(count >= 3) {
            pado_ppi_install(tokens[2]);
        } else {
            pado_print("Usage: ppi install <package_name>\n");
        }
    } else {
        pado_print("Unknown ppi command.\n");
    }
}
