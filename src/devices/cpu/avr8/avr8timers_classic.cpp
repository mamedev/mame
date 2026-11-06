// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Sandro Ronco, Felipe Sanches, wurthless-elektroniks
/****************************************************************************

    Classic AVR8 series timers
    Device-ified and modularized by würthless elektroniks
    With much love to all the AVR8 core devs and maintainers

    This covers the various timers present on the "classic" megaAVR and tinyAVR lineups.
    The modern AVR8 chips use a completely different timer subsystem which will need
    to go into a separate file, if we ever get to the point where we need to emulate the
    newer chips.

    Brief intro to the classic AVR8 timer subsytem:
    -----------------------------------------------------------------

    Typically, the classic AVR8s have the following kinds of timers: 8-bit low-precision,
    16-bit high precision, and/or a high-speed timer subsystem.

    The "classic" megaAVRs, and their timers, are:

    | Series       | Timer 0 | Timer 1 | Timer 2 | Timer 3 | Timer 4 | Timer 5 |
    |--------------|---------|---------|---------|---------|---------|---------|
    | ATMega       | Generic | Generic | 8-bit   |  n/a    |  n/a    |  n/a    |
    | 88, 168, 328 | timer 0 | timer 1 | fakeRTC |         |         |         |
    |--------------|---------|---------|---------|---------|---------|---------|
    | ATMega       | Generic | Generic |  n/a    | 16-bit  | 10-bit  |  n/a    |
    | 16U4, 32U4   | timer 0 | timer 1 |         |         | hispeed |         |
    |--------------|---------|---------|---------|---------|---------|---------|
    | ATMega 640,  | Generic | 16-bit  | 8-bit   | 16-bit  | 16-bit  | 16-bit  |
    | 1280, 2560   | timer 0 | 3 ch.   | fakeRTC | 3 ch.   | 3 ch.   | 3 ch.   |
    |--------------|---------|---------|---------|---------|---------|---------|
    | ATMega 324,  | Generic | Generic | 8-bit   |  n/a    |  n/a    |  n/a    |
    | 644          | timer 0 | timer 1 | fakeRTC |         |         |         |
    |--------------|---------|---------|---------|---------|---------|---------|
    | ATMega 1284, | Generic | Generic | 8-bit   | Generic |  n/a    |  n/a    | 
    | 2564         | timer 0 | timer 1 | fakeRTC | timer 1 |         |         |
    |--------------|---------|---------|---------|---------|---------|---------|

    The most common timer types are implementations of a generic 8-bit and 16-bit design.
    
    The generic 8-bit timer is always slotted into the Timer 0 role, and often there is an
    asynchronous timer used for faking an RTC that has different clock selection modes,
    but counts the same.

    The generic 16-bit timer is usually in the Timer 1 slot. On most devices it will have
    two compare channels, but some implementations give it three compare channels.
    
    The 10-bit highspeed timer on the ATMegaXU4 series is driven either from
    clk_io or a high-speed PLL. Since its subsystem is different enough from the
    standard AVR8 scheme, its implementation goes in a separate file.

    The oldest AVR8s and old ATTinys have different registers on the CPU side,
    but they can also use this timer subsystem to an extent.
    Those devices are:

    | Series       | Timer 0 | Timer 1 | Timer 2 | Timer 3 | Timer 4 | Timer 5 |
    |--------------|---------|---------|---------|---------|---------|---------|
    | ATMega 8, 16,| Generic | Generic | 8-bit   |  n/a    |  n/a    |  n/a    |
    | 32           | timer 0 | timer 1 | fakeRTC |         |         |         | 
    |--------------|---------|---------|---------|---------|---------|---------|
    | ATTiny 15    | Generic | 8-bit   |  n/a    |  n/a    |  n/a    |  n/a    |
    |              | timer 0 | hispeed |         |         |         |         |
    |--------------|---------|---------|---------|---------|---------|---------|
    | ATTiny 25,   | Generic | 8-bit   |  n/a    |  n/a    |  n/a    |  n/a    |
    | 45, 85       | timer 0 | hispeed |         |         |         |         |
    |--------------|---------|---------|---------|---------|---------|---------|

    The ATTiny15 has its own high-speed timer implementation, this time with an 8-bit
    timer. The ATTiny25/45/85 inherits this timer, but it adds more features on top of
    having a ATTiny15 backwards compatibility mode. Like the 10-bit counter on the
    ATMegaXU4 series, the timer subsystem should go in its own file, and not here.

    16-bit register accesses:
    -----------------------------------------------------------------

    Paraphrasing the common "Accessing 16-bit Registers" from the ATmega datasheets...

    The 16-bit timer registers must be accessed atomically, so each 16-bit timer
    device contains an 8-bit temporary register to help with this. And to complicate
    our emulation, of course.

    The temporary register is updated when the lower 8 bits of a 16-bit register
    are read, or the upper 8 bits of a register are written. In both cases, the
    temporary register gets the upper 8 bits of the value to be read or written
    to the 16-bit register. Writes only take effect when the lower 8 bits are written.
    
    The datasheets therefore recommend that reads are done lower 8 bits followed by the
    upper 8 bits, and writes be performed by writing the upper 8 bits first,
    followed by the lower 8 bits. Interrupts also have to be disabled, as ISRs
    can overwrite the contents of the temporary register and buggy behavior will result.

    A bonus sideeffect of this temporary register is that you can latch the upper 8 bits
    in that register, and then writes to the lower halves of other 16-bit registers will
    reuse whatever you put there.

    There are obviously exceptions to this behavior to add a few more headaches,
    but those apply to the specific registers. In general, TCNTx will obey these
    rules exactly.

    Output compare signals:
    -----------------------------------------------------------------

    When the COMxx modes are switched in the timers to a non-zero mode, the following happens:

    1. Whatever value was being fed from the PORTx register gets latched into place somewhere.

    2. The PORTx line is disconnected from the GPIO driver entirely; either the output compare signal
       or compare modulator will be fed to the output matrix, but only when the pin is set as an output
       (the GPIO mux still obeys DDRx settings).

    3. When COMxx is set back to 0, the PORTx bit is reconnected to its output immediately.

    Output compare modulator:
    -----------------------------------------------------------------

    - ATMega88, ATMega168, ATMega328: Not present. ATMega328PB has one.

    - ATMega640, ATMega1280, ATMega2560: inputs on OC1C/OC0A, output to OC1C/OC0A/PB7 pin.

    - ATMega16U4/ATMega32U4: inputs on OC1C/OC0A, output to OC1C/OC0A/PB7 pin.

****************************************************************************/

#include "emu.h"
#include "avr8timers_classic.h"

#define LOG_TIMER0          (1U << 0)
#define LOG_TIMER0_TICK     (1U << 1)
#define LOG_TIMER1          (1U << 2)
#define LOG_TIMER1_TICK     (1U << 3)

#define LOG_INT_ACKS (1 << 4)

#define VERBOSE             (LOG_TIMER1 | LOG_TIMER1_TICK)
//#define LOG_OUTPUT_FUNC     osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(AVR8_CLASSIC_GENERIC_TIMER_0, avr8_generic_t0_timer_device, "cavr8gent0", "Classic AVR8 generic 8-bit timer 0");
DEFINE_DEVICE_TYPE(AVR8_CLASSIC_MEGAAVR_TIMER_0, avr8_megaavr_timer_0_device,  "cmegaavrt0", "Classic megaAVR Timer 0");

DEFINE_DEVICE_TYPE(AVR8_CLASSIC_GENERIC_TIMER_1, avr8_generic_timer_1_device, "cavr8gent1", "Classic AVR8 generic 16-bit timer");
DEFINE_DEVICE_TYPE(AVR8_CLASSIC_MEGAAVR_TIMER_1, avr8_megaavr_timer_1_device, "cmegaavrt1", "Classic megaAVR 16-bit timer");

DEFINE_DEVICE_TYPE(AVR8_CLASSIC_10BIT_PRESCALER, avr8_10bit_prescaler_device, "avr8presc", "Classic AVR8 10-bit prescaler");
DEFINE_DEVICE_TYPE(AVR8_CLASSIC_OUTPUT_COMPARE_MODULATOR, avr8_output_compare_modulator_device, "avr8ocm", "Classic AVR8 Output Compare Modulator");

DEFINE_DEVICE_TYPE(AVR8_CLASSIC_GENERIC_FAKE_RTC_TIMER, avr8_generic_fakertc_timer_device, "cavr8frtct", "Classic AVR8 asynchronous 8-bit timer");
DEFINE_DEVICE_TYPE(AVR8_CLASSIC_MEGAAVR_FAKE_RTC_TIMER, avr8_megaavr_fakertc_timer_device, "cmegaavrfrtct", "Classic megaAVR asynchronous 8-bit timer");
DEFINE_DEVICE_TYPE(AVR8_CLASSIC_FAKE_RTC_ASYNC_CONTROLLER, avr8_fakertc_async_controller_device, "cavr8frtcc", "Classic AVR8 asynchronous timer control unit");
DEFINE_DEVICE_TYPE(AVR8_CLASSIC_MEGAAVR_FAKE_RTC_ASYNC_CONTROLLER, avr8_megaavr_fakertc_async_controller_device, "cmegaavrfrtcc", "Classic megaAVR asynchronous timer control unit");

