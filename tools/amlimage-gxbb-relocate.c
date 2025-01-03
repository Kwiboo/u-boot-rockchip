// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright Contributors to the U-Boot project.

/*
 * The following commands can be used to reproduce the gxbb_relocate
 * byte array in amlimage.c
 *
 * Start U-Boot CI docker container from U-Boot source code root folder:
 *  docker run --rm -v $(pwd):/build -u uboot -it docker.io/trini/u-boot-gitlab-ci-runner:jammy-20240911.1-08Dec2024
 *
 * Run the following commands inside the docker container:
 *  export PATH=/opt/gcc-13.2.0-nolibc/aarch64-linux/bin:$PATH
 *  cd /build/tools
 *
 * Generate assembly code for the c code in this file:
 *  aarch64-linux-gcc -nostdlib -ffreestanding -Os -S -o amlimage-gxbb-relocate.S amlimage-gxbb-relocate.c
 *
 * Manually remove 'mov x16, x2' and replace 'x16' with 'x2' on the last line.
 *
 * Compile assembly code and extract the AArch64 binary code:
 *  aarch64-linux-as -o amlimage-gxbb-relocate.o amlimage-gxbb-relocate.S
 *  aarch64-linux-objcopy -O binary -j .text amlimage-gxbb-relocate.o amlimage-gxbb-relocate.bin
 *
 * Print binary code as a byte array that can be copied into amlimage.c
 *  hexdump -ve '1/1 "0x%.2x, "' amlimage-gxbb-relocate.bin | fold -w 72 && echo
 *
 * Remember to update assembly code below when byte array is updated.
 */

#include <stdint.h>

#define TZRAM_BASE		0xd9000000
#define PAYLOAD_OFFSET		0x200
#define BL2_OFFSET		0x1000
#define BL2_BASE		(void *)(TZRAM_BASE + BL2_OFFSET)
#define BL2_SIZE		0xb000

void _start(uint64_t x0, uint64_t x1)
{
	void (*bl2)(uint64_t, uint64_t) = BL2_BASE;
	uint64_t i, *dst = BL2_BASE, *src = BL2_BASE + PAYLOAD_OFFSET;

	/* memmove payload from 0x1200 to 0x1000 offset in TZRAM */
	for (i = 0; i < BL2_SIZE / sizeof(*src); i++)
		*(dst + i) = *(src + i);

	/* goto entry point with x0 and x1 reg intact */
	bl2(x0, x1);
}

/*
	.arch armv8-a
	.file	"amlimage-gxbb-relocate.c"
	.text
	.align	2
	.global	_start
	.type	_start, %function
_start:
.LFB0:
	.cfi_startproc
	mov	x2, 4608
	movk	x2, 0xd900, lsl 16
	add	x3, x2, 45056
.L2:
	sub	x4, x2, #32768
	add	x2, x2, 8
	ldr	x5, [x2, -8]
	str	x5, [x4, 32256]
	cmp	x2, x3
	bne	.L2
	mov	x2, 4096
	movk	x2, 0xd900, lsl 16
	br	x2
	.cfi_endproc
.LFE0:
	.size	_start, .-_start
	.ident	"GCC: (GNU) 13.2.0"
	.section	.note.GNU-stack,"",@progbits
*/
