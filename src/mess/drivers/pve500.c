// license:MAME|GPL-2.0+
// copyright-holders: Felipe Sanches
/***************************************************************************

  SONY PVE-500 Editing Control Unit
  "A/B roll edit controller for professional video editing applications"

  Driver by Felipe Correa da Silva Sanches <juca@members.fsf.org>
  Technical info at https://www.garoa.net.br/wiki/PVE-500

  Changelog:

   2014 JAN 14 [Felipe Sanches]:
   * Initial driver skeleton
*/

#include "emu.h"
#include "cpu/z80/tmpz84c015.h"
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
	required_device<tmpz84c015_device> m_maincpu;
	required_device<tmpz84c015_device> m_subcpu;
	UINT8 io_SEL, io_LD, io_LE, io_SC, io_KY;
};


static const z80_daisy_config maincpu_daisy_chain[] =
{
	TMPZ84C015_DAISY_INTERNAL,
	{ "external_ctc" },
	{ "external_sio" },
	{ NULL }
};


static ADDRESS_MAP_START(maincpu_io, AS_IO, 8, pve500_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("external_sio", z80sio0_device, cd_ba_r, cd_ba_w)
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
	PORT_START("SCAN0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TRANS")       PORT_CODE(KEYCODE_5)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A/B")         PORT_CODE(KEYCODE_4)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("FROM TO")     PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P2")          PORT_CODE(KEYCODE_2)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P1")          PORT_CODE(KEYCODE_1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTRY")       PORT_CODE(KEYCODE_SPACE)

	PORT_START("SCAN1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ALL STOP")    PORT_CODE(KEYCODE_M)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LAST EDIT")   PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("AUDIO SPLIT") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A2")          PORT_CODE(KEYCODE_9)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ASMBL")       PORT_CODE(KEYCODE_6)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V")           PORT_CODE(KEYCODE_7)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A1")          PORT_CODE(KEYCODE_8)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTRY")       PORT_CODE(KEYCODE_SPACE)

	PORT_START("SCAN2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RVW/JUMP")    PORT_CODE(KEYCODE_N)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("AUTO EDIT")   PORT_CODE(KEYCODE_B)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PREVIEW")     PORT_CODE(KEYCODE_V)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P-FF")        PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P-REW")       PORT_CODE(KEYCODE_E)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P-STILL")     PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P-PLAY")      PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTRY")       PORT_CODE(KEYCODE_SPACE)

	PORT_START("SCAN3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R-OUT")       PORT_CODE(KEYCODE_K)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R-IN")        PORT_CODE(KEYCODE_J)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("GO TO")       PORT_CODE(KEYCODE_H)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P-OUT")       PORT_CODE(KEYCODE_G)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P-IN")        PORT_CODE(KEYCODE_F)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TRIM+")       PORT_CODE(KEYCODE_U)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TRIM-")       PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTRY")       PORT_CODE(KEYCODE_SPACE)

	PORT_START("SCAN4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R-FF")        PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R-REW")       PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R-STILL")     PORT_CODE(KEYCODE_P)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R-PLAY")      PORT_CODE(KEYCODE_O)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("EDIT")        PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("REC")         PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTRY")       PORT_CODE(KEYCODE_SPACE)

	PORT_START("SCAN5")
		PORT_DIPNAME( 0x03, 0x02, "R-EDIT REF" )
		PORT_DIPSETTING(    0x02, "TC" )
		PORT_DIPSETTING(    0x00, "RTC" )
		PORT_DIPSETTING(    0x01, "CTL" )

		PORT_DIPNAME( 0x0C, 0x08, "P2-EDIT REF" )
		PORT_DIPSETTING(    0x08, "TC" )
		PORT_DIPSETTING(    0x00, "RTC" )
		PORT_DIPSETTING(    0x04, "CTL" )

		PORT_DIPNAME( 0x30, 0x20, "P1-EDIT REF" )
		PORT_DIPSETTING(    0x20, "TC" )
		PORT_DIPSETTING(    0x00, "RTC" )
		PORT_DIPSETTING(    0x10, "CTL" )

	PORT_START("SCAN6")
		PORT_DIPNAME( 0x03, 0x02, "SYNCHRO" )
		PORT_DIPSETTING(    0x02, "ON/CF" )
		PORT_DIPSETTING(    0x00, "ON" )
		PORT_DIPSETTING(    0x01, "OFF" )

		PORT_DIPNAME( 0x0C, 0x08, "PREROLL" )
		PORT_DIPSETTING(    0x08, "7" )
		PORT_DIPSETTING(    0x00, "5" )
		PORT_DIPSETTING(    0x04, "3" )

	PORT_START("SCAN7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TOTAL")       PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LEARN")       PORT_CODE(KEYCODE_L)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TRANS-1F")    PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TRANS-10F")   PORT_CODE(KEYCODE_X)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TRANS-100F")  PORT_CODE(KEYCODE_C)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R-RESET")     PORT_CODE(KEYCODE_A)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P2-RESET")    PORT_CODE(KEYCODE_S)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P1-RESET")    PORT_CODE(KEYCODE_D)