///////////////////////////////////////////////////////////////
// BASE TIMER
///////////////////////////////////////////////////////////////

avr8_base_timer_device::avr8_base_timer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
    device_t(mconfig, type, tag, owner, clock),
    m_upstream_prescaler(*this, "syncprescaler"),
    m_int_cbs(*this),
    m_oc_en_w_cbs(*this),
    m_oc_w_cbs(*this),
    m_oc_toggle_w_cbs(*this)
{}

void avr8_base_timer_device::device_start()
{
    save_item(NAME(m_tcnt));
    save_item(NAME(m_ocr_buf));
    save_item(NAME(m_ocr));
    save_item(NAME(m_com));
    save_item(NAME(m_wgm));
    save_item(NAME(m_cs));
    save_item(NAME(m_int));
    save_item(NAME(m_int_en));
    save_item(NAME(m_prtim));
    save_item(NAME(m_t));
    save_item(NAME(m_t_last));
    save_item(NAME(m_ocr_changed));

    
    m_max = (1 << get_num_bits()) - 1;
    save_item(NAME(m_max));
}

void avr8_base_timer_device::device_reset()
{
    m_tcnt = 0;
    m_wgm = 0;
    m_cs = 0;

    for (int i = 0; i < AVR8_TIMER_CMPCHANNEL_MAX; i++)
    {
        m_ocr[i] = 0;
        m_ocr_buf[i] = 0;
        m_com[i] = AVR8_TIMER_COM_NORMAL;
        // GPIO driver should reset its output states for us
    }

    for (int i = 0; i < AVR8_TIMER_INTCHANNEL_MAX; i++)
    {
        m_int[i] = false;
        m_int_en[i] = false;
    }

    m_prtim = false;
    m_t = false;
    m_t_last = false;
}

template <uint8_t Channel> 
void avr8_base_timer_device::int_w(uint8_t data)
{
    if (!data) return;
    if (!m_int[Channel]) return;

    // any nonzero write to the interrupt flag (from the CPU program) will clear the interrupt
    m_int[Channel] = false;
    if (m_int_en[Channel] && !m_int_cbs[Channel].isunset()) m_int_cbs[Channel](0);
}

template <uint8_t Channel> 
void avr8_base_timer_device::int_ack_w(uint8_t data)
{
    // this is an interrupt acknowledge coming from the AVR8 CPU itself (because it just took the interrupt),
    // so don't run callback or we'll get infinite recursion headaches
    m_int[Channel] = false;
}

template <uint8_t Channel> 
void avr8_base_timer_device::int_en_w(uint8_t data)
{
    if ((data != 0) == m_int_en[Channel]) return;

    m_int_en[Channel] = data != 0;
    if (!m_int_cbs[Channel].isunset()) m_int_cbs[Channel](m_int_en[Channel] && m_int[Channel]);
}

// if you remove these, then i hope you like linker errors...
template void avr8_base_timer_device::int_w<AVR8_TIMER_INTCHANNEL_OV>(uint8_t);
template void avr8_base_timer_device::int_w<AVR8_TIMER_INTCHANNEL_OC_A>(uint8_t);
template void avr8_base_timer_device::int_w<AVR8_TIMER_INTCHANNEL_OC_B>(uint8_t);
template void avr8_base_timer_device::int_w<AVR8_TIMER_INTCHANNEL_OC_C>(uint8_t);
template void avr8_base_timer_device::int_w<AVR8_TIMER_INTCHANNEL_IC>(uint8_t);
template void avr8_base_timer_device::int_en_w<AVR8_TIMER_INTCHANNEL_OV>(uint8_t);
template void avr8_base_timer_device::int_en_w<AVR8_TIMER_INTCHANNEL_OC_A>(uint8_t);
template void avr8_base_timer_device::int_en_w<AVR8_TIMER_INTCHANNEL_OC_B>(uint8_t);
template void avr8_base_timer_device::int_en_w<AVR8_TIMER_INTCHANNEL_OC_C>(uint8_t);
template void avr8_base_timer_device::int_en_w<AVR8_TIMER_INTCHANNEL_IC>(uint8_t);
template void avr8_base_timer_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OV>(uint8_t);
template void avr8_base_timer_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_A>(uint8_t);
template void avr8_base_timer_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_B>(uint8_t);
template void avr8_base_timer_device::int_ack_w<AVR8_TIMER_INTCHANNEL_OC_C>(uint8_t);
template void avr8_base_timer_device::int_ack_w<AVR8_TIMER_INTCHANNEL_IC>(uint8_t);

template <uint8_t Channel> 
void avr8_base_timer_device::raise_int()
{
    if (m_int[Channel]) return;
    m_int[Channel] = true;
    if (m_int_en[Channel] && !m_int_cbs[Channel].isunset()) m_int_cbs[Channel](1);
}

template <uint8_t Channel> 
void avr8_base_timer_device::clear_int()
{
    if (!m_int[Channel]) return;
    m_int[Channel] = false;
    if (m_int_en[Channel] && !m_int_cbs[Channel].isunset()) m_int_cbs[Channel](0);
}

void avr8_base_timer_device::clk_io_w(uint8_t data)
{
    if (!m_clk_io && data != 0)
    {
        tick();
    }
    m_clk_io = data != 0;
}

void avr8_base_timer_device::advance_illegal_wgm_state()
{
    logerror("%s: advance timer on illegal/unimplemented wgm state %02x\n", machine().describe_context(), m_wgm);
}

void avr8_base_timer_device::advance_normal_generic()
{
    if (will_next_count_overflow())
    {
        raise_tov();
    }
    m_tcnt = next_count();
}

template <uint8_t Channel,
            uint8_t SetState,
            uint8_t ClearState,
            bool Toggling>
void avr8_base_timer_device::update_oc()
{
    if (m_com[Channel] == AVR8_TIMER_COM_NORMAL) return;

    if (m_oc_w_cbs[Channel].isunset() || m_oc_toggle_w_cbs[Channel].isunset())
    {
        return;
    }

    if (Toggling && m_com[Channel] == AVR8_TIMER_COM_TOGGLE) m_oc_toggle_w_cbs[Channel](1);
    else if (m_com[Channel] == AVR8_TIMER_COM_CLEAR) m_oc_w_cbs[Channel](ClearState);
    else if (m_com[Channel] == AVR8_TIMER_COM_SET) m_oc_w_cbs[Channel](SetState);
}

template <uint8_t Channel>
void avr8_base_timer_device::com_w(uint8_t data)
{
    m_com[Channel] = BIT(data, 0, 2); // COM always a value between 0-3

    // default is to assume the output is enabled if COM changes to anything
    // other than AVR8_TIMER_COM_NORMAL.
    // this will not always be the case; subclasses have to catch that situation.
    m_oc_en_w_cbs[Channel](m_com[Channel] != AVR8_TIMER_COM_NORMAL && com_mode_allows_output(Channel));
}

template void avr8_base_timer_device::com_w<AVR8_TIMER_CMPCHANNEL_OC_A>(uint8_t);
template void avr8_base_timer_device::com_w<AVR8_TIMER_CMPCHANNEL_OC_B>(uint8_t);
template void avr8_base_timer_device::com_w<AVR8_TIMER_CMPCHANNEL_OC_C>(uint8_t);

void avr8_base_timer_device::sample_t_input()
{
    // TODO: this really belongs in a pin matrix driver
    if (m_t == m_t_last) return;
    m_t_rising = !m_t_last && m_t;
    m_t_falling = m_t_last && !m_t;
}

///////////////////////////////////////////////////////////////
// COMMON 10-BIT PRESCALER
///////////////////////////////////////////////////////////////

avr8_10bit_prescaler_device::avr8_10bit_prescaler_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
    device_t(mconfig, AVR8_CLASSIC_10BIT_PRESCALER, tag, owner, clock)
{}

void avr8_10bit_prescaler_device::device_start()
{
    save_item(NAME(m_count));
    save_item(NAME(m_sync));
    save_item(NAME(m_taps));
}

void avr8_10bit_prescaler_device::device_reset()
{
    m_count = 0;
    m_taps = 0;
    m_sync = false;
}

void avr8_10bit_prescaler_device::tick()
{
    // TODO: this is a guess; if the signals rise and stay risen
    // then they should not cause the timers downstream to advance
    // 
    // this is also somewhat of a performance bottleneck because the
    // downstream timers will be ticked regardless of if the condition is
    // satisfied for this prescaler clock cycle.
    int next_count = m_sync ? 0 : (m_count + 1) & 0x3ff;
    m_taps = ((m_count & ~next_count) << 1);
    m_count = next_count;
}

