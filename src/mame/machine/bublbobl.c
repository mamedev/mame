/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/bublbobl.h"


WRITE8_MEMBER(bublbobl_state::bublbobl_bankswitch_w)
{

	/* bits 0-2 select ROM bank */
	memory_set_bank(machine(), "bank1", (data ^ 4) & 7);

	/* bit 3 n.c. */

	/* bit 4 resets second Z80 */
	device_set_input_line(m_slave, INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);

	/* bit 5 resets mcu */
	if (m_mcu != NULL) // only if we have a MCU
		device_set_input_line(m_mcu, INPUT_LINE_RESET, (data & 0x20) ? CLEAR_LINE : ASSERT_LINE);

	/* bit 6 enables display */
	m_video_enable = data & 0x40;

	/* bit 7 flips screen */
	flip_screen_set(machine(), data & 0x80);
}

WRITE8_MEMBER(bublbobl_state::tokio_bankswitch_w)
{
	/* bits 0-2 select ROM bank */
	memory_set_bank(machine(), "bank1", data & 7);

	/* bits 3-7 unknown */
}

WRITE8_MEMBER(bublbobl_state::tokio_videoctrl_w)
{
	/* bit 7 flips screen */
	flip_screen_set(machine(), data & 0x80);

	/* other bits unknown */
}

WRITE8_MEMBER(bublbobl_state::bublbobl_nmitrigger_w)
{
	device_set_input_line(m_slave, INPUT_LINE_NMI, PULSE_LINE);
}


static const UINT8 tokio_prot_data[] =
{
	0x6c,
	0x7f,0x5f,0x7f,0x6f,0x5f,0x77,0x5f,0x7f,0x5f,0x7f,0x5f,0x7f,0x5b,0x7f,0x5f,0x7f,
	0x5f,0x77,0x59,0x7f,0x5e,0x7e,0x5f,0x6d,0x57,0x7f,0x5d,0x7d,0x5f,0x7e,0x5f,0x7f,
	0x5d,0x7d,0x5f,0x7e,0x5e,0x79,0x5f,0x7f,0x5f,0x7f,0x5d,0x7f,0x5f,0x7b,0x5d,0x7e,
	0x5f,0x7f,0x5d,0x7d,0x5f,0x7e,0x5e,0x7e,0x5f,0x7d,0x5f,0x7f,0x5f,0x7e,0x7f,0x5f,
	0x01,0x00,0x02,0x01,0x01,0x01,0x03,0x00,0x05,0x02,0x04,0x01,0x03,0x00,0x05,0x01,
	0x02,0x03,0x00,0x04,0x04,0x01,0x02,0x00,0x05,0x03,0x02,0x01,0x04,0x05,0x00,0x03,
	0x00,0x05,0x02,0x01,0x03,0x04,0x05,0x00,0x01,0x04,0x04,0x02,0x01,0x04,0x01,0x00,
	0x03,0x01,0x02,0x05,0x00,0x03,0x00,0x01,0x02,0x00,0x03,0x04,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x00,0x00,0x00,0x00,0x01,0x02,0x00,0x00,0x00,
	0x01,0x02,0x01,0x00,0x00,0x00,0x02,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x01,
	0x00,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x00,0x01,0x01,0x00,0x00,0x02,0x01,0x00,
	0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x01
};

READ8_MEMBER(bublbobl_state::tokio_mcu_r)
{

	m_tokio_prot_count %= sizeof(tokio_prot_data);
	return tokio_prot_data[m_tokio_prot_count++];
}

READ8_MEMBER(bublbobl_state::tokiob_mcu_r)
{
	return 0xbf; /* ad-hoc value set to pass initial testing */
}


static TIMER_CALLBACK( nmi_callback )
{
	bublbobl_state *state = machine.driver_data<bublbobl_state>();

	if (state->m_sound_nmi_enable)
		device_set_input_line(state->m_audiocpu, INPUT_LINE_NMI, PULSE_LINE);
	else
		state->m_pending_nmi = 1;
}

