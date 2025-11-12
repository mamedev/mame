// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner
/* Hyper NeoGeo 64 Audio */

// uses a V53A ( == V33A with extra peripherals eg. DMA, Timers, MMU giving virtual 24-bit address space etc.)

/* The uploaded code shows that several different sound program revisions were used

sams64    (#)SNK R&D Center (R) NEO-GEO64 Sound Driver Ver 1.00a.     (#)Copyright (C) SNK Corp. 1996-1997 All rights reserved
roadedge  (#)SNK R&D Center (R) NEO-GEO64 Sound Driver Ver 1.10.      (#)Copyright (C) SNK Corp. 1996-1997 All rights reserved
xrally    (#)SNK R&D Center (R) HYPER NEOGEO64 Sound Driver Ver 1.10. (#)Copyright (C) SNK Corp. 1997,1998 All rights reserved
bbust2    (#)SNK R&D Center (R) HYPER NEOGEO64 Sound Driver Ver 1.11. (#)Copyright (C) SNK Corp. 1997,1998 All rights reserved
sams64_2  (#)SNK R&D Center (R) HYPER NEOGEO64 Sound Driver Ver 1.14. (#)Copyright (C) SNK Corp. 1997,1998 All rights reserved
fatfurwa  (#)SNK R&D Center (R) HYPER NEOGEO64 Sound Driver Ver 1.14. (#)Copyright (C) SNK Corp. 1997,1998 All rights reserved
buriki    (#)SNK R&D Center (R) HYPER NEOGEO64 Sound Driver Ver 1.15. (#)Copyright (C) SNK Corp. 1997,1998 All rights reserved

The earlier revisions appear to have 2 banks of code (there are vectors at the end of the 0x1e0000 block and the 0x1f0000 block)

If the banking setup is wrong then those first two revisions also spam the entire range of I/O ports with values several times
on startup causing some unexpected writes to the V53 internal registers.

data structures look very similar between all of them

IRQ mask register on the internal interrupt controller is set to 0xd8

so levels 0,1,2,5 are unmasked, vectors get set during the sound CPU init code.

 level 0/1 irq (fatfurwa) starts at 0xd277 (both the same vector)
 serial comms related, maybe to get commands from main CPU if not done with shared ram?

 level 2 irq (fatfurwa) 0xdd20
 simple routine increases counter in RAM, maybe hooked to one / all of the timer irqs

 level 5 irq: (fatfurwa) starts at 0xc1e1
 largest irq, does things with ports 100 / 102 / 104 / 106, 10a  (not 108 directly tho)

 no other irqs (or the NMI) are valid.

*/


#include "emu.h"
#include "hng64.h"
#include "speaker.h"

#define LOG_SOUND (1 << 1)

#define VERBOSE (0)

#include "logmacro.h"

#define LOGSOUND(...)  LOGMASKED(LOG_SOUND, __VA_ARGS__)

// save the sound program?
#define DUMP_SOUNDPRG  0

// ----------------------------------------------
// MIPS side
// ----------------------------------------------

// if you actually map RAM here on the MIPS side then xrally will upload the actual sound program here and blank out the area where
// the program would usually be uploaded (and where all other games upload it) this seems to suggest that the area is unmapped on
// real hardware.
void hng64_state::soundram2_w(u32 data)
{
}

u32 hng64_state::soundram2_r()
{
	return 0x0000;
}


void hng64_state::soundram_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGSOUND("%s: soundram_w %08x: %08x %08x\n", machine().describe_context(), offset, data, mem_mask);

	const u32 mem_mask32 = mem_mask;
	const u32 data32 = data;

	/* swap data around.. keep the v53 happy */
	data = data32 >> 16;
	data = swapendian_int16(data);
	mem_mask = mem_mask32 >> 16;
	mem_mask = swapendian_int16(mem_mask);
	COMBINE_DATA(&m_soundram[offset * 2 + 0]);

	data = data32 & 0xffff;
	data = swapendian_int16(data);
	mem_mask = mem_mask32 & 0xffff;
	mem_mask = swapendian_int16(mem_mask);
	COMBINE_DATA(&m_soundram[offset * 2 + 1]);

	if (DUMP_SOUNDPRG)
	{
		if (offset == 0x7ffff)
		{
			LOGSOUND("dumping sound program in m_soundram\n");
			auto filename = "soundram_" + std::string(machine().system().name);
			auto fp = fopen(filename.c_str(), "w+b");
			if (fp)
			{
				fwrite((u8*)m_soundram.get(), 0x80000 * 4, 1, fp);
				fclose(fp);
			}
		}
	}
}


