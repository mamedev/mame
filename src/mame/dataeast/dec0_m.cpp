// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*******************************************************************************

Data East machine functions - Bryan McPhail, mish@tendril.co.uk

* Control reads, protection chip emulations & cycle skipping patches

*******************************************************************************/

#include "emu.h"
#include "dec0.h"

#include "cpu/h6280/h6280.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/i8051.h"


/******************************************************************************/

void drgninjab_state::machine_start()
{
	dec0_state::machine_start();

	save_item(NAME(m_i8751_return));
	save_item(NAME(m_i8751_command));
	save_item(NAME(m_i8751_ports));
}

u16 dec0_state::dec0_controls_r(offs_t offset)
{
	switch (offset << 1)
	{
		case 0x0: /* Player 1 & 2 joystick & buttons */
			return m_io_inputs->read();

		case 0x2: /* Credits, start buttons */
			return m_io_system->read();

		case 0x4: /* Byte 4: Dipswitch bank 2, Byte 5: Dipswitch Bank 1 */
			return m_io_dsw->read();

		case 0x8: /* MCU data, see below */
			return 0;
	}

	if (!machine().side_effects_disabled())
		logerror("%s: Unknown dec0_controls_r read at %02x\n", machine().describe_context(), offset);
	return ~0;
}

u16 drgninjab_state::dec0_controls_r(offs_t offset)
{
	switch (offset << 1)
	{
		case 0x8: /* Intel 8751 mc, Bad Dudes, Heavy Barrel & Birdie Try only */
			//logerror("%s: warning - read i8751 %02x - %04x\n", machine().describe_context(), offset, m_i8751_return);
			return m_i8751_return;
		default:
			return dec0_state::dec0_controls_r(offset);
	}
}


/******************************************************************************/

u16 dec0_state::midres_controls_r(offs_t offset)
{
	switch (offset << 1)
	{
		case 0: /* Player 1 Joystick + start, Player 2 Joystick + start */
			return m_io_inputs->read();

		case 2: /* Dipswitches */
			return m_io_dsw->read();

		case 4: /* Player 1 rotary */
			return m_io_an[0]->read();

		case 6: /* Player 2 rotary */
			return m_io_an[1]->read();

		case 8: /* Credits, start buttons */
			return m_io_system->read();

		case 0xa: // clr.w
			return 0;

		case 0xc:
			return 0;   /* ?? watchdog ?? */
	}

	if (!machine().side_effects_disabled())
		logerror("%s: unknown midres_controls_r read at %06x\n", machine().describe_context(), 0x180000 + (offset << 1));
	return ~0;
}

/******************************************************************************/

/******************************************************************************/


u8 hippodrm_state::prot_r(offs_t offset)
{
//logerror("%s - prot_r Read %06x\n", machine().describe_context(), offset + 0x1d0000);
	if (m_prot_lsb == 0x45) return 0x4e;
	if (m_prot_lsb == 0x92) return 0x15;
	return 0;
}

void hippodrm_state::prot_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 4: m_prot_msb = data; break;
		case 5: m_prot_lsb = data; break;
	}
//logerror("%s - prot_w Wrote %02x to %06x\n", machine().describe_context(), data, offset + 0x1d0000);
}

u16 hippodrm_state::sharedram_r(offs_t offset)
{
	/* A wee helper */
	if ((offset == 0) && !machine().side_effects_disabled())
		m_maincpu->yield();
	return m_sharedram[offset] & 0xff;
}

void hippodrm_state::sharedram_w(offs_t offset, u16 data)
{
	m_sharedram[offset] = data & 0xff;
}

/******************************************************************************/

/*
    Heavy Barrel I8751 connections

    P0.0 - P0.7 <-> 4 * LS374 latches 8B,8C,7B,7C

    P1.0    -> MIXFLG1
    P1.1    -> MIXFLG2
    P1.2    -> B0FLG
    P1.3    -> B1FLG1
    P1.4    -> B1FLG2
    P1.5    -> SOUNDFLG1
    P1.6    -> SOUNDFLG2

    P2.0    -> B2FLG 0
    P2.1    -> B2FLG 1
    P2.2    <- SEL2
    P2.3    -> acknowledge INT1
    P2.4    -> Enable latch 7B
    P2.5    -> Enable latch 8B
    P2.6    -> Enable latch 8C
    P2.7    -> Enable latch 7C

    P3.0    -> CRBACK0
    P3.1    -> CRBACK1
    P3.2    -> CRBACK2
    P3.3    <- /INT1
    P3.5    <- SEL3
    P3.6    <- SEL4
    P3.7    <- SEL5

    The outputs to the graphics & audio hardware are not directly emulated, but the
    values are not known to change after bootup.
*/


