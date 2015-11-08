// license:???
// copyright-holders:Sean Young,Tomas Karlsson
/*
** Spectravideo SVI-318 and SVI-328
**
** Sean Young, Tomas Karlsson
**
*/

#include "emu.h"
#include "video/mc6845.h"
#include "includes/svi318.h"
#include "cpu/z80/z80.h"
#include "video/tms9928a.h"
#include "imagedev/flopdrv.h"
#include "formats/svi_cas.h"
#include "sound/ay8910.h"

enum {
	SVI_INTERNAL    = 0,
	SVI_CART        = 1,
	SVI_EXPRAM2     = 2,
	SVI_EXPRAM3     = 3
};


/* Serial ports */

WRITE_LINE_MEMBER(svi318_state::ins8250_interrupt)
{
	if (m_bank_low != SVI_CART)
		m_maincpu->set_input_line(0, (state ? HOLD_LINE : CLEAR_LINE));
}

#if 0
static INS8250_REFRESH_CONNECT( svi318_com_refresh_connected )
{
	/* Motorola MC14412 modem */
	ins8250_handshake_in(device, UART8250_HANDSHAKE_IN_CTS|UART8250_HANDSHAKE_IN_DSR|UART8250_INPUTS_RING_INDICATOR|UART8250_INPUTS_DATA_CARRIER_DETECT);
}
#endif

/* Cartridge */

bool svi318_state::cart_verify(UINT8 *ROM)
{
	if (ROM[0] != 0xf3 || ROM[1] != 0x31)
		return false;

	return true;
}

