.global pado_kb_isr_stub
.global pado_idt_load

pado_kb_isr_stub:
    cli
    pusha
    call pado_kb_handler
    popa
    sti
    iret

pado_idt_load:
    movl 4(%esp), %eax
    lidt (%eax)
    ret
