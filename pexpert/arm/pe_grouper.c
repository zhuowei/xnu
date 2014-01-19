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
 * Pee Expert for Asus Nexus 7 2012 (Grouper).
 * Originally contributed by Zhuowei Zhang.
 */

#include <mach/mach_types.h>

#include <pexpert/pexpert.h>
#include <pexpert/arm/protos.h>
#include <pexpert/arm/boot.h>

#include <machine/machine_routines.h>

#include <vm/pmap.h>
#include <arm/pmap.h>

#include "grouper.h"
#include "arm_gic.h"

/*
 * This is board specific stuff.
 */
#if defined(BOARD_CONFIG_TEGRA30_GROUPER)

#define KPRINTF_PREFIX      "PE_Grouper: "

extern void rtclock_intr(arm_saved_state_t * regs);
extern void rtc_configure(uint64_t hz);

vm_offset_t gTegraUartBase;
vm_offset_t gTegraPicBase;
vm_offset_t gTegraTimerBase;
vm_offset_t gTegraSysControllerBase;

vm_offset_t gTegraPicDistribBase;

vm_offset_t gTegraPl111Base;
vm_offset_t gTegraGICDistributorBase;
vm_offset_t gTegraGICCPUBase;

vm_offset_t gTegraResetBase;
vm_offset_t gTegraICTLRBase;

static uint64_t clock_decrementer = 0;
static boolean_t clock_initialized = FALSE;
static boolean_t clock_had_irq = FALSE;
static uint64_t clock_absolute_time = 0;

static uint32_t num_irq = 0;

#define CLK_RST_CONTROLLER_RST_DEVICES 0x4

static void reboot_system(void) {
    HwReg(gTegraResetBase + CLK_RST_CONTROLLER_RST_DEVICES) |= 1 << 0;
}

static void timer_configure(void)
{
    uint64_t hz = 100000; //1MHz
    clock_decrementer = 1000;//(hz / 7);   // For 500Hz.

    gPEClockFrequencyInfo.timebase_frequency_hz = hz;

    kprintf(KPRINTF_PREFIX "decrementer frequency = %llu\n", clock_decrementer);

    rtc_configure(hz);
}

void Grouper_putc(int c)
{

}

int Grouper_getc(void)
{
    return 'x';
}

void Grouper_uart_init(void)
{
    int periphBase = TEGRA_ARM_PERIPHBASE; //TODO: read this from CP15
    //The Nexus 7 has only one user-accessible UART through the headphone port, and that requires a special cable.
    //WTF is timer init here?
    gTegraTimerBase = ml_io_map(TEGRA_TIMER0_BASE, PAGE_SIZE);
    gTegraGICDistributorBase = ml_io_map(periphBase + GIC_DIST_PERIPHBASE_OFFSET, PAGE_SIZE);
    gTegraGICCPUBase = ml_io_map(periphBase, PAGE_SIZE) + GIC_CPU_PERIPHBASE_OFFSET;
    gTegraResetBase = ml_io_map(0x60006000, PAGE_SIZE);
    gTegraICTLRBase = ml_io_map(0x60004000, PAGE_SIZE);
}
#define gTouchPadqGICDistributerBase gTegraGICDistributorBase
#define gTouchpadqGICCPUBase gTegraGICCPUBase
/* Intialize distributor */
static void qgic_dist_init(void)
{
    uint32_t i;
    //uint32_t num_irq = 0;
    uint32_t cpumask = 1;

    cpumask |= cpumask << 8;
    cpumask |= cpumask << 16;

    /* Disabling GIC */
    HwReg(gTouchPadqGICDistributerBase + GIC_DIST_CTRL) = 0;

    /*
     * Find out how many interrupts are supported.
     */
    num_irq = HwReg(gTouchPadqGICDistributerBase + GIC_DIST_CTR) & 0x1f;
    num_irq = (num_irq + 1) * 32;
    kprintf("Number of IRQs: %x\n", num_irq);

    /* Set each interrupt line to use N-N software model
     * and edge sensitive, active high
     */
    for (i=32; i < num_irq; i += 16)
        HwReg(gTouchPadqGICDistributerBase + GIC_DIST_CONFIG + i * 4/16) = 0xffffffff;

    HwReg(gTouchPadqGICDistributerBase + GIC_DIST_CONFIG + 4) = 0xffffffff;

    /* Set up interrupts for this CPU */
    for (i = 32; i < num_irq; i += 4)
        HwReg(gTouchPadqGICDistributerBase + GIC_DIST_TARGET + i * 4 / 4) = cpumask;

    /* Set priority of all interrupts*/

    /*
     * In bootloader we dont care about priority so
     * setting up equal priorities for all
     */
    for (i=0; i < num_irq; i += 4)
        HwReg(gTouchPadqGICDistributerBase + GIC_DIST_PRI + i * 4 / 4) = 0xa0a0a0a0;

    /* Disabling interrupts*/
    for (i=0; i < num_irq; i += 32)
        HwReg(gTouchPadqGICDistributerBase + GIC_DIST_ENABLE_CLEAR + i * 4/32) = 0xffffffff;

    HwReg(gTouchPadqGICDistributerBase + GIC_DIST_ENABLE_SET) = 0xffff;

    /*Enabling GIC*/
    HwReg(gTouchPadqGICDistributerBase + GIC_DIST_CTRL) = 0x1;
}