DEVICE_IMAGE_LOAD_MEMBER( svi318_state, svi318_cart )
{
	UINT32 size = m_cart->common_get_size("rom");

	if (size > 0x8000)
	{
		logerror("Cart image %s larger than expected. Please report the issue.\n", image.filename());
		return IMAGE_INIT_FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	if (image.software_entry() == NULL && !cart_verify(m_cart->get_rom_base()))
		return IMAGE_INIT_FAIL;

	return IMAGE_INIT_PASS;
}



/* PPI */

/*
 PPI Port A Input (address 98H)
 Bit Name     Description
  1  TA       Joystick 1, /SENSE
  2  TB       Joystick 1, EOC
  3  TC       Joystick 2, /SENSE
  4  TD       Joystick 2, EOC
  5  TRIGGER1 Joystick 1, Trigger
  6  TRIGGER2 Joystick 2, Trigger
  7  /READY   Cassette, Ready
  8  CASR     Cassette, Read data
*/

READ8_MEMBER(svi318_state::ppi_port_a_r)
{
	int data = 0x0f;

	if (m_cassette->input() > 0.0038)
		data |= 0x80;
	if (!m_cassette->exists())
		data |= 0x40;

	data |= m_buttons->read() & 0x30;

	return data;
}

/*
 PPI Port B Input (address 99H)
 Bit Name Description
  1  IN0  Keyboard, Column status of selected line
  2  IN1  Keyboard, Column status of selected line
  3  IN2  Keyboard, Column status of selected line
  4  IN3  Keyboard, Column status of selected line
  5  IN4  Keyboard, Column status of selected line
  6  IN5  Keyboard, Column status of selected line
  7  IN6  Keyboard, Column status of selected line
  8  IN7  Keyboard, Column status of selected line
*/

READ8_MEMBER(svi318_state::ppi_port_b_r)
{
	if (m_keyboard_row <= 10)
		return m_line[m_keyboard_row]->read();
	else
		return 0xff;
}

/*
 PPI Port C Output (address 97H)
 Bit Name   Description
  1  KB0    Keyboard, Line select 0
  2  KB1    Keyboard, Line select 1
  3  KB2    Keyboard, Line select 2
  4  KB3    Keyboard, Line select 3
  5  CASON  Cassette, Motor relay control (0=on, 1=off)
  6  CASW   Cassette, Write data
  7  CASAUD Cassette, Audio out (pulse)
  8  SOUND  Keyboard, Click sound bit (pulse)
*/

WRITE8_MEMBER(svi318_state::ppi_port_c_w)
{
	int val;

	/* key click */
	val = (data & 0x80) ? 0x3e : 0;
	val += (data & 0x40) ? 0x3e : 0;
	m_dac->write_signed8(val);

	/* cassette motor on/off */
	if (m_cassette->exists())
	{
			m_cassette->change_state(
			(data & 0x10) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED,
			CASSETTE_MOTOR_DISABLED);
	}

	/* cassette signal write */
	m_cassette->output((data & 0x20) ? -1.0 : +1.0);

	m_keyboard_row = data & 0x0f;
}

WRITE8_MEMBER(svi318_state::ppi_w)
{
	m_ppi->write(space, offset + 2, data);
}


/* PSG */

/*
 PSG Port A Input
 Bit Name   Description
  1  FWD1   Joystick 1, Forward
  2  BACK1  Joystick 1, Back
  3  LEFT1  Joystick 1, Left
  4  RIGHT1 Joystick 1, Right
  5  FWD2   Joystick 2, Forward
  6  BACK2  Joystick 2, Back
  7  LEFT2  Joystick 2, Left
  8  RIGHT2 Joystick 2, Right
*/

READ8_MEMBER(svi318_state::psg_port_a_r)
{
	return m_joysticks->read();
}

/*
 PSG Port B Output
 Bit Name    Description
  1  /CART   Memory bank 11, ROM 0000-7FFF (Cartridge /CCS1, /CCS2)
  2  /BK21   Memory bank 21, RAM 0000-7FFF
  3  /BK22   Memory bank 22, RAM 8000-FFFF
  4  /BK31   Memory bank 31, RAM 0000-7FFF
  5  /BK32   Memory bank 32, RAM 8000-7FFF
  6  CAPS    Caps-Lock diod
  7  /ROMEN0 Memory bank 12, ROM 8000-BFFF* (Cartridge /CCS3)
  8  /ROMEN1 Memory bank 12, ROM C000-FFFF* (Cartridge /CCS4)

 * The /CART signal must be active for any effect and all banks
 with RAM are disabled.
*/

WRITE8_MEMBER(svi318_state::psg_port_b_w)
{
	if ((m_bank_switch ^ data) & 0x20)
		set_led_status(machine(), 0, !(data & 0x20));

	m_bank_switch = data;
	set_banks();
}

/* Disk drives  */

WRITE_LINE_MEMBER(svi318_state::fdc_intrq_w)
{
	m_irq = state;
}

WRITE_LINE_MEMBER(svi318_state::fdc_drq_w)
{
	m_drq = state;
}

WRITE8_MEMBER(svi318_state::fdc_drive_motor_w)
{
	m_floppy = NULL;

	if (BIT(data, 0)) m_floppy = m_floppy0->get_device();
	if (BIT(data, 1)) m_floppy = m_floppy1->get_device();

	m_fd1793->set_floppy(m_floppy);

	if (m_floppy0->get_device())
		m_floppy0->get_device()->mon_w(!BIT(data, 2));

	if (m_floppy1->get_device())
		m_floppy1->get_device()->mon_w(!BIT(data, 3));
}

WRITE8_MEMBER(svi318_state::fdc_density_side_w)
{
	m_fd1793->dden_w(BIT(data, 0));

	if (m_floppy)
		m_floppy->ss_w(BIT(data, 1));
}

READ8_MEMBER(svi318_state::fdc_irqdrq_r)
{
	UINT8 result = 0;

	result |= m_drq << 6;
	result |= m_irq << 7;

	return result;
}

MC6845_UPDATE_ROW( svi318_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();

	for (int i = 0; i < x_count; i++)
	{
		UINT8 data = m_svi806_gfx[m_svi806_ram[(ma + i) & 0x7ff] * 16 + ra];

		if (i == cursor_x)
		{
			data = 0xff;
		}

		for (int j = 0; j < 8; j++)
		{
			bitmap.pix32(y, i * 8 + j) = palette[TMS9928A_PALETTE_SIZE + BIT(data, 7)];
			data = data << 1;
		}
	}
}


WRITE8_MEMBER(svi318_state::svi806_ram_enable_w)
{
	m_svi806_ram_enabled = (data & 0x01);
	set_banks();
}


/* Init functions */

static const UINT8 cc_op[0x100] = {
	4+1,10+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1, 4+1,11+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	8+1,10+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,12+1,11+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	7+1,10+1,16+1, 6+1, 4+1, 4+1, 7+1, 4+1, 7+1,11+1,16+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	7+1,10+1,13+1, 6+1,11+1,11+1,10+1, 4+1, 7+1,11+1,13+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	7+1, 7+1, 7+1, 7+1, 7+1, 7+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	5+1,10+1,10+1,10+1,10+1,11+1, 7+1,11+1, 5+1,10+1,10+1, 0+1,10+1,17+1, 7+1,11+1,
	5+1,10+1,10+1,11+1,10+1,11+1, 7+1,11+1, 5+1, 4+1,10+1,11+1,10+1, 0+1, 7+1,11+1,
	5+1,10+1,10+1,19+1,10+1,11+1, 7+1,11+1, 5+1, 4+1,10+1, 4+1,10+1, 0+1, 7+1,11+1,
	5+1,10+1,10+1, 4+1,10+1,11+1, 7+1,11+1, 5+1, 6+1,10+1, 4+1,10+1, 0+1, 7+1,11+1
};

static const UINT8 cc_cb[0x100] = {
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2
};

static const UINT8 cc_ed[0x100] = {
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
12+2,12+2,15+2,20+2, 8+2,14+2, 8+2, 9+2,12+2,12+2,15+2,20+2, 8+2,14+2, 8+2, 9+2,
12+2,12+2,15+2,20+2, 8+2,14+2, 8+2, 9+2,12+2,12+2,15+2,20+2, 8+2,14+2, 8+2, 9+2,
12+2,12+2,15+2,20+2, 8+2,14+2, 8+2,18+2,12+2,12+2,15+2,20+2, 8+2,14+2, 8+2,18+2,
12+2,12+2,15+2,20+2, 8+2,14+2, 8+2, 8+2,12+2,12+2,15+2,20+2, 8+2,14+2, 8+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
16+2,16+2,16+2,16+2, 8+2, 8+2, 8+2, 8+2,16+2,16+2,16+2,16+2, 8+2, 8+2, 8+2, 8+2,
16+2,16+2,16+2,16+2, 8+2, 8+2, 8+2, 8+2,16+2,16+2,16+2,16+2, 8+2, 8+2, 8+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2
};

static const UINT8 cc_xy[0x100] = {
	4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2,15+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2,
	4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2,15+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2,
	4+2,14+2,20+2,10+2, 9+2, 9+2, 9+2, 4+2, 4+2,15+2,20+2,10+2, 9+2, 9+2, 9+2, 4+2,
	4+2, 4+2, 4+2, 4+2,23+2,23+2,19+2, 4+2, 4+2,15+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2,
	4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2, 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2,
	4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2, 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2,
	9+2, 9+2, 9+2, 9+2, 9+2, 9+2,19+2, 9+2, 9+2, 9+2, 9+2, 9+2, 9+2, 9+2,19+2, 9+2,
19+2,19+2,19+2,19+2,19+2,19+2, 4+2,19+2, 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2,
	4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2, 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2,
	4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2, 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2,
	4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2, 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2,
	4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2, 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2,
	4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 0+2, 4+2, 4+2, 4+2, 4+2,
	4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2,
	4+2,14+2, 4+2,23+2, 4+2,15+2, 4+2, 4+2, 4+2, 8+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2,
	4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2,10+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2
};

static const UINT8 cc_xycb[0x100] = {
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,
20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,
20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,
20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2
};

/* extra cycles if jr/jp/call taken and 'interrupt latency' on rst 0-7 */
static const UINT8 cc_ex[0x100] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* DJNZ */
	5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, /* JR NZ/JR Z */
	5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, /* JR NC/JR C */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0+1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	5, 5, 5, 5, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 0, 0, /* LDIR/CPIR/INIR/OTIR LDDR/CPDR/INDR/OTDR */
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2+1
};


