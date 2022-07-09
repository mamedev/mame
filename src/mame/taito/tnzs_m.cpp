// license:BSD-3-Clause
// copyright-holders:Luca Elia, Mirko Buffoni, Takahiro Nogi,Stephane Humbert
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

  The I8742 MCU takes care of handling the coin inputs and the tilt switch.
  To simulate this, we read the status in the interrupt handler for the main
  CPU and update the counters appropriately. We also must take care of
  handling the coin/credit settings ourselves.

***************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "tnzs.h"

uint8_t tnzs_mcu_state::mcu_r(offs_t offset)
{
	uint8_t data = m_mcu->upi41_master_r(offset & 1);
	m_subcpu->yield();

//  logerror("%s: read %02x from mcu $c00%01x\n", m_maincpu->pcbase(), data, offset);

	return data;
}

void tnzs_mcu_state::mcu_w(offs_t offset, uint8_t data)
{
//  logerror("%s: write %02x to mcu $c00%01x\n", m_maincpu->pcbase(), data, offset);

	m_mcu->upi41_master_w(offset & 1, data);
}

uint8_t tnzs_mcu_state::mcu_port1_r()
{
	int data = 0;

	switch (m_input_select)
	{
		case 0x0a:  data = m_in2->read(); break;
		case 0x0c:  data = m_in0->read(); break;
		case 0x0d:  data = m_in1->read(); break;
		default:    data = 0xff; break;
	}

//  logerror("%s:  Read %02x from port 1\n", m_maincpu->pcbase(), data);

	return data;
}

void tnzs_mcu_state::mcu_port2_w(uint8_t data)
{
	machine().bookkeeping().coin_lockout_w(0, (data & 0x40) != 0 ? m_lockout_level : !m_lockout_level);
	machine().bookkeeping().coin_lockout_w(1, (data & 0x80) != 0 ? m_lockout_level : !m_lockout_level);
	machine().bookkeeping().coin_counter_w(0, (~data & 0x10));
	machine().bookkeeping().coin_counter_w(1, (~data & 0x20));

	m_input_select = data & 0xf;
}

uint8_t tnzs_mcu_state::analog_r(offs_t offset)
{
	return m_upd4701.found() ? m_upd4701->read_xy(offset) : 0;
}

void arknoid2_state::mcu_reset()
{
	m_mcu_initializing = 3;
	m_mcu_coinage_init = 0;
	m_mcu_coinage[0] = 1;
	m_mcu_coinage[1] = 1;
	m_mcu_coinage[2] = 1;
	m_mcu_coinage[3] = 1;
	m_mcu_coins_a = 0;
	m_mcu_coins_b = 0;
	m_mcu_credits = 0;
	m_mcu_reportcoin = 0;
	m_mcu_command = 0;
}

