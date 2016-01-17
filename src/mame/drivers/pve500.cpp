// license:GPL-2.0+
// copyright-holders: Felipe Sanches
/***************************************************************************

  SONY PVE-500 Editing Control Unit
  "A/B roll edit controller for professional video editing applications"

  Driver by Felipe Correa da Silva Sanches <juca@members.fsf.org>
  Technical info at https://www.garoa.net.br/wiki/PVE-500

  Notes:
  One can induce the self-diagnose by booting the device holding LEARN and P2-RESET buttons togheter
  With the default keyboard map, this can be done by holding keys L and S while pressing F3.
    (Don't forget to unlock the keyboard by using the UI TOGGLE key)

    This self-diagnose routine displays the value C817, which is the checksum value of the subcpu ROM
  and afterwards it displays the following message:

  SELFdIAG Error___ _F3 F3_CtC3c

  which means it detected an error in the CTC circuitry (it means we're emulating it wrong!)
  F3 is the coordinate of the subcpu EPROM chip in the PCB.

    According to the service manual, this error code means: "ICF3 CTC CH-3 counter operation failure (No interruption)"

  Known issues:
  There's still an annoying blinking in the 7-seg display.

  Changelog:

     2014 SEP 01 [Felipe Sanches]:
   * hooked-up MB8421 device (dual-port SRAM)

     2014 JUN 24 [Felipe Sanches]:
   * figured out the multiplexing signals for the 7-seg display

     2014 JUN 23 [Felipe Sanches]:
   * hooked-up the RS422 ports

   2014 JAN 14 [Felipe Sanches]:
   * Initial driver skeleton
*/

#define LOG_7SEG_DISPLAY_SIGNALS 0
#define DEBUGGING_INDUCE_SELFDIAGNOSE 0

#include "emu.h"
#include "cpu/z80/tmpz84c015.h"
#include "cpu/mb88xx/mb88xx.h"
#include "sound/beep.h"
#include "bus/rs232/rs232.h" /* actually meant to be RS422 ports */
#include "pve500.lh"
#include "machine/mb8421.h"
#include "machine/eepromser.h"

#define IO_EXPANDER_PORTA 0
#define IO_EXPANDER_PORTB 1
#define IO_EXPANDER_PORTC 2
#define IO_EXPANDER_PORTD 3
#define IO_EXPANDER_PORTE 4

class pve500_state : public driver_device
{
public:
	pve500_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_eeprom(*this, "eeprom")
		, m_buzzer(*this, "buzzer")
	{ }

	DECLARE_WRITE_LINE_MEMBER(mb8421_intl);
	DECLARE_WRITE_LINE_MEMBER(mb8421_intr);
	DECLARE_WRITE_LINE_MEMBER(GPI_w);
	DECLARE_WRITE_LINE_MEMBER(external_monitor_w);

	DECLARE_WRITE8_MEMBER(io_expander_w);
	DECLARE_READ8_MEMBER(io_expander_r);
	DECLARE_WRITE8_MEMBER(eeprom_w);
	DECLARE_READ8_MEMBER(eeprom_r);
	DECLARE_DRIVER_INIT(pve500);
private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<tmpz84c015_device> m_maincpu;
	required_device<tmpz84c015_device> m_subcpu;
	required_device<eeprom_serial_er5911_device> m_eeprom;
	required_device<beep_device> m_buzzer;
	UINT8 io_SEL, io_LD, io_LE, io_SC, io_KY;
};

WRITE_LINE_MEMBER( pve500_state::GPI_w )
{
	/* TODO: Implement-me */
}

WRITE_LINE_MEMBER( pve500_state::external_monitor_w )
{
	/* TODO: Implement-me */
}

static const z80_daisy_config maincpu_daisy_chain[] =
{
	TMPZ84C015_DAISY_INTERNAL,
	{ "external_ctc" },
	{ "external_sio" },
	{ nullptr }
};