void avr8_10bit_prescaler_device::sync_w(uint8_t data)
{
    if (data != 0) m_count = 0;

    // remember: GTCCR and others can keep this signal asserted
    m_sync = data != 0;
}

uint8_t avr8_10bit_prescaler_device::sync_r()
{
    return m_sync;
}

///////////////////////////////////////////////////////////////
// OUTPUT COMPARE MODULATOR
///////////////////////////////////////////////////////////////

avr8_output_compare_modulator_device::avr8_output_compare_modulator_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
    device_t(mconfig, AVR8_CLASSIC_OUTPUT_COMPARE_MODULATOR, tag, owner, clock),
    m_gpio_bit_w_wb(*this)
{ }

void avr8_output_compare_modulator_device::device_start()
{
    save_item(NAME(m_compare_out_a));
    save_item(NAME(m_compare_out_en_a));
    save_item(NAME(m_compare_out_b));
    save_item(NAME(m_compare_out_en_b));
    save_item(NAME(m_port_bit));
}

void avr8_output_compare_modulator_device::device_reset()
{
    m_compare_out_a = false;
    m_compare_out_en_a = false;
    m_compare_out_b = false;
    m_compare_out_en_b = false;
    m_port_bit = false;
}

void avr8_output_compare_modulator_device::update_outputs()
{
    // passthrough mode: PORTx bit goes directly to GPIO output
    // when both compare outputs disabled
    if (!m_compare_out_en_a && !m_compare_out_en_b)
    {
        m_gpio_bit_w_wb(m_port_bit);
        return;
    }
    
    // modulation mode: if both compare outputs enabled,
    // PORTx bit selects between AND modulation (=0) or
    // OR modulation (=1)
    if (m_compare_out_en_a && m_compare_out_en_b)
    {
        if (m_port_bit)
        {
            m_gpio_bit_w_wb(m_compare_out_a | m_compare_out_b);
        }
        else
        {
            m_gpio_bit_w_wb(m_compare_out_a & m_compare_out_b);
        }
        return;
    }

    // in all other cases, the active output compare channel
    // feeds the output with the appropriate value
    if (m_compare_out_en_a) m_gpio_bit_w_wb(m_compare_out_a);
    if (m_compare_out_en_b) m_gpio_bit_w_wb(m_compare_out_b);
}

void avr8_output_compare_modulator_device::compare_out_a_w(uint8_t data)
{
    m_compare_out_a = data != 0;
    update_outputs();
}

void avr8_output_compare_modulator_device::compare_out_en_a_w(uint8_t data)
{
    bool old = m_compare_out_en_a;
    m_compare_out_en_a = data != 0;

    if (!old && m_compare_out_en_a && !m_compare_out_en_b)
    {
        // when one output channel is enabled, the existing PORTx bit is latched
        // but no longer forwarded.
        // not sure of behavior when both COMxx signals are set at the same time...
        m_compare_out_a = m_port_bit;
    }
    
    update_outputs();
}

void avr8_output_compare_modulator_device::compare_out_toggle_a_w(uint8_t _)
{
    if (m_compare_out_en_a && !m_compare_out_en_b)
    {
        // this value is not propagated back to the PORTx bit
        m_compare_out_a = !m_compare_out_a;
        update_outputs();
    }
}

void avr8_output_compare_modulator_device::compare_out_b_w(uint8_t data)
{
    m_compare_out_b = data != 0;
    update_outputs();
}

void avr8_output_compare_modulator_device::compare_out_en_b_w(uint8_t data)
{
    bool old = m_compare_out_en_b;
    m_compare_out_en_b = data != 0;

    if (!old && m_compare_out_en_b && !m_compare_out_en_a)
    {
        // when one output channel is enabled, the existing PORTx bit is latched
        // but no longer forwarded.
        // not sure of behavior when both COMxx signals are set at the same time...
        m_compare_out_b = m_port_bit;
    }
    
    update_outputs();
}

void avr8_output_compare_modulator_device::compare_out_toggle_b_w(uint8_t _)
{
    if (!m_compare_out_en_a && m_compare_out_en_b)
    {
        // this value is not propagated back to the PORTx bit
        m_compare_out_b = !m_compare_out_b;
        update_outputs();
    }
}

void avr8_output_compare_modulator_device::port_bit_w(uint8_t data)
{
    m_port_bit = data != 0;
    update_outputs();
}

///////////////////////////////////////////////////////////////
// COMMON 8-BIT TIMER FUNCTIONS
// (SHARED BETWEEN TIMER 0 AND TIMER 2)
///////////////////////////////////////////////////////////////

avr8_generic_8bit_timer_device::avr8_generic_8bit_timer_device(const machine_config &mconfig,
    const char *tag,
    device_t *owner,
    uint32_t clock,
    const device_type device_type) :
    avr8_base_timer_device(mconfig, device_type, tag, owner, clock)
{ }

// TOP = MAX
// TOV flag set on MAX (so if OCRA is hit, no interrupt is generated)
void avr8_generic_8bit_timer_device::advance_normal()
{
    if (is_compare_a())
    {
        raise_ocf_a();
        update_oc_generic<AVR8_TIMER_CMPCHANNEL_OC_A>();
    }

    if (is_compare_b())
    {
        raise_ocf_b();
        update_oc_generic<AVR8_TIMER_CMPCHANNEL_OC_B>();
    }

    advance_normal_generic();
}


// TOP = OCRA
// Update of OCRx is IMMEDIATE
// TOV flag set on BOTTOM
void avr8_generic_8bit_timer_device::advance_pwm()
{
    uint8_t next_count_value = m_dual_slope_counting_down ? prev_count() : next_count();
    uint8_t count_limit = m_dual_slope_counting_down ? 0 : max_count();

    if (is_compare_a())
    {
        if (m_ocr[AVR8_TIMER_CMPCHANNEL_OC_A] != max_count())
        {
            raise_ocf_a();
        }

        if (m_dual_slope_counting_down)
        {
            update_oc_generic_notoggle_inverse<AVR8_TIMER_CMPCHANNEL_OC_A>();
        }
        else
        {
            update_oc_generic_notoggle<AVR8_TIMER_CMPCHANNEL_OC_A>();
        }
    }

    if (is_compare_b())
    {
        if (m_dual_slope_counting_down)
        {
            update_oc_generic_notoggle_inverse<AVR8_TIMER_CMPCHANNEL_OC_B>();
        }
        else
        {
            update_oc_generic_notoggle<AVR8_TIMER_CMPCHANNEL_OC_B>();
        }
    }

    if (is_min_count())
    {
        raise_tov();
    }

    if (next_count_value == count_limit)
    {
        m_dual_slope_counting_down = !m_dual_slope_counting_down;
    }

    m_tcnt = next_count_value;
}

// TOP = OCRA
// Update of OCRx is IMMEDIATE
// TOV flag set on MAX (so if OCRA is hit, no interrupt is generated)
void avr8_generic_8bit_timer_device::advance_ctc()
{
    bool forcing_wrap = false;
    if (is_compare_a())
    {
        raise_ocf_a();
        update_oc_generic<AVR8_TIMER_CMPCHANNEL_OC_A>();
        forcing_wrap = true;
    }

    if (is_compare_b())
    {
        raise_ocf_b();
        update_oc_generic<AVR8_TIMER_CMPCHANNEL_OC_B>();
    }

    if (will_next_count_overflow())
    {
        raise_tov();
    }

    m_tcnt = forcing_wrap ? 0 : next_count();
}


// TOP = OCRA
// Update of OCRx when BOTTOM is hit
// TOV flag set on MAX (so if OCRA is hit, no interrupt is generated)
void avr8_generic_8bit_timer_device::advance_pwm_ocr_a()
{
    uint8_t next_count_value = m_dual_slope_counting_down ? prev_count() : next_count();

    if (is_compare_a())
    {
        // TODO: confirm this; datasheet phrasing is a bit odd
        if (m_ocr[AVR8_TIMER_CMPCHANNEL_OC_A] != max_count())
        {
            raise_ocf_a();
        }

        // OCR mode has WGM02 = 1; so toggling is allowed
        if (m_dual_slope_counting_down)
        {
            update_oc_generic<AVR8_TIMER_CMPCHANNEL_OC_A>();
        }
        else
        {
            update_oc_generic_inverse<AVR8_TIMER_CMPCHANNEL_OC_A>();
        }
    }

    if (is_compare_b())
    {
        // TODO: confirm this; datasheet phrasing is a bit odd
        if (m_ocr[AVR8_TIMER_CMPCHANNEL_OC_B] != max_count())
        {
            raise_ocf_b();
        }

        // COMxB of 0b01 is reserved/illegal, so do not allow toggling in this mode
        if (m_dual_slope_counting_down)
        {
            update_oc_generic_notoggle<AVR8_TIMER_CMPCHANNEL_OC_A>();
        }
        else
        {
            update_oc_generic_notoggle_inverse<AVR8_TIMER_CMPCHANNEL_OC_A>();
        }
    }

    if (m_dual_slope_counting_down && next_count_value == 0)
    {
        m_dual_slope_counting_down = !m_dual_slope_counting_down;
    }
    else if (is_min_count())
    {
        // TOV raised at BOTTOM
        raise_tov();
    }

    m_tcnt = next_count_value;
}

