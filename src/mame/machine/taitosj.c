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

	save_item(NAME(m_fromz80));
	save_item(NAME(m_toz80));
	save_item(NAME(m_zaccept));
	save_item(NAME(m_zready));
	save_item(NAME(m_busreq));

	save_item(NAME(m_portA_in));
	save_item(NAME(m_portA_out));
	save_item(NAME(m_address));
	save_item(NAME(m_spacecr_prot_value));
	save_item(NAME(m_protection_value));
}

void taitosj_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	/* set the default ROM bank (many games only have one bank and */
	/* never write to the bank selector register) */
	taitosj_bankswitch_w(space, 0, 0);


	m_zaccept = 1;
	m_zready = 0;
	m_busreq = 0;
	if (m_mcu != NULL)
		m_mcu->set_input_line(0, CLEAR_LINE);

	m_spacecr_prot_value = 0;
}


WRITE8_MEMBER(taitosj_state::taitosj_bankswitch_w)
{
	coin_lockout_global_w(machine(), ~data & 1);

	if(data & 0x80) membank("bank1")->set_entry(1);
	else membank("bank1")->set_entry(0);
}



/***************************************************************************

                           PROTECTION HANDLING

 Some of the games running on this hardware are protected with a 68705 mcu.
 It can either be on a daughter board containing Z80+68705+one ROM, which
 replaces the Z80 on an unprotected main board; or it can be built-in on the
 main board. The two are fucntionally equivalent.

 The 68705 can read commands from the Z80, send back result codes, and has
 direct access to the Z80 memory space. It can also trigger IRQs on the Z80.

***************************************************************************/
READ8_MEMBER(taitosj_state::taitosj_fake_data_r)
{
	LOG(("%04x: protection read\n",space.device().safe_pc()));
	return 0;
}

WRITE8_MEMBER(taitosj_state::taitosj_fake_data_w)
{
	LOG(("%04x: protection write %02x\n",space.device().safe_pc(),data));
}

READ8_MEMBER(taitosj_state::taitosj_fake_status_r)
{
	LOG(("%04x: protection status read\n",space.device().safe_pc()));
	return 0xff;
}


/* timer callback : */
READ8_MEMBER(taitosj_state::taitosj_mcu_data_r)
{
	LOG(("%04x: protection read %02x\n",space.device().safe_pc(),m_toz80));
	m_zaccept = 1;
	return m_toz80;
}

/* timer callback : */
TIMER_CALLBACK_MEMBER(taitosj_state::taitosj_mcu_real_data_w)
{
	m_zready = 1;
	m_mcu->set_input_line(0, ASSERT_LINE);
	m_fromz80 = param;
}

WRITE8_MEMBER(taitosj_state::taitosj_mcu_data_w)
{
	LOG(("%04x: protection write %02x\n",space.device().safe_pc(),data));
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(taitosj_state::taitosj_mcu_real_data_w),this), data);
	/* temporarily boost the interleave to sync things up */
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(10));
}

READ8_MEMBER(taitosj_state::taitosj_mcu_status_r)
{
	/* temporarily boost the interleave to sync things up */
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(10));

	/* bit 0 = the 68705 has read data from the Z80 */
	/* bit 1 = the 68705 has written data for the Z80 */
	return ~((m_zready << 0) | (m_zaccept << 1));
}

READ8_MEMBER(taitosj_state::taitosj_68705_portA_r)
{
	LOG(("%04x: 68705 port A read %02x\n",space.device().safe_pc(),m_portA_in));
	return m_portA_in;
}

WRITE8_MEMBER(taitosj_state::taitosj_68705_portA_w)
{
	LOG(("%04x: 68705 port A write %02x\n",space.device().safe_pc(),data));
	m_portA_out = data;
}