u32 hng64_state::soundram_r(offs_t offset)
{
	const u16 datalo = m_soundram[offset * 2 + 0];
	const u16 datahi = m_soundram[offset * 2 + 1];

	return swapendian_int16(datahi) | (swapendian_int16(datalo) << 16);
}

void hng64_state::soundcpu_enable_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_16_31)
	{
		const int cmd = data >> 16;
		// I guess it's only one of the bits, the commands are inverse of each other
		if (cmd == 0x55AA)
		{
			LOGSOUND("%s: soundcpu ON\n", machine().describe_context());
			m_audiocpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			m_audiocpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		}
		else if (cmd == 0xAA55)
		{
			LOGSOUND("%s: soundcpu OFF\n", machine().describe_context());
			m_audiocpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		}
		else
		{
			LOGSOUND("%s: unknown soundcpu_enable_w cmd %04x\n", machine().describe_context(), cmd);
		}
	}

	if (ACCESSING_BITS_0_15)
	{
		LOGSOUND("%s: unknown soundcpu_enable_w %08x %08x\n", machine().describe_context(), data, mem_mask);
	}
}

// ----------------------------------------------
// General
// ----------------------------------------------

void hng64_state::init_sound()
{
	u8 *RAM = (u8*)m_soundram.get();
	for (int i = 0; i < 16; i++)
	{
		m_audio_bank[i]->configure_entries(0, 0x20, RAM, 0x10000);
	}
}

void hng64_state::reset_sound()
{
	for (int i = 0; i < 16; i++)
	{
		m_audio_bank[i]->set_entry(0x1f);
	}

	m_audiocpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

// ----------------------------------------------
// V53A side
// ----------------------------------------------


void hng64_state::sound_map(address_map &map)
{
	for (int i = 0; i < 16; i++)
	{
		map(i << 16, (i << 16) | 0xffff).bankrw(m_audio_bank[i]);
	}
}

// but why not just use the V33/V53 XA mode??
void hng64_state::sound_bank_w(offs_t offset, u16 data)
{
	LOGSOUND("%s sound_bank_w? %02x %04x\n", machine().describe_context(), offset, data);
	// buriki writes 0x3f to 0x200 before jumping to the low addresses..
	// where it expects to find data from 0x1f0000

	// the 2 early games don't do this.. maybe all banks actuallly default to that region tho?
	// the sound code on those games seems buggier anyway.
	m_audio_bank[offset & 0xf]->set_entry(data & 0x1f);
}


void hng64_state::sound_port_0080_w(u16 data)
{
	LOGSOUND("%s: hng64_port 0x0080 %04x\n", machine().describe_context(), data);
}


void hng64_state::sound_comms_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset << 1)
	{
		// Data to main CPU
		case 0x0:
			COMBINE_DATA(&sound_latch[0]);
			return;
		// Latch status to main CPU
		case 0x2:
			COMBINE_DATA(&sound_latch[1]);
			return;
		case 0xa:
			m_audiocpu->set_input_line(5, CLEAR_LINE);
			return;
	}
}

u16 hng64_state::sound_comms_r(offs_t offset)
{
	switch (offset << 1)
	{
		case 0x00:
			return sound_latch[0];
		case 0x02:
			return sound_latch[1];
		case 0x04:
			return main_latch[0];
		case 0x06:
			return main_latch[1];
	}

	return 0;
}