// TOP = MAX
// Update of OCRx at BOTTOM
// TOV flag set on MAX
void avr8_generic_8bit_timer_device::advance_fast_pwm()
{
    // special case: if OCRxx == TOP, then the compare match is ignored.
    // See https://ww1.microchip.com/downloads/en/DeviceDoc/ATmega48A-PA-88A-PA-168A-PA-328-P-DS-DS40002061A.pdf
    // Figure 15-6 
	if (is_compare_a() && m_ocr[AVR8_TIMER_CMPCHANNEL_OC_A] != max_count())
	{
        raise_ocf_a();

        // toggle is disabled in this mode (WGMx2 is 0)
        update_oc_generic_notoggle<AVR8_TIMER_CMPCHANNEL_OC_A>();
	}
	
    // same as above
    if (is_compare_b() && m_ocr[AVR8_TIMER_CMPCHANNEL_OC_B] != max_count())
	{
        raise_ocf_b();

        // OCxB is set to normal value on compare; COMxB of 0b01 is reserved/illegal
        update_oc_generic_notoggle<AVR8_TIMER_CMPCHANNEL_OC_B>();
	}

	if (will_next_count_overflow())
	{
		raise_tov();
	}

    if (is_min_count())
    {
        // OCxB is set to inverse on BOTTOM; COMxB of 0b01 is reserved/illegal
        update_oc_generic_notoggle_inverse<AVR8_TIMER_CMPCHANNEL_OC_A>();
        update_oc_generic_notoggle_inverse<AVR8_TIMER_CMPCHANNEL_OC_B>();
    }
    
    m_tcnt = next_count();
}

// TOP = OCRA
// Update of OCRx at BOTTOM
// TOV flag set on TOP (so, when OCRA is hit)
void avr8_generic_8bit_timer_device::advance_fast_pwm_ocr_a()
{
    bool hit_top = false;
    if (is_compare_a())
    {
        update_oc_generic_notoggle<AVR8_TIMER_CMPCHANNEL_OC_A>();
        hit_top = true;
        raise_ocf_a();
        raise_tov(); // since TOP was hit and we are about to wrap
    }

    if (is_compare_b())
    {
        raise_ocf_b();
        update_oc_generic_notoggle<AVR8_TIMER_CMPCHANNEL_OC_B>();
    }

    if (is_min_count())
    {
        update_oc_generic_notoggle_inverse<AVR8_TIMER_CMPCHANNEL_OC_A>();
        update_oc_generic_notoggle_inverse<AVR8_TIMER_CMPCHANNEL_OC_B>();
    }

    m_tcnt = hit_top ? 0 : next_count();
}

void avr8_generic_8bit_timer_device::force_oc0a()
{
    if (m_wgm == GENERIC_8BIT_TIMER_WGM_NORMAL || m_wgm == GENERIC_8BIT_TIMER_WGM_CTC)
    {
        update_oc_generic<AVR8_TIMER_CMPCHANNEL_OC_A>();
    }
}

void avr8_generic_8bit_timer_device::force_oc0b()
{
    if (m_wgm == GENERIC_8BIT_TIMER_WGM_NORMAL || m_wgm == GENERIC_8BIT_TIMER_WGM_CTC)
    {
        update_oc_generic<AVR8_TIMER_CMPCHANNEL_OC_B>();
    }
}

void avr8_generic_8bit_timer_device::wgm_w(uint8_t data)
{
    if (m_wgm == data) return;

    logerror("%s: switch wgm to %02x\n", machine().describe_context(), data);
    m_wgm = data;

    // output lines will switch when WGM switches, so rewrite COM modes
    com_w<AVR8_TIMER_CMPCHANNEL_OC_A>(com_r<AVR8_TIMER_CMPCHANNEL_OC_A>());
    com_w<AVR8_TIMER_CMPCHANNEL_OC_B>(com_r<AVR8_TIMER_CMPCHANNEL_OC_B>());
}

bool avr8_generic_8bit_timer_device::should_reload_ocrs()
{
    // This behavior should be true for the generic Timer 0 and the async Timer 2.
    // See: https://ww1.microchip.com/downloads/en/DeviceDoc/ATmega48A-PA-88A-PA-168A-PA-328-P-DS-DS40002061A.pdf
    // Table 15-8 (Timer 0), Table 18-8 (Timer 2)
    switch (m_wgm)
    {
        case GENERIC_8BIT_TIMER_WGM_NORMAL:
        case GENERIC_8BIT_TIMER_WGM_CTC:
            return true; // updates immediately

        case GENERIC_8BIT_TIMER_WGM_PWM:
            return (m_tcnt & max_count()) == max_count(); // updates at TOP (= 0xff)

        case GENERIC_8BIT_TIMER_WGM_PWM_OCR_A:
            return (m_tcnt & max_count()) == m_ocr[AVR8_TIMER_CMPCHANNEL_OC_A]; // updates at TOP (= OCR A)

        case GENERIC_8BIT_TIMER_WGM_FAST_PWM:
        case GENERIC_8BIT_TIMER_WGM_FAST_PWM_OCR_A:
            return (m_tcnt & max_count()) == 0; // updates at BOTTOM

        default:
            return false;
    }
}

///////////////////////////////////////////////////////////////
// TIMER 0 COMMON
///////////////////////////////////////////////////////////////

avr8_generic_t0_timer_device::avr8_generic_t0_timer_device(const machine_config &mconfig,
    const char *tag,
    device_t *owner,
    uint32_t clock,
    const device_type device_type) :
    avr8_generic_8bit_timer_device(mconfig, tag, owner, clock, device_type)
{ }

avr8_generic_t0_timer_device::avr8_generic_t0_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
    avr8_generic_t0_timer_device(mconfig, tag, owner, clock, AVR8_CLASSIC_GENERIC_TIMER_0)
{ }

avr8_megaavr_timer_0_device::avr8_megaavr_timer_0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
    avr8_generic_t0_timer_device(mconfig, tag, owner, clock, AVR8_CLASSIC_MEGAAVR_TIMER_0)
{ }

void avr8_generic_t0_timer_device::tick()
{
    if (m_prtim) return;

    if (m_cs == GENERIC_TIMER0_CS_T0_FALL || m_cs == GENERIC_TIMER0_CS_T0_FALL)
    {
        sample_t_input();
    }

    if (m_cs == GENERIC_TIMER0_CS_DISABLED)
    {
        // LOGMASKED(LOG_TIMER0_TICK, "%s: timer is disabled (m_cs == GENERIC_TIMER0_CS_DISABLED), doing nothing.\n", machine().describe_context());
        return;
    }

    if (m_cs == GENERIC_TIMER0_CS_PRESCALE_DIV8 && !(m_upstream_prescaler->taps_r() & avr8_10bit_prescaler_device::AVR8_PRESCALER_TAPMASK_DIV8)) return;
    if (m_cs == GENERIC_TIMER0_CS_PRESCALE_DIV64 && !(m_upstream_prescaler->taps_r() & avr8_10bit_prescaler_device::AVR8_PRESCALER_TAPMASK_DIV64)) return;
    if (m_cs == GENERIC_TIMER0_CS_PRESCALE_DIV256 && !(m_upstream_prescaler->taps_r() & avr8_10bit_prescaler_device::AVR8_PRESCALER_TAPMASK_DIV256)) return;
    if (m_cs == GENERIC_TIMER0_CS_PRESCALE_DIV1024 && !(m_upstream_prescaler->taps_r() & avr8_10bit_prescaler_device::AVR8_PRESCALER_TAPMASK_DIV1024)) return;
    if (m_cs == GENERIC_TIMER0_CS_T0_FALL && !m_t_falling) return;
    if (m_cs == GENERIC_TIMER0_CS_T0_FALL && !m_t_rising) return;
    // GENERIC_TIMER0_CS_DIRECT = just fall through

    if (m_ocr_changed && should_reload_ocrs())
    {
        reload_ocrs();
    }

    LOGMASKED(LOG_TIMER0_TICK, "%s: running wgm callback %d\n", machine().describe_context(), m_wgm);
    
    switch(m_wgm)
    {
        case GENERIC_8BIT_TIMER_WGM_NORMAL:
            advance_normal();
            break;
        case GENERIC_8BIT_TIMER_WGM_CTC:
            advance_ctc();
            break;
        case GENERIC_8BIT_TIMER_WGM_PWM:
            advance_pwm();
            break;
        case GENERIC_8BIT_TIMER_WGM_FAST_PWM:
            advance_fast_pwm();
            break;
        case GENERIC_8BIT_TIMER_WGM_FAST_PWM_OCR_A:
            advance_fast_pwm_ocr_a();
            break;
        default:
            advance_illegal_wgm_state();
            break;
    }

    m_no_compare_this_cycle = false; // because the current timer cycle has ended
}

