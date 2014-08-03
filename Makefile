WARNINGS := -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-align \
                -Wwrite-strings -Wredundant-decls -Wnested-externs -Winline \
-Wuninitialized -Wstrict-prototypes \
-Wno-unused-parameter -Wno-cast-align -Werror -Wno-unused-function 

TOOLCHAININC = 
GCCINC = 
CC = i686-pc-orbitos-gcc
CFLAGS := -O0 -nostdlib -nostdinc -I./kernel/include -I$(GCCINC) -I$(TOOLCHAININC) -std=gnu99 -march=i586 $(WARNINGS) -ggdb3 
LD = i686-pc-orbitos-ld
CPPFLAGS := -D__ASSEMBLY__ -D__i386__ -I./kernel/include

USERSPACE = $(shell find userspace/ -type f -name '*.c') $(shell find userspace/ -type f -name '*.cpp') $(shell find userspace/ -type f -name '*.h')
EMU = qemu-system-i386
EMUARGS     = -sdl -kernel orbitos-kernel -m 1424  -vga std -serial stdio -hda ext2_final.img -initrd initrd.img  -monitor stdio
EMUKVM      = -enable-kvm 


SUBMODULES  := $(patsubst %.c,%.o,$(wildcard kernel/*/*.c))


.PHONY: all system clean clean-once clean-hard clean-soft clean-bin clean-aux clean-core install run
.SECONDARY: 

all: system tags
system: orbitos-kernel

run: system
	${EMU} ${EMUARGS} -append "vid=qemu hdd"
debug: system
	#sudo mount /dev/sdc1 /mnt
	#sudo cp chiffos-kernel /mnt/boot/
	#sudo cp initrd.img /mnt/boot
	#sudo umount /mnt
	${EMU} ${EMUARGS} ${EMUKVM}

################
#    Kernel    #
################
orbitos-kernel: kernel/start.o kernel/link.ld kernel/main.o ${SUBMODULES} 
	${LD} -T kernel/link.ld -o orbitos-kernel kernel/*.o ${SUBMODULES}
	
kernel/head.o: kernel/head.s
	${CC} ${CPPFLAGS} -c -o kernel/head.o kernel/head.s
kernel/data.o: kernel/start.s
	nasm -f elf -o kernel/data.o kernel/data.s	

kernel/start.o: kernel/start.s
	nasm -f elf -o kernel/start.o kernel/start.s

kernel/sys/version.o: kernel/*/*.c kernel/*.c

%.o: %.c
	${CC} ${CFLAGS} -I./kernel/include -c -o $@ $< 


#USERSPACEPROG := $(shell find userspace/gui/shell/ -maxdepth 2 -name 'Makefile' -exec dirname {} \;)
.userspace-check: ${USERSPACE}


orbitos-disk.img: .userspace-check
	@set -e; for prog in $(USERSPACEPROG); do \
		make -C $$prog; \
	done

###############
#    clean    #
###############
clean-soft:
	@-rm -f kernel/*.o
	@-rm -f ${SUBMODULES}

clean-core:
	@-rm -f orbitos-kernel

clean: clean-soft clean-core


