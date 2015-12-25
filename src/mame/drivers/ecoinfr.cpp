// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Electrocoin Fruit Machines

 This seems to be the most common Electrocoin hardware type, used
 extensively by Electrocoin with a number of 3rd party sets running on
 the same boards / same cabinets to provide different gameplay features.
 Some of these are scrambled (see the MAB sets)

 Build dates in the sets seem to range from 1991 - 2007
  (todo, put correct dates for each rom in the driver)

 A PIC16C54A is used for security.

 the UART chips in the board are either an NEC D71051C-10 or an OKI M82C51A-2

------------------------------

 WARNING: I have little faith in many of these sets being what they claim to be,
          the headers in the ROMs often indicate different titles, and sevearl of
          the sets contain what look to be games on different hardware, or entirely
          different game, there was even a Data East Pinball ROM in with this lot.

          Proceed with caution if emulating this stuff, if in doubt, and things
          aren't acting as you expect, try one of the other ROMs from the sets!

          Some roms are in HEX format and should be converted to binary.


  30/08/11 - Started to sort out the roms by header type, some things are clearly
             just newer revisions / alt titles of other things. DH


 ------
 the 'Brunel Research' (ec_sbxbr) sets seem interesting for now
 ------

*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8251.h"
#include "ecoinfr.lh"
#include "machine/steppers.h" // stepper motor
#include "video/awpvid.h" // drawing reels


#define UPD8251_TAG      "upd8251"

#define GAME_FLAGS MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL

class ecoinfr_state : public driver_device
{
public:
	ecoinfr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_reel0(*this, "reel0"),
		m_reel1(*this, "reel1"),
		m_reel2(*this, "reel2"),
		m_reel3(*this, "reel3")
		{ }

	int irq_toggle;
	int m_optic_pattern;
	DECLARE_WRITE_LINE_MEMBER(reel0_optic_cb) { if (state) m_optic_pattern |= 0x01; else m_optic_pattern &= ~0x01; }
	DECLARE_WRITE_LINE_MEMBER(reel1_optic_cb) { if (state) m_optic_pattern |= 0x02; else m_optic_pattern &= ~0x02; }
	DECLARE_WRITE_LINE_MEMBER(reel2_optic_cb) { if (state) m_optic_pattern |= 0x04; else m_optic_pattern &= ~0x04; }
	DECLARE_WRITE_LINE_MEMBER(reel3_optic_cb) { if (state) m_optic_pattern |= 0x08; else m_optic_pattern &= ~0x08; }

	UINT8 port09_value;
	UINT8 port10_value;
	UINT8 port11_value;
	UINT8 port12_value;
	UINT8 port13_value;
	UINT8 port14_value;
	UINT8 port15_value;
	UINT8 port16_value;
	UINT8 port17_value;

	DECLARE_WRITE8_MEMBER(ec_port00_out_w);
	DECLARE_WRITE8_MEMBER(ec_port01_out_w);
	DECLARE_WRITE8_MEMBER(ec_port02_out_w);
	DECLARE_WRITE8_MEMBER(ec_port03_out_w);
	DECLARE_WRITE8_MEMBER(ec_port04_out_w);
	DECLARE_WRITE8_MEMBER(ec_port05_out_w);
	DECLARE_WRITE8_MEMBER(ec_port06_out_w);
	DECLARE_WRITE8_MEMBER(ec_port07_out_w);
	DECLARE_WRITE8_MEMBER(ec_port08_out_bank_strobe_w);
	DECLARE_WRITE8_MEMBER(ec_port09_out_reelen_w);
	DECLARE_WRITE8_MEMBER(ec_port0a_out_w);
	DECLARE_WRITE8_MEMBER(ec_port0b_out_w);
	DECLARE_WRITE8_MEMBER(ec_port0c_out_cred_strobe_w);
	DECLARE_WRITE8_MEMBER(ec_port0d_out_cred_data_w);
	DECLARE_WRITE8_MEMBER(ec_port0e_out_w);
	DECLARE_WRITE8_MEMBER(ec_port0f_out_bank_segdata_w);
	DECLARE_WRITE8_MEMBER(ec_port10_out_w);
	DECLARE_WRITE8_MEMBER(ec_port11_out_w);
	DECLARE_WRITE8_MEMBER(ec_port12_out_w);
	DECLARE_WRITE8_MEMBER(ec_port13_out_w);
	DECLARE_WRITE8_MEMBER(ec_port14_out_w);
	DECLARE_WRITE8_MEMBER(ec_port15_out_w);
	DECLARE_WRITE8_MEMBER(ec_port16_out_w);
	DECLARE_WRITE8_MEMBER(ec_port17_out_w);
	DECLARE_WRITE8_MEMBER(ec_port18_out_w);
	DECLARE_CUSTOM_INPUT_MEMBER(ecoinfr_reel1_opto_r);
	DECLARE_CUSTOM_INPUT_MEMBER(ecoinfr_reel2_opto_r);
	DECLARE_CUSTOM_INPUT_MEMBER(ecoinfr_reel3_opto_r);

	DECLARE_DRIVER_INIT(ecoinfrbr);
	DECLARE_DRIVER_INIT(ecoinfr);
	DECLARE_DRIVER_INIT(ecoinfrmab);
	virtual void machine_reset() override;
	TIMER_DEVICE_CALLBACK_MEMBER(ecoinfr_irq_timer);

	UINT8 m_banksel;
	UINT8 m_credsel;

	required_device<cpu_device> m_maincpu;
	required_device<stepper_device> m_reel0;
	required_device<stepper_device> m_reel1;
	required_device<stepper_device> m_reel2;
	required_device<stepper_device> m_reel3;
};



TIMER_DEVICE_CALLBACK_MEMBER(ecoinfr_state::ecoinfr_irq_timer)
{
	irq_toggle^=1;

	//printf("blah %d\n", irq_toggle);

	/* What are the IRQ sources / freq?
	 It runs in IM2
	 0xe0 / 0xe4 seem to be the valid interrupts
	 0xf0 / 0xf4 mirror those

	 there seem to be plenty of valid looking vectors between
	 these too.. really not sure what all the sources are, nor
	 the frequency

	 NMI is also valid

	*/

	if (irq_toggle==0)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xe4);
	}
	else
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xe0);
	}


}


WRITE8_MEMBER(ecoinfr_state::ec_port00_out_w)
{
	if (data&0x70) // lots of games set 0x80
	{
		printf("ec_port0a_out_w (reel 1 port) unk bits used %02x\n", data);
	}

	m_reel0->update(data&0x0f);

	awp_draw_reel("reel1", m_reel0);
}

WRITE8_MEMBER(ecoinfr_state::ec_port01_out_w)
{
	if (data&0xf0)
	{
		printf("ec_port01_out_w (reel 2 port) unk bits used %02x\n", data);
	}

	m_reel1->update(data&0x0f);

	awp_draw_reel("reel2", m_reel1);
}

WRITE8_MEMBER(ecoinfr_state::ec_port02_out_w)
{
	if (data&0xf0)
	{
		printf("ec_port02_out_w (reel 3 port) unk bits used %02x\n", data);
	}

	m_reel2->update(data&0x0f);

	awp_draw_reel("reel3", m_reel2);
}



WRITE8_MEMBER(ecoinfr_state::ec_port03_out_w)
{
}

WRITE8_MEMBER(ecoinfr_state::ec_port04_out_w)
{
}

WRITE8_MEMBER(ecoinfr_state::ec_port05_out_w)
{
}

WRITE8_MEMBER(ecoinfr_state::ec_port06_out_w)
{
}

WRITE8_MEMBER(ecoinfr_state::ec_port07_out_w)
{
}

WRITE8_MEMBER(ecoinfr_state::ec_port08_out_bank_strobe_w)
{
	switch (data)
	{
		case 0x00:
			// ?? written after select values
			break;

		case 0x01:
			m_banksel = 0;
			break;
		case 0x02:
			m_banksel = 1;
			break;
		case 0x04:
			m_banksel = 2;
			break;
		case 0x08:
			m_banksel = 3;
			break;
		case 0x10:
			m_banksel = 4;
			break;
		case 0x20:
			m_banksel = 5;
			break;
		case 0x40:
			m_banksel = 6;
			break;
		case 0x80:
			m_banksel = 7;
			break;

		default:
			printf("ec_port08_out_bank_strobe_w unk data %02x\n", data);
			break;

	}
}

// we could do the same thing here with input ports configured to outputs, however
// I've done it with handlers for now as it allows greater flexibility while the driver
// is developed
WRITE8_MEMBER(ecoinfr_state::ec_port09_out_reelen_w)
{
	int old_port09_value = port09_value;
	port09_value = data;

	if ((port09_value&0x01) != (old_port09_value&0x01)) printf("port09 0x01 changed %02x\n", port09_value&0x01);
	if ((port09_value&0x02) != (old_port09_value&0x02)) printf("port09 0x02 changed %02x\n", port09_value&0x02);
	if ((port09_value&0x04) != (old_port09_value&0x04)) printf("port09 0x04 changed %02x (REEL3 ENABLE)\n", port09_value&0x04);
	if ((port09_value&0x08) != (old_port09_value&0x08)) printf("port09 0x08 changed %02x (REEL2 ENABLE)\n", port09_value&0x08);
	if ((port09_value&0x10) != (old_port09_value&0x10)) printf("port09 0x10 changed %02x (REEL1 ENABLE)\n", port09_value&0x10);
	if ((port09_value&0x20) != (old_port09_value&0x20)) printf("port09 0x20 changed %02x\n", port09_value&0x20);
	if ((port09_value&0x40) != (old_port09_value&0x40)) printf("port09 0x40 changed %02x\n", port09_value&0x40);
	if ((port09_value&0x80) != (old_port09_value&0x80)) printf("port09 0x80 changed %02x\n", port09_value&0x80);
}

WRITE8_MEMBER(ecoinfr_state::ec_port0a_out_w)
{
}


WRITE8_MEMBER(ecoinfr_state::ec_port0b_out_w)
{
}

WRITE8_MEMBER(ecoinfr_state::ec_port0c_out_cred_strobe_w)
{
	switch (data)
	{
	case 0x00:
		break;
	case 0x01:
		m_credsel = 0;
		break;
	case 0x02:
		m_credsel = 1;
		break;
	case 0x04:
		m_credsel = 2;
		break;
	case 0x08:
		m_credsel = 3;
		break;
	case 0x10:
		m_credsel = 4;
		break;
	case 0x20:
		m_credsel = 5;
		break;
	case 0x40:
		m_credsel = 6;
		break;
	case 0x80:
		m_credsel = 7;
		break;
	default:
		printf("ec_port0c_out_cred_strobe_w unk %02x\n", data);
	}
}

WRITE8_MEMBER(ecoinfr_state::ec_port0d_out_cred_data_w)
{
	if (m_credsel!=0xff)
	{
		UINT8 bf7segdata = BITSWAP8(data,7,0,1,2,3,4,5,6);
		output_set_digit_value(m_credsel+8, bf7segdata);
	}
}

WRITE8_MEMBER(ecoinfr_state::ec_port0e_out_w)
{
}

WRITE8_MEMBER(ecoinfr_state::ec_port0f_out_bank_segdata_w)
{
	if (m_banksel!=0xff)
	{
		UINT8 bf7segdata = BITSWAP8(data,7,0,1,2,3,4,5,6);
		output_set_digit_value(m_banksel, bf7segdata);
	}
}

WRITE8_MEMBER(ecoinfr_state::ec_port10_out_w)
{
	int old_port10_value = port10_value;
	port10_value = data;

	if ((port10_value&0x01) != (old_port10_value&0x01)) printf("port10 0x01 changed %02x\n", port10_value&0x01);
	if ((port10_value&0x02) != (old_port10_value&0x02)) printf("port10 0x02 changed %02x\n", port10_value&0x02);
	if ((port10_value&0x04) != (old_port10_value&0x04)) printf("port10 0x04 changed %02x\n", port10_value&0x04);
	if ((port10_value&0x08) != (old_port10_value&0x08)) printf("port10 0x08 changed %02x\n", port10_value&0x08);
	if ((port10_value&0x10) != (old_port10_value&0x10)) printf("port10 0x10 changed %02x\n", port10_value&0x10);
	if ((port10_value&0x20) != (old_port10_value&0x20)) printf("port10 0x20 changed %02x\n", port10_value&0x20);
	if ((port10_value&0x40) != (old_port10_value&0x40)) printf("port10 0x40 changed %02x\n", port10_value&0x40);
	if ((port10_value&0x80) != (old_port10_value&0x80)) printf("port10 0x80 changed %02x\n", port10_value&0x80);
}

WRITE8_MEMBER(ecoinfr_state::ec_port11_out_w)
{
	int old_port11_value = port11_value;
	port11_value = data;

	if ((port11_value&0x01) != (old_port11_value&0x01)) printf("port11 0x01 changed %02x\n", port11_value&0x01);
	if ((port11_value&0x02) != (old_port11_value&0x02)) printf("port11 0x02 changed %02x\n", port11_value&0x02);
	if ((port11_value&0x04) != (old_port11_value&0x04)) printf("port11 0x04 changed %02x\n", port11_value&0x04);
	if ((port11_value&0x08) != (old_port11_value&0x08)) printf("port11 0x08 changed %02x\n", port11_value&0x08);
	if ((port11_value&0x10) != (old_port11_value&0x10)) printf("port11 0x10 changed %02x\n", port11_value&0x10);
	if ((port11_value&0x20) != (old_port11_value&0x20)) printf("port11 0x20 changed %02x\n", port11_value&0x20);
	if ((port11_value&0x40) != (old_port11_value&0x40)) printf("port11 0x40 changed %02x\n", port11_value&0x40);
	if ((port11_value&0x80) != (old_port11_value&0x80)) printf("port11 0x80 changed %02x\n", port11_value&0x80);
}

WRITE8_MEMBER(ecoinfr_state::ec_port12_out_w)
{
	int old_port12_value = port12_value;
	port12_value = data;

	if ((port12_value&0x01) != (old_port12_value&0x01)) printf("port12 0x01 changed %02x\n", port12_value&0x01);
	if ((port12_value&0x02) != (old_port12_value&0x02)) printf("port12 0x02 changed %02x\n", port12_value&0x02);
	if ((port12_value&0x04) != (old_port12_value&0x04)) printf("port12 0x04 changed %02x\n", port12_value&0x04);
	if ((port12_value&0x08) != (old_port12_value&0x08)) printf("port12 0x08 changed %02x\n", port12_value&0x08);
	if ((port12_value&0x10) != (old_port12_value&0x10)) printf("port12 0x10 changed %02x\n", port12_value&0x10);
	if ((port12_value&0x20) != (old_port12_value&0x20)) printf("port12 0x20 changed %02x\n", port12_value&0x20);
	if ((port12_value&0x40) != (old_port12_value&0x40)) printf("port12 0x40 changed %02x\n", port12_value&0x40);
	if ((port12_value&0x80) != (old_port12_value&0x80)) printf("port12 0x80 changed %02x\n", port12_value&0x80);
}

WRITE8_MEMBER(ecoinfr_state::ec_port13_out_w)
{
	int old_port13_value = port13_value;
	port13_value = data;

	if ((port13_value&0x01) != (old_port13_value&0x01)) printf("port13 0x01 changed %02x\n", port13_value&0x01);
	if ((port13_value&0x02) != (old_port13_value&0x02)) printf("port13 0x02 changed %02x\n", port13_value&0x02);
	if ((port13_value&0x04) != (old_port13_value&0x04)) printf("port13 0x04 changed %02x\n", port13_value&0x04);
	if ((port13_value&0x08) != (old_port13_value&0x08)) printf("port13 0x08 changed %02x\n", port13_value&0x08);
	if ((port13_value&0x10) != (old_port13_value&0x10)) printf("port13 0x10 changed %02x\n", port13_value&0x10);
	if ((port13_value&0x20) != (old_port13_value&0x20)) printf("port13 0x20 changed %02x\n", port13_value&0x20);
	if ((port13_value&0x40) != (old_port13_value&0x40)) printf("port13 0x40 changed %02x\n", port13_value&0x40);
	if ((port13_value&0x80) != (old_port13_value&0x80)) printf("port13 0x80 changed %02x\n", port13_value&0x80);
}

WRITE8_MEMBER(ecoinfr_state::ec_port14_out_w)
{
	int old_port14_value = port14_value;
	port14_value = data;

	if ((port14_value&0x01) != (old_port14_value&0x01)) printf("port14 0x01 changed %02x\n", port14_value&0x01);
	if ((port14_value&0x02) != (old_port14_value&0x02)) printf("port14 0x02 changed %02x\n", port14_value&0x02);
	if ((port14_value&0x04) != (old_port14_value&0x04)) printf("port14 0x04 changed %02x\n", port14_value&0x04);
	if ((port14_value&0x08) != (old_port14_value&0x08)) printf("port14 0x08 changed %02x\n", port14_value&0x08);
	if ((port14_value&0x10) != (old_port14_value&0x10)) printf("port14 0x10 changed %02x\n", port14_value&0x10);
	if ((port14_value&0x20) != (old_port14_value&0x20)) printf("port14 0x20 changed %02x\n", port14_value&0x20);
	if ((port14_value&0x40) != (old_port14_value&0x40)) printf("port14 0x40 changed %02x\n", port14_value&0x40);
	if ((port14_value&0x80) != (old_port14_value&0x80)) printf("port14 0x80 changed %02x\n", port14_value&0x80);
}

WRITE8_MEMBER(ecoinfr_state::ec_port15_out_w)
{
	int old_port15_value = port15_value;
	port15_value = data;

	if ((port15_value&0x01) != (old_port15_value&0x01)) printf("port15 0x01 changed %02x\n", port15_value&0x01);
	if ((port15_value&0x02) != (old_port15_value&0x02)) printf("port15 0x02 changed %02x\n", port15_value&0x02);
	if ((port15_value&0x04) != (old_port15_value&0x04)) printf("port15 0x04 changed %02x\n", port15_value&0x04);
	if ((port15_value&0x08) != (old_port15_value&0x08)) printf("port15 0x08 changed %02x\n", port15_value&0x08);
	if ((port15_value&0x10) != (old_port15_value&0x10)) printf("port15 0x10 changed %02x\n", port15_value&0x10);
	if ((port15_value&0x20) != (old_port15_value&0x20)) printf("port15 0x20 changed %02x\n", port15_value&0x20);
	if ((port15_value&0x40) != (old_port15_value&0x40)) printf("port15 0x40 changed %02x\n", port15_value&0x40);
	if ((port15_value&0x80) != (old_port15_value&0x80)) printf("port15 0x80 changed %02x\n", port15_value&0x80);

	// some 3rd party stuff has VDF
	//  printf("ec_port15_out_w data %02x - VDF reset %02x clock %02x\n", data, data & 0x80, data & 0x40);
}

WRITE8_MEMBER(ecoinfr_state::ec_port16_out_w)
{
	int old_port16_value = port16_value;
	port16_value = data;

	if ((port16_value&0x01) != (old_port16_value&0x01)) printf("port16 0x01 changed %02x\n", port16_value&0x01);
	if ((port16_value&0x02) != (old_port16_value&0x02)) printf("port16 0x02 changed %02x\n", port16_value&0x02);
	if ((port16_value&0x04) != (old_port16_value&0x04)) printf("port16 0x04 changed %02x\n", port16_value&0x04);
	if ((port16_value&0x08) != (old_port16_value&0x08)) printf("port16 0x08 changed %02x\n", port16_value&0x08);
	if ((port16_value&0x10) != (old_port16_value&0x10)) printf("port16 0x10 changed %02x\n", port16_value&0x10);
	if ((port16_value&0x20) != (old_port16_value&0x20)) printf("port16 0x20 changed %02x\n", port16_value&0x20);
	if ((port16_value&0x40) != (old_port16_value&0x40)) printf("port16 0x40 changed %02x\n", port16_value&0x40);
	if ((port16_value&0x80) != (old_port16_value&0x80)) printf("port16 0x80 changed %02x\n", port16_value&0x80);
}

WRITE8_MEMBER(ecoinfr_state::ec_port17_out_w)
{
	int old_port17_value = port17_value;
	port17_value = data;

	if ((port17_value&0x01) != (old_port17_value&0x01)) printf("port17 0x01 changed %02x\n", port17_value&0x01);
	if ((port17_value&0x02) != (old_port17_value&0x02)) printf("port17 0x02 changed %02x\n", port17_value&0x02);
	if ((port17_value&0x04) != (old_port17_value&0x04)) printf("port17 0x04 changed %02x\n", port17_value&0x04);
	if ((port17_value&0x08) != (old_port17_value&0x08)) printf("port17 0x08 changed %02x\n", port17_value&0x08);
	if ((port17_value&0x10) != (old_port17_value&0x10)) printf("port17 0x10 changed %02x\n", port17_value&0x10);
	if ((port17_value&0x20) != (old_port17_value&0x20)) printf("port17 0x20 changed %02x\n", port17_value&0x20);
	if ((port17_value&0x40) != (old_port17_value&0x40)) printf("port17 0x40 changed %02x\n", port17_value&0x40);
	if ((port17_value&0x80) != (old_port17_value&0x80)) printf("port17 0x80 changed %02x\n", port17_value&0x80);

	// some 3rd party stuff has VDF
	//  printf("ec_port17_out_w data %02x - VDF data %02x\n", data, data & 0x40);
}

WRITE8_MEMBER(ecoinfr_state::ec_port18_out_w)
{
	// Kick Me (Watchdog)
}