void avr8_generic_t0_timer_device::cs_w(uint8_t data)
{
    if (data == m_cs) return;

    // Switch timer source internally ONLY.
    // Timer 0 will always run in sync with the I/O clock, even
    // when switched to Tx pin triggers.
    m_cs = BIT(data, 0, 3);
}

void avr8_generic_t0_timer_device::tcnt_w(uint8_t data)
{
    // CPU writes override any compare matches for the current timer clock cycle
    m_tcnt = data;
    m_no_compare_this_cycle = true;
}


void avr8_megaavr_timer_0_device::tccr_b_w(uint8_t data)
{
    LOGMASKED(LOG_TIMER0, "tccr_b_w: %02x\n", data);

    if (BIT(data,7)) force_oc0a();
    if (BIT(data,6)) force_oc0b();
    cs_w(BIT(data,0,3));
    wgm_w(BIT(wgm_r(),0,2) | BIT(data,3) << 2);
}

uint8_t avr8_megaavr_timer_0_device::tccr_b_r()
{
    // the FOCxx bits always read as zero; they are never stored.
    return cs_r() | (BIT(wgm_r(),2) << 3);
}

///////////////////////////////////////////////////////////////
// GENERIC 16-BIT TIMER 1
///////////////////////////////////////////////////////////////

avr8_generic_timer_1_device::avr8_generic_timer_1_device(const machine_config &mconfig,
    const char *tag,
    device_t *owner,
    uint32_t clock) :
    avr8_generic_timer_1_device(mconfig, tag, owner, clock, AVR8_CLASSIC_GENERIC_TIMER_1)
{}

avr8_generic_timer_1_device::avr8_generic_timer_1_device(const machine_config &mconfig,
    const char *tag,
    device_t *owner,
    uint32_t clock,
    const device_type device_type) :
    avr8_base_timer_device(mconfig, device_type, tag, owner, clock)
{ }

avr8_megaavr_timer_1_device::avr8_megaavr_timer_1_device(const machine_config &mconfig,
        const char *tag,
        device_t *owner,
        uint32_t clock) :
    avr8_generic_timer_1_device(mconfig, tag, owner, clock, AVR8_CLASSIC_MEGAAVR_TIMER_1)
{}

void avr8_generic_timer_1_device::device_start()
{
    avr8_base_timer_device::device_start();
    save_item(NAME(m_icr));
}

void avr8_generic_timer_1_device::device_reset()
{
    avr8_base_timer_device::device_reset();
    m_icr = 0;
} 

void avr8_generic_timer_1_device::tick()
{
    // this routine very obviously copypasted from timer 0.
    // remember that the GENERIC_TIMER0_CS_xx and GENERIC_TIMER1_CS_xx enums
    // can't be commoned together.
    if (m_prtim) return;

    // Tx pin is sampled on every I/O clock, so check that first
    if (m_cs == GENERIC_TIMER1_CS_T1_FALL || m_cs == GENERIC_TIMER1_CS_T1_RISE)
    {
        sample_t_input();
    }

    if (m_cs == GENERIC_TIMER1_CS_DISABLED)
    {
        return;
    }

    if (m_cs == GENERIC_TIMER1_CS_PRESCALE_DIV8 && !(m_upstream_prescaler->taps_r() & avr8_10bit_prescaler_device::AVR8_PRESCALER_TAPMASK_DIV8)) return;
    if (m_cs == GENERIC_TIMER1_CS_PRESCALE_DIV64 && !(m_upstream_prescaler->taps_r() & avr8_10bit_prescaler_device::AVR8_PRESCALER_TAPMASK_DIV64)) return;
    if (m_cs == GENERIC_TIMER1_CS_PRESCALE_DIV256 && !(m_upstream_prescaler->taps_r() & avr8_10bit_prescaler_device::AVR8_PRESCALER_TAPMASK_DIV256)) return;
    if (m_cs == GENERIC_TIMER1_CS_PRESCALE_DIV1024 && !(m_upstream_prescaler->taps_r() & avr8_10bit_prescaler_device::AVR8_PRESCALER_TAPMASK_DIV1024)) return;
    if (m_cs == GENERIC_TIMER1_CS_T1_FALL && !m_t_falling) return;
    if (m_cs == GENERIC_TIMER1_CS_T1_RISE && !m_t_rising) return;
    // GENERIC_TIMER1_CS_DIRECT = just fall through

    if (m_ocr_changed && should_reload_ocrs())
    {
        reload_ocrs();
    }

    // LOGMASKED(LOG_TIMER1_TICK, "%s: running wgm callback %d\n", machine().describe_context(), m_wgm);
    switch (m_wgm)
    {
        case GENERIC_TIMER1_WGM_NORMAL:
            advance_normal();
            break;
            
        case GENERIC_TIMER1_WGM_PWM_8_BIT:
            advance_pwm_8bit();
            break;

        case GENERIC_TIMER1_WGM_PWM_9_BIT:
            advance_pwm_9bit();
            break;

        case GENERIC_TIMER1_WGM_PWM_10_BIT:
            advance_pwm_10bit();
            break;

        case GENERIC_TIMER1_WGM_CTC:
            advance_ctc_ocr_a();
            break;

        case GENERIC_TIMER1_WGM_FAST_PWM_8_BIT:
            advance_fast_pwm_8bit();
            break;
        
        case GENERIC_TIMER1_WGM_FAST_PWM_9_BIT:
            advance_fast_pwm_9bit();
            break;
    
        case GENERIC_TIMER1_WGM_FAST_PWM_10_BIT:
            advance_fast_pwm_10bit();
            break;
        
        case GENERIC_TIMER1_WGM_FAST_PWM_ICR1:
            advance_fast_pwm_icr();
            break;
        
        case GENERIC_TIMER1_WGM_FAST_PWM_OCRA:
            advance_fast_pwm_ocr_a();
            break;

        case GENERIC_TIMER1_WGM_PWM_OCR1A:
        case GENERIC_TIMER1_WGM_PWM_PFC_OCR1A:
            advance_pwm_ocr_a(); // same counter behavior; OCRxA reload behavior is the only difference
            break;

        case GENERIC_TIMER1_WGM_PWM_PFC_ICR1:
        case GENERIC_TIMER1_WGM_PWM_ICR1:
            advance_pwm_common(m_icr); // as with OCRxA mode, only difference is when OCRxA is reloaded
            break;

        case GENERIC_TIMER1_WGM_CTC_ICR1:
            advance_ctc_icr();
            break;
        
        default:
            advance_illegal_wgm_state();
            break;
    }

    m_no_compare_this_cycle = false; // because the current timer cycle has ended
}


void avr8_generic_timer_1_device::advance_normal()
{
    // this is basically the same as timer 0's advance_normal(), but 16-bit.
    // underlying framework will handle the wraparound for us,
    // but we still have to do compares ourself.
    if (is_compare_a())
    {
        raise_ocf_a();
        update_oc_generic<AVR8_TIMER_CMPCHANNEL_OC_A>();
    }

    if (is_compare_b())
    {
        raise_ocf_b();
        update_oc_generic<AVR8_TIMER_CMPCHANNEL_OC_B>();
    }

    if (m_ocr_c_enabled && is_compare_c())
    {
        raise_ocf_c();
        update_oc_generic<AVR8_TIMER_CMPCHANNEL_OC_C>();
    }

    // fall through to normal case, which will catch the wraparound and set TOV1
    advance_normal_generic();
}

void avr8_generic_timer_1_device::advance_pwm_common(uint16_t top)
{ 
    uint8_t next_count_value = m_dual_slope_counting_down ? prev_count() : next_count();
   
    if (is_compare_a())
    {
        raise_ocf_a();
        if (m_dual_slope_counting_down)
            update_oc_generic_notoggle<AVR8_TIMER_CMPCHANNEL_OC_A>();
        else
            update_oc_generic_notoggle_inverse<AVR8_TIMER_CMPCHANNEL_OC_A>();
    }

    if (is_compare_b())
    {
        raise_ocf_b();
        if (m_dual_slope_counting_down)
            update_oc_generic_notoggle<AVR8_TIMER_CMPCHANNEL_OC_B>();
        else
            update_oc_generic_notoggle_inverse<AVR8_TIMER_CMPCHANNEL_OC_B>();
    }

    if (m_ocr_c_enabled && is_compare_c())
    {
        raise_ocf_c();
        if (m_dual_slope_counting_down)
            update_oc_generic_notoggle<AVR8_TIMER_CMPCHANNEL_OC_C>();
        else
            update_oc_generic_notoggle_inverse<AVR8_TIMER_CMPCHANNEL_OC_C>();
    }

    if ((m_dual_slope_counting_down && will_prev_count_reach_bottom()) ||
        (!m_dual_slope_counting_down && next_count_value == top))
    {
        // toggle direction when about to hit TOP or BOTTOM
        m_dual_slope_counting_down = !m_dual_slope_counting_down;
    }
    else if (is_min_count())
    {
        raise_tov();
    }

    m_tcnt = next_count_value;
}

