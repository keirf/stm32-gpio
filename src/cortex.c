/*
 * cortex.c
 * 
 * STM32 ARM Cortex management.
 * 
 * Written & released by Keir Fraser <keir.xen@gmail.com>
 * 
 * This is free and unencumbered software released into the public domain.
 * See the file COPYING for more details, or visit <http://unlicense.org>.
 */

struct extra_exception_frame {
    uint32_t r4, r5, r6, r7, r8, r9, r10, r11, lr;
};

void EXC_unexpected(struct extra_exception_frame *extra)
{
    struct exception_frame *frame;
    uint8_t exc = (uint8_t)read_special(psr);
    uint32_t msp, psp;

    if (extra->lr & 4) {
        frame = (struct exception_frame *)read_special(psp);
        psp = (uint32_t)(frame + 1);
        msp = (uint32_t)(extra + 1);
    } else {
        frame = (struct exception_frame *)(extra + 1);
        psp = read_special(psp);
        msp = (uint32_t)(frame + 1);
    }

    printk("Unexpected %s #%u at PC=%08x (%s):\n",
           (exc < 16) ? "Exception" : "IRQ",
           (exc < 16) ? exc : exc - 16,
           frame->pc, (extra->lr & 8) ? "Thread" : "Handler");
    printk(" r0:  %08x   r1:  %08x   r2:  %08x   r3:  %08x\n",
           frame->r0, frame->r1, frame->r2, frame->r3);
    printk(" r4:  %08x   r5:  %08x   r6:  %08x   r7:  %08x\n",
           extra->r4, extra->r5, extra->r6, extra->r7);
    printk(" r8:  %08x   r9:  %08x   r10: %08x   r11: %08x\n",
           extra->r8, extra->r9, extra->r10, extra->r11);
    printk(" r12: %08x   sp:  %08x   lr:  %08x   pc:  %08x\n",
           frame->r12, (extra->lr & 4) ? psp : msp, frame->lr, frame->pc);
    printk(" msp: %08x   psp: %08x   psr: %08x\n",
           msp, psp, frame->psr);

    system_reset();
}

static void exception_init(void)
{
    /* Initialise and switch to Process SP. Explicit asm as must be
     * atomic wrt updates to SP. We can't guarantee that in C. */
    asm volatile (
        "    mrs  r1,msp     \n"
        "    msr  psp,r1     \n" /* Set up Process SP    */
        "    movs r1,%0      \n"
        "    msr  control,r1 \n" /* Switch to Process SP */
        "    isb             \n" /* Flush the pipeline   */
        :: "i" (CONTROL_SPSEL) : "r1" );

    /* Set up Main SP for IRQ/Exception context. */
    write_special(msp, _irq_stacktop);

    /* Initialise interrupts and exceptions. */
    scb->vtor = (uint32_t)(unsigned long)vector_table;
    scb->ccr |= SCB_CCR_STKALIGN | SCB_CCR_DIV_0_TRP;
    /* GCC inlines memcpy() using full-word load/store regardless of buffer
     * alignment. Hence it is unsafe to trap on unaligned accesses. */
    /*scb->ccr |= SCB_CCR_UNALIGN_TRP;*/
    scb->shcsr |= (SCB_SHCSR_USGFAULTENA |
                   SCB_SHCSR_BUSFAULTENA |
                   SCB_SHCSR_MEMFAULTENA);

    /* SVCall/PendSV exceptions have lowest priority. */
    scb->shpr2 = 0xff<<24;
    scb->shpr3 = 0xff<<16;
}

static void sysclk_init(void)
{
    /* Enable SysTick counter. */
    stk->load = STK_MASK;
    stk->ctrl = STK_CTRL_ENABLE;
}

void cortex_init(void)
{
    exception_init();
    cpu_sync();
    sysclk_init();
}

void delay_ticks(unsigned int ticks)
{
    unsigned int diff, cur, prev = stk->val;

    for (;;) {
        cur = stk->val;
        diff = (prev - cur) & STK_MASK;
        if (ticks <= diff)
            break;
        ticks -= diff;
        prev = cur;
    }
}

void delay_ns(unsigned int ns)
{
    delay_ticks((ns * STK_MHZ) / 1000u);
}

void delay_us(unsigned int us)
{
    delay_ticks(us * STK_MHZ);
}

void delay_ms(unsigned int ms)
{
    delay_ticks(ms * 1000u * STK_MHZ);
}

void system_reset(void)
{
    IRQ_global_disable();
    printk("Resetting...\n");
    /* Request reset and loop waiting for it to happen. */
    cpu_sync();
    scb->aircr = SCB_AIRCR_VECTKEY | SCB_AIRCR_SYSRESETREQ;
    for (;;) ;
}

#if defined(CORTEX_M7)

void icache_invalidate_all(void)
{
    cpu_sync(); 
    cache->iciallu = 0;
    cpu_sync(); 
}

void icache_enable(void)
{
    icache_invalidate_all();
    scb->ccr |= SCB_CCR_IC;
    cpu_sync(); 
}

static void _dcache_op_all(volatile uint32_t *opreg)
{
    uint32_t ccsidr;
    unsigned int sets, ways;

    cpufeat->csselr = 0; /* L1 DCache */
    cpu_sync();
    ccsidr = cpufeat->ccsidr;
    sets = CCSIDR_SETS(ccsidr);
    do {
        ways = CCSIDR_WAYS(ccsidr);
        do {
            *opreg = DCISW_SET(sets) | DCISW_WAY(ways);
        } while (ways--);
    } while (sets--);
    cpu_sync();
}

void dcache_invalidate_all(void)
{
    _dcache_op_all(&cache->dcisw);
}

void dcache_clear_and_invalidate_all(void)
{
    _dcache_op_all(&cache->dccisw);
}

void dcache_enable(void)
{
    dcache_invalidate_all();
    scb->ccr |= SCB_CCR_DC;
    cpu_sync();
}

void dcache_disable(void)
{
    scb->ccr &= ~SCB_CCR_DC;
    cpu_sync();
    dcache_clear_and_invalidate_all();
}

#endif

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