static ADDRESS_MAP_START( memmap, AS_PROGRAM, 8, ecoinfr_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_RAM

	AM_RANGE(0xa000, 0xa000) AM_DEVREADWRITE(UPD8251_TAG, i8251_device, data_r, data_w)
	AM_RANGE(0xa001, 0xa001) AM_DEVREADWRITE(UPD8251_TAG, i8251_device, status_r, control_w)

ADDRESS_MAP_END



static ADDRESS_MAP_START( portmap, AS_IO, 8, ecoinfr_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(ec_port00_out_w) AM_READ_PORT("IN0") // Reel 1 Write
	AM_RANGE(0x01, 0x01) AM_WRITE(ec_port01_out_w) AM_READ_PORT("IN1") // Reel 2 Write + Reels Opto Read
	AM_RANGE(0x02, 0x02) AM_WRITE(ec_port02_out_w) AM_READ_PORT("IN2") // Reel 3 Write
	AM_RANGE(0x03, 0x03) AM_WRITE(ec_port03_out_w) AM_READ_PORT("IN3")
	AM_RANGE(0x04, 0x04) AM_WRITE(ec_port04_out_w) AM_READ_PORT("IN4")
	AM_RANGE(0x05, 0x05) AM_WRITE(ec_port05_out_w) AM_READ_PORT("IN5")
	AM_RANGE(0x06, 0x06) AM_WRITE(ec_port06_out_w) AM_READ_PORT("IN6")
	AM_RANGE(0x07, 0x07) AM_WRITE(ec_port07_out_w) AM_READ_PORT("IN7")
	AM_RANGE(0x08, 0x08) AM_WRITE(ec_port08_out_bank_strobe_w)
	AM_RANGE(0x09, 0x09) AM_WRITE(ec_port09_out_reelen_w) // 09 Reel Enables
	AM_RANGE(0x0a, 0x0a) AM_WRITE(ec_port0a_out_w) // 10 (Sound 1)
	AM_RANGE(0x0b, 0x0b) AM_WRITE(ec_port0b_out_w) // 11 (Sound 2)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(ec_port0c_out_cred_strobe_w)
	AM_RANGE(0x0d, 0x0d) AM_WRITE(ec_port0d_out_cred_data_w)
//  AM_RANGE(0x0e, 0x0e) AM_WRITE(ec_port0e_out_w)
	AM_RANGE(0x0f, 0x0f) AM_WRITE(ec_port0f_out_bank_segdata_w)
	AM_RANGE(0x10, 0x10) AM_WRITE(ec_port10_out_w) // 16 (Meter)
	AM_RANGE(0x11, 0x11) AM_WRITE(ec_port11_out_w) // SEC
	AM_RANGE(0x12, 0x12) AM_WRITE(ec_port12_out_w) // SEC
	AM_RANGE(0x13, 0x13) AM_WRITE(ec_port13_out_w)
	AM_RANGE(0x14, 0x14) AM_WRITE(ec_port14_out_w)
	AM_RANGE(0x15, 0x15) AM_WRITE(ec_port15_out_w) // SEC + VDF (3rd party)
	AM_RANGE(0x16, 0x16) AM_WRITE(ec_port16_out_w)
	AM_RANGE(0x17, 0x17) AM_WRITE(ec_port17_out_w) // Hopper + VDF (3rd party)
	AM_RANGE(0x18, 0x18) AM_WRITE(ec_port18_out_w) // 24 (Watchdog)
ADDRESS_MAP_END

CUSTOM_INPUT_MEMBER(ecoinfr_state::ecoinfr_reel1_opto_r)
{
	if (m_optic_pattern & 0x1) return 1;
	return 0;
}

CUSTOM_INPUT_MEMBER(ecoinfr_state::ecoinfr_reel2_opto_r)
{
	if (m_optic_pattern & 0x2) return 1;
	return 0;
}

CUSTOM_INPUT_MEMBER(ecoinfr_state::ecoinfr_reel3_opto_r)
{
	if (m_optic_pattern & 0x4) return 1;
	return 0;
}

static INPUT_PORTS_START( ecoinfr_barx )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN0:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN0:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN0:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN0:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN0:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN0:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN0:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, ecoinfr_state,ecoinfr_reel1_opto_r, NULL)
	PORT_DIPNAME( 0x02, 0x02, "IN1:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, ecoinfr_state,ecoinfr_reel3_opto_r, NULL)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, ecoinfr_state,ecoinfr_reel2_opto_r, NULL)
	PORT_DIPNAME( 0x10, 0x10, "IN1:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN1:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN1:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN1:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN2:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN2:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN2:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN2:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN2:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN2:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN2:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "IN3:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN3:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN3:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN3:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN3:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN3:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN3:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN3:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x01, "IN4:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN4:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN4:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN4:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN4:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN4:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN4:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN4:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x01, "IN5:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN5:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN5:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN5:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN5:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN5:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN5:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN5:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN6")
	PORT_DIPNAME( 0x01, 0x01, "IN6:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN6:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN6:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN6:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN6:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN6:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN6:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN6:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN7")
	PORT_DIPNAME( 0x01, 0x01, "IN7:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN7:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN7:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN7:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN7:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN7:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN7:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN7:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

void ecoinfr_state::machine_reset()
{
//  port00_value = 0x00;
//  port01_value = 0x00;
//  port02_value = 0x00;
//  port03_value = 0x00;
//  port04_value = 0x00;
//  port05_value = 0x00;
//  port06_value = 0x00;
//  port07_value = 0x00;
//  port08_value = 0x00;
	port09_value = 0x00;
//  port0a_value = 0x00;
//  port0b_value = 0x00;
//  port0c_value = 0x00;
//  port0d_value = 0x00;
//  port0e_value = 0x00;
//  port0f_value = 0x00;
	port10_value = 0x00;
	port11_value = 0x00;
	port12_value = 0x00;
	port13_value = 0x00;
	port14_value = 0x00;
	port15_value = 0x00;
	port16_value = 0x00;
	port17_value = 0x00;

	irq_toggle = 0;

	m_banksel = 0xff;
	m_credsel = 0xff;

}


static MACHINE_CONFIG_START( ecoinfr, ecoinfr_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,4000000)
	MCFG_CPU_PROGRAM_MAP(memmap)
	MCFG_CPU_IO_MAP(portmap)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("ectimer", ecoinfr_state, ecoinfr_irq_timer, attotime::from_hz(250))

	MCFG_DEFAULT_LAYOUT(layout_ecoinfr)


	MCFG_DEVICE_ADD(UPD8251_TAG, I8251, 0)

	MCFG_ECOIN_200STEP_ADD("reel0")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(ecoinfr_state, reel0_optic_cb))
	MCFG_ECOIN_200STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(ecoinfr_state, reel1_optic_cb))
	MCFG_ECOIN_200STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(ecoinfr_state, reel2_optic_cb))
	MCFG_ECOIN_200STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(ecoinfr_state, reel3_optic_cb))
MACHINE_CONFIG_END







/********************************************************************************************************************
 ROMs for REGULAR Hw type
********************************************************************************************************************/


#define EC_BARX_OTHERS \
	ROM_REGION( 0x200000, "pal", 0 ) \
	/* Pal dump? check it.. */ \
	ROM_LOAD( "bxpal", 0x0000, 0x000c80, CRC(e30cd1ff) SHA1(4a1ee1703a677143412aa367cfe7d7d346812d87) ) \
	ROM_REGION( 0x200000, "sndz80", 0 ) \
	/* apparently all games using these PCBs had the same sound rom.. */ \
	ROM_LOAD( "barxsnd.bin", 0x0000, 0x001000, CRC(7d37fda1) SHA1(fb906615067887d9daecdbc741cfa4ac710c4627) )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		EC_BARX_OTHERS \
	ROM_END \
	GAME(year, setname, parent ,ecoinfr ,ecoinfr_barx , ecoinfr_state,ecoinfr ,ROT0,company,title,GAME_FLAGS )

/* 32Kb With Header / Space for Header */

// Some Roms below use a header at the start, containing 8x8 Byte strings to describe what type of set it is
// address | string    description                                        | legend for comments
// 20 - 27 | Protoco | Protocol? (Data recording etc.)                    | P--- ----
// 28 - 2f | NoteAc1 | Note Acceptor Type 1? (just NoteAcc on some sets)  | -1-- ----  (or -N-- ----)
// 30 - 37 | NoteAc2 | Note Acceptor Type 2?                              | --2- ----
// 38 - 3f | SecMete | Secondary Meters?                                  | ---S ----
// 40 - 47 | Keys    | Uses JP Keys?                                      | ---- K---
// 48 - 4f | 10pHopp | Different Hopper Type?                             | ---- -H--
// 50 - 57 | ?????   | not seen used                                      | ---- --?-
// 58 - 5f | GALA    | ? (in some of the earlier sets this is a 0 fill)   | ---- ---G  (or ---- ---0)
GAME_CUSTOM( 199?, ec_barx,     0,          "iss354.rom",   0x0000, 0x008000, CRC(0da15b8e) SHA1(435451f7c428beaacf182d112214482503dec483), "Electrocoin","Bar X (Electrocoin) (set 1)" ) // ELCNSBRX - Wed Feb 07 17:28:02 1996
GAME_CUSTOM( 199?, ec_barx__a,  ec_barx,    "iss9007.rom",  0x0000, 0x008000, CRC(c73b7c4e) SHA1(2d1fecb8efd4b80d1249034efc5ea9c1d3cb660b), "Electrocoin","Bar X (Electrocoin) (set 2)" ) // ELCNCBX1 - Thu May 01 16:23:14 1997
GAME_CUSTOM( 199?, ec_barx__b,  ec_barx,    "iss9011.rom",  0x0000, 0x008000, CRC(7b69ff3c) SHA1(f13e71fa2ae997fd2c80ca060cdbe2115468df6b), "Electrocoin","Bar X (Electrocoin) (set 3)" ) // ELCNCBX1 - Sun Sep 21 10:28:56 1997
GAME_CUSTOM( 199?, ec_barx__c,  ec_barx,    "iss9015.rom",  0x0000, 0x008000, CRC(fd2fabe8) SHA1(2a0261c39187746a53ff7c32a759ba1311ec56a9), "Electrocoin","Bar X (Electrocoin) (set 4)" ) // ELCNCBX1 - Tue May 19 14:26:38 1998
GAME_CUSTOM( 199?, ec_barx__d,  ec_barx,    "iss9201.rom",  0x0000, 0x008000, CRC(35cf9280) SHA1(d271a89178b026c2847b8f192c72f8ce841a1548), "Electrocoin","Bar X (Electrocoin) (set 5)" ) // ELCNCBX1 - Sun Jun 21 11:07:37 1998
GAME_CUSTOM( 199?, ec_barx__e,  ec_barx,    "iss9204.rom",  0x0000, 0x008000, CRC(e8ced9c9) SHA1(a028c2bf35add11c2ff3b98cf34925acb99ef1c4), "Electrocoin","Bar X (Electrocoin) (set 6)" ) // ELCPCBX1 - Thu Sep 03 12:41:08 1998
// No Header info, or title info (although there is space for one)
GAME_CUSTOM( 199?, ec_barx__f,  ec_barx,    "barx5ft",      0x0000, 0x008000, CRC(6a549ff3) SHA1(02766642c5aee5fa3f1e0d9d7a0ec30192e597f1), "Electrocoin","Bar X (Electrocoin) (set 7)" ) // ELCNCBX1 - Thu Sep 03 12:40:21 1998
GAME_CUSTOM( 199?, ec_barx__g,  ec_barx,    "bx503cas",     0x0000, 0x008000, CRC(ac974ac2) SHA1(d317730506c075b108c68b3fc5628837b12863fe), "Electrocoin","Bar X (Electrocoin) (set 8)" ) // ELCNCBX1 - Thu Sep 03 12:40:21 1998
// Sets below all marked '2001 BARX' (older header type?)
GAME_CUSTOM( 199?, ec_barx__h,  ec_barx,    "issa091",      0x0000, 0x008000, CRC(6748c76c) SHA1(115b6f30971fbbbd67ece3eeba66431c7440267e), "Electrocoin","Bar X (Electrocoin) (set 9)" ) // ELCNSBRX - Fri Feb 21 20:39:04 2003 // ---- ---0
GAME_CUSTOM( 199?, ec_barx__i,  ec_barx,    "issa092",      0x0000, 0x008000, CRC(82c4e44d) SHA1(a253779c3666eb1c3b4a45f22478b2310a23540a), "Electrocoin","Bar X (Electrocoin) (set 10)" )// ELCNSBRX - Fri Feb 21 20:39:29 2003 // -N-- ---0
GAME_CUSTOM( 199?, ec_barx__j,  ec_barx,    "issa096",      0x0000, 0x008000, CRC(8536c23d) SHA1(3acca3016dd5a8a183f646095856fffca9d0fd9c), "Electrocoin","Bar X (Electrocoin) (set 11)" )// ELCPSBRX - Fri Feb 21 20:42:18 2003 // P--- ---0
GAME_CUSTOM( 199?, ec_barx__k,  ec_barx,    "issa097",      0x0000, 0x008000, CRC(0650275f) SHA1(eb06a7b245103aeb53973897128063b04e599fde), "Electrocoin","Bar X (Electrocoin) (set 12)" )// ELCPSBRX - Fri Feb 21 20:46:35 2003 // PN-- ---0
// 2001 BARX (newer header type?)
// Are these actually 'Super Bar X'? They have SBARX strings in them near build dates etc.
GAME_CUSTOM( 199?, ec_barx__l,  ec_barx,    "issa793",      0x0000, 0x008000, CRC(e3de7b43) SHA1(5d33d39f59e30510ac89d9a03979f17a4a3707eb), "Electrocoin","Bar X (Electrocoin) (set 13)" ) // ---- ----
GAME_CUSTOM( 199?, ec_barx__m,  ec_barx,    "issa794",      0x0000, 0x008000, CRC(47334130) SHA1(08204545d20fa017321183126a856446b08e09b9), "Electrocoin","Bar X (Electrocoin) (set 14)" ) // -1-- ----
GAME_CUSTOM( 199?, ec_barx__n,  ec_barx,    "issa795",      0x0000, 0x008000, CRC(d24936fd) SHA1(f0efa2d30c71285d31ae2c47ce2baef3bb72bc66), "Electrocoin","Bar X (Electrocoin) (set 15)" ) // --2S K---
GAME_CUSTOM( 199?, ec_barx__o,  ec_barx,    "issa796",      0x0000, 0x008000, CRC(0d5c020d) SHA1(f41e015773c908228f55f1ce3e35b22ad4b6bf33), "Electrocoin","Bar X (Electrocoin) (set 16)" ) // -1-S K---
GAME_CUSTOM( 199?, ec_barx__p,  ec_barx,    "issa797",      0x0000, 0x008000, CRC(57cf216a) SHA1(070297c07404f92928581d73751e82158e9567d7), "Electrocoin","Bar X (Electrocoin) (set 17)" ) // ---S K---
GAME_CUSTOM( 199?, ec_barx__q,  ec_barx,    "issa798",      0x0000, 0x008000, CRC(c15f25e2) SHA1(b7a32876a7f8512451d911f0611cbdc8a083a79e), "Electrocoin","Bar X (Electrocoin) (set 18)" ) // -1-- ----
GAME_CUSTOM( 199?, ec_barx__r,  ec_barx,    "issa799",      0x0000, 0x008000, CRC(9682ca8c) SHA1(a6846bff4aaa9ccf997f7049300b62138a405e20), "Electrocoin","Bar X (Electrocoin) (set 19)" ) // --2S K---
GAME_CUSTOM( 199?, ec_barx__s,  ec_barx,    "issa800",      0x0000, 0x008000, CRC(55ab4892) SHA1(7b71d6c70f6f2083b2cce93198a74034502f61fa), "Electrocoin","Bar X (Electrocoin) (set 20)" ) // -1-S K---
GAME_CUSTOM( 199?, ec_barx__t,  ec_barx,    "issa801",      0x0000, 0x008000, CRC(4823d2ec) SHA1(df9cbea4c96411fb5d7707627ea2fc3aca0681cf), "Electrocoin","Bar X (Electrocoin) (set 21)" ) // ---- -H--
GAME_CUSTOM( 199?, ec_barx__u,  ec_barx,    "issa802",      0x0000, 0x008000, CRC(5408417a) SHA1(1a45271ae593bb071a4fa0053cae8b10bd1ba49a), "Electrocoin","Bar X (Electrocoin) (set 22)" ) // ---S KH--
GAME_CUSTOM( 199?, ec_barx__v,  ec_barx,    "issa803",      0x0000, 0x008000, CRC(748981a0) SHA1(bbad9f0ea44883e458710b15e2652b0e76dc873d), "Electrocoin","Bar X (Electrocoin) (set 23)" ) // P--- ----
GAME_CUSTOM( 199?, ec_barx__w,  ec_barx,    "issa804",      0x0000, 0x008000, CRC(a6730955) SHA1(7ebf9967b9e40ca89da8951a1711d592ef87160d), "Electrocoin","Bar X (Electrocoin) (set 24)" ) // P1-- ----
GAME_CUSTOM( 199?, ec_barx__x,  ec_barx,    "issa805",      0x0000, 0x008000, CRC(8c1cf7f4) SHA1(0ff139c38d68a66b40c8ac611bf05cb3a9d852fa), "Electrocoin","Bar X (Electrocoin) (set 25)" ) // P-2S K---
GAME_CUSTOM( 199?, ec_barx__y,  ec_barx,    "issa806",      0x0000, 0x008000, CRC(a1aee26b) SHA1(966e595029b5518ddee422afae6d633da0e8e4e4), "Electrocoin","Bar X (Electrocoin) (set 26)" ) // P1-S K---
GAME_CUSTOM( 199?, ec_barx__z,  ec_barx,    "issa807",      0x0000, 0x008000, CRC(b9332da4) SHA1(622a94a1c5226cf42263b0642e695e1af71c611c), "Electrocoin","Bar X (Electrocoin) (set 27)" ) // P--S K---
GAME_CUSTOM( 199?, ec_barx__0,  ec_barx,    "issa808",      0x0000, 0x008000, CRC(3b8fda84) SHA1(74cfaef125900d89b8c936a7cb3668fd7642fbfe), "Electrocoin","Bar X (Electrocoin) (set 28)" ) // P1-- ----
GAME_CUSTOM( 199?, ec_barx__1,  ec_barx,    "issa809",      0x0000, 0x008000, CRC(58c10603) SHA1(653c0afb57feda9d4a02f6590aacb9cf63b931c9), "Electrocoin","Bar X (Electrocoin) (set 29)" ) // P-2S K---
GAME_CUSTOM( 199?, ec_barx__2,  ec_barx,    "issa810",      0x0000, 0x008000, CRC(aafff06c) SHA1(0ae798d965299b2b9f10d3707877ede722c0eb7a), "Electrocoin","Bar X (Electrocoin) (set 30)" ) // P1-S K---
GAME_CUSTOM( 199?, ec_barx__3,  ec_barx,    "issa811",      0x0000, 0x008000, CRC(ac2ceda1) SHA1(3299f07db8670bffbcfbbdfc1fd44179f5a5ccf6), "Electrocoin","Bar X (Electrocoin) (set 31)" ) // P--- -H--
GAME_CUSTOM( 199?, ec_barx__4,  ec_barx,    "issa812",      0x0000, 0x008000, CRC(8a1e9002) SHA1(3c82e3761007feaa61a2c029951c6e3336224a1c), "Electrocoin","Bar X (Electrocoin) (set 32)" ) // P--S KH--
GAME_CUSTOM( 199?, ec_barx__5,  ec_barx,    "issa813",      0x0000, 0x008000, CRC(0ea31930) SHA1(16d38501dba2079e4d573beca5f1216820bac1bc), "Electrocoin","Bar X (Electrocoin) (set 33)" ) // --2- ----
GAME_CUSTOM( 199?, ec_barx__6,  ec_barx,    "issa814",      0x0000, 0x008000, CRC(50e4f6ff) SHA1(84758c19e36b03af2f6f2645ebb685795d667f9f), "Electrocoin","Bar X (Electrocoin) (set 34)" ) // --2- ----
GAME_CUSTOM( 199?, ec_barx__7,  ec_barx,    "issa815",      0x0000, 0x008000, CRC(a3b72d9e) SHA1(f08fe4372392ff72301dafca972953e779a546c4), "Electrocoin","Bar X (Electrocoin) (set 35)" ) // --2S K---
GAME_CUSTOM( 199?, ec_barx__8,  ec_barx,    "issa816",      0x0000, 0x008000, CRC(7b79e1dd) SHA1(ec2fc0a60bd90addbd79a1620e97f290907dbd5c), "Electrocoin","Bar X (Electrocoin) (set 36)" ) // --2S K---
GAME_CUSTOM( 199?, ec_barx__9,  ec_barx,    "issa817",      0x0000, 0x008000, CRC(05006125) SHA1(6b71c68579f8ec9b3bb0ba208df69c2125ebb9e7), "Electrocoin","Bar X (Electrocoin) (set 37)" ) // P-2- ----
GAME_CUSTOM( 199?, ec_barx__aa, ec_barx,    "issa818",      0x0000, 0x008000, CRC(3aee13d9) SHA1(3645a83c6c9f40b5ed356ce45129fe860aba907d), "Electrocoin","Bar X (Electrocoin) (set 38)" ) // P-2- ----
GAME_CUSTOM( 199?, ec_barx__ab, ec_barx,    "issa819",      0x0000, 0x008000, CRC(76f10c3a) SHA1(bcdb9c82e9b14c2e351bf6caeb44173c2376e48e), "Electrocoin","Bar X (Electrocoin) (set 39)" ) // P-2S K---
GAME_CUSTOM( 199?, ec_barx__ac, ec_barx,    "issa820",      0x0000, 0x008000, CRC(9c04f02a) SHA1(c0bf63fe00679025a56d867b216f84ec4536d06c), "Electrocoin","Bar X (Electrocoin) (set 40)" ) // P-2S K---
GAME_CUSTOM( 199?, ec_barx__ad, ec_barx,    "issa821",      0x0000, 0x008000, CRC(cb72cc59) SHA1(c1c12a921a9b57a252ad00eaadbba35073b9b64d), "Electrocoin","Bar X (Electrocoin) (set 41)" ) // -1-- ----
GAME_CUSTOM( 199?, ec_barx__ae, ec_barx,    "issa822",      0x0000, 0x008000, CRC(97e04639) SHA1(fc769882bb9a96de0d1121c7ceae60960b654915), "Electrocoin","Bar X (Electrocoin) (set 42)" ) // -1-- ----
GAME_CUSTOM( 199?, ec_barx__af, ec_barx,    "issa823",      0x0000, 0x008000, CRC(a306982b) SHA1(e0a442145728c563ed9020346db32e89a3dac985), "Electrocoin","Bar X (Electrocoin) (set 43)" ) // -1-S K---
GAME_CUSTOM( 199?, ec_barx__ag, ec_barx,    "issa824",      0x0000, 0x008000, CRC(14b24861) SHA1(f90850d0bb38ade91dcdd7aaa29c916341d3f65f), "Electrocoin","Bar X (Electrocoin) (set 44)" ) // -1-S K---
GAME_CUSTOM( 199?, ec_barx__ah, ec_barx,    "issa825",      0x0000, 0x008000, CRC(5543a633) SHA1(3cfcea2c123b90704e69e5ce9f06920022911802), "Electrocoin","Bar X (Electrocoin) (set 45)" ) // P1-- ----
GAME_CUSTOM( 199?, ec_barx__ai, ec_barx,    "issa826",      0x0000, 0x008000, CRC(809f651f) SHA1(86c2f813dba787b2774b49ed272f825725ec3712), "Electrocoin","Bar X (Electrocoin) (set 46)" ) // P1-- ----
GAME_CUSTOM( 199?, ec_barx__aj, ec_barx,    "issa827",      0x0000, 0x008000, CRC(90714254) SHA1(b9610d220ecfedf26c3c4942f0dbb569841cdf56), "Electrocoin","Bar X (Electrocoin) (set 47)" ) // P1-S K---
GAME_CUSTOM( 199?, ec_barx__ak, ec_barx,    "issa828",      0x0000, 0x008000, CRC(f8695abf) SHA1(2837d6a6b69dd27070cbf1309b51f02b0df98a94), "Electrocoin","Bar X (Electrocoin) (set 48)" ) // P1-S K---
GAME_CUSTOM( 199?, ec_barx__al, ec_barx,    "issa829",      0x0000, 0x008000, CRC(cab2e171) SHA1(e6f9e91350dd41ec3c12fc221a59529277f47b2b), "Electrocoin","Bar X (Electrocoin) (set 49)" ) // P1-- ---G
GAME_CUSTOM( 199?, ec_barx__am, ec_barx,    "issa830",      0x0000, 0x008000, CRC(9c0984cf) SHA1(bc80f0e31c726bd03aaeaa3cd9b0f99a8fecf79b), "Electrocoin","Bar X (Electrocoin) (set 50)" ) // P-2S K--G
GAME_CUSTOM( 199?, ec_barx__an, ec_barx,    "issa831",      0x0000, 0x008000, CRC(4baceee5) SHA1(23f5acba763d7ba49f017c0cf1a4a11f21febe63), "Electrocoin","Bar X (Electrocoin) (set 51)" ) // P1-S K--G
GAME_CUSTOM( 199?, ec_barx__ao, ec_barx,    "issa832",      0x0000, 0x008000, CRC(2148f157) SHA1(6b948797b5032e4b4968af55f71e03bbf78f7434), "Electrocoin","Bar X (Electrocoin) (set 52)" ) // P1-- ---G
GAME_CUSTOM( 199?, ec_barx__ap, ec_barx,    "issa833",      0x0000, 0x008000, CRC(50d050f3) SHA1(59ad7193aef694be6b8905a233828f292ebd5d5b), "Electrocoin","Bar X (Electrocoin) (set 53)" ) // P-2S K--G
GAME_CUSTOM( 199?, ec_barx__aq, ec_barx,    "issa834",      0x0000, 0x008000, CRC(819c1c27) SHA1(863830eed8dc3e7e92321c163d26ae3a9b97a649), "Electrocoin","Bar X (Electrocoin) (set 54)" ) // P1-S K--G
GAME_CUSTOM( 199?, ec_barx__ar, ec_barx,    "issa835",      0x0000, 0x008000, CRC(a8674b53) SHA1(5808991783779a9aca730d8a1fde70552f2c9bf5), "Electrocoin","Bar X (Electrocoin) (set 55)" ) // P1-- ---G
GAME_CUSTOM( 199?, ec_barx__as, ec_barx,    "issa836",      0x0000, 0x008000, CRC(47df4193) SHA1(3a4e05e1fcc0cf6471fa40751e8f80548ebc09cc), "Electrocoin","Bar X (Electrocoin) (set 56)" ) // P-2S K--G
GAME_CUSTOM( 199?, ec_barx__at, ec_barx,    "issa837",      0x0000, 0x008000, CRC(cce0c4a9) SHA1(a7e30dd7de82bf36e8b442eded6b07a9df24c7a6), "Electrocoin","Bar X (Electrocoin) (set 57)" ) // P1-S K--G
GAME_CUSTOM( 199?, ec_barx__au, ec_barx,    "issa838",      0x0000, 0x008000, CRC(a9fdedb4) SHA1(8d987939a7779e896e4af560b2a39ca9b1fb3ac7), "Electrocoin","Bar X (Electrocoin) (set 58)" ) // P1-- ---G
GAME_CUSTOM( 199?, ec_barx__av, ec_barx,    "issa839",      0x0000, 0x008000, CRC(d58f6e4f) SHA1(cb91aa8db2b4730b25e7e5da7d03d9637fbec59c), "Electrocoin","Bar X (Electrocoin) (set 59)" ) // P-2S K--G
GAME_CUSTOM( 199?, ec_barx__aw, ec_barx,    "issa840",      0x0000, 0x008000, CRC(1025caf3) SHA1(859081242091976c222729199eb3fec6f6c45621), "Electrocoin","Bar X (Electrocoin) (set 60)" ) // P1-S K--G
GAME_CUSTOM( 199?, ec_barx__ax, ec_barx,    "issa841",      0x0000, 0x008000, CRC(b1685ed8) SHA1(42995a5219ec697b5e760c25b9bddace41ebded8), "Electrocoin","Bar X (Electrocoin) (set 61)" ) // P1-- ---G
GAME_CUSTOM( 199?, ec_barx__ay, ec_barx,    "issa842",      0x0000, 0x008000, CRC(20dca8c1) SHA1(46b76df179fc306cfd0054f723fc9763f3b46a84), "Electrocoin","Bar X (Electrocoin) (set 62)" ) // P-2S K--G
GAME_CUSTOM( 199?, ec_barx__az, ec_barx,    "issa843",      0x0000, 0x008000, CRC(15b6f976) SHA1(fef5db76d61fda4e62e50fd891e4981cc0323a22), "Electrocoin","Bar X (Electrocoin) (set 63)" ) // P1-S K--G
GAME_CUSTOM( 199?, ec_barx__a0, ec_barx,    "issa844",      0x0000, 0x008000, CRC(6e07e53b) SHA1(b2bd1613fbaf0e0f3b009347c30073f2fec91784), "Electrocoin","Bar X (Electrocoin) (set 64)" ) // P1-- ---G
GAME_CUSTOM( 199?, ec_barx__a1, ec_barx,    "issa845",      0x0000, 0x008000, CRC(422f6ccb) SHA1(fe5eaaa98c30a6d4ec72d5f9e276afe7359a1db7), "Electrocoin","Bar X (Electrocoin) (set 65)" ) // P-2S K--G
GAME_CUSTOM( 199?, ec_barx__a2, ec_barx,    "issa846",      0x0000, 0x008000, CRC(c8938b90) SHA1(819ac3de9a0ca19469f60d26e363c292faa10abf), "Electrocoin","Bar X (Electrocoin) (set 66)" ) // P1-S K--G
GAME_CUSTOM( 199?, ec_barx__a3, ec_barx,    "issa847",      0x0000, 0x008000, CRC(dc56de4b) SHA1(a4cce8bba89ae1d803b7fe050dc2e9bde1383f7c), "Electrocoin","Bar X (Electrocoin) (set 67)" ) // P1-- ---G
GAME_CUSTOM( 199?, ec_barx__a4, ec_barx,    "issa848",      0x0000, 0x008000, CRC(8410fe03) SHA1(6ee50e699b67ac73cb38ab8aa9d3f6efb6865918), "Electrocoin","Bar X (Electrocoin) (set 68)" ) // P-2S K--G
GAME_CUSTOM( 199?, ec_barx__a5, ec_barx,    "issa849",      0x0000, 0x008000, CRC(3cf53845) SHA1(e6e9dc3a8757e95647db2f64912ea5ad88cfcd60), "Electrocoin","Bar X (Electrocoin) (set 69)" ) // P1-S K--G
GAME_CUSTOM( 199?, ec_barx__a6, ec_barx,    "issa850",      0x0000, 0x008000, CRC(ed830402) SHA1(8fa389e9f04c446864784736c4bc08006cb37304), "Electrocoin","Bar X (Electrocoin) (set 70)" ) // P1-- ---G
GAME_CUSTOM( 199?, ec_barx__a7, ec_barx,    "issa851",      0x0000, 0x008000, CRC(cf79dc09) SHA1(7f4bf280431a800ae742507cb944c2c01bc54d15), "Electrocoin","Bar X (Electrocoin) (set 71)" ) // P-2S K--G
GAME_CUSTOM( 199?, ec_barx__a8, ec_barx,    "issa852",      0x0000, 0x008000, CRC(3b4a2615) SHA1(b466e15d2dfce81f2a89ab9a5b41b32158f109f1), "Electrocoin","Bar X (Electrocoin) (set 72)" ) // P1-S K--G
GAME_CUSTOM( 199?, ec_barx__a9, ec_barx,    "issa853",      0x0000, 0x008000, CRC(5d0c39c2) SHA1(debe88d7f8d35ba621388d5a21a6e5358faafa06), "Electrocoin","Bar X (Electrocoin) (set 73)" ) // P1-- ---G
GAME_CUSTOM( 199?, ec_barx__ba, ec_barx,    "issa854",      0x0000, 0x008000, CRC(ce227e95) SHA1(83cee7b83e66cea40a5b7f6025e010f45309c64b), "Electrocoin","Bar X (Electrocoin) (set 74)" ) // P-2S K--G
GAME_CUSTOM( 199?, ec_barx__bb, ec_barx,    "issa855",      0x0000, 0x008000, CRC(0dd7873b) SHA1(bc64924cbfc16289c6e7365c0b3276d9a940a917), "Electrocoin","Bar X (Electrocoin) (set 75)" ) // P1-S K--G
GAME_CUSTOM( 199?, ec_barx__bc, ec_barx,    "issa856",      0x0000, 0x008000, CRC(0477e51f) SHA1(53c7a5fab006b8545f1aeed562920a099cbdb73e), "Electrocoin","Bar X (Electrocoin) (set 76)" ) // P1-- ---G
GAME_CUSTOM( 199?, ec_barx__bd, ec_barx,    "issa857",      0x0000, 0x008000, CRC(a1d646ef) SHA1(4cdb39d4623d514b0cec673aa5523f128797b152), "Electrocoin","Bar X (Electrocoin) (set 77)" ) // P-2S K--G
GAME_CUSTOM( 199?, ec_barx__be, ec_barx,    "issa858",      0x0000, 0x008000, CRC(228533f5) SHA1(e89d5078e319d48b7d313b4f54c1d18d0b29598b), "Electrocoin","Bar X (Electrocoin) (set 78)" ) // P1-S K--G
GAME_CUSTOM( 199?, ec_barx__bf, ec_barx,    "issa859",      0x0000, 0x008000, CRC(f782eab9) SHA1(791a07d3cb2c77c9a22eb4d9cbf949049bab9bf7), "Electrocoin","Bar X (Electrocoin) (set 79)" ) // --2S K---
GAME_CUSTOM( 199?, ec_barx__bg, ec_barx,    "issa860",      0x0000, 0x008000, CRC(6d4ff59a) SHA1(fc404e037bd63adc8de4b6cc857958007406dc8c), "Electrocoin","Bar X (Electrocoin) (set 80)" ) // --2S K---
GAME_CUSTOM( 199?, ec_barx__bh, ec_barx,    "issa861",      0x0000, 0x008000, CRC(1b8fd981) SHA1(70221f793c092534bc8c0825aa759aa548d01c98), "Electrocoin","Bar X (Electrocoin) (set 81)" ) // --2S K---
GAME_CUSTOM( 199?, ec_barx__bi, ec_barx,    "issa862",      0x0000, 0x008000, CRC(ceface32) SHA1(4a869b83ffd5c59a7cc64b1ee088fc788f57ff0f), "Electrocoin","Bar X (Electrocoin) (set 82)" ) // --2S K---
GAME_CUSTOM( 199?, ec_barx__bj, ec_barx,    "issa863",      0x0000, 0x008000, CRC(1a7fa7d2) SHA1(06855a05102ff757a397b2c438b8c60cb66477c8), "Electrocoin","Bar X (Electrocoin) (set 83)" ) // P1-- ---G
GAME_CUSTOM( 199?, ec_barx__bk, ec_barx,    "issa864",      0x0000, 0x008000, CRC(08869eee) SHA1(6df205a743d64799635075170eb752ece35ff9af), "Electrocoin","Bar X (Electrocoin) (set 84)" ) // P-2S K--G
GAME_CUSTOM( 199?, ec_barx__bl, ec_barx,    "issa865",      0x0000, 0x008000, CRC(5b487e21) SHA1(aa9dbc1491e0a82f7634ddf73f9df3cffc85a1d9), "Electrocoin","Bar X (Electrocoin) (set 85)" ) // P1-S K--G
GAME_CUSTOM( 199?, ec_barx__bm, ec_barx,    "issa866",      0x0000, 0x008000, CRC(d23d2999) SHA1(7616d5ab8d9b371a625b9fb667b6807333f89c6e), "Electrocoin","Bar X (Electrocoin) (set 86)" ) // P1-- ---G
GAME_CUSTOM( 199?, ec_barx__bn, ec_barx,    "issa867",      0x0000, 0x008000, CRC(1e8b47ff) SHA1(2328e0b39ad1d0bc40e2f403d59cf4c5793dd1fa), "Electrocoin","Bar X (Electrocoin) (set 87)" ) // P-2S K--G
GAME_CUSTOM( 199?, ec_barx__bo, ec_barx,    "issa868",      0x0000, 0x008000, CRC(e2616b21) SHA1(32b0dcadd5a1fbde41426fdf03a1a7515384c685), "Electrocoin","Bar X (Electrocoin) (set 88)" ) // P1-S K--G
GAME_CUSTOM( 199?, ec_barx__bp, ec_barx,    "issa869",      0x0000, 0x008000, CRC(1bce989f) SHA1(704f165ca34e4df3e0699ebc7867294df860edb3), "Electrocoin","Bar X (Electrocoin) (set 89)" ) // P1-- ---G
GAME_CUSTOM( 199?, ec_barx__bq, ec_barx,    "issa870",      0x0000, 0x008000, CRC(592f94f7) SHA1(f845c5324a1c49e1ca4dc0538b422de30f669d95), "Electrocoin","Bar X (Electrocoin) (set 90)" ) // P-2S K--G
GAME_CUSTOM( 199?, ec_barx__br, ec_barx,    "issa871",      0x0000, 0x008000, CRC(aed724b8) SHA1(9ccb4c1a10e86610ac6c241df31a0d2513985127), "Electrocoin","Bar X (Electrocoin) (set 91)" ) // P1-S K--G
GAME_CUSTOM( 199?, ec_barx__bs, ec_barx,    "issa872",      0x0000, 0x008000, CRC(957536b1) SHA1(4ca031ba9b3bd5e178abe7951498f8202fa4cd48), "Electrocoin","Bar X (Electrocoin) (set 92)" ) // P1-- ---G
GAME_CUSTOM( 199?, ec_barx__bt, ec_barx,    "issa873",      0x0000, 0x008000, CRC(171cdb19) SHA1(f0f7cb81b220d757c5dadbe9e2cc0dbc6aa02962), "Electrocoin","Bar X (Electrocoin) (set 93)" ) // P-2S K--G
GAME_CUSTOM( 199?, ec_barx__bu, ec_barx,    "issa874",      0x0000, 0x008000, CRC(704f999c) SHA1(259430d175c22a33f222ab1138159b8fc838c98f), "Electrocoin","Bar X (Electrocoin) (set 94)" ) // P1-S K--G


