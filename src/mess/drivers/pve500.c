// license:MAME|GPL-2.0+
// copyright-holders:Felipe Correa
/***************************************************************************

  SONY PVE-500 Editing Control Unit
  "A/B roll edit controller for professional video editing applications"

  Driver by Felipe Correa da Silva Sanches <juca@members.fsf.org>
  Technical info at https://www.garoa.net.br/wiki/PVE-500

  Licensed under GPLv2 or later.

  NOTE: Even though the MAME/MESS project has been adopting a non-commercial additional licensing clause, I do allow commercial usage
  of my portion of the code according to the plain terms of the GPL license (version 2 or later). This is useful if you happen to use
  my code in another project or in case the other MAME/MESS developers happen to drop the non-comercial clause completely. I suggest
  that other developers consider doing the same. --Felipe Sanches

  Changelog:

   2014 JAN 14 [Felipe Sanches]:
   * Initial driver skeleton
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "pve500.lh"

#define IO_EXPANDER_PORTA 0
#define IO_EXPANDER_PORTB 1
#define IO_EXPANDER_PORTC 2
#define IO_EXPANDER_PORTD 3
#define IO_EXPANDER_PORTE 4

class pve500_state : public driver_device
{
public:
	pve500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
	{ }

	DECLARE_WRITE8_MEMBER(dualport_ram_left_w);
	DECLARE_WRITE8_MEMBER(dualport_ram_right_w);
	DECLARE_READ8_MEMBER(dualport_ram_left_r);
	DECLARE_READ8_MEMBER(dualport_ram_right_r);

	DECLARE_WRITE8_MEMBER(io_expander_w);
	DECLARE_READ8_MEMBER(io_expander_r);
	DECLARE_DRIVER_INIT(pve500);
private:
	UINT8 dualport_7FE_data;
	UINT8 dualport_7FF_data;

	virtual void machine_start();
	virtual void machine_reset();
	required_device<tlcs_z80_device> m_maincpu;
	required_device<tlcs_z80_device> m_subcpu;
	UINT8 io_SEL, io_LD, io_LE, io_SC, io_KY;
};


static Z80CTC_INTERFACE( external_ctc_intf )
{
	DEVCB_NULL, /* interrupt handler */
	DEVCB_NULL, /* ZC/TO0 callback */
	DEVCB_NULL, /* ZC/TO1 callback */
	DEVCB_NULL  /* ZC/TO2 callback */
};

static const z80sio_interface external_sio_intf =
{
	DEVCB_NULL, /* interrupt handler */
	DEVCB_NULL, /* DTR changed handler */
	DEVCB_NULL, /* RTS changed handler */
	DEVCB_NULL, /* BREAK changed handler */
	DEVCB_NULL, /* transmit handler */
	DEVCB_NULL  /* receive handler */
};

