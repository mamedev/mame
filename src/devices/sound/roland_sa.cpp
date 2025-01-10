// license:BSD-3-Clause
// copyright-holders:giulioz

/*
    Emulator for the gate arrays found in the Roland CPU-B board of SA-synthesis digital pianos.
    Reverse engineering done from silicon images.
    - IC19 R06-0001 (Fujitsu MB60VH142)
    - IC9  R06-0002 (Fujitsu MB60V141)
    - IC8  R06-0003 (Fujitsu MB61V125)

    The system is essentially a sample player, which can play 16 voices, each one with 10 sample parts.

    IC19 controls the CPU->RAM interface and computes the volume envelopes.
    The CPU can control the envelopes by specifying 8 bits of destination and 8 bits of speed (bit 7 is polarity).
    Every time the envelope reaches a destination, an IRQ is raised and the CPU can read which voice/part triggered it.

    IC9 is a phase accumulator, which can take 16 bit of playback speed value and generate an address for the wave rom.
    The upper part of the wave rom address is fixed, making the samples at most 2048 long.

    IC8 sums the 160 parts together into a buffer with 8 samples, performing interpolation and volume control.
    Volume control is done in exponential space to avoid multiplication. The samples are in exponential format.
*/

#include "emu.h"
#include "roland_sa.h"

DEFINE_DEVICE_TYPE(ROLAND_SA, roland_sa_device, "roland_sa", "Roland SA CPU-B Sound Generator")

roland_sa_device::roland_sa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
    : device_t(mconfig, ROLAND_SA, tag, owner, clock)
    , device_sound_interface(mconfig, *this)
    , m_int_callback(*this)
    , m_stream(nullptr)
{
}

void roland_sa_device::device_start()
{
    m_stream = stream_alloc(0, 2, 20000, STREAM_SYNCHRONOUS);
}

void roland_sa_device::device_reset()
{
    m_int_callback(CLEAR_LINE);

    m_irq_id = 0;
    m_irq_triggered = false;
    memset(m_parts, 0, sizeof(m_parts));
}

void roland_sa_device::set_sr_mode(bool mode)
{
    if (m_sr_mode != mode)
        m_stream->set_sample_rate(mode ? 20000 : 32000);
    m_sr_mode = mode;
}