/* Intialize cpu specific controller */
static void qgic_cpu_init(void)
{
    HwReg(gTouchpadqGICCPUBase + GIC_CPU_PRIMASK) = 0xf0;
    HwReg(gTouchpadqGICCPUBase + GIC_CPU_CTRL) = 0x3;
}

void Grouper_interrupt_init(void)
{
    assert(gTouchPadqGICDistributerBase && gTouchpadqGICCPUBase);
    kprintf("qGIC init\n");
    /* Initialize qGIC. */
    qgic_dist_init();
    qgic_cpu_init();
    return;
}

static void fake_interrupt(void) {
    uint32_t reg = GIC_DIST_PENDING_SET + (TEGRA_INT_TIMER1_EXP/32)*4;
    uint32_t bit = 1 << (TEGRA_INT_TIMER1_EXP & 31);
    HwReg(gTouchPadqGICDistributerBase + reg) |= (bit);
}

void Grouper_timebase_init(void)
{

    assert(gTegraTimerBase);

    timer_configure();
int i = 0;

    /*
     * disable timer 
     */
    Grouper_timer_enabled(FALSE);

    /* Unmask interrupt. */
    uint32_t reg = GIC_DIST_ENABLE_SET + (TEGRA_INT_TIMER1_EXP/32)*4;
    uint32_t bit = 1 << (TEGRA_INT_TIMER1_EXP & 31);
    HwReg(gTouchPadqGICDistributerBase + reg) = (bit);
    for (i=0; i < num_irq; i += 32) 
        HwReg(gTouchPadqGICDistributerBase + GIC_DIST_ENABLE_SET + i * 4/32) = 0xffffffff; //Enable ALL the IRQ!
    HwReg(gTegraICTLRBase + 0x24) = 0x1; //Enable the legacy IRQ Path

    /*
     * set timer values and initialize decrementer 
     */
    HARDWARE_REGISTER(gTegraTimerBase + TIMER_PTV) = clock_decrementer | TIMER_MODE_PERIODIC;

    /*
     * enable irqs so we can get ahold of the timer when it decrements 
     */
    ml_set_interrupts_enabled(TRUE);

    /*
     * re-enable timer 
     */
    Grouper_timer_enabled(TRUE);

    clock_initialized = TRUE;
    kprintf("Set PTV to %x\n", HARDWARE_REGISTER(gTegraTimerBase + TIMER_PTV));
    int timeval;
    while (!clock_had_irq) {
        barrier();
    }

    kprintf(KPRINTF_PREFIX "Grouper Timer initialized, Timer value %llu\n", Grouper_timer_value());
while(1){};
    return;
}

void Grouper_handle_interrupt(void *context)
{
    uint32_t irq_no = HwReg(gTouchpadqGICCPUBase + GIC_CPU_INTACK);
kprintf("Interrupt %x\n", irq_no);
    /*if(irq_no > NR_IRQS) {
        kprintf(KPRINTF_PREFIX "Got a bogus IRQ?");
        return;
    }*/

    /* Timer interrupt? */
    if(irq_no = 0x1f||irq_no == TEGRA_INT_TIMER1_EXP) {
        kprintf("%x\n", HwReg(gTegraICTLRBase + 0));
        Grouper_timer_enabled(FALSE);
        HARDWARE_REGISTER(gTegraTimerBase + TIMER_PCR) = (1 << 30); //ack the timer interrupt
        clock_absolute_time += (clock_decrementer - (int64_t) Grouper_timer_value());
        rtclock_intr((arm_saved_state_t *) context);
    //HARDWARE_REGISTER(gTegraTimerBase + TIMER_PTV) = clock_decrementer | TIMER_MODE_PERIODIC;
        Grouper_timer_enabled(TRUE);
        clock_had_irq = TRUE;

    } else {
        irq_iokit_dispatch(irq_no);
    }

    /* EOI. */
    HwReg(gTouchpadqGICCPUBase + GIC_CPU_EOI) = irq_no;
kprintf("EOI\n");
    return;
}
/*    arm_saved_state_t *regs = (arm_saved_state_t *) context;
    uint32_t ack;

    /*
     * Acknowledge interrupt 
     */
    //ack = HARDWARE_REGISTER(gTegraPicBase + 0xC);

    /*
     * Update absolute time 
     */
    //Grouper_timer_enabled(FALSE);
    //clock_absolute_time += (clock_decrementer - Grouper_timer_value());

    /*
     * Kill the timer 
     */
    //HARDWARE_REGISTER(gTegraTimerBase + TIMER_INTCLR) = 1;
    //rtclock_intr((arm_saved_state_t *) context);

    /*
     * Restart timer. 
     */
    //HARDWARE_REGISTER(gTegraTimerBase) = clock_decrementer;
    //Grouper_timer_enabled(TRUE);

    //clock_had_irq = TRUE;

    /*
     * EOI. 
     */
    //HARDWARE_REGISTER(gTegraPicBase + 0x10) = ack;

    //return;
