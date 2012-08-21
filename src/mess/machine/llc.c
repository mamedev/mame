/***************************************************************************

        LLC driver by Miodrag Milanovic

        17/04/2009 Preliminary driver.

****************************************************************************/


#include "includes/llc.h"


// LLC1 BASIC keyboard
READ8_MEMBER(llc_state::llc1_port2_b_r)
{
	UINT8 retVal = 0;

	if (m_term_status)
	{
		retVal = m_term_status;
		m_term_status = 0;
	}
	else
		retVal = m_term_data;

	return retVal;
}

READ8_MEMBER(llc_state::llc1_port2_a_r)
{
	return 0;
}

// LLC1 Monitor keyboard
READ8_MEMBER(llc_state::llc1_port1_a_r)
{
	UINT8 data = 0;
	if (!BIT(m_porta, 4))
		data = ioport("X4")->read();
	if (!BIT(m_porta, 5))
		data = ioport("X5")->read();
	if (!BIT(m_porta, 6))
		data = ioport("X6")->read();
	if (data & 0xf0)
		data = (data >> 4) | 0x80;

	data |= (m_porta & 0x70);

	// do not repeat key
	if (data & 15)
	{
		if (data == m_llc1_key)
			data &= 0x70;
		else
			m_llc1_key = data;
	}
	else
	if ((data & 0x70) == (m_llc1_key & 0x70))
		m_llc1_key = 0;

	return data;
}

WRITE8_MEMBER(llc_state::llc1_port1_a_w)
{
	m_porta = data;
}

WRITE8_MEMBER(llc_state::llc1_port1_b_w)
{
	static UINT8 count = 0, digit = 0;

	if (data == 0)
	{
		digit = 0;
		count = 0;
	}
	else
		count++;

	if (count == 1)
		output_set_digit_value(digit, data & 0x7f);
	else
	if (count == 3)
	{
		count = 0;
		digit++;
	}
}

// timer 0 irq does digit display, and timer 3 irq does scan of the
// monitor keyboard.
// No idea how the CTC is connected, so guessed.
Z80CTC_INTERFACE( llc1_ctc_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),
	DEVCB_DEVICE_LINE_MEMBER("z80ctc", z80ctc_device, trg1),
	DEVCB_DEVICE_LINE_MEMBER("z80ctc", z80ctc_device, trg3),
	DEVCB_NULL
};

Z80PIO_INTERFACE( llc1_z80pio1_intf )
{
	DEVCB_NULL,	/* callback when change interrupt status */
	DEVCB_DRIVER_MEMBER(llc_state, llc1_port1_a_r),
	DEVCB_DRIVER_MEMBER(llc_state, llc1_port1_a_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(llc_state, llc1_port1_b_w),
	DEVCB_NULL
};

Z80PIO_INTERFACE( llc1_z80pio2_intf )
{
	DEVCB_NULL,	/* callback when change interrupt status */
	DEVCB_DRIVER_MEMBER(llc_state, llc1_port2_a_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(llc_state, llc1_port2_b_r),
	DEVCB_NULL,
	DEVCB_NULL
};

DRIVER_INIT_MEMBER(llc_state,llc1)
{
}

MACHINE_RESET( llc1 )
{
	llc_state *state = machine.driver_data<llc_state>();
	state->m_term_status = 0;
	state->m_llc1_key = 0;
}

MACHINE_START(llc1)
{
}

Z80CTC_INTERFACE( llc2_ctc_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


/* Driver initialization */
DRIVER_INIT_MEMBER(llc_state,llc2)
{
	m_p_videoram.set_target( machine().device<ram_device>(RAM_TAG)->pointer() + 0xc000,m_p_videoram.bytes());
}

MACHINE_RESET( llc2 )
{
	llc_state *state = machine.driver_data<llc_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	space->unmap_write(0x0000, 0x3fff);
	state->membank("bank1")->set_base(machine.root_device().memregion("maincpu")->base());

	space->unmap_write(0x4000, 0x5fff);
	state->membank("bank2")->set_base(machine.root_device().memregion("maincpu")->base() + 0x4000);

	space->unmap_write(0x6000, 0xbfff);
	state->membank("bank3")->set_base(machine.root_device().memregion("maincpu")->base() + 0x6000);

	space->install_write_bank(0xc000, 0xffff, "bank4");
	state->membank("bank4")->set_base(machine.device<ram_device>(RAM_TAG)->pointer() + 0xc000);

}

WRITE8_MEMBER(llc_state::llc2_rom_disable_w)
{
	address_space *mem_space = machine().device("maincpu")->memory().space(AS_PROGRAM);
	UINT8 *ram = machine().device<ram_device>(RAM_TAG)->pointer();

	mem_space->install_write_bank(0x0000, 0xbfff, "bank1");
	membank("bank1")->set_base(ram);

	mem_space->install_write_bank(0x4000, 0x5fff, "bank2");
	membank("bank2")->set_base(ram + 0x4000);

	mem_space->install_write_bank(0x6000, 0xbfff, "bank3");
	membank("bank3")->set_base(ram + 0x6000);

	mem_space->install_write_bank(0xc000, 0xffff, "bank4");
	membank("bank4")->set_base(ram + 0xc000);

}

WRITE8_MEMBER(llc_state::llc2_basic_enable_w)
{
	address_space *mem_space = machine().device("maincpu")->memory().space(AS_PROGRAM);
	if (data & 0x02)
	{
		mem_space->unmap_write(0x4000, 0x5fff);
		membank("bank2")->set_base(machine().root_device().memregion("maincpu")->base() + 0x10000);
	}
	else
	{
		mem_space->install_write_bank(0x4000, 0x5fff, "bank2");
		membank("bank2")->set_base(machine().device<ram_device>(RAM_TAG)->pointer() + 0x4000);
	}

}

READ8_MEMBER(llc_state::llc2_port1_b_r)
{
	return 0;
}

WRITE8_MEMBER(llc_state::llc2_port1_b_w)
{
	speaker_level_w(m_speaker, BIT(data, 6) );
	m_rv = BIT(data, 5);
}

Z80PIO_INTERFACE( llc2_z80pio1_intf )
{
	DEVCB_NULL,	/* callback when change interrupt status */
	DEVCB_DEVICE_MEMBER(K7659_KEYBOARD_TAG, k7659_keyboard_device, read),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(llc_state, llc2_port1_b_r),
	DEVCB_DRIVER_MEMBER(llc_state, llc2_port1_b_w),
	DEVCB_NULL
};

READ8_MEMBER(llc_state::llc2_port2_a_r)
{
	return 0; // bit 2 low or hangs on ^Z^X^C sequence
}

Z80PIO_INTERFACE( llc2_z80pio2_intf )
{
	DEVCB_NULL,	/* callback when change interrupt status */
	DEVCB_DRIVER_MEMBER(llc_state, llc2_port2_a_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