/* 1993 Electrocoin Copyright - z180 code - these might belong in the pyramid driver, oxo driver, or their own driver */
GAME_CUSTOM( 199?, ec_bx180,    0,          "sbarx6c.bin",  0x0000, 0x008000, CRC(f747fa74) SHA1(7820e9225924c8b2fd78c625cc61871f7c76357f), "Electrocoin","Bar X (Z180 hardware) (Electrocoin) (set 1)" ) // ELCNBARX - no build date?
GAME_CUSTOM( 199?, ec_bx180a,   ec_bx180,   "bxc1&6c.rom",  0x0000, 0x008000, CRC(356964c3) SHA1(68522a0d379ab49f5975e0628f3e813cfe3287a3), "Electrocoin","Bar X (Z180 hardware) (Electrocoin) (set 2)" ) // ELCNBARX - no date string



#define EC_BIG7_OTHERS \
	ROM_REGION( 0x200000, "sndz80", 0 ) \
	ROM_LOAD( "big7snd", 0x0000, 0x002000, CRC(b530d91f) SHA1(f4e70e05d11e92a82f4bf8d78859b2a94fa5f22b) )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		EC_BIG7_OTHERS \
	ROM_END \
	GAME(year, setname, parent ,ecoinfr ,ecoinfr_barx , ecoinfr_state,ecoinfr ,ROT0,company,title,GAME_FLAGS )
// This is almost certainly a mix of 'Big7' and 'Super Big7' ROMs
/* All have 'BIG7' and type info in header */
GAME_CUSTOM( 199?, ec_big7,     0,          "big7.bin",                 0x0000, 0x008000, CRC(12a08de2) SHA1(cce3526d3b47567d240739111ed4b7e2ba994de6), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 1)" )
GAME_CUSTOM( 199?, ec_big7__a,  ec_big7,    "iss3025.rom",              0x0000, 0x008000, CRC(26c9382a) SHA1(8c4fe06a8e5171e6f2c91b0aee14484aca386a9c), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 2)" )
GAME_CUSTOM( 199?, ec_big7__b,  ec_big7,    "iss3027.rom",              0x0000, 0x008000, CRC(7dc5ccbe) SHA1(2e904f6dced08ed38c4e5c0adfa6904b80a0a0fa), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 3)" )
GAME_CUSTOM( 199?, ec_big7__c,  ec_big7,    "iss3033.rom",              0x0000, 0x008000, CRC(52e6c6b7) SHA1(9ff5c6cca014735f8cffffb56a85657b0941e9f8), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 4)" )
GAME_CUSTOM( 199?, ec_big7__d,  ec_big7,    "iss3034.rom",              0x0000, 0x008000, CRC(7f27bf12) SHA1(1fb7ca712cb801f67da6a9b50eddc3992972534e), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 5)" )
GAME_CUSTOM( 199?, ec_big7__e,  ec_big7,    "iss3035.rom",              0x0000, 0x008000, CRC(8612b896) SHA1(31fb781a4dd2f82e77dc87d37be378974983ade4), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 6)" )
GAME_CUSTOM( 199?, ec_big7__f,  ec_big7,    "iss3049.rom",              0x0000, 0x008000, CRC(b820d03e) SHA1(80e0208a31468ace7d75ce10f88c2267c0eb92b4), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 7)" )
GAME_CUSTOM( 199?, ec_big7__g,  ec_big7,    "iss3050.rom",              0x0000, 0x008000, CRC(cff49d4c) SHA1(3a6c58f942cbd716218468a8061d1f3f7be6ea13), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 8)" )
GAME_CUSTOM( 199?, ec_big7__h,  ec_big7,    "iss3051.rom",              0x0000, 0x008000, CRC(3b5b37d1) SHA1(56070c1f7d00b7b3984590d4824da88850ff6a9f), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 9)" )
GAME_CUSTOM( 199?, ec_big7__i,  ec_big7,    "iss3052.rom",              0x0000, 0x008000, CRC(4f3512bb) SHA1(f9b3dd180143fc40f7b737aed23b78920ac5d267), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 10)" )
GAME_CUSTOM( 199?, ec_big7__j,  ec_big7,    "iss3053.rom",              0x0000, 0x008000, CRC(99ba426a) SHA1(b0545b3ae649d89a14da61e56ac3899896a37e82), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 11)" )
GAME_CUSTOM( 199?, ec_big7__k,  ec_big7,    "iss3054.rom",              0x0000, 0x008000, CRC(9598d331) SHA1(194339222b97ff8d97aa1d49e5fecc666a67ea49), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 12)" )
GAME_CUSTOM( 199?, ec_big7__l,  ec_big7,    "iss3055.rom",              0x0000, 0x008000, CRC(3c4eb15c) SHA1(3bb7bdf206fc0fc4310df86733b459e1558aea4f), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 13)" )
GAME_CUSTOM( 199?, ec_big7__m,  ec_big7,    "iss3056.rom",              0x0000, 0x008000, CRC(202a820e) SHA1(a9b2c9f7995b4e1b0d4e8009a026174f0352d15f), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 14)" )
GAME_CUSTOM( 199?, ec_big7__n,  ec_big7,    "iss3057.rom",              0x0000, 0x008000, CRC(db7b5c05) SHA1(c4ef81636766154a7b65be42d7689d32a0a922e7), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 15)" )
GAME_CUSTOM( 199?, ec_big7__o,  ec_big7,    "iss3058.rom",              0x0000, 0x008000, CRC(a772f630) SHA1(8e60a08bfe884ef51893c51f11e9a4d2024f6e2f), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 16)" )
GAME_CUSTOM( 199?, ec_big7__p,  ec_big7,    "iss3059.rom",              0x0000, 0x008000, CRC(3b217d60) SHA1(7b39df64ce1cff64e737fe9c78e6de3cb3546336), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 17)" )
GAME_CUSTOM( 199?, ec_big7__q,  ec_big7,    "iss3060.rom",              0x0000, 0x008000, CRC(29a1f750) SHA1(33a0de2f240228842c93e39939c28a5d6bba669e), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 18)" )
GAME_CUSTOM( 199?, ec_big7__r,  ec_big7,    "iss3061.rom",              0x0000, 0x008000, CRC(f1a7da0b) SHA1(0ffed598ba8a5dfb83c8b05a80f3499cb00686ec), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 19)" )
GAME_CUSTOM( 199?, ec_big7__s,  ec_big7,    "iss3062.rom",              0x0000, 0x008000, CRC(bff8c7e7) SHA1(db23240eafea82e540a410b135f32c64260fba2e), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 20)" )
GAME_CUSTOM( 199?, ec_big7__t,  ec_big7,    "iss3063.rom",              0x0000, 0x008000, CRC(c3471a8c) SHA1(edde9a96ec380f95ad2fc473f78fc5d34fd1769d), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 21)" )
GAME_CUSTOM( 199?, ec_big7__u,  ec_big7,    "iss3064.rom",              0x0000, 0x008000, CRC(a635c5bc) SHA1(476e1fffbddefa230b96b0e1d3bb50f9ef08b24a), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 22)" )
GAME_CUSTOM( 199?, ec_big7__v,  ec_big7,    "iss3065.rom",              0x0000, 0x008000, CRC(bd2315f8) SHA1(365f87e6ef68f330d47e7d614f02b3775758ac4c), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 23)" )
GAME_CUSTOM( 199?, ec_big7__w,  ec_big7,    "iss3066.rom",              0x0000, 0x008000, CRC(ccfb82e0) SHA1(08095517eb0bd8931286567171c307603b0cdeff), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 24)" )
GAME_CUSTOM( 199?, ec_big7__x,  ec_big7,    "iss3067.rom",              0x0000, 0x008000, CRC(4543588f) SHA1(dd888f113fb2a326565e73514d682db43ad545b7), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 25)" )
GAME_CUSTOM( 199?, ec_big7__y,  ec_big7,    "iss3068.rom",              0x0000, 0x008000, CRC(2329e40e) SHA1(c5072f40b334eedb3a62a234b2f49498165b30d2), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 26)" )
GAME_CUSTOM( 199?, ec_big7__z,  ec_big7,    "iss3220.rom",              0x0000, 0x008000, CRC(005a926b) SHA1(8fcbf14e44a61f3db96c500c8f9912ab1dbe9c39), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 27)" )
GAME_CUSTOM( 199?, ec_big7__0,  ec_big7,    "iss3221.rom",              0x0000, 0x008000, CRC(ed6d729b) SHA1(c897a9d58731cd82fdf8d4ee492ea5fe5542f3e8), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 28)" )
GAME_CUSTOM( 199?, ec_big7__1,  ec_big7,    "iss3222.rom",              0x0000, 0x008000, CRC(d5f340d6) SHA1(0b9aba173cdec3c9a54038e042902420c78ae1b2), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 29)" )
GAME_CUSTOM( 199?, ec_big7__2,  ec_big7,    "iss3223.rom",              0x0000, 0x008000, CRC(b5c9465d) SHA1(c51270c597bd6264e6440cdad726d032e8df45e6), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 30)" )
GAME_CUSTOM( 199?, ec_big7__3,  ec_big7,    "iss3224.rom",              0x0000, 0x008000, CRC(6f776b1f) SHA1(027689cf24bbf2386d9710c7e13329988168c253), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 31)" )
GAME_CUSTOM( 199?, ec_big7__4,  ec_big7,    "iss3225.rom",              0x0000, 0x008000, CRC(3fb0b783) SHA1(b65deadcb5fc1b50064d7f6cfc8fe141051074fb), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 32)" )
GAME_CUSTOM( 199?, ec_big7__5,  ec_big7,    "iss3226.rom",              0x0000, 0x008000, CRC(c9ee61ff) SHA1(bd5fb65ed2f1e3a23325aee98b420f6c263bf0c1), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 33)" )
GAME_CUSTOM( 199?, ec_big7__6,  ec_big7,    "iss3227.rom",              0x0000, 0x008000, CRC(a4065969) SHA1(1aa88869ed17844b993bf3138e616b60198e6603), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 34)" )
GAME_CUSTOM( 199?, ec_big7__7,  ec_big7,    "iss3228.rom",              0x0000, 0x008000, CRC(1893a5dc) SHA1(6e5069ddf3356742f7edf8ac04cd9d0897ac436c), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 35)" )
GAME_CUSTOM( 199?, ec_big7__8,  ec_big7,    "iss3229.rom",              0x0000, 0x008000, CRC(b9368f58) SHA1(614cd2940e2429923945e42411ff59b52d4fff9c), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 36)" )
GAME_CUSTOM( 199?, ec_big7__9,  ec_big7,    "iss3230.rom",              0x0000, 0x008000, CRC(9bf662c8) SHA1(7909d1e4775d9efad299cbce7b86dda2d3a21bed), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 37)" )
GAME_CUSTOM( 199?, ec_big7__aa, ec_big7,    "iss3231.rom",              0x0000, 0x008000, CRC(4862536a) SHA1(d5d80467c798dd3361c8ac367a1b2734741cc8f8), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 38)" )
GAME_CUSTOM( 199?, ec_big7__ab, ec_big7,    "iss3232.rom",              0x0000, 0x008000, CRC(7c5b1a26) SHA1(292ababf9be8303724b0cff12004202ac8cee674), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 39)" )
GAME_CUSTOM( 199?, ec_big7__ac, ec_big7,    "iss3233.rom",              0x0000, 0x008000, CRC(b753592b) SHA1(e0414808276c76e609ac4fb006b08952528603d3), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 40)" )
GAME_CUSTOM( 199?, ec_big7__ad, ec_big7,    "iss3234.rom",              0x0000, 0x008000, CRC(6e96db38) SHA1(ae569a37c866183a08706d0b50254822382cd156), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 41)" )
GAME_CUSTOM( 199?, ec_big7__ae, ec_big7,    "iss3235.rom",              0x0000, 0x008000, CRC(672f3f29) SHA1(7497cf5fa3cd9e5652dbbd4c691b8bdc0943a9e5), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 42)" )
GAME_CUSTOM( 199?, ec_big7__af, ec_big7,    "iss3236.rom",              0x0000, 0x008000, CRC(3eb8a0b4) SHA1(f41c76fac44bfd9c9275e2cf45e8bd16d72b800b), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 43)" )
GAME_CUSTOM( 199?, ec_big7__ag, ec_big7,    "iss3237.rom",              0x0000, 0x008000, CRC(db876c46) SHA1(f986407029e1c35651daea1fde87f8a3bb1b1965), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 44)" )
GAME_CUSTOM( 199?, ec_big7__ah, ec_big7,    "iss3238.rom",              0x0000, 0x008000, CRC(c7d1d398) SHA1(3b37b9596bc3771a6f1a698bee4dce8d642d982f), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 45)" )
GAME_CUSTOM( 199?, ec_big7__ai, ec_big7,    "iss3239.rom",              0x0000, 0x008000, CRC(f62450a6) SHA1(d2c88483cb0d3a83a2974550e8e8e71642bb28ce), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 46)" )
GAME_CUSTOM( 199?, ec_big7__aj, ec_big7,    "iss3240.rom",              0x0000, 0x008000, CRC(e8e56ca4) SHA1(d16390b600f9966b779638e3bc2e7f9a72e8d1be), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 47)" ) // 'Super Big 7' ?
/* No indication, header space */
GAME_CUSTOM( 199?, ec_big7__au, ec_big7,    "big76c.bin",               0x0000, 0x008000, CRC(12048afc) SHA1(a9da4d65efd794ebdb3daad0615a5c6a81135763), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 58)" )
GAME_CUSTOM( 199?, ec_big7__av, ec_big7,    "big7_issue382_8tkn.bin",   0x0000, 0x008000, CRC(706d87dd) SHA1(9c066ca8d5119d15bd09c07110fc66c1fe890a0c), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 59)" )
GAME_CUSTOM( 199?, ec_big7__az, ec_big7,    "iss179.rom",               0x0000, 0x008000, CRC(ef34fa31) SHA1(4cd19c50449af95d8448266b8fca6ff94437c22d), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 63)" )
GAME_CUSTOM( 199?, ec_big7__a0, ec_big7,    "iss2017.rom",              0x0000, 0x008000, CRC(165dc63c) SHA1(f820bc99755f38a911357e705075d24d3aac43b7), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 64)" )
GAME_CUSTOM( 199?, ec_big7__a1, ec_big7,    "iss2019.rom",              0x0000, 0x008000, CRC(475b224a) SHA1(c837aa0c73cf5947b6b4d106d4f0967da040e5dc), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 65)" )
GAME_CUSTOM( 199?, ec_big7__a2, ec_big7,    "iss513.rom",               0x0000, 0x008000, CRC(ca302c47) SHA1(9fb9cdd140baa0ec36250b4ebd0a25450348075f), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 66)" )
// No Header - Taken from a Super Big 7 Set
GAME_CUSTOM( 199?, ec_big7__ak, ec_big7,    "iss197.rom",               0x0000, 0x008000, CRC(45d975c8) SHA1(1ef7693fb000b85f661ebd06512f916297d0662c), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 48)" ) // ELCNBIG7 - Thu Mar 16 13:21:54 1995
GAME_CUSTOM( 199?, ec_big7__al, ec_big7,    "sb7.58",                   0x0000, 0x008000, CRC(0876d8bf) SHA1(b15584c7c994d29010652cdf8d9c79b661e01b01), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 49)" ) // ELCNBIG7 - Thu May 27 18:47:36 1993
/* sets below have different behavior, maybe 3rd party? */
// Different Code structure, no space for header - checking some kind of device at 0xa000 */
GAME_CUSTOM( 199?, ec_big7__am, ec_big7,    "sb710d",                   0x0000, 0x008000, CRC(9d9d14fe) SHA1(acc4c92a800d0891ebace8a60d04df09b43bfb1c), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 50)" ) // ELCNBIG7 - no date string
GAME_CUSTOM( 199?, ec_big7__an, ec_big7,    "b710",                     0x0000, 0x008000, CRC(0cdae404) SHA1(e8d713e172e5ff37e31e68d096fac77fbe676006), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 51)" ) // ELCNBIG7 - no date string
GAME_CUSTOM( 199?, ec_big7__as, ec_big7,    "big78t",                   0x0000, 0x008000, CRC(310ffd92) SHA1(1cfc3801bb04d4e3d4c2d6e271c3ac71c49d466b), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 56)" ) // ELCNBIG7 - no date string
GAME_CUSTOM( 199?, ec_big7__at, ec_big7,    "genbig.bin",               0x0000, 0x008000, CRC(025b129f) SHA1(07d53f8780fca7b90243c01f5892f3c0622ca387), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 57)" ) // ELCNBIG7 - no date string
/* device at a100? */
GAME_CUSTOM( 199?, ec_big7__ar, ec_big7,    "big 7 8 1-0.bin",          0x0000, 0x008000, CRC(164fd1e6) SHA1(25be8962f8b7a6a78345dd60319a391c583b6b2f), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 55)" ) // ELCNBIG7 - no date string
/* No indication, no space for header */
GAME_CUSTOM( 199?, ec_big7__ao, ec_big7,    "b75p4",                    0x0000, 0x008000, CRC(27ad1971) SHA1(4c1248d5815143dc0b23ada909c4f1fc16a1a18b), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 52)" ) // ELCNBIG7 - no date string
GAME_CUSTOM( 199?, ec_big7__ap, ec_big7,    "b78ac",                    0x0000, 0x008000, CRC(454e9ac5) SHA1(a700a399632fa546473503f8e7e8dc3abc966ee6), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 53)" ) // ELCNBIG7 - no date string
GAME_CUSTOM( 199?, ec_big7__aq, ec_big7,    "b7rb5",                    0x0000, 0x008000, CRC(cc59283a) SHA1(63d53f6f5e9c16df77a430443aade18722d7bcd7), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 54)" ) // ELCNBIG7 - no date string
/* device at a000? */
GAME_CUSTOM( 199?, ec_big7__aw, ec_big7,    "bigcon10.hex",             0x0000, 0x008000, CRC(b1176841) SHA1(ef23a61355ff194b1dd2c54bc94b175272a8058d), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 60)" ) // ELCNBIG7 - Tue Mar 08 10:05:02 1994
GAME_CUSTOM( 199?, ec_big7__ax, ec_big7,    "bigcon8c.hex",             0x0000, 0x008000, CRC(5b586abc) SHA1(8cabb266db4e0453e081ce6ff3ee0c850b66bede), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 61)" ) // ELCNBIG7 - Tue Mar 08 10:05:02 1994
GAME_CUSTOM( 199?, ec_big7__ay, ec_big7,    "bigcon8t.hex",             0x0000, 0x008000, CRC(809e2ec5) SHA1(15a1da75f24c167089051645362c9f53be54e16e), "Electrocoin","Big 7 / Super Big 7 (Electrocoin) (set 62)" ) // ELCNBIG7 - Tue Mar 08 10:05:02 1994


