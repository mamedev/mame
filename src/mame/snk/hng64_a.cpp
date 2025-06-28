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


// save the sound program?
#define DUMP_SOUNDPRG  0

// ----------------------------------------------
// MIPS side
// ----------------------------------------------

// if you actually map RAM here on the MIPS side then xrally will upload the actual sound program here and blank out the area where
// the program would usually be uploaded (and where all other games upload it) this seems to suggest that the area is unmapped on
// real hardware.
void hng64_state::hng64_soundram2_w(uint32_t data)
{
}

uint32_t hng64_state::hng64_soundram2_r()
{
	return 0x0000;
}


void hng64_state::hng64_soundram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	//logerror("hng64_soundram_w %08x: %08x %08x\n", offset, data, mem_mask);

	uint32_t mem_mask32 = mem_mask;
	uint32_t data32 = data;

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
		if (offset==0x7ffff)
		{
			logerror("dumping sound program in m_soundram\n");
			auto filename = "soundram_" + std::string(machine().system().name);
			auto fp = fopen(filename.c_str(), "w+b");
			if (fp)
			{
				fwrite((uint8_t*)m_soundram.get(), 0x80000*4, 1, fp);
				fclose(fp);
			}
		}
	}
}


uint32_t hng64_state::hng64_soundram_r(offs_t offset)
{
	uint16_t datalo = m_soundram[offset * 2 + 0];
	uint16_t datahi = m_soundram[offset * 2 + 1];

	return swapendian_int16(datahi) | (swapendian_int16(datalo) << 16);
}

void hng64_state::hng64_soundcpu_enable_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_16_31)
	{
		int cmd = data >> 16;
		// I guess it's only one of the bits, the commands are inverse of each other
		if (cmd==0x55AA)
		{
			logerror("soundcpu ON\n");
			m_audiocpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			m_audiocpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		}
		else if (cmd==0xAA55)
		{
			logerror("soundcpu OFF\n");
			m_audiocpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		}
		else
		{
			logerror("unknown hng64_soundcpu_enable_w cmd %04x\n", cmd);
		}
	}

	if (ACCESSING_BITS_0_15)
	{
		logerror("unknown hng64_soundcpu_enable_w %08x %08x\n", data, mem_mask);
	}
}

// ----------------------------------------------
// General
// ----------------------------------------------


void hng64_state::reset_sound()
{
	uint8_t *RAM = (uint8_t*)m_soundram.get();
	membank("bank0")->set_base(&RAM[0x1f0000]);
	membank("bank1")->set_base(&RAM[0x1f0000]);
	membank("bank2")->set_base(&RAM[0x1f0000]);
	membank("bank3")->set_base(&RAM[0x1f0000]);
	membank("bank4")->set_base(&RAM[0x1f0000]);
	membank("bank5")->set_base(&RAM[0x1f0000]);
	membank("bank6")->set_base(&RAM[0x1f0000]);
	membank("bank7")->set_base(&RAM[0x1f0000]);
	membank("bank8")->set_base(&RAM[0x1f0000]);
	membank("bank9")->set_base(&RAM[0x1f0000]);
	membank("banka")->set_base(&RAM[0x1f0000]);
	membank("bankb")->set_base(&RAM[0x1f0000]);
	membank("bankc")->set_base(&RAM[0x1f0000]);
	membank("bankd")->set_base(&RAM[0x1f0000]);
	membank("banke")->set_base(&RAM[0x1f0000]);
	membank("bankf")->set_base(&RAM[0x1f0000]);

	m_audiocpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

// ----------------------------------------------
// V53A side
// ----------------------------------------------


void hng64_state::hng_sound_map(address_map &map)
{
	map(0x00000, 0x0ffff).bankrw("bank0");
	map(0x10000, 0x1ffff).bankrw("bank1");
	map(0x20000, 0x2ffff).bankrw("bank2");
	map(0x30000, 0x3ffff).bankrw("bank3");
	map(0x40000, 0x4ffff).bankrw("bank4");
	map(0x50000, 0x5ffff).bankrw("bank5");
	map(0x60000, 0x6ffff).bankrw("bank6");
	map(0x70000, 0x7ffff).bankrw("bank7");
	map(0x80000, 0x8ffff).bankrw("bank8");
	map(0x90000, 0x9ffff).bankrw("bank9");
	map(0xa0000, 0xaffff).bankrw("banka");
	map(0xb0000, 0xbffff).bankrw("bankb");
	map(0xc0000, 0xcffff).bankrw("bankc");
	map(0xd0000, 0xdffff).bankrw("bankd");
	map(0xe0000, 0xeffff).bankrw("banke");
	map(0xf0000, 0xfffff).bankrw("bankf");
}

void hng64_state::hng64_sound_port_0008_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
//  logerror("hng64_sound_port_0008_w %04x %04x\n", data, mem_mask);
	// seems to one or more of the DMARQ on the V53, writes here when it expects DMA channel 3 to transfer ~0x20 bytes just after startup

	/* TODO: huh? */
	m_audiocpu->dreq_w<3>(data&0x1);
	m_dsp->l7a1045_sound_w(8/2,data,mem_mask);
//  m_audiocpu->hack_w(1);

}


