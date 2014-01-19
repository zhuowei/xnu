/*
 * Copyright 2013, winocm. <winocm@icloud.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 *   Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 *   If you are going to use this software in any form that does not involve
 *   releasing the source to this project or improving it, let me know beforehand.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * PE bringup (semihosting)
 */

#include <mach/mach_types.h>
#include <pexpert/pexpert.h>
#include <pexpert/machine/protos.h>
#include <pexpert/machine/boot.h>
#include <machine/machine_routines.h>
#include <kern/debug.h>
#include <stdarg.h>

#include "semihost.h"
#include "consolefont.h"

#define VIDEOBASE 0xabe01000
#define WIDTH 800
#define HEIGHT 1280

#define FONT_WIDTH 8

static int Tegra_console_row = 0;
static int Tegra_console_col = 0;

/**
 * PE_early_puts
 *
 * Bringup function, use for semihosting. Prints to SYS_OUTPUT0.
 */

static void Tegra_framebuffer_setPixel(int x, int y, int color) {
	int loc = VIDEOBASE + (((y * WIDTH) + x) * 4);
	*((int*) loc) = color;
}

static int Tegra_framebuffer_drawCharacter(char mychar, int screenr, int screenc) {
	int index = ((int) mychar) * FONT_WIDTH;
	int rr, cc;
	for (rr = 0; rr < FONT_WIDTH; rr++) {
		int myrow = console_font[index + rr];
		for (cc = 0; cc < FONT_WIDTH; cc++) {
			int thisPixel = (myrow >> (FONT_WIDTH - 1 - cc)) & 0x1;
			if (thisPixel) {
				Tegra_framebuffer_setPixel(screenc + cc, screenr + rr, 0xffffff);
			}
		}
	}
	return console_font_widths[(int) mychar] + 1;
}

void Tegra_console_putchar(char c) {
	//if (Tegra_console_row + FONT_WIDTH + 1 > HEIGHT) {
	//	memset((void*) VIDEOBASE, 0, WIDTH*HEIGHT*4);
	//	Tegra_console_row = 0;
	//}
if (Tegra_console_row > HEIGHT) {
    Tegra_console_row = 0;
}
	if (c == '\n') {
		Tegra_console_row += (FONT_WIDTH + 1);
		Tegra_console_col = 0;
		return;
	}
	Tegra_console_col += Tegra_framebuffer_drawCharacter(c, Tegra_console_row, Tegra_console_col);
	if (Tegra_console_col + FONT_WIDTH > WIDTH) {
		Tegra_console_row += (FONT_WIDTH + 1);
		Tegra_console_col = 0;
	}
}

void PE_early_puts(char *s)
{
    int x = 0;
    int color = 0x00ff00;
    while (*s != '\0') {
        //Tegra_console_putchar(*s);
        PE_semihost_write_char(*s);
        //int loc = VIDEOBASE + (((y * WIDTH) + x) * 4);
        //*((int*) loc) = color;
        s++;
        //x++;
    }
    //y++;
    return;
}