#define EC_SBARX_OTHERS \
	ROM_REGION( 0x200000, "sndz80", ROMREGION_ERASE00 ) \
	/* probably the same sound rom */
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		EC_SBARX_OTHERS \
	ROM_END \
	GAME(year, setname, parent ,ecoinfr ,ecoinfr_barx , ecoinfr_state,ecoinfr ,ROT0,company,title,GAME_FLAGS )

/* Identified as 'SBARX2' header like BIG7 */
GAME_CUSTOM( 199?, ec_sbarx,       0,          "iss3001.rom",                          0x0000, 0x008000, CRC(01390318) SHA1(e01a4160f774e376b5527ddee084a0be3eef865e), "Electrocoin","Super Bar X (Electrocoin) (set 1)" )
GAME_CUSTOM( 199?, ec_sbarx__a,    ec_sbarx,   "iss3002.rom",                          0x0000, 0x008000, CRC(84b323f9) SHA1(911b1355a8baa5adb4f956ead7379cb4b69abdcb), "Electrocoin","Super Bar X (Electrocoin) (set 2)" )
GAME_CUSTOM( 199?, ec_sbarx__b,    ec_sbarx,   "iss3003.rom",                          0x0000, 0x008000, CRC(aeac581f) SHA1(ffafdf444b77a1cbc71ba0dbd4e08b48a1182a6d), "Electrocoin","Super Bar X (Electrocoin) (set 3)" )
GAME_CUSTOM( 199?, ec_sbarx__c,    ec_sbarx,   "iss3004.rom",                          0x0000, 0x008000, CRC(a3f9d261) SHA1(ae8657a4336a3508f79fbe089afddfcfdb76ef7f), "Electrocoin","Super Bar X (Electrocoin) (set 4)" )
GAME_CUSTOM( 199?, ec_sbarx__d,    ec_sbarx,   "iss3005.rom",                          0x0000, 0x008000, CRC(cd0d29ff) SHA1(fb52aea3cd2b2c7e133594b92657466988fae8aa), "Electrocoin","Super Bar X (Electrocoin) (set 5)" )
GAME_CUSTOM( 199?, ec_sbarx__e,    ec_sbarx,   "iss3006.rom",                          0x0000, 0x008000, CRC(d8cd43af) SHA1(9bc1131a860b2f5421c17546720d4eb438215c63), "Electrocoin","Super Bar X (Electrocoin) (set 6)" )
GAME_CUSTOM( 199?, ec_sbarx__f,    ec_sbarx,   "iss3007.rom",                          0x0000, 0x008000, CRC(fecc57d6) SHA1(54b324049fae1dbef7b8b2eb7dd7967dc20d6f0f), "Electrocoin","Super Bar X (Electrocoin) (set 7)" )
GAME_CUSTOM( 199?, ec_sbarx__g,    ec_sbarx,   "iss3008.rom",                          0x0000, 0x008000, CRC(714459a7) SHA1(887391e73dfc216631273b56ea55ccafe566000a), "Electrocoin","Super Bar X (Electrocoin) (set 8)" )
GAME_CUSTOM( 199?, ec_sbarx__h,    ec_sbarx,   "iss3009.rom",                          0x0000, 0x008000, CRC(702a1225) SHA1(eb7b3b3ab4f41fdf7ea04b0a50b6ea1bdf9678e8), "Electrocoin","Super Bar X (Electrocoin) (set 9)" )
GAME_CUSTOM( 199?, ec_sbarx__i,    ec_sbarx,   "iss3010.rom",                          0x0000, 0x008000, CRC(a6fcfc08) SHA1(0626072425f93d95cd782bdbf62f528621bb86d6), "Electrocoin","Super Bar X (Electrocoin) (set 10)" )
GAME_CUSTOM( 199?, ec_sbarx__j,    ec_sbarx,   "iss3011.rom",                          0x0000, 0x008000, CRC(4ab461f0) SHA1(521eb529838ca84ce26f6a77ba60a272426243f0), "Electrocoin","Super Bar X (Electrocoin) (set 11)" )
GAME_CUSTOM( 199?, ec_sbarx__k,    ec_sbarx,   "iss3012.rom",                          0x0000, 0x008000, CRC(2f4c9ad8) SHA1(f78c5bdf3b0f75db59ce4075b82bb58d6ce2cf8c), "Electrocoin","Super Bar X (Electrocoin) (set 12)" )
GAME_CUSTOM( 199?, ec_sbarx__l,    ec_sbarx,   "iss3013.rom",                          0x0000, 0x008000, CRC(6bcf4550) SHA1(ae80f1482992d681556b10bfe86251920f317a8e), "Electrocoin","Super Bar X (Electrocoin) (set 13)" )
GAME_CUSTOM( 199?, ec_sbarx__m,    ec_sbarx,   "iss3014.rom",                          0x0000, 0x008000, CRC(ddb2220f) SHA1(244e3a481a386d01b473f041e3fb3cc343b5a966), "Electrocoin","Super Bar X (Electrocoin) (set 14)" )
GAME_CUSTOM( 199?, ec_sbarx__n,    ec_sbarx,   "iss3015.rom",                          0x0000, 0x008000, CRC(83a51dc7) SHA1(6dea8ae51fd9ca057db3495f2c616e347dfd9c07), "Electrocoin","Super Bar X (Electrocoin) (set 15)" )
GAME_CUSTOM( 199?, ec_sbarx__o,    ec_sbarx,   "iss3016.rom",                          0x0000, 0x008000, CRC(d7fdccff) SHA1(4d2490cf5577b5d757183dbc47a1f869863e15c0), "Electrocoin","Super Bar X (Electrocoin) (set 16)" )
GAME_CUSTOM( 199?, ec_sbarx__p,    ec_sbarx,   "iss3017.rom",                          0x0000, 0x008000, CRC(d83961b0) SHA1(0144cf5a2bd45735ce44df6ed119e37ed7bf82c2), "Electrocoin","Super Bar X (Electrocoin) (set 17)" )
GAME_CUSTOM( 199?, ec_sbarx__q,    ec_sbarx,   "iss3018.rom",                          0x0000, 0x008000, CRC(6d7fc134) SHA1(af82b6e7e16c5a9df284d0c2d44b1a000bcdf9aa), "Electrocoin","Super Bar X (Electrocoin) (set 18)" )
GAME_CUSTOM( 199?, ec_sbarx__r,    ec_sbarx,   "iss3019.rom",                          0x0000, 0x008000, CRC(a1e8b73b) SHA1(d2400ed2ac4d9b41a5fd2cb0910677b329b17ca5), "Electrocoin","Super Bar X (Electrocoin) (set 19)" )
GAME_CUSTOM( 199?, ec_sbarx__s,    ec_sbarx,   "iss3020.rom",                          0x0000, 0x008000, CRC(40eb69d5) SHA1(0fe9d62dabc909c85176e187b95bb99c4372b0d5), "Electrocoin","Super Bar X (Electrocoin) (set 20)" )
GAME_CUSTOM( 199?, ec_sbarx__t,    ec_sbarx,   "iss3200.rom",                          0x0000, 0x008000, CRC(16cb8ba6) SHA1(b98b4b9b97deb35e9286188ee3e5e0f977f97271), "Electrocoin","Super Bar X (Electrocoin) (set 21)" )
GAME_CUSTOM( 199?, ec_sbarx__u,    ec_sbarx,   "iss3201.rom",                          0x0000, 0x008000, CRC(79fea244) SHA1(2332d2d587eb138293edb169201112a028e26a2f), "Electrocoin","Super Bar X (Electrocoin) (set 22)" )
GAME_CUSTOM( 199?, ec_sbarx__v,    ec_sbarx,   "iss3202.rom",                          0x0000, 0x008000, CRC(bb928182) SHA1(d4294cfd0b4b94257d436eadf500cc12dcdc495e), "Electrocoin","Super Bar X (Electrocoin) (set 23)" )
GAME_CUSTOM( 199?, ec_sbarx__w,    ec_sbarx,   "iss3203.rom",                          0x0000, 0x008000, CRC(b417a15f) SHA1(f1f82b54178848573504f9a9841f30e191ac8455), "Electrocoin","Super Bar X (Electrocoin) (set 24)" )
GAME_CUSTOM( 199?, ec_sbarx__x,    ec_sbarx,   "iss3204.rom",                          0x0000, 0x008000, CRC(09b3b872) SHA1(eed041162751658e0270f4e27a7411d61b84b4a9), "Electrocoin","Super Bar X (Electrocoin) (set 25)" )
GAME_CUSTOM( 199?, ec_sbarx__y,    ec_sbarx,   "iss3205.rom",                          0x0000, 0x008000, CRC(67f3bdcc) SHA1(00d4ef2b50b1eda0aedfa3cb6dcef78d9b80bd35), "Electrocoin","Super Bar X (Electrocoin) (set 26)" )
GAME_CUSTOM( 199?, ec_sbarx__z,    ec_sbarx,   "iss3206.rom",                          0x0000, 0x008000, CRC(f7bcbf95) SHA1(6a71bce7fcec1e8135dd42901974aa0debdb566c), "Electrocoin","Super Bar X (Electrocoin) (set 27)" )
GAME_CUSTOM( 199?, ec_sbarx__0,    ec_sbarx,   "iss3207.rom",                          0x0000, 0x008000, CRC(ac0b929d) SHA1(cc40f128a5a3c2e4ff6b30f1bf95fbdfa68137b5), "Electrocoin","Super Bar X (Electrocoin) (set 28)" )
GAME_CUSTOM( 199?, ec_sbarx__1,    ec_sbarx,   "iss3208.rom",                          0x0000, 0x008000, CRC(594fe5c2) SHA1(6bf7402c899ba31c1063301468b3fb89063fb58f), "Electrocoin","Super Bar X (Electrocoin) (set 29)" )
GAME_CUSTOM( 199?, ec_sbarx__2,    ec_sbarx,   "iss3209.rom",                          0x0000, 0x008000, CRC(cd5bf63f) SHA1(043f67bb669cfbe8548c5689a69cde8260528ffd), "Electrocoin","Super Bar X (Electrocoin) (set 30)" )
GAME_CUSTOM( 199?, ec_sbarx__3,    ec_sbarx,   "iss3210.rom",                          0x0000, 0x008000, CRC(0faec005) SHA1(b22b2dbcc5e023d7c76a6d4fb5636b5ae2e08d13), "Electrocoin","Super Bar X (Electrocoin) (set 31)" )
GAME_CUSTOM( 199?, ec_sbarx__4,    ec_sbarx,   "iss3211.rom",                          0x0000, 0x008000, CRC(4861770e) SHA1(ee4813370b27ff58dc78aa62c799efbaefc1e61d), "Electrocoin","Super Bar X (Electrocoin) (set 32)" )
GAME_CUSTOM( 199?, ec_sbarx__5,    ec_sbarx,   "iss3212.rom",                          0x0000, 0x008000, CRC(d11bcb08) SHA1(854318b64cc1ff7eed4d57796ae873f7088ef48a), "Electrocoin","Super Bar X (Electrocoin) (set 33)" )
GAME_CUSTOM( 199?, ec_sbarx__6,    ec_sbarx,   "iss3213.rom",                          0x0000, 0x008000, CRC(0f57908b) SHA1(2a15b2659b4db7caa1d3b0dfdc712a746dcf189d), "Electrocoin","Super Bar X (Electrocoin) (set 34)" )
GAME_CUSTOM( 199?, ec_sbarx__7,    ec_sbarx,   "iss3214.rom",                          0x0000, 0x008000, CRC(61f13078) SHA1(d934972e3124a1ed8a0e86c52ab4733db86c7c23), "Electrocoin","Super Bar X (Electrocoin) (set 35)" )
GAME_CUSTOM( 199?, ec_sbarx__8,    ec_sbarx,   "iss3215.rom",                          0x0000, 0x008000, CRC(c61a459d) SHA1(e44ccd607bed807281358b405bb1d1f66f9eb26b), "Electrocoin","Super Bar X (Electrocoin) (set 36)" )
GAME_CUSTOM( 199?, ec_sbarx__9,    ec_sbarx,   "iss3216.rom",                          0x0000, 0x008000, CRC(803847c8) SHA1(ccf04a669d4f43dae74b4d37fefd3bc696299162), "Electrocoin","Super Bar X (Electrocoin) (set 37)" )
GAME_CUSTOM( 199?, ec_sbarx__aa,   ec_sbarx,   "iss3217.rom",                          0x0000, 0x008000, CRC(a63e76ff) SHA1(f19c848eca3b63743f9ab4f43f872a00a023d51c), "Electrocoin","Super Bar X (Electrocoin) (set 38)" )
GAME_CUSTOM( 199?, ec_sbarx__ab,   ec_sbarx,   "iss3218.rom",                          0x0000, 0x008000, CRC(ba47f5d5) SHA1(b20e02782ac25713dfeb0513740eb3d048dee282), "Electrocoin","Super Bar X (Electrocoin) (set 39)" )
GAME_CUSTOM( 199?, ec_sbarx__ac,   ec_sbarx,   "iss3219.rom",                          0x0000, 0x008000, CRC(72a9fd90) SHA1(191f375b41f56fab20b01926e3e55ddd691cd488), "Electrocoin","Super Bar X (Electrocoin) (set 40)" )
GAME_CUSTOM( 199?, ec_sbarx__ad,   ec_sbarx,   "iss3268.rom",                          0x0000, 0x008000, CRC(9ed62096) SHA1(78962170324b2af08143885d6033f14910195490), "Electrocoin","Super Bar X (Electrocoin) (set 41)" )
GAME_CUSTOM( 199?, ec_sbarx__ae,   ec_sbarx,   "iss3269.rom",                          0x0000, 0x008000, CRC(f3ae26cb) SHA1(18bf2c468f91a56b461e7f8037dd822735d40b23), "Electrocoin","Super Bar X (Electrocoin) (set 42)" )
GAME_CUSTOM( 199?, ec_sbarx__af,   ec_sbarx,   "iss3270.rom",                          0x0000, 0x008000, CRC(1b364354) SHA1(497f3a24e8c7da967ead5c460f5d7395d1ce689a), "Electrocoin","Super Bar X (Electrocoin) (set 43)" )
GAME_CUSTOM( 199?, ec_sbarx__ag,   ec_sbarx,   "iss3271.rom",                          0x0000, 0x008000, CRC(9f3ebc4e) SHA1(084ac6b0e90a735b139ac2624650127672f79ee7), "Electrocoin","Super Bar X (Electrocoin) (set 44)" )
GAME_CUSTOM( 199?, ec_sbarx__ah,   ec_sbarx,   "iss3272.rom",                          0x0000, 0x008000, CRC(b2f6e8cb) SHA1(0477a6b9ae0d900435fa570c1cada77eb902c25b), "Electrocoin","Super Bar X (Electrocoin) (set 45)" )
GAME_CUSTOM( 199?, ec_sbarx__ai,   ec_sbarx,   "iss3273.rom",                          0x0000, 0x008000, CRC(05b6c2c4) SHA1(c78eb44d440f8ca75f6904e6ab780708663351a9), "Electrocoin","Super Bar X (Electrocoin) (set 46)" )
GAME_CUSTOM( 199?, ec_sbarx__aj,   ec_sbarx,   "iss3274.rom",                          0x0000, 0x008000, CRC(489ecef1) SHA1(64d18423407670ac2afff70de4d6f4f371afd74b), "Electrocoin","Super Bar X (Electrocoin) (set 47)" )
GAME_CUSTOM( 199?, ec_sbarx__ak,   ec_sbarx,   "iss3275.rom",                          0x0000, 0x008000, CRC(8597c0ab) SHA1(774e5c2e91f7317ca4e3cd305a387f2d284de15f), "Electrocoin","Super Bar X (Electrocoin) (set 48)" )
GAME_CUSTOM( 199?, ec_sbarx__al,   ec_sbarx,   "iss3276.rom",                          0x0000, 0x008000, CRC(59528755) SHA1(81373e0625f93e68900c0ba1c986011fa8541028), "Electrocoin","Super Bar X (Electrocoin) (set 49)" )
GAME_CUSTOM( 199?, ec_sbarx__am,   ec_sbarx,   "iss3277.rom",                          0x0000, 0x008000, CRC(ae614832) SHA1(055a0cecbb6e9939c26db1af67e2823b9c55de0b), "Electrocoin","Super Bar X (Electrocoin) (set 50)" )
GAME_CUSTOM( 199?, ec_sbarx__an,   ec_sbarx,   "iss3278.rom",                          0x0000, 0x008000, CRC(a5b504e1) SHA1(5c9e17482f204073f8aab8540463231ccac85c7e), "Electrocoin","Super Bar X (Electrocoin) (set 51)" )
GAME_CUSTOM( 199?, ec_sbarx__ao,   ec_sbarx,   "iss3279.rom",                          0x0000, 0x008000, CRC(bd2a7c56) SHA1(1b4e95b3e82999e276bd72c768311ccfaaeae4a9), "Electrocoin","Super Bar X (Electrocoin) (set 52)" )
GAME_CUSTOM( 199?, ec_sbarx__ap,   ec_sbarx,   "iss3280.rom",                          0x0000, 0x008000, CRC(511d4f2f) SHA1(d9fa6baf0e23eaa7d62d3a09cbdd7fc05f955f68), "Electrocoin","Super Bar X (Electrocoin) (set 53)" )
GAME_CUSTOM( 199?, ec_sbarx__aq,   ec_sbarx,   "iss3281.rom",                          0x0000, 0x008000, CRC(37a21ce2) SHA1(165015ece80706ca0a0062b884c25c054906d9f7), "Electrocoin","Super Bar X (Electrocoin) (set 54)" )
GAME_CUSTOM( 199?, ec_sbarx__ar,   ec_sbarx,   "iss3282.rom",                          0x0000, 0x008000, CRC(1d44636f) SHA1(3502c576b4806685a28da3c70a4a534dfe8446f5), "Electrocoin","Super Bar X (Electrocoin) (set 55)" )
GAME_CUSTOM( 199?, ec_sbarx__as,   ec_sbarx,   "iss3283.rom",                          0x0000, 0x008000, CRC(54155620) SHA1(33e1d0332cff80cab8402ea4aa6048a8e64445e8), "Electrocoin","Super Bar X (Electrocoin) (set 56)" )
GAME_CUSTOM( 199?, ec_sbarx__at,   ec_sbarx,   "iss3284.rom",                          0x0000, 0x008000, CRC(cf0db191) SHA1(dd41b9a89c5a7061ae63ba9dd10d407b58621b43), "Electrocoin","Super Bar X (Electrocoin) (set 57)" )
GAME_CUSTOM( 199?, ec_sbarx__au,   ec_sbarx,   "iss3285.rom",                          0x0000, 0x008000, CRC(b0f3d198) SHA1(06c6d7a3d7aa4c108d4f9c9e5854fb8c0db8749c), "Electrocoin","Super Bar X (Electrocoin) (set 58)" )
GAME_CUSTOM( 199?, ec_sbarx__av,   ec_sbarx,   "iss3286.rom",                          0x0000, 0x008000, CRC(50fe610b) SHA1(18aa1f884933606bbb5e970aaee89ca7f31cb177), "Electrocoin","Super Bar X (Electrocoin) (set 59)" )
GAME_CUSTOM( 199?, ec_sbarx__aw,   ec_sbarx,   "iss3287.rom",                          0x0000, 0x008000, CRC(694aa6a5) SHA1(a679bfd98b105028a87ec8366af67ffaefde6711), "Electrocoin","Super Bar X (Electrocoin) (set 60)" )
GAME_CUSTOM( 199?, ec_sbarx__ax,   ec_sbarx,   "iss9401.rom",                          0x0000, 0x008000, CRC(abe83480) SHA1(581fab39096b6327b8e88c7ce848126123f524b8), "Electrocoin","Super Bar X (Electrocoin) (set 61)" )
GAME_CUSTOM( 199?, ec_sbarx__ay,   ec_sbarx,   "iss9405.rom",                          0x0000, 0x008000, CRC(6435586d) SHA1(95f2cda1bc80bb8f7c3d2d2b41abbfd634a88237), "Electrocoin","Super Bar X (Electrocoin) (set 62)" ) // from unknown set
/* No header (all 0x00) space for one tho */
GAME_CUSTOM( 199?, ec_sbarx__az,   ec_sbarx,   "iss129.rom",                           0x0000, 0x008000, CRC(b4adae28) SHA1(5d747624dafc8d65fd3b49ff3649ad9973d9271b), "Electrocoin","Super Bar X (Electrocoin) (set 63)" ) // ELCNSBRX - Sat Jun 22 13:24:27 1996
GAME_CUSTOM( 199?, ec_sbarx__a0,   ec_sbarx,   "superbarx_issue158_sitdown10cash.bin", 0x0000, 0x008000, CRC(80c2d523) SHA1(1252bf66987aa9ac610c3e9f0919f29a6ad6cc52), "Electrocoin","Super Bar X (Electrocoin) (set 64)" ) // ELCNSBRX - Mon Oct 14 11:03:39 1996
GAME_CUSTOM( 199?, ec_sbarx__a1,   ec_sbarx,   "iss173.rom",                           0x0000, 0x008000, CRC(984f9c18) SHA1(f160eeee6ea70c79502fc68f70cb973e1ca029b8), "Electrocoin","Super Bar X (Electrocoin) (set 65)" ) // ELCNSBRX - Thu Mar 16 12:12:19 1995
GAME_CUSTOM( 199?, ec_sbarx__a2,   ec_sbarx,   "iss2012.rom",                          0x0000, 0x008000, CRC(455cfdcb) SHA1(53fb0748a544b432b88455fa597b7017e06b3059), "Electrocoin","Super Bar X (Electrocoin) (set 66)" ) // ELCNSBRX - Thu Sep 25 10:18:38 1997
GAME_CUSTOM( 199?, ec_sbarx__a3,   ec_sbarx,   "sbx5red",                              0x0000, 0x008000, CRC(7991231a) SHA1(cd1978c48a3c214666d51ca930d3d480540448ec), "Electrocoin","Super Bar X (Electrocoin) (set 67)" ) // ELCNSBRX - Tue Mar 08 09:51:56 1994
GAME_CUSTOM( 199?, ec_sbarx__a4,   ec_sbarx,   "sbx8elac",                             0x0000, 0x008000, CRC(102a3f38) SHA1(5f4f55904b00dde47e9841de313ed76a56e711df), "Electrocoin","Super Bar X (Electrocoin) (set 68)" ) // ELCNSBRX - Mon Apr 21 19:01:53 1997