void avr8_generic_timer_1_device::advance_pwm_8bit()
{
    advance_pwm_common(0xff);
}

void avr8_generic_timer_1_device::advance_pwm_9bit()
{
    advance_pwm_common(0x1ff);
}

void avr8_generic_timer_1_device::advance_pwm_10bit()
{
    advance_pwm_common(0x3ff);
}

void avr8_generic_timer_1_device::advance_pwm_ocr_a()
{
    bool new_counting_direction = m_dual_slope_counting_down;
    if (is_compare_a())
    {
        raise_ocf_a();
        if (!m_dual_slope_counting_down)
        {
            update_oc_generic_notoggle_inverse<AVR8_TIMER_CMPCHANNEL_OC_A>();            
            new_counting_direction = true;
        }
        else
        {
            update_oc_generic_notoggle<AVR8_TIMER_CMPCHANNEL_OC_A>();
        }
    }

    if (is_compare_b())
    {
        raise_ocf_b();
        if (m_dual_slope_counting_down)
            update_oc_generic_notoggle<AVR8_TIMER_CMPCHANNEL_OC_B>();
        else
            update_oc_generic_notoggle_inverse<AVR8_TIMER_CMPCHANNEL_OC_B>();
    }

    if (m_ocr_c_enabled && is_compare_c())
    {
        raise_ocf_c();
        if (m_dual_slope_counting_down)
            update_oc_generic_notoggle<AVR8_TIMER_CMPCHANNEL_OC_C>();
        else
            update_oc_generic_notoggle_inverse<AVR8_TIMER_CMPCHANNEL_OC_C>();
    }

    if (m_dual_slope_counting_down && is_min_count())
    {
        new_counting_direction = false;
        raise_tov();
    }
   
    m_dual_slope_counting_down = new_counting_direction;
    m_tcnt = m_dual_slope_counting_down ? prev_count() : next_count();
}

void avr8_generic_timer_1_device::advance_fast_pwm_common(uint16_t top)
{
    bool will_increment = true;

    if (is_compare_a())
    {
        raise_ocf_a();
        update_oc_generic_notoggle<AVR8_TIMER_CMPCHANNEL_OC_A>();
    }

    if (is_compare_b())
    {
        raise_ocf_b();
        update_oc_generic_notoggle<AVR8_TIMER_CMPCHANNEL_OC_B>();
    }

    if (m_ocr_c_enabled && is_compare_c())
    {
        raise_ocf_c();
        update_oc_generic_notoggle<AVR8_TIMER_CMPCHANNEL_OC_C>();
    }

    if (is_min_count())
    {
        update_oc_generic_notoggle_inverse<AVR8_TIMER_CMPCHANNEL_OC_A>();
        update_oc_generic_notoggle_inverse<AVR8_TIMER_CMPCHANNEL_OC_B>();
        if (m_ocr_c_enabled) update_oc_generic_notoggle_inverse<AVR8_TIMER_CMPCHANNEL_OC_C>();
    }

    if ((m_tcnt & max_count()) == top)
    {
        raise_tov();
        m_tcnt = 0;
        will_increment = false;
    }

    if (will_increment) m_tcnt = next_count();
}

void avr8_generic_timer_1_device::advance_fast_pwm_8bit()
{
    advance_fast_pwm_common(0xff);
}

void avr8_generic_timer_1_device::advance_fast_pwm_9bit()
{
    advance_fast_pwm_common(0x1ff);
}

void avr8_generic_timer_1_device::advance_fast_pwm_10bit()
{
    advance_fast_pwm_common(0x3ff);
}

void avr8_generic_timer_1_device::advance_ctc_ocr_a()
{
    bool is_wrapping = false;

    if (will_next_count_overflow())
    {
        raise_tov();
    }

    if (is_compare_a())
    {
        is_wrapping = true;
        raise_ocf_a();
        update_oc_generic<AVR8_TIMER_CMPCHANNEL_OC_A>();
    }
    
    if (is_compare_b())
    {
        raise_ocf_b();
        update_oc_generic<AVR8_TIMER_CMPCHANNEL_OC_B>();
    }
    
    if (m_ocr_c_enabled && is_compare_c())
    {
        raise_ocf_c();
        update_oc_generic<AVR8_TIMER_CMPCHANNEL_OC_C>();
    }

    m_tcnt = is_wrapping ? 0 : next_count();
}


void avr8_generic_timer_1_device::advance_ctc_icr()
{
    bool is_wrapping = false;

    if (will_next_count_overflow())
    {
        raise_tov();
    }

    if (m_tcnt == m_icr)
    {
        is_wrapping = true;
        raise_icf();   
    }

    if (is_compare_a())
    {
        raise_ocf_a();
        update_oc_generic<AVR8_TIMER_CMPCHANNEL_OC_A>();
    }
    
    if (is_compare_b())
    {
        raise_ocf_b();
        update_oc_generic<AVR8_TIMER_CMPCHANNEL_OC_B>();
    }
    
    if (m_ocr_c_enabled && is_compare_c())
    {
        raise_ocf_c();
        update_oc_generic<AVR8_TIMER_CMPCHANNEL_OC_C>();
    }

    m_tcnt = is_wrapping ? 0 : next_count();
}

void avr8_generic_timer_1_device::advance_fast_pwm_ocr_a()
{
    bool is_wrapping = false;

    if (is_min_count())
    {
        update_oc_generic_notoggle_inverse<AVR8_TIMER_CMPCHANNEL_OC_A>();
        update_oc_generic_notoggle_inverse<AVR8_TIMER_CMPCHANNEL_OC_B>();
        if (m_ocr_c_enabled) update_oc_generic_notoggle_inverse<AVR8_TIMER_CMPCHANNEL_OC_C>();
    }

    if (is_compare_a())
    {
        raise_tov();
        raise_ocf_a();

        update_oc_generic<AVR8_TIMER_CMPCHANNEL_OC_A>();
        is_wrapping = true;
    }

    if (is_compare_b())
    {
        raise_ocf_b();
        update_oc_generic_notoggle<AVR8_TIMER_CMPCHANNEL_OC_B>();
    }
    
    if (m_ocr_c_enabled && is_compare_c())
    {
        raise_ocf_c();
        update_oc_generic_notoggle<AVR8_TIMER_CMPCHANNEL_OC_C>();
    }

    // in all other cases, just keep ticking the timer upwards
    m_tcnt = is_wrapping ? 0 : next_count();
}

void avr8_generic_timer_1_device::advance_fast_pwm_icr()
{
    advance_fast_pwm_common(m_icr);
}

bool avr8_generic_timer_1_device::com_mode_allows_output(uint8_t channel)
{
    // SET/CLEAR modes get their outputs enabled so just return in that case
    if (m_com[channel] == AVR8_TIMER_COM_SET || m_com[channel] == AVR8_TIMER_COM_CLEAR)
    {
        return true;
    }

    // Outputs will obviously be disabled in normal mode
    if (m_com[channel] == AVR8_TIMER_COM_NORMAL)
    {
        return false;
    }

    // In toggle mode, OCxA will output only when WGM = 9, 11, 14, 15;
    // at all other times it is disconnected
    if (channel == AVR8_TIMER_CMPCHANNEL_OC_A)
    {
        if (!(m_wgm == GENERIC_TIMER1_WGM_PWM_PFC_OCR1A ||
              m_wgm == GENERIC_TIMER1_WGM_PWM_OCR1A ||
              m_wgm == GENERIC_TIMER1_WGM_FAST_PWM_ICR1 ||
              m_wgm == GENERIC_TIMER1_WGM_FAST_PWM_OCRA))
        {
            logerror("%s: attempt to switch 16-bit timer COMxA to TOGGLE in a mode that doesn't support it (m_wgm=0x%02x)\n",
                machine().describe_context(),
                m_wgm);
            return false;
        }

        return true;
    }

    // OCxB and OCxC are disconnected in toggle mode for all WGM modes on this timer
    logerror("%s: attempt to switch 16-bit timer COMxx to TOGGLE on a channel that doesn't support it\n",
                machine().describe_context(),
                m_wgm);
    return false;
}

