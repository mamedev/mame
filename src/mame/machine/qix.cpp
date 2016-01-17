// license:???
// copyright-holders:John Butler, Ed Mueller, Aaron Giles
/***************************************************************************

    Taito Qix hardware

    driver by John Butler, Ed Mueller, Aaron Giles

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6805/m6805.h"
#include "cpu/m6809/m6809.h"
#include "includes/qix.h"


/*************************************
 *
 *  Machine initialization
 *
 *************************************/

void qix_state::machine_reset()
{
	/* reset the coin counter register */
	m_coinctrl = 0x00;
}


MACHINE_START_MEMBER(qix_state,qixmcu)
{
	/* set up save states */
	save_item(NAME(m_68705_port_in));
	save_item(NAME(m_coinctrl));
}



/*************************************
 *
 *  VSYNC change callback
 *
 *************************************/

WRITE_LINE_MEMBER(qix_state::qix_vsync_changed)
{
	m_sndpia0->cb1_w(state);
}



/*************************************
 *
 *  Zoo Keeper bankswitching
 *
 *************************************/

WRITE8_MEMBER(qix_state::zookeep_bankswitch_w)
{
	membank("bank1")->set_entry((data >> 2) & 1);
	/* not necessary, but technically correct */
	qix_palettebank_w(space, offset, data);
}



/*************************************
 *
 *  Data CPU FIRQ generation/ack
 *
 *************************************/

WRITE8_MEMBER(qix_state::qix_data_firq_w)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
}


WRITE8_MEMBER(qix_state::qix_data_firq_ack_w)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
}


READ8_MEMBER(qix_state::qix_data_firq_r)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
	return 0xff;
}


READ8_MEMBER(qix_state::qix_data_firq_ack_r)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	return 0xff;
}



/*************************************
 *
 *  Video CPU FIRQ generation/ack
 *
 *************************************/

WRITE8_MEMBER(qix_state::qix_video_firq_w)
{
	m_videocpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
}


WRITE8_MEMBER(qix_state::qix_video_firq_ack_w)
{
	m_videocpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
}


READ8_MEMBER(qix_state::qix_video_firq_r)
{
	m_videocpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
	return 0xff;
}


READ8_MEMBER(qix_state::qix_video_firq_ack_r)
{
	m_videocpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	return 0xff;
}



/*************************************
 *
 *  68705 Communication
 *
 *************************************/

READ8_MEMBER(qix_state::qixmcu_coin_r)
{
	logerror("6809:qixmcu_coin_r = %02X\n", m_68705_port_out[0]);
	return m_68705_port_out[0];
}


WRITE8_MEMBER(qix_state::qixmcu_coin_w)
{
	logerror("6809:qixmcu_coin_w = %02X\n", data);
	/* this is a callback called by pia6821_device::write(), so I don't need to synchronize */
	/* the CPUs - they have already been synchronized by qix_pia_w() */
	m_68705_port_in[0] = data;
}


WRITE8_MEMBER(qix_state::qixmcu_coinctrl_w)
{
	/* if (!(data & 0x04)) */
	if (data & 0x04)
	{
		m_mcu->set_input_line(M68705_IRQ_LINE, ASSERT_LINE);
		/* temporarily boost the interleave to sync things up */
		/* note: I'm using 50 because 30 is not enough for space dungeon at game over */
		machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(50));
	}
	else
		m_mcu->set_input_line(M68705_IRQ_LINE, CLEAR_LINE);

	/* this is a callback called by pia6821_device::write(), so I don't need to synchronize */
	/* the CPUs - they have already been synchronized by qix_pia_w() */
	m_coinctrl = data;
	logerror("6809:qixmcu_coinctrl_w = %02X\n", data);
}



/*************************************
 *
 *  68705 Port Inputs
 *
 *************************************/

READ8_MEMBER(qix_state::qix_68705_portA_r)
{
	UINT8 ddr = m_68705_ddr[0];
	UINT8 out = m_68705_port_out[0];
	UINT8 in = m_68705_port_in[0];
	logerror("68705:portA_r = %02X (%02X)\n", (out & ddr) | (in & ~ddr), in);
	return (out & ddr) | (in & ~ddr);
}


READ8_MEMBER(qix_state::qix_68705_portB_r)
{
	UINT8 ddr = m_68705_ddr[1];
	UINT8 out = m_68705_port_out[1];
	UINT8 in = (ioport("COIN")->read() & 0x0f) | ((ioport("COIN")->read() & 0x80) >> 3);
	return (out & ddr) | (in & ~ddr);
}


READ8_MEMBER(qix_state::qix_68705_portC_r)
{
	UINT8 ddr = m_68705_ddr[2];
	UINT8 out = m_68705_port_out[2];
	UINT8 in = (m_coinctrl & 0x08) | ((ioport("COIN")->read() & 0x70) >> 4);
	return (out & ddr) | (in & ~ddr);
}



/*************************************
 *
 *  68705 Port Outputs
 *
 *************************************/

WRITE8_MEMBER(qix_state::qix_68705_portA_w)
{
	logerror("68705:portA_w = %02X\n", data);
	m_68705_port_out[0] = data;
}


WRITE8_MEMBER(qix_state::qix_68705_portB_w)
{
	m_68705_port_out[1] = data;
	machine().bookkeeping().coin_lockout_w(0, (~data >> 6) & 1);
	machine().bookkeeping().coin_counter_w(0, (data >> 7) & 1);
}


WRITE8_MEMBER(qix_state::qix_68705_portC_w)
{
	m_68705_port_out[2] = data;
}



/*************************************
 *
 *  Data CPU PIA 0 synchronization
 *
 *************************************/

TIMER_CALLBACK_MEMBER(qix_state::pia_w_callback)
{
	m_pia0->write(generic_space(), param >> 8, param & 0xff);
}


WRITE8_MEMBER(qix_state::qix_pia_w)
{
	/* make all the CPUs synchronize, and only AFTER that write the command to the PIA */
	/* otherwise the 68705 will miss commands */
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(qix_state::pia_w_callback), this), data | (offset << 8));
}



/*************************************
 *
 *  Coin I/O for games without coin CPU
 *
 *************************************/

WRITE8_MEMBER(qix_state::qix_coinctl_w)
{
	machine().bookkeeping().coin_lockout_w(0, (~data >> 2) & 1);
	machine().bookkeeping().coin_counter_w(0, (data >> 1) & 1);
}



/*************************************
 *
 *  Slither SN76489 I/O
 *
 *************************************/

WRITE8_MEMBER(qix_state::slither_76489_0_w)
{
	/* write to the sound chip */
	m_sn1->write(generic_space(), 0, data);

	/* clock the ready line going back into CB1 */
	m_pia1->cb1_w(0);
	m_pia1->cb1_w(1);
}


WRITE8_MEMBER(qix_state::slither_76489_1_w)
{
	/* write to the sound chip */
	m_sn2->write(generic_space(), 0, data);

	/* clock the ready line going back into CB1 */
	m_pia2->cb1_w(0);
	m_pia2->cb1_w(1);
}



/*************************************
 *
 *  Slither trackball I/O
 *
 *************************************/

READ8_MEMBER(qix_state::slither_trak_lr_r)
{
	return ioport(m_flip ? "AN3" : "AN1")->read();
}


READ8_MEMBER(qix_state::slither_trak_ud_r)
{
	return ioport(m_flip ? "AN2" : "AN0")->read();
}