INPUT_PORTS_END

void pve500_state::machine_start()
{
	io_LD = 0;
	io_SC = 0;
	io_LE = 0;
	io_SEL = 0;
	io_KY = 0;

	for (int i=0; i<27; i++)
		output_set_digit_value(i, 0x00);
}

void pve500_state::machine_reset()
{
}

READ8_MEMBER(pve500_state::dualport_ram_left_r)
{
	//printf("dualport_ram: Left READ\n");
	m_subcpu->trg1(1); //(INT_Right)
	return dualport_7FE_data;
}

WRITE8_MEMBER(pve500_state::dualport_ram_left_w)
{
	//printf("dualport_ram: Left WRITE\n");
	dualport_7FF_data = data;
	m_subcpu->trg1(0); //(INT_Right)
}

READ8_MEMBER(pve500_state::dualport_ram_right_r)
{
	//printf("dualport_ram: Right READ\n");
	m_maincpu->trg1(1); //(INT_Left)
	return dualport_7FF_data;
}

WRITE8_MEMBER(pve500_state::dualport_ram_right_w)
{
	//printf("dualport_ram: Right WRITE\n");
	dualport_7FE_data = data;
	m_maincpu->trg1(0); //(INT_Left)
}

READ8_MEMBER(pve500_state::io_expander_r)
{
	switch (offset){
		case IO_EXPANDER_PORTA:
			return io_SC;
		case IO_EXPANDER_PORTB:
			return io_LE;
		case IO_EXPANDER_PORTC:
			io_KY = 0x00;
			if (io_SC & 0x01) io_KY |= ioport("SCAN0")->read();
			if (io_SC & 0x02) io_KY |= ioport("SCAN1")->read();
			if (io_SC & 0x04) io_KY |= ioport("SCAN2")->read();
			if (io_SC & 0x08) io_KY |= ioport("SCAN3")->read();
			if (io_SC & 0x10) io_KY |= ioport("SCAN4")->read();
			if (io_SC & 0x20) io_KY |= ioport("SCAN5")->read();
			if (io_SC & 0x40) io_KY |= ioport("SCAN6")->read();
			if (io_SC & 0x80) io_KY |= ioport("SCAN7")->read();
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
					for (int j=0; j<8; j++){
						if (io_SC & (1<<j)){
							output_set_digit_value(8*i + j, BITSWAP8(io_LD & 0x7F, 7, 0, 1, 2, 3, 4, 5, 6));
							//printf("io_expander_w PORTE data=%02X\n", data);
						}
					}
				}
			}
			break;
		default:
			break;
	}
}

static MACHINE_CONFIG_START( pve500, pve500_state )
	MCFG_CPU_ADD("maincpu", TMPZ84C015, XTAL_12MHz / 2) /* TMPZ84C015BF-6 */
	MCFG_CPU_PROGRAM_MAP(maincpu_prg)
	MCFG_CPU_IO_MAP(maincpu_io)
	MCFG_CPU_CONFIG(maincpu_daisy_chain)

	MCFG_DEVICE_ADD("external_ctc", Z80CTC, XTAL_12MHz / 2)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_Z80SIO0_ADD("external_sio", XTAL_12MHz / 2, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_CPU_ADD("subcpu", TMPZ84C015, XTAL_12MHz / 2) /* TMPZ84C015BF-6 */
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