void avr8_generic_timer_1_device::wgm_w(uint8_t data)
{
    if (m_wgm == data) return;

    m_wgm = data;

    // a change of WGM means outputs must change,
    // so force COM rewrites so they change output states appropriately
    com_w<AVR8_TIMER_CMPCHANNEL_OC_A>(com_r<AVR8_TIMER_CMPCHANNEL_OC_A>());
    com_w<AVR8_TIMER_CMPCHANNEL_OC_B>(com_r<AVR8_TIMER_CMPCHANNEL_OC_B>());
    if (m_ocr_c_enabled) com_w<AVR8_TIMER_CMPCHANNEL_OC_C>(com_r<AVR8_TIMER_CMPCHANNEL_OC_C>());

    switch (m_wgm)
    {
        case GENERIC_TIMER1_WGM_PWM_8_BIT:
        case GENERIC_TIMER1_WGM_PWM_9_BIT:
        case GENERIC_TIMER1_WGM_PWM_10_BIT:
        case GENERIC_TIMER1_WGM_PWM_PFC_ICR1:
        case GENERIC_TIMER1_WGM_PWM_PFC_OCR1A:
        case GENERIC_TIMER1_WGM_PWM_OCR1A:
        case GENERIC_TIMER1_WGM_PWM_ICR1:
            m_dual_slope_counting_down = false; // guessed
            break;

        case GENERIC_TIMER1_WGM_NORMAL:
        case GENERIC_TIMER1_WGM_CTC:
        case GENERIC_TIMER1_WGM_FAST_PWM_8_BIT:
        case GENERIC_TIMER1_WGM_FAST_PWM_9_BIT:
        case GENERIC_TIMER1_WGM_FAST_PWM_10_BIT:
        case GENERIC_TIMER1_WGM_FAST_PWM_ICR1:
        case GENERIC_TIMER1_WGM_FAST_PWM_OCRA:
            // not sure of any sideeffects (yet)
            break;
        
        default:
            logerror("%s: switched to illegal/reserved timer mode %02x\n", machine().describe_context(), m_wgm);
            break;
    };
}

void avr8_generic_timer_1_device::cs_w(uint8_t data)
{
    if (data == m_cs) return;

    // Like Timer 0, this always runs in sync with the I/O clock,
    // and m_cs should contain the same structure.
    m_cs = BIT(data, 0, 3);
}

void avr8_generic_timer_1_device::tcnt_h_w(uint8_t data)
{
    m_temp = data;
}

uint8_t avr8_generic_timer_1_device::tcnt_h_r()
{
    return m_temp;
}

void avr8_generic_timer_1_device::tcnt_l_w(uint8_t data)
{
    m_tcnt = (m_temp << 8) | data;
    // LOGMASKED(LOG_TIMER1, "%s: tcnt changing to %04x\n", machine().describe_context(), m_tcnt);
}

uint8_t avr8_generic_timer_1_device::tcnt_l_r()
{
    if (!machine().side_effects_disabled())
        m_temp = BIT(m_tcnt, 8, 8);
    return BIT(m_tcnt, 0, 8);
}

template <uint8_t Channel>
void avr8_generic_timer_1_device::ocr_h_w(uint8_t data)
{
    // Channel doesn't matter here because data always ends up in the temp register.
    // however, you can use it for debuglogging.
    // LOGMASKED(LOG_TIMER1, "%s: OCRx%cH setting temp register to %02x\n", machine().describe_context(), 'A' + Channel, data);
    m_temp = data;
}

template void avr8_generic_timer_1_device::ocr_h_w<AVR8_TIMER_CMPCHANNEL_OC_A>(uint8_t data);
template void avr8_generic_timer_1_device::ocr_h_w<AVR8_TIMER_CMPCHANNEL_OC_B>(uint8_t data);
template void avr8_generic_timer_1_device::ocr_h_w<AVR8_TIMER_CMPCHANNEL_OC_C>(uint8_t data);

template <uint8_t Channel>
void avr8_generic_timer_1_device::ocr_l_w(uint8_t data)
{
    m_ocr_buf[Channel] = (m_temp << 8) | data;
    m_ocr_changed = true;
    // LOGMASKED(LOG_TIMER1, "%s: OCR %c switching to %04x\n", machine().describe_context(), 'A' + Channel, m_ocr_buf[Channel]);
}

template void avr8_generic_timer_1_device::ocr_l_w<AVR8_TIMER_CMPCHANNEL_OC_A>(uint8_t data);
template void avr8_generic_timer_1_device::ocr_l_w<AVR8_TIMER_CMPCHANNEL_OC_B>(uint8_t data);
template void avr8_generic_timer_1_device::ocr_l_w<AVR8_TIMER_CMPCHANNEL_OC_C>(uint8_t data);

void avr8_generic_timer_1_device::icr_h_w(uint8_t data)
{
    m_temp = data;
}

uint8_t avr8_generic_timer_1_device::icr_h_r()
{
    return m_temp;
}

void avr8_generic_timer_1_device::icr_l_w(uint8_t data)
{
    m_icr = (m_temp << 8) | data;
    LOGMASKED(LOG_TIMER1, "%s: ICR switching to %04x\n", machine().describe_context(), m_icr);
}

uint8_t avr8_generic_timer_1_device::icr_l_r()
{
    if (!machine().side_effects_disabled())
        m_temp = BIT(m_icr, 8, 8);
    return BIT(m_icr, 0, 8);
}

void avr8_megaavr_timer_1_device::tccr_b_w(uint8_t data)
{
    cs_w(BIT(data,0,3));
    wgm_w(BIT(wgm_r(), 0, 2) | (BIT(data, 3, 2) << 2));

    if (BIT(data, 7))
    {
        // TODO: bit 7 = ICNC1 (noise canceller)
        logerror("%s (%s): TCCRxB: attempt to turn on noise canceller (not supported yet)\n",
                 machine().describe_context(),
                 tag());
    }

    if (BIT(data, 6))
    {
        // TODO: bit 6 = ICES1 (what edge of ICP1 triggers the capture event)
        logerror("%s (%s): TCCRxB: attempt to set ICES1 to rising edge (not supported yet)\n",
                 machine().describe_context(),
                 tag());
    }
}

uint8_t avr8_megaavr_timer_1_device::tccr_b_r()
{
    // bits 0,1,2 = CS
    // bits 3,4 = WGMx3,2
    return 
        BIT(cs_r(), 0, 3) |
        (BIT(wgm_r(), 2, 2) << 3);

    // TODO: bit 7 = ICNC1 (noise canceller)
    // TODO: bit 6 = ICES1
}

void avr8_megaavr_timer_1_device::tccr_c_w(uint8_t data)
{
    // TODO: FOC1A, FOC1B, FOC1C
}

uint8_t avr8_megaavr_timer_1_device::tccr_c_r()
{
    // all zero; FOCxA, FOCxB, FOCxC bits will flip to 0 when written
    return 0;
}

bool avr8_generic_timer_1_device::should_reload_ocrs()
{
    // See https://ww1.microchip.com/downloads/en/DeviceDoc/ATmega48A-PA-88A-PA-168A-PA-328-P-DS-DS40002061A.pdf
    // Table 16-4.
    switch (m_wgm)
    {
        case GENERIC_TIMER1_WGM_NORMAL:
        case GENERIC_TIMER1_WGM_CTC:
        case GENERIC_TIMER1_WGM_CTC_ICR1:
            return true;

        case GENERIC_TIMER1_WGM_PWM_8_BIT:
            return (m_tcnt & max_count()) == 0xff; // update at TOP (=0x00ff)

        case GENERIC_TIMER1_WGM_PWM_9_BIT:
            return (m_tcnt & max_count()) == 0x1ff; // update at TOP (=0x01ff)

        case GENERIC_TIMER1_WGM_PWM_10_BIT:
            return (m_tcnt & max_count()) == 0x3ff; // update at TOP (=0x03ff)

        case GENERIC_TIMER1_WGM_PWM_ICR1:
            return false; // (m_tcnt & max_count()) == m_icr; // update at TOP (=ICR)

        case GENERIC_TIMER1_WGM_PWM_OCR1A:
            return (m_tcnt & max_count()) == m_ocr[AVR8_TIMER_CMPCHANNEL_OC_A]; // update at TOP (=OCRxA)

        case GENERIC_TIMER1_WGM_PWM_PFC_ICR1:
        case GENERIC_TIMER1_WGM_PWM_PFC_OCR1A:
        case GENERIC_TIMER1_WGM_FAST_PWM_8_BIT:
        case GENERIC_TIMER1_WGM_FAST_PWM_9_BIT:
        case GENERIC_TIMER1_WGM_FAST_PWM_10_BIT:
        case GENERIC_TIMER1_WGM_FAST_PWM_ICR1:
        case GENERIC_TIMER1_WGM_FAST_PWM_OCRA:
            return is_min_count(); // update at BOTTOM (=0)
        
        default:
            return false;
    }
}

///////////////////////////////////////////////////////////////
// FAKE RTC TIMER 2
///////////////////////////////////////////////////////////////

avr8_generic_fakertc_timer_device::avr8_generic_fakertc_timer_device(const machine_config &mconfig,
    const char *tag,
    device_t *owner,
    uint32_t clock,
    const device_type device_type) :
    avr8_generic_8bit_timer_device(mconfig, tag, owner, clock, device_type)
{ }