DRIVER_INIT_MEMBER(svi318_state, svi318)
{
	/* z80 stuff */
	m_maincpu->z80_set_cycle_tables(cc_op, cc_cb, cc_ed, cc_xy, cc_xycb, cc_ex);

	m_maincpu->set_input_line_vector(0, 0xff);

	/* memory */
	m_empty_bank = auto_alloc_array(machine(), UINT8, 0x8000);
	memset(m_empty_bank, 0xff, 0x8000);

	m_bank_low_ptr = m_empty_bank;
	m_bank_high1_ptr = m_empty_bank;
	m_bank_high2_ptr = m_empty_bank;
}

DRIVER_INIT_MEMBER(svi318_state, svi328_806)
{
	DRIVER_INIT_CALL(svi318);
	m_svi806_present = 1;
}

void svi318_state::machine_start()
{
	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	m_bios_rom = memregion("maincpu");

	// 80 column card start
	if (m_svi806_present)
	{
		// 2K RAM, but allocating 4KB to make banking easier
		// The upper 2KB will be set to FFs and will never be written to
		m_svi806_ram.resize(0x1000);
		save_item(NAME(m_svi806_ram));
		memset(&m_svi806_ram[0], 0x00, 0x800);
		memset(&m_svi806_ram[0x800], 0xff, 0x800);

		m_svi806_gfx = memregion("gfx1")->base();

		// Set SVI-806 80 column card palette
		m_palette->set_pen_color(TMS9928A_PALETTE_SIZE, 0, 0, 0);     /* Monochrome black */
		m_palette->set_pen_color(TMS9928A_PALETTE_SIZE+1, 0, 224, 0); /* Monochrome green */
	}

	// register for savestates
	save_item(NAME(m_drq));
	save_item(NAME(m_irq));

	save_item(NAME(m_bank_switch));
	save_item(NAME(m_bank_low));
	save_item(NAME(m_bank_high));
	save_item(NAME(m_bank_low_read_only));
	save_item(NAME(m_bank_high1_read_only));
	save_item(NAME(m_bank_high2_read_only));
	save_item(NAME(m_keyboard_row));
	save_item(NAME(m_centronics_busy));

	save_item(NAME(m_svi806_present));
	save_item(NAME(m_svi806_ram_enabled));

	machine().save().register_postload(save_prepost_delegate(FUNC(svi318_state::postload), this));
}