static ADDRESS_MAP_START(maincpu_io, AS_IO, 8, pve500_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("external_sio", z80sio0_device, cd_ba_r, cd_ba_w)
	AM_RANGE(0x08, 0x0B) AM_DEVREADWRITE("external_ctc", z80ctc_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START(maincpu_prg, AS_PROGRAM, 8, pve500_state)
	AM_RANGE (0x0000, 0xBFFF) AM_ROM // ICB7: 48kbytes EPROM
	AM_RANGE (0xC000, 0xDFFF) AM_RAM // ICD6: 8kbytes of RAM
	AM_RANGE (0xE000, 0xE7FF) AM_MIRROR(0x1800) AM_DEVREADWRITE("mb8421", mb8421_device, left_r, left_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(subcpu_io, AS_IO, 8, pve500_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static ADDRESS_MAP_START(subcpu_prg, AS_PROGRAM, 8, pve500_state)
	AM_RANGE (0x0000, 0x7FFF) AM_ROM // ICG5: 32kbytes EPROM
	AM_RANGE (0x8000, 0xBFFF) AM_MIRROR(0x3FF8) AM_READWRITE(io_expander_r, io_expander_w) // ICG3: I/O Expander
	AM_RANGE (0xC000, 0xC7FF) AM_MIRROR(0x3800) AM_DEVREADWRITE("mb8421", mb8421_device, right_r, right_w)
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
}

void pve500_state::machine_reset()
{
	/* Setup beep */
	m_buzzer->set_state(0);
	m_buzzer->set_frequency(3750); //CLK2 coming out of IC D4 (frequency divider circuitry)
}

WRITE_LINE_MEMBER(pve500_state::mb8421_intl)
{
	// shared ram interrupt request from subcpu side
	m_maincpu->trg1(state);
}

WRITE_LINE_MEMBER(pve500_state::mb8421_intr)
{
	// shared ram interrupt request from maincpu side
	m_subcpu->trg1(state);
}

READ8_MEMBER(pve500_state::eeprom_r)
{
	return (m_eeprom->ready_read() << 1) | m_eeprom->do_read();
}

WRITE8_MEMBER(pve500_state::eeprom_w)
{
	m_eeprom->di_write( (data & (1 << 2)) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->clk_write( (data & (1 << 3)) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->cs_write( (data & (1 << 4)) ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER(pve500_state::io_expander_r)
{
//  printf("READ IO_EXPANDER_PORT%c\n", 'A'+offset);
	switch (offset){
		case IO_EXPANDER_PORTA:
			return io_SC;
		case IO_EXPANDER_PORTB:
			return io_LE;
		case IO_EXPANDER_PORTC:
			io_KY = 0x00;
			if (!BIT(io_SC, 0)) io_KY |= ioport("SCAN0")->read();
			if (!BIT(io_SC, 1)) io_KY |= ioport("SCAN1")->read();
			if (!BIT(io_SC, 2)) io_KY |= ioport("SCAN2")->read();
			if (!BIT(io_SC, 3)) io_KY |= ioport("SCAN3")->read();
			if (!BIT(io_SC, 4)) io_KY |= ioport("SCAN4")->read();
			if (!BIT(io_SC, 5)) io_KY |= ioport("SCAN5")->read();
			if (!BIT(io_SC, 6)) io_KY |= ioport("SCAN6")->read();
			if (!BIT(io_SC, 7)) io_KY |= ioport("SCAN7")->read();
#if DEBUGGING_INDUCE_SELFDIAGNOSE
			io_KY = 0x42; //according to procedure described in the service manual
#endif
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
	static int LD_data[4];
	int swap[4] = {2,1,0,3};
	switch (offset){
		case IO_EXPANDER_PORTA:
#if LOG_7SEG_DISPLAY_SIGNALS
printf("io_expander_w: PORTA (io_SC=%02X)\n", data);
#endif
			io_SC = data;

			for (int j=0; j<8; j++){
				if (!BIT(io_SC,j)){
					for (int i=0; i<4; i++)
						output().set_digit_value(8*swap[i] + j, LD_data[i]);
				}
			}
			break;
		case IO_EXPANDER_PORTB:
#if LOG_7SEG_DISPLAY_SIGNALS
			printf("io_expander_w: PORTB (io_LE=%02X)\n", data);
#endif
			io_LE = data;
			break;
		case IO_EXPANDER_PORTC:
#if LOG_7SEG_DISPLAY_SIGNALS
			printf("io_expander_w: PORTC (io_KY=%02X)\n", data);
#endif
			io_KY = data;
			break;
		case IO_EXPANDER_PORTD:
#if LOG_7SEG_DISPLAY_SIGNALS
			printf("io_expander_w: PORTD (io_LD=%02X)\n", data);
#endif
			io_LD = data;
			break;
		case IO_EXPANDER_PORTE:
#if LOG_7SEG_DISPLAY_SIGNALS
			printf("io_expander_w PORTE (io_SEL=%02X)\n", data);
#endif
			io_SEL = data;
			for (int i=0; i<4; i++){
				if (BIT(io_SEL, i)){
					LD_data[i] = 0x7F & BITSWAP8(io_LD ^ 0xFF, 7, 0, 1, 2, 3, 4, 5, 6);
				}
			}
			break;
		default:
			break;
	}
}

static MACHINE_CONFIG_START( pve500, pve500_state )
	/* Main CPU */
	MCFG_CPU_ADD("maincpu", TMPZ84C015, XTAL_12MHz / 2) /* TMPZ84C015BF-6 */
	MCFG_CPU_PROGRAM_MAP(maincpu_prg)
	MCFG_CPU_IO_MAP(maincpu_io)
	MCFG_CPU_CONFIG(maincpu_daisy_chain)
	MCFG_TMPZ84C015_OUT_DTRA_CB(WRITELINE(pve500_state, GPI_w))
	MCFG_TMPZ84C015_OUT_DTRB_CB(DEVWRITELINE("buzzer", beep_device, set_state))
	MCFG_TMPZ84C015_OUT_TXDA_CB(DEVWRITELINE("recorder", rs232_port_device, write_txd))
	MCFG_TMPZ84C015_OUT_TXDB_CB(DEVWRITELINE("player1", rs232_port_device, write_txd))

	MCFG_DEVICE_ADD("external_ctc", Z80CTC, XTAL_12MHz / 2)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_Z80SIO0_ADD("external_sio", XTAL_12MHz / 2, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE("player2", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE("edl_inout", rs232_port_device, write_txd))

	/* Secondary CPU */
	MCFG_CPU_ADD("subcpu", TMPZ84C015, XTAL_12MHz / 2) /* TMPZ84C015BF-6 */
	MCFG_CPU_PROGRAM_MAP(subcpu_prg)
	MCFG_CPU_IO_MAP(subcpu_io)
	MCFG_TMPZ84C015_OUT_DTRB_CB(WRITELINE(pve500_state, external_monitor_w))
	MCFG_TMPZ84C015_OUT_TXDA_CB(DEVWRITELINE("switcher", rs232_port_device, write_txd))
	MCFG_TMPZ84C015_OUT_TXDB_CB(DEVWRITELINE("serial_mixer", rs232_port_device, write_txd))

	// PIO callbacks
	MCFG_TMPZ84C015_IN_PA_CB(READ8(pve500_state, eeprom_r))
	MCFG_TMPZ84C015_OUT_PA_CB(WRITE8(pve500_state, eeprom_w))

	/* Search Dial MCUs */
	MCFG_CPU_ADD("dial_mcu_left", MB88201, XTAL_4MHz) /* PLAYER DIAL MCU */
	MCFG_CPU_ADD("dial_mcu_right", MB88201, XTAL_4MHz) /* RECORDER DIAL MCU */

	/* Serial EEPROM (128 bytes, 8-bit data organization) */
	/* The EEPROM stores the setup data */
	MCFG_EEPROM_SERIAL_MSM16911_8BIT_ADD("eeprom")

	/* FIX-ME: These are actually RS422 ports (except EDL IN/OUT which is indeed an RS232 port)*/
	MCFG_RS232_PORT_ADD("recorder", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("maincpu", tmpz84c015_device, rxa_w))

	MCFG_RS232_PORT_ADD("player1", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("maincpu", tmpz84c015_device, rxb_w))

	MCFG_RS232_PORT_ADD("player2", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("external_sio", z80dart_device, rxa_w))

	MCFG_RS232_PORT_ADD("edl_inout", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("external_sio", z80dart_device, rxb_w))

	MCFG_RS232_PORT_ADD("switcher", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("subcpu", tmpz84c015_device, rxa_w))

	MCFG_RS232_PORT_ADD("serial_mixer", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("subcpu", tmpz84c015_device, rxb_w))

	/* ICF5: 2kbytes of RAM shared between the two CPUs (dual-port RAM)*/
	MCFG_DEVICE_ADD("mb8421", MB8421, 0)
	MCFG_MB8421_INTL_HANDLER(WRITELINE(pve500_state, mb8421_intl))
	MCFG_MB8421_INTR_HANDLER(WRITELINE(pve500_state, mb8421_intr))

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_pve500)

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("buzzer", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.05)

MACHINE_CONFIG_END

ROM_START( pve500 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("pve500.icb7",  0x00000, 0x10000, CRC(1036709c) SHA1(207d6fcad5c2f081a138184060ce7bd02736965b) ) //48kbyte main-cpu program + 16kbyte of unreachable memory

	ROM_REGION( 0x8000, "subcpu", 0 )
	ROM_LOAD("pve500.icg5",  0x00000, 0x8000, CRC(28cca60a) SHA1(308d70062653769250327ede7a4e1a8a76fc9ab9) ) //32kbyte sub-cpu program

	ROM_REGION( 0x200, "dial_mcu_left", 0 ) /* PLAYER DIAL MCU */
	ROM_LOAD( "pve500.icd3", 0x0000, 0x0200, NO_DUMP )

	ROM_REGION( 0x200, "dial_mcu_right", 0 ) /* RECORDER DIAL MCU */
	ROM_LOAD( "pve500.icc3", 0x0000, 0x0200, NO_DUMP )

	ROM_REGION( 0x80, "eeprom", 0 ) /* The EEPROM stores the setup data */
	ROM_LOAD( "pve500.ice3", 0x0000, 0x080, NO_DUMP )
ROM_END

/*    YEAR  NAME    PARENT  COMPAT  MACHINE     INPUT   CLASS           INIT   COMPANY    FULLNAME                    FLAGS */
COMP( 1995, pve500, 0,      0,      pve500,     pve500, pve500_state, pve500, "SONY", "PVE-500", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS)
