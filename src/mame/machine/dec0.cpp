// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*******************************************************************************

Data East machine functions - Bryan McPhail, mish@tendril.co.uk

* Control reads, protection chip emulations & cycle skipping patches

*******************************************************************************/

#include "emu.h"
#include "includes/dec0.h"

#include "cpu/h6280/h6280.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"


/******************************************************************************/

void dec0_state::machine_start()
{
	save_item(NAME(m_i8751_return));
	save_item(NAME(m_i8751_command));
	save_item(NAME(m_i8751_ports));
}

READ16_MEMBER(dec0_state::dec0_controls_r)
{
	switch (offset<<1)
	{
		case 0: /* Player 1 & 2 joystick & buttons */
			return ioport("INPUTS")->read();

		case 2: /* Credits, start buttons */
			return ioport("SYSTEM")->read();

		case 4: /* Byte 4: Dipswitch bank 2, Byte 5: Dipswitch Bank 1 */
			return ioport("DSW")->read();

		case 8: /* Intel 8751 mc, Bad Dudes, Heavy Barrel & Birdie Try only */
			//logerror("CPU #0 PC %06x: warning - read i8751 %06x - %04x\n", m_maincpu->pc(), 0x30c000+offset, m_i8751_return);
			return m_i8751_return;
	}

	logerror("CPU #0 PC %06x: warning - read unmapped memory address %06x\n", m_maincpu->pc(), 0x30c000+offset);
	return ~0;
}


/******************************************************************************/

READ16_MEMBER(dec0_state::midres_controls_r)
{
	switch (offset<<1)
	{
		case 0: /* Player 1 Joystick + start, Player 2 Joystick + start */
			return ioport("INPUTS")->read();

		case 2: /* Dipswitches */
			return ioport("DSW")->read();

		case 4: /* Player 1 rotary */
			return ioport("AN0")->read();

		case 6: /* Player 2 rotary */
			return ioport("AN1")->read();

		case 8: /* Credits, start buttons */
			return ioport("SYSTEM")->read();

		case 0xa: // clr.w
			return 0;

		case 0xc:
			return 0;   /* ?? watchdog ?? */
	}

	logerror("PC %06x unknown control read at %02x\n", m_maincpu->pc(), 0x180000+(offset<<1));
	return ~0;
}

/******************************************************************************/

/******************************************************************************/


READ8_MEMBER(dec0_state::hippodrm_prot_r)
{
//logerror("6280 PC %06x - Read %06x\n",cpu_getpc(),offset+0x1d0000);
	if (m_hippodrm_lsb == 0x45) return 0x4e;
	if (m_hippodrm_lsb == 0x92) return 0x15;
	return 0;
}

WRITE8_MEMBER(dec0_state::hippodrm_prot_w)
{
	switch (offset)
	{
		case 4: m_hippodrm_msb = data; break;
		case 5: m_hippodrm_lsb = data; break;
	}
//logerror("6280 PC %06x - Wrote %06x to %04x\n",cpu_getpc(),data,offset+0x1d0000);
}

READ16_MEMBER(dec0_state::hippodrm_68000_share_r)
{
	if (offset == 0) m_maincpu->yield(); /* A wee helper */
	return m_hippodrm_shared_ram[offset] & 0xff;
}

WRITE16_MEMBER(dec0_state::hippodrm_68000_share_w)
{
	m_hippodrm_shared_ram[offset] = data & 0xff;
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


READ8_MEMBER(dec0_state::dec0_mcu_port0_r)
{
	uint8_t result = 0xff;

	// P0 connected to latches
	if (!BIT(m_i8751_ports[2], 4))
		result &= m_i8751_command >> 8;
	if (!BIT(m_i8751_ports[2], 5))
		result &= m_i8751_command & 0x00ff;

	return result;
}

WRITE8_MEMBER(dec0_state::dec0_mcu_port0_w)
{
	m_i8751_ports[0] = data;
}

WRITE8_MEMBER(dec0_state::dec0_mcu_port1_w)
{
	logerror("dec0_mcu_port1_w: %02x\n", data);
	m_i8751_ports[1] = data;
}

WRITE8_MEMBER(dec0_state::dec0_mcu_port2_w)
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

WRITE8_MEMBER(dec0_state::dec0_mcu_port3_w)
{
	logerror("dec0_mcu_port3_w: %02x\n", data);
	m_i8751_ports[3] = data;
}

void dec0_state::baddudes_i8751_write(int data)
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

	if (!m_i8751_return) logerror("%s: warning - write unknown command %02x to 8571\n",machine().describe_context(),data);
	m_maincpu->set_input_line(5, HOLD_LINE);
}

void dec0_state::dec0_i8751_write(int data)
{
	m_i8751_command = data;

	/* Writes to this address raise an IRQ on the i8751 microcontroller */
	switch (m_game)
	{
	case mcu_type::EMULATED:
		if (BIT(m_i8751_ports[2], 3))
			m_mcu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
		break;
	case mcu_type::BADDUDES_SIM:
		baddudes_i8751_write(data);
		break;
	}

	//logerror("%s: warning - write %02x to i8751\n",machine().describe_context(),data);
}

void dec0_state::dec0_i8751_reset()
{
	m_i8751_return = m_i8751_command = 0;
}

/******************************************************************************/

WRITE16_MEMBER(dec0_state::sprite_mirror_w)
{
	COMBINE_DATA(&m_spriteram->live()[offset]);
}

/******************************************************************************/

READ16_MEMBER(dec0_state::robocop_68000_share_r)
{
//logerror("%08x: Share read %04x\n",m_maincpu->pc(),offset);

	return m_robocop_shared_ram[offset];
}

WRITE16_MEMBER(dec0_state::robocop_68000_share_w)
{
//  logerror("%08x: Share write %04x %04x\n",m_maincpu->pc(),offset,data);

	m_robocop_shared_ram[offset] = data & 0xff;

	if (offset == 0x7ff) /* A control address - not standard ram */
		m_subcpu->set_input_line(0, HOLD_LINE);
}

/******************************************************************************/

void dec0_state::h6280_decrypt(const char *cputag)
{
	int i;
	uint8_t *RAM = memregion(cputag)->base();

	/* Read each byte, decrypt it */
	for (i = 0x00000; i < 0x10000; i++)
		RAM[i] = (RAM[i] & 0x7e) | ((RAM[i] & 0x1) << 7) | ((RAM[i] & 0x80) >> 7);
}

void dec0_state::init_hippodrm()
{
	uint8_t *RAM = memregion("sub")->base();

	h6280_decrypt("sub");

	/* The protection cpu has additional memory mapped protection! */
	RAM[0x189] = 0x60; /* RTS prot area */
	RAM[0x1af] = 0x60; /* RTS prot area */
	RAM[0x1db] = 0x60; /* RTS prot area */
	RAM[0x21a] = 0x60; /* RTS prot area */

	save_item(NAME(m_hippodrm_msb));
	save_item(NAME(m_hippodrm_lsb));
}

void dec0_state::init_slyspy()
{
	h6280_decrypt("audiocpu");

	save_item(NAME(m_slyspy_state));
	save_item(NAME(m_slyspy_sound_state));
}

void dec0_state::init_drgninja()
{
	m_game = mcu_type::BADDUDES_SIM;
}

void dec0_state::init_hbarrel()
{
	m_game = mcu_type::EMULATED;
}
