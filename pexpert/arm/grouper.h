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
 * ARM Grouper PB-A8 platform information
 */

#include "arm_gic.h"

#ifndef _PEXPERT_TEGRA_GROUPER_H_
#define _PEXPERT_TEGRA_GROUPER_H_

#define AMBA_UART_FR(base)      (*(volatile unsigned char *)((base) + 0x18))
#define AMBA_UART_DR(base)      (*(volatile unsigned char *)((base) + 0x00))

#define TEGRA_UART0_BASE     0x70006000
#define REALVIEW_PIC0_BASE      0x1E000000
#define TEGRA_TIMER0_BASE        0x60005000
#define REALVIEW_SYSCTL_BASE    0x10000000

#define REALVIEW_EB_PIC0_BASE   0x10050000

#define HARDWARE_REGISTER(x)    *((unsigned int*)(x)) 
#define HwReg(x)    *((unsigned int*)(x)) 

#define barrier()               __asm__ __volatile__("": : :"memory");

#define PIC_ENABLE              0x1
#define PIC_ALLOW_INTR          0xF0

#define PICPRIOMASK             0x4

#define PIC_INTPRIO             0x10101010
#define PIC_CPUPRIO             0x01010101

#define TIMER_LOAD              0x0
#define TIMER_VALUE             0x4
#define TIMER_CONTROL           0x0
#define TIMER_INTCLR            0xC
#define TIMER_RIS               0x10
#define TIMER_MIS               0x14

#define TIMER_PTV               0x0
#define TIMER_PCR               0x4

#define TIMER_SET_ENABLE            (1 << 31)

#define TIMER_MODE_PERIODIC     (1 << 30)

#define TEGRA_ARM_PERIPHBASE 0x50040000
#define TEGRA_GIC_BASE 0x50041000

#define TEGRA_INT_TIMER1_EXP (GIC_SPI_START + 0)

int Grouper_getc(void);
void Grouper_uart_init(void);
void Grouper_interrupt_init(void);
void Grouper_timebase_init(void);
void Grouper_handle_interrupt(void* context);
uint64_t Grouper_get_timebase(void);
uint64_t Grouper_timer_value(void);
void Grouper_timer_enabled(int enable);
void Grouper_framebuffer_init(void);

#endif
 
