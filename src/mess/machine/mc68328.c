/**********************************************************************

    Motorola 68328 ("DragonBall") System-on-a-Chip implementation

    By MooglyGuy
    contact mooglyguy@gmail.com with licensing and usage questions.

**********************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/mc68328.h"
#include "mc68328.h"

#define VERBOSE_LEVEL   (0)

INLINE void verboselog(running_machine &machine, int n_level, const char *s_fmt, ...)
{
    if( VERBOSE_LEVEL >= n_level )
    {
        va_list v;
        char buf[ 32768 ];
        va_start( v, s_fmt );
        vsprintf( buf, s_fmt, v );
        va_end( v );
        logerror( "%s: %s", machine.describe_context(), buf );
    }
}

static void mc68328_set_interrupt_line(device_t *device, UINT32 line, UINT32 active)
{
    mc68328_t* mc68328 = mc68328_get_safe_token( device );
    device_t *cpu = device->machine().device(mc68328->iface->m68k_cpu_tag);

    if(active)
    {
        mc68328->regs.ipr |= line;

        if(!(mc68328->regs.imr & line) && !(mc68328->regs.isr & line))
        {
            mc68328->regs.isr |= line;

            if(mc68328->regs.isr & INT_M68K_LINE7)
            {
                device_set_input_line_and_vector(cpu, M68K_IRQ_7, ASSERT_LINE, mc68328->regs.ivr | 0x07);
            }
            else if(mc68328->regs.isr & INT_M68K_LINE6)
            {
                device_set_input_line_and_vector(cpu, M68K_IRQ_6, ASSERT_LINE, mc68328->regs.ivr | 0x06);
            }
            else if(mc68328->regs.isr & INT_M68K_LINE5)
            {
                device_set_input_line_and_vector(cpu, M68K_IRQ_5, ASSERT_LINE, mc68328->regs.ivr | 0x05);
            }
            else if(mc68328->regs.isr & INT_M68K_LINE4)
            {
                device_set_input_line_and_vector(cpu, M68K_IRQ_4, ASSERT_LINE, mc68328->regs.ivr | 0x04);
            }
            else if(mc68328->regs.isr & INT_M68K_LINE3)
            {
                device_set_input_line_and_vector(cpu, M68K_IRQ_3, ASSERT_LINE, mc68328->regs.ivr | 0x03);
            }
            else if(mc68328->regs.isr & INT_M68K_LINE2)
            {
                device_set_input_line_and_vector(cpu, M68K_IRQ_2, ASSERT_LINE, mc68328->regs.ivr | 0x02);
            }
            else if(mc68328->regs.isr & INT_M68K_LINE1)
            {
                device_set_input_line_and_vector(cpu, M68K_IRQ_1, ASSERT_LINE, mc68328->regs.ivr | 0x01);
            }
        }
    }
    else
    {
        mc68328->regs.isr &= ~line;

        if((line & INT_M68K_LINE7) && !(mc68328->regs.isr & INT_M68K_LINE7))
        {
            device_set_input_line(cpu, M68K_IRQ_7, CLEAR_LINE);
        }
        if((line & INT_M68K_LINE6) && !(mc68328->regs.isr & INT_M68K_LINE6))
        {
            device_set_input_line(cpu, M68K_IRQ_6, CLEAR_LINE);
        }
        if((line & INT_M68K_LINE5) && !(mc68328->regs.isr & INT_M68K_LINE5))
        {
            device_set_input_line(cpu, M68K_IRQ_5, CLEAR_LINE);
        }
        if((line & INT_M68K_LINE4) && !(mc68328->regs.isr & INT_M68K_LINE4))
        {
            device_set_input_line(cpu, M68K_IRQ_4, CLEAR_LINE);
        }
        if((line & INT_M68K_LINE3) && !(mc68328->regs.isr & INT_M68K_LINE3))
        {
            device_set_input_line(cpu, M68K_IRQ_3, CLEAR_LINE);
        }
        if((line & INT_M68K_LINE2) && !(mc68328->regs.isr & INT_M68K_LINE2))
        {
            device_set_input_line(cpu, M68K_IRQ_2, CLEAR_LINE);
        }
        if((line & INT_M68K_LINE1) && !(mc68328->regs.isr & INT_M68K_LINE1))
        {
            device_set_input_line(cpu, M68K_IRQ_1, CLEAR_LINE);
        }
    }
}

static void mc68328_poll_port_d_interrupts(device_t *device)
{
    mc68328_t* mc68328 = mc68328_get_safe_token( device );
    UINT8 line_transitions = mc68328->regs.pddataedge & mc68328->regs.pdirqedge;
    UINT8 line_holds = mc68328->regs.pddata &~ mc68328->regs.pdirqedge;
    UINT8 line_interrupts = (line_transitions | line_holds) & mc68328->regs.pdirqen;

    if(line_interrupts)
    {
        mc68328_set_interrupt_line(device, line_interrupts << 8, 1);
    }
    else
    {
        mc68328_set_interrupt_line(device, INT_KBDINTS, 0);
    }
}

void mc68328_set_penirq_line(device_t *device, int state)
{
    mc68328_t* mc68328 = mc68328_get_safe_token( device );

    if(state)
    {
        mc68328_set_interrupt_line(device, INT_PEN, 1);
    }
    else
    {
        mc68328->regs.ipr &= ~INT_PEN;
        mc68328_set_interrupt_line(device, INT_PEN, 0);
    }
}

void mc68328_set_port_d_lines(device_t *device, UINT8 state, int bit)
{
    mc68328_t* mc68328 = mc68328_get_safe_token( device );
    UINT8 old_button_state = mc68328->regs.pddata;

    if(state & (1 << bit))
    {
        mc68328->regs.pddata |= (1 << bit);
    }
    else
    {
        mc68328->regs.pddata &= ~(1 << bit);
    }

    mc68328->regs.pddataedge |= ~old_button_state & mc68328->regs.pddata;

    mc68328_poll_port_d_interrupts(device);
}

static UINT32 mc68328_get_timer_frequency(device_t *device, UINT32 index)
{
    mc68328_t* mc68328 = mc68328_get_safe_token( device );
    UINT32 frequency = 0;

    switch(mc68328->regs.tctl[index] & TCTL_CLKSOURCE)
    {
        case TCTL_CLKSOURCE_SYSCLK:
            frequency = 32768 * 506;
            break;

        case TCTL_CLKSOURCE_SYSCLK16:
            frequency = (32768 * 506) / 16;
            break;

        case TCTL_CLKSOURCE_32KHZ4:
        case TCTL_CLKSOURCE_32KHZ5:
        case TCTL_CLKSOURCE_32KHZ6:
        case TCTL_CLKSOURCE_32KHZ7:
            frequency = 32768;
            break;
    }
    frequency /= (mc68328->regs.tprer[index] + 1);

    return frequency;
}

static void mc68328_maybe_start_timer(device_t *device, UINT32 index, UINT32 new_enable)
{
    mc68328_t* mc68328 = mc68328_get_safe_token( device );

    if((mc68328->regs.tctl[index] & TCTL_TEN) == TCTL_TEN_ENABLE && (mc68328->regs.tctl[index] & TCTL_CLKSOURCE) > TCTL_CLKSOURCE_STOP)
    {
        if((mc68328->regs.tctl[index] & TCTL_CLKSOURCE) == TCTL_CLKSOURCE_TIN)
        {
            mc68328->gptimer[index]->adjust(attotime::never);
        }
        else if(mc68328->regs.tcmp[index] == 0)
        {
            mc68328->gptimer[index]->adjust(attotime::never);
        }
        else
        {
            UINT32 frequency = mc68328_get_timer_frequency(device, index);
            attotime period = (attotime::from_hz(frequency) *  mc68328->regs.tcmp[index]);

            if(new_enable)
            {
                mc68328->regs.tcn[index] = 0x0000;
            }

            mc68328->gptimer[index]->adjust(period);
        }
    }
    else
    {
        mc68328->gptimer[index]->adjust(attotime::never);
    }
}

static void mc68328_timer_compare_event(device_t *device, UINT32 index)
{
    mc68328_t* mc68328 = mc68328_get_safe_token( device );

    mc68328->regs.tcn[index] = mc68328->regs.tcmp[index];
    mc68328->regs.tstat[index] |= TSTAT_COMP;

    if((mc68328->regs.tctl[index] & TCTL_FRR) == TCTL_FRR_RESTART)
    {
        UINT32 frequency = mc68328_get_timer_frequency(device, index);

        if(frequency > 0)
        {
            attotime period = attotime::from_hz(frequency) * mc68328->regs.tcmp[index];

            mc68328->regs.tcn[index] = 0x0000;

            mc68328->gptimer[index]->adjust(period);
        }
        else
        {
            mc68328->gptimer[index]->adjust(attotime::never);
        }
    }
    else
    {
        UINT32 frequency = mc68328_get_timer_frequency(device, index);

        if(frequency > 0)
        {
            attotime period = attotime::from_hz(frequency) * 0x10000;

            mc68328->gptimer[index]->adjust(period);
        }
        else
        {
            mc68328->gptimer[index]->adjust(attotime::never);
        }
    }
    if((mc68328->regs.tctl[index] & TCTL_IRQEN) == TCTL_IRQEN_ENABLE)
    {
        mc68328_set_interrupt_line(device, (index == 0) ? INT_TIMER1 : INT_TIMER2, 1);
    }
}

static TIMER_CALLBACK( mc68328_timer1_hit )
{
    device_t *device = machine.device(MC68328_TAG);

    mc68328_timer_compare_event(device, 0);
}

static TIMER_CALLBACK( mc68328_timer2_hit )
{
    device_t *device = machine.device(MC68328_TAG);

    mc68328_timer_compare_event(device, 1);
}

static TIMER_CALLBACK( mc68328_pwm_transition )
{
    device_t *device = machine.device(MC68328_TAG);
    mc68328_t* mc68328 = mc68328_get_safe_token( device );

    if(mc68328->regs.pwmw >= mc68328->regs.pwmp || mc68328->regs.pwmw == 0 || mc68328->regs.pwmp == 0)
    {
        mc68328->pwm->adjust(attotime::never);
        return;
    }

    if(((mc68328->regs.pwmc & PWMC_POL) == 0 && (mc68328->regs.pwmc & PWMC_PIN) != 0) ||
       ((mc68328->regs.pwmc & PWMC_POL) != 0 && (mc68328->regs.pwmc & PWMC_PIN) == 0))
    {
        UINT32 frequency = 32768 * 506;
        UINT32 divisor = 4 << (mc68328->regs.pwmc & PWMC_CLKSEL); // ?? Datasheet says 2 <<, but then we're an octave higher than CoPilot.
        attotime period;

        frequency /= divisor;
        period = attotime::from_hz(frequency) * (mc68328->regs.pwmp - mc68328->regs.pwmw);

        mc68328->pwm->adjust(period);

        if(mc68328->regs.pwmc & PWMC_IRQEN)
        {
            mc68328_set_interrupt_line(device, INT_PWM, 1);
        }
    }
    else
    {
        UINT32 frequency = 32768 * 506;
        UINT32 divisor = 4 << (mc68328->regs.pwmc & PWMC_CLKSEL); // ?? Datasheet says 2 <<, but then we're an octave higher than CoPilot.
        attotime period;

        frequency /= divisor;
        period = attotime::from_hz(frequency) * mc68328->regs.pwmw;

        mc68328->pwm->adjust(period);
    }

    mc68328->regs.pwmc ^= PWMC_PIN;

    if( mc68328->iface->out_pwm_func )
    {
        (mc68328->iface->out_pwm_func)( device, 0, (mc68328->regs.pwmc & PWMC_PIN) ? 1 : 0 );
    }
}

static TIMER_CALLBACK( mc68328_rtc_tick )
{
    device_t *device = machine.device(MC68328_TAG);
    mc68328_t* mc68328 = mc68328_get_safe_token( device );

    if(mc68328->regs.rtcctl & RTCCTL_ENABLE)
    {
        UINT32 set_int = 0;

        mc68328->regs.hmsr++;

        if(mc68328->regs.rtcienr & RTCINT_SECOND)
        {
            set_int = 1;
            mc68328->regs.rtcisr |= RTCINT_SECOND;
        }

        if((mc68328->regs.hmsr & 0x0000003f) == 0x0000003c)
        {
            mc68328->regs.hmsr &= 0xffffffc0;
            mc68328->regs.hmsr += 0x00010000;

            if(mc68328->regs.rtcienr & RTCINT_MINUTE)
            {
                set_int = 1;
                mc68328->regs.rtcisr |= RTCINT_MINUTE;
            }

            if((mc68328->regs.hmsr & 0x003f0000) == 0x003c0000)
            {
                mc68328->regs.hmsr &= 0xffc0ffff;
                mc68328->regs.hmsr += 0x0100000;

                if((mc68328->regs.hmsr & 0x1f000000) == 0x18000000)
                {
                    mc68328->regs.hmsr &= 0xe0ffffff;

                    if(mc68328->regs.rtcienr & RTCINT_DAY)
                    {
                        set_int = 1;
                        mc68328->regs.rtcisr |= RTCINT_DAY;
                    }
                }
            }

            if(mc68328->regs.stpwtch != 0x003f)
            {
                mc68328->regs.stpwtch--;
                mc68328->regs.stpwtch &= 0x003f;

                if(mc68328->regs.stpwtch == 0x003f)
                {
                    if(mc68328->regs.rtcienr & RTCINT_STOPWATCH)
                    {
                        set_int = 1;
                        mc68328->regs.rtcisr |= RTCINT_STOPWATCH;
                    }
                }
            }
        }

        if(mc68328->regs.hmsr == mc68328->regs.alarm)
        {
            if(mc68328->regs.rtcienr & RTCINT_ALARM)
            {
                set_int = 1;
                mc68328->regs.rtcisr |= RTCINT_STOPWATCH;
            }
        }

        if(set_int)
        {
            mc68328_set_interrupt_line(device, INT_RTC, 1);
        }
        else
        {
            mc68328_set_interrupt_line(device, INT_RTC, 0);
        }
    }
}

WRITE16_DEVICE_HANDLER( mc68328_w )
{
    mc68328_t* mc68328 = mc68328_get_safe_token( device );
    UINT32 address = offset << 1;
    UINT16 temp16[4] = { 0 };
    UINT32 imr_old = mc68328->regs.imr, imr_diff;

    switch(address)
    {
        case 0x000:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: Unknown address (0xfff001) = %02x\n", data & 0x00ff);
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: SCR = %02x\n", (data >> 8) & 0x00ff);
            }
            break;

        case 0x100:
            verboselog(device->machine(), 2, "mc68328_w: GRPBASEA = %04x\n", data);
            mc68328->regs.grpbasea = data;
            break;

        case 0x102:
            verboselog(device->machine(), 2, "mc68328_w: GRPBASEB = %04x\n", data);
            mc68328->regs.grpbaseb = data;
            break;

        case 0x104:
            verboselog(device->machine(), 2, "mc68328_w: GRPBASEC = %04x\n", data);
            mc68328->regs.grpbasec = data;
            break;

        case 0x106:
            verboselog(device->machine(), 2, "mc68328_w: GRPBASED = %04x\n", data);
            mc68328->regs.grpbased = data;
            break;

        case 0x108:
            verboselog(device->machine(), 2, "mc68328_w: GRPMASKA = %04x\n", data);
            mc68328->regs.grpmaska = data;
            break;

        case 0x10a:
            verboselog(device->machine(), 2, "mc68328_w: GRPMASKB = %04x\n", data);
            mc68328->regs.grpmaskb = data;
            break;

        case 0x10c:
            verboselog(device->machine(), 2, "mc68328_w: GRPMASKC = %04x\n", data);
            mc68328->regs.grpmaskc = data;
            break;

        case 0x10e:
            verboselog(device->machine(), 2, "mc68328_w: GRPMASKD = %04x\n", data);
            mc68328->regs.grpmaskd = data;
            break;

        case 0x110:
            verboselog(device->machine(), 5, "mc68328_w: CSA0(0) = %04x\n", data);
            mc68328->regs.csa0 &= 0xffff0000 | (~mem_mask);
            mc68328->regs.csa0 |= data & mem_mask;
            break;

        case 0x112:
            verboselog(device->machine(), 5, "mc68328_w: CSA0(16) = %04x\n", data);
            mc68328->regs.csa0 &= ~(mem_mask << 16);
            mc68328->regs.csa0 |= (data & mem_mask) << 16;
            break;

        case 0x114:
            verboselog(device->machine(), 5, "mc68328_w: CSA1(0) = %04x\n", data);
            mc68328->regs.csa1 &= 0xffff0000 | (~mem_mask);
            mc68328->regs.csa1 |= data & mem_mask;
            break;

        case 0x116:
            verboselog(device->machine(), 5, "mc68328_w: CSA1(16) = %04x\n", data);
            mc68328->regs.csa1 &= ~(mem_mask << 16);
            mc68328->regs.csa1 |= (data & mem_mask) << 16;
            break;

        case 0x118:
            verboselog(device->machine(), 5, "mc68328_w: CSA2(0) = %04x\n", data);
            mc68328->regs.csa2 &= 0xffff0000 | (~mem_mask);
            mc68328->regs.csa2 |= data & mem_mask;
            break;

        case 0x11a:
            verboselog(device->machine(), 5, "mc68328_w: CSA2(16) = %04x\n", data);
            mc68328->regs.csa2 &= ~(mem_mask << 16);
            mc68328->regs.csa2 |= (data & mem_mask) << 16;
            break;

        case 0x11c:
            verboselog(device->machine(), 5, "mc68328_w: CSA3(0) = %04x\n", data);
            mc68328->regs.csa3 &= 0xffff0000 | (~mem_mask);
            mc68328->regs.csa3 |= data & mem_mask;
            break;

        case 0x11e:
            verboselog(device->machine(), 5, "mc68328_w: CSA3(16) = %04x\n", data);
            mc68328->regs.csa3 &= ~(mem_mask << 16);
            mc68328->regs.csa3 |= (data & mem_mask) << 16;
            break;

        case 0x120:
            verboselog(device->machine(), 5, "mc68328_w: CSB0(0) = %04x\n", data);
            mc68328->regs.csb0 &= 0xffff0000 | (~mem_mask);
            mc68328->regs.csb0 |= data & mem_mask;
            break;

        case 0x122:
            verboselog(device->machine(), 5, "mc68328_w: CSB0(16) = %04x\n", data);
            mc68328->regs.csb0 &= ~(mem_mask << 16);
            mc68328->regs.csb0 |= (data & mem_mask) << 16;
            break;

        case 0x124:
            verboselog(device->machine(), 5, "mc68328_w: CSB1(0) = %04x\n", data);
            mc68328->regs.csb1 &= 0xffff0000 | (~mem_mask);
            mc68328->regs.csb1 |= data & mem_mask;
            break;

        case 0x126:
            verboselog(device->machine(), 5, "mc68328_w: CSB1(16) = %04x\n", data);
            mc68328->regs.csb1 &= ~(mem_mask << 16);
            mc68328->regs.csb1 |= (data & mem_mask) << 16;
            break;

        case 0x128:
            verboselog(device->machine(), 5, "mc68328_w: CSB2(0) = %04x\n", data);
            mc68328->regs.csb2 &= 0xffff0000 | (~mem_mask);
            mc68328->regs.csb2 |= data & mem_mask;
            break;

        case 0x12a:
            verboselog(device->machine(), 5, "mc68328_w: CSB2(16) = %04x\n", data);
            mc68328->regs.csb2 &= ~(mem_mask << 16);
            mc68328->regs.csb2 |= (data & mem_mask) << 16;
            break;

        case 0x12c:
            verboselog(device->machine(), 5, "mc68328_w: CSB3(0) = %04x\n", data);
            mc68328->regs.csb3 &= 0xffff0000 | (~mem_mask);
            mc68328->regs.csb3 |= data & mem_mask;
            break;

        case 0x12e:
            verboselog(device->machine(), 5, "mc68328_w: CSB3(16) = %04x\n", data);
            mc68328->regs.csb3 &= ~(mem_mask << 16);
            mc68328->regs.csb3 |= (data & mem_mask) << 16;
            break;

        case 0x130:
            verboselog(device->machine(), 5, "mc68328_w: CSC0(0) = %04x\n", data);
            mc68328->regs.csc0 &= 0xffff0000 | (~mem_mask);
            mc68328->regs.csc0 |= data & mem_mask;
            break;

        case 0x132:
            verboselog(device->machine(), 5, "mc68328_w: CSC0(16) = %04x\n", data);
            mc68328->regs.csc0 &= ~(mem_mask << 16);
            mc68328->regs.csc0 |= (data & mem_mask) << 16;
            break;

        case 0x134:
            verboselog(device->machine(), 5, "mc68328_w: CSC1(0) = %04x\n", data);
            mc68328->regs.csc1 &= 0xffff0000 | (~mem_mask);
            mc68328->regs.csc1 |= data & mem_mask;
            break;

        case 0x136:
            verboselog(device->machine(), 5, "mc68328_w: CSC1(16) = %04x\n", data);
            mc68328->regs.csc1 &= ~(mem_mask << 16);
            mc68328->regs.csc1 |= (data & mem_mask) << 16;
            break;

        case 0x138:
            verboselog(device->machine(), 5, "mc68328_w: CSC2(0) = %04x\n", data);
            mc68328->regs.csc2 &= 0xffff0000 | (~mem_mask);
            mc68328->regs.csc2 |= data & mem_mask;
            break;

        case 0x13a:
            verboselog(device->machine(), 5, "mc68328_w: CSC2(16) = %04x\n", data);
            mc68328->regs.csc2 &= ~(mem_mask << 16);
            mc68328->regs.csc2 |= (data & mem_mask) << 16;
            break;

        case 0x13c:
            verboselog(device->machine(), 5, "mc68328_w: CSC3(0) = %04x\n", data);
            mc68328->regs.csc3 &= 0xffff0000 | (~mem_mask);
            mc68328->regs.csc3 |= data & mem_mask;
            break;

        case 0x13e:
            verboselog(device->machine(), 5, "mc68328_w: CSC3(16) = %04x\n", data);
            mc68328->regs.csc3 &= ~(mem_mask << 16);
            mc68328->regs.csc3 |= (data & mem_mask) << 16;
            break;

        case 0x140:
            verboselog(device->machine(), 5, "mc68328_w: CSD0(0) = %04x\n", data);
            mc68328->regs.csd0 &= 0xffff0000 | (~mem_mask);
            mc68328->regs.csd0 |= data & mem_mask;
            break;

        case 0x142:
            verboselog(device->machine(), 5, "mc68328_w: CSD0(16) = %04x\n", data);
            mc68328->regs.csd0 &= ~(mem_mask << 16);
            mc68328->regs.csd0 |= (data & mem_mask) << 16;
            break;

        case 0x144:
            verboselog(device->machine(), 5, "mc68328_w: CSD1(0) = %04x\n", data);
            mc68328->regs.csd1 &= 0xffff0000 | (~mem_mask);
            mc68328->regs.csd1 |= data & mem_mask;
            break;

        case 0x146:
            verboselog(device->machine(), 5, "mc68328_w: CSD1(16) = %04x\n", data);
            mc68328->regs.csd1 &= ~(mem_mask << 16);
            mc68328->regs.csd1 |= (data & mem_mask) << 16;
            break;

        case 0x148:
            verboselog(device->machine(), 5, "mc68328_w: CSD2(0) = %04x\n", data);
            mc68328->regs.csd2 &= 0xffff0000 | (~mem_mask);
            mc68328->regs.csd2 |= data & mem_mask;
            break;

        case 0x14a:
            verboselog(device->machine(), 5, "mc68328_w: CSD2(16) = %04x\n", data);
            mc68328->regs.csd2 &= ~(mem_mask << 16);
            mc68328->regs.csd2 |= (data & mem_mask) << 16;
            break;

        case 0x14c:
            verboselog(device->machine(), 5, "mc68328_w: CSD3(0) = %04x\n", data);
            mc68328->regs.csd3 &= 0xffff0000 | (~mem_mask);
            mc68328->regs.csd3 |= data & mem_mask;
            break;

        case 0x14e:
            verboselog(device->machine(), 5, "mc68328_w: CSD3(16) = %04x\n", data);
            mc68328->regs.csd3 &= ~(mem_mask << 16);
            mc68328->regs.csd3 |= (data & mem_mask) << 16;
            break;

        case 0x200:
            verboselog(device->machine(), 2, "mc68328_w: PLLCR = %04x\n", data);
            mc68328->regs.pllcr = data;
            break;

        case 0x202:
            verboselog(device->machine(), 2, "mc68328_w: PLLFSR = %04x\n", data);
            mc68328->regs.pllfsr = data;
            break;

        case 0x206:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PCTLR = %02x\n", data & 0x00ff);
                mc68328->regs.pctlr = data & 0x00ff;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: Unknown address (0xfff206) = %02x\n", (data >> 8) & 0x00ff);
            }
            break;

        case 0x300:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: Unknown address (0xfff301) = %02x\n", data & 0x00ff);
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: IVR = %02x\n", (data >> 8) & 0x00ff);
                mc68328->regs.ivr = (data >> 8) & 0x00ff;
            }
            break;

        case 0x302:
            verboselog(device->machine(), 2, "mc68328_w: ICR = %04x\n", data);
            mc68328->regs.icr = data;
            break;

        case 0x304:
            verboselog(device->machine(), 2, "mc68328_w: IMR(16) = %04x\n", data);
            mc68328->regs.imr &= ~(mem_mask << 16);
            mc68328->regs.imr |= (data & mem_mask) << 16;
            mc68328->regs.isr &= ~((data & mem_mask) << 16);

            imr_diff = imr_old ^ mc68328->regs.imr;
            mc68328_set_interrupt_line(device, imr_diff, 0);
            break;

        case 0x306:
            verboselog(device->machine(), 2, "mc68328_w: IMR(0) = %04x\n", data);
            mc68328->regs.imr &= 0xffff0000 | (~mem_mask);
            mc68328->regs.imr |= data & mem_mask;
            mc68328->regs.isr &= ~(data & mem_mask);

            imr_diff = imr_old ^ mc68328->regs.imr;
            mc68328_set_interrupt_line(device, imr_diff, 0);
            break;

        case 0x308:
        {
            verboselog(device->machine(), 2, "mc68328_w: IWR(16) = %04x\n", data);
            mc68328->regs.iwr &= ~(mem_mask << 16);
            mc68328->regs.iwr |= (data & mem_mask) << 16;
        }
        break;

        case 0x30a:
            verboselog(device->machine(), 2, "mc68328_w: IWR(0) = %04x\n", data);
            mc68328->regs.iwr &= 0xffff0000 | (~mem_mask);
            mc68328->regs.iwr |= data & mem_mask;
            break;

        case 0x30c:
            verboselog(device->machine(), 2, "mc68328_w: ISR(16) = %04x\n", data);
            // Clear edge-triggered IRQ1
            if((mc68328->regs.icr & ICR_ET1) == ICR_ET1 && (data & INT_IRQ1_SHIFT) == INT_IRQ1_SHIFT)
            {
                mc68328->regs.isr &= ~INT_IRQ1;
            }

            // Clear edge-triggered IRQ2
            if((mc68328->regs.icr & ICR_ET2) == ICR_ET2 && (data & INT_IRQ2_SHIFT) == INT_IRQ2_SHIFT)
            {
                mc68328->regs.isr &= ~INT_IRQ2;
            }

            // Clear edge-triggered IRQ3
            if((mc68328->regs.icr & ICR_ET3) == ICR_ET3 && (data & INT_IRQ3_SHIFT) == INT_IRQ3_SHIFT)
            {
                mc68328->regs.isr &= ~INT_IRQ3;
            }

            // Clear edge-triggered IRQ6
            if((mc68328->regs.icr & ICR_ET6) == ICR_ET6 && (data & INT_IRQ6_SHIFT) == INT_IRQ6_SHIFT)
            {
                mc68328->regs.isr &= ~INT_IRQ6;
            }

            // Clear edge-triggered IRQ7
            if((data & INT_IRQ7_SHIFT) == INT_IRQ7_SHIFT)
            {
                mc68328->regs.isr &= ~INT_IRQ7;
            }
            break;

        case 0x30e:
            verboselog(device->machine(), 2, "mc68328_w: ISR(0) = %04x (Ignored)\n", data);
            break;

        case 0x310:
            verboselog(device->machine(), 2, "mc68328_w: IPR(16) = %04x (Ignored)\n");
            break;

        case 0x312:
            verboselog(device->machine(), 2, "mc68328_w: IPR(0) = %04x (Ignored)\n");
            break;

        case 0x400:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PADATA = %02x\n", data & 0x00ff);
                mc68328->regs.padata = data & 0x00ff;
                if(mc68328->iface->out_port_a_func)
                {
                    (mc68328->iface->out_port_a_func)( device, 0, data & 0x00ff );
                }
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: PADIR = %02x\n", (data >> 8) & 0x00ff);
                mc68328->regs.padir = (data >> 8) & 0x00ff;
            }
            break;

        case 0x402:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PASEL = %02x\n", data & 0x00ff);
                mc68328->regs.pasel = data & 0x00ff;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: Unknown address (0xfff402) = %02x\n", (data >> 8) & 0x00ff);
            }
            break;

        case 0x408:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PBDATA = %02x\n", data & 0x00ff);
                mc68328->regs.pbdata = data & 0x00ff;
                if(mc68328->iface->out_port_b_func)
                {
                    (mc68328->iface->out_port_b_func)( device, 0, data & 0x00ff );
                }
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: PBDIR = %02x\n", (data >> 8) & 0x00ff);
                mc68328->regs.pbdir = (data >> 8) & 0x00ff;
            }
            break;

        case 0x40a:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PBSEL = %02x\n", data & 0x00ff);
                mc68328->regs.pbsel = data & 0x00ff;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: Unknown address (0xfff40a) = %02x\n", (data >> 8) & 0x00ff);
            }
            break;

        case 0x410:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PCDATA = %02x\n", data & 0x00ff);
                mc68328->regs.pcdata = data & 0x00ff;
                if(mc68328->iface->out_port_c_func)
                {
                    (mc68328->iface->out_port_c_func)( device, 0, data & 0x00ff );
                }
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: PCDIR = %02x\n", (data >> 8) & 0x00ff);
                mc68328->regs.pcdir = (data >> 8) & 0x00ff;
            }
            break;

        case 0x412:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PCSEL = %02x\n", data & 0x00ff);
                mc68328->regs.pcsel = data & 0x00ff;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: Unknown address (0xfff412) = %02x\n", (data >> 8) & 0x00ff);
            }
            break;

        case 0x418:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PDDATA = %02x\n", data & 0x00ff);

                mc68328->regs.pddataedge &= ~(data & 0x00ff);
                mc68328_poll_port_d_interrupts(device);
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: PDDIR = %02x\n", (data >> 8) & 0x00ff);
                mc68328->regs.pddir = (data >> 8) & 0x00ff;
            }
            break;

        case 0x41a:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: Unknown address (0xfff41b) = %02x\n", data & 0x00ff);
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: PDPUEN = %02x\n", (data >> 8) & 0x00ff);
                mc68328->regs.pdpuen = (data >> 8) & 0x00ff;
            }
            break;

        case 0x41c:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PDIRQEN = %02x\n", data & 0x00ff);
                mc68328->regs.pdirqen = data & 0x00ff;

                mc68328_poll_port_d_interrupts(device);
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: PDPOL = %02x\n", (data >> 8) & 0x00ff);
                mc68328->regs.pdpol = (data >> 8) & 0x00ff;
            }
            break;

        case 0x41e:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PDIRQEDGE = %02x\n", data & 0x00ff);
                mc68328->regs.pdirqedge = data & 0x00ff;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: Unknown address (0xfff41e) = %02x\n", (data >> 8) & 0x00ff);
            }
            break;

        case 0x420:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PEDATA = %02x\n", data & 0x00ff);
                mc68328->regs.pedata = data & 0x00ff;
                if(mc68328->iface->out_port_e_func)
                {
                    (mc68328->iface->out_port_e_func)( device, 0, data & 0x00ff );
                }
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: PEDIR = %02x\n", (data >> 8) & 0x00ff);
                mc68328->regs.pedir = (data >> 8) & 0x00ff;
            }
            break;

        case 0x422:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PESEL = %02x\n", data & 0x00ff);
                mc68328->regs.pesel = data & 0x00ff;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: PEPUEN = %02x\n", (data >> 8) & 0x00ff);
                mc68328->regs.pepuen = (data >> 8) & 0x00ff;
                mc68328->regs.pedata |= mc68328->regs.pepuen;
            }
            break;

        case 0x428:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PFDATA = %02x\n", data & 0x00ff);
                mc68328->regs.pfdata = data & 0x00ff;
                if(mc68328->iface->out_port_f_func)
                {
                    (mc68328->iface->out_port_f_func)( device, 0, data & 0x00ff );
                }
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: PFDIR = %02x\n", (data >> 8) & 0x00ff);
                mc68328->regs.pfdir = (data >> 8) & 0x00ff;
            }
            break;

        case 0x42a:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PFSEL = %02x\n", data & 0x00ff);
                mc68328->regs.pfsel = data & 0x00ff;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: PFPUEN = %02x\n", (data >> 8) & 0x00ff);
                mc68328->regs.pfpuen = (data >> 8) & 0x00ff;
            }
            break;

        case 0x430:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PGDATA = %02x\n", data & 0x00ff);
                mc68328->regs.pgdata = data & 0x00ff;
                if(mc68328->iface->out_port_g_func)
                {
                    (mc68328->iface->out_port_g_func)( device, 0, data & 0x00ff );
                }
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: PGDIR = %02x\n", (data >> 8) & 0x00ff);
                mc68328->regs.pgdir = (data >> 8) & 0x00ff;
            }
            break;

        case 0x432:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PGSEL = %02x\n", data & 0x00ff);
                mc68328->regs.pgsel = data & 0x00ff;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: PGPUEN = %02x\n", (data >> 8) & 0x00ff);
                mc68328->regs.pgpuen = (data >> 8) & 0x00ff;
            }
            break;

        case 0x438:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PJDATA = %02x\n", data & 0x00ff);
                mc68328->regs.pjdata = data & 0x00ff;
                if(mc68328->iface->out_port_j_func)
                {
                    (mc68328->iface->out_port_j_func)( device, 0, data & 0x00ff );
                }
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: PJDIR = %02x\n", (data >> 8) & 0x00ff);
                mc68328->regs.pjdir = (data >> 8) & 0x00ff;
            }
            break;

        case 0x43a:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PJSEL = %02x\n", data & 0x00ff);
                mc68328->regs.pjsel = data & 0x00ff;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: Unknown address (0xfff43a) = %02x\n", (data >> 8) & 0x00ff);
            }
            break;

        case 0x440:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PKDATA = %02x\n", data & 0x00ff);
                mc68328->regs.pkdata = data & 0x00ff;
                if(mc68328->iface->out_port_k_func)
                {
                    (mc68328->iface->out_port_k_func)( device, 0, data & 0x00ff );
                }
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: PKDIR = %02x\n", (data >> 8) & 0x00ff);
                mc68328->regs.pkdir = (data >> 8) & 0x00ff;
            }
            break;

        case 0x442:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PKSEL = %02x\n", data & 0x00ff);
                mc68328->regs.pksel = data & 0x00ff;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: PKPUEN = %02x\n", (data >> 8) & 0x00ff);
                mc68328->regs.pgpuen = (data >> 8) & 0x00ff;
            }
            break;

        case 0x448:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PMDATA = %02x\n", data & 0x00ff);
                mc68328->regs.pmdata = data & 0x00ff;
                if(mc68328->iface->out_port_m_func)
                {
                    (mc68328->iface->out_port_m_func)( device, 0, data & 0x00ff );
                }
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: PMDIR = %02x\n", (data >> 8) & 0x00ff);
                mc68328->regs.pmdir = (data >> 8) & 0x00ff;
            }
            break;

        case 0x44a:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: PMSEL = %02x\n", data & 0x00ff);
                mc68328->regs.pmsel = data & 0x00ff;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: PMPUEN = %02x\n", (data >> 8) & 0x00ff);
                mc68328->regs.pmpuen = (data >> 8) & 0x00ff;
            }
            break;

        case 0x500:
            verboselog(device->machine(), 2, "mc68328_w: PWMC = %04x\n", data);

            mc68328->regs.pwmc = data;

            if(mc68328->regs.pwmc & PWMC_PWMIRQ)
            {
                mc68328_set_interrupt_line(device, INT_PWM, 1);
            }

            mc68328->regs.pwmc &= ~PWMC_LOAD;

            if((mc68328->regs.pwmc & PWMC_PWMEN) != 0 && mc68328->regs.pwmw != 0 && mc68328->regs.pwmp != 0)
            {
                UINT32 frequency = 32768 * 506;
                UINT32 divisor = 4 << (mc68328->regs.pwmc & PWMC_CLKSEL); // ?? Datasheet says 2 <<, but then we're an octave higher than CoPilot.
                attotime period;
                frequency /= divisor;
                period = attotime::from_hz(frequency) * mc68328->regs.pwmw;
                mc68328->pwm->adjust(period);
                if(mc68328->regs.pwmc & PWMC_IRQEN)
                {
                    mc68328_set_interrupt_line(device, INT_PWM, 1);
                }
                mc68328->regs.pwmc ^= PWMC_PIN;
            }
            else
            {
                mc68328->pwm->adjust(attotime::never);
            }
            break;

        case 0x502:
            verboselog(device->machine(), 2, "mc68328_w: PWMP = %04x\n", data);
            mc68328->regs.pwmp = data;
            break;

        case 0x504:
            verboselog(device->machine(), 2, "mc68328_w: PWMW = %04x\n", data);
            mc68328->regs.pwmw = data;
            break;

        case 0x506:
            verboselog(device->machine(), 2, "mc68328_w: PWMCNT = %04x\n", data);
            mc68328->regs.pwmcnt = 0;
            break;

        case 0x600:
            verboselog(device->machine(), 2, "mc68328_w: TCTL1 = %04x\n", data);
            temp16[0] = mc68328->regs.tctl[0];
            mc68328->regs.tctl[0] = data;
            if((temp16[0] & TCTL_TEN) == (mc68328->regs.tctl[0] & TCTL_TEN))
            {
                mc68328_maybe_start_timer(device, 0, 0);
            }
            else if((temp16[0] & TCTL_TEN) != TCTL_TEN_ENABLE && (mc68328->regs.tctl[0] & TCTL_TEN) == TCTL_TEN_ENABLE)
            {
                mc68328_maybe_start_timer(device, 0, 1);
            }
            break;

        case 0x602:
            verboselog(device->machine(), 2, "mc68328_w: TPRER1 = %04x\n", data);
            mc68328->regs.tprer[0] = data;
            mc68328_maybe_start_timer(device, 0, 0);
            break;

        case 0x604:
            verboselog(device->machine(), 2, "mc68328_w: TCMP1 = %04x\n", data);
            mc68328->regs.tcmp[0] = data;
            mc68328_maybe_start_timer(device, 0, 0);
            break;

        case 0x606:
            verboselog(device->machine(), 2, "mc68328_w: TCR1 = %04x (Ignored)\n", data);
            break;

        case 0x608:
            verboselog(device->machine(), 2, "mc68328_w: TCN1 = %04x (Ignored)\n", data);
            break;

        case 0x60a:
            verboselog(device->machine(), 5, "mc68328_w: TSTAT1 = %04x\n", data);
            mc68328->regs.tstat[0] &= ~mc68328->regs.tclear[0];
            if(!(mc68328->regs.tstat[0] & TSTAT_COMP))
            {
                mc68328_set_interrupt_line(device, INT_TIMER1, 0);
            }
            break;

        case 0x60c:
            verboselog(device->machine(), 2, "mc68328_w: TCTL2 = %04x\n", data);
            temp16[0] = mc68328->regs.tctl[1];
            mc68328->regs.tctl[1] = data;
            if((temp16[0] & TCTL_TEN) == (mc68328->regs.tctl[1] & TCTL_TEN))
            {
                mc68328_maybe_start_timer(device, 1, 0);
            }
            else if((temp16[0] & TCTL_TEN) != TCTL_TEN_ENABLE && (mc68328->regs.tctl[1] & TCTL_TEN) == TCTL_TEN_ENABLE)
            {
                mc68328_maybe_start_timer(device, 1, 1);
            }
            break;

        case 0x60e:
            verboselog(device->machine(), 2, "mc68328_w: TPRER2 = %04x\n", data);
            mc68328->regs.tprer[1] = data;
            mc68328_maybe_start_timer(device, 1, 0);
            break;

        case 0x610:
            verboselog(device->machine(), 2, "mc68328_w: TCMP2 = %04x\n", data);
            mc68328->regs.tcmp[1] = data;
            mc68328_maybe_start_timer(device, 1, 0);
            break;

        case 0x612:
            verboselog(device->machine(), 2, "mc68328_w: TCR2 = %04x (Ignored)\n", data);
            break;

        case 0x614:
            verboselog(device->machine(), 2, "mc68328_w: TCN2 = %04x (Ignored)\n", data);
            break;

        case 0x616:
            verboselog(device->machine(), 2, "mc68328_w: TSTAT2 = %04x\n", data);
            mc68328->regs.tstat[1] &= ~mc68328->regs.tclear[1];
            if(!(mc68328->regs.tstat[1] & TSTAT_COMP))
            {
                mc68328_set_interrupt_line(device, INT_TIMER2, 0);
            }
            break;

        case 0x618:
            verboselog(device->machine(), 2, "mc68328_w: WCTLR = %04x\n", data);
            mc68328->regs.wctlr = data;
            break;

        case 0x61a:
            verboselog(device->machine(), 2, "mc68328_w: WCMPR = %04x\n", data);
            mc68328->regs.wcmpr = data;
            break;

        case 0x61c:
            verboselog(device->machine(), 2, "mc68328_w: WCN = %04x (Ignored)\n", data);
            break;

        case 0x700:
            verboselog(device->machine(), 2, "mc68328_w: SPISR = %04x\n", data);
            mc68328->regs.spisr = data;
            break;

        case 0x800:
            verboselog(device->machine(), 2, "mc68328_w: SPIMDATA = %04x\n", data);
            if(mc68328->iface->out_spim_func)
            {
                (mc68328->iface->out_spim_func)( device, 0, data, 0xffff );
            }
            else
            {
                mc68328->regs.spimdata = data;
            }
            break;

        case 0x802:
            verboselog(device->machine(), 2, "mc68328_w: SPIMCONT = %04x\n", data);
            verboselog(device->machine(), 3, "           Count = %d\n", data & SPIM_CLOCK_COUNT);
            verboselog(device->machine(), 3, "           Polarity = %s\n", (data & SPIM_POL) ? "Inverted" : "Active-high");
            verboselog(device->machine(), 3, "           Phase = %s\n", (data & SPIM_PHA) ? "Opposite" : "Normal");
            verboselog(device->machine(), 3, "           IRQ Enable = %s\n", (data & SPIM_IRQEN) ? "Enable" : "Disable");
            verboselog(device->machine(), 3, "           IRQ Pending = %s\n", (data & SPIM_SPIMIRQ) ? "Yes" : "No");
            verboselog(device->machine(), 3, "           Exchange = %s\n", (data & SPIM_XCH) ? "Initiate" : "Idle");
            verboselog(device->machine(), 3, "           SPIM Enable = %s\n", (data & SPIM_SPMEN) ? "Enable" : "Disable");
            verboselog(device->machine(), 3, "           Data Rate = Divide By %d\n", 1 << ((((data & SPIM_RATE) >> 13) & 0x0007) + 2) );
            mc68328->regs.spimcont = data;
            // $$HACK$$ We should probably emulate the ADS7843 A/D device properly.
            if(data & SPIM_XCH)
            {
                mc68328->regs.spimcont &= ~SPIM_XCH;
                if(mc68328->iface->spim_xch_trigger)
                {
                    (mc68328->iface->spim_xch_trigger)( device );
                }
                if(data & SPIM_IRQEN)
                {
                    mc68328->regs.spimcont |= SPIM_SPIMIRQ;
                    verboselog(device->machine(), 3, "Triggering SPIM Interrupt\n" );
                    mc68328_set_interrupt_line(device, INT_SPIM, 1);
                }
            }
            if(!(data & SPIM_IRQEN))
            {
                mc68328_set_interrupt_line(device, INT_SPIM, 0);
            }
            break;

        case 0x900:
            verboselog(device->machine(), 2, "mc68328_w: USTCNT = %04x\n", data);
            mc68328->regs.ustcnt = data;
            break;

        case 0x902:
            verboselog(device->machine(), 2, "mc68328_w: UBAUD = %04x\n", data);
            mc68328->regs.ubaud = data;
            break;

        case 0x904:
            verboselog(device->machine(), 2, "mc68328_w: URX = %04x\n", data);
            break;

        case 0x906:
            verboselog(device->machine(), 2, "mc68328_w: UTX = %04x\n", data);
            break;

        case 0x908:
            verboselog(device->machine(), 2, "mc68328_w: UMISC = %04x\n", data);
            mc68328->regs.umisc = data;
            break;

        case 0xa00:
            verboselog(device->machine(), 2, "mc68328_w: LSSA(16) = %04x\n", data);
            mc68328->regs.lssa &= ~(mem_mask << 16);
            mc68328->regs.lssa |= (data & mem_mask) << 16;
            verboselog(device->machine(), 3, "              Address: %08x\n", mc68328->regs.lssa);
            break;

        case 0xa02:
            verboselog(device->machine(), 2, "mc68328_w: LSSA(0) = %04x\n", data);
            mc68328->regs.lssa &= 0xffff0000 | (~mem_mask);
            mc68328->regs.lssa |= data & mem_mask;
            verboselog(device->machine(), 3, "              Address: %08x\n", mc68328->regs.lssa);
            break;

        case 0xa04:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: LVPW = %02x\n", data & 0x00ff);
                mc68328->regs.lvpw = data & 0x00ff;
                verboselog(device->machine(), 3, "              Page Width: %d or %d\n", (mc68328->regs.lvpw + 1) * ((mc68328->regs.lpicf & 0x01) ? 8 : 16));
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: Unknown address (0xfffa04) = %02x\n", (data >> 8) & 0x00ff);
            }
            break;

        case 0xa08:
            verboselog(device->machine(), 2, "mc68328_w: LXMAX = %04x\n", data);
            mc68328->regs.lxmax = data;
            verboselog(device->machine(), 3, "              Width: %d\n", (data & 0x03ff) + 1);
            break;

        case 0xa0a:
            verboselog(device->machine(), 2, "mc68328_w: LYMAX = %04x\n", data);
            mc68328->regs.lymax = data;
            verboselog(device->machine(), 3, "              Height: %d\n", (data & 0x03ff) + 1);
            break;

        case 0xa18:
            verboselog(device->machine(), 2, "mc68328_w: LCXP = %04x\n", data);
            mc68328->regs.lcxp = data;
            verboselog(device->machine(), 3, "              X Position: %d\n", data & 0x03ff);
            switch(mc68328->regs.lcxp >> 14)
            {
                case 0:
                    verboselog(device->machine(), 3, "              Cursor Control: Transparent\n");
                    break;

                case 1:
                    verboselog(device->machine(), 3, "              Cursor Control: Black\n");
                    break;

                case 2:
                    verboselog(device->machine(), 3, "              Cursor Control: Reverse\n");
                    break;

                case 3:
                    verboselog(device->machine(), 3, "              Cursor Control: Invalid\n");
                    break;
            }
            break;

        case 0xa1a:
            verboselog(device->machine(), 2, "mc68328_w: LCYP = %04x\n", data);
            mc68328->regs.lcyp = data;
            verboselog(device->machine(), 3, "              Y Position: %d\n", data & 0x01ff);
            break;

        case 0xa1c:
            verboselog(device->machine(), 2, "mc68328_w: LCWCH = %04x\n", data);
            mc68328->regs.lcwch = data;
            verboselog(device->machine(), 3, "              Width:  %d\n", (data >> 8) & 0x1f);
            verboselog(device->machine(), 3, "              Height: %d\n", data & 0x1f);
            break;

        case 0xa1e:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: LBLKC = %02x\n", data & 0x00ff);
                mc68328->regs.lblkc = data & 0x00ff;
                verboselog(device->machine(), 3, "              Blink Enable:  %d\n", mc68328->regs.lblkc >> 7);
                verboselog(device->machine(), 3, "              Blink Divisor: %d\n", mc68328->regs.lblkc & 0x7f);
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: Unknown address (0xfffa1e) = %02x\n", (data >> 8) & 0x00ff);
            }
            break;

        case 0xa20:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: LPOLCF = %02x\n", data & 0x00ff);
                mc68328->regs.lpolcf = data & 0x00ff;
                verboselog(device->machine(), 3, "              LCD Shift Clock Polarity: %s\n", (mc68328->regs.lpicf & 0x08) ? "Active positive edge of LCLK" : "Active negative edge of LCLK");
                verboselog(device->machine(), 3, "              First-line marker polarity: %s\n", (mc68328->regs.lpicf & 0x04) ? "Active Low" : "Active High");
                verboselog(device->machine(), 3, "              Line-pulse polarity: %s\n", (mc68328->regs.lpicf & 0x02) ? "Active Low" : "Active High");
                verboselog(device->machine(), 3, "              Pixel polarity: %s\n", (mc68328->regs.lpicf & 0x01) ? "Active Low" : "Active High");
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: LPICF = %02x\n", (data >> 8) & 0x00ff);
                mc68328->regs.lpicf = (data >> 8) & 0x00ff;
                switch((mc68328->regs.lpicf >> 1) & 0x03)
                {
                    case 0:
                        verboselog(device->machine(), 3, "              Bus Size: 1-bit\n");
                        break;

                    case 1:
                        verboselog(device->machine(), 3, "              Bus Size: 2-bit\n");
                        break;

                    case 2:
                        verboselog(device->machine(), 3, "              Bus Size: 4-bit\n");
                        break;

                    case 3:
                        verboselog(device->machine(), 3, "              Bus Size: unused\n");
                        break;
                }
                verboselog(device->machine(), 3, "              Gray scale enable: %d\n", mc68328->regs.lpicf & 0x01);
            }
            break;

        case 0xa22:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: LACDRC = %02x\n", data & 0x00ff);
                mc68328->regs.lacdrc = data & 0x00ff;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: Unknown address (0xfffa22) = %02x\n", (data >> 8) & 0x00ff);
            }
            break;

        case 0xa24:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: LPXCD = %02x\n", data & 0x00ff);
                mc68328->regs.lpxcd = data & 0x00ff;
                verboselog(device->machine(), 3, "              Clock Divisor: %d\n", mc68328->regs.lpxcd + 1);
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: Unknown address (0xfffa24) = %02x\n", (data >> 8) & 0x00ff);
            }
            break;

        case 0xa26:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: LCKCON = %02x\n", data & 0x00ff);
                mc68328->regs.lckcon = data & 0x00ff;
                verboselog(device->machine(), 3, "              LCDC Enable: %d\n", (mc68328->regs.lckcon >> 7) & 0x01);
                verboselog(device->machine(), 3, "              DMA Burst Length: %d\n", ((mc68328->regs.lckcon >> 6) & 0x01) ? 16 : 8);
                verboselog(device->machine(), 3, "              DMA Bursting Clock Control: %d\n", ((mc68328->regs.lckcon >> 4) & 0x03) + 1);
                verboselog(device->machine(), 3, "              Bus Width: %d\n", ((mc68328->regs.lckcon >> 1) & 0x01) ? 8 : 16);
                verboselog(device->machine(), 3, "              Pixel Clock Divider Source: %s\n", (mc68328->regs.lckcon & 0x01) ? "PIX" : "SYS");
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: Unknown address (0xfffa26) = %02x\n", (data >> 8) & 0x00ff);
            }
            break;

        case 0xa28:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: LLBAR = %02x\n", data & 0x00ff);
                mc68328->regs.llbar = data & 0x00ff;
                verboselog(device->machine(), 3, "              Address: %d\n", (mc68328->regs.llbar & 0x7f) * ((mc68328->regs.lpicf & 0x01) ? 8 : 16));
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: Unknown address (0xfffa28) = %02x\n", (data >> 8) & 0x00ff);
            }
            break;

        case 0xa2a:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: LOTCR = %02x\n", data & 0x00ff);
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: Unknown address (0xfffa2a) = %02x\n", (data >> 8) & 0x00ff);
            }
            break;

        case 0xa2c:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: LPOSR = %02x\n", data & 0x00ff);
                mc68328->regs.lposr = data & 0x00ff;
                verboselog(device->machine(), 3, "              Byte Offset: %d\n", (mc68328->regs.lposr >> 3) & 0x01);
                verboselog(device->machine(), 3, "              Pixel Offset: %d\n", mc68328->regs.lposr & 0x07);
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: Unknown address (0xfffa2c) = %02x\n", (data >> 8) & 0x00ff);
            }
            break;

        case 0xa30:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_w: LFRCM = %02x\n", data & 0x00ff);
                mc68328->regs.lfrcm = data & 0x00ff;
                verboselog(device->machine(), 3, "              X Modulation: %d\n", (mc68328->regs.lfrcm >> 4) & 0x0f);
                verboselog(device->machine(), 3, "              Y Modulation: %d\n", mc68328->regs.lfrcm & 0x0f);
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_w: Unknown address (0xfffa30) = %02x\n", (data >> 8) & 0x00ff);
            }
            break;

        case 0xa32:
            verboselog(device->machine(), 2, "mc68328_w: LGPMR = %04x\n", data);
            mc68328->regs.lgpmr = data;
            verboselog(device->machine(), 3, "              Palette 0: %d\n", (mc68328->regs.lgpmr >>  8) & 0x07);
            verboselog(device->machine(), 3, "              Palette 1: %d\n", (mc68328->regs.lgpmr >> 12) & 0x07);
            verboselog(device->machine(), 3, "              Palette 2: %d\n", (mc68328->regs.lgpmr >>  0) & 0x07);
            verboselog(device->machine(), 3, "              Palette 3: %d\n", (mc68328->regs.lgpmr >>  4) & 0x07);
            break;

        case 0xb00:
            verboselog(device->machine(), 2, "mc68328_w: HMSR(0) = %04x\n", data);
            mc68328->regs.hmsr &= ~(mem_mask << 16);
            mc68328->regs.hmsr |= (data & mem_mask) << 16;
            mc68328->regs.hmsr &= 0x1f3f003f;
            break;

        case 0xb02:
            verboselog(device->machine(), 2, "mc68328_w: HMSR(16) = %04x\n", data);
            mc68328->regs.hmsr &= 0xffff0000 | (~mem_mask);
            mc68328->regs.hmsr |= data & mem_mask;
            mc68328->regs.hmsr &= 0x1f3f003f;
            break;

        case 0xb04:
            verboselog(device->machine(), 2, "mc68328_w: ALARM(0) = %04x\n", data);
            mc68328->regs.alarm &= ~(mem_mask << 16);
            mc68328->regs.alarm |= (data & mem_mask) << 16;
            mc68328->regs.alarm &= 0x1f3f003f;
            break;

        case 0xb06:
            verboselog(device->machine(), 2, "mc68328_w: ALARM(16) = %04x\n", data);
            mc68328->regs.alarm &= 0xffff0000 | (~mem_mask);
            mc68328->regs.alarm |= data & mem_mask;
            mc68328->regs.alarm &= 0x1f3f003f;
            break;

        case 0xb0c:
            verboselog(device->machine(), 2, "mc68328_w: RTCCTL = %04x\n", data);
            mc68328->regs.rtcctl = data & 0x00a0;
            break;

        case 0xb0e:
            verboselog(device->machine(), 2, "mc68328_w: RTCISR = %04x\n", data);
            mc68328->regs.rtcisr &= ~data;
            if(mc68328->regs.rtcisr == 0)
            {
                mc68328_set_interrupt_line(device, INT_RTC, 0);
            }
            break;

        case 0xb10:
            verboselog(device->machine(), 2, "mc68328_w: RTCIENR = %04x\n", data);
            mc68328->regs.rtcienr = data & 0x001f;
            break;

        case 0xb12:
            verboselog(device->machine(), 2, "mc68328_w: STPWTCH = %04x\n", data);
            mc68328->regs.stpwtch = data & 0x003f;
            break;

        default:
            verboselog(device->machine(), 0, "mc68328_w: Unknown address (0x%06x) = %04x (%04x)\n", 0xfff000 + address, data, mem_mask);
            break;
    }
}

READ16_DEVICE_HANDLER( mc68328_r )
{
    mc68328_t* mc68328 = mc68328_get_safe_token( device );
    UINT16 temp16 = 0;
    UINT32 address = offset << 1;

    switch(address)
    {
        case 0x000:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): Unknown address (0xfff001)\n", mem_mask);
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): SCR = %02x\n", mem_mask, mc68328->regs.scr);
                return mc68328->regs.scr << 8;
            }
            break;

        case 0x100:
            verboselog(device->machine(), 2, "mc68328_r (%04x): GRPBASEA = %04x\n", mem_mask, mc68328->regs.grpbasea);
            return mc68328->regs.grpbasea;

        case 0x102:
            verboselog(device->machine(), 2, "mc68328_r (%04x): GRPBASEB = %04x\n", mem_mask, mc68328->regs.grpbaseb);
            return mc68328->regs.grpbaseb;

        case 0x104:
            verboselog(device->machine(), 2, "mc68328_r (%04x): GRPBASEC = %04x\n", mem_mask, mc68328->regs.grpbasec);
            return mc68328->regs.grpbasec;

        case 0x106:
            verboselog(device->machine(), 2, "mc68328_r (%04x): GRPBASED = %04x\n", mem_mask, mc68328->regs.grpbased);
            return mc68328->regs.grpbased;

        case 0x108:
            verboselog(device->machine(), 2, "mc68328_r (%04x): GRPMASKA = %04x\n", mem_mask, mc68328->regs.grpmaska);
            return mc68328->regs.grpmaska;

        case 0x10a:
            verboselog(device->machine(), 2, "mc68328_r (%04x): GRPMASKB = %04x\n", mem_mask, mc68328->regs.grpmaskb);
            return mc68328->regs.grpmaskb;

        case 0x10c:
            verboselog(device->machine(), 2, "mc68328_r (%04x): GRPMASKC = %04x\n", mem_mask, mc68328->regs.grpmaskc);
            return mc68328->regs.grpmaskc;

        case 0x10e:
            verboselog(device->machine(), 2, "mc68328_r (%04x): GRPMASKD = %04x\n", mem_mask, mc68328->regs.grpmaskd);
            return mc68328->regs.grpmaskd;

        case 0x110:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSA0(0) = %04x\n", mem_mask, mc68328->regs.csa0 & 0x0000ffff);
            return mc68328->regs.csa0 & 0x0000ffff;

        case 0x112:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSA0(16) = %04x\n", mem_mask, mc68328->regs.csa0 >> 16);
            return mc68328->regs.csa0 >> 16;

        case 0x114:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSA1(0) = %04x\n", mem_mask, mc68328->regs.csa1 & 0x0000ffff);
            return mc68328->regs.csa1 & 0x0000ffff;

        case 0x116:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSA1(16) = %04x\n", mem_mask, mc68328->regs.csa1 >> 16);
            return mc68328->regs.csa1 >> 16;

        case 0x118:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSA2(0) = %04x\n", mem_mask, mc68328->regs.csa2 & 0x0000ffff);
            return mc68328->regs.csa2 & 0x0000ffff;

        case 0x11a:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSA2(16) = %04x\n", mem_mask, mc68328->regs.csa2 >> 16);
            return mc68328->regs.csa2 >> 16;

        case 0x11c:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSA3(0) = %04x\n", mem_mask, mc68328->regs.csa3 & 0x0000ffff);
            return mc68328->regs.csa3 & 0x0000ffff;

        case 0x11e:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSA3(16) = %04x\n", mem_mask, mc68328->regs.csa3 >> 16);
            return mc68328->regs.csa3 >> 16;

        case 0x120:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSB0(0) = %04x\n", mem_mask, mc68328->regs.csb0 & 0x0000ffff);
            return mc68328->regs.csb0 & 0x0000ffff;

        case 0x122:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSB0(16) = %04x\n", mem_mask, mc68328->regs.csb0 >> 16);
            return mc68328->regs.csb0 >> 16;

        case 0x124:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSB1(0) = %04x\n", mem_mask, mc68328->regs.csb1 & 0x0000ffff);
            return mc68328->regs.csb1 & 0x0000ffff;

        case 0x126:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSB1(16) = %04x\n", mem_mask, mc68328->regs.csb1 >> 16);
            return mc68328->regs.csb1 >> 16;

        case 0x128:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSB2(0) = %04x\n", mem_mask, mc68328->regs.csb2 & 0x0000ffff);
            return mc68328->regs.csb2 & 0x0000ffff;

        case 0x12a:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSB2(16) = %04x\n", mem_mask, mc68328->regs.csb2 >> 16);
            return mc68328->regs.csb2 >> 16;

        case 0x12c:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSB3(0) = %04x\n", mem_mask, mc68328->regs.csb3 & 0x0000ffff);
            return mc68328->regs.csb3 & 0x0000ffff;

        case 0x12e:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSB3(16) = %04x\n", mem_mask, mc68328->regs.csb3 >> 16);
            return mc68328->regs.csb3 >> 16;

        case 0x130:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSC0(0) = %04x\n", mem_mask, mc68328->regs.csc0 & 0x0000ffff);
            return mc68328->regs.csc0 & 0x0000ffff;

        case 0x132:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSC0(16) = %04x\n", mem_mask, mc68328->regs.csc0 >> 16);
            return mc68328->regs.csc0 >> 16;

        case 0x134:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSC1(0) = %04x\n", mem_mask, mc68328->regs.csc1 & 0x0000ffff);
            return mc68328->regs.csc1 & 0x0000ffff;

        case 0x136:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSC1(16) = %04x\n", mem_mask, mc68328->regs.csc1 >> 16);
            return mc68328->regs.csc1 >> 16;

        case 0x138:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSC2(0) = %04x\n", mem_mask, mc68328->regs.csc2 & 0x0000ffff);
            return mc68328->regs.csc2 & 0x0000ffff;

        case 0x13a:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSC2(16) = %04x\n", mem_mask, mc68328->regs.csc2 >> 16);
            return mc68328->regs.csc2 >> 16;

        case 0x13c:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSC3(0) = %04x\n", mem_mask, mc68328->regs.csc3 & 0x0000ffff);
            return mc68328->regs.csc3 & 0x0000ffff;

        case 0x13e:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSC3(16) = %04x\n", mem_mask, mc68328->regs.csc3 >> 16);
            return mc68328->regs.csc3 >> 16;

        case 0x140:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSD0(0) = %04x\n", mem_mask, mc68328->regs.csd0 & 0x0000ffff);
            return mc68328->regs.csd0 & 0x0000ffff;

        case 0x142:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSD0(16) = %04x\n", mem_mask, mc68328->regs.csd0 >> 16);
            return mc68328->regs.csd0 >> 16;

        case 0x144:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSD1(0) = %04x\n", mem_mask, mc68328->regs.csd1 & 0x0000ffff);
            return mc68328->regs.csd1 & 0x0000ffff;

        case 0x146:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSD1(16) = %04x\n", mem_mask, mc68328->regs.csd1 >> 16);
            return mc68328->regs.csd1 >> 16;

        case 0x148:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSD2(0) = %04x\n", mem_mask, mc68328->regs.csd2 & 0x0000ffff);
            return mc68328->regs.csd2 & 0x0000ffff;

        case 0x14a:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSD2(16) = %04x\n", mem_mask, mc68328->regs.csd2 >> 16);
            return mc68328->regs.csd2 >> 16;

        case 0x14c:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSD3(0) = %04x\n", mem_mask, mc68328->regs.csd3 & 0x0000ffff);
            return mc68328->regs.csd3 & 0x0000ffff;

        case 0x14e:
            verboselog(device->machine(), 5, "mc68328_r (%04x): CSD3(16) = %04x\n", mem_mask, mc68328->regs.csd3 >> 16);
            return mc68328->regs.csd3 >> 16;

        case 0x200:
            verboselog(device->machine(), 2, "mc68328_r (%04x): PLLCR = %04x\n", mem_mask, mc68328->regs.pllcr);
            return mc68328->regs.pllcr;

        case 0x202:
            verboselog(device->machine(), 2, "mc68328_r (%04x): PLLFSR = %04x\n", mem_mask, mc68328->regs.pllfsr);
            mc68328->regs.pllfsr ^= 0x8000;
            return mc68328->regs.pllfsr;

        case 0x206:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): Unknown address (0xfff206)\n", mem_mask);
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PCTLR = %02x\n", mem_mask, mc68328->regs.pctlr);
                return mc68328->regs.pctlr << 8;
            }
            break;

        case 0x300:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): Unknown address (0xfff301)\n", mem_mask);
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): IVR = %02x\n", mem_mask, mc68328->regs.ivr);
                return mc68328->regs.ivr << 8;
            }
            break;

        case 0x302:
            verboselog(device->machine(), 2, "mc68328_r (%04x): ICR = %04x\n", mem_mask, mc68328->regs.icr);
            return mc68328->regs.icr;

        case 0x304:
            verboselog(device->machine(), 2, "mc68328_r (%04x): IMR(16) = %04x\n", mem_mask, mc68328->regs.imr >> 16);
            return mc68328->regs.imr >> 16;

        case 0x306:
            verboselog(device->machine(), 2, "mc68328_r (%04x): IMR(0) = %04x\n", mem_mask, mc68328->regs.imr & 0x0000ffff);
            return mc68328->regs.imr & 0x0000ffff;

        case 0x308:
            verboselog(device->machine(), 2, "mc68328_r (%04x): IWR(16) = %04x\n", mem_mask, mc68328->regs.iwr >> 16);
            return mc68328->regs.iwr >> 16;

        case 0x30a:
            verboselog(device->machine(), 2, "mc68328_r (%04x): IWR(0) = %04x\n", mem_mask, mc68328->regs.iwr & 0x0000ffff);
            return mc68328->regs.iwr & 0x0000ffff;

        case 0x30c:
            verboselog(device->machine(), 2, "mc68328_r (%04x): ISR(16) = %04x\n", mem_mask, mc68328->regs.isr >> 16);
            return mc68328->regs.isr >> 16;

        case 0x30e:
            verboselog(device->machine(), 2, "mc68328_r (%04x): ISR(0) = %04x\n", mem_mask, mc68328->regs.isr & 0x0000ffff);
            return mc68328->regs.isr & 0x0000ffff;

        case 0x310:
            verboselog(device->machine(), 2, "mc68328_r (%04x): IPR(16) = %04x\n", mem_mask, mc68328->regs.ipr >> 16);
            return mc68328->regs.ipr >> 16;

        case 0x312:
            verboselog(device->machine(), 2, "mc68328_r (%04x): IPR(0) = %04x\n", mem_mask, mc68328->regs.ipr & 0x0000ffff);
            return mc68328->regs.ipr & 0x0000ffff;

        case 0x400:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PADATA = %02x\n", mem_mask, mc68328->regs.padata);
                if(mc68328->iface->in_port_a_func)
                {
                    return (mc68328->iface->in_port_a_func)( device, 0 );
                }
                else
                {
                    return mc68328->regs.padata;
                }
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PADIR = %02x\n", mem_mask, mc68328->regs.padir);
                return mc68328->regs.padir << 8;
            }
            break;

        case 0x402:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PASEL = %02x\n", mem_mask, mc68328->regs.pasel);
                return mc68328->regs.pasel;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): Unknown address (0xfff402)\n", mem_mask);
            }
            break;

        case 0x408:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PBDATA = %02x\n", mem_mask, mc68328->regs.pbdata);
                if(mc68328->iface->in_port_b_func)
                {
                    return (mc68328->iface->in_port_b_func)( device, 0 );
                }
                else
                {
                    return mc68328->regs.pbdata;
                }
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PBDIR = %02x\n", mem_mask, mc68328->regs.pbdir);
                return mc68328->regs.pbdir << 8;
            }
            break;

        case 0x40a:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PBSEL = %02x\n", mem_mask, mc68328->regs.pbsel);
                return mc68328->regs.pbsel;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): Unknown address (0xfff40a)\n", mem_mask);
            }
            break;

        case 0x410:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PCDATA = %02x\n", mem_mask, mc68328->regs.pcdata);
                if(mc68328->iface->in_port_c_func)
                {
                    return (mc68328->iface->in_port_c_func)( device, 0 );
                }
                else
                {
                    return mc68328->regs.pcdata;
                }
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PCDIR = %02x\n", mem_mask, mc68328->regs.pcdir);
                return mc68328->regs.pcdir << 8;
            }
            break;

        case 0x412:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PCSEL = %02x\n", mem_mask, mc68328->regs.pcsel);
                return mc68328->regs.pcsel;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): Unknown address (0xfff412)\n", mem_mask);
            }
            break;

        case 0x418:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PDDATA = %02x\n", mem_mask, mc68328->regs.pddata);
                if(mc68328->iface->in_port_d_func)
                {
                    return (mc68328->iface->in_port_d_func)( device, 0 );
                }
                else
                {
                    return mc68328->regs.pddata;
                }
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PDDIR = %02x\n", mem_mask, mc68328->regs.pddir);
                return mc68328->regs.pddir << 8;
            }
            break;

        case 0x41a:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): Unknown address (0xfff41b)\n", mem_mask);
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PDPUEN = %02x\n", mem_mask, mc68328->regs.pdpuen);
                return mc68328->regs.pdpuen << 8;
            }
            break;

        case 0x41c:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PDIRQEN = %02x\n", mem_mask, mc68328->regs.pdirqen);
                return mc68328->regs.pdirqen;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PDPOL = %02x\n", mem_mask, mc68328->regs.pdpol);
                return mc68328->regs.pdpol << 8;
            }
            break;

        case 0x41e:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PDIRQEDGE = %02x\n", mem_mask, mc68328->regs.pdirqedge);
                return mc68328->regs.pdirqedge;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): Unknown address (0xfff41e)\n", mem_mask);
            }
            break;

        case 0x420:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PEDATA = %02x\n", mem_mask, mc68328->regs.pedata);
                if(mc68328->iface->in_port_e_func)
                {
                    return (mc68328->iface->in_port_e_func)( device, 0 );
                }
                else
                {
                    return mc68328->regs.pedata;
                }
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PEDIR = %02x\n", mem_mask, mc68328->regs.pedir);
                return mc68328->regs.pedir << 8;
            }
            break;

        case 0x422:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PESEL = %02x\n", mem_mask, mc68328->regs.pesel);
                return mc68328->regs.pesel;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PEPUEN = %02x\n", mem_mask, mc68328->regs.pepuen);
                return mc68328->regs.pepuen << 8;
            }
            break;

        case 0x428:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PFDATA = %02x\n", mem_mask, mc68328->regs.pfdata);
                if(mc68328->iface->in_port_f_func)
                {
                    return (mc68328->iface->in_port_f_func)( device, 0 );
                }
                else
                {
                    return mc68328->regs.pfdata;
                }
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PFDIR = %02x\n", mem_mask, mc68328->regs.pfdir);
                return mc68328->regs.pfdir << 8;
            }
            break;

        case 0x42a:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PFSEL = %02x\n", mem_mask, mc68328->regs.pfsel);
                return mc68328->regs.pfsel;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PFPUEN = %02x\n", mem_mask, mc68328->regs.pfpuen);
                return mc68328->regs.pfpuen << 8;
            }
            break;

        case 0x430:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PGDATA = %02x\n", mem_mask, mc68328->regs.pgdata);
                if(mc68328->iface->in_port_g_func)
                {
                    return (mc68328->iface->in_port_g_func)( device, 0 );
                }
                else
                {
                    return mc68328->regs.pgdata;
                }
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PGDIR = %02x\n", mem_mask, mc68328->regs.pgdir);
                return mc68328->regs.pgdir << 8;
            }
            break;

        case 0x432:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PGSEL = %02x\n", mem_mask, mc68328->regs.pgsel);
                return mc68328->regs.pgsel;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PGPUEN = %02x\n", mem_mask, mc68328->regs.pgpuen);
                return mc68328->regs.pgpuen << 8;
            }
            break;

        case 0x438:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PJDATA = %02x\n", mem_mask, mc68328->regs.pjdata);
                if(mc68328->iface->in_port_j_func)
                {
                    return (mc68328->iface->in_port_j_func)( device, 0 );
                }
                else
                {
                    return mc68328->regs.pjdata;
                }
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PJDIR = %02x\n", mem_mask, mc68328->regs.pjdir);
                return mc68328->regs.pjdir << 8;
            }
            break;

        case 0x43a:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PJSEL = %02x\n", mem_mask, mc68328->regs.pjsel);
                return mc68328->regs.pjsel;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): Unknown address (0xfff43a)\n", mem_mask);
            }
            break;

        case 0x440:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PKDATA = %02x\n", mem_mask, mc68328->regs.pkdata);
                if(mc68328->iface->in_port_k_func)
                {
                    return (mc68328->iface->in_port_k_func)( device, 0 );
                }
                else
                {
                    return mc68328->regs.pkdata;
                }
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PKDIR = %02x\n", mem_mask, mc68328->regs.pkdir);
                return mc68328->regs.pkdir << 8;
            }
            break;

        case 0x442:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PKSEL = %02x\n", mem_mask, mc68328->regs.pksel);
                return mc68328->regs.pksel;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PKPUEN = %02x\n", mem_mask, mc68328->regs.pkpuen);
                return mc68328->regs.pkpuen << 8;
            }
            break;

        case 0x448:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PMDATA = %02x\n", mem_mask, mc68328->regs.pmdata);
                if(mc68328->iface->in_port_m_func)
                {
                    return (mc68328->iface->in_port_m_func)( device, 0 );
                }
                else
                {
                    return mc68328->regs.pmdata;
                }
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PMDIR = %02x\n", mem_mask, mc68328->regs.pmdir);
                return mc68328->regs.pmdir << 8;
            }
            break;

        case 0x44a:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PMSEL = %02x\n", mem_mask, mc68328->regs.pmsel);
                return mc68328->regs.pmsel;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): PMPUEN = %02x\n", mem_mask, mc68328->regs.pmpuen);
                return mc68328->regs.pmpuen << 8;
            }
            break;

        case 0x500:
            verboselog(device->machine(), 2, "mc68328_r (%04x): PWMC = %04x\n", mem_mask, mc68328->regs.pwmc);
            temp16 = mc68328->regs.pwmc;
            if(mc68328->regs.pwmc & PWMC_PWMIRQ)
            {
                mc68328->regs.pwmc &= ~PWMC_PWMIRQ;
                mc68328_set_interrupt_line(device, INT_PWM, 0);
            }
            return temp16;

        case 0x502:
            verboselog(device->machine(), 2, "mc68328_r (%04x): PWMP = %04x\n", mem_mask, mc68328->regs.pwmp);
            return mc68328->regs.pwmp;

        case 0x504:
            verboselog(device->machine(), 2, "mc68328_r (%04x): PWMW = %04x\n", mem_mask, mc68328->regs.pwmw);
            return mc68328->regs.pwmw;

        case 0x506:
            verboselog(device->machine(), 2, "mc68328_r (%04x): PWMCNT = %04x\n", mem_mask, mc68328->regs.pwmcnt);
            return mc68328->regs.pwmcnt;

        case 0x600:
            verboselog(device->machine(), 2, "mc68328_r (%04x): TCTL1 = %04x\n", mem_mask, mc68328->regs.tctl[0]);
            return mc68328->regs.tctl[0];

        case 0x602:
            verboselog(device->machine(), 2, "mc68328_r (%04x): TPRER1 = %04x\n", mem_mask, mc68328->regs.tprer[0]);
            return mc68328->regs.tprer[0];

        case 0x604:
            verboselog(device->machine(), 2, "mc68328_r (%04x): TCMP1 = %04x\n", mem_mask, mc68328->regs.tcmp[0]);
            return mc68328->regs.tcmp[0];

        case 0x606:
            verboselog(device->machine(), 2, "mc68328_r (%04x): TCR1 = %04x\n", mem_mask, mc68328->regs.tcr[0]);
            return mc68328->regs.tcr[0];

        case 0x608:
            verboselog(device->machine(), 2, "mc68328_r (%04x): TCN1 = %04x\n", mem_mask, mc68328->regs.tcn[0]);
            return mc68328->regs.tcn[0];

        case 0x60a:
            verboselog(device->machine(), 5, "mc68328_r (%04x): TSTAT1 = %04x\n", mem_mask, mc68328->regs.tstat[0]);
            mc68328->regs.tclear[0] |= mc68328->regs.tstat[0];
            return mc68328->regs.tstat[0];

        case 0x60c:
            verboselog(device->machine(), 2, "mc68328_r (%04x): TCTL2 = %04x\n", mem_mask, mc68328->regs.tctl[1]);
            return mc68328->regs.tctl[1];

        case 0x60e:
            verboselog(device->machine(), 2, "mc68328_r (%04x): TPREP2 = %04x\n", mem_mask, mc68328->regs.tprer[1]);
            return mc68328->regs.tprer[1];

        case 0x610:
            verboselog(device->machine(), 2, "mc68328_r (%04x): TCMP2 = %04x\n", mem_mask, mc68328->regs.tcmp[1]);
            return mc68328->regs.tcmp[1];

        case 0x612:
            verboselog(device->machine(), 2, "mc68328_r (%04x): TCR2 = %04x\n", mem_mask, mc68328->regs.tcr[1]);
            return mc68328->regs.tcr[1];

        case 0x614:
            verboselog(device->machine(), 2, "mc68328_r (%04x): TCN2 = %04x\n", mem_mask, mc68328->regs.tcn[1]);
            return mc68328->regs.tcn[1];

        case 0x616:
            verboselog(device->machine(), 2, "mc68328_r (%04x): TSTAT2 = %04x\n", mem_mask, mc68328->regs.tstat[1]);
            mc68328->regs.tclear[1] |= mc68328->regs.tstat[1];
            return mc68328->regs.tstat[1];

        case 0x618:
            verboselog(device->machine(), 2, "mc68328_r (%04x): WCTLR = %04x\n", mem_mask, mc68328->regs.wctlr);
            return mc68328->regs.wctlr;

        case 0x61a:
            verboselog(device->machine(), 2, "mc68328_r (%04x): WCMPR = %04x\n", mem_mask, mc68328->regs.wcmpr);
            return mc68328->regs.wcmpr;

        case 0x61c:
            verboselog(device->machine(), 2, "mc68328_r (%04x): WCN = %04x\n", mem_mask, mc68328->regs.wcn);
            return mc68328->regs.wcn;

        case 0x700:
            verboselog(device->machine(), 2, "mc68328_r (%04x): SPISR = %04x\n", mem_mask, mc68328->regs.spisr);
            return mc68328->regs.spisr;

        case 0x800:
            verboselog(device->machine(), 2, "mc68328_r (%04x): SPIMDATA = %04x\n", mem_mask, mc68328->regs.spimdata);
            if(mc68328->iface->in_spim_func)
            {
                return (mc68328->iface->in_spim_func)( device, 0, 0xffff );
            }
            return mc68328->regs.spimdata;

        case 0x802:
            verboselog(device->machine(), 2, "mc68328_r (%04x): SPIMCONT = %04x\n", mem_mask, mc68328->regs.spimcont);
            if(mc68328->regs.spimcont & SPIM_XCH)
            {
                mc68328->regs.spimcont &= ~SPIM_XCH;
                mc68328->regs.spimcont |= SPIM_SPIMIRQ;
                return ((mc68328->regs.spimcont | SPIM_XCH) &~ SPIM_SPIMIRQ);
            }
            return mc68328->regs.spimcont;

        case 0x900:
            verboselog(device->machine(), 2, "mc68328_r (%04x): USTCNT = %04x\n", mem_mask, mc68328->regs.ustcnt);
            return mc68328->regs.ustcnt;

        case 0x902:
            verboselog(device->machine(), 2, "mc68328_r (%04x): UBAUD = %04x\n", mem_mask, mc68328->regs.ubaud);
            return mc68328->regs.ubaud;

        case 0x904:
            verboselog(device->machine(), 5, "mc68328_r (%04x): URX = %04x\n", mem_mask, mc68328->regs.urx);
            return mc68328->regs.urx;

        case 0x906:
            verboselog(device->machine(), 5, "mc68328_r (%04x): UTX = %04x\n", mem_mask, mc68328->regs.utx);
            return mc68328->regs.utx | UTX_FIFO_EMPTY | UTX_FIFO_HALF | UTX_TX_AVAIL;

        case 0x908:
            verboselog(device->machine(), 2, "mc68328_r (%04x): UMISC = %04x\n", mem_mask, mc68328->regs.umisc);
            return mc68328->regs.umisc;

        case 0xa00:
            verboselog(device->machine(), 2, "mc68328_r (%04x): LSSA(16) = %04x\n", mem_mask, mc68328->regs.lssa >> 16);
            return mc68328->regs.lssa >> 16;

        case 0xa02:
            verboselog(device->machine(), 2, "mc68328_r (%04x): LSSA(0) = %04x\n", mem_mask, mc68328->regs.lssa & 0x0000ffff);
            return mc68328->regs.lssa & 0x0000ffff;

        case 0xa04:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): LVPW = %02x\n", mem_mask, mc68328->regs.lvpw);
                return mc68328->regs.lvpw;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): Unknown address (0xfffa04)\n", mem_mask);
            }
            break;

        case 0xa08:
            verboselog(device->machine(), 2, "mc68328_r (%04x): LXMAX = %04x\n", mem_mask, mc68328->regs.lxmax);
            return mc68328->regs.lxmax;

        case 0xa0a:
            verboselog(device->machine(), 2, "mc68328_r (%04x): LYMAX = %04x\n", mem_mask, mc68328->regs.lymax);
            return mc68328->regs.lymax;

        case 0xa18:
            verboselog(device->machine(), 2, "mc68328_r (%04x): LCXP = %04x\n", mem_mask, mc68328->regs.lcxp);
            return mc68328->regs.lcxp;

        case 0xa1a:
            verboselog(device->machine(), 2, "mc68328_r (%04x): LCYP = %04x\n", mem_mask, mc68328->regs.lcyp);
            return mc68328->regs.lcyp;

        case 0xa1c:
            verboselog(device->machine(), 2, "mc68328_r (%04x): LCWCH = %04x\n", mem_mask, mc68328->regs.lcwch);
            return mc68328->regs.lcwch;

        case 0xa1e:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): LBLKC = %02x\n", mem_mask, mc68328->regs.lblkc);
                return mc68328->regs.lblkc;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): Unknown address (0xfffa1e)\n", mem_mask);
            }
            break;

        case 0xa20:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): LPOLCF = %02x\n", mem_mask, mc68328->regs.lpolcf);
                return mc68328->regs.lpolcf;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): LPICF = %02x\n", mem_mask, mc68328->regs.lpicf);
                return mc68328->regs.lpicf << 8;
            }
            break;

        case 0xa22:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): LACDRC = %02x\n", mem_mask, mc68328->regs.lacdrc);
                return mc68328->regs.lacdrc;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): Unknown address (0xfffa22)\n", mem_mask);
            }
            break;

        case 0xa24:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): LPXCD = %02x\n", mem_mask, mc68328->regs.lpxcd);
                return mc68328->regs.lpxcd;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): Unknown address (0xfffa24)\n", mem_mask);
            }
            break;

        case 0xa26:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): LCKCON = %02x\n", mem_mask, mc68328->regs.lckcon);
                return mc68328->regs.lckcon;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): Unknown address (0xfffa26)\n", mem_mask);
            }
            break;

        case 0xa28:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): LLBAR = %02x\n", mem_mask, mc68328->regs.llbar);
                return mc68328->regs.llbar;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): Unknown address (0xfffa28)\n", mem_mask);
            }
            break;

        case 0xa2a:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): LOTCR = %02x\n", mem_mask, mc68328->regs.lotcr);
                return mc68328->regs.lotcr;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): Unknown address (0xfffa2a)\n", mem_mask);
            }
            break;

        case 0xa2c:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): LPOSR = %02x\n", mem_mask, mc68328->regs.lposr);
                return mc68328->regs.lposr;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): Unknown address (0xfffa2c)\n", mem_mask);
            }
            break;

        case 0xa30:
            if( mem_mask & 0x00ff )
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): LFRCM = %02x\n", mem_mask, mc68328->regs.lfrcm);
                return mc68328->regs.lfrcm;
            }
            else
            {
                verboselog(device->machine(), 2, "mc68328_r (%04x): Unknown address (0xfffa30)\n", mem_mask);
            }
            break;

        case 0xa32:
            verboselog(device->machine(), 2, "mc68328_r (%04x): LGPMR = %04x\n", mem_mask, mc68328->regs.lgpmr);
            return mc68328->regs.lgpmr;

        case 0xb00:
            verboselog(device->machine(), 2, "mc68328_r (%04x): HMSR(0) = %04x\n", mem_mask, mc68328->regs.hmsr & 0x0000ffff);
            return mc68328->regs.hmsr & 0x0000ffff;

        case 0xb02:
            verboselog(device->machine(), 2, "mc68328_r (%04x): HMSR(16) = %04x\n", mem_mask, mc68328->regs.hmsr >> 16);
            return mc68328->regs.hmsr >> 16;

        case 0xb04:
            verboselog(device->machine(), 2, "mc68328_r (%04x): ALARM(0) = %04x\n", mem_mask, mc68328->regs.alarm & 0x0000ffff);
            return mc68328->regs.alarm & 0x0000ffff;

        case 0xb06:
            verboselog(device->machine(), 2, "mc68328_r (%04x): ALARM(16) = %04x\n", mem_mask, mc68328->regs.alarm >> 16);
            return mc68328->regs.alarm >> 16;

        case 0xb0c:
            verboselog(device->machine(), 2, "mc68328_r (%04x): RTCCTL = %04x\n", mem_mask, mc68328->regs.rtcctl);
            return mc68328->regs.rtcctl;

        case 0xb0e:
            verboselog(device->machine(), 2, "mc68328_r (%04x): RTCISR = %04x\n", mem_mask, mc68328->regs.rtcisr);
            return mc68328->regs.rtcisr;

        case 0xb10:
            verboselog(device->machine(), 2, "mc68328_r (%04x): RTCIENR = %04x\n", mem_mask, mc68328->regs.rtcienr);
            return mc68328->regs.rtcienr;

        case 0xb12:
            verboselog(device->machine(), 2, "mc68328_r (%04x): STPWTCH = %04x\n", mem_mask, mc68328->regs.stpwtch);
            return mc68328->regs.stpwtch;

        default:
            verboselog(device->machine(), 0, "mc68328_r (%04x): Unknown address (0x%06x)\n", mem_mask, 0xfff000 + address);
            break;
    }
    return 0;
}

static DEVICE_RESET( mc68328 )
{
    mc68328_t* mc68328 = mc68328_get_safe_token( device );

    mc68328->regs.scr = 0x0c;
    mc68328->regs.grpbasea = 0x0000;
    mc68328->regs.grpbaseb = 0x0000;
    mc68328->regs.grpbasec = 0x0000;
    mc68328->regs.grpbased = 0x0000;
    mc68328->regs.grpmaska = 0x0000;
    mc68328->regs.grpmaskb = 0x0000;
    mc68328->regs.grpmaskc = 0x0000;
    mc68328->regs.grpmaskd = 0x0000;
    mc68328->regs.csa0 = 0x00010006;
    mc68328->regs.csa1 = 0x00010006;
    mc68328->regs.csa2 = 0x00010006;
    mc68328->regs.csa3 = 0x00010006;
    mc68328->regs.csb0 = 0x00010006;
    mc68328->regs.csb1 = 0x00010006;
    mc68328->regs.csb2 = 0x00010006;
    mc68328->regs.csb3 = 0x00010006;
    mc68328->regs.csc0 = 0x00010006;
    mc68328->regs.csc1 = 0x00010006;
    mc68328->regs.csc2 = 0x00010006;
    mc68328->regs.csc3 = 0x00010006;
    mc68328->regs.csd0 = 0x00010006;
    mc68328->regs.csd1 = 0x00010006;
    mc68328->regs.csd2 = 0x00010006;
    mc68328->regs.csd3 = 0x00010006;

    mc68328->regs.pllcr = 0x2400;
    mc68328->regs.pllfsr = 0x0123;
    mc68328->regs.pctlr = 0x1f;

    mc68328->regs.ivr = 0x00;
    mc68328->regs.icr = 0x0000;
    mc68328->regs.imr = 0x00ffffff;
    mc68328->regs.iwr = 0x00ffffff;
    mc68328->regs.isr = 0x00000000;
    mc68328->regs.ipr = 0x00000000;

    mc68328->regs.padir = 0x00;
    mc68328->regs.padata = 0x00;
    mc68328->regs.pasel = 0x00;
    mc68328->regs.pbdir = 0x00;
    mc68328->regs.pbdata = 0x00;
    mc68328->regs.pbsel = 0x00;
    mc68328->regs.pcdir = 0x00;
    mc68328->regs.pcdata = 0x00;
    mc68328->regs.pcsel = 0x00;
    mc68328->regs.pddir = 0x00;
    mc68328->regs.pddata = 0x00;
    mc68328->regs.pdpuen = 0xff;
    mc68328->regs.pdpol = 0x00;
    mc68328->regs.pdirqen = 0x00;
    mc68328->regs.pddataedge = 0x00;
    mc68328->regs.pdirqedge = 0x00;
    mc68328->regs.pedir = 0x00;
    mc68328->regs.pedata = 0x00;
    mc68328->regs.pepuen = 0x80;
    mc68328->regs.pesel = 0x80;
    mc68328->regs.pfdir = 0x00;
    mc68328->regs.pfdata = 0x00;
    mc68328->regs.pfpuen = 0xff;
    mc68328->regs.pfsel = 0xff;
    mc68328->regs.pgdir = 0x00;
    mc68328->regs.pgdata = 0x00;
    mc68328->regs.pgpuen = 0xff;
    mc68328->regs.pgsel = 0xff;
    mc68328->regs.pjdir = 0x00;
    mc68328->regs.pjdata = 0x00;
    mc68328->regs.pjsel = 0x00;
    mc68328->regs.pkdir = 0x00;
    mc68328->regs.pkdata = 0x00;
    mc68328->regs.pkpuen = 0xff;
    mc68328->regs.pksel = 0xff;
    mc68328->regs.pmdir = 0x00;
    mc68328->regs.pmdata = 0x00;
    mc68328->regs.pmpuen = 0xff;
    mc68328->regs.pmsel = 0xff;

    mc68328->regs.pwmc = 0x0000;
    mc68328->regs.pwmp = 0x0000;
    mc68328->regs.pwmw = 0x0000;
    mc68328->regs.pwmcnt = 0x0000;

    mc68328->regs.tctl[0] = mc68328->regs.tctl[1] = 0x0000;
    mc68328->regs.tprer[0] = mc68328->regs.tprer[1] = 0x0000;
    mc68328->regs.tcmp[0] = mc68328->regs.tcmp[1] = 0xffff;
    mc68328->regs.tcr[0] = mc68328->regs.tcr[1] = 0x0000;
    mc68328->regs.tcn[0] = mc68328->regs.tcn[1] = 0x0000;
    mc68328->regs.tstat[0] = mc68328->regs.tstat[1] = 0x0000;
    mc68328->regs.wctlr = 0x0000;
    mc68328->regs.wcmpr = 0xffff;
    mc68328->regs.wcn = 0x0000;

    mc68328->regs.spisr = 0x0000;

    mc68328->regs.spimdata = 0x0000;
    mc68328->regs.spimcont = 0x0000;

    mc68328->regs.ustcnt = 0x0000;
    mc68328->regs.ubaud = 0x003f;
    mc68328->regs.urx = 0x0000;
    mc68328->regs.utx = 0x0000;
    mc68328->regs.umisc = 0x0000;

    mc68328->regs.lssa = 0x00000000;
    mc68328->regs.lvpw = 0xff;
    mc68328->regs.lxmax = 0x03ff;
    mc68328->regs.lymax = 0x01ff;
    mc68328->regs.lcxp = 0x0000;
    mc68328->regs.lcyp = 0x0000;
    mc68328->regs.lcwch = 0x0101;
    mc68328->regs.lblkc = 0x7f;
    mc68328->regs.lpicf = 0x00;
    mc68328->regs.lpolcf = 0x00;
    mc68328->regs.lacdrc = 0x00;
    mc68328->regs.lpxcd = 0x00;
    mc68328->regs.lckcon = 0x40;
    mc68328->regs.llbar = 0x3e;
    mc68328->regs.lotcr = 0x3f;
    mc68328->regs.lposr = 0x00;
    mc68328->regs.lfrcm = 0xb9;
    mc68328->regs.lgpmr = 0x1073;

    mc68328->regs.hmsr = 0x00000000;
    mc68328->regs.alarm = 0x00000000;
    mc68328->regs.rtcctl = 0x00;
    mc68328->regs.rtcisr = 0x00;
    mc68328->regs.rtcienr = 0x00;
    mc68328->regs.stpwtch = 0x00;

    mc68328->rtc->adjust(attotime::from_hz(1), 0, attotime::from_hz(1));
}

static void mc68328_register_state_save(device_t *device)
{
    mc68328_t* mc68328 = mc68328_get_safe_token( device );

    state_save_register_global(device->machine(), mc68328->regs.scr);
    state_save_register_global(device->machine(), mc68328->regs.grpbasea);
    state_save_register_global(device->machine(), mc68328->regs.grpbaseb);
    state_save_register_global(device->machine(), mc68328->regs.grpbasec);
    state_save_register_global(device->machine(), mc68328->regs.grpbased);
    state_save_register_global(device->machine(), mc68328->regs.grpmaska);
    state_save_register_global(device->machine(), mc68328->regs.grpmaskb);
    state_save_register_global(device->machine(), mc68328->regs.grpmaskc);
    state_save_register_global(device->machine(), mc68328->regs.grpmaskd);
    state_save_register_global(device->machine(), mc68328->regs.csa0);
    state_save_register_global(device->machine(), mc68328->regs.csa1);
    state_save_register_global(device->machine(), mc68328->regs.csa2);
    state_save_register_global(device->machine(), mc68328->regs.csa3);
    state_save_register_global(device->machine(), mc68328->regs.csb0);
    state_save_register_global(device->machine(), mc68328->regs.csb1);
    state_save_register_global(device->machine(), mc68328->regs.csb2);
    state_save_register_global(device->machine(), mc68328->regs.csb3);
    state_save_register_global(device->machine(), mc68328->regs.csc0);
    state_save_register_global(device->machine(), mc68328->regs.csc1);
    state_save_register_global(device->machine(), mc68328->regs.csc2);
    state_save_register_global(device->machine(), mc68328->regs.csc3);
    state_save_register_global(device->machine(), mc68328->regs.csd0);
    state_save_register_global(device->machine(), mc68328->regs.csd1);
    state_save_register_global(device->machine(), mc68328->regs.csd2);
    state_save_register_global(device->machine(), mc68328->regs.csd3);

    state_save_register_global(device->machine(), mc68328->regs.pllcr);
    state_save_register_global(device->machine(), mc68328->regs.pllfsr);
    state_save_register_global(device->machine(), mc68328->regs.pctlr);

    state_save_register_global(device->machine(), mc68328->regs.ivr);
    state_save_register_global(device->machine(), mc68328->regs.icr);
    state_save_register_global(device->machine(), mc68328->regs.imr);
    state_save_register_global(device->machine(), mc68328->regs.iwr);
    state_save_register_global(device->machine(), mc68328->regs.isr);
    state_save_register_global(device->machine(), mc68328->regs.ipr);

    state_save_register_global(device->machine(), mc68328->regs.padir);
    state_save_register_global(device->machine(), mc68328->regs.padata);
    state_save_register_global(device->machine(), mc68328->regs.pasel);
    state_save_register_global(device->machine(), mc68328->regs.pbdir);
    state_save_register_global(device->machine(), mc68328->regs.pbdata);
    state_save_register_global(device->machine(), mc68328->regs.pbsel);
    state_save_register_global(device->machine(), mc68328->regs.pcdir);
    state_save_register_global(device->machine(), mc68328->regs.pcdata);
    state_save_register_global(device->machine(), mc68328->regs.pcsel);
    state_save_register_global(device->machine(), mc68328->regs.pddir);
    state_save_register_global(device->machine(), mc68328->regs.pddata);
    state_save_register_global(device->machine(), mc68328->regs.pdpuen);
    state_save_register_global(device->machine(), mc68328->regs.pdpol);
    state_save_register_global(device->machine(), mc68328->regs.pdirqen);
    state_save_register_global(device->machine(), mc68328->regs.pddataedge);
    state_save_register_global(device->machine(), mc68328->regs.pdirqedge);
    state_save_register_global(device->machine(), mc68328->regs.pedir);
    state_save_register_global(device->machine(), mc68328->regs.pedata);
    state_save_register_global(device->machine(), mc68328->regs.pepuen);
    state_save_register_global(device->machine(), mc68328->regs.pesel);
    state_save_register_global(device->machine(), mc68328->regs.pfdir);
    state_save_register_global(device->machine(), mc68328->regs.pfdata);
    state_save_register_global(device->machine(), mc68328->regs.pfpuen);
    state_save_register_global(device->machine(), mc68328->regs.pfsel);
    state_save_register_global(device->machine(), mc68328->regs.pgdir);
    state_save_register_global(device->machine(), mc68328->regs.pgdata);
    state_save_register_global(device->machine(), mc68328->regs.pgpuen);
    state_save_register_global(device->machine(), mc68328->regs.pgsel);
    state_save_register_global(device->machine(), mc68328->regs.pjdir);
    state_save_register_global(device->machine(), mc68328->regs.pjdata);
    state_save_register_global(device->machine(), mc68328->regs.pjsel);
    state_save_register_global(device->machine(), mc68328->regs.pkdir);
    state_save_register_global(device->machine(), mc68328->regs.pkdata);
    state_save_register_global(device->machine(), mc68328->regs.pkpuen);
    state_save_register_global(device->machine(), mc68328->regs.pksel);
    state_save_register_global(device->machine(), mc68328->regs.pmdir);
    state_save_register_global(device->machine(), mc68328->regs.pmdata);
    state_save_register_global(device->machine(), mc68328->regs.pmpuen);
    state_save_register_global(device->machine(), mc68328->regs.pmsel);

    state_save_register_global(device->machine(), mc68328->regs.pwmc);
    state_save_register_global(device->machine(), mc68328->regs.pwmp);
    state_save_register_global(device->machine(), mc68328->regs.pwmw);
    state_save_register_global(device->machine(), mc68328->regs.pwmcnt);

    state_save_register_global(device->machine(), mc68328->regs.tctl[0]);
    state_save_register_global(device->machine(), mc68328->regs.tctl[1]);
    state_save_register_global(device->machine(), mc68328->regs.tprer[0]);
    state_save_register_global(device->machine(), mc68328->regs.tprer[1]);
    state_save_register_global(device->machine(), mc68328->regs.tcmp[0]);
    state_save_register_global(device->machine(), mc68328->regs.tcmp[1]);
    state_save_register_global(device->machine(), mc68328->regs.tcr[0]);
    state_save_register_global(device->machine(), mc68328->regs.tcr[1]);
    state_save_register_global(device->machine(), mc68328->regs.tcn[0]);
    state_save_register_global(device->machine(), mc68328->regs.tcn[1]);
    state_save_register_global(device->machine(), mc68328->regs.tstat[0]);
    state_save_register_global(device->machine(), mc68328->regs.tstat[1]);
    state_save_register_global(device->machine(), mc68328->regs.wctlr);
    state_save_register_global(device->machine(), mc68328->regs.wcmpr);
    state_save_register_global(device->machine(), mc68328->regs.wcn);

    state_save_register_global(device->machine(), mc68328->regs.spisr);

    state_save_register_global(device->machine(), mc68328->regs.spimdata);
    state_save_register_global(device->machine(), mc68328->regs.spimcont);

    state_save_register_global(device->machine(), mc68328->regs.ustcnt);
    state_save_register_global(device->machine(), mc68328->regs.ubaud);
    state_save_register_global(device->machine(), mc68328->regs.urx);
    state_save_register_global(device->machine(), mc68328->regs.utx);
    state_save_register_global(device->machine(), mc68328->regs.umisc);

    state_save_register_global(device->machine(), mc68328->regs.lssa);
    state_save_register_global(device->machine(), mc68328->regs.lvpw);
    state_save_register_global(device->machine(), mc68328->regs.lxmax);
    state_save_register_global(device->machine(), mc68328->regs.lymax);
    state_save_register_global(device->machine(), mc68328->regs.lcxp);
    state_save_register_global(device->machine(), mc68328->regs.lcyp);
    state_save_register_global(device->machine(), mc68328->regs.lcwch);
    state_save_register_global(device->machine(), mc68328->regs.lblkc);
    state_save_register_global(device->machine(), mc68328->regs.lpicf);
    state_save_register_global(device->machine(), mc68328->regs.lpolcf);
    state_save_register_global(device->machine(), mc68328->regs.lacdrc);
    state_save_register_global(device->machine(), mc68328->regs.lpxcd);
    state_save_register_global(device->machine(), mc68328->regs.lckcon);
    state_save_register_global(device->machine(), mc68328->regs.llbar);
    state_save_register_global(device->machine(), mc68328->regs.lotcr);
    state_save_register_global(device->machine(), mc68328->regs.lposr);
    state_save_register_global(device->machine(), mc68328->regs.lfrcm);
    state_save_register_global(device->machine(), mc68328->regs.lgpmr);

    state_save_register_global(device->machine(), mc68328->regs.hmsr);
    state_save_register_global(device->machine(), mc68328->regs.alarm);
    state_save_register_global(device->machine(), mc68328->regs.rtcctl);
    state_save_register_global(device->machine(), mc68328->regs.rtcisr);
    state_save_register_global(device->machine(), mc68328->regs.rtcienr);
    state_save_register_global(device->machine(), mc68328->regs.stpwtch);
}

static DEVICE_START( mc68328 )
{
    mc68328_t* mc68328 = mc68328_get_safe_token( device );

    mc68328->iface = (const mc68328_interface*)device->static_config();

    mc68328->gptimer[0] = device->machine().scheduler().timer_alloc(FUNC(mc68328_timer1_hit));
    mc68328->gptimer[1] = device->machine().scheduler().timer_alloc(FUNC(mc68328_timer2_hit));
    mc68328->rtc = device->machine().scheduler().timer_alloc(FUNC(mc68328_rtc_tick));
    mc68328->pwm = device->machine().scheduler().timer_alloc(FUNC(mc68328_pwm_transition));

    mc68328_register_state_save(device);
}

DEVICE_GET_INFO( mc68328 )
{
    switch ( state )
    {
        /* --- the following bits of info are returned as 64-bit signed integers --- */
        case DEVINFO_INT_TOKEN_BYTES:           info->i = sizeof(mc68328_t);                    break;

        /* --- the following bits of info are returned as pointers to data or functions --- */
        case DEVINFO_FCT_START:                 info->start = DEVICE_START_NAME(mc68328);       break;
        case DEVINFO_FCT_STOP:                  /* nothing */                                   break;
        case DEVINFO_FCT_RESET:                 info->reset = DEVICE_RESET_NAME(mc68328);       break;

        /* --- the following bits of info are returned as NULL-terminated strings --- */
        case DEVINFO_STR_NAME:                  strcpy(info->s, "Motorola MC68328 (DragonBall) Integrated Processor"); break;
        case DEVINFO_STR_FAMILY:                strcpy(info->s, "MC68328");                     break;
        case DEVINFO_STR_VERSION:               strcpy(info->s, "1.00");                        break;
        case DEVINFO_STR_SOURCE_FILE:           strcpy(info->s, __FILE__);                      break;
        case DEVINFO_STR_CREDITS:               strcpy(info->s, "Copyright the MESS Teams and Ryan Holtz"); break;
    }
}

DEFINE_LEGACY_DEVICE(MC68328, mc68328);