static ADDRESS_MAP_START(maincpu_io, AS_IO, 8, pve500_state)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("external_sio", z80sio_device, read, write)
	AM_RANGE(0x08, 0x0B) AM_DEVREADWRITE("external_ctc", z80ctc_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START(maincpu_prg, AS_PROGRAM, 8, pve500_state)
	AM_RANGE (0x0000, 0xBFFF) AM_ROM // ICB7: 48kbytes EEPROM
	AM_RANGE (0xC000, 0xDFFF) AM_RAM // ICD6: 8kbytes of RAM
	AM_RANGE (0xE7FE, 0xE7FE) AM_MIRROR(0x1800) AM_READ(dualport_ram_left_r)
	AM_RANGE (0xE7FF, 0xE7FF) AM_MIRROR(0x1800) AM_WRITE(dualport_ram_left_w)
	AM_RANGE (0xE000, 0xE7FF) AM_MIRROR(0x1800) AM_RAM AM_SHARE("sharedram") //  ICF5: 2kbytes of RAM shared between the two CPUs
ADDRESS_MAP_END

static ADDRESS_MAP_START(subcpu_prg, AS_PROGRAM, 8, pve500_state)
	AM_RANGE (0x0000, 0x7FFF) AM_ROM // ICG5: 32kbytes EEPROM
	AM_RANGE (0x8000, 0xBFFF) AM_MIRROR(0x3FF8) AM_READWRITE(io_expander_r, io_expander_w) // ICG3: I/O Expander
	AM_RANGE (0xC7FE, 0xC7FE) AM_MIRROR(0x1800) AM_WRITE(dualport_ram_right_w)
	AM_RANGE (0xC7FF, 0xC7FF) AM_MIRROR(0x1800) AM_READ(dualport_ram_right_r)
	AM_RANGE (0xC000, 0xC7FF) AM_MIRROR(0x3800) AM_RAM AM_SHARE("sharedram") //  ICF5: 2kbytes of RAM shared between the two CPUs
ADDRESS_MAP_END

DRIVER_INIT_MEMBER( pve500_state, pve500 )
{
}

static INPUT_PORTS_START( pve500 )
	PORT_START("keyboard")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_NAME("TODO") PORT_CODE(KEYCODE_A)
INPUT_PORTS_END

void pve500_state::machine_start()
{
	io_LD = 0;
	io_SC = 0;
	io_LE = 0;
	io_SEL = 0;
	io_KY = 0;

	for (int i=0; i<27; i++)
		output_set_digit_value(i, 0xff);
}

void pve500_state::machine_reset()
{
}

READ8_MEMBER(pve500_state::dualport_ram_left_r)
{
	//printf("dualport_ram: Left READ\n");
	m_subcpu->ctc_trg1(1); //(INT_Right)
	return dualport_7FE_data;
}

WRITE8_MEMBER(pve500_state::dualport_ram_left_w)
{
	//printf("dualport_ram: Left WRITE\n");
	dualport_7FF_data = data;
	m_subcpu->ctc_trg1(0); //(INT_Right)
}

READ8_MEMBER(pve500_state::dualport_ram_right_r)
{
	//printf("dualport_ram: Right READ\n");
	m_maincpu->ctc_trg1(1); //(INT_Left)
	return dualport_7FF_data;
}

WRITE8_MEMBER(pve500_state::dualport_ram_right_w)
{
	//printf("dualport_ram: Right WRITE\n");
	dualport_7FE_data = data;
	m_maincpu->ctc_trg1(0); //(INT_Left)
}

READ8_MEMBER(pve500_state::io_expander_r)
{
	switch (offset){
		case IO_EXPANDER_PORTA:
			return io_SC;
		case IO_EXPANDER_PORTB:
			return io_LE;
		case IO_EXPANDER_PORTC:
			return io_KY;
		case IO_EXPANDER_PORTD:
			return io_LD;
		case IO_EXPANDER_PORTE:
			return io_SEL & 0x0F; //This is a 4bit port.
		default:
			return 0;
	}
}

WRITE8_MEMBER(pve500_state::io_expander_w)
{
	//printf("io_expander_w: offset=%d data=%02X\n", offset, data);
	switch (offset){
		case IO_EXPANDER_PORTA:
			io_SC = data;
			break;
		case IO_EXPANDER_PORTB:
			io_LE = data;
			break;
		case IO_EXPANDER_PORTC:
			io_KY = data;
			break;
		case IO_EXPANDER_PORTD:
			io_LD = data;
			break;
		case IO_EXPANDER_PORTE:
			io_SEL = data;
			for (int i=0; i<4; i++){
				if (io_SEL & (1 << i)){
					switch (io_SC){
						case 1:   output_set_digit_value(8*i + 0, io_LD & 0x7F); break;
						case 2:   output_set_digit_value(8*i + 1, io_LD & 0x7F); break;
						case 4:   output_set_digit_value(8*i + 2, io_LD & 0x7F); break;
						case 8:   output_set_digit_value(8*i + 3, io_LD & 0x7F); break;
						case 16:  output_set_digit_value(8*i + 4, io_LD & 0x7F); break;
						case 32:  output_set_digit_value(8*i + 5, io_LD & 0x7F); break;
						case 64:  output_set_digit_value(8*i + 6, io_LD & 0x7F); break;
						case 128: output_set_digit_value(8*i + 7, io_LD & 0x7F); break;
						default:
							/*software should not do it.
					any idea how to emulate that in case it does? */ break;
					}
				}
			}
			break;
		default:
			break;
	}
}

static MACHINE_CONFIG_START( pve500, pve500_state )
	MCFG_CPU_ADD("maincpu", TLCS_Z80, XTAL_12MHz / 2) /* TMPZ84C015BF-6 (TOSHIBA TLCS-Z80) */
	MCFG_CPU_PROGRAM_MAP(maincpu_prg)
	MCFG_CPU_IO_MAP(maincpu_io)
	MCFG_Z80CTC_ADD("external_ctc", XTAL_12MHz / 2, external_ctc_intf)
	MCFG_Z80SIO_ADD("external_sio", XTAL_12MHz / 2, external_sio_intf)

	MCFG_CPU_ADD("subcpu", TLCS_Z80, XTAL_12MHz / 2) /* TMPZ84C015BF-6 (TOSHIBA TLCS-Z80) */
	MCFG_CPU_PROGRAM_MAP(subcpu_prg)

/* TODO:
-> There are a few LEDs and a sequence of 7-seg displays with atotal of 27 digits
-> Sound hardware consists of a buzzer connected to a signal of the maincpu SIO and a logic-gate that attaches/detaches it from the
   system clock Which apparently means you can only beep the buzzer to a certain predefined tone or keep it mute.
*/

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_pve500)

MACHINE_CONFIG_END

ROM_START( pve500 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("pve500.icb7",  0x00000, 0x10000, CRC(1036709c) SHA1(207d6fcad5c2f081a138184060ce7bd02736965b) ) //48kbyte main-cpu program + 16kbyte of unreachable memory

	ROM_REGION( 0x8000, "subcpu", 0 )
	ROM_LOAD("pve500.icg5",  0x00000, 0x8000, CRC(28cca60a) SHA1(308d70062653769250327ede7a4e1a8a76fc9ab9) ) //32kbyte sub-cpu program
ROM_END

/*    YEAR  NAME    PARENT  COMPAT  MACHINE     INPUT   CLASS           INIT   COMPANY    FULLNAME                    FLAGS */
COMP( 1995, pve500, 0,      0,      pve500,     pve500, pve500_state, pve500, "SONY", "PVE-500", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND)
