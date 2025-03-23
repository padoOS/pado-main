#include "pado.h"
#include "pado_net.h"

void kernel_main(void) {
    pado_clear_screen();
    pado_print("Welcome to pado OS\n");
    pado_fs_init();
    pado_kb_init();
    pado_net_init();
    pado_shell();
    while(1) { }
}
