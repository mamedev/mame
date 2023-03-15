// license:BSD-3-Clause
// copyright-holders:Chris Moore, Nicola Salmoria
/***************************************************************************

  bublbobl.cpp

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "bublbobl.h"

#include "cpu/z80/z80.h"


void bublbobl_state::common_sreset(int state)
{
	if ((state != CLEAR_LINE) && !m_sreset_old)
	{
		if (m_ym2203 != nullptr) m_ym2203->reset(); // ym2203, if present, is reset
		if (m_ym3526 != nullptr) m_ym3526->reset(); // ym3526, if present, is reset
		m_audiocpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE); // if a sound irq is active, it is cleared. is this necessary? if the above two devices de-assert /IRQ on reset (as a device_line write) properly, it shouldn't be...
		m_sound_to_main->acknowledge_w(); // sound->main semaphore is cleared
		m_soundnmi->in_w<0>(0); // sound nmi enable is unset
	}
	m_audiocpu->set_input_line(INPUT_LINE_RESET, state); // soundcpu is reset
	m_sreset_old = (ASSERT_LINE == state);
}

/* bublbobl bankswitch reg
   76543210
   |||||\\\- Select ROM bank
   ||||\---- N.C.
   |||\----- /SBRES [SUBCPU /RESET]
   ||\------ /SEQRES [MCU /RESET]
   |\------- /BLACK [Video Enable]
   \-------- VHINV [flip screen]
// 44 74 74 76 or 76 36 76 once or more per frame...
*/

void bublbobl_state::bublbobl_bankswitch_w(uint8_t data)
{
	//logerror("bankswitch_w:  write of %02X\n", data);
	/* bits 0-2 select ROM bank */
	membank("bank1")->set_entry((data ^ 4) & 7);

	/* bit 3 n.c. */

	/* bit 4 resets subcpu Z80 */
	m_subcpu->set_input_line(INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);

	/* bit 5 resets mcu */
	if (m_mcu != nullptr) // only if we have a MCU
		m_mcu->set_input_line(INPUT_LINE_RESET, (data & 0x20) ? CLEAR_LINE : ASSERT_LINE);

	/* bit 6 enables display */
	m_video_enable = data & 0x40;

	/* bit 7 flips screen */
	flip_screen_set(data & 0x80);
}

/* tokio bankswitch reg
   76543210
   |||||\\\- Select ROM bank
   ||||\---- ? used (idle high, /SEQRES?)
   |||\----- not used?
   ||\------ not used?
   |\------- ? used (idle high, /BLACK?)
   \-------- ? used (idle high, /SRESET?)
// bublboblp: test and main: 00 C8 C9 C8 C9...; tokio: test 00 09 09 49 main 00 09 C8 CF
*/
void bublbobl_state::tokio_bankswitch_w(uint8_t data)
{
	/* bits 0-2 select ROM bank */
	membank("bank1")->set_entry(data & 7);

	/* bit 3 unknown */
	/* GUESS: bit 3 resets mcu */
	if (m_mcu != nullptr) // only if we have a MCU
		m_mcu->set_input_line(INPUT_LINE_RESET, (data & 0x08) ? CLEAR_LINE : ASSERT_LINE);

	/* bit 4 and 5 unknown, not used? */

	/* bit 6 is unknown */
	/* GUESS: bit 6 is video enable "/BLACK" */
	m_video_enable = data & 0x40; // guess

	/* bit 7 is unknown but used */
}

/* tokio videoctrl reg
   76543210
   ||||\\\\- not used?
   |||\----- OUT (coin lockout to pc030cm, active low)
   ||\------ ? used (idle low, maybe 2WAY to pc030cm?)
   |\------- ? used (idle high, /SBRES? or /SBINT?)
   \-------- VHINV (flip screen)
*/
void bublbobl_state::tokio_videoctrl_w(uint8_t data)
{
	//logerror("tokio_videoctrl_w:  write of %02X\n", data);
	/* bits 0-3 not used? */

	/* bit 4 is the coin lockout */
	machine().bookkeeping().coin_lockout_global_w(~data & 0x10);

	/* bit 5 and 6 are unknown but used */

	/* bit 7 flips screen */
	flip_screen_set(data & 0x80);
}