void hng64_state::sound_io_map(address_map &map)
{
	map(0x0000, 0x000f).m(m_dsp, FUNC(l7a1045_sound_device::map));

	map(0x0080, 0x0081).w(FUNC(hng64_state::sound_port_0080_w));

	map(0x0100, 0x010f).rw(FUNC(hng64_state::sound_comms_r), FUNC(hng64_state::sound_comms_w));

	map(0x0200, 0x021f).w(FUNC(hng64_state::sound_bank_w)); // ??

}

void hng64_state::dma_hreq_cb(int state)
{
	m_audiocpu->hack_w(1);
}

u8 hng64_state::dma_memr_cb(offs_t offset)
{
	return m_audiocpu->space(AS_PROGRAM).read_byte(offset);
}

void hng64_state::tcu_tm0_cb(int state)
{
	// this goes high once near startup
}

void hng64_state::tcu_tm1_cb(int state)
{
}

void hng64_state::tcu_tm2_cb(int state)
{
	m_audiocpu->set_input_line(2, state ? ASSERT_LINE : CLEAR_LINE);
}

void hng64_state::dsp_map(address_map &map)
{
	map(0x0000'0000, 0x00ff'ffff).rom().region("l7a1045", 0);
}

void hng64_state::hng64_audio_base(machine_config &config)
{
	V53A(config, m_audiocpu, 32_MHz_XTAL); // reference footage indicates the timer must be the full 32 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &hng64_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &hng64_state::sound_io_map);
	m_audiocpu->out_hreq_cb().set(FUNC(hng64_state::dma_hreq_cb));
	m_audiocpu->in_memr_cb().set(FUNC(hng64_state::dma_memr_cb));
	m_audiocpu->in_io16r_cb<3>().set(m_dsp, FUNC(l7a1045_sound_device::dma_r16_cb));
	m_audiocpu->out_io16w_cb<3>().set(m_dsp, FUNC(l7a1045_sound_device::dma_w16_cb));

	m_audiocpu->tout_handler<0>().set(FUNC(hng64_state::tcu_tm0_cb));
	m_audiocpu->tout_handler<1>().set(FUNC(hng64_state::tcu_tm1_cb));
	m_audiocpu->tout_handler<2>().set(FUNC(hng64_state::tcu_tm2_cb));

	L7A1045(config, m_dsp, 33.8688_MHz_XTAL);
	m_dsp->set_addrmap(AS_DATA, &hng64_state::dsp_map);
	m_dsp->drq_handler_cb().set(m_audiocpu, FUNC(v53a_device::dreq_w<3>));
}

void hng64_state::hng64_audio(machine_config &config)
{
	hng64_audio_base(config);

	SPEAKER(config, "speaker", 2).front();
	SPEAKER(config, "rear", 1).rear_center();
	SPEAKER(config, "subwoofer", 1).lfe();

	m_dsp->add_route(l7a1045_sound_device::L6028_LEFT, "speaker", 1.0, 0);
	m_dsp->add_route(l7a1045_sound_device::L6028_RIGHT, "speaker", 1.0, 1);
	m_dsp->add_route(l7a1045_sound_device::L6028_OUT7, "rear", 1.0, 0);
	m_dsp->add_route(l7a1045_sound_device::L6028_OUT6, "subwoofer", 1.0, 0);
}

void hng64_state::hng64_audio_bbust2(machine_config &config)
{
	hng64_audio_base(config);

	SPEAKER(config, "speaker", 2).front();
	SPEAKER(config, "gun_1", 1).front_center();
	SPEAKER(config, "gun_2", 1).front_center();
	SPEAKER(config, "gun_3", 1).front_center();

	m_dsp->add_route(l7a1045_sound_device::L6028_LEFT, "speaker", 1.0, 0);
	m_dsp->add_route(l7a1045_sound_device::L6028_RIGHT, "speaker", 1.0, 1);
	m_dsp->add_route(l7a1045_sound_device::L6028_OUT2, "gun_1", 1.0, 0);
	m_dsp->add_route(l7a1045_sound_device::L6028_OUT7, "gun_2", 1.0, 0);
	m_dsp->add_route(l7a1045_sound_device::L6028_OUT6, "gun_3", 1.0, 0);
}