void arknoid2_state::mcu_handle_coins( int coin )
{
	/* The coin inputs and coin counters are managed by the i8742 mcu. */
	/* Here we simulate it. */
	/* Credits are limited to 9, so more coins should be rejected */
	/* Coin/Play settings must also be taken into consideration */

	if (coin & 0x08)    /* tilt */
		m_mcu_reportcoin = coin;
	else if (coin && coin != m_insertcoin)
	{
		if (coin & 0x01)    /* coin A */
		{
//          logerror("Coin dropped into slot A\n");
			machine().bookkeeping().coin_counter_w(0,1); machine().bookkeeping().coin_counter_w(0,0); /* Count slot A */
			m_mcu_coins_a++;
			if (m_mcu_coins_a >= m_mcu_coinage[0])
			{
				m_mcu_coins_a -= m_mcu_coinage[0];
				m_mcu_credits += m_mcu_coinage[1];
				if (m_mcu_credits >= 9)
				{
					m_mcu_credits = 9;
					machine().bookkeeping().coin_lockout_global_w(1); /* Lock all coin slots */
				}
				else
				{
					machine().bookkeeping().coin_lockout_global_w(0); /* Unlock all coin slots */
				}
			}
		}

		if (coin & 0x02)    /* coin B */
		{
//          logerror("Coin dropped into slot B\n");
			machine().bookkeeping().coin_counter_w(1,1); machine().bookkeeping().coin_counter_w(1,0); /* Count slot B */
			m_mcu_coins_b++;
			if (m_mcu_coins_b >= m_mcu_coinage[2])
			{
				m_mcu_coins_b -= m_mcu_coinage[2];
				m_mcu_credits += m_mcu_coinage[3];
				if (m_mcu_credits >= 9)
				{
					m_mcu_credits = 9;
					machine().bookkeeping().coin_lockout_global_w(1); /* Lock all coin slots */
				}
				else
				{
					machine().bookkeeping().coin_lockout_global_w(0); /* Unlock all coin slots */
				}
			}
		}

		if (coin & 0x04)    /* service */
		{
//          logerror("Coin dropped into service slot C\n");
			m_mcu_credits++;
		}

		m_mcu_reportcoin = coin;
	}
	else
	{
		if (m_mcu_credits < 9)
			machine().bookkeeping().coin_lockout_global_w(0); /* Unlock all coin slots */

		m_mcu_reportcoin = 0;
	}
	m_insertcoin = coin;
}

/*********************************

TNZS sync bug kludge

In all TNZS versions there is code like this:

0C5E: ld   ($EF10),a
0C61: ld   a,($EF10)
0C64: inc  a
0C65: ret  nz
0C66: jr   $0C61

which is sometimes executed by the main cpu when it writes to shared RAM a
command for the second CPU. The intended purpose of the code is to wait an
acknowledge from the sub CPU: the sub CPU writes FF to the same location
after reading the command.

However the above code is wrong. The "ret nz" instruction means that the
loop will be exited only when the contents of $EF10 are *NOT* $FF!!
On the real board, this casues little harm: the main CPU will just write
the command, read it back and, since it's not $FF, return immediately. There
is a chance that the command might go lost, but this will cause no major
harm, the worse that can happen is that the background tune will not change.

In MAME, however, since CPU interleaving is not perfect, it can happen that
the main CPU ends its timeslice after writing to EF10 but before reading it
back. In the meantime, the sub CPU will run, read the command and write FF
there - therefore causing the main CPU to enter an endless loop.

Unlike the usual sync problems in MAME, which can be fixed by increasing the
interleave factor, in this case increasing it will actually INCREASE the
chance of entering the endless loop - because it will increase the chances of
the main CPU ending its timeslice at the wrong moment.

So what we do here is catch writes by the main CPU to the RAM location, and
process them using a timer, in order to
a) force a resync of the two CPUs
b) make sure the main CPU will be the first one to run after the location is
   changed

Since the answer from the sub CPU is ignored, we don't even need to boost
interleave.

*********************************/

/*
TIMER_CALLBACK_MEMBER(tnzs_base_state::kludge_callback)
{
    tnzs_sharedram[0x0f10] = param;
}

void tnzs_base_state::tnzs_sync_kludge_w(uint8_t data)
{
    machine().scheduler().synchronize(timer_expired_delegate(FUNC(tnzs_base_state::kludge_callback),this), data);
}
*/