uint16_t hng64_state::hng64_sound_port_0008_r(offs_t offset, uint16_t mem_mask)
{
	// read in irq5
	//logerror("%s: hng64_sound_port_0008_r mask (%04x)\n", machine().describe_context(), mem_mask);
	return 0;
}



// but why not just use the V33/V53 XA mode??
void hng64_state::hng64_sound_bank_w(offs_t offset, uint16_t data)
{
	logerror("%s hng64_sound_bank_w? %02x %04x\n", machine().describe_context(), offset, data);
	// buriki writes 0x3f to 0x200 before jumping to the low addresses..
	// where it expects to find data from 0x1f0000

	// the 2 early games don't do this.. maybe all banks actuallly default to that region tho?
	// the sound code on those games seems buggier anyway.
	uint8_t *RAM = (uint8_t*)m_soundram.get();

	int bank = data & 0x1f;

	if (offset == 0x0) membank("bank0")->set_base(&RAM[bank*0x10000]);
	if (offset == 0x1) membank("bank1")->set_base(&RAM[bank*0x10000]);
	if (offset == 0x2) membank("bank2")->set_base(&RAM[bank*0x10000]);
	if (offset == 0x3) membank("bank3")->set_base(&RAM[bank*0x10000]);
	if (offset == 0x4) membank("bank4")->set_base(&RAM[bank*0x10000]);
	if (offset == 0x5) membank("bank5")->set_base(&RAM[bank*0x10000]);
	if (offset == 0x6) membank("bank6")->set_base(&RAM[bank*0x10000]);
	if (offset == 0x7) membank("bank7")->set_base(&RAM[bank*0x10000]);
	if (offset == 0x8) membank("bank8")->set_base(&RAM[bank*0x10000]);
	if (offset == 0x9) membank("bank9")->set_base(&RAM[bank*0x10000]);
	if (offset == 0xa) membank("banka")->set_base(&RAM[bank*0x10000]);
	if (offset == 0xb) membank("bankb")->set_base(&RAM[bank*0x10000]);
	if (offset == 0xc) membank("bankc")->set_base(&RAM[bank*0x10000]);
	if (offset == 0xd) membank("bankd")->set_base(&RAM[bank*0x10000]);
	if (offset == 0xe) membank("banke")->set_base(&RAM[bank*0x10000]);
	if (offset == 0xf) membank("bankf")->set_base(&RAM[bank*0x10000]);

}


void hng64_state::hng64_sound_port_000a_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: hng64_port hng64_sound_port_000a_w %04x mask (%04x)\n", machine().describe_context(), data, mem_mask);
}

void hng64_state::hng64_sound_port_000c_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: hng64_port hng64_sound_port_000c_w %04x mask (%04x)\n", machine().describe_context(), data, mem_mask);
}


void hng64_state::hng64_sound_port_0080_w(uint16_t data)
{
	logerror("%s: hng64_port 0x0080 %04x\n", machine().describe_context(), data);
}


void hng64_state::sound_comms_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch(offset*2)
	{
		case 0x0:
			COMBINE_DATA(&sound_latch[0]);
			return;
		case 0x2:
			COMBINE_DATA(&sound_latch[1]);
			return;
		case 0xa:
			/* correct? */
			m_audiocpu->set_input_line(5, CLEAR_LINE);
			//if(data)
			//  logerror("IRQ ACK %02x?\n",data);
			return;
	}

	//logerror("SOUND W %02x %04x\n",offset*2,data);
}

uint16_t hng64_state::sound_comms_r(offs_t offset)
{
	switch(offset*2)
	{
		case 0x04:
			return main_latch[0];
		case 0x06:
			return main_latch[1];
	}
	//logerror("SOUND R %02x\n",offset*2);

	return 0;
}