/* Unique ROM containing 1993 Electrocoin Copyright - for a z180 instead? */



/* is Brunel Research the company, or related to the Brunel language? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		EC_SBARX_OTHERS \
	ROM_END \
	GAME(year, setname, parent ,ecoinfr ,ecoinfr_barx , ecoinfr_state,ecoinfrbr ,ROT0,company,title,GAME_FLAGS )

/* No Header - very similar to Brunel sets, but no device at 0xa000 */ // spin the reels a lot more than anything else
GAME_CUSTOM( 1991, ec_sbxbr,         0,          "sbx5nc.10",                            0x0000, 0x008000, CRC(beb7254a) SHA1(137e91e0b92d970d09d165a42b890a5d31d795d9), "Brunel Research","Super Bar X (Brunel Research) (set 1)" )
GAME_CUSTOM( 1991, ec_sbxbra,        ec_sbxbr,   "sbx5nc.20",                            0x0000, 0x008000, CRC(0ceb3e29) SHA1(e96e1470292208825407ba64750121dd3c7bf857), "Brunel Research","Super Bar X (Brunel Research) (set 2)" )
GAME_CUSTOM( 1991, ec_sbxbrb,        ec_sbxbr,   "sbxup",                                0x0000, 0x008000, CRC(f8d7e9db) SHA1(7dea1f7215070a8a413af63d0e379b2e228e63d7), "Brunel Research","Super Bar X (Brunel Research) (set 3)" )
GAME_CUSTOM( 1991, ec_sbxbrc,        ec_sbxbr,   "sbxup_10",                             0x0000, 0x008000, CRC(3c932de3) SHA1(2c1e09436a5895aa738567843c7f25ed047dc9ac), "Brunel Research","Super Bar X (Brunel Research) (set 4)" )
/* 1991 BRUNEL RESEARCH Copyright, device at a000 / a001 / a100 */
GAME_CUSTOM( 1991, ec_sbxbrd,        ec_sbxbr,   "sbarx.210",                            0x0000, 0x008000, CRC(1e9933b2) SHA1(ee546cd2f0659c669b98a14f032298ebc4fa7e5c), "Brunel Research","Super Bar X (Brunel Research) (set 5)" ) // sbx210
GAME_CUSTOM( 1991, ec_sbxbre,        ec_sbxbr,   "sbx18ac",                              0x0000, 0x008000, CRC(a3b4cfbe) SHA1(20f78d565504878d0d6a53b6bc32e31d3a32c736), "Brunel Research","Super Bar X (Brunel Research) (set 6)" )
GAME_CUSTOM( 1991, ec_sbxbrf,        ec_sbxbr,   "sbx2 8t",                              0x0000, 0x008000, CRC(c63e8d0a) SHA1(17ccb75602a2738296b419761835008ef798fdb0), "Brunel Research","Super Bar X (Brunel Research) (set 7)" ) // sbx8d
GAME_CUSTOM( 1991, ec_sbxbrg,        ec_sbxbr,   "sbx28ac",                              0x0000, 0x008000, CRC(338ff3e3) SHA1(d8470b029aff7b6b8f07df19d9edcf3d01b7e3d0), "Brunel Research","Super Bar X (Brunel Research) (set 8)" )
GAME_CUSTOM( 1991, ec_sbxbrh,        ec_sbxbr,   "super bar x 8 1-0.bin",                0x0000, 0x008000, CRC(b33e2891) SHA1(c0383740776a20f41de3f1a46c766a8e6c53101f), "Brunel Research","Super Bar X (Brunel Research) (set 9)" )



#define EC_MAG7S_OTHERS \
	ROM_REGION( 0x200000, "sndz80", ROMREGION_ERASE00 ) \
	/* probably the same sound rom */
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		EC_MAG7S_OTHERS \
	ROM_END \
	GAME(year, setname, parent ,ecoinfr ,ecoinfr_barx , ecoinfr_state,ecoinfr ,ROT0,company,title,GAME_FLAGS )



// These are '2001 COOL7' (older header type with 0 at end)
GAME_CUSTOM( 199?, ec_mag7s,       0,          "issa111", 0x0000, 0x008000, CRC(dd98d4b6) SHA1(a66bb771f7ce66f38033c2704830500e876b9043), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 1)" ) // ---- ---0
GAME_CUSTOM( 199?, ec_mag7s__a,    ec_mag7s,   "issa112", 0x0000, 0x008000, CRC(14ba229d) SHA1(7506cb0e080643d33cdbf5d8c37743555fc117cd), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 2)" ) // -N-- ---0
GAME_CUSTOM( 199?, ec_mag7s__b,    ec_mag7s,   "issa116", 0x0000, 0x008000, CRC(a235cb7b) SHA1(4efa2b61203c2a4d01ecc0b0e4712c84eb7ad928), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 3)" ) // P--- ---0
GAME_CUSTOM( 199?, ec_mag7s__c,    ec_mag7s,   "issa117", 0x0000, 0x008000, CRC(9e30c9bf) SHA1(b9af56ff70d5740c2adde06e26458a2b024a5a57), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 4)" ) // PN-- ---0
// These are '2001 COOL7' (newer header type)
GAME_CUSTOM( 199?, ec_mag7s__d,    ec_mag7s,   "issa933", 0x0000, 0x008000, CRC(ebb6b015) SHA1(1c02663f1193b9aa92183ac46146c49cdb9fa420), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 5)" )
GAME_CUSTOM( 199?, ec_mag7s__e,    ec_mag7s,   "issa934", 0x0000, 0x008000, CRC(522c9ed5) SHA1(ae3f2760f10f8d884500d9fa67169bde9913fd52), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 6)" )
GAME_CUSTOM( 199?, ec_mag7s__f,    ec_mag7s,   "issa935", 0x0000, 0x008000, CRC(407c02d0) SHA1(f6f0216c7f39da462649711259a692dd519191e4), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 7)" )
GAME_CUSTOM( 199?, ec_mag7s__g,    ec_mag7s,   "issa936", 0x0000, 0x008000, CRC(a1e74625) SHA1(2b6e77c7031c646713c9af9aa6ba66c47982def7), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 8)" )
GAME_CUSTOM( 199?, ec_mag7s__h,    ec_mag7s,   "issa937", 0x0000, 0x008000, CRC(d293c379) SHA1(9cf85813475f821b87d7fee74ce7c1bd9f943ce5), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 9)" )
GAME_CUSTOM( 199?, ec_mag7s__i,    ec_mag7s,   "issa938", 0x0000, 0x008000, CRC(cd8b6b24) SHA1(6e7a5fb90b1d0520d3ea24074cb2e0224d84b3d8), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 10)" )
GAME_CUSTOM( 199?, ec_mag7s__j,    ec_mag7s,   "issa939", 0x0000, 0x008000, CRC(5427927f) SHA1(da5a33a078da47ef3730bb2e24c240d9e416895e), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 11)" )
GAME_CUSTOM( 199?, ec_mag7s__k,    ec_mag7s,   "issa940", 0x0000, 0x008000, CRC(7255ad02) SHA1(bc433974cd6250805277eed113d63ed9475ff2bd), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 12)" )
GAME_CUSTOM( 199?, ec_mag7s__l,    ec_mag7s,   "issa941", 0x0000, 0x008000, CRC(02d9eeea) SHA1(dbde3b1792b7b25261021ef30e9669c1eeb65ff2), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 13)" )
GAME_CUSTOM( 199?, ec_mag7s__m,    ec_mag7s,   "issa942", 0x0000, 0x008000, CRC(761b4ad6) SHA1(306c4b0af936582233ef98fa647e69a6b23948b6), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 14)" )
GAME_CUSTOM( 199?, ec_mag7s__n,    ec_mag7s,   "issa944", 0x0000, 0x008000, CRC(46abcf90) SHA1(d151d4badaaa8c2c140e4eddbd4f3a9b2456f3c1), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 15)" )
GAME_CUSTOM( 199?, ec_mag7s__o,    ec_mag7s,   "issa946", 0x0000, 0x008000, CRC(a1086dae) SHA1(33207d400f02f93b4360c37a01a719893c6ae7c8), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 16)" )
GAME_CUSTOM( 199?, ec_mag7s__p,    ec_mag7s,   "issa947", 0x0000, 0x008000, CRC(d2ace438) SHA1(c87e9c7c6debc534d488543a6991bfbd284119e6), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 17)" )
GAME_CUSTOM( 199?, ec_mag7s__q,    ec_mag7s,   "issa948", 0x0000, 0x008000, CRC(e7b26788) SHA1(d6460f1254746248ddf639c958f13b34eeb0db3e), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 18)" )
GAME_CUSTOM( 199?, ec_mag7s__r,    ec_mag7s,   "issa949", 0x0000, 0x008000, CRC(39553f21) SHA1(6b06d2e8fdf375f8727471f66545ece8bf40cd1d), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 19)" )
GAME_CUSTOM( 199?, ec_mag7s__s,    ec_mag7s,   "issa950", 0x0000, 0x008000, CRC(f323803d) SHA1(a334e6e0c130c1192ffd3018ae691928cbc6123c), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 20)" )
GAME_CUSTOM( 199?, ec_mag7s__t,    ec_mag7s,   "issa951", 0x0000, 0x008000, CRC(4437bd3d) SHA1(6a3020c4d826167e434b079407e2a83e52193bf5), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 21)" )
GAME_CUSTOM( 199?, ec_mag7s__u,    ec_mag7s,   "issa952", 0x0000, 0x008000, CRC(db3ece7e) SHA1(ee298113780466ad01183c7d4135ff4bcd4a4d73), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 22)" )
GAME_CUSTOM( 199?, ec_mag7s__v,    ec_mag7s,   "issa953", 0x0000, 0x008000, CRC(be22b80a) SHA1(a69b489382087276e44c95fa68777f6707dad1f6), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 23)" )
GAME_CUSTOM( 199?, ec_mag7s__w,    ec_mag7s,   "issa954", 0x0000, 0x008000, CRC(9b9ef183) SHA1(8a784f669e3f44734f315988328fa6ecd4d05769), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 24)" )
GAME_CUSTOM( 199?, ec_mag7s__x,    ec_mag7s,   "issa955", 0x0000, 0x008000, CRC(1d589cdb) SHA1(def29aa1ce198d17ffe9384481f275e02696131a), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 25)" )
GAME_CUSTOM( 199?, ec_mag7s__y,    ec_mag7s,   "issa956", 0x0000, 0x008000, CRC(e55e3f4b) SHA1(b6a5bc30bd490b4db92e7a4417c5e7775930ef85), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 26)" )
GAME_CUSTOM( 199?, ec_mag7s__z,    ec_mag7s,   "issa957", 0x0000, 0x008000, CRC(0115f3e9) SHA1(fc443ceab666fbac0b7abb26dff7802be5eb57db), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 27)" )
GAME_CUSTOM( 199?, ec_mag7s__0,    ec_mag7s,   "issa958", 0x0000, 0x008000, CRC(05bfa8a7) SHA1(ff28d391669be6060d4aa8813d5c76d41460acf0), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 28)" )
GAME_CUSTOM( 199?, ec_mag7s__1,    ec_mag7s,   "issa959", 0x0000, 0x008000, CRC(15155b9b) SHA1(015e088b95ed5e762004918392b12442655dee2e), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 29)" )
GAME_CUSTOM( 199?, ec_mag7s__2,    ec_mag7s,   "issa960", 0x0000, 0x008000, CRC(ab6527cd) SHA1(9ccafa117efde2e2940af0ccfa4bda578999d22f), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 30)" )
GAME_CUSTOM( 199?, ec_mag7s__3,    ec_mag7s,   "issa961", 0x0000, 0x008000, CRC(9112dbce) SHA1(0b8ff8e9e3583db35d5e3bd177b971d786b01f54), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 31)" )
GAME_CUSTOM( 199?, ec_mag7s__4,    ec_mag7s,   "issa962", 0x0000, 0x008000, CRC(4983813d) SHA1(d338bab5a18ac52b397f611a3c56ef2b31ec68ac), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 32)" )
GAME_CUSTOM( 199?, ec_mag7s__5,    ec_mag7s,   "issa963", 0x0000, 0x008000, CRC(367d0c84) SHA1(104d24c92a76fd28b39464d7c22a1528ff17fa84), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 33)" )
GAME_CUSTOM( 199?, ec_mag7s__6,    ec_mag7s,   "issa964", 0x0000, 0x008000, CRC(3d97a784) SHA1(d14564292dcc253711bccae59edcdd21f7d9fcdb), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 34)" )
GAME_CUSTOM( 199?, ec_mag7s__7,    ec_mag7s,   "issa965", 0x0000, 0x008000, CRC(bfa13545) SHA1(c235de4b223bf8daa26d948b0f7e707a1794ca8d), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 35)" )
GAME_CUSTOM( 199?, ec_mag7s__8,    ec_mag7s,   "issa966", 0x0000, 0x008000, CRC(13a2e85c) SHA1(33cbc8757db492e92687e7008ca31425b6fe8d1c), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 36)" )
GAME_CUSTOM( 199?, ec_mag7s__9,    ec_mag7s,   "issa967", 0x0000, 0x008000, CRC(6b781c77) SHA1(10c70edd2f02dd888c3931f97b2731c4cc503e94), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 37)" )
GAME_CUSTOM( 199?, ec_mag7s__aa,   ec_mag7s,   "issa968", 0x0000, 0x008000, CRC(1cf678d8) SHA1(99827a46fccf341085da877a39104eb5d1e51b68), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 38)" )
GAME_CUSTOM( 199?, ec_mag7s__ab,   ec_mag7s,   "issa969", 0x0000, 0x008000, CRC(f8b218c7) SHA1(f58e5ad680226201c6e3fec277d9fc4907e918f1), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 39)" )
GAME_CUSTOM( 199?, ec_mag7s__ac,   ec_mag7s,   "issa970", 0x0000, 0x008000, CRC(e7297102) SHA1(216a7a52a10a4bc78fc78ac5fe0185ecae7467d7), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 40)" )
GAME_CUSTOM( 199?, ec_mag7s__ad,   ec_mag7s,   "issa971", 0x0000, 0x008000, CRC(9a375d99) SHA1(e0e9bde17ff0570e7bac7aecf0d0db1e4bb71fd7), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 41)" )
GAME_CUSTOM( 199?, ec_mag7s__ae,   ec_mag7s,   "issa972", 0x0000, 0x008000, CRC(2eec15a7) SHA1(396d23f8b3b22119df254249bb3db0cc60248020), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 42)" )
GAME_CUSTOM( 199?, ec_mag7s__af,   ec_mag7s,   "issa973", 0x0000, 0x008000, CRC(9d54fb14) SHA1(4d209c65468d43a4ccc7a30a2182f7bbe30a4b4a), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 43)" )
GAME_CUSTOM( 199?, ec_mag7s__ag,   ec_mag7s,   "issa974", 0x0000, 0x008000, CRC(e28680cf) SHA1(f0d15dc5a362967bebe1a70fb71860aaaf1cc48d), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 44)" )
GAME_CUSTOM( 199?, ec_mag7s__ah,   ec_mag7s,   "issa975", 0x0000, 0x008000, CRC(75568fd7) SHA1(09e16ad200bc0af4818ef6d9fdacd67a1e359b3e), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 45)" )
GAME_CUSTOM( 199?, ec_mag7s__ai,   ec_mag7s,   "issa976", 0x0000, 0x008000, CRC(0019933a) SHA1(891df20d970380f76248f2c562dc82babbf78eb3), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 46)" )
GAME_CUSTOM( 199?, ec_mag7s__aj,   ec_mag7s,   "issa977", 0x0000, 0x008000, CRC(ba347b31) SHA1(e5d61ce699687cd411f1baf89f9de649388837bb), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 47)" )
GAME_CUSTOM( 199?, ec_mag7s__ak,   ec_mag7s,   "issa978", 0x0000, 0x008000, CRC(630ab687) SHA1(e5dc589d8118adc848aa22e8758a17544fcf272d), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 48)" )
GAME_CUSTOM( 199?, ec_mag7s__al,   ec_mag7s,   "issa979", 0x0000, 0x008000, CRC(d8ebade9) SHA1(60e21dd0e4639214db384f7a645849ca79637ac0), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 49)" )
GAME_CUSTOM( 199?, ec_mag7s__am,   ec_mag7s,   "issa980", 0x0000, 0x008000, CRC(2f9f870e) SHA1(eb6578ffeb510e0fb76296bc1c22b322643db26f), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 50)" )
GAME_CUSTOM( 199?, ec_mag7s__an,   ec_mag7s,   "issa981", 0x0000, 0x008000, CRC(41cf7c23) SHA1(0f9da4143bb568cf99fe5db9752a5790ca089db6), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 51)" )
GAME_CUSTOM( 199?, ec_mag7s__ao,   ec_mag7s,   "issa982", 0x0000, 0x008000, CRC(771bc3ac) SHA1(f4c56b7b534a1e290f3880089f5cefaf9213fe79), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 52)" )
GAME_CUSTOM( 199?, ec_mag7s__ap,   ec_mag7s,   "issa983", 0x0000, 0x008000, CRC(581ef88a) SHA1(eaca5c5551506aac5ca94236509c6d6ad6ccd1ae), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 53)" )
GAME_CUSTOM( 199?, ec_mag7s__aq,   ec_mag7s,   "issa984", 0x0000, 0x008000, CRC(4cbdc18c) SHA1(1250223e5d2703dce0aa3b43e84da4324c90afdd), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 54)" )
GAME_CUSTOM( 199?, ec_mag7s__ar,   ec_mag7s,   "issa985", 0x0000, 0x008000, CRC(542b94fd) SHA1(778ca3baef947e0fd58037940edf6e9ac80a6ac8), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 55)" )
GAME_CUSTOM( 199?, ec_mag7s__as,   ec_mag7s,   "issa986", 0x0000, 0x008000, CRC(26716482) SHA1(6c6d5008b0a956b9458b232789644cc29a6d8ea8), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 56)" )
GAME_CUSTOM( 199?, ec_mag7s__at,   ec_mag7s,   "issa987", 0x0000, 0x008000, CRC(b0934975) SHA1(5ae813d2aa2fa0afcd11d76d308ee23dcbafe9de), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 57)" )
GAME_CUSTOM( 199?, ec_mag7s__au,   ec_mag7s,   "issa988", 0x0000, 0x008000, CRC(0e5d421e) SHA1(b61fc2d7651dc684168be2f0c8f0739a442a062c), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 58)" )
GAME_CUSTOM( 199?, ec_mag7s__av,   ec_mag7s,   "issa989", 0x0000, 0x008000, CRC(a469a2f8) SHA1(351f6e2849b2d5778fb96c96b2bb356f4b02787c), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 59)" )
GAME_CUSTOM( 199?, ec_mag7s__aw,   ec_mag7s,   "issa990", 0x0000, 0x008000, CRC(ad90f5a5) SHA1(2b09f7eb46f054550c5f638bc83708231c34e189), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 60)" )
// These are '2001 COOL7' (from Bar X 7 set
GAME_CUSTOM( 199?, ec_mag7s__ax,   ec_mag7s,   "issa943", 0x0000, 0x008000, CRC(e13a597f) SHA1(ab833fb8cc9529fc307b0252b922a77911802abe), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 61)" ) // P--- ----
GAME_CUSTOM( 199?, ec_mag7s__ay,   ec_mag7s,   "issa945", 0x0000, 0x008000, CRC(9c251b36) SHA1(319a82e9f0a5cd0e3c9d72ddb8203a9363cc3936), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 62)" ) // P-2S K---
// 2001 COOL7 (from Unknown set)
GAME_CUSTOM( 199?, ec_mag7s__az,   ec_mag7s,   "issa998", 0x0000, 0x008000, CRC(7314e2a8) SHA1(3a108bf2ba0173ecab85fe7110174f5db8f75e17), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 63)" )
// This just has 'Cool7' with no other header information (all 0x00)
// (C)1993/97 ELECTROCOIN
GAME_CUSTOM( 199?, ec_mag7s__a0,   ec_mag7s,   "majic",   0x0000, 0x008000, CRC(a1ca176f) SHA1(90dc3204091c6328764122dc583e47ea8ac314e4), "Electrocoin","Magic 7s / Cool 7 / Bar X 7 (2001 COOL7) (Electrocoin) (set 64)" ) // ELCNCOOL - on Jan 10 19:22:08 2000