//}

uint64_t Grouper_get_timebase(void)
{
return 0;
    uint32_t timestamp;

    if (!clock_initialized)
        return 0;

    timestamp = Grouper_timer_value();
    if (timestamp) {
        uint64_t v = clock_absolute_time;
        v += (uint64_t) (((uint64_t) clock_decrementer) - (uint64_t) (timestamp));
        return v;
    } else {
        HARDWARE_REGISTER(gTegraTimerBase) = clock_decrementer;
        Grouper_timer_enabled(TRUE);
        clock_absolute_time += clock_decrementer;

        return clock_absolute_time;
    }
}

uint64_t Grouper_timer_value(void)
{
return 0;
    //return (HARDWARE_REGISTER(gTegraTimerBase + TIMER_PCR)) & 0x1fffffff;
}

void Grouper_timer_enabled(int enable)
{
    if (enable)
        HARDWARE_REGISTER(gTegraTimerBase + TIMER_PTV) |= TIMER_SET_ENABLE;
    else
        HARDWARE_REGISTER(gTegraTimerBase + TIMER_PTV) &= ~TIMER_SET_ENABLE;
}

/*
 * Stub for printing out to framebuffer.
 */
void vcputc(__unused int l, __unused int u, int c);

static void _fb_putc(int c)
{
    if (c == '\n') {
        _fb_putc('\r');
    }
    vcputc(0, 0, c);
    Grouper_putc(c);
}

void Grouper_framebuffer_init(void)
{

    uint32_t lcd_width, lcd_height;

    lcd_width = 800;
    lcd_height = 1280;

    //TODO: Generic framebuffer init for other Tegra devices
    PE_state.video.v_baseAddr = (unsigned long) 0xabe01000;
    PE_state.video.v_rowBytes = lcd_width * 4;
    PE_state.video.v_width = lcd_width;
    PE_state.video.v_height = lcd_height;
    PE_state.video.v_depth = 4 * (8);   // 32bpp

    kprintf(KPRINTF_PREFIX "framebuffer initialized\n");

    char tempbuf[16];
    
    //if (PE_parse_boot_argn("-graphics-mode", tempbuf, sizeof(tempbuf))) {
        /*
         * BootX like framebuffer. 
         */
        //memset((void*) 0xabe01000, 0xb9, PE_state.video.v_rowBytes * PE_state.video.v_height);
        //initialize_screen((void *) &PE_state.video, kPEGraphicsMode);
    //} else {
        initialize_screen((void *) &PE_state.video, kPETextMode);
    //}
}

void PE_init_SocSupport_grouper(void)
{
    gPESocDispatch.uart_getc = Grouper_getc;
    gPESocDispatch.uart_putc = Grouper_putc;
    gPESocDispatch.uart_init = Grouper_uart_init;

    gPESocDispatch.interrupt_init = Grouper_interrupt_init;
    gPESocDispatch.timebase_init = Grouper_timebase_init;

    gPESocDispatch.get_timebase = Grouper_get_timebase;

    gPESocDispatch.handle_interrupt = Grouper_handle_interrupt;

    gPESocDispatch.timer_value = Grouper_timer_value;
    gPESocDispatch.timer_enabled = Grouper_timer_enabled;

    gPESocDispatch.framebuffer_init = Grouper_framebuffer_init;

    Grouper_uart_init();
    Grouper_framebuffer_init();

}

void PE_init_SocSupport_stub(void)
{
    PE_early_puts("PE_init_SocSupport: Initializing for Grouper\n");
    PE_init_SocSupport_grouper();
}

#endif                          // BOARD_CONFIG_TEGRA30_GROUPER
