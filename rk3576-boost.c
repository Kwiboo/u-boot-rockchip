// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright Contributors to the U-Boot project.

/*
 * Generate assembly code for the c code in this file:
 *  aarch64-linux-gnu-gcc -nostdlib -ffreestanding -Os -S -o rk3576-boost.S rk3576-boost.c
 *
 * Compile assembly code and extract the AArch64 binary code:
 *  aarch64-linux-gnu-as -o rk3576-boost.o rk3576-boost.S
 *  aarch64-linux-gnu-objcopy -O binary -j .text rk3576-boost.o rk3576-boost.bin
 */

#include <stdint.h>

#define SYS_SRAM_BASE	0x3ff80000
#define OFFSET		0x03b0

int _start(void)
{
	uint32_t *sram = (void*)(SYS_SRAM_BASE + OFFSET);

	/* set unknown value in sram to fix boot from sdmmc */
	*(sram) = 0x3ffff800;

	return 0;
}

/*
	.arch armv8-a
	.file	"rk3576-boost.c"
	.text
	.align	2
	.global	_start
	.type	_start, %function
_start:
.LFB0:
	.cfi_startproc
	mov	x0, 944
	mov	w1, 1073739776
	movk	x0, 0x3ff8, lsl 16
	str	w1, [x0]
	mov	w0, 0
	ret
	.cfi_endproc
.LFE0:
	.size	_start, .-_start
	.ident	"GCC: (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0"
	.section	.note.GNU-stack,"",@progbits
*/