u8 dec0_8751_state::dec0_mcu_port0_r()
{
	u8 result = 0xff;

	// P0 connected to latches
	if (!BIT(m_i8751_ports[2], 4))
		result &= m_i8751_command >> 8;
	if (!BIT(m_i8751_ports[2], 5))
		result &= m_i8751_command & 0x00ff;

	return result;
}

void dec0_8751_state::dec0_mcu_port0_w(u8 data)
{
	m_i8751_ports[0] = data;
}

void dec0_8751_state::dec0_mcu_port1_w(u8 data)
{
	logerror("%s: dec0_mcu_port1_w: %02x\n", machine().describe_context(), data);
	m_i8751_ports[1] = data;
}

void dec0_8751_state::dec0_mcu_port2_w(u8 data)
{
	if (!BIT(data, 2) && BIT(m_i8751_ports[2], 2))
		m_maincpu->set_input_line(M68K_IRQ_5, HOLD_LINE);
	if (!BIT(data, 3))
		m_mcu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);
	if (BIT(data, 6) && !BIT(m_i8751_ports[2], 6))
		m_i8751_return = (m_i8751_return & 0xff00) | m_i8751_ports[0];
	if (BIT(data, 7) && !BIT(m_i8751_ports[2], 7))
		m_i8751_return = (m_i8751_return & 0x00ff) | (m_i8751_ports[0] << 8);

	m_i8751_ports[2] = data;
}

void dec0_8751_state::dec0_mcu_port3_w(u8 data)
{
	logerror("%s: dec0_mcu_port3_w: %02x\n", machine().describe_context(), data);
	m_i8751_ports[3] = data;
}

void drgninjab_state::dec0_i8751_w(u16 data)
{
	m_i8751_return = 0;

	switch (data & 0xffff)
	{
		case 0x714: m_i8751_return = 0x700; break;
		case 0x73b: m_i8751_return = 0x701; break;
		case 0x72c: m_i8751_return = 0x702; break;
		case 0x73f: m_i8751_return = 0x703; break;
		case 0x755: m_i8751_return = 0x704; break;
		case 0x722: m_i8751_return = 0x705; break;
		case 0x72b: m_i8751_return = 0x706; break;
		case 0x724: m_i8751_return = 0x707; break;
		case 0x728: m_i8751_return = 0x708; break;
		case 0x735: m_i8751_return = 0x709; break;
		case 0x71d: m_i8751_return = 0x70a; break;
		case 0x721: m_i8751_return = 0x70b; break;
		case 0x73e: m_i8751_return = 0x70c; break;
		case 0x761: m_i8751_return = 0x70d; break;
		case 0x753: m_i8751_return = 0x70e; break;
		case 0x75b: m_i8751_return = 0x70f; break;
	}

	if (!m_i8751_return) logerror("%s: warning - write unknown command %04x to 8751\n", machine().describe_context(), data);
	m_maincpu->set_input_line(M68K_IRQ_5, HOLD_LINE);
}

void dec0_8751_state::dec0_i8751_w(u16 data)
{
	m_i8751_command = data;

	/* Writes to this address raise an IRQ on the i8751 microcontroller */
	if (BIT(m_i8751_ports[2], 3))
		m_mcu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);

	//logerror("%s: warning - write %04x to i8751\n", machine().describe_context(), data);
}

void drgninjab_state::dec0_i8751_reset_w(u16 data)
{
	m_i8751_return = m_i8751_command = 0;
}

/******************************************************************************/

void hippodrm_state::sprite_mirror_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_spriteram->live()[offset]);
}

/******************************************************************************/

void dec0_state::h6280_decrypt(const char *cputag)
{
	u8 *RAM = memregion(cputag)->base();

	/* Read each byte, decrypt it */
	for (int i = 0x00000; i < 0x10000; i++)
		RAM[i] = (RAM[i] & 0x7e) | ((RAM[i] & 0x1) << 7) | ((RAM[i] & 0x80) >> 7);
}

void hippodrm_state::init_hippodrm()
{
	u8 *RAM = memregion("sub")->base();

	h6280_decrypt("sub");

	/* The protection cpu has additional memory mapped protection! */
	RAM[0x189] = 0x60; /* RTS prot area */
	RAM[0x1af] = 0x60; /* RTS prot area */
	RAM[0x1db] = 0x60; /* RTS prot area */
	RAM[0x21a] = 0x60; /* RTS prot area */

	save_item(NAME(m_prot_msb));
	save_item(NAME(m_prot_lsb));
}

void slyspy_state::init_slyspy()
{
	h6280_decrypt("audiocpu");

	save_item(NAME(m_prot_state));
	save_item(NAME(m_sound_prot_state));
}
