CFLAGS = -ffreestanding -O2 -Wall -Wextra -nostdlib -I.
ASFLAGS = -felf
LDFLAGS = -nostdlib -T link.ld

all: pado.iso

kernel.o: kernel.c pado.h pado_net.h
	i686-elf-gcc $(CFLAGS) -c kernel.c -o kernel.o

pado_base.o: pado_base.c pado.h
	i686-elf-gcc $(CFLAGS) -c pado_base.c -o pado_base.o

pado_fs.o: pado_fs.c pado.h
	i686-elf-gcc $(CFLAGS) -c pado_fs.c -o pado_fs.o

pado_elf.o: pado_elf.c pado.h
	i686-elf-gcc $(CFLAGS) -c pado_elf.c -o pado_elf.o

pado_kb.o: pado_kb.c pado.h
	i686-elf-gcc $(CFLAGS) -c pado_kb.c -o pado_kb.o

pado_net.o: pado_net.c pado_net.h pado.h
	i686-elf-gcc $(CFLAGS) -c pado_net.c -o pado_net.o

pado_tcp.o: pado_tcp.c pado_tcp.h pado.h pado_net.h
	i686-elf-gcc $(CFLAGS) -c pado_tcp.c -o pado_tcp.o

pado_ppi.o: pado_ppi.c pado.h
	i686-elf-gcc $(CFLAGS) -c pado_ppi.c -o pado_ppi.o

pado_interrupts.o: pado_interrupts.s
	i686-elf-as $(ASFLAGS) pado_interrupts.s -o pado_interrupts.o

kernel.bin: kernel.o pado_base.o pado_fs.o pado_elf.o pado_kb.o pado_net.o pado_tcp.o pado_ppi.o pado_interrupts.o
	i686-elf-ld $(LDFLAGS) -o kernel.bin kernel.o pado_base.o pado_fs.o pado_elf.o pado_kb.o pado_net.o pado_tcp.o pado_ppi.o pado_interrupts.o

pado.iso: kernel.bin
	mkdir -p isodir/boot/grub
	cp kernel.bin isodir/boot/kernel.bin
	echo 'set timeout=0' > isodir/boot/grub/grub.cfg
	echo 'set default=0' >> isodir/boot/grub/grub.cfg
	echo 'menuentry "pado" {' >> isodir/boot/grub/grub.cfg
	echo '   multiboot /boot/kernel.bin' >> isodir/boot/grub/grub.cfg
	echo '   boot' >> isodir/boot/grub/grub.cfg
	echo '}' >> isodir/boot/grub/grub.cfg
	grub-mkrescue -o pado.iso isodir

clean:
	rm -rf *.o *.bin *.iso isodir