#define EC_REDBR_OTHERS \
	ROM_REGION( 0x200000, "sndz80", ROMREGION_ERASE00 ) \
	/* probably the same sound rom */
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		EC_REDBR_OTHERS \
	ROM_END \
	GAME(year, setname, parent ,ecoinfr ,ecoinfr_barx , ecoinfr_state,ecoinfr ,ROT0,company,title,GAME_FLAGS )
// These are '2001 REDBAR' (older header type with 0 at end)
GAME_CUSTOM( 199?, ec_redbr,       0,          "issa101",                  0x0000, 0x008000, CRC(05bba52d) SHA1(fe1f80a6621564f8ea0fd741618ebd80a78a0055), "Electrocoin","Red Bar (Electrocoin) (set 1)" )
GAME_CUSTOM( 199?, ec_redbr__a,    ec_redbr,   "issa102",                  0x0000, 0x008000, CRC(9aebf74c) SHA1(4da5d9240a2dcfdaa96a8a784ea5745c90108f9e), "Electrocoin","Red Bar (Electrocoin) (set 2)" )
GAME_CUSTOM( 199?, ec_redbr__b,    ec_redbr,   "issa106",                  0x0000, 0x008000, CRC(77219ea1) SHA1(e4432e7c16b8911b272ae0bf3daf993649c4fd5f), "Electrocoin","Red Bar (Electrocoin) (set 3)" )
GAME_CUSTOM( 199?, ec_redbr__c,    ec_redbr,   "issa107",                  0x0000, 0x008000, CRC(e6e84ed9) SHA1(2151304bf7c2032edefd84ceb91a02af9df09c64), "Electrocoin","Red Bar (Electrocoin) (set 4)" )
// These are '2001 REDBAR' (newer header type)
GAME_CUSTOM( 199?, ec_redbr__d,    ec_redbr,   "issa875",                  0x0000, 0x008000, CRC(44cf12be) SHA1(4b9f001c9776989b4b21ed589b1dceb6cb040096), "Electrocoin","Red Bar (Electrocoin) (set 5)" )
GAME_CUSTOM( 199?, ec_redbr__e,    ec_redbr,   "issa876",                  0x0000, 0x008000, CRC(ef5d1343) SHA1(a609377a3dcfa7914a7a548e80e31fee3a6237fd), "Electrocoin","Red Bar (Electrocoin) (set 6)" )
GAME_CUSTOM( 199?, ec_redbr__f,    ec_redbr,   "issa877",                  0x0000, 0x008000, CRC(b39fa433) SHA1(0e2b012246259eab5f60575f404937077a285355), "Electrocoin","Red Bar (Electrocoin) (set 7)" )
GAME_CUSTOM( 199?, ec_redbr__g,    ec_redbr,   "issa878",                  0x0000, 0x008000, CRC(10ae2224) SHA1(f911e1762856bf5e0e23671bc3b34619d826363b), "Electrocoin","Red Bar (Electrocoin) (set 8)" )
GAME_CUSTOM( 199?, ec_redbr__h,    ec_redbr,   "issa879",                  0x0000, 0x008000, CRC(b50cf166) SHA1(89d5e0d5a4c72b34873c973806731afd2297539a), "Electrocoin","Red Bar (Electrocoin) (set 9)" )
GAME_CUSTOM( 199?, ec_redbr__i,    ec_redbr,   "issa880",                  0x0000, 0x008000, CRC(13c9cc9a) SHA1(d49b53e65066dded0b9cbf7b25ece320efa25a6b), "Electrocoin","Red Bar (Electrocoin) (set 10)" )
GAME_CUSTOM( 199?, ec_redbr__j,    ec_redbr,   "issa881",                  0x0000, 0x008000, CRC(5293c78f) SHA1(88de1a47bb01b47ecf36965e204be31c1e0edff8), "Electrocoin","Red Bar (Electrocoin) (set 11)" )
GAME_CUSTOM( 199?, ec_redbr__k,    ec_redbr,   "issa882",                  0x0000, 0x008000, CRC(2647ec6c) SHA1(58d14f140f4f421642a25054a4ebf121c77341b3), "Electrocoin","Red Bar (Electrocoin) (set 12)" )
GAME_CUSTOM( 199?, ec_redbr__l,    ec_redbr,   "issa883",                  0x0000, 0x008000, CRC(fbfbd720) SHA1(19c2ddf3edada5b93bef062e3f018f191f4b7c27), "Electrocoin","Red Bar (Electrocoin) (set 13)" )
GAME_CUSTOM( 199?, ec_redbr__m,    ec_redbr,   "issa884",                  0x0000, 0x008000, CRC(d453b1eb) SHA1(bc1f71c5bb8cbce8d7b3b5e5dbebd6739e1dc8d5), "Electrocoin","Red Bar (Electrocoin) (set 14)" )
GAME_CUSTOM( 199?, ec_redbr__n,    ec_redbr,   "issa885",                  0x0000, 0x008000, CRC(84032ad0) SHA1(f208a3d49d92e41a8027e8dae639984eeb4f38e1), "Electrocoin","Red Bar (Electrocoin) (set 15)" )
GAME_CUSTOM( 199?, ec_redbr__o,    ec_redbr,   "issa886",                  0x0000, 0x008000, CRC(08b62c24) SHA1(6284e2a60e4a4f134a850a2939ae6fd1c35527eb), "Electrocoin","Red Bar (Electrocoin) (set 16)" )
GAME_CUSTOM( 199?, ec_redbr__p,    ec_redbr,   "issa887",                  0x0000, 0x008000, CRC(71bfe8d0) SHA1(3de358518bc45287544a1473765d25931defec74), "Electrocoin","Red Bar (Electrocoin) (set 17)" )
GAME_CUSTOM( 199?, ec_redbr__q,    ec_redbr,   "issa888",                  0x0000, 0x008000, CRC(2e540dca) SHA1(920d294a06d0f79296c3ea36ffda38ffe6163865), "Electrocoin","Red Bar (Electrocoin) (set 18)" )
GAME_CUSTOM( 199?, ec_redbr__r,    ec_redbr,   "issa889",                  0x0000, 0x008000, CRC(12e36b6c) SHA1(77532657aa292fe921bff568c9cb2e5ca9a9dcc3), "Electrocoin","Red Bar (Electrocoin) (set 19)" )
GAME_CUSTOM( 199?, ec_redbr__s,    ec_redbr,   "issa890",                  0x0000, 0x008000, CRC(95dfeeef) SHA1(e3410ef6e58cd359c0474dfa5cf03c6a41324b5c), "Electrocoin","Red Bar (Electrocoin) (set 20)" )
GAME_CUSTOM( 199?, ec_redbr__t,    ec_redbr,   "issa891",                  0x0000, 0x008000, CRC(c8dbc495) SHA1(2a515df781859a14bf08d127648e021ba6555437), "Electrocoin","Red Bar (Electrocoin) (set 21)" )
GAME_CUSTOM( 199?, ec_redbr__u,    ec_redbr,   "issa892",                  0x0000, 0x008000, CRC(ece4429f) SHA1(b216f3c6a0c93871f24b0c15d1d8318a3cf94ca4), "Electrocoin","Red Bar (Electrocoin) (set 22)" )
GAME_CUSTOM( 199?, ec_redbr__v,    ec_redbr,   "issa893",                  0x0000, 0x008000, CRC(78ff5b1a) SHA1(5c26d234372a84ae02ba9a1b45f28db11beab0b5), "Electrocoin","Red Bar (Electrocoin) (set 23)" )
GAME_CUSTOM( 199?, ec_redbr__w,    ec_redbr,   "issa894",                  0x0000, 0x008000, CRC(d80638d6) SHA1(0df25fd35fbd91df2a3f0e4285a25ae0aad495b6), "Electrocoin","Red Bar (Electrocoin) (set 24)" )
GAME_CUSTOM( 199?, ec_redbr__x,    ec_redbr,   "issa895",                  0x0000, 0x008000, CRC(c6cf5865) SHA1(429a36c79dc415634d1a654fea12af1296b9d079), "Electrocoin","Red Bar (Electrocoin) (set 25)" )
GAME_CUSTOM( 199?, ec_redbr__y,    ec_redbr,   "issa896",                  0x0000, 0x008000, CRC(be073a53) SHA1(454cdb86aefd4cee15eeeb8711368321c2b6847b), "Electrocoin","Red Bar (Electrocoin) (set 26)" )
GAME_CUSTOM( 199?, ec_redbr__z,    ec_redbr,   "issa897",                  0x0000, 0x008000, CRC(1b5dbe15) SHA1(5f22b49ad28aeacdab66166e686051169543c302), "Electrocoin","Red Bar (Electrocoin) (set 27)" )
GAME_CUSTOM( 199?, ec_redbr__0,    ec_redbr,   "issa898",                  0x0000, 0x008000, CRC(d10ffa0f) SHA1(d5355672efc175e9b087d17da15f7de8d605c787), "Electrocoin","Red Bar (Electrocoin) (set 28)" )
GAME_CUSTOM( 199?, ec_redbr__1,    ec_redbr,   "issa899",                  0x0000, 0x008000, CRC(53291642) SHA1(2b18ee9ba5ac3f5dde1424fb8e0b90e5b07c98d8), "Electrocoin","Red Bar (Electrocoin) (set 29)" )
GAME_CUSTOM( 199?, ec_redbr__2,    ec_redbr,   "issa900",                  0x0000, 0x008000, CRC(45d906ea) SHA1(bd6253bbfc33b5c23a4abeb9c458b761a838c3f9), "Electrocoin","Red Bar (Electrocoin) (set 30)" )
GAME_CUSTOM( 199?, ec_redbr__3,    ec_redbr,   "issa901",                  0x0000, 0x008000, CRC(2ad3fad8) SHA1(7fb431744077923275be27af2244f6e58e3a7d8c), "Electrocoin","Red Bar (Electrocoin) (set 31)" )
GAME_CUSTOM( 199?, ec_redbr__4,    ec_redbr,   "issa902",                  0x0000, 0x008000, CRC(e8960b96) SHA1(2849f8a6da2499c5a9d1138ec1a4187e7105f4d4), "Electrocoin","Red Bar (Electrocoin) (set 32)" )
GAME_CUSTOM( 199?, ec_redbr__5,    ec_redbr,   "issa903",                  0x0000, 0x008000, CRC(68963097) SHA1(a7d859306d46b69893d76657fd13eee9f24f31dd), "Electrocoin","Red Bar (Electrocoin) (set 33)" )
GAME_CUSTOM( 199?, ec_redbr__6,    ec_redbr,   "issa904",                  0x0000, 0x008000, CRC(4b02cb77) SHA1(a221fa9eb17e55b9802d9080694a9df3985da238), "Electrocoin","Red Bar (Electrocoin) (set 34)" )
GAME_CUSTOM( 199?, ec_redbr__7,    ec_redbr,   "issa905",                  0x0000, 0x008000, CRC(ad3c4b1d) SHA1(fc44eb8af0ecc43e49df7a3bec867af7558300ab), "Electrocoin","Red Bar (Electrocoin) (set 35)" )
GAME_CUSTOM( 199?, ec_redbr__8,    ec_redbr,   "issa906",                  0x0000, 0x008000, CRC(04b8defd) SHA1(11dc07778cf8fb78ceded78370c4638c446898b7), "Electrocoin","Red Bar (Electrocoin) (set 36)" )
GAME_CUSTOM( 199?, ec_redbr__9,    ec_redbr,   "issa907",                  0x0000, 0x008000, CRC(2e8d3d19) SHA1(c60198aced2d78a5d3e24f56190ba70d51bb9f5a), "Electrocoin","Red Bar (Electrocoin) (set 37)" )
GAME_CUSTOM( 199?, ec_redbr__aa,   ec_redbr,   "issa908",                  0x0000, 0x008000, CRC(3bac5c64) SHA1(0a838c29e7df2a635d61dc41ba07831876c56e4f), "Electrocoin","Red Bar (Electrocoin) (set 38)" )
GAME_CUSTOM( 199?, ec_redbr__ab,   ec_redbr,   "issa909",                  0x0000, 0x008000, CRC(195c27a3) SHA1(a04b0ad0a34549a10c8ffd5ed1002073f3cc1df7), "Electrocoin","Red Bar (Electrocoin) (set 39)" )
GAME_CUSTOM( 199?, ec_redbr__ac,   ec_redbr,   "issa910",                  0x0000, 0x008000, CRC(47a6064b) SHA1(7862fab231aab326e09a6c94369c2632869f0ca4), "Electrocoin","Red Bar (Electrocoin) (set 40)" )
GAME_CUSTOM( 199?, ec_redbr__ad,   ec_redbr,   "issa911",                  0x0000, 0x008000, CRC(f51e04c6) SHA1(894598a099f4074c30daa3edbba3ac79ccec06bf), "Electrocoin","Red Bar (Electrocoin) (set 41)" )
GAME_CUSTOM( 199?, ec_redbr__ae,   ec_redbr,   "issa912",                  0x0000, 0x008000, CRC(39e16f1a) SHA1(d579a11e048addee45391f95b13686ebd7634e18), "Electrocoin","Red Bar (Electrocoin) (set 42)" )
GAME_CUSTOM( 199?, ec_redbr__af,   ec_redbr,   "issa913",                  0x0000, 0x008000, CRC(eee9e4aa) SHA1(b35e6a520c5fd87b0d5aa831feb84efc99b0536c), "Electrocoin","Red Bar (Electrocoin) (set 43)" )
GAME_CUSTOM( 199?, ec_redbr__ag,   ec_redbr,   "issa914",                  0x0000, 0x008000, CRC(0172f29d) SHA1(5ad731226cc5fca9c7a098bbf457d0feffa4444c), "Electrocoin","Red Bar (Electrocoin) (set 44)" )
GAME_CUSTOM( 199?, ec_redbr__ah,   ec_redbr,   "issa915",                  0x0000, 0x008000, CRC(9c0c74cc) SHA1(c109f119c7374d767ec79f2eca1c8e611c35c562), "Electrocoin","Red Bar (Electrocoin) (set 45)" )
GAME_CUSTOM( 199?, ec_redbr__ai,   ec_redbr,   "issa916",                  0x0000, 0x008000, CRC(8109abbb) SHA1(d62babfda5c08b566741c9e4a3680d5d28f3d9a3), "Electrocoin","Red Bar (Electrocoin) (set 46)" )
GAME_CUSTOM( 199?, ec_redbr__aj,   ec_redbr,   "issa917",                  0x0000, 0x008000, CRC(59030083) SHA1(1c7fed8717b9f291ed171f4d948470c183e25474), "Electrocoin","Red Bar (Electrocoin) (set 47)" )
GAME_CUSTOM( 199?, ec_redbr__ak,   ec_redbr,   "issa918",                  0x0000, 0x008000, CRC(a8b59637) SHA1(3a5b372643212fe478c0f83828c27a46ed923a87), "Electrocoin","Red Bar (Electrocoin) (set 48)" )
GAME_CUSTOM( 199?, ec_redbr__al,   ec_redbr,   "issa919",                  0x0000, 0x008000, CRC(6d8e60fe) SHA1(af3a09438176ddfce74b4d377268b9946d5c96cc), "Electrocoin","Red Bar (Electrocoin) (set 49)" )
GAME_CUSTOM( 199?, ec_redbr__am,   ec_redbr,   "issa920",                  0x0000, 0x008000, CRC(d9a6839d) SHA1(46ab0b6262f66d07f3cb3eaad2ecced7a2dc418d), "Electrocoin","Red Bar (Electrocoin) (set 50)" )
GAME_CUSTOM( 199?, ec_redbr__an,   ec_redbr,   "issa921",                  0x0000, 0x008000, CRC(58aa9d2a) SHA1(bab8e920cfd939913722d8a7b502624a7a67db71), "Electrocoin","Red Bar (Electrocoin) (set 51)" )
GAME_CUSTOM( 199?, ec_redbr__ao,   ec_redbr,   "issa922",                  0x0000, 0x008000, CRC(accf4ce0) SHA1(92830ece912145d01bb7c177ce8d0686dbb1bea1), "Electrocoin","Red Bar (Electrocoin) (set 52)" )
GAME_CUSTOM( 199?, ec_redbr__ap,   ec_redbr,   "issa923",                  0x0000, 0x008000, CRC(8f4328e9) SHA1(a0dd8a1ec643d8f5d98492f6fd9b7d2f08ecd64c), "Electrocoin","Red Bar (Electrocoin) (set 53)" )
GAME_CUSTOM( 199?, ec_redbr__aq,   ec_redbr,   "issa924",                  0x0000, 0x008000, CRC(f238edb7) SHA1(b4bf3c1831928b9f6bed9ca7b8cefea8c132eb17), "Electrocoin","Red Bar (Electrocoin) (set 54)" )
GAME_CUSTOM( 199?, ec_redbr__ar,   ec_redbr,   "issa925",                  0x0000, 0x008000, CRC(0179d257) SHA1(bcee53ab7430274539362d30ddf8f351783c10ad), "Electrocoin","Red Bar (Electrocoin) (set 55)" )
GAME_CUSTOM( 199?, ec_redbr__as,   ec_redbr,   "issa926",                  0x0000, 0x008000, CRC(eea16728) SHA1(b9aa46a2adbed7ac8b155f0de569ab2e2ac21b84), "Electrocoin","Red Bar (Electrocoin) (set 56)" )
GAME_CUSTOM( 199?, ec_redbr__at,   ec_redbr,   "issa927",                  0x0000, 0x008000, CRC(ce53791c) SHA1(daa0704f6a98404dc50d3a2f659a08075842c6fb), "Electrocoin","Red Bar (Electrocoin) (set 57)" )
GAME_CUSTOM( 199?, ec_redbr__au,   ec_redbr,   "issa928",                  0x0000, 0x008000, CRC(67201399) SHA1(7f014000a070173b90f54ab7639502fb2c594167), "Electrocoin","Red Bar (Electrocoin) (set 58)" )
GAME_CUSTOM( 199?, ec_redbr__av,   ec_redbr,   "issa929",                  0x0000, 0x008000, CRC(0fe2dfb5) SHA1(78aeeca13e3fc32afb1317d59be4e4eade022cc0), "Electrocoin","Red Bar (Electrocoin) (set 59)" )
GAME_CUSTOM( 199?, ec_redbr__aw,   ec_redbr,   "issa930",                  0x0000, 0x008000, CRC(b24d4e38) SHA1(36eb0415b19e0abaa7eab45c1121c0757509e4eb), "Electrocoin","Red Bar (Electrocoin) (set 60)" )
GAME_CUSTOM( 199?, ec_redbr__ax,   ec_redbr,   "issa931",                  0x0000, 0x008000, CRC(5047eb2d) SHA1(dbd57dbf6a0ca6f2f532811700dfa8f5e2d96810), "Electrocoin","Red Bar (Electrocoin) (set 61)" )
GAME_CUSTOM( 199?, ec_redbr__ay,   ec_redbr,   "issa932",                  0x0000, 0x008000, CRC(5577e4b6) SHA1(85ce8761d7ea37b5e5bca213a655e40af98594e1), "Electrocoin","Red Bar (Electrocoin) (set 62)" )
// Header 'REDBAR' - From 'Super Red Bar' set
GAME_CUSTOM( 199?, ec_redbr__az,   ec_redbr,   "iss3037.rom",              0x0000, 0x008000, CRC(b1984539) SHA1(b8ff3690e47d10ef2d15ccc9198715a83d75a428), "Electrocoin","Red Bar (Electrocoin) (set 63)" )
GAME_CUSTOM( 199?, ec_redbr__a0,   ec_redbr,   "iss3038.rom",              0x0000, 0x008000, CRC(890109fe) SHA1(69c47284497ad3488cff8f36b1ec615bf043fc5f), "Electrocoin","Red Bar (Electrocoin) (set 64)" )
GAME_CUSTOM( 199?, ec_redbr__a1,   ec_redbr,   "iss3039.rom",              0x0000, 0x008000, CRC(25179e39) SHA1(6dd59e5eb3bb769d0018ae2691422108260a2c87), "Electrocoin","Red Bar (Electrocoin) (set 65)" )
GAME_CUSTOM( 199?, ec_redbr__a2,   ec_redbr,   "iss3041.rom",              0x0000, 0x008000, CRC(eefe3086) SHA1(971e71026b8d519fa424180ecf029b6ba9abf5c2), "Electrocoin","Red Bar (Electrocoin) (set 66)" )
GAME_CUSTOM( 199?, ec_redbr__a3,   ec_redbr,   "iss3042.rom",              0x0000, 0x008000, CRC(e5f69b11) SHA1(8de839b74dd3eea85b956ae0cb1d535926ce9489), "Electrocoin","Red Bar (Electrocoin) (set 67)" )
GAME_CUSTOM( 199?, ec_redbr__a4,   ec_redbr,   "iss3043.rom",              0x0000, 0x008000, CRC(276fa423) SHA1(a69962ad6fa38d45b36da6bcff95f69f7175fff0), "Electrocoin","Red Bar (Electrocoin) (set 68)" )
GAME_CUSTOM( 199?, ec_redbr__a5,   ec_redbr,   "iss3044.rom",              0x0000, 0x008000, CRC(9ce127a6) SHA1(2f223b37d0c6aca27b001c0ec81e413ff04dca86), "Electrocoin","Red Bar (Electrocoin) (set 69)" )
GAME_CUSTOM( 199?, ec_redbr__a6,   ec_redbr,   "iss3045.rom",              0x0000, 0x008000, CRC(a79742a3) SHA1(42950e9e61bdf134753cd3fdc6e65446586530fd), "Electrocoin","Red Bar (Electrocoin) (set 70)" )
GAME_CUSTOM( 199?, ec_redbr__a7,   ec_redbr,   "iss3047.rom",              0x0000, 0x008000, CRC(6f9defbe) SHA1(52c9791225373f109f63d5476a5b19aaeceb5058), "Electrocoin","Red Bar (Electrocoin) (set 71)" )
GAME_CUSTOM( 199?, ec_redbr__a8,   ec_redbr,   "iss3048.rom",              0x0000, 0x008000, CRC(f26fcfe5) SHA1(0176366fb46d897a5e106611da885065655df576), "Electrocoin","Red Bar (Electrocoin) (set 72)" )
GAME_CUSTOM( 199?, ec_redbr__a9,   ec_redbr,   "iss3256.rom",              0x0000, 0x008000, CRC(e9909913) SHA1(b53466238b8e39a45cdbc09dd18e19aab9044027), "Electrocoin","Red Bar (Electrocoin) (set 73)" )
GAME_CUSTOM( 199?, ec_redbr__ba,   ec_redbr,   "iss3257.rom",              0x0000, 0x008000, CRC(27837c49) SHA1(4408a2066ae427b6f66b2d2be3928d85213c3dcf), "Electrocoin","Red Bar (Electrocoin) (set 74)" )
GAME_CUSTOM( 199?, ec_redbr__bb,   ec_redbr,   "iss3258.rom",              0x0000, 0x008000, CRC(5a8214b9) SHA1(40cf50468157020ffd52f69308210cb93d94e6ab), "Electrocoin","Red Bar (Electrocoin) (set 75)" )
GAME_CUSTOM( 199?, ec_redbr__bc,   ec_redbr,   "iss3259.rom",              0x0000, 0x008000, CRC(170a2827) SHA1(2d5f9991468e999c3874f04ef0396abc18c5de1d), "Electrocoin","Red Bar (Electrocoin) (set 76)" )
GAME_CUSTOM( 199?, ec_redbr__bd,   ec_redbr,   "iss3260.rom",              0x0000, 0x008000, CRC(5f27fa81) SHA1(27d4463211f824abfb3a09270b38ddb68da75691), "Electrocoin","Red Bar (Electrocoin) (set 77)" )
GAME_CUSTOM( 199?, ec_redbr__be,   ec_redbr,   "iss3261.rom",              0x0000, 0x008000, CRC(38dbbb65) SHA1(184dc9257db5cbf255fc997547be72c27ad9179b), "Electrocoin","Red Bar (Electrocoin) (set 78)" )
GAME_CUSTOM( 199?, ec_redbr__bf,   ec_redbr,   "iss3262.rom",              0x0000, 0x008000, CRC(a51a240a) SHA1(fce1a96b15726bd08acd487c61776f7f805880c6), "Electrocoin","Red Bar (Electrocoin) (set 79)" )
GAME_CUSTOM( 199?, ec_redbr__bg,   ec_redbr,   "iss3263.rom",              0x0000, 0x008000, CRC(e672b24d) SHA1(f33b750b2ede7d107684cea05903266ae98d8203), "Electrocoin","Red Bar (Electrocoin) (set 80)" )
GAME_CUSTOM( 199?, ec_redbr__bh,   ec_redbr,   "iss3264.rom",              0x0000, 0x008000, CRC(a6c6efb7) SHA1(9ea95ee91745008edd1bed3c83e40325d92d6fb0), "Electrocoin","Red Bar (Electrocoin) (set 81)" )
GAME_CUSTOM( 199?, ec_redbr__bi,   ec_redbr,   "iss3265.rom",              0x0000, 0x008000, CRC(10948d89) SHA1(bb503c895777dee197ad8fba49c3b52a5380a06e), "Electrocoin","Red Bar (Electrocoin) (set 82)" )
GAME_CUSTOM( 199?, ec_redbr__bj,   ec_redbr,   "iss3266.rom",              0x0000, 0x008000, CRC(8228a9bf) SHA1(42f44bc7708703905f55143107395c7c10d4e150), "Electrocoin","Red Bar (Electrocoin) (set 83)" )
GAME_CUSTOM( 199?, ec_redbr__bk,   ec_redbr,   "iss3267.rom",              0x0000, 0x008000, CRC(5398a151) SHA1(e1d37141707c703b5f6c13fd839bfd3c2da632a3), "Electrocoin","Red Bar (Electrocoin) (set 84)" )
GAME_CUSTOM( 199?, ec_redbr__bl,   ec_redbr,   "srb58.bin",                0x0000, 0x008000, CRC(b2855bc7) SHA1(c9bc47250077050fb689fc552abc0f60c2acb8ea), "Electrocoin","Red Bar (Electrocoin) (set 85)" )
GAME_CUSTOM( 199?, ec_redbr__bm,   ec_redbr,   "srb_10cash_ver153.bin",    0x0000, 0x008000, CRC(96f966f9) SHA1(82a87f8eb4914ed7fcc90751b119f72dda29532e), "Electrocoin","Red Bar (Electrocoin) (set 86)" )
// These are 'REDBAR' they use the same header format as ec_big7
GAME_CUSTOM( 199?, ec_redbr__bn,   ec_redbr,   "iss2021.rom",              0x0000, 0x008000, CRC(71fffd80) SHA1(49cc502e54e135bb131b8ac096619df9f1f29055), "Electrocoin","Red Bar (Electrocoin) (set 87)" )
GAME_CUSTOM( 199?, ec_redbr__bo,   ec_redbr,   "iss3040.bin",              0x0000, 0x008000, CRC(530c52a8) SHA1(65cc627baadd6385c314a4477475c69c1b213a5d), "Electrocoin","Red Bar (Electrocoin) (set 88)" )
GAME_CUSTOM( 199?, ec_redbr__bp,   ec_redbr,   "iss3046.rom",              0x0000, 0x008000, CRC(14109012) SHA1(d008488216d8e9c0dbe6d1c07d59b84637a8f41c), "Electrocoin","Red Bar (Electrocoin) (set 89)" )
GAME_CUSTOM( 199?, ec_redbr__bq,   ec_redbr,   "iss9013.rom",              0x0000, 0x008000, CRC(d18d50b2) SHA1(7c471a15f33d22d8d1eb4971c8e3d2c360ec8db9), "Electrocoin","Red Bar (Electrocoin) (set 90)" )
// These are 'REDBAR'  Header area is all 0x00
GAME_CUSTOM( 199?, ec_redbr__br,   ec_redbr,   "iss9409.rom",              0x0000, 0x008000, CRC(d35db982) SHA1(6f171e133a932c94843b6d03431bf6a3befaae86), "Electrocoin","Red Bar (Electrocoin) (set 91)" ) // from 'Casino Red Bar' set
GAME_CUSTOM( 199?, ec_redbr__bs,   ec_redbr,   "iss9403.rom",              0x0000, 0x008000, CRC(b82c3ce7) SHA1(a13d9ea7dd6dd5172240dc51ccdfb8dabdc8f5b2), "Electrocoin","Red Bar (Electrocoin) (set 92)" )
GAME_CUSTOM( 199?, ec_redbr__bt,   ec_redbr,   "iss9407.rom",              0x0000, 0x008000, CRC(e48992cf) SHA1(5d8dcf7be0d1f86ad795b2722f62009641d92528), "Electrocoin","Red Bar (Electrocoin) (set 93)" )
GAME_CUSTOM( 199?, ec_redbr__bu,   ec_redbr,   "iss9410.rom",              0x0000, 0x008000, CRC(3711d488) SHA1(2455bc5635d9d318e0b2716547405e18a2d71bbe), "Electrocoin","Red Bar (Electrocoin) (set 94)" )
GAME_CUSTOM( 199?, ec_redbr__bv,   ec_redbr,   "iss9411.rom",              0x0000, 0x008000, CRC(3ea6f32b) SHA1(e489c6a210f37e9c3c755321bfe979bf2f4898f5), "Electrocoin","Red Bar (Electrocoin) (set 95)" )
GAME_CUSTOM( 199?, ec_redbr__bw,   ec_redbr,   "iss9412.rom",              0x0000, 0x008000, CRC(ddde37fb) SHA1(8a3a61bbe75e2d0e916a31a55fbd03ec38ed0c3e), "Electrocoin","Red Bar (Electrocoin) (set 96)" )
// Header 'REDBAR' - From Super Big 7 sets (some device at a001)
GAME_CUSTOM( 199?, ec_redbr__bx,   ec_redbr,   "sb78ac",                   0x0000, 0x008000, CRC(1eee47a4) SHA1(ebf5a535cddc50299ed07a2c424b4a46f5cf2b27), "Electrocoin","Red Bar (Electrocoin) (set 97)" )
GAME_CUSTOM( 199?, ec_redbr__by,   ec_redbr,   "sbig.710",                 0x0000, 0x008000, CRC(1220cea2) SHA1(97a8f6d1221acc1a6c3f84dd8e14693a40bd8de7), "Electrocoin","Red Bar (Electrocoin) (set 98)" )
GAME_CUSTOM( 199?, ec_redbr__b0,   ec_redbr,   "sbig78d",                  0x0000, 0x008000, CRC(bd5af5f2) SHA1(d1efaf21aad9869f593a9cb3732a7d120f2ff55b), "Electrocoin","Red Bar (Electrocoin) (set 100)" )
GAME_CUSTOM( 199?, ec_redbr__b1,   ec_redbr,   "sbig78t",                  0x0000, 0x008000, CRC(598b2bc3) SHA1(e9bc7dac5328e1973e56a4d8f3929d9cb7c606f1), "Electrocoin","Red Bar (Electrocoin) (set 101)" )



