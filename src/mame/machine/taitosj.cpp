// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "cpu/m6805/m6805.h"
#include "includes/taitosj.h"


#define VERBOSE 1
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


void taitosj_state::machine_start()
{
	membank("bank1")->configure_entry(0, memregion("maincpu")->base() + 0x6000);
	membank("bank1")->configure_entry(1, memregion("maincpu")->base() + 0x10000);

	save_item(NAME(m_spacecr_prot_value));
	save_item(NAME(m_protection_value));
}

void taitosj_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	/* set the default ROM bank (many games only have one bank and */
	/* never write to the bank selector register) */
	taitosj_bankswitch_w(space, 0, 0);

	m_spacecr_prot_value = 0;
}


WRITE8_MEMBER(taitosj_state::taitosj_bankswitch_w)
{
	machine().bookkeeping().coin_lockout_global_w(~data & 1);

	/* this is a bit of a hack, but works.
	    Eventually the mixing of the ay1 outs and
	    amplitude-overdrive-mute stuff done by
	    bit 1 here should be done on a netlist.
	*/
	m_ay1->set_output_gain(0, (data & 0x2) ? 1.0 : 0.0); // 3 outputs for Ay1 since it doesn't use tied together outs
	m_ay1->set_output_gain(1, (data & 0x2) ? 1.0 : 0.0);
	m_ay1->set_output_gain(2, (data & 0x2) ? 1.0 : 0.0);
	m_ay2->set_output_gain(0, (data & 0x2) ? 1.0 : 0.0);
	m_ay3->set_output_gain(0, (data & 0x2) ? 1.0 : 0.0);
	m_ay4->set_output_gain(0, (data & 0x2) ? 1.0 : 0.0);
	m_dac->set_output_gain(0, (data & 0x2) ? 1.0 : 0.0);

	if(data & 0x80) membank("bank1")->set_entry(1);
	else membank("bank1")->set_entry(0);
}



/***************************************************************************

                           PROTECTION HANDLING

 Some of the games running on this hardware are protected with a 68705 mcu.
 It can either be on a daughter board containing Z80+68705+one ROM, which
 replaces the Z80 on an unprotected main board; or it can be built-in on the
 main board. The two are functionally equivalent.

 The 68705 can read commands from the Z80, send back result codes, and has
 direct access to the Z80 memory space. It can also trigger IRQs on the Z80.

***************************************************************************/
READ8_MEMBER(taitosj_state::taitosj_fake_data_r)
{
	LOG(("%04x: protection read\n",m_maincpu->pc()));
	return 0;
}

WRITE8_MEMBER(taitosj_state::taitosj_fake_data_w)
{
	LOG(("%04x: protection write %02x\n",m_maincpu->pc(),data));
}

READ8_MEMBER(taitosj_state::taitosj_fake_status_r)
{
	LOG(("%04x: protection status read\n",m_maincpu->pc()));
	return 0xff;
}


READ8_MEMBER(taitosj_state::mcu_mem_r)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

WRITE8_MEMBER(taitosj_state::mcu_mem_w)
{
	m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}

WRITE_LINE_MEMBER(taitosj_state::mcu_intrq_w)
{
	// FIXME: there's a logic network here that makes this edge sensitive or something and mixes it with other interrupt sources
	if (CLEAR_LINE != state)
		LOG(("68705  68INTRQ **NOT SUPPORTED**!\n"));
}

WRITE_LINE_MEMBER(taitosj_state::mcu_busrq_w)
{
	// this actually goes to the Z80 BUSRQ (aka WAIT) pin, and the MCU waits for the bus to become available
	// we're pretending this happens immediately to make life easier
	m_mcu->busak_w(state);
}


/* Space Cruiser protection (otherwise the game resets on the asteroids level) */

READ8_MEMBER(taitosj_state::spacecr_prot_r)
{
	int pc = m_maincpu->pc();

	if( pc != 0x368A && pc != 0x36A6 )
		logerror("Read protection from an unknown location: %04X\n",pc);

	m_spacecr_prot_value ^= 0xff;

	return m_spacecr_prot_value;
}


/* Alpine Ski protection crack routines */

WRITE8_MEMBER(taitosj_state::alpine_protection_w)
{
	switch (data)
	{
	case 0x05:
		m_protection_value = 0x18;
		break;
	case 0x07:
	case 0x0c:
	case 0x0f:
		m_protection_value = 0x00;      /* not used as far as I can tell */
		break;
	case 0x16:
		m_protection_value = 0x08;
		break;
	case 0x1d:
		m_protection_value = 0x18;
		break;
	default:
		m_protection_value = data;      /* not used as far as I can tell */
		break;
	}
}

WRITE8_MEMBER(taitosj_state::alpinea_bankswitch_w)
{
	taitosj_bankswitch_w(space, offset, data);
	m_protection_value = data >> 2;
}

READ8_MEMBER(taitosj_state::alpine_port_2_r)
{
	return ioport("IN2")->read() | m_protection_value;
}