uint8_t arknoid2_state::mcu_r(offs_t offset)
{
	static const char mcu_startup[] = "\x55\xaa\x5a";

	//logerror("%s: read mcu %04x\n", m_maincpu->pc(), 0xc000 + offset);

	if (offset == 0)
	{
		/* if the mcu has just been reset, return startup code */
		if (m_mcu_initializing)
		{
			m_mcu_initializing--;
			return mcu_startup[2 - m_mcu_initializing];
		}

		switch (m_mcu_command)
		{
			case 0x41:
				return m_mcu_credits;

			case 0xc1:
				/* Read the credit counter or the inputs */
				if (m_mcu_readcredits == 0)
				{
					m_mcu_readcredits = 1;
					if (m_mcu_reportcoin & 0x08)
					{
						m_mcu_initializing = 3;
						return 0xee;    /* tilt */
					}
					else return m_mcu_credits;
				}
				else return m_in0->read();  /* buttons */

			default:
				logerror("error, unknown mcu command\n");
				/* should not happen */
				return 0xff;
		}
	}
	else
	{
		/*
		status bits:
		0 = mcu is ready to send data (read from c000)
		1 = mcu has read data (from c000)
		2 = unused
		3 = unused
		4-7 = coin code
		      0 = nothing
		      1,2,3 = coin switch pressed
		      e = tilt
		*/
		if (m_mcu_reportcoin & 0x08) return 0xe1;   /* tilt */
		if (m_mcu_reportcoin & 0x01) return 0x11;   /* coin 1 (will trigger "coin inserted" sound) */
		if (m_mcu_reportcoin & 0x02) return 0x21;   /* coin 2 (will trigger "coin inserted" sound) */
		if (m_mcu_reportcoin & 0x04) return 0x31;   /* coin 3 (will trigger "coin inserted" sound) */
		return 0x01;
	}
}

void arknoid2_state::mcu_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
		//logerror("%s: write %02x to mcu %04x\n", m_maincpu->pc(), data, 0xc000 + offset);
		if (m_mcu_command == 0x41)
		{
			m_mcu_credits = (m_mcu_credits + data) & 0xff;
		}
	}
	else
	{
		/*
		0xc1: read number of credits, then buttons
		0x54+0x41: add value to number of credits
		0x15: sub 1 credit (when "Continue Play" only)
		0x84: coin 1 lockout (issued only in test mode)
		0x88: coin 2 lockout (issued only in test mode)
		0x80: release coin lockout (issued only in test mode)
		during initialization, a sequence of 4 bytes sets coin/credit settings
		*/
		//logerror("%s: write %02x to mcu %04x\n", m_maincpu->pc(), data, 0xc000 + offset);

		if (m_mcu_initializing)
		{
			/* set up coin/credit settings */
			m_mcu_coinage[m_mcu_coinage_init++] = data;
			if (m_mcu_coinage_init == 4)
				m_mcu_coinage_init = 0; /* must not happen */
		}

		if (data == 0xc1)
			m_mcu_readcredits = 0;  /* reset input port number */

		if (data == 0x15)
		{
			m_mcu_credits = (m_mcu_credits - 1) & 0xff;
			if (m_mcu_credits == 0xff)
				m_mcu_credits = 0;
		}
		m_mcu_command = data;
	}
}

INTERRUPT_GEN_MEMBER(arknoid2_state::mcu_interrupt)
{
	int coin = ((m_coin1->read() & 1) << 0);
	coin |= ((m_coin2->read() & 1) << 1);
	coin |= ((m_in2->read() & 3) << 2);
	coin ^= 0x0c;
	mcu_handle_coins(coin);

	device.execute().set_input_line(0, HOLD_LINE);
}

void arknoid2_state::machine_reset()
{
	/* initialize the mcu simulation */
	mcu_reset();

	m_mcu_readcredits = 0;
	m_insertcoin = 0;
}

void kageki_state::machine_reset()
{
	tnzs_base_state::machine_reset();
	m_csport_sel = 0;
}

void tnzs_base_state::machine_start()
{
	uint8_t *sub = memregion("sub")->base();

	m_bank2 = 0;
	m_mainbank->set_bank(2);

	m_subbank->configure_entries(0, 4, &sub[0x08000], 0x2000);
	m_subbank->set_entry(m_bank2);

	save_item(NAME(m_bank2));
}

