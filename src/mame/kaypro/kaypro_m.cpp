// license:BSD-3-Clause
// copyright-holders:Robbbert


#include "emu.h"
#include "kaypro.h"




/***********************************************************

    PIO

    Port B is unused on both PIOs

************************************************************/

void kaypro_state::write_centronics_busy(int state)
{
	m_centronics_busy = state;
}

u8 kayproii_state::pio_system_r()
{
	u8 data = 0;

	/* centronics busy */
	data |= m_centronics_busy << 3;

	/* PA7 is pulled high */
	data |= 0x80;

	return data;
}

void kayproii_state::kayproii_pio_system_w(u8 data)
{
/*  d7 bank select
    d6 disk drive motors - (0=on)
    d5 double-density enable (0=double density)
    d4 Centronics strobe
    d2 side select (1=side 1)
    d1 drive B
    d0 drive A */

	m_bankr->set_entry(BIT(data, 7));
	m_bankw->set_entry(BIT(data, 7));
	m_bank3->set_entry(BIT(data, 7));
	m_is_motor_off = BIT(data, 6);

	m_floppy = nullptr;
	if (BIT(data, 0))
		m_floppy = m_floppy0->get_device();
	else
	if (BIT(data, 1))
		m_floppy = m_floppy1->get_device();

	m_fdc->set_floppy(m_floppy);
	m_fdc->dden_w(BIT(data, 5));

	if (m_floppy)
	{
		m_floppy->mon_w(BIT(data, 6)); // motor on
		m_floppy->ss_w(!BIT(data, 2)); // signal exists even though drives are single sided
	}

	m_leds[0] = BIT(data, 0);     // LEDs in artwork
	m_leds[1] = BIT(data, 1);

	m_centronics->write_strobe(BIT(data, 4));

	m_system_port = data;
}

void kayproii_state::kayproiv_pio_system_w(u8 data)
{
	kayproii_pio_system_w(data);

	/* side select */
	if (m_floppy)
		m_floppy->ss_w(BIT(data, 2));
}

/***********************************************************

    KAYPRO484 SYSTEM PORT

    The PIOs were replaced by a few standard 74xx chips

************************************************************/

u8 kaypro84_state::kaypro484_system_port_r()
{
	u8 data = m_centronics_busy << 6;
	return (m_system_port & 0xbf) | data;
}

void kaypro84_state::kaypro484_system_port_w(u8 data)
{
/*  d7 bank select
    d6 alternate character set (write only)
    d5 double-density enable
    d4 disk drive motors (1=on)
    d3 Centronics strobe
    d2 side select (appears that 0=side 1?)
    d1 drive B
    d0 drive A */

	m_bankr->set_entry(BIT(data, 7));
	m_bankw->set_entry(BIT(data, 7));
	m_bank3->set_entry(BIT(data, 7));
	m_is_motor_off = !BIT(data, 4);

	m_floppy = nullptr;
	if (!BIT(data, 0))
		m_floppy = m_floppy0->get_device();
	else
	if (m_floppy1 && (!BIT(data, 1)))
		m_floppy = m_floppy1->get_device();

	m_fdc->set_floppy(m_floppy);
	m_fdc->dden_w(BIT(data, 5));

	if (m_floppy)
	{
		m_floppy->mon_w(!BIT(data, 4)); // motor on
		m_floppy->ss_w(!BIT(data, 2));
	}

	m_leds[0] = BIT(data, 0);     // LEDs in artwork
	m_leds[1] = BIT(data, 1);

	m_centronics->write_strobe(BIT(data, 3));

	m_system_port = data;
}


/***********************************************************************

    SIO

    On Kaypro484, Channel B on both SIOs is hardwired to 300 baud.

    Both devices on sio2 (printer and modem) are not emulated.

************************************************************************/

/* Set baud rate. bits 0..3 Rx and Tx are tied together. Baud Rate Generator is a AY-5-8116, SMC8116, WD1943, etc.
    00h    50
    11h    75
    22h    110
    33h    134.5
    44h    150
    55h    300
    66h    600
    77h    1200
    88h    1800
    99h    2000
    AAh    2400
    BBh    3600
    CCh    4800
    DDh    7200
    EEh    9600
    FFh    19200 */


