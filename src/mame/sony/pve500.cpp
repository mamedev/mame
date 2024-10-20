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

#include "emu.h"
#include "bus/rs232/rs232.h" /* actually meant to be RS422 ports */
#include "cpu/mb88xx/mb88xx.h"
#include "cpu/z80/tmpz84c015.h"
#include "machine/clock.h"
#include "machine/cxd1095.h"
#include "machine/eepromser.h"
#include "machine/mb8421.h"
#include "machine/z80sio.h"
#include "sound/beep.h"
#include "speaker.h"

#include "pve500.lh"

#define LOG_7SEG_DISPLAY_SIGNALS (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

#define DEBUGGING_INDUCE_SELFDIAGNOSE 0

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
		, m_cxdio(*this, "cxdio")
		, m_eeprom(*this, "eeprom")
		, m_buzzer(*this, "buzzer")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void pve500(machine_config &config);

	void init_pve500();

private:
	void mb8421_intl(int state);
	void mb8421_intr(int state);
	void GPI_w(int state);
	void cxdio_reset_w(int state);
	void external_monitor_w(int state);

	uint8_t io_ky_r();
	void io_sc_w(uint8_t data);
	void io_le_w(uint8_t data);
	void io_ld_w(uint8_t data);
	void io_sel_w(uint8_t data);
	void eeprom_w(uint8_t data);
	uint8_t eeprom_r();
	void maincpu_io(address_map &map) ATTR_COLD;
	void maincpu_prg(address_map &map) ATTR_COLD;
	void subcpu_io(address_map &map) ATTR_COLD;
	void subcpu_prg(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<tmpz84c015_device> m_maincpu;
	required_device<tmpz84c015_device> m_subcpu;
	required_device<cxd1095_device> m_cxdio;
	required_device<eeprom_serial_er5911_device> m_eeprom;
	required_device<beep_device> m_buzzer;
	output_finder<27> m_digits;

	uint8_t io_SEL = 0, io_LD = 0, io_LE = 0, io_SC = 0, io_KY = 0;
	int LD_data[4]{};
};

void pve500_state::GPI_w(int state)
{
	/* TODO: Implement-me */
}

void pve500_state::cxdio_reset_w(int state)
{
	if (!state)
		m_cxdio->reset();
}

void pve500_state::external_monitor_w(int state)
{
	/* TODO: Implement-me */
}

static const z80_daisy_config maincpu_daisy_chain[] =
{
	{ "external_ctc" },
	{ "external_sio" },
	{ nullptr }
};


void pve500_state::maincpu_io(address_map &map)
{
	map(0x00, 0x03).mirror(0xff00).rw("external_sio", FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x08, 0x0B).mirror(0xff00).rw("external_ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

void pve500_state::maincpu_prg(address_map &map)
{
	map(0x0000, 0xbfff).rom(); // ICB7: 48kbytes EPROM
	map(0xc000, 0xdfff).ram(); // ICD6: 8kbytes of RAM
	map(0xe000, 0xe7ff).mirror(0x1800).rw("mb8421", FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w));
}

void pve500_state::subcpu_io(address_map &map)
{
}

void pve500_state::subcpu_prg(address_map &map)
{
	map(0x0000, 0x7fff).rom(); // ICG5: 32kbytes EPROM
	map(0x8000, 0x8007).mirror(0x3ff8).rw(m_cxdio, FUNC(cxd1095_device::read), FUNC(cxd1095_device::write));
	map(0xc000, 0xc7ff).mirror(0x3800).rw("mb8421", FUNC(mb8421_device::right_r), FUNC(mb8421_device::right_w));
}

void pve500_state::init_pve500()
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
	m_digits.resolve();
}

void pve500_state::machine_reset()
{
	/* Setup beep */
	m_buzzer->set_state(0);
}

void pve500_state::mb8421_intl(int state)
{
	// shared ram interrupt request from subcpu side
	m_maincpu->trg1(state);
}

void pve500_state::mb8421_intr(int state)
{
	// shared ram interrupt request from maincpu side
	m_subcpu->trg1(state);
}

uint8_t pve500_state::eeprom_r()
{
	return (m_eeprom->ready_read() << 1) | m_eeprom->do_read();
}