void svi318_state::machine_reset()
{
	m_keyboard_row = 0;
	m_centronics_busy = 0;
	m_svi806_present = 0;
	m_svi806_ram_enabled = 0;
	m_drq = 0;
	m_irq = 0;

	m_bank_low = 0;
	m_bank_high = 0;
	m_bank_low_read_only = 0;
	m_bank_high1_read_only = 0;
	m_bank_high2_read_only = 0;

	m_bank_switch = 0xff;
	set_banks();
}

/* Memory */

WRITE8_MEMBER(svi318_state::writemem1)
{
	if (m_bank_low_read_only)
		return;

	m_bank_low_ptr[offset] = data;
}

WRITE8_MEMBER(svi318_state::writemem2)
{
	if (m_bank_high1_read_only)
		return;

	m_bank_high1_ptr[offset] = data;
}

WRITE8_MEMBER(svi318_state::writemem3)
{
	if (m_bank_high2_read_only)
		return;

	m_bank_high2_ptr[offset] = data;
}

WRITE8_MEMBER(svi318_state::writemem4)
{
	if (m_svi806_ram_enabled)
	{
		if (offset < 0x800)
			m_svi806_ram[offset] = data;
	}
	else
	{
		if (m_bank_high2_read_only)
			return;

		m_bank_high2_ptr[0x3000 + offset] = data;
	}
}

void svi318_state::set_banks()
{
	const UINT8 v = m_bank_switch;
	UINT8 *ram = m_ram->pointer();
	UINT32 ram_size = m_ram->size();

	m_bank_low = (v & 1) ? ((v & 2) ? ((v & 8) ? SVI_INTERNAL : SVI_EXPRAM3) : SVI_EXPRAM2) : SVI_CART;
	m_bank_high = (v & 4) ? ((v & 16) ? SVI_INTERNAL : SVI_EXPRAM3) : SVI_EXPRAM2;

	m_bank_low_ptr = m_empty_bank;
	m_bank_low_read_only = 1;

	switch (m_bank_low)
	{
	case SVI_INTERNAL:
		m_bank_low_ptr = m_bios_rom->base();
		break;
	case SVI_CART:
		if (m_cart_rom)
			m_bank_low_ptr = m_cart_rom->base();
		break;
	case SVI_EXPRAM2:
		if (ram_size >= 64 * 1024)
		{
			m_bank_low_ptr = ram + ram_size - 64 * 1024;
			m_bank_low_read_only = 0;
		}
		break;
	case SVI_EXPRAM3:
		if (ram_size > 128 * 1024)
		{
			m_bank_low_ptr = ram + ram_size - 128 * 1024;
			m_bank_low_read_only = 0;
		}
		break;
	}

	m_bank_high1_ptr = m_empty_bank;
	m_bank_high1_read_only = 1;
	m_bank_high2_ptr = m_empty_bank;
	m_bank_high2_read_only = 1;

	switch (m_bank_high)
	{
	case SVI_INTERNAL:
		if (ram_size == 16 * 1024)
		{
			m_bank_high2_ptr = ram;
			m_bank_high2_read_only = 0;
		}
		else
		{
			m_bank_high1_ptr = ram;
			m_bank_high1_read_only = 0;
			m_bank_high2_ptr = ram + 0x4000;
			m_bank_high2_read_only = 0;
		}
		break;
	case SVI_EXPRAM2:
		if (ram_size > 64 * 1024)
		{
			m_bank_high1_ptr = ram + ram_size - 64 * 1024 + 32 * 1024;
			m_bank_high1_read_only = 0;
			m_bank_high2_ptr = ram + ram_size - 64 * 1024 + 48 * 1024;
			m_bank_high2_read_only = 0;
		}
		break;
	case SVI_EXPRAM3:
		if (ram_size > 128 * 1024)
		{
			m_bank_high1_ptr = ram + ram_size - 128 * 1024 + 32 * 1024;
			m_bank_high1_read_only = 0;
			m_bank_high2_ptr = ram + ram_size - 128 * 1024 + 48 * 1024;
			m_bank_high2_read_only = 0;
		}
		break;
	}

	/* Check for special CART based banking */
	if (m_bank_low == SVI_CART && (v & 0xc0 ) != 0xc0)
	{
		m_bank_high1_ptr = m_empty_bank;
		m_bank_high1_read_only = 1;
		m_bank_high2_ptr = m_empty_bank;
		m_bank_high2_read_only = 1;

		if (m_cart_rom && !(v & 0x80))
			m_bank_high2_ptr = m_cart_rom->base() + 0x4000;
		if (m_cart_rom && !(v & 0x40))
			m_bank_high1_ptr = m_cart_rom->base();
	}

	m_bank1->set_base(m_bank_low_ptr);
	m_bank2->set_base(m_bank_high1_ptr);
	m_bank3->set_base(m_bank_high2_ptr);

	/* SVI-806 80 column card specific banking */
	if (m_svi806_present)
	{
		if (m_svi806_ram_enabled)
			m_bank4->set_base(&m_svi806_ram[0]);
		else
			m_bank4->set_base(m_bank_high2_ptr + 0x3000);
	}
}