void bublbobl_state::bublbobl_nmitrigger_w(uint8_t data)
{
	m_subcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

uint8_t bublbobl_state::tokiob_mcu_r()
{
	/* This return value is literally set by a resistor on the bootleg tokio pcb;
	the MCU footprint is unpopulated but for a resistor tying what would be the
	PA6 pin to ground. The remaining pins seem to float high. */
	return 0xbf;
}


TIMER_CALLBACK_MEMBER(bublbobl_state::irq_ack)
{
	m_mcu->set_input_line(0, CLEAR_LINE);
}

void bublbobl_state::bublbobl_soundcpu_reset_w(uint8_t data)
{
	//logerror("soundcpu_reset_w called with data of %d\n", data);
	common_sreset(data ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t bublbobl_state::common_sound_semaphores_r()
{
	uint8_t ret = 0xfc;
	ret |= m_main_to_sound->pending_r() ? 0x2 : 0x0;
	ret |= m_sound_to_main->pending_r() ? 0x1 : 0x0;
	return ret;
}

IRQ_CALLBACK_MEMBER(bublbobl_state::mcram_vect_r)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	return m_mcu_sharedram[0];
}


/***************************************************************************

Bubble Bobble MCU

***************************************************************************/

void bublbobl_state::bublbobl_mcu_port1_w(uint8_t data)
{
	//logerror("%04x: 6801U4 port 1 write %02x\n", m_mcu->pc(), data);

	// bit 4: coin lockout
	machine().bookkeeping().coin_lockout_global_w(~data & 0x10);

	// bit 5: select 1-way or 2-way coin counter

	// bit 6: trigger IRQ on main CPU (jumper switchable to vblank)
	// trigger on high->low transition
	if ((m_port1_out & 0x40) && (~data & 0x40))
	{
		// logerror("triggering IRQ on main CPU\n");
		m_maincpu->set_input_line(0, ASSERT_LINE);
	}

	// bit 7: select read or write shared RAM
	m_port1_out = data;
}

void bublbobl_state::bublbobl_mcu_port2_w(uint8_t data)
{
	//logerror("%04x: 6801U4 port 2 write %02x\n", m_mcu->pc(), data);
	static const char *const portnames[] = { "DSW0", "DSW1", "IN1", "IN2" };

	// bits 0-3: bits 8-11 of shared RAM address

	// bit 4: clock (goes to PAL A78-04.12)
	// latch on low->high transition
	if ((~m_port2_out & 0x10) && (data & 0x10))
	{
		int address = m_port4_out | ((data & 0x0f) << 8);

		if (m_port1_out & 0x80)
		{
			// read
			if ((address & 0x0800) == 0x0000)
				m_port3_in = ioport(portnames[address & 3])->read();
			else if ((address & 0x0c00) == 0x0c00)
				m_port3_in = m_mcu_sharedram[address & 0x03ff];
			// logerror("reading %02x from shared RAM %04x\n", m_port3_in, address);
		}
		else
		{
			// write
			// logerror("writing %02x to shared RAM %04x\n", m_port3_out, address);
			if ((address & 0x0c00) == 0x0c00)
				m_mcu_sharedram[address & 0x03ff] = m_port3_out;
		}
	}

	m_port2_out = data;
}

uint8_t bublbobl_state::bublbobl_mcu_port3_r()
{
	//logerror("%04x: 6801U4 port 3 read\n", m_mcu->pc());
	return m_port3_in;
}

void bublbobl_state::bublbobl_mcu_port3_w(uint8_t data)
{
	//logerror("%04x: 6801U4 port 3 write %02x\n", m_mcu->pc(), data);
	m_port3_out = data;
}

void bublbobl_state::bublbobl_mcu_port4_w(uint8_t data)
{
	//logerror("%04x: 6801U4 port 4 write %02x\n", m_mcu->pc(), data);

	// bits 0-7 of shared RAM address
	m_port4_out = data;
}

/***************************************************************************

Bobble Bobble protection (IC43). This appears to be a PAL.

Note: the checks on the values returned by ic43_b_r are actually patched out
in boblbobl, so they don't matter. All checks are patched out in sboblbob.

***************************************************************************/

uint8_t bublbobl_state::boblbobl_ic43_a_r(offs_t offset)
{
	// if (offset >= 2)
	//     logerror("%04x: ic43_a_r (offs %d) res = %02x\n", m_mcu->pc(), offset, res);

	if (offset == 0)
		return m_ic43_a << 4;
	else
		return machine().rand() & 0xff;
}

void bublbobl_state::boblbobl_ic43_a_w(offs_t offset, uint8_t data)
{
	int res = 0;

	switch (offset)
	{
		case 0:
			if (~m_ic43_a & 8) res ^= 1;
			if (~m_ic43_a & 1) res ^= 2;
			if (~m_ic43_a & 1) res ^= 4;
			if (~m_ic43_a & 2) res ^= 4;
			if (~m_ic43_a & 4) res ^= 8;
			break;
		case 1:
			if (~m_ic43_a & 8) res ^= 1;
			if (~m_ic43_a & 2) res ^= 1;
			if (~m_ic43_a & 8) res ^= 2;
			if (~m_ic43_a & 1) res ^= 4;
			if (~m_ic43_a & 4) res ^= 8;
			break;
		case 2:
			if (~m_ic43_a & 4) res ^= 1;
			if (~m_ic43_a & 8) res ^= 2;
			if (~m_ic43_a & 2) res ^= 4;
			if (~m_ic43_a & 1) res ^= 8;
			if (~m_ic43_a & 4) res ^= 8;
			break;
		case 3:
			if (~m_ic43_a & 2) res ^= 1;
			if (~m_ic43_a & 4) res ^= 2;
			if (~m_ic43_a & 8) res ^= 2;
			if (~m_ic43_a & 8) res ^= 4;
			if (~m_ic43_a & 1) res ^= 8;
			break;
	}
	m_ic43_a = res;
}

void bublbobl_state::boblbobl_ic43_b_w(offs_t offset, uint8_t data)
{
	static const int xorval[4] = { 4, 1, 8, 2 };

	//  logerror("%04x: ic43_b_w (offs %d) %02x\n", m_mcu->pc(), offset, data);
	m_ic43_b = (data >> 4) ^ xorval[offset];
}

uint8_t bublbobl_state::boblbobl_ic43_b_r(offs_t offset)
{
	//  logerror("%04x: ic43_b_r (offs %d)\n", m_mcu->pc(), offset);
	if (offset == 0)
		return m_ic43_b << 4;
	else
		return 0xff;    // not used?
}



/***************************************************************************

 Bootleg Bubble Bobble 68705 protection interface

 This is used by the 68705 bootleg version. Note that this actually
 wasn't working 100%, for some unknown reason the enemy movement wasn't right.

 The following is ENTIRELY GUESSWORK!!!

***************************************************************************/
INTERRUPT_GEN_MEMBER(bub68705_state::bublbobl_m68705_interrupt)
{
	device.execute().set_input_line(M68705_IRQ_LINE, ASSERT_LINE);
	m_irq_ack_timer->adjust(attotime::from_msec(1000/60)); /* TODO: understand how this is ack'ed */
}


void bub68705_state::port_a_w(uint8_t data)
{
	//logerror("%04x: 68705 port A write %02x\n", m_mcu->pc(), data);
	m_port_a_out = data;
}



/*
 *  Port B connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  0   W  enables latch which holds data from main Z80 memory
 *  1   W  loads the latch which holds the low 8 bits of the address of
 *               the main Z80 memory location to access
 *  2   W  loads the latch which holds the high 4 bits of the address of
 *               the main Z80 memory location to access
 *         00-07 = read input ports
 *         0c-0f = access z80 memory at 0xfc00
 *  3   W  selects Z80 memory access direction (0 = write 1 = read)
 *  4   W  clocks main Z80 memory access (goes to a PAL)
 *  5   W  clocks a flip-flop which causes IRQ on the main Z80
 *  6   W  not used?
 *  7   W  not used?
 */

void bub68705_state::port_b_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	//logerror("%04x: 68705 port B write %02x\n", m_mcu->pc(), data);

	if (BIT(mem_mask, 0) && !BIT(data, 0) && BIT(m_port_b_out, 0))
		m_mcu->pa_w(m_latch);

	if (BIT(mem_mask, 1) && BIT(data, 1) && !BIT(m_port_b_out, 1)) /* positive edge trigger */
	{
		m_address = (m_address & 0xff00) | m_port_a_out;
		//logerror("%04x: 68705 address %02x\n", m_mcu->pc(), m_port_a_out);
	}

	if (BIT(mem_mask, 2) && BIT(data, 2) && !BIT(m_port_b_out, 2)) /* positive edge trigger */
		m_address = (m_address & 0x00ff) | ((m_port_a_out & 0x0f) << 8);

	if (BIT(mem_mask, 4) && !BIT(data, 4) && BIT(m_port_b_out, 4))
	{
		if (BIT(data, 3)) /* read */
		{
			if ((m_address & 0x0800) == 0x0000)
			{
				//logerror("%04x: 68705 read input port %02x\n", m_mcu->pc(), m_address);
				m_latch = m_mux_ports[m_address & 3]->read();
			}
			else if ((m_address & 0x0c00) == 0x0c00)
			{
				//logerror("%04x: 68705 read %02x from address %04x\n", m_mcu->pc(), m_mcu_sharedram[m_address], m_address);
				m_latch = m_mcu_sharedram[m_address & 0x03ff];
			}
			else
			{
				logerror("%04x: 68705 unknown read address %04x\n", m_mcu->pc(), m_address);
			}
		}
		else /* write */
		{
			if ((m_address & 0x0c00) == 0x0c00)
			{
				//logerror("%04x: 68705 write %02x to address %04x\n", m_mcu->pc(), m_port_a_out, m_address);
				m_mcu_sharedram[m_address & 0x03ff] = m_port_a_out;
			}
			else
			{
				logerror("%04x: 68705 unknown write to address %04x\n", m_mcu->pc(), m_address);
			}
		}
	}

	if (BIT(mem_mask, 5) && !BIT(data, 5) && BIT(m_port_b_out, 5))
	{
		/* hack to get random EXTEND letters (who is supposed to do this? 68705? PAL?) */
		m_mcu_sharedram[0x7c] = machine().rand() % 6;

		m_maincpu->set_input_line(0, ASSERT_LINE);
	}

	if (BIT(mem_mask, 6) && !BIT(data, 6) && BIT(m_port_b_out, 6))
		logerror("%04x: 68705 unknown port B bit %02x\n", m_mcu->pc(), data);

	if (BIT(mem_mask, 7) && !BIT(data, 7) && BIT(m_port_b_out, 7))
		logerror("%04x: 68705 unknown port B bit %02x\n", m_mcu->pc(), data);

	m_port_b_out = data;
}