/*************************************************************************************

    Floppy Disk

    If DRQ or IRQ is set, and cpu is halted, the NMI goes low.
    Since the HALT occurs last (and has no callback mechanism), we need to set
    a short delay, to give time for the processor to execute the HALT before NMI
    becomes active.

*************************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(kaypro_state::floppy_timer)
{
	bool halt;
	halt = (bool)m_maincpu->state_int(Z80_HALT);
	if (m_is_motor_off)
	{
		m_floppy_timer->adjust(attotime::from_hz(10));
		return;
	}

	if ((halt) && (m_fdc_rq & 3) && (m_fdc_rq < 0x80))
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		m_fdc_rq |= 0x80;
	}
	else
	if ((m_fdc_rq == 0x80) || ((!halt) && BIT(m_fdc_rq, 7)))
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		m_fdc_rq &= 0x7f;
	}
	m_floppy_timer->adjust(attotime::from_hz(1e5));
}


void kaypro_state::fdc_intrq_w(int state)
{
	m_fdc_rq = (m_fdc_rq & 0x82) | state;
}

void kaypro_state::fdc_drq_w(int state)
{
	m_fdc_rq = (m_fdc_rq & 0x81) | (state << 1);
}


void kaypro84_state::rtc_address_w(u8 data)
{
	m_rtc_address = data & 0x1f;
}

u8 kaypro84_state::rtc_r()
{
	return m_rtc->read(m_rtc_address);
}

void kaypro84_state::rtc_w(u8 data)
{
	m_rtc->write(m_rtc_address, data);
}


/***********************************************************

    Machine

************************************************************/
void kaypro_state::machine_start()
{
	m_leds.resolve();

	save_pointer(NAME(m_vram), 0x1000);
	save_pointer(NAME(m_ram),  0x4000);

	save_item(NAME(m_framecnt));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_is_motor_off));
	save_item(NAME(m_fdc_rq));
	save_item(NAME(m_system_port));

	m_framecnt = 0;
}

void kayproii_state::machine_start()
{
	kaypro_state::machine_start();

	m_pio_s->strobe_a(0);
}

void kaypro84_state::machine_start()
{
	kaypro_state::machine_start();

	save_item(NAME(m_mc6845_reg));
	save_item(NAME(m_mc6845_ind));
	save_item(NAME(m_mc6845_video_address));

	if (m_rtc.found())
		save_item(NAME(m_rtc_address));
}

void kaypro_state::machine_reset()
{
	m_bankr->set_entry(1); // point at rom
	m_bankw->set_entry(1); // always write to ram
	m_bank3->set_entry(1); // point at video ram
	m_system_port = 0x80;
	m_fdc_rq = 0;
	m_maincpu->reset();
	m_floppy_timer->adjust(attotime::from_hz(1));   /* kick-start the nmi timer */
}


/***********************************************************

    Quickload

    This loads a .COM file to address 0x100 then jumps
    there. Sometimes .COM has been renamed to .CPM to
    prevent windows going ballistic. These can be loaded
    as well.

************************************************************/

QUICKLOAD_LOAD_MEMBER(kaypro_state::quickload_cb)
{
	m_bankr->set_entry(0);
	m_bankw->set_entry(0);
	m_bank3->set_entry(0);

	address_space& prog_space = m_maincpu->space(AS_PROGRAM);

	// Avoid loading a program if CP/M-80 is not in memory
	if ((prog_space.read_byte(0) != 0xc3) || (prog_space.read_byte(5) != 0xc3))
		return std::make_pair(image_error::UNSUPPORTED, "CP/M must already be running");

	// Check for sufficient RAM based on position of CPM
	const int mem_avail = 256 * prog_space.read_byte(7) + prog_space.read_byte(6) - 512;
	if (mem_avail < image.length())
		return std::make_pair(image_error::UNSPECIFIED, "Insufficient memory available");

	// Load image to the TPA (Transient Program Area)
	u16 quickload_size = image.length();
	for (u16 i = 0; i < quickload_size; i++)
	{
		u8 data;
		if (image.fread(&data, 1) != 1)
			return std::make_pair(image_error::UNSPECIFIED, "Problem reading the image at offset " + std::to_string(i));
		prog_space.write_byte(i + 0x100, data);
	}

	// clear out command tail
	prog_space.write_byte(0x80, 0);
	prog_space.write_byte(0x81, 0);

	m_maincpu->set_state_int(Z80_SP, mem_avail + 384); // put the stack a bit before BDOS
	m_maincpu->set_pc(0x100); // start program

	return std::make_pair(std::error_condition(), std::string());
}