void arknoid2_state::machine_start()
{
	tnzs_base_state::machine_start();

	save_item(NAME(m_mcu_readcredits));
	save_item(NAME(m_insertcoin));
	save_item(NAME(m_mcu_initializing));
	save_item(NAME(m_mcu_coinage_init));
	save_item(NAME(m_mcu_coinage));
	save_item(NAME(m_mcu_coins_a));
	save_item(NAME(m_mcu_coins_b));
	save_item(NAME(m_mcu_credits));
	save_item(NAME(m_mcu_reportcoin));
	save_item(NAME(m_mcu_command));

	// kludge to make device work with active-high coin inputs
	m_upd4701->left_w(0);
	m_upd4701->middle_w(0);
}

void kageki_state::machine_start()
{
	tnzs_base_state::machine_start();
	save_item(NAME(m_csport_sel));
}

void kabukiz_state::machine_start()
{
	tnzs_base_state::machine_start();
	uint8_t *sound = memregion("audiocpu")->base();
	m_audiobank->configure_entries(0, 8, &sound[0x00000], 0x4000);
}

void tnzs_base_state::ramrom_bankswitch_w(uint8_t data)
{
//  logerror("%s: writing %02x to bankswitch\n", m_maincpu->pc(),data);

	// bit 4 resets the second CPU
	m_subcpu->set_input_line(INPUT_LINE_RESET, BIT(data, 4) ? CLEAR_LINE : ASSERT_LINE);

	// bits 0-2 select RAM/ROM bank
	m_mainbank->set_bank(data & 0x07);
}

void arknoid2_state::bankswitch1_w(uint8_t data)
{
	tnzs_base_state::bankswitch1_w(data);

	if (BIT(data, 2))
		mcu_reset();

	// never actually written by arknoid2 (though code exists to do it)
	m_upd4701->resetx_w(BIT(data, 5));
	m_upd4701->resety_w(BIT(data, 5));
}

void insectx_state::bankswitch1_w(uint8_t data)
{
	tnzs_base_state::bankswitch1_w(data);

	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 2));
	machine().bookkeeping().coin_lockout_w(1, BIT(~data, 3));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 4));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 5));
}

void tnzsb_state::bankswitch1_w(uint8_t data) // kabukiz_state
{
	tnzs_base_state::bankswitch1_w(data);

	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 4));
	machine().bookkeeping().coin_lockout_w(1, BIT(~data, 5));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 3));
}

void kageki_state::bankswitch1_w(uint8_t data)
{
	tnzs_base_state::bankswitch1_w(data);

	machine().bookkeeping().coin_lockout_global_w(BIT(~data, 5));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 3));
}

void tnzs_mcu_state::bankswitch1_w(uint8_t data)
{
	tnzs_base_state::bankswitch1_w(data);

	if (BIT(data, 2) && m_mcu)
		m_mcu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);

	// written only at startup by plumppop?
	if (m_upd4701.found())
	{
		m_upd4701->resetx_w(BIT(data, 5));
		m_upd4701->resety_w(BIT(data, 5));
	}
}

void tnzs_base_state::bankswitch1_w(uint8_t data)
{
//  logerror("%s: writing %02x to bankswitch 1\n", m_maincpu->pc(),data);

	// bits 0-1 select ROM bank
	m_bank2 = data & 0x03;
	m_subbank->set_entry(m_bank2);
}

void jpopnics_state::machine_reset()
{
	tnzs_base_state::machine_reset();
}

void jpopnics_state::subbankswitch_w(uint8_t data)
{
	// bits 0-1 select ROM bank
	m_subbank->set_entry(data & 0x03);

	// written once at startup
	m_upd4701->resetx_w(BIT(data, 5));
	m_upd4701->resety_w(BIT(data, 5));
}

void tnzsb_state::sound_command_w(uint8_t data)
{
	m_soundlatch->write(data);
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

// handler called by the 2203 emulator when the internal timers cause an IRQ
WRITE_LINE_MEMBER(tnzsb_state::ym2203_irqhandler)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}

void kabukiz_state::sound_bank_w(uint8_t data)
{
	// to avoid the write when the sound chip is initialized
	if (data != 0xff)
		m_audiobank->set_entry(data & 0x07);
}