#define EC_BXD7S_OTHERS \
	ROM_REGION( 0x200000, "sndz80", ROMREGION_ERASE00 ) \
	/* probably the same sound rom */
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		EC_BXD7S_OTHERS \
	ROM_END \
	GAME(year, setname, parent ,ecoinfr ,ecoinfr_barx , ecoinfr_state,ecoinfr ,ROT0,company,title,GAME_FLAGS )

// These are '2006 COOL7'
GAME_CUSTOM( 199?, ec_bxd7s,       0,          "issc193.dat",  0x0000, 0x008000, CRC(2f3fb9e2) SHA1(426f7436c8a22f1d8a05a5ccef6b6b5551441028), "Electrocoin","Bar X Diamond 7s (2006 COOL7) (Electrocoin) (set 1)" )  // P-2S K---
GAME_CUSTOM( 199?, ec_bxd7s__a,    ec_bxd7s,   "issc331",      0x0000, 0x008000, CRC(83c09f9d) SHA1(4ef9bb5ae779309d25bf673d8a59ea8cf65c84ba), "Electrocoin","Bar X Diamond 7s (2006 COOL7) (Electrocoin) (set 2)" )  // --2S K---
GAME_CUSTOM( 199?, ec_bxd7s__b,    ec_bxd7s,   "issc330",      0x0000, 0x008000, CRC(4a8231ff) SHA1(470813fff14eeff3caad2cde710d4d1361231299), "Electrocoin","Bar X Diamond 7s (2006 COOL7) (Electrocoin) (set 3)" ) // -1-- ----
GAME_CUSTOM( 199?, ec_bxd7s__c,    ec_bxd7s,   "issc325.rom",  0x0000, 0x008000, CRC(153f90a2) SHA1(df250a02e6b9c130b5f8856c1fdb9012517d15ce), "Electrocoin","Bar X Diamond 7s (2006 COOL7) (Electrocoin) (set 4)" ) // in an unknown set
GAME_CUSTOM( 199?, ec_bxd7s__d,    ec_bxd7s,   "issc337",      0x0000, 0x008000, CRC(79b791aa) SHA1(ee6257b198b950d31690f1b12b98bdf483216b9d), "Electrocoin","Bar X Diamond 7s (2006 COOL7) (Electrocoin) (set 5)" ) // P-2S K---   in a set marked 'magic bars'





#define EC_CASBX_OTHERS \
	ROM_REGION( 0x200000, "sndz80", ROMREGION_ERASE00 ) \
	/* probably the same sound rom */
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		EC_CASBX_OTHERS \
	ROM_END \
	GAME(year, setname, parent ,ecoinfr ,ecoinfr_barx , ecoinfr_state,ecoinfr ,ROT0,company,title,GAME_FLAGS )


/* (C)1993-97/2002-03 ELECTROCOIN */
// These are '2006 BARX'
GAME_CUSTOM( 199?, ec_casbx,       0,          "issc287",      0x0000, 0x008000, CRC(fe528b9f) SHA1(ac2a7648b9a706de780a059e7f77573be1d6b9cd), "Electrocoin","Casino Bar X (Electrocoin) (set 1)" ) // P--- ----   // ELCPSBRX - Thu Jun 07 11:45:38 2007
GAME_CUSTOM( 199?, ec_casbx__b,    ec_casbx,   "issc289",      0x0000, 0x008000, CRC(31e8ae3a) SHA1(accc14b292f220dfc9695638c0402de28fe19bae), "Electrocoin","Casino Bar X (Electrocoin) (set 3)" ) // P-2S K---   // ELCPSBRX - Thu Jun 07 11:46:21 2007
GAME_CUSTOM( 199?, ec_casbx__a,    ec_casbx,   "issc293",      0x0000, 0x008000, CRC(93c83913) SHA1(b4cfed0836f57d7f6c828273468a89532607cde2), "Electrocoin","Casino Bar X (Electrocoin) (set 2)" ) // P-2S K---   // ELCPSBRX - Thu Jun 07 11:47:48 2007


	// these both have a date string of "Tue Dec 06 15:16:04 2005" and the title of "Bar X 125" along with various other strings for an alpha display
GAME_CUSTOM( 2005, ec_bx125,   0,          "bx125p25",     0x0000, 0x010000, CRC(beff03e1) SHA1(d0bf997f9766a801274a02242755df3419879bd4), "Electrocoin","Bar X 125 (Electrocoin) (set 1)" )
GAME_CUSTOM( 2005, ec_bx125a,  ec_bx125,   "x125n34.bin",  0x0000, 0x010000, CRC(5ab73808) SHA1(f857bd9a9a2f1c7c795a7203f2932acac051ae55), "Electrocoin","Bar X 125 (Electrocoin) (set 2)" )




#define EC_SPBDX_OTHERS \
	ROM_REGION( 0x200000, "sndz80", ROMREGION_ERASE00 ) \
	/* probably the same sound rom */
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		EC_SPBDX_OTHERS \
	ROM_END \
	GAME(year, setname, parent ,ecoinfr ,ecoinfr_barx , ecoinfr_state,ecoinfr ,ROT0,company,title,GAME_FLAGS )

/* No header (space for one, but 0x00 fill) - Electrocoin 1993 copyright near end */
GAME_CUSTOM( 199?, ec_spbdx,       0,          "iss132.rom",   0x0000, 0x008000, CRC(fd2ea535) SHA1(6deda1825bfce9481bf85a500e031242a2c9cf8c), "Electrocoin","Super Bar X Deluxe (Electrocoin) (set 1)" ) // ELCNSBRX - Sat Jun 22 13:28:41 1996
GAME_CUSTOM( 199?, ec_spbdx__a,    ec_spbdx,   "iss133.rom",   0x0000, 0x008000, CRC(9522c295) SHA1(7b9f1c672c15b5e353c19ad5237ffd85f4c83fdb), "Electrocoin","Super Bar X Deluxe (Electrocoin) (set 2)" ) // ELCNSBRX - Sat Jun 22 13:29:06 1996
GAME_CUSTOM( 199?, ec_spbdx__b,    ec_spbdx,   "iss134.rom",   0x0000, 0x008000, CRC(888809a6) SHA1(5e1163ef63616f4934a4894772457b70c5ef4fb2), "Electrocoin","Super Bar X Deluxe (Electrocoin) (set 3)" ) // ELCNSBRX - Sat Jun 22 13:29:28 1996
GAME_CUSTOM( 199?, ec_spbdx__c,    ec_spbdx,   "iss176.rom",   0x0000, 0x008000, CRC(f6d0c2cd) SHA1(dfd5e040f2aa42d2186a0d3c94f692cfc560236a), "Electrocoin","Super Bar X Deluxe (Electrocoin) (set 4)" ) // ELCNSBRX - Thu Mar 16 12:14:40 1995
GAME_CUSTOM( 199?, ec_spbdx__d,    ec_spbdx,   "300615",       0x0000, 0x008000, CRC(8a5a4e35) SHA1(be3acfaf116ae23a61aac581d9f83287cddcdaab), "Electrocoin","Super Bar X Deluxe (Electrocoin) (set 5)" ) // ELCNSBRX - Fri Sep 11 13:01:02 1998 // bad?


#define EC_UNK5_OTHERS \
	ROM_REGION( 0x200000, "sndz80", ROMREGION_ERASE00 ) \
	/* probably the same sound rom */
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		EC_UNK5_OTHERS \
	ROM_END \
	GAME(year, setname, parent ,ecoinfr ,ecoinfr_barx , ecoinfr_state,ecoinfr ,ROT0,company,title,GAME_FLAGS )

// No Header info (all 0x00)
// (C)1993/97 ELECTROCOIN
GAME_CUSTOM( 199?, ec_unk5,     0,          "iss9016.rom",          0x0000, 0x008000, CRC(e8ebafd0) SHA1(fa9fc04d04f9ac2360c66727afaf567527e95e01), "Electrocoin","unknown 'Electrocoin' Fruit Machines (Electrocoin) (set 1)" ) // ELCPCBX1 - Tue May 19 14:27:48 1998
GAME_CUSTOM( 199?, ec_unk5__a,  ec_unk5,    "iss9207.rom",          0x0000, 0x008000, CRC(f646702a) SHA1(9e2e7da0edaecd021861145b6abd1498fc3b563a), "Electrocoin","unknown 'Electrocoin' Fruit Machines (Electrocoin) (set 2)" ) // ELCNCBX1 - Fri Nov 24 16:27:23 2000
GAME_CUSTOM( 199?, ec_unk5__b,  ec_unk5,    "iss9208.rom",          0x0000, 0x008000, CRC(b4c3c98a) SHA1(10aeeca8c7b2923e3768f82c672229898c51062d), "Electrocoin","unknown 'Electrocoin' Fruit Machines (Electrocoin) (set 3)" ) // ELCPCBX1 - Fri Nov 24 16:27:47 2000


// No header.. no space for header
GAME_CUSTOM( 199?, ec_unk5__c,  ec_unk5,    "v1.1non_protocol.hex", 0x0000, 0x02680d, CRC(0b76e2de) SHA1(1bc330558e69b316a26d659463406324f24b5978), "Electrocoin","unknown 'Electrocoin' Fruit Machines (Electrocoin) (set 4)" ) // convert from HEX and check



/********************************************************************************************************************
 ROMs with MAB scramble
********************************************************************************************************************/

// these are scrambled roms using 'MAB' hw.

ROM_START( ec_bar7 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "b7tok.bin", 0x0000, 0x020000, CRC(c3913709) SHA1(73024a3bbfbe13477e4daae78f54c694d112b936) )
ROM_END


ROM_START( ec_barxmab )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "mab-bx15", 0x0000, 0x020000, CRC(a58dba57) SHA1(50131ac706c5b0baa793e79e7a0b42eb28c2c61d) )
	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "barx5p5", 0x0000, 0x020000, CRC(374a83a1) SHA1(6151e6d2e7cdd3997dc009dd4a11a1e8fd405ac5) )
ROM_END

ROM_START( ec_fltr )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "128.bin", 0x0000, 0x020000, CRC(a10a59a8) SHA1(967fc890daccc9cfc880404c37714391a71fe0db) )
	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "226.bin", 0x0000, 0x020000, CRC(96a684e5) SHA1(0b699af7ab3719543ca6f9fa623a2658154ff79d) )
	ROM_LOAD( "f131bx", 0x0000, 0x020000, CRC(80049e42) SHA1(d9d5d8bc115652db40e762df9adca81849475620) )
	ROM_LOAD( "f132bx.bin", 0x0000, 0x020000, CRC(cbb76052) SHA1(f0888c7466dde33e9831d4e0130d49281ba2e7d0) )
	ROM_LOAD( "f312.bin", 0x0000, 0x020000, CRC(dc3b60f0) SHA1(c76a59fb0996a5e1e014120a6efc756a96e1a976) )
	ROM_LOAD( "f313bx.bin", 0x0000, 0x020000, CRC(6e957245) SHA1(a50d5f27fcadfdb3d0806b3ef5210422b16403ad) )
	ROM_LOAD( "f314.bin", 0x0000, 0x020000, CRC(ea42b72d) SHA1(2dce355419f0a9118700034ae17c33afaf5a8b8a) )
	ROM_LOAD( "f513b7.bin", 0x0000, 0x020000, CRC(691da0ef) SHA1(c7b3fdfed2008fd7e12934a86ea87a1025bbe7e0) )
	ROM_LOAD( "f514.bin", 0x0000, 0x020000, CRC(fd77043f) SHA1(77e7e1b8d0cf90ebdad32e838002e273edcfb7eb) )
	ROM_LOAD( "fb7228.bin", 0x0000, 0x020000, CRC(325abb4d) SHA1(ba0f4fca37e187724dd79f37e4128daa5a429921) )
	ROM_LOAD( "fb7229.bin", 0x0000, 0x020000, CRC(3ae1e092) SHA1(840bcba60be4a102876d978d5b95ef44153f10a7) )
	ROM_LOAD( "v133.bin", 0x0000, 0x020000, CRC(474f4fdc) SHA1(e86bcecfde72355b4a41d32532f4c7f439131e80) )
	ROM_LOAD( "v230.bin", 0x0000, 0x020000, CRC(63954009) SHA1(0697de05ad8cbb10f1c93a005b160109e0420e8d) )
	ROM_LOAD( "v512.bin", 0x0000, 0x020000, CRC(c2f4f566) SHA1(403a9a3d20c6f6d7e7e89f14e67f502200149e88) )
ROM_END