void pve500_state::eeprom_w(uint8_t data)
{
	m_eeprom->di_write( (data & (1 << 2)) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->clk_write( (data & (1 << 3)) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->cs_write( (data & (1 << 4)) ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t pve500_state::io_ky_r()
{
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
}

void pve500_state::io_sc_w(uint8_t data)
{
	const int swap[4] = {2,1,0,3};

	LOGMASKED(LOG_7SEG_DISPLAY_SIGNALS, "CXD1095 PORTA (io_SC=%02X)\n", data);
	io_SC = data;

	for (int j=0; j<8; j++){
		if (!BIT(io_SC,j)){
			int digits = (j < 3) ? 4 : 3;
			for (int i = 0; i < digits; i++)
			{
				assert(8*swap[i] + j < 27);
				m_digits[8*swap[i] + j] = LD_data[i];
			}
		}
	}
}

void pve500_state::io_le_w(uint8_t data)
{
	LOGMASKED(LOG_7SEG_DISPLAY_SIGNALS, "CXD1095 PORTB (io_LE=%02X)\n", data);
	io_LE = data;
}

void pve500_state::io_ld_w(uint8_t data)
{
	LOGMASKED(LOG_7SEG_DISPLAY_SIGNALS, "CXD1095 PORTD (io_LD=%02X)\n", data);
	io_LD = data;
}

void pve500_state::io_sel_w(uint8_t data)
{
	LOGMASKED(LOG_7SEG_DISPLAY_SIGNALS, "CXD1095 PORTE (io_SEL=%02X)\n", data);
	io_SEL = data;
	for (int i=0; i<4; i++){
		if (BIT(io_SEL, i)){
			LD_data[i] = 0x7F & bitswap<8>(io_LD ^ 0xFF, 7, 0, 1, 2, 3, 4, 5, 6);
		}
	}
}

void pve500_state::pve500(machine_config &config)
{
	/* Main CPU */
	TMPZ84C015(config, m_maincpu, 12_MHz_XTAL / 2); // TMPZ84C015BF-6
	m_maincpu->set_addrmap(AS_PROGRAM, &pve500_state::maincpu_prg);
	m_maincpu->set_addrmap(AS_IO, &pve500_state::maincpu_io);
	m_maincpu->set_daisy_config(maincpu_daisy_chain);
	m_maincpu->out_dtra_callback().set(FUNC(pve500_state::GPI_w));
	m_maincpu->out_dtrb_callback().set(m_buzzer, FUNC(beep_device::set_state)).invert();
	m_maincpu->out_txda_callback().set("recorder", FUNC(rs232_port_device::write_txd));
	m_maincpu->out_txdb_callback().set("player1", FUNC(rs232_port_device::write_txd));

	z80ctc_device& ctc(Z80CTC(config, "external_ctc", 12_MHz_XTAL / 2));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80sio_device& sio(Z80SIO(config, "external_sio", 12_MHz_XTAL / 2)); // TMPZ84C40AP-8
	sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	sio.out_txda_callback().set("player2", FUNC(rs232_port_device::write_txd));
	sio.out_txdb_callback().set("edl_inout", FUNC(rs232_port_device::write_txd));

	/* Secondary CPU */
	TMPZ84C015(config, m_subcpu, 12_MHz_XTAL / 2); /* TMPZ84C015BF-6 */
	m_subcpu->set_addrmap(AS_PROGRAM, &pve500_state::subcpu_prg);
	m_subcpu->set_addrmap(AS_IO, &pve500_state::subcpu_io);
	m_subcpu->out_dtra_callback().set(FUNC(pve500_state::cxdio_reset_w));
	m_subcpu->out_dtrb_callback().set(FUNC(pve500_state::external_monitor_w));
	m_subcpu->out_txda_callback().set("switcher", FUNC(rs232_port_device::write_txd));
	m_subcpu->out_txdb_callback().set("serial_mixer", FUNC(rs232_port_device::write_txd));

	// PIO callbacks
	m_subcpu->in_pa_callback().set(FUNC(pve500_state::eeprom_r));
	m_subcpu->out_pa_callback().set(FUNC(pve500_state::eeprom_w));

	// ICG3: I/O Expander
	CXD1095(config, m_cxdio);
	m_cxdio->out_porta_cb().set(FUNC(pve500_state::io_sc_w));
	m_cxdio->out_portb_cb().set(FUNC(pve500_state::io_le_w));
	m_cxdio->in_portc_cb().set(FUNC(pve500_state::io_ky_r));
	m_cxdio->out_portd_cb().set(FUNC(pve500_state::io_ld_w));
	m_cxdio->out_porte_cb().set(FUNC(pve500_state::io_sel_w));

	/* Search Dial MCUs */
	MB88201(config, "dial_mcu_left", 4_MHz_XTAL).set_disable(); /* PLAYER DIAL MCU */
	MB88201(config, "dial_mcu_right", 4_MHz_XTAL).set_disable(); /* RECORDER DIAL MCU */

	/* Serial EEPROM (128 bytes, 8-bit data organization) */
	/* The EEPROM stores the setup data */
	EEPROM_MSM16911_8BIT(config, "eeprom");

	/* FIX-ME: These are actually RS422 ports (except EDL IN/OUT which is indeed an RS232 port)*/
	rs232_port_device &recorder(RS232_PORT(config, "recorder", default_rs232_devices, nullptr));
	recorder.rxd_handler().set(m_maincpu, FUNC(tmpz84c015_device::rxa_w));

	rs232_port_device &player1(RS232_PORT(config, "player1", default_rs232_devices, nullptr));
	player1.rxd_handler().set(m_maincpu, FUNC(tmpz84c015_device::rxb_w));

	rs232_port_device &player2(RS232_PORT(config, "player2", default_rs232_devices, nullptr));
	player2.rxd_handler().set("external_sio", FUNC(z80sio_device::rxa_w));

	rs232_port_device &edl_inout(RS232_PORT(config, "edl_inout", default_rs232_devices, nullptr));
	edl_inout.rxd_handler().set("external_sio", FUNC(z80sio_device::rxb_w));

	rs232_port_device &switcher(RS232_PORT(config, "switcher", default_rs232_devices, nullptr));
	switcher.rxd_handler().set(m_subcpu, FUNC(tmpz84c015_device::rxa_w));

	rs232_port_device &serial_mixer(RS232_PORT(config, "serial_mixer", default_rs232_devices, nullptr));
	serial_mixer.rxd_handler().set(m_subcpu, FUNC(tmpz84c015_device::rxb_w));

	clock_device &clk1(CLOCK(config, "clk1", 12_MHz_XTAL / 20));
	clk1.signal_handler().set(m_maincpu, FUNC(tmpz84c015_device::rxca_w));
	clk1.signal_handler().append(m_maincpu, FUNC(tmpz84c015_device::txca_w));
	clk1.signal_handler().append(m_maincpu, FUNC(tmpz84c015_device::rxcb_w));
	clk1.signal_handler().append(m_maincpu, FUNC(tmpz84c015_device::txcb_w));
	clk1.signal_handler().append(m_subcpu, FUNC(tmpz84c015_device::rxca_w));
	clk1.signal_handler().append(m_subcpu, FUNC(tmpz84c015_device::txca_w));
	clk1.signal_handler().append(m_subcpu, FUNC(tmpz84c015_device::rxcb_w));
	clk1.signal_handler().append(m_subcpu, FUNC(tmpz84c015_device::txcb_w));

	/* ICF5: 2kbytes of RAM shared between the two CPUs (dual-port RAM)*/
	mb8421_device &mb8421(MB8421(config, "mb8421"));
	mb8421.intl_callback().set(FUNC(pve500_state::mb8421_intl));
	mb8421.intr_callback().set(FUNC(pve500_state::mb8421_intr));

	/* video hardware */
	config.set_default_layout(layout_pve500);

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, "buzzer", 12_MHz_XTAL / 3200).add_route(ALL_OUTPUTS, "mono", 0.05); // 3.75 kHz CLK2 coming out of IC D4 (frequency divider circuitry)
}

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

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY  FULLNAME   FLAGS
COMP( 1995, pve500, 0,      0,      pve500,  pve500, pve500_state, init_pve500, "SONY",  "PVE-500", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS)