void svi318_state::postload()
{
	set_banks();
}


/* External I/O */

WRITE_LINE_MEMBER(svi318_state::write_centronics_busy)
{
	m_centronics_busy = state;
}

READ8_MEMBER(svi318_state::io_ext_r)
{
	if (m_bank_low == SVI_CART)
		return 0xff;

	switch (offset)
	{
	case 0x12:
		return 0xfe | m_centronics_busy;

	case 0x20:
	case 0x21:
	case 0x22:
	case 0x23:
	case 0x24:
	case 0x25:
	case 0x26:
	case 0x27:
		return m_ins8250_0->ins8250_r(space, offset & 7);

	case 0x28:
	case 0x29:
	case 0x2a:
	case 0x2b:
	case 0x2c:
	case 0x2d:
	case 0x2e:
	case 0x2f:
		return m_ins8250_1->ins8250_r(space, offset & 7);

	case 0x30:
		return m_fd1793->status_r(space, 0);

	case 0x31:
		return m_fd1793->track_r(space, 0);

	case 0x32:
		return m_fd1793->sector_r(space, 0);

	case 0x33:
		return m_fd1793->data_r(space, 0);

	case 0x34:
		return fdc_irqdrq_r(space, 0);

	case 0x51:
		return m_crtc->register_r(space, offset);
	}

	return 0xff;
}

WRITE8_MEMBER(svi318_state::io_ext_w)
{
	if (m_bank_low == SVI_CART)
		return;

	switch (offset)
	{
	case 0x10:
		m_cent_data_out->write(space, 0, data);
		break;

	case 0x11:
		m_centronics->write_strobe(BIT(data, 0));
		break;

	case 0x20:
	case 0x21:
	case 0x22:
	case 0x23:
	case 0x24:
	case 0x25:
	case 0x26:
	case 0x27:
		m_ins8250_0->ins8250_w(space, offset & 7, data);
		break;

	case 0x28:
	case 0x29:
	case 0x2A:
	case 0x2B:
	case 0x2C:
	case 0x2D:
	case 0x2E:
	case 0x2F:
		m_ins8250_1->ins8250_w(space, offset & 7, data);
		break;

	case 0x30:
		m_fd1793->cmd_w(space, 0, data);
		break;
	case 0x31:
		m_fd1793->track_w(space, 0, data);
		break;
	case 0x32:
		m_fd1793->sector_w(space, 0, data);
		break;
	case 0x33:
		m_fd1793->data_w(space, 0, data);
		break;
	case 0x34:
		fdc_drive_motor_w(space, 0, data);
		break;
	case 0x38:
		fdc_density_side_w(space, 0, data);
		break;

	case 0x50:
		m_crtc->address_w(space, offset, data);
		break;
	case 0x51:
		m_crtc->register_w(space, offset, data);
		break;

	case 0x58:
		svi806_ram_enable_w(space, 0, data);
		break;
	}
}