avr8_generic_fakertc_timer_device::avr8_generic_fakertc_timer_device(const machine_config &mconfig,
    const char *tag,
    device_t *owner,
    uint32_t clock) :
    avr8_generic_fakertc_timer_device(mconfig, tag, owner, clock, AVR8_CLASSIC_GENERIC_FAKE_RTC_TIMER)
{ }

void avr8_generic_fakertc_timer_device::tick()
{
    // TODO: this is WRONG. PRTIM is ignored if the timer is in asynchronous mode
    if (m_prtim) return;

    if (m_cs == GENERIC_FAKERTC_CS_DISABLED)
    {
        return;
    }

    if (m_cs != GENERIC_FAKERTC_CS_DIRECT)
    {
        if (m_cs == GENERIC_FAKERTC_CS_PRESCALE_DIV8 && !(m_upstream_prescaler->taps_r() & avr8_10bit_prescaler_device::AVR8_PRESCALER_TAPMASK_DIV8)) return;
        if (m_cs == GENERIC_FAKERTC_CS_PRESCALE_DIV32 && !(m_upstream_prescaler->taps_r() & avr8_10bit_prescaler_device::AVR8_PRESCALER_TAPMASK_DIV32)) return;
        if (m_cs == GENERIC_FAKERTC_CS_PRESCALE_DIV64 && !(m_upstream_prescaler->taps_r() & avr8_10bit_prescaler_device::AVR8_PRESCALER_TAPMASK_DIV64)) return;
        if (m_cs == GENERIC_FAKERTC_CS_PRESCALE_DIV128 && !(m_upstream_prescaler->taps_r() & avr8_10bit_prescaler_device::AVR8_PRESCALER_TAPMASK_DIV128)) return;
        if (m_cs == GENERIC_FAKERTC_CS_PRESCALE_DIV256 && !(m_upstream_prescaler->taps_r() & avr8_10bit_prescaler_device::AVR8_PRESCALER_TAPMASK_DIV256)) return;
        if (m_cs == GENERIC_FAKERTC_CS_PRESCALE_DIV1024 && !(m_upstream_prescaler->taps_r() & avr8_10bit_prescaler_device::AVR8_PRESCALER_TAPMASK_DIV1024)) return;
    }

    if (m_ocr_changed && should_reload_ocrs())
    {
        reload_ocrs();
    }

    // TODO: unify this and generic timer0
    switch(m_wgm)
    {
        case GENERIC_8BIT_TIMER_WGM_NORMAL:
            advance_normal();
            break;
        case GENERIC_8BIT_TIMER_WGM_CTC:
            advance_ctc();
            break;
        case GENERIC_8BIT_TIMER_WGM_PWM:
            advance_pwm();
            break;
        case GENERIC_8BIT_TIMER_WGM_FAST_PWM:
            advance_fast_pwm();
            break;
        case GENERIC_8BIT_TIMER_WGM_FAST_PWM_OCR_A:
            advance_fast_pwm_ocr_a();
            break;
        default:
            advance_illegal_wgm_state();
            break;
    }

    m_no_compare_this_cycle = false; // because the current timer cycle has ended
}

void avr8_generic_fakertc_timer_device::cs_w(uint8_t data)
{
    // same workflow as Timer 0, but remember that the CS enum is different
    // and a debuglog function here would need to print different strings
    if (data == m_cs) return;

    m_cs = BIT(data, 0, 3);
}

void avr8_generic_fakertc_timer_device::tcnt_w(uint8_t data)
{
    // CPU writes override any compare matches for the current timer clock cycle
    m_tcnt = data;
    m_no_compare_this_cycle = true;
}

void avr8_megaavr_fakertc_timer_device::tccr_b_w(uint8_t data)
{
    if (BIT(data,7)) force_oc0a();
    if (BIT(data,6)) force_oc0b();
    cs_w(BIT(data,0,3));
    wgm_w(BIT(wgm_r(),0,2) | BIT(data,3) << 2);
}

uint8_t avr8_megaavr_fakertc_timer_device::tccr_b_r()
{
    // the FOCxx bits always read as zero; they are never stored.
    return cs_r() | (BIT(wgm_r(),2) << 3);
}

avr8_megaavr_fakertc_timer_device::avr8_megaavr_fakertc_timer_device(const machine_config &mconfig,
    const char *tag,
    device_t *owner,
    uint32_t clock) :
    avr8_generic_fakertc_timer_device(mconfig, tag, owner, clock, AVR8_CLASSIC_MEGAAVR_FAKE_RTC_TIMER)
{ }

///////////////////////////////////////////////////////////////
// FAKE RTC ASYNCHRONOUS CLOCK MODULE
///////////////////////////////////////////////////////////////

avr8_fakertc_async_controller_device::avr8_fakertc_async_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
    avr8_fakertc_async_controller_device(mconfig, tag, owner, clock, AVR8_CLASSIC_FAKE_RTC_ASYNC_CONTROLLER)
    // device_t(mconfig, AVR8_CLASSIC_FAKE_RTC_ASYNC_CONTROLLER, tag, owner, clock),
    // m_prescaler(*this, "prescaler"),
    // m_child_timer(*this, "atimer2"),
    // m_external_crystal_period(attotime::never)
{}

avr8_fakertc_async_controller_device::avr8_fakertc_async_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const device_type device_type) :
    device_t(mconfig, device_type, tag, owner, clock),
    m_prescaler(*this, "prescaler"),
    m_child_timer(*this, "atimer2"),
    m_external_crystal_period(attotime::never)
{}

void avr8_fakertc_async_controller_device::device_add_mconfig(machine_config &config)
{
    AVR8_CLASSIC_10BIT_PRESCALER(config, m_prescaler, 0);
}

void avr8_fakertc_async_controller_device::device_start()
{
    save_item(NAME(m_external_crystal_period));
    save_item(NAME(m_as2));
    save_item(NAME(m_exclk));
    save_item(NAME(m_tosc_last_state));
    save_item(NAME(m_clk_io));
    save_item(NAME(m_exclk_from_crystal));
    m_external_crystal_timer = timer_alloc(FUNC(avr8_fakertc_async_controller_device::external_crystal_tick), this);
}

void avr8_fakertc_async_controller_device::device_reset()
{
    m_exclk = false;
    m_as2 = false;
    m_tosc_last_state = false;
    m_clk_io = false;
}

void avr8_fakertc_async_controller_device::clk_io_w(uint8_t data)
{
    bool new_state = data != 0;
    if (new_state == m_clk_io) return;
    m_clk_io = new_state;

    // in synchronous mode, the rising edge of the clock ticks the timer
    if (!m_as2 && m_clk_io)
    {
        m_prescaler->tick();
        m_child_timer->tick();
        return;
    }
    
    // TODO: determine asynchronous "busy" flags for the timer registers
    // and how many cycles it takes from when a write happens to when
    // those flags clear.
}

void avr8_fakertc_async_controller_device::tosc_w(uint8_t data)
{
    bool new_state = data != 0;
    if (new_state == m_tosc_last_state) return;
    m_tosc_last_state = new_state;

    if (m_as2 && !using_external_crystal() && m_tosc_last_state)
    {
        m_prescaler->tick();
        m_child_timer->tick();
    }
}

void avr8_fakertc_async_controller_device::set_external_crystal_frequency(attotime period)
{
    m_external_crystal_period = period;
}

void avr8_fakertc_async_controller_device::as2_w(uint8_t data)
{
    m_as2 = data != 0;
    update_crystal_state();
}

void avr8_fakertc_async_controller_device::exclk_w(uint8_t data)
{
    m_exclk = data != 0;
    update_crystal_state();
}

void avr8_fakertc_async_controller_device::update_crystal_state()
{
    m_external_crystal_timer->adjust(using_external_crystal() ? m_external_crystal_period : attotime::never);
}

TIMER_CALLBACK_MEMBER(avr8_fakertc_async_controller_device::external_crystal_tick)
{
    if (!using_external_crystal()) return;

    m_prescaler->tick();
    m_child_timer->tick();

    update_crystal_state();
}

avr8_megaavr_fakertc_async_controller_device::avr8_megaavr_fakertc_async_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
    avr8_fakertc_async_controller_device(mconfig, tag, owner, clock, AVR8_CLASSIC_MEGAAVR_FAKE_RTC_ASYNC_CONTROLLER)
{}

void avr8_megaavr_fakertc_async_controller_device::assr_w(uint8_t data)
{
    as2_w(BIT(data, 5));
    exclk_w(BIT(data, 6));
}

uint8_t avr8_megaavr_fakertc_async_controller_device::assr_r()
{
    // TODO: update busy bits
    // - bit 0: TCR2BUB
    // - bit 1: TCR2AUB
    // - bit 2: OCR2BUB
    // - bit 3: OCR2AUB
    // - bit 4: TCN2UB
    return (m_as2 << 5) | (m_exclk << 6);
}
