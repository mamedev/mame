/***************************************************************************

    Taito Qix hardware

    driver by John Butler, Ed Mueller, Aaron Giles

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6805/m6805.h"
#include "cpu/m6809/m6809.h"
#include "includes/qix.h"


/***************************************************************************

    Qix has 6 PIAs on board:

    From the ROM I/O schematic:

    PIA 0 = U11: (mapped to $9400 on the data CPU)
        port A = external input (input_port_0)
        port B = external input (input_port_1) (coin)

    PIA 1 = U20: (mapped to $9800/$9900 on the data CPU)
        port A = external input (input_port_2)
        port B = external input (input_port_3)

    PIA 2 = U30: (mapped to $9c00 on the data CPU)
        port A = external input (input_port_4)
        port B = external output (coin control)


    From the data/sound processor schematic:

    PIA 3 = U20: (mapped to $9000 on the data CPU)
        port A = data CPU to sound CPU communication
        port B = stereo volume control, 2 4-bit values
        CA1 = interrupt signal from sound CPU
        CA2 = interrupt signal to sound CPU
        CB1 = VS input signal (vertical sync)
        CB2 = INV output signal (cocktail flip)
        IRQA = /DINT1 signal
        IRQB = /DINT1 signal

    PIA 4 = U8: (mapped to $4000 on the sound CPU)
        port A = sound CPU to data CPU communication
        port B = DAC value (port B)
        CA1 = interrupt signal from data CPU
        CA2 = interrupt signal to data CPU
        IRQA = /SINT1 signal
        IRQB = /SINT1 signal

    PIA 5 = U7: (never actually used, mapped to $2000 on the sound CPU)
        port A = unused
        port B = sound CPU to TMS5220 communication
        CA1 = interrupt signal from TMS5220
        CA2 = write signal to TMS5220
        CB1 = ready signal from TMS5220
        CB2 = read signal to TMS5220
        IRQA = /SINT2 signal
        IRQB = /SINT2 signal

***************************************************************************/

const pia6821_interface qix_pia_0_intf =
{
	DEVCB_INPUT_PORT("P1"),     /* port A in */
	DEVCB_INPUT_PORT("COIN"),   /* port B in */
	DEVCB_NULL,     /* line CA1 in */
	DEVCB_NULL,     /* line CB1 in */
	DEVCB_NULL,     /* line CA2 in */
	DEVCB_NULL,     /* line CB2 in */
	DEVCB_NULL,     /* port A out */
	DEVCB_NULL,     /* port B out */
	DEVCB_NULL,     /* line CA2 out */
	DEVCB_NULL,     /* port CB2 out */
	DEVCB_NULL,     /* IRQA */
	DEVCB_NULL      /* IRQB */
};

const pia6821_interface qix_pia_1_intf =
{
	DEVCB_INPUT_PORT("SPARE"),  /* port A in */
	DEVCB_INPUT_PORT("IN0"),    /* port B in */
	DEVCB_NULL,     /* line CA1 in */
	DEVCB_NULL,     /* line CB1 in */
	DEVCB_NULL,     /* line CA2 in */
	DEVCB_NULL,     /* line CB2 in */
	DEVCB_NULL,     /* port A out */
	DEVCB_NULL,     /* port B out */
	DEVCB_NULL,     /* line CA2 out */
	DEVCB_NULL,     /* port CB2 out */
	DEVCB_NULL,     /* IRQA */
	DEVCB_NULL      /* IRQB */
};

const pia6821_interface qix_pia_2_intf =
{
	DEVCB_INPUT_PORT("P2"),             /* port A in */
	DEVCB_NULL,     /* port B in */
	DEVCB_NULL,     /* line CA1 in */
	DEVCB_NULL,     /* line CB1 in */
	DEVCB_NULL,     /* line CA2 in */
	DEVCB_NULL,     /* line CB2 in */
	DEVCB_NULL,     /* port A out */
	DEVCB_DRIVER_MEMBER(qix_state,qix_coinctl_w),       /* port B out */
	DEVCB_NULL,     /* line CA2 out */
	DEVCB_NULL,     /* port CB2 out */
	DEVCB_NULL,     /* IRQA */
	DEVCB_NULL      /* IRQB */
};



/***************************************************************************

    Games with an MCU need to handle coins differently, and provide
    communication with the MCU

***************************************************************************/

const pia6821_interface qixmcu_pia_0_intf =
{
	DEVCB_INPUT_PORT("P1"),         /* port A in */
	DEVCB_DRIVER_MEMBER(qix_state,qixmcu_coin_r),   /* port B in */
	DEVCB_NULL,     /* line CA1 in */
	DEVCB_NULL,     /* line CB1 in */
	DEVCB_NULL,     /* line CA2 in */
	DEVCB_NULL,     /* line CB2 in */
	DEVCB_NULL,     /* port A out */
	DEVCB_DRIVER_MEMBER(qix_state,qixmcu_coin_w),   /* port B out */
	DEVCB_NULL,     /* line CA2 out */
	DEVCB_NULL,     /* port CB2 out */
	DEVCB_NULL,     /* IRQA */
	DEVCB_NULL      /* IRQB */
};

const pia6821_interface qixmcu_pia_2_intf =
{
	DEVCB_INPUT_PORT("P2"),             /* port A in */
	DEVCB_NULL,     /* port B in */
	DEVCB_NULL,     /* line CA1 in */
	DEVCB_NULL,     /* line CB1 in */
	DEVCB_NULL,     /* line CA2 in */
	DEVCB_NULL,     /* line CB2 in */
	DEVCB_NULL,     /* port A out */
	DEVCB_DRIVER_MEMBER(qix_state,qixmcu_coinctrl_w),   /* port B out */
	DEVCB_NULL,     /* line CA2 out */
	DEVCB_NULL,     /* port CB2 out */
	DEVCB_NULL,     /* IRQA */
	DEVCB_NULL      /* IRQB */
};