WRITE8_MEMBER(bublbobl_state::bublbobl_sound_command_w)
{
	soundlatch_w(space, offset, data);
	machine().scheduler().synchronize(FUNC(nmi_callback), data);
}

WRITE8_MEMBER(bublbobl_state::bublbobl_sh_nmi_disable_w)
{
	m_sound_nmi_enable = 0;
}

WRITE8_MEMBER(bublbobl_state::bublbobl_sh_nmi_enable_w)
{

	m_sound_nmi_enable = 1;
	if (m_pending_nmi)
	{
		device_set_input_line(m_audiocpu, INPUT_LINE_NMI, PULSE_LINE);
		m_pending_nmi = 0;
	}
}

WRITE8_MEMBER(bublbobl_state::bublbobl_soundcpu_reset_w)
{
	device_set_input_line(m_audiocpu, INPUT_LINE_RESET, data ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER(bublbobl_state::bublbobl_sound_status_r)
{
	return m_sound_status;
}

WRITE8_MEMBER(bublbobl_state::bublbobl_sound_status_w)
{
	m_sound_status = data;
}



/***************************************************************************

Bubble Bobble MCU

***************************************************************************/

READ8_MEMBER(bublbobl_state::bublbobl_mcu_ddr1_r)
{
	return m_ddr1;
}

WRITE8_MEMBER(bublbobl_state::bublbobl_mcu_ddr1_w)
{
	m_ddr1 = data;
}

READ8_MEMBER(bublbobl_state::bublbobl_mcu_ddr2_r)
{
	return m_ddr2;
}

WRITE8_MEMBER(bublbobl_state::bublbobl_mcu_ddr2_w)
{
	m_ddr2 = data;
}

READ8_MEMBER(bublbobl_state::bublbobl_mcu_ddr3_r)
{
	return m_ddr3;
}

WRITE8_MEMBER(bublbobl_state::bublbobl_mcu_ddr3_w)
{
	m_ddr3 = data;
}

READ8_MEMBER(bublbobl_state::bublbobl_mcu_ddr4_r)
{
	return m_ddr4;
}

WRITE8_MEMBER(bublbobl_state::bublbobl_mcu_ddr4_w)
{
	m_ddr4 = data;
}

READ8_MEMBER(bublbobl_state::bublbobl_mcu_port1_r)
{

	//logerror("%04x: 6801U4 port 1 read\n", cpu_get_pc(&space.device()));
	m_port1_in = input_port_read(machine(), "IN0");
	return (m_port1_out & m_ddr1) | (m_port1_in & ~m_ddr1);
}

WRITE8_MEMBER(bublbobl_state::bublbobl_mcu_port1_w)
{
	//logerror("%04x: 6801U4 port 1 write %02x\n", cpu_get_pc(&space.device()), data);

	// bit 4: coin lockout
	coin_lockout_global_w(machine(), ~data & 0x10);

	// bit 5: select 1-way or 2-way coin counter

	// bit 6: trigger IRQ on main CPU (jumper switchable to vblank)
	// trigger on high->low transition
	if ((m_port1_out & 0x40) && (~data & 0x40))
	{
		// logerror("triggering IRQ on main CPU\n");
		device_set_input_line_vector(m_maincpu, 0, m_mcu_sharedram[0]);
		device_set_input_line(m_maincpu, 0, HOLD_LINE);
	}

	// bit 7: select read or write shared RAM

	m_port1_out = data;
}

READ8_MEMBER(bublbobl_state::bublbobl_mcu_port2_r)
{

	//logerror("%04x: 6801U4 port 2 read\n", cpu_get_pc(&space.device()));
	return (m_port2_out & m_ddr2) | (m_port2_in & ~m_ddr2);
}

WRITE8_MEMBER(bublbobl_state::bublbobl_mcu_port2_w)
{
	//logerror("%04x: 6801U4 port 2 write %02x\n", cpu_get_pc(&space.device()), data);
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
				m_port3_in = input_port_read(machine(), portnames[address & 3]);
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

READ8_MEMBER(bublbobl_state::bublbobl_mcu_port3_r)
{
	//logerror("%04x: 6801U4 port 3 read\n", cpu_get_pc(&space.device()));
	return (m_port3_out & m_ddr3) | (m_port3_in & ~m_ddr3);
}

WRITE8_MEMBER(bublbobl_state::bublbobl_mcu_port3_w)
{
	//logerror("%04x: 6801U4 port 3 write %02x\n", cpu_get_pc(&space.device()), data);
	m_port3_out = data;
}

READ8_MEMBER(bublbobl_state::bublbobl_mcu_port4_r)
{
	//logerror("%04x: 6801U4 port 4 read\n", cpu_get_pc(&space.device()));
	return (m_port4_out & m_ddr4) | (m_port4_in & ~m_ddr4);
}

WRITE8_MEMBER(bublbobl_state::bublbobl_mcu_port4_w)
{
	//logerror("%04x: 6801U4 port 4 write %02x\n", cpu_get_pc(&space.device()), data);

	// bits 0-7 of shared RAM address

	m_port4_out = data;
}

/***************************************************************************

Bobble Bobble protection (IC43). This appears to be a PAL.

Note: the checks on the values returned by ic43_b_r are actually patched out
in boblbobl, so they don't matter. All checks are patched out in sboblbob.

***************************************************************************/

READ8_MEMBER(bublbobl_state::boblbobl_ic43_a_r)
{
	// if (offset >= 2)
	//     logerror("%04x: ic43_a_r (offs %d) res = %02x\n", cpu_get_pc(&space.device()), offset, res);

	if (offset == 0)
		return m_ic43_a << 4;
	else
		return machine().rand() & 0xff;
}

WRITE8_MEMBER(bublbobl_state::boblbobl_ic43_a_w)
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

WRITE8_MEMBER(bublbobl_state::boblbobl_ic43_b_w)
{
	static const int xorval[4] = { 4, 1, 8, 2 };

	//  logerror("%04x: ic43_b_w (offs %d) %02x\n", cpu_get_pc(&space.device()), offset, data);
	m_ic43_b = (data >> 4) ^ xorval[offset];
}

READ8_MEMBER(bublbobl_state::boblbobl_ic43_b_r)
{
	//  logerror("%04x: ic43_b_r (offs %d)\n", cpu_get_pc(&space.device()), offset);
	if (offset == 0)
		return m_ic43_b << 4;
	else
		return 0xff;	// not used?
}



/***************************************************************************

 Bootleg Bubble Bobble 68705 protection interface

 This is used by the 68705 bootleg version. Note that this actually
 wasn't working 100%, for some unknown reason the enemy movement wasn't right.

 The following is ENTIRELY GUESSWORK!!!

***************************************************************************/
static TIMER_CALLBACK( bublbobl_m68705_irq_ack )
{
	cputag_set_input_line(machine, "mcu", 0, CLEAR_LINE);
}

INTERRUPT_GEN( bublbobl_m68705_interrupt )
{
	device_set_input_line(device, 0, ASSERT_LINE);

	device->machine().scheduler().timer_set(attotime::from_msec(1000/60), FUNC(bublbobl_m68705_irq_ack)); /* TODO: understand how this is ack'ed */
}


READ8_MEMBER(bublbobl_state::bublbobl_68705_port_a_r)
{
	//logerror("%04x: 68705 port A read %02x\n", cpu_get_pc(&space.device()), m_port_a_in);
	return (m_port_a_out & m_ddr_a) | (m_port_a_in & ~m_ddr_a);
}

WRITE8_MEMBER(bublbobl_state::bublbobl_68705_port_a_w)
{
	//logerror("%04x: 68705 port A write %02x\n", cpu_get_pc(&space.device()), data);
	m_port_a_out = data;
}

WRITE8_MEMBER(bublbobl_state::bublbobl_68705_ddr_a_w)
{
	m_ddr_a = data;
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

READ8_MEMBER(bublbobl_state::bublbobl_68705_port_b_r)
{
	return (m_port_b_out & m_ddr_b) | (m_port_b_in & ~m_ddr_b);
}

WRITE8_MEMBER(bublbobl_state::bublbobl_68705_port_b_w)
{
	//logerror("%04x: 68705 port B write %02x\n", cpu_get_pc(&space.device()), data);
	static const char *const portnames[] = { "DSW0", "DSW1", "IN1", "IN2" };

	if ((m_ddr_b & 0x01) && (~data & 0x01) && (m_port_b_out & 0x01))
	{
		m_port_a_in = m_latch;
	}
	if ((m_ddr_b & 0x02) && (data & 0x02) && (~m_port_b_out & 0x02)) /* positive edge trigger */
	{
		m_address = (m_address & 0xff00) | m_port_a_out;
		//logerror("%04x: 68705 address %02x\n", cpu_get_pc(&space.device()), m_port_a_out);
	}
	if ((m_ddr_b & 0x04) && (data & 0x04) && (~m_port_b_out & 0x04)) /* positive edge trigger */
	{
		m_address = (m_address & 0x00ff) | ((m_port_a_out & 0x0f) << 8);
	}
	if ((m_ddr_b & 0x10) && (~data & 0x10) && (m_port_b_out & 0x10))
	{
		if (data & 0x08)	/* read */
		{
			if ((m_address & 0x0800) == 0x0000)
			{
				//logerror("%04x: 68705 read input port %02x\n", cpu_get_pc(&space.device()), m_address);
				m_latch = input_port_read(machine(), portnames[m_address & 3]);
			}
			else if ((m_address & 0x0c00) == 0x0c00)
			{
				//logerror("%04x: 68705 read %02x from address %04x\n", cpu_get_pc(&space.device()), m_mcu_sharedram[m_address], m_address);
				m_latch = m_mcu_sharedram[m_address & 0x03ff];
			}
			else
				logerror("%04x: 68705 unknown read address %04x\n", cpu_get_pc(&space.device()), m_address);
		}
		else	/* write */
		{
			if ((m_address & 0x0c00) == 0x0c00)
			{
				//logerror("%04x: 68705 write %02x to address %04x\n", cpu_get_pc(&space.device()), m_port_a_out, m_address);
				m_mcu_sharedram[m_address & 0x03ff] = m_port_a_out;
			}
			else
				logerror("%04x: 68705 unknown write to address %04x\n", cpu_get_pc(&space.device()), m_address);
		}
	}
	if ((m_ddr_b & 0x20) && (~data & 0x20) && (m_port_b_out & 0x20))
	{
		/* hack to get random EXTEND letters (who is supposed to do this? 68705? PAL?) */
		m_mcu_sharedram[0x7c] = machine().rand() % 6;

		device_set_input_line_vector(m_maincpu, 0, m_mcu_sharedram[0]);
		device_set_input_line(m_maincpu, 0, HOLD_LINE);
	}
	if ((m_ddr_b & 0x40) && (~data & 0x40) && (m_port_b_out & 0x40))
	{
		logerror("%04x: 68705 unknown port B bit %02x\n", cpu_get_pc(&space.device()), data);
	}
	if ((m_ddr_b & 0x80) && (~data & 0x80) && (m_port_b_out & 0x80))
	{
		logerror("%04x: 68705 unknown port B bit %02x\n", cpu_get_pc(&space.device()), data);
	}

	m_port_b_out = data;
}

WRITE8_MEMBER(bublbobl_state::bublbobl_68705_ddr_b_w)
{
	m_ddr_b = data;
}

