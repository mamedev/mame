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

namespace {

// LUT for the address speed
const uint32_t env_table[] = {
	0x000000, 0x000023, 0x000026, 0x000029, 0x00002d, 0x000031, 0x000036,
	0x00003b, 0x000040, 0x000046, 0x00004c, 0x000052, 0x00005a, 0x000062,
	0x00006c, 0x000076, 0x000080, 0x00008c, 0x000098, 0x0000a4, 0x0000b4,
	0x0000c4, 0x0000d8, 0x0000ec, 0x000104, 0x00011c, 0x000134, 0x00014c,
	0x00016c, 0x00018c, 0x0001b4, 0x0001dc, 0x000200, 0x000230, 0x000260,
	0x000290, 0x0002d0, 0x000310, 0x000360, 0x0003b0, 0x000400, 0x000460,
	0x0004c0, 0x000520, 0x0005a0, 0x000620, 0x0006c0, 0x000760, 0x000800,
	0x0008c0, 0x000980, 0x000a40, 0x000b40, 0x000c40, 0x000d80, 0x000ec0,
	0x001000, 0x001180, 0x001300, 0x001480, 0x001680, 0x001880, 0x001b00,
	0x001d80, 0x002000, 0x002300, 0x002600, 0x002900, 0x002d00, 0x003100,
	0x003600, 0x003b00, 0x004000, 0x004600, 0x004c00, 0x005200, 0x005a00,
	0x006200, 0x006c00, 0x007600, 0x008000, 0x008c00, 0x009800, 0x00a400,
	0x00b400, 0x00c400, 0x00d800, 0x00ec00, 0x010000, 0x011800, 0x013000,
	0x014800, 0x016800, 0x018800, 0x01b000, 0x01d800, 0x020000, 0x023000,
	0x026000, 0x029000, 0x02d000, 0x031000, 0x036000, 0x03b000, 0x040000,
	0x046000, 0x04c000, 0x052000, 0x05a000, 0x062000, 0x06c000, 0x076000,
	0x080000, 0x08c000, 0x098000, 0x0a4000, 0x0b4000, 0x0c4000, 0x0d8000,
	0x0ec000, 0x100000, 0x118000, 0x130000, 0x148000, 0x168000, 0x188000,
	0x1b0000, 0x1d8000, 0x000000, 0x1fffdc, 0x1fffd9, 0x1fffd6, 0x1fffd2,
	0x1fffce, 0x1fffc9, 0x1fffc4, 0x1fffbf, 0x1fffb9, 0x1fffb3, 0x1fffad,
	0x1fffa5, 0x1fff9d, 0x1fff93, 0x1fff89, 0x1fff7f, 0x1fff73, 0x1fff67,
	0x1fff5b, 0x1fff4b, 0x1fff3b, 0x1fff27, 0x1fff13, 0x1ffefb, 0x1ffee3,
	0x1ffecb, 0x1ffeb3, 0x1ffe93, 0x1ffe73, 0x1ffe4b, 0x1ffe23, 0x1ffdff,
	0x1ffdcf, 0x1ffd9f, 0x1ffd6f, 0x1ffd2f, 0x1ffcef, 0x1ffc9f, 0x1ffc4f,
	0x1ffbff, 0x1ffb9f, 0x1ffb3f, 0x1ffadf, 0x1ffa5f, 0x1ff9df, 0x1ff93f,
	0x1ff89f, 0x1ff7ff, 0x1ff73f, 0x1ff67f, 0x1ff5bf, 0x1ff4bf, 0x1ff3bf,
	0x1ff27f, 0x1ff13f, 0x1fefff, 0x1fee7f, 0x1fecff, 0x1feb7f, 0x1fe97f,
	0x1fe77f, 0x1fe4ff, 0x1fe27f, 0x1fdfff, 0x1fdcff, 0x1fd9ff, 0x1fd6ff,
	0x1fd2ff, 0x1fceff, 0x1fc9ff, 0x1fc4ff, 0x1fbfff, 0x1fb9ff, 0x1fb3ff,
	0x1fadff, 0x1fa5ff, 0x1f9dff, 0x1f93ff, 0x1f89ff, 0x1f7fff, 0x1f73ff,
	0x1f67ff, 0x1f5bff, 0x1f4bff, 0x1f3bff, 0x1f27ff, 0x1f13ff, 0x1effff,
	0x1ee7ff, 0x1ecfff, 0x1eb7ff, 0x1e97ff, 0x1e77ff, 0x1e4fff, 0x1e27ff,
	0x1dffff, 0x1dcfff, 0x1d9fff, 0x1d6fff, 0x1d2fff, 0x1cefff, 0x1c9fff,
	0x1c4fff, 0x1bffff, 0x1b9fff, 0x1b3fff, 0x1adfff, 0x1a5fff, 0x19dfff,
	0x193fff, 0x189fff, 0x17ffff, 0x173fff, 0x167fff, 0x15bfff, 0x14bfff,
	0x13bfff, 0x127fff, 0x113fff, 0x0fffff, 0x0e7fff, 0x0cffff, 0x0b7fff,
	0x097fff, 0x077fff, 0x04ffff, 0x027fff};

// LUT for bits 5/6/7/8 of the subphase
const uint16_t addr_table[] = {0x1e0, 0x080, 0x060, 0x04d, 0x040, 0x036, 0x02d, 0x026,
												 0x020, 0x01b, 0x016, 0x011, 0x00d, 0x00a, 0x006, 0x003};

} // anonymous namespace


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
		bool r10_9 = BIT(r10, 0);
		bool r10_8 = BIT(r10, 1);
		bool r10_0 = BIT(r10, 2);
		bool r10_1 = BIT(r10, 3);
		bool r10_2 = BIT(r10, 4);
		bool r10_3 = BIT(~r10, 5);
		bool r10_4 = BIT(~r10, 6);
		bool r10_5 = BIT(~r10, 7);
		bool r10_6 = BIT(~r10, 8);
		bool r10_7 = BIT(~r10, 9);

		bool wavein_sign = i >= 0x4000;
		uint8_t add_r_0 = BIT(i / 0x400, 0);
		uint8_t add_r_1 = BIT(i / 0x400, 1);
		uint8_t add_r_2 = BIT(i / 0x400, 2);
		uint8_t add_r_3 = BIT(i / 0x400, 3);

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
		const size_t descrambled_i = i ^ 0b0'00000011'00101010;

		const uint16_t exp_sample =
				BIT( ic5[descrambled_i], 0) << 13 |
				BIT( ic6[descrambled_i], 4) << 12 |
				BIT( ic7[descrambled_i], 4) << 11 |
				BIT(~ic6[descrambled_i], 0) << 10 |
				BIT( ic7[descrambled_i], 7) <<  9 |
				BIT( ic5[descrambled_i], 7) <<  8 |
				BIT(~ic5[descrambled_i], 5) <<  7 |
				BIT( ic6[descrambled_i], 2) <<  6 |
				BIT( ic7[descrambled_i], 2) <<  5 |
				BIT( ic7[descrambled_i], 1) <<  4 |
				BIT(~ic5[descrambled_i], 1) <<  3 |
				BIT( ic5[descrambled_i], 3) <<  2 |
				BIT( ic6[descrambled_i], 5) <<  1 |
				BIT(~ic6[descrambled_i], 7) <<  0;
		const bool exp_sign = BIT(~ic7[descrambled_i], 3);
		samples_exp[i] = exp_sample;
		samples_exp_sign[i] = exp_sign;

		const uint16_t delta_sample =
				BIT(~ic7[descrambled_i], 6) << 8 |
				BIT( ic5[descrambled_i], 4) << 7 |
				BIT( ic7[descrambled_i], 0) << 6 |
				BIT(~ic6[descrambled_i], 3) << 5 |
				BIT( ic5[descrambled_i], 2) << 4 |
				BIT(~ic5[descrambled_i], 6) << 3 |
				BIT( ic6[descrambled_i], 6) << 2 |
				BIT( ic7[descrambled_i], 5) << 1 |
				BIT(~ic6[descrambled_i], 7) << 0;
		const bool delta_sign = BIT(ic6[descrambled_i], 1);
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

void roland_sa_device::sound_stream_update(sound_stream &stream)
{
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

					stream.add_int(0, i, exp_val, 0x10000);
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
}