void hng64_state::hng_sound_io(address_map &map)
{
	map(0x0000, 0x0007).rw(m_dsp, FUNC(l7a1045_sound_device::l7a1045_sound_r), FUNC(l7a1045_sound_device::l7a1045_sound_w));

	map(0x0008, 0x0009).rw(FUNC(hng64_state::hng64_sound_port_0008_r), FUNC(hng64_state::hng64_sound_port_0008_w));
	map(0x000a, 0x000b).w(FUNC(hng64_state::hng64_sound_port_000a_w));
	map(0x000c, 0x000d).w(FUNC(hng64_state::hng64_sound_port_000c_w));

	map(0x0080, 0x0081).w(FUNC(hng64_state::hng64_sound_port_0080_w));

	map(0x0100, 0x010f).rw(FUNC(hng64_state::sound_comms_r), FUNC(hng64_state::sound_comms_w));

	map(0x0200, 0x021f).w(FUNC(hng64_state::hng64_sound_bank_w)); // ??

}

void hng64_state::dma_hreq_cb(int state)
{
	m_audiocpu->hack_w(1);
}

uint8_t hng64_state::dma_memr_cb(offs_t offset)
{
	return m_audiocpu->space(AS_PROGRAM).read_byte(offset);
}

void hng64_state::dma_iow3_cb(uint8_t data)
{
	// currently it reads a block of 0x20 '0x00' values from a very specific block of RAM where there is a 0x20 space in the data and transfers them repeatedly, I assume
	// this is some kind of buffer for the audio or DSP and eventually will be populated with other values...
	// if this comes to life maybe something interesting is happening!

	if (data!=0x00) logerror("dma_iow3_cb %02x\n", data);
}

void hng64_state::tcu_tm0_cb(int state)
{
	// this goes high once near startup
	logerror("tcu_tm0_cb %02x\n", state);
}

void hng64_state::tcu_tm1_cb(int state)
{
	// these are very active, maybe they feed back into the v53 via one of the IRQ pins?  TM2 toggles more rapidly than TM1
//  logerror("tcu_tm1_cb %02x\n", state);
	//m_audiocpu->set_input_line(5, state? ASSERT_LINE:CLEAR_LINE); // not accurate, just so we have a trigger
	/* Almost likely wrong */
	m_audiocpu->set_input_line(2, state? ASSERT_LINE :CLEAR_LINE);

}

void hng64_state::tcu_tm2_cb(int state)
{
	// these are very active, maybe they feed back into the v53 via one of the IRQ pins?  TM2 toggles more rapidly than TM1
//  logerror("tcu_tm2_cb %02x\n", state);
	//m_audiocpu->set_input_line(1, state? ASSERT_LINE :CLEAR_LINE);
	//m_audiocpu->set_input_line(2, state? ASSERT_LINE :CLEAR_LINE);


	// NOT ACCURATE, just so that all the interrupts get triggered for now.
	#if 0
	static int i;
	if(machine().input().code_pressed_once(KEYCODE_Z))
		i++;

	if(machine().input().code_pressed_once(KEYCODE_X))
		i--;

	if(i < 0)
		i = 0;
	if(i > 7)
		i = 7;

	//logerror("trigger %02x %d\n",i,state);

	//if(machine().input().code_pressed_once(KEYCODE_C))
	{
		m_audiocpu->set_input_line(i, state? ASSERT_LINE :CLEAR_LINE);
	}
	//i++;
	//if (i == 3) i = 0;
	#endif
}



void hng64_state::hng64_audio(machine_config &config)
{
	V53A(config, m_audiocpu, 32000000/2);              // V53A, 16? mhz!
	m_audiocpu->set_addrmap(AS_PROGRAM, &hng64_state::hng_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &hng64_state::hng_sound_io);
	m_audiocpu->out_hreq_cb().set(FUNC(hng64_state::dma_hreq_cb));
	m_audiocpu->in_memr_cb().set(FUNC(hng64_state::dma_memr_cb));
	m_audiocpu->out_iow_cb<3>().set(FUNC(hng64_state::dma_iow3_cb));

	m_audiocpu->tout_handler<0>().set(FUNC(hng64_state::tcu_tm0_cb));
	m_audiocpu->tout_handler<1>().set(FUNC(hng64_state::tcu_tm1_cb));
	m_audiocpu->tout_handler<2>().set(FUNC(hng64_state::tcu_tm2_cb));

	SPEAKER(config, "speaker", 2).front();

	L7A1045(config, m_dsp, 32000000/2); // ??
	m_dsp->add_route(0, "speaker", 0.1, 0);
	m_dsp->add_route(1, "speaker", 0.1, 1);
}