/***************************************************************************

    Slither uses 2 SN76489's for sound instead of the 6802+DAC; these
    are accessed via the PIAs.

***************************************************************************/

const pia6821_interface slither_pia_1_intf =
{
	DEVCB_DRIVER_MEMBER(qix_state,slither_trak_lr_r),   /* port A in */
	DEVCB_NULL,     /* port B in */
	DEVCB_NULL,     /* line CA1 in */
	DEVCB_NULL,     /* line CB1 in */
	DEVCB_NULL,     /* line CA2 in */
	DEVCB_NULL,     /* line CB2 in */
	DEVCB_NULL,     /* port A out */
	DEVCB_DRIVER_MEMBER(qix_state,slither_76489_0_w),       /* port B out */
	DEVCB_NULL,     /* line CA2 out */
	DEVCB_NULL,     /* port CB2 out */
	DEVCB_NULL,     /* IRQA */
	DEVCB_NULL      /* IRQB */
};

const pia6821_interface slither_pia_2_intf =
{
	DEVCB_DRIVER_MEMBER(qix_state,slither_trak_ud_r),   /* port A in */
	DEVCB_NULL,     /* port B in */
	DEVCB_NULL,     /* line CA1 in */
	DEVCB_NULL,     /* line CB1 in */
	DEVCB_NULL,     /* line CA2 in */
	DEVCB_NULL,     /* line CB2 in */
	DEVCB_NULL,     /* port A out */
	DEVCB_DRIVER_MEMBER(qix_state,slither_76489_1_w),       /* port B out */
	DEVCB_NULL,     /* line CA2 out */
	DEVCB_NULL,     /* port CB2 out */
	DEVCB_NULL,     /* IRQA */
	DEVCB_NULL      /* IRQB */
};



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
	pia6821_device *pia = machine().device<pia6821_device>("sndpia0");
	pia->cb1_w(state);
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
	machine().device("maincpu")->execute().set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
}


WRITE8_MEMBER(qix_state::qix_data_firq_ack_w)
{
	machine().device("maincpu")->execute().set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
}


READ8_MEMBER(qix_state::qix_data_firq_r)
{
	machine().device("maincpu")->execute().set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
	return 0xff;
}


READ8_MEMBER(qix_state::qix_data_firq_ack_r)
{
	machine().device("maincpu")->execute().set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	return 0xff;
}



/*************************************
 *
 *  Video CPU FIRQ generation/ack
 *
 *************************************/

WRITE8_MEMBER(qix_state::qix_video_firq_w)
{
	machine().device("videocpu")->execute().set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
}


WRITE8_MEMBER(qix_state::qix_video_firq_ack_w)
{
	machine().device("videocpu")->execute().set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
}


READ8_MEMBER(qix_state::qix_video_firq_r)
{
	machine().device("videocpu")->execute().set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
	return 0xff;
}


READ8_MEMBER(qix_state::qix_video_firq_ack_r)
{
	machine().device("videocpu")->execute().set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
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
		machine().device("mcu")->execute().set_input_line(M68705_IRQ_LINE, ASSERT_LINE);
		/* temporarily boost the interleave to sync things up */
		/* note: I'm using 50 because 30 is not enough for space dungeon at game over */
		machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(50));
	}
	else
		machine().device("mcu")->execute().set_input_line(M68705_IRQ_LINE, CLEAR_LINE);

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
	coin_lockout_w(machine(), 0, (~data >> 6) & 1);
	coin_counter_w(machine(), 0, (data >> 7) & 1);
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
	pia6821_device *device = (pia6821_device *)ptr;
	device->write(device->machine().driver_data()->generic_space(), param >> 8, param & 0xff);
}


WRITE8_MEMBER(qix_state::qix_pia_w)
{
	/* make all the CPUs synchronize, and only AFTER that write the command to the PIA */
	/* otherwise the 68705 will miss commands */
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(qix_state::pia_w_callback),this), data | (offset << 8), (void *)downcast<pia6821_device *>(machine().device("pia0")));
}



/*************************************
 *
 *  Coin I/O for games without coin CPU
 *
 *************************************/

WRITE8_MEMBER(qix_state::qix_coinctl_w)
{
	coin_lockout_w(machine(), 0, (~data >> 2) & 1);
	coin_counter_w(machine(), 0, (data >> 1) & 1);
}



/*************************************
 *
 *  Slither SN76489 I/O
 *
 *************************************/

	WRITE8_MEMBER(qix_state::slither_76489_0_w)
{
	/* write to the sound chip */
	m_sn1->write(space.machine().device<legacy_cpu_device>("maincpu")->space(), 0, data);

	/* clock the ready line going back into CB1 */
	pia6821_device *pia = downcast<pia6821_device *>(machine().device("pia1"));
	pia->cb1_w(0);
	pia->cb1_w(1);
}


WRITE8_MEMBER(qix_state::slither_76489_1_w)
{

	/* write to the sound chip */
	m_sn2->write(machine().device<legacy_cpu_device>("maincpu")->space(), 0, data);

	/* clock the ready line going back into CB1 */
	pia6821_device *pia = downcast<pia6821_device *>(machine().device("pia2"));
	pia->cb1_w(0);
	pia->cb1_w(1);
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