/*
 *  Port B connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  0   W  !68INTRQ
 *  1   W  !68LRD (enables latch which holds command from the Z80)
 *  2   W  !68LWR (loads the latch which holds data for the Z80, and sets a
 *                 status bit so the Z80 knows there's data waiting)
 *  3   W  to Z80 !BUSRQ (aka !WAIT) pin
 *  4   W  !68WRITE (triggers write to main Z80 memory area and increases low
 *                   8 bits of the latched address)
 *  5   W  !68READ (triggers read from main Z80 memory area and increases low
 *                   8 bits of the latched address)
 *  6   W  !LAL (loads the latch which holds the low 8 bits of the address of
 *               the main Z80 memory location to access)
 *  7   W  !UAL (loads the latch which holds the high 8 bits of the address of
 *               the main Z80 memory location to access)
 */

READ8_MEMBER(taitosj_state::taitosj_68705_portB_r)
{
	return 0xff;
}

/* timer callback : 68705 is going to read data from the Z80 */
TIMER_CALLBACK_MEMBER(taitosj_state::taitosj_mcu_data_real_r)
{
	m_zready = 0;
}

/* timer callback : 68705 is writing data for the Z80 */
TIMER_CALLBACK_MEMBER(taitosj_state::taitosj_mcu_status_real_w)
{
	m_toz80 = param;
	m_zaccept = 0;
}

WRITE8_MEMBER(taitosj_state::taitosj_68705_portB_w)
{
	LOG(("%04x: 68705 port B write %02x\n", space.device().safe_pc(), data));

	if (~data & 0x01)
	{
		LOG(("%04x: 68705  68INTRQ **NOT SUPPORTED**!\n", space.device().safe_pc()));
	}
	if (~data & 0x02)
	{
		/* 68705 is going to read data from the Z80 */
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(taitosj_state::taitosj_mcu_data_real_r),this));
		m_mcu->set_input_line(0, CLEAR_LINE);
		m_portA_in = m_fromz80;
		LOG(("%04x: 68705 <- Z80 %02x\n", space.device().safe_pc(), m_portA_in));
	}
	if (~data & 0x08)
		m_busreq = 1;
	else
		m_busreq = 0;
	if (~data & 0x04)
	{
		LOG(("%04x: 68705 -> Z80 %02x\n", space.device().safe_pc(), m_portA_out));

		/* 68705 is writing data for the Z80 */
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(taitosj_state::taitosj_mcu_status_real_w),this), m_portA_out);
	}
	if (~data & 0x10)
	{
		address_space &cpu0space = m_maincpu->space(AS_PROGRAM);
		LOG(("%04x: 68705 write %02x to address %04x\n",space.device().safe_pc(), m_portA_out, m_address));

		cpu0space.write_byte(m_address, m_portA_out);

		/* increase low 8 bits of latched address for burst writes */
		m_address = (m_address & 0xff00) | ((m_address + 1) & 0xff);
	}
	if (~data & 0x20)
	{
		address_space &cpu0space = m_maincpu->space(AS_PROGRAM);
		m_portA_in = cpu0space.read_byte(m_address);
		LOG(("%04x: 68705 read %02x from address %04x\n", space.device().safe_pc(), m_portA_in, m_address));
	}
	if (~data & 0x40)
	{
		LOG(("%04x: 68705 address low %02x\n", space.device().safe_pc(), m_portA_out));
		m_address = (m_address & 0xff00) | m_portA_out;
	}
	if (~data & 0x80)
	{
		LOG(("%04x: 68705 address high %02x\n", space.device().safe_pc(), m_portA_out));
		m_address = (m_address & 0x00ff) | (m_portA_out << 8);
	}
}

/*
 *  Port C connections:
 *
 *  0   R  ZREADY (1 when the Z80 has written a command in the latch)
 *  1   R  ZACCEPT (1 when the Z80 has read data from the latch)
 *  2   R  from Z80 !BUSAK pin
 *  3   R  68INTAK (goes 0 when the interrupt request done with 68INTRQ
 *                  passes through)
 */

READ8_MEMBER(taitosj_state::taitosj_68705_portC_r)
{
	int res;

	res = (m_zready << 0) | (m_zaccept << 1) | ((m_busreq^1) << 2);
	LOG(("%04x: 68705 port C read %02x\n",space.device().safe_pc(),res));
	return res;
}


/* Space Cruiser protection (otherwise the game resets on the asteroids level) */

READ8_MEMBER(taitosj_state::spacecr_prot_r)
{
	int pc = space.device().safe_pc();

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