void roland_sa_device::load_roms(uint8_t *ic5, uint8_t *ic6, uint8_t *ic7)
{
    // Exp table to for the subphase
    // TODO: This is bit accurate, but I want to believe there is a better way to compute this function
    for (size_t i = 0; i < 0x10000; i++)
    {
        // ROM IC11
        uint16_t r11_pos = i % 4096;
        uint16_t r11 = (uint16_t)round(exp2f(13.0 + r11_pos / 4096.0) - 4096 * 2);
        bool r11_12 = !((r11 >> 12) & 1);
        bool r11_11 = !((r11 >> 11) & 1);
        bool r11_10 = !((r11 >> 10) & 1);
        bool r11_9 =  !((r11 >> 9) & 1);
        bool r11_8 =  !((r11 >> 8) & 1);
        bool r11_7 =  !((r11 >> 7) & 1);
        bool r11_6 =  !((r11 >> 6) & 1);
        bool r11_5 =  !((r11 >> 5) & 1);
        bool r11_4 =  (r11 >> 4) & 1;
        bool r11_3 =  (r11 >> 3) & 1;
        bool r11_2 =  (r11 >> 2) & 1;
        bool r11_1 =  (r11 >> 1) & 1;
        bool r11_0 =  (r11 >> 0) & 1;

        uint8_t param_bus_0 = ((i / 0x1000) >> 0) & 1;
        uint8_t param_bus_1 = ((i / 0x1000) >> 1) & 1;
        uint8_t param_bus_2 = ((i / 0x1000) >> 2) & 1;
        uint8_t param_bus_3 = ((i / 0x1000) >> 3) & 1;

        // Copy pasted from silicon
        bool result_b0 = (!r11_6 && !param_bus_0 && !param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_5 && param_bus_0 && !param_bus_1 && !param_bus_2 && !param_bus_3) || (r11_4 && !param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (r11_3 && param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (r11_2 && !param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (r11_1 && param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (r11_0 && !param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) || (param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3 && param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3);
        bool result_b1 = (!r11_7 && !param_bus_0 && !param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_6 && param_bus_0 && !param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_5 && !param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (r11_4 && param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (r11_3 && !param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (r11_2 && param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (r11_1 && !param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) || (r11_0 && param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3);
        bool result_b2 = !(!((!r11_8 && !param_bus_0 && !param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_7 && param_bus_0 && !param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_6 && !param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_5 && param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (r11_4 && !param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (r11_3 && param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (r11_2 && !param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) || (r11_1 && param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3)) && !(r11_0 && !param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3));
        bool result_b3 = !(!((!r11_9 && !param_bus_0 && !param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_8 && param_bus_0 && !param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_7 && !param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_6 && param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_5 && !param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (r11_4 && param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (r11_3 && !param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) || (r11_2 && param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3)) && !((r11_1 && !param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (r11_0 && param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3)));
        bool result_b4 = !(!((!r11_10 && !param_bus_0 && !param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_9 && param_bus_0 && !param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_8 && !param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_7 && param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_6 && !param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_5 && param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (r11_4 && !param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) || (r11_3 && param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3)) && !((r11_2 && !param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (r11_1 && param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (r11_0 && !param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3) || (0 && param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3)));
        bool result_b5 = !(!((!r11_11 && !param_bus_0 && !param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_10 && param_bus_0 && !param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_9 && !param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_8 && param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_7 && !param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_6 && param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_5 && !param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) || (r11_4 && param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3)) && !((r11_3 && !param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (r11_2 && param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (r11_1 && !param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3) || (r11_0 && param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3)));
        bool result_b6 = !(!((!r11_12 && !param_bus_0 && !param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_11 && param_bus_0 && !param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_10 && !param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_9 && param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_8 && !param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_7 && param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_6 && !param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_5 && param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3)) && !((r11_4 && !param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (r11_3 && param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (r11_2 && !param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3) || (r11_1 && param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3)));
        bool result_b7 = !(!((1 && !param_bus_0 && !param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_12 && param_bus_0 && !param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_11 && !param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_10 && param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_9 && !param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_8 && param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_7 && !param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_6 && param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3)) && !((!r11_5 && !param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (r11_4 && param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (r11_3 && !param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3) || (r11_2 && param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3)));
        bool result_b8 = !(!((0 && !param_bus_0 && !param_bus_1 && !param_bus_2 && !param_bus_3) || (1 && param_bus_0 && !param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_12 && !param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_11 && param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_10 && !param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_9 && param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_8 && !param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_7 && param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3)) && !((!r11_6 && !param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (!r11_5 && param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (r11_4 && !param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3) || (r11_3 && param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3)));
        bool result_b9 = !(!((1 && !param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_12 && param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_11 && !param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_10 && param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_9 && !param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_8 && param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_7 && !param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (!r11_6 && param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3)) && !((!r11_5 && !param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3) || (r11_4 && param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3)));
        bool result_b10 = !(!((1 && param_bus_0 && param_bus_1 && !param_bus_2 && !param_bus_3) || (!r11_12 && !param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_11 && param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_10 && !param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_9 && param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_8 && !param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (!r11_7 && param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (!r11_6 && !param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3)) && !(!r11_5 && param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3));
        bool result_b11 = (1 && !param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_12 && param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_11 && !param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_10 && param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_9 && !param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (!r11_8 && param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (!r11_7 && !param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3) || (!r11_6 && param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3);
        bool result_b12 = (0 && !param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (1 && param_bus_0 && !param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_12 && !param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_11 && param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_10 && !param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (!r11_9 && param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (!r11_8 && !param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3) || (!r11_7 && param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3);
        bool result_b13 = (1 && !param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_12 && param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) || (!r11_11 && !param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (!r11_10 && param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) || (!r11_9 && !param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3) || (!r11_8 && param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3);
        bool result_b14 = !(1 && !(1 && param_bus_0 && param_bus_1 && param_bus_2 && !param_bus_3) && !(!r11_12 && !param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) && !(!r11_11 && param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) && !(!r11_10 && !param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3) && !(!r11_9 && param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3));
        bool result_b15 = !(!(!param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) && !(!r11_12 && param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) && !(!r11_11 && !param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3) && !(!r11_10 && param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3));
        bool result_b16 = !(!(param_bus_0 && !param_bus_1 && !param_bus_2 && param_bus_3) && !(!r11_12 && !param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3) && !(!r11_11 && param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3));
        bool result_b17 = !(!(!param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3) && !(!r11_12 && param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3));
        bool result_b18 = param_bus_0 && param_bus_1 && !param_bus_2 && param_bus_3;
        
        uint32_t result =
            result_b18 << 18 | result_b17 << 17 | result_b16 << 16 | result_b15 << 15 | result_b14 << 14 | result_b13 << 13 |
            result_b12 << 12 | result_b11 << 11 | result_b10 << 10 | result_b9 << 9 | result_b8 << 8 | result_b7 << 7 |
            result_b6 << 6 | result_b5 << 5 | result_b4 << 4 | result_b3 << 3 | result_b2 << 2 | result_b1 << 1 | result_b0 << 0;
        phase_exp_table[i] = result;
    }

    // Exp table to decode samples
    // TODO: This is bit accurate, but I want to believe there is a better way to compute this function
    for (size_t i = 0; i < 0x8000; i++)
    {
        // ROM IC10
        uint16_t r10_pos = i % 1024;
        uint16_t r10 = (uint16_t)round(exp2f(11.0 + ~r10_pos / 1024.0) - 1024);
        bool r10_9 = (r10 >> 0) & 1;
        bool r10_8 = (r10 >> 1) & 1;
        bool r10_0 = (r10 >> 2) & 1;
        bool r10_1 = (r10 >> 3) & 1;
        bool r10_2 = (r10 >> 4) & 1;
        bool r10_3 = !((r10 >> 5) & 1);
        bool r10_4 = !((r10 >> 6) & 1);
        bool r10_5 = !((r10 >> 7) & 1);
        bool r10_6 = !((r10 >> 8) & 1);
        bool r10_7 = !((r10 >> 9) & 1);

        bool wavein_sign = i >= 0x4000;
        uint8_t add_r_0 = ((i / 0x400) >> 0) & 1;
        uint8_t add_r_1 = ((i / 0x400) >> 1) & 1;
        uint8_t add_r_2 = ((i / 0x400) >> 2) & 1;
        uint8_t add_r_3 = ((i / 0x400) >> 3) & 1;

        // Copy pasted from silicon
        bool result_b14 = !((!(!add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) && !wavein_sign) || (!add_r_3 && !add_r_2 && !add_r_1 && !add_r_0 && wavein_sign));
        bool result_b13 = !((((!r10_7 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (!add_r_3 && !add_r_2 && !add_r_1 && add_r_0)) && wavein_sign) || (!((!r10_7 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (!add_r_3 && !add_r_2 && !add_r_1 && add_r_0)) && !wavein_sign));
        bool result_b12 = !((((!r10_6 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (!r10_7 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (!add_r_3 && !add_r_2 && add_r_1 && !add_r_0)) && wavein_sign) || (!((!r10_6 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (!r10_7 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (!add_r_3 && !add_r_2 && add_r_1 && !add_r_0)) && !wavein_sign));
        bool result_b11 = !((((!r10_5 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (!r10_6 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (!r10_7 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (1 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0)) && wavein_sign) || (!((!r10_5 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (!r10_6 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (!r10_7 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (1 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0)) && !wavein_sign));
        bool result_b10 = !((!((!r10_7 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (!r10_6 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (!r10_5 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (!r10_4 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0)) && !(!add_r_3 && add_r_2 && !add_r_1 && !add_r_0) && !wavein_sign) || (!(!((!r10_7 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (!r10_6 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (!r10_5 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (!r10_4 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0)) && !(!add_r_3 && add_r_2 && !add_r_1 && !add_r_0)) && wavein_sign));
        bool result_b9 = !((((1 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (!r10_7 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (!r10_6 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (!r10_5 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (!r10_4 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (!r10_3 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0)) && wavein_sign) || (!((1 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (!r10_7 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (!r10_6 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (!r10_5 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (!r10_4 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (!r10_3 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0)) && !wavein_sign));
        bool result_b8 = !((((1 && !add_r_3 && add_r_2 && add_r_1 && !add_r_0) || (!r10_7 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (!r10_6 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (!r10_5 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (!r10_4 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (!r10_3 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (r10_2 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (1 && 0)) && wavein_sign) || (!((1 && !add_r_3 && add_r_2 && add_r_1 && !add_r_0) || (!r10_7 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (!r10_6 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (!r10_5 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (!r10_4 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (!r10_3 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (r10_2 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (1 && 0)) && !wavein_sign));
        bool result_b7 = !((((1 && !add_r_3 && add_r_2 && add_r_1 && add_r_0) || (!r10_7 && !add_r_3 && add_r_2 && add_r_1 && !add_r_0) || (!r10_6 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (!r10_5 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (!r10_4 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (!r10_3 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (r10_2 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (r10_1 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0)) && wavein_sign) || (!((1 && !add_r_3 && add_r_2 && add_r_1 && add_r_0) || (!r10_7 && !add_r_3 && add_r_2 && add_r_1 && !add_r_0) || (!r10_6 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (!r10_5 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (!r10_4 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (!r10_3 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (r10_2 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (r10_1 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0)) && !wavein_sign));
        bool result_b6 = !((!((1 && add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (!r10_7 && !add_r_3 && add_r_2 && add_r_1 && add_r_0) || (!r10_6 && !add_r_3 && add_r_2 && add_r_1 && !add_r_0) || (!r10_5 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (!r10_4 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (!r10_3 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (r10_2 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (r10_1 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0)) && !(r10_0 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) && !wavein_sign) || (!(!((1 && add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (!r10_7 && !add_r_3 && add_r_2 && add_r_1 && add_r_0) || (!r10_6 && !add_r_3 && add_r_2 && add_r_1 && !add_r_0) || (!r10_5 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (!r10_4 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (!r10_3 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (r10_2 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (r10_1 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0)) && !(r10_0 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0)) && wavein_sign));
        bool result_b5 = !((!((!r10_7 && add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (!r10_6 && !add_r_3 && add_r_2 && add_r_1 && add_r_0) || (!r10_5 && !add_r_3 && add_r_2 && add_r_1 && !add_r_0) || (!r10_4 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (!r10_3 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (r10_2 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (r10_1 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (r10_0 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0)) && !((r10_9 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (add_r_3 && !add_r_2 && !add_r_1 && add_r_0)) && !wavein_sign) || (!(!((!r10_7 && add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (!r10_6 && !add_r_3 && add_r_2 && add_r_1 && add_r_0) || (!r10_5 && !add_r_3 && add_r_2 && add_r_1 && !add_r_0) || (!r10_4 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (!r10_3 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (r10_2 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (r10_1 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (r10_0 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0)) && !((r10_9 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (add_r_3 && !add_r_2 && !add_r_1 && add_r_0))) && wavein_sign));
        bool result_b4 = !((!((r10_8 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (r10_9 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (r10_0 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (r10_1 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (r10_2 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (!r10_3 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (!r10_4 && !add_r_3 && add_r_2 && add_r_1 && !add_r_0) || (!r10_5 && !add_r_3 && add_r_2 && add_r_1 && add_r_0)) && !((!r10_6 && add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (!r10_7 && add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (add_r_3 && !add_r_2 && add_r_1 && !add_r_0)) && !wavein_sign) || (!(!((r10_8 && !add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (r10_9 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (r10_0 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (r10_1 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (r10_2 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (!r10_3 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (!r10_4 && !add_r_3 && add_r_2 && add_r_1 && !add_r_0) || (!r10_5 && !add_r_3 && add_r_2 && add_r_1 && add_r_0)) && !((!r10_6 && add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (!r10_7 && add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (add_r_3 && !add_r_2 && add_r_1 && !add_r_0))) && wavein_sign));
        bool result_b3 = !((!((r10_8 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (r10_9 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (r10_0 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (r10_1 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (r10_2 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (!r10_3 && !add_r_3 && add_r_2 && add_r_1 && !add_r_0) || (!r10_4 && !add_r_3 && add_r_2 && add_r_1 && add_r_0) || (!r10_5 && add_r_3 && !add_r_2 && !add_r_1 && !add_r_0)) && !((!r10_6 && add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (!r10_7 && add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (add_r_3 && !add_r_2 && add_r_1 && add_r_0)) && !wavein_sign) || (!(!((r10_8 && !add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (r10_9 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (r10_0 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (r10_1 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (r10_2 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (!r10_3 && !add_r_3 && add_r_2 && add_r_1 && !add_r_0) || (!r10_4 && !add_r_3 && add_r_2 && add_r_1 && add_r_0) || (!r10_5 && add_r_3 && !add_r_2 && !add_r_1 && !add_r_0)) && !((!r10_6 && add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (!r10_7 && add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (add_r_3 && !add_r_2 && add_r_1 && add_r_0))) && wavein_sign));
        bool result_b2 = !((!((r10_8 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (r10_9 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (r10_0 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (r10_1 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (r10_2 && !add_r_3 && add_r_2 && add_r_1 && !add_r_0) || (!r10_3 && !add_r_3 && add_r_2 && add_r_1 && add_r_0) || (!r10_4 && add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (!r10_5 && add_r_3 && !add_r_2 && !add_r_1 && add_r_0)) && !((!r10_6 && add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (!r10_7 && add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (add_r_3 && add_r_2 && !add_r_1 && !add_r_0)) && !wavein_sign) || (!(!((r10_8 && !add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (r10_9 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (r10_0 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (r10_1 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (r10_2 && !add_r_3 && add_r_2 && add_r_1 && !add_r_0) || (!r10_3 && !add_r_3 && add_r_2 && add_r_1 && add_r_0) || (!r10_4 && add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (!r10_5 && add_r_3 && !add_r_2 && !add_r_1 && add_r_0)) && !((!r10_6 && add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (!r10_7 && add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (add_r_3 && add_r_2 && !add_r_1 && !add_r_0))) && wavein_sign));
        bool result_b1 = !((!((r10_8 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (r10_9 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (r10_0 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (r10_1 && !add_r_3 && add_r_2 && add_r_1 && !add_r_0) || (r10_2 && !add_r_3 && add_r_2 && add_r_1 && add_r_0) || (!r10_3 && add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (!r10_4 && add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (!r10_5 && add_r_3 && !add_r_2 && add_r_1 && !add_r_0)) && !((!r10_6 && add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (!r10_7 && add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (add_r_3 && add_r_2 && !add_r_1 && add_r_0)) && !wavein_sign) || (!(!((r10_8 && !add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (r10_9 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (r10_0 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (r10_1 && !add_r_3 && add_r_2 && add_r_1 && !add_r_0) || (r10_2 && !add_r_3 && add_r_2 && add_r_1 && add_r_0) || (!r10_3 && add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (!r10_4 && add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (!r10_5 && add_r_3 && !add_r_2 && add_r_1 && !add_r_0)) && !((!r10_6 && add_r_3 && !add_r_2 && add_r_1 && add_r_0) || (!r10_7 && add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (add_r_3 && add_r_2 && !add_r_1 && add_r_0))) && wavein_sign));
        bool result_b0 = !((!((r10_8 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (r10_9 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (r10_0 && !add_r_3 && add_r_2 && add_r_1 && !add_r_0) || (r10_1 && !add_r_3 && add_r_2 && add_r_1 && add_r_0) || (r10_2 && add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (!r10_3 && add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (!r10_4 && add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (!r10_5 && add_r_3 && !add_r_2 && add_r_1 && add_r_0)) && !((!r10_6 && add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (!r10_7 && add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (add_r_3 && add_r_2 && add_r_1 && !add_r_0)) && !wavein_sign) || (!(!((r10_8 && !add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (r10_9 && !add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (r10_0 && !add_r_3 && add_r_2 && add_r_1 && !add_r_0) || (r10_1 && !add_r_3 && add_r_2 && add_r_1 && add_r_0) || (r10_2 && add_r_3 && !add_r_2 && !add_r_1 && !add_r_0) || (!r10_3 && add_r_3 && !add_r_2 && !add_r_1 && add_r_0) || (!r10_4 && add_r_3 && !add_r_2 && add_r_1 && !add_r_0) || (!r10_5 && add_r_3 && !add_r_2 && add_r_1 && add_r_0)) && !((!r10_6 && add_r_3 && add_r_2 && !add_r_1 && !add_r_0) || (!r10_7 && add_r_3 && add_r_2 && !add_r_1 && add_r_0) || (add_r_3 && add_r_2 && add_r_1 && !add_r_0))) && wavein_sign));
        
        uint16_t result =
            result_b14 << 14 | result_b13 << 13 | result_b12 << 12 | result_b11 << 11 | result_b10 << 10 |
            result_b9 << 9 | result_b8 << 8 | result_b7 << 7 | result_b6 << 6 | result_b5 << 5 |
            result_b4 << 4 | result_b3 << 3 | result_b2 << 2 | result_b1 << 1 | result_b0 << 0;
        samples_exp_table[i] = result;
    }

    // Wave rom values
    for (size_t i = 0; i < 0x20000; i++)
    {
        size_t descrambled_i = (
            ((i >> 0) & 1) << 0 |
            ((~i >> 1) & 1) << 1 |
            ((i >> 2) & 1) << 2 |
            ((~i >> 3) & 1) << 3 |
            ((i >> 4) & 1) << 4 |
            ((~i >> 5) & 1) << 5 |
            ((i >> 6) & 1) << 6 |
            ((i >> 7) & 1) << 7 |
            ((~i >> 8) & 1) << 8 |
            ((~i >> 9) & 1) << 9 |
            ((i >> 10) & 1) << 10 |
            ((i >> 11) & 1) << 11 |
            ((i >> 12) & 1) << 12 |
            ((i >> 13) & 1) << 13 |
            ((i >> 14) & 1) << 14 |
            ((i >> 15) & 1) << 15 |
            ((i >> 16) & 1) << 16
        );

        uint16_t exp_sample = (
            ((ic5[descrambled_i] >> 0) & 1) << 13 |
            ((ic6[descrambled_i] >> 4) & 1) << 12 |
            ((ic7[descrambled_i] >> 4) & 1) << 11 |
            ((~ic6[descrambled_i] >> 0) & 1) << 10 |
            ((ic7[descrambled_i] >> 7) & 1) << 9 |
            ((ic5[descrambled_i] >> 7) & 1) << 8 |
            ((~ic5[descrambled_i] >> 5) & 1) << 7 |
            ((ic6[descrambled_i] >> 2) & 1) << 6 |
            ((ic7[descrambled_i] >> 2) & 1) << 5 |
            ((ic7[descrambled_i] >> 1) & 1) << 4 |
            ((~ic5[descrambled_i] >> 1) & 1) << 3 |
            ((ic5[descrambled_i] >> 3) & 1) << 2 |
            ((ic6[descrambled_i] >> 5) & 1) << 1 |
            ((~ic6[descrambled_i] >> 7) & 1) << 0
        );
        bool exp_sign = (~ic7[descrambled_i] >> 3) & 1;
        samples_exp[i] = exp_sample;
        samples_exp_sign[i] = exp_sign;

        uint16_t delta_sample = (
            ((~ic7[descrambled_i] >> 6) & 1) << 8 |
            ((ic5[descrambled_i] >> 4) & 1) << 7 |
            ((ic7[descrambled_i] >> 0) & 1) << 6 |
            ((~ic6[descrambled_i] >> 3) & 1) << 5 |
            ((ic5[descrambled_i] >> 2) & 1) << 4 |
            ((~ic5[descrambled_i] >> 6) & 1) << 3 |
            ((ic6[descrambled_i] >> 6) & 1) << 2 |
            ((ic7[descrambled_i] >> 5) & 1) << 1 |
            ((~ic6[descrambled_i] >> 7) & 1) << 0
        );
        bool delta_sign = (ic6[descrambled_i] >> 1) & 1;
        samples_delta[i] = delta_sample;
        samples_delta_sign[i] = delta_sign;
    }
}

u8 roland_sa_device::read(offs_t offset)
{
    if (!machine().side_effects_disabled())
        return m_irq_id;

    return m_ctrl_mem[offset];
}

void roland_sa_device::write(offs_t offset, u8 data)
{
    m_int_callback(CLEAR_LINE);
    m_irq_triggered = false;

    m_ctrl_mem[offset] = data;
}

void roland_sa_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
    outputs[0].fill(0);

    int32_t *int_buffer = new int32_t[outputs[0].samples()];
    for (size_t i = 0; i < outputs[0].samples(); i++)
        int_buffer[i] = 0;

    for (size_t voiceI = 0; voiceI < NUM_VOICES; voiceI++)
    {
        for (size_t partI = 0; partI < PARTS_PER_VOICE; partI++)
        {
            SA_Part &part = m_parts[voiceI][partI];
            size_t mem_offset = voiceI * 0x100 + partI * 0x10;
            uint32_t pitch_lut_i     = m_ctrl_mem[mem_offset + 1] | (m_ctrl_mem[mem_offset + 0] << 8);
            uint32_t wave_addr_loop  = m_ctrl_mem[mem_offset + 2];
            uint32_t wave_addr_high  = m_ctrl_mem[mem_offset + 3];
            uint32_t env_dest        = m_ctrl_mem[mem_offset + 4];
            uint32_t env_speed       = m_ctrl_mem[mem_offset + 5];
            uint32_t flags           = m_ctrl_mem[mem_offset + 6];
            uint32_t env_offset      = m_ctrl_mem[mem_offset + 7];

            bool irq = false;

            for (size_t i = 0; i < outputs[0].samples(); i++)
            {
                uint32_t volume;
                uint32_t waverom_addr;
                bool ag3_sel_sample_type;
                bool ag1_phase_hi;

                // IC19
                {
                    bool env_speed_some_high =
                        BIT(env_speed, 6) || BIT(env_speed, 5) || BIT(env_speed, 4) || BIT(env_speed, 3) ||
                        BIT(env_speed, 2) || BIT(env_speed, 1) || BIT(env_speed, 0);

                    uint32_t adder1_a = part.env_value;
                    if (BIT(flags, 0))
                        adder1_a = 1 << 25;
                    uint32_t adder1_b = env_table[env_speed];
                    bool adder1_ci = env_speed_some_high && BIT(env_speed, 7);
                    if (adder1_ci)
                        adder1_b |= 0x7f << 21;

                    uint32_t adder3_o = 1 + (adder1_a >> 20) + env_offset;
                    uint32_t adder3_of = adder3_o > 0xff;
                    adder3_o &= 0xff;

                    volume = ~(
                        ((adder1_a >> 14) & 0b111111) |
                        ((adder3_o & 0b1111) << 6) |
                        (adder3_of ? ((adder3_o & 0b11110000) << 6) : 0)
                    ) & 0x3fff;

                    uint32_t adder1_o = adder1_a + adder1_b + (adder1_ci ? 1 : 0);
                    uint32_t adder1_of = adder1_o > 0xfffffff;
                    adder1_o &= 0xfffffff;

                    uint32_t adder2_o = (adder1_o >> 20) + (~env_dest & 0xff) + 1;
                    uint32_t adder2_of = adder2_o > 0xff;

                    bool end_reached = env_speed_some_high && ((adder1_of != (BIT(env_speed, 7))) || ((BIT(env_speed, 7)) != adder2_of));
                    irq |= end_reached;

                    part.env_value = end_reached ? (env_dest << 20) : adder1_o;
                }

                // IC9
                {
                    uint32_t adder1 = (phase_exp_table[pitch_lut_i] + part.sub_phase) & 0xffffff;
                    uint32_t adder2 = 1 + (adder1 >> 16) + ((~wave_addr_loop) & 0xff);
                    bool adder2_co = adder2 > 0xff;
                    adder2 &= 0xff;
                    uint32_t adder1_and = BIT(flags, 1) ? 0 : (adder1 & 0xffff);
                    adder1_and |= (BIT(flags, 1) ? 0 : (adder2_co ? adder2 : (adder1 >> 16))) << 16;

                    part.sub_phase = adder1_and;
                    waverom_addr = (wave_addr_high << 11) | ((part.sub_phase >> 9) & 0x7ff);

                    ag3_sel_sample_type = BIT(waverom_addr, 16) || BIT(waverom_addr, 15) || BIT(waverom_addr, 14) ||
                                       !((BIT(waverom_addr, 13) && !BIT(waverom_addr, 11) && !BIT(waverom_addr, 12)) || !BIT(waverom_addr, 13));
                    ag1_phase_hi = (
                        (BIT(pitch_lut_i, 15) && BIT(pitch_lut_i, 14)) ||
                        (BIT(part.sub_phase, 23) || BIT(part.sub_phase, 22) || BIT(part.sub_phase, 21) || BIT(part.sub_phase, 20)) ||
                        BIT(flags, 1)
                    );
                }

                // IC8
                {
                    uint32_t waverom_pa = samples_exp[waverom_addr];
                    uint32_t waverom_pb = samples_delta[waverom_addr];
                    bool sign_pa = samples_exp_sign[waverom_addr];
                    bool sign_pb = samples_delta_sign[waverom_addr];
                    waverom_pa |= ag3_sel_sample_type ? 1 : 0;
                    waverom_pb |= ag3_sel_sample_type ? 0 : 1;

                    if (ag1_phase_hi)
                        volume |= 0b1111 << 10;

                    uint32_t tmp_1, tmp_2;

                    uint32_t adder1_o = volume + waverom_pa;
                    bool adder1_co = adder1_o > 0x3fff;
                    adder1_o &= 0x3fff;
                    if (adder1_co)
                        adder1_o |= 0x3c00;
                    tmp_1 = adder1_o;

                    uint32_t adder3_o = addr_table[(part.sub_phase >> 5) & 0xf] + (waverom_pb & 0x1ff);
                    bool adder3_of = adder3_o > 0x1ff;
                    adder3_o &= 0x1ff;
                    if (adder3_of)
                        adder3_o |= 0x1e0;
                    
                    adder1_o = volume + (adder3_o << 5);
                    adder1_co = adder1_o > 0x3fff;
                    adder1_o &= 0x3fff;
                    if (adder1_co)
                        adder1_o |= 0x3c00;
                    tmp_2 = adder1_o;
                    
                    int32_t exp_val1 = samples_exp_table[(16384 * sign_pa) + (1024 * (tmp_1 >> 10)) + (tmp_1 & 1023)];
                    int32_t exp_val2 = samples_exp_table[(16384 * sign_pb) + (1024 * (tmp_2 >> 10)) + (tmp_2 & 1023)];
                    if (sign_pa)
                        exp_val1 = exp_val1 - 0x8000;
                    if (sign_pb)
                        exp_val2 = exp_val2 - 0x8000;
                    int32_t exp_val = exp_val1 + exp_val2;
                    
                    int_buffer[i] += exp_val;
                }
            }

            if (irq && !m_irq_triggered)
            {
                m_irq_id = partI | (voiceI << 4);
                m_int_callback(ASSERT_LINE);
                m_irq_triggered = true;
            }
        }
    }

    for (size_t i = 0; i < outputs[0].samples(); i++)
        outputs[0].put_int(i, int_buffer[i], 0xffff);

    delete[] int_buffer;
}