ROM_START( ec_supbxmab )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// use the MAB scramble
	ROM_LOAD( "sbx1.3v", 0x0000, 0x020000, CRC(375795fb) SHA1(3dbc95aba850ef3e307e6b4c6d58a40a1e8ee8f1) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "sbx1.6", 0x0000, 0x020000, CRC(e8cfb340) SHA1(d37f0a72c7b59836c5abec8b58066ff4bbd85723) )
	ROM_LOAD( "sbx1.9", 0x0000, 0x020000, CRC(521098a1) SHA1(b8e5a05b085015c7b3b5964471a5ee784a3362d7) )

	ROM_LOAD( "s_barxup", 0x0000, 0x020000, CRC(b373f5f1) SHA1(1746557bafa5203dfdf702b92f3640516d0d510d) )
	ROM_LOAD( "sbxs1v2", 0x0000, 0x020000, CRC(0f7a4a25) SHA1(b0b34e1687b0f4352f2de125be900c6687ccf27a) )
	ROM_LOAD( "sbxs2v3", 0x0000, 0x020000, CRC(9e541763) SHA1(4f42ecf1051c396ba349e57986d26f03a73e7d7a) )
	// not so sure about this one!
	ROM_LOAD( "s_bar_x._pound25", 0x0000, 0x040000, CRC(25885575) SHA1(ea9321eae615b2d11b6801c30ac4c7c8fc90a44f) )


	ROM_REGION( 0x200000, "snd", 0 )
	ROM_LOAD( "sbxsnd", 0x0000, 0x002000, CRC(2ab21d4d) SHA1(1865cbfe603397a3617bed158b39ba607c27226a) )
ROM_END

ROM_START( ec_spbg7mab )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// MAB Scramble
	ROM_LOAD( "sb73.8", 0x0000, 0x020000, CRC(01353cd1) SHA1(5f603280096ce5ad3f7bbe5548deb2452a7168e7) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "sb73.9", 0x0000, 0x020000, CRC(f253f61d) SHA1(28bd8628681fef865e984b9284a5e445b3b0cce7) )
ROM_END

ROM_START( ec_casbxcon )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// MAB Scramble
	ROM_LOAD( "v301.bin", 0x0000, 0x020000, CRC(706cef3c) SHA1(7aeb2885872493569cdb99cbe723c9318fc246ee) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "v302.bin", 0x0000, 0x020000, CRC(d3b5f24c) SHA1(c2f80fb8143fa9f45b38357d913593c8de3fa2da) )
	ROM_LOAD( "v303.bin", 0x0000, 0x020000, CRC(008916e6) SHA1(144b3f02e5bfd4ed233d6ceae5a5662cfc66367f) )
	ROM_LOAD( "v305.bin", 0x0000, 0x020000, CRC(f08f0ffa) SHA1(6a84442359f76087ac3fc2470438b9ef5e135cb6) )
	ROM_LOAD( "v306.bin", 0x0000, 0x020000, CRC(dc9f96c6) SHA1(9d501ca3d5bde787a5271c198ed1cade4044275a) )
	ROM_LOAD( "v307.bin", 0x0000, 0x020000, CRC(73c18cec) SHA1(65d89c03253d9251187cc6ca90841ebdcd35b604) )
	ROM_LOAD( "v308.bin", 0x0000, 0x020000, CRC(e49860c3) SHA1(b0967e81fb16c53ff2caf10abf9ded546f18aae7) )
ROM_END

ROM_START( ec_multb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// MAB Scramble
	ROM_LOAD( "v214.bin", 0x0000, 0x020000, CRC(5bf0a838) SHA1(7e177444cc36f0ef6c6cb2be77978e92561690b0) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "v214.bin", 0x0000, 0x020000, CRC(5bf0a838) SHA1(7e177444cc36f0ef6c6cb2be77978e92561690b0) )
	ROM_LOAD( "v217.bin", 0x0000, 0x020000, CRC(7b60f6b7) SHA1(c42aa42a5e6b7a193f842317d685a13395520533) )
	ROM_LOAD( "v218.bin", 0x0000, 0x020000, CRC(1d5de902) SHA1(24407dd00df6e95ab1ee71473e2e3bef6d0657c6) )
	ROM_LOAD( "v219.bin", 0x0000, 0x020000, CRC(13e92639) SHA1(d71e47639bffc8598c2549b78de6c8a44a691614) )
	ROM_LOAD( "v220.bin", 0x0000, 0x020000, CRC(f9987122) SHA1(1e3c900f44b9e1d1f934e2fb17a892551b2d7413) )
	ROM_LOAD( "v221.bin", 0x0000, 0x020000, CRC(f52ffe1a) SHA1(afdbab93d4428e6eab83177e3e181fad9afc83a8) )
	ROM_LOAD( "v222.bin", 0x0000, 0x020000, CRC(5603554a) SHA1(e2ae5e81ee7d0bc3e7b1fcf150c250fa827bcdee) )
	ROM_LOAD( "v229.bin", 0x0000, 0x020000, CRC(3ae1e092) SHA1(840bcba60be4a102876d978d5b95ef44153f10a7) )
ROM_END

ROM_START( ec_casmb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// MAB Scramble
	ROM_LOAD( "v501.bin", 0x0000, 0x020000, CRC(4cbe1fa7) SHA1(a7dfa2f193651c670102159560c2ab188ee1f006) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "v502.bin", 0x0000, 0x020000, CRC(2bc42175) SHA1(d9a154071247809ca6ca37ee1abb00883106a2ca) )
	ROM_LOAD( "v505.bin", 0x0000, 0x020000, CRC(2bc42175) SHA1(d9a154071247809ca6ca37ee1abb00883106a2ca) )
	ROM_LOAD( "v506.bin", 0x0000, 0x020000, CRC(e3232f70) SHA1(cb21a777d0f28ed80e00f2e6b310193000eef8a2) )
	ROM_LOAD( "v507.bin", 0x0000, 0x020000, CRC(9aaf9dd6) SHA1(a98eae02f3b791a1db434077915c590e4614913e) )
	ROM_LOAD( "v508.bin", 0x0000, 0x020000, CRC(de9dc5f4) SHA1(7b7fb6ffbac2d6e074d186f0b053e6e9f955139c) )
ROM_END

ROM_START( ec_stkex )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// MAB Scramble
	ROM_LOAD( "stakex.bin", 0x0000, 0x020000, CRC(01fe1d8f) SHA1(540d23e33f7930237c91f4cd9c9938e4d203f5d4) )
ROM_END


ROM_START( ec_supmb )
	//Slightly different ROM mapping
	ROM_REGION( 0x200000, "maincpu", 0 )
	// MAB Scramble
	ROM_LOAD( "smb1v6", 0x0000, 0x020000, CRC(e0e9afa7) SHA1(0b0f47e5d9b5b538f23f37e4b279343308a87b94) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "smb1v7", 0x0000, 0x020000, CRC(76ec2100) SHA1(ac484b3235149dfd2561d78d5917c02ccd9be413) )
	ROM_LOAD( "smb2v03.dat", 0x0000, 0x020000, CRC(e3a0ef84) SHA1(d979c03c11ea9252424e28da1ba4e8a44643ba52) )
	ROM_LOAD( "smb2v16", 0x0000, 0x020000, CRC(49769761) SHA1(80dab199e69a43508347989654859be6ce2aaced) )
	ROM_LOAD( "v116.bin", 0x0000, 0x020000, CRC(8da57cbc) SHA1(48f78e226c90bf51cce70ffd9251bcd9275ea091) )
ROM_END

ROM_START( ec_gold7 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// MAB Scramble
	ROM_LOAD( "gsb.bin", 0x0000, 0x010000, CRC(3cd64b27) SHA1(af96dca68843afc4e0f55b238c32ae910bc40cf5) )
	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "g7205.bin", 0x0000, 0x020000, CRC(59f33b96) SHA1(c5f5095e9ce14aedc6cb7048ab282523d3fed7f3) )
	ROM_LOAD( "g7207.bin", 0x0000, 0x020000, CRC(b8f60d63) SHA1(69bbcedebd14e56372d3e7d27dd1590de6c046d9) )
	ROM_REGION( 0x200000, "altsrec", 0 )
	ROM_LOAD( "fg73v2.bin", 0x0000, 0x040000, CRC(d3b9bf29) SHA1(9a30e80cdec46bd57c0e4d86e5d9dcf4063f58ab) )
	ROM_LOAD( "fg73v3.bin", 0x0000, 0x040000, CRC(8c36652e) SHA1(2a0fa9826486b81ee710977f2889e05ac93fd86e) )
	ROM_LOAD( "fg73v4.bin", 0x0000, 0x040000, CRC(d1c76f30) SHA1(51b58cdd285f7e50e0ad4e4e62809ad53755b453) )
	ROM_LOAD( "fg73v5.bin", 0x0000, 0x040000, CRC(6043cda5) SHA1(9b726acd37239a9567b00321a9ebcc050300b59f) )
	ROM_LOAD( "g72v2.bin",  0x0000, 0x040000, CRC(7ba8335b) SHA1(18f1e2d30dc832cbd9db0eac336d779861822e95) )
	ROM_LOAD( "g7show.bin", 0x0000, 0x040000, CRC(fd6c299d) SHA1(85fd47e223d0119710ebdfd74de7b5682a3bb160) )
ROM_END


ROM_START( ec_mgbel )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// MAB Scramble
	ROM_LOAD( "mabx.bin", 0x0000, 0x010000, CRC(c504f7ca) SHA1(809e1ce7ed27a60bf430486018e3d05d57d4ae76) )
	ROM_REGION( 0x200000, "altsrec", 0 )
	ROM_LOAD( "code.bin",     0x0000, 0x040000, CRC(566a9b0a) SHA1(893f40610736ba0853f1e482f48622a99d01952f) )
	ROM_LOAD( "mb1v0a.bin",   0x0000, 0x040000, CRC(eb71e687) SHA1(43fde579d4e5695c2a11b3e33b001fe98c0f172e) )
	ROM_LOAD( "mb1v0b.bin",   0x0000, 0x040000, CRC(6a49a223) SHA1(9696b1ad98bc75770a1fd9c09683750618bb01f8) )
	ROM_LOAD( "mb1v0c.bin",   0x0000, 0x040000, CRC(d6a48709) SHA1(6bc17d3bba5f47104d834f4e6902b056054a1b44) )
	ROM_LOAD( "mb1v0d.bin",   0x0000, 0x040000, CRC(09f663d8) SHA1(b9f1b846a1fec4aa77581a276fb8142734ae8cd1) )
	ROM_LOAD( "mb1v1.bin",    0x0000, 0x040000, CRC(12e8a0ee) SHA1(6ab6d8f28559af07b42059fb3901d542a437caa6) )
	ROM_LOAD( "mb1v1a.bin",   0x0000, 0x040000, CRC(dec8a50a) SHA1(3ad07bb3d14b40117b7b9a0cbca1a60778ee8c0b) )
	ROM_LOAD( "mb1v1c.bin",   0x0000, 0x040000, CRC(b2d5e21b) SHA1(d5de15e643147167a7e54ebe2352dbe43b2189aa) )
	ROM_LOAD( "mb1v2.bin",    0x0000, 0x040000, CRC(ed58b2d1) SHA1(0d4c468160ea37a8e19ccf7566bb7bf24635bcc8) )
	ROM_LOAD( "mb1v4.bin",    0x0000, 0x040000, CRC(22720e28) SHA1(b33bcfcd1b9b805c81af1a12e0d819c528158c28) )
	ROM_LOAD( "mb1v5.bin",    0x0000, 0x040000, CRC(3d6b1d4b) SHA1(d733d53a02814dbdd664e3f462f5693f23355190) )
	ROM_LOAD( "mb1v6.bin",    0x0000, 0x040000, CRC(4056ee56) SHA1(89c78800c11dd14536606a5a879a61b4676ec6db) )
	ROM_LOAD( "mb1v7.bin",    0x0000, 0x040000, CRC(ac86d06d) SHA1(fa56fa6bf7d91a56a6906e3efe65d5c9a997d856) )
	ROM_LOAD( "mb1v8.bin",    0x0000, 0x040000, CRC(4d3d027b) SHA1(ab6d3df9273acd1378b2a6d371012c6501fce0bc) )
	ROM_LOAD( "mb1v8p.bin",   0x0000, 0x040000, CRC(80d2695a) SHA1(49cbe863942b3b7783e2d86407ce1a036c1577ed) )
	ROM_LOAD( "mb1v9.bin",    0x0000, 0x040000, CRC(3ed3fcd7) SHA1(a611a6c8b238dd83228f3c1e5627bb68c99b6952) )
	ROM_LOAD( "mb2v0.bin",    0x0000, 0x040000, CRC(9ab7e40d) SHA1(69064b549fd557fec32f6346d698869756734537) )
	ROM_LOAD( "mb2v1.bin",    0x0000, 0x040000, CRC(247a5dcc) SHA1(f634f610e7685676840465b9e2786f1637ff8270) )
	ROM_LOAD( "mbell1v0.bin", 0x0000, 0x040000, CRC(faa64e27) SHA1(53c7f2c96385aafd313e046c9124f8e195de2c4f) )
	ROM_LOAD( "tmb2v3.bin",   0x0000, 0x040000, CRC(87074e94) SHA1(f145ffd450914111e8e6f44bd857f16779f0bdce) )
	ROM_LOAD( "tmb2v3p.bin",  0x0000, 0x040000, CRC(4f0311e6) SHA1(225526b04c46b5eca1f8379806df4d5cd95f7eb6) )
ROM_END

ROM_START( ec_supbxcon )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// MAB Scramble
	ROM_LOAD( "sbsx.bin", 0x0000, 0x010000, CRC(fcbbd3ff) SHA1(ceb95c945f73b008a2d9dc3b48971d3f4721c88b) )
	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "bxtok.bin", 0x0000, 0x020000, CRC(6bafbfa8) SHA1(8f36c0ff6ba0327f9ed51fc46a98959fdd2a41ac) )
	ROM_LOAD( "scb1v11", 0x0000, 0x020000, CRC(264a7608) SHA1(0c94675ac8f6609dbb1ae8d4915fbd071cb9728b) )
	ROM_LOAD( "scb1v12", 0x0000, 0x020000, CRC(1c995bc5) SHA1(bbb69f6c90b5f25832ad14f3066c732dbeba978c) )
	ROM_LOAD( "v113.bin", 0x0000, 0x020000, CRC(b373f5f1) SHA1(1746557bafa5203dfdf702b92f3640516d0d510d) )
	ROM_LOAD( "v114.bin", 0x0000, 0x020000, CRC(daef56d8) SHA1(e4f2b0edccd45b7e3876f57831d83ff5d60f0c41) )
	ROM_LOAD( "v115.bin", 0x0000, 0x020000, CRC(3ef3d1a8) SHA1(8d7aee58c0477b8252e12b2fadb2da6b1ecef300) )
	ROM_LOAD( "v116.bin", 0x0000, 0x020000, CRC(8da57cbc) SHA1(48f78e226c90bf51cce70ffd9251bcd9275ea091) )
	ROM_LOAD( "v117.bin", 0x0000, 0x020000, CRC(3d45fd91) SHA1(69ed98b9b5c0493b938c1249f0aa22f326b845d1) )
	ROM_LOAD( "v118.bin", 0x0000, 0x020000, CRC(fca522ed) SHA1(007f627f06ef511595007939fa57b861ec01659e) )
	ROM_LOAD( "v119.bin", 0x0000, 0x020000, CRC(08d50628) SHA1(ae5a8bc6b548127c1a433886ecf65bd07b393e18) )
	ROM_LOAD( "v120.bin", 0x0000, 0x020000, CRC(57ea3d86) SHA1(dbb12590aa735271b5f994f27726e61b6d52a1c0) )
ROM_END

ROM_START( ec_jackb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "jakx1v2.bin", 0x0000, 0x040000, CRC(50b0ad13) SHA1(b7850fcd9091b0f2293bbe4ada9204fbebd7533e) )
ROM_END

ROM_START( ec_ndgxs )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// MAB Scramble
	ROM_LOAD( "code.bin",       0x0000, 0x040000, CRC(55afc87b) SHA1(7c5061093ef1c0ce74714419e25420338b1dceb5) )
	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "nudex2v0.bin",   0x0000, 0x040000, CRC(e63cfb71) SHA1(e6af1e5db0d716fe67c02b5c18b0277eb061a6ca) )
	ROM_LOAD( "nudex2v2.bin",   0x0000, 0x040000, CRC(00b04625) SHA1(1a3df05f1b5ba8da0af07d81820d49ef55303e91) )
	ROM_LOAD( "nudge2v4.bin",   0x0000, 0x040000, CRC(c4448b0c) SHA1(13502fd169314651998ca9ba2b13ea6c9aaf8476) )
	ROM_LOAD( "nudge2v5.bin",   0x0000, 0x040000, CRC(4e4c4f65) SHA1(d07424cd0b3a9b768f84a6fa11508741efe7c244) )
	ROM_LOAD( "nudge2v6.bin",   0x0000, 0x040000, CRC(16a3c54c) SHA1(ffa1c2243ba7fc89e267390090d55187d84506a4) )
	ROM_LOAD( "nudge2v7.bin",   0x0000, 0x040000, CRC(3d26121c) SHA1(977524c910cd4e79ee07adf1e40afcca8e3fe3db) )
	ROM_LOAD( "nudge2v8.bin",   0x0000, 0x040000, CRC(cf12fc69) SHA1(1d80504a9f12f9db67713785001b8c8e83c821b1) )
	ROM_LOAD( "nudge2v9.bin",   0x0000, 0x040000, CRC(79f0a17a) SHA1(9e1a618a32996432aab3d5afd171c84b5ce69672) )
	ROM_LOAD( "nudge2v9p.bin",  0x0000, 0x040000, CRC(4cc6704a) SHA1(d0be6626917568427dfccecca199a8f54b731916) )
	ROM_LOAD( "nudge3v0.p.bin", 0x0000, 0x040000, CRC(0d7af34e) SHA1(ef924518601f50b15960a5e0963fe74696db8668) )
ROM_END

ROM_START( ec_rdht7 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "rh7m.bin", 0x0000, 0x040000, CRC(9a27075e) SHA1(868ef67ddfbb0f72d3d9b8d4efa1aeab51867197) )
ROM_END

ROM_START( ec_unkt )
	ROM_REGION( 0x200000, "maincpu", 0 )
	//Strange data, possibly bad, but looks like some sort of linked board ROM
	ROM_LOAD( "t.bin", 0x0000, 0x000789, CRC(ee41f048) SHA1(d9b0539ac822218f71cd85301e3da35b3dea783b) )
	ROM_LOAD( "t2.bin", 0x0000, 0x000989, CRC(0992ffa6) SHA1(cffb6e0a9a72bb2bf9a6e262074062bd06cfa1fb) )
ROM_END

DRIVER_INIT_MEMBER(ecoinfr_state,ecoinfr)
{
}

DRIVER_INIT_MEMBER(ecoinfr_state,ecoinfrmab)
{
	// descramble here
}

// for the Brunel Research sets
DRIVER_INIT_MEMBER(ecoinfr_state,ecoinfrbr)
{
}

// 3rd party sets with MAB scrambling, game names might be incorrect, should be the same basic hardware as these tho.
GAME( 19??, ec_barxmab, ec_barx  , ecoinfr,   ecoinfr_barx, ecoinfr_state,   ecoinfrmab,    ROT0,  "Electrocoin", "Bar X (MAB PCB) (Electrocoin)"       , GAME_FLAGS) // scrambled roms
GAME( 19??, ec_spbg7mab,ec_big7  , ecoinfr,   ecoinfr_barx, ecoinfr_state,   ecoinfrmab,    ROT0,  "Electrocoin", "Super Big 7 (MAB PCB) (Electrocoin) (?)"     , GAME_FLAGS)
GAME( 19??, ec_supbxmab,ec_sbarx , ecoinfr,   ecoinfr_barx, ecoinfr_state,   ecoinfrmab,    ROT0,  "Electrocoin", "Super Bar X (MAB PCB) (Electrocoin) (?)"     , GAME_FLAGS)

//Games using the MAB scrambling, but identified as being from Concept Games
GAME( 19??, ec_casbxcon,ec_casbx , ecoinfr,   ecoinfr_barx, ecoinfr_state,   ecoinfrmab,    ROT0,  "Concept Games Ltd", "Casino Bar X (Concept Games Ltd) (?)"      , GAME_FLAGS)
GAME( 19??, ec_multb,   0        , ecoinfr,   ecoinfr_barx, ecoinfr_state,   ecoinfrmab,    ROT0,  "Concept Games Ltd", "Multi Bar (Concept Games Ltd) (?)"     , GAME_FLAGS)
GAME( 19??, ec_supbxcon,ec_sbarx , ecoinfr,   ecoinfr_barx, ecoinfr_state,   ecoinfrmab,    ROT0,  "Concept Games Ltd", "Super Bar X (MAB PCB) (Concept Games Ltd) (?)"     , GAME_FLAGS)
GAME( 19??, ec_casmb,   0        , ecoinfr,   ecoinfr_barx, ecoinfr_state,   ecoinfrmab,    ROT0,  "Concept Games Ltd", "Casino Multi Bar (Concept Games Ltd) (?)"      , GAME_FLAGS)
GAME( 19??, ec_supmb,   0        , ecoinfr,   ecoinfr_barx, ecoinfr_state,   ecoinfrmab,    ROT0,  "Concept Games Ltd", "Super Multi Bar (Concept Games Ltd) (?)"       , GAME_FLAGS)
GAME( 19??, ec_stkex,   0        , ecoinfr,   ecoinfr_barx, ecoinfr_state,   ecoinfrmab,    ROT0,  "Concept Games Ltd", "Stake X (Concept Games Ltd) (?)"       , GAME_FLAGS)
GAME( 19??, ec_bar7,    0        , ecoinfr,   ecoinfr_barx, ecoinfr_state,   ecoinfrmab,    ROT0,  "Concept Games Ltd", "Bar 7 (Concept Games Ltd) (?)"       , GAME_FLAGS)
GAME( 19??, ec_fltr,    0        , ecoinfr,   ecoinfr_barx, ecoinfr_state,   ecoinfrmab,    ROT0,  "Concept Games Ltd", "Flutter (Concept Games Ltd) (?)"       , GAME_FLAGS)
GAME( 19??, ec_rdht7,   0        , ecoinfr,   ecoinfr_barx, ecoinfr_state,   ecoinfrmab,    ROT0,  "Concept Games Ltd", "Red Hot 7 (MAB PCB?) (Concept Games Ltd) (?)"      , GAME_FLAGS)
GAME( 19??, ec_unkt,    0        , ecoinfr,   ecoinfr_barx, ecoinfr_state,   ecoinfrmab,    ROT0,  "Concept Games Ltd", "unknown 'T' (MAB PCB?) (Concept Games Ltd) (?)"        , GAME_FLAGS)

//These look more like some variant of Astra Gaming hardware than the MAB PCB, but I can't be sure. Certainly they don't seem to be on the base hardware
GAME( 19??, ec_gold7,   0        , ecoinfr,   ecoinfr_barx, ecoinfr_state,   ecoinfrmab,    ROT0,  "Concept Games Ltd", "Golden 7 (Concept Games Ltd) (?)"      , GAME_FLAGS)
GAME( 19??, ec_mgbel,   0        , ecoinfr,   ecoinfr_barx, ecoinfr_state,   ecoinfrmab,    ROT0,  "Concept Games Ltd", "Megabell (Concept Games Ltd) (?)"      , GAME_FLAGS)
GAME( 19??, ec_jackb,   0        , ecoinfr,   ecoinfr_barx, ecoinfr_state,   ecoinfrmab,    ROT0,  "Concept Games Ltd", "Jackpot Bars (MAB PCB?) (Concept Games Ltd) (?)"       , GAME_FLAGS)
GAME( 19??, ec_ndgxs,   0        , ecoinfr,   ecoinfr_barx, ecoinfr_state,   ecoinfrmab,    ROT0,  "Concept Games Ltd", "Nudge Xcess (MAB PCB?) (Concept Games Ltd) (?)"        , GAME_FLAGS)
