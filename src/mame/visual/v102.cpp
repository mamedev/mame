// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Skeleton driver for Visual 102 display terminal.

************************************************************************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/eeprompar.h"
#include "machine/input_merger.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/z80sio.h"
//#include "video/crt9006.h"
#include "video/crt9007.h"
//#include "video/crt9021.h"
#include "screen.h"

#include "v102_kbd.h"


namespace {

class v102_state : public driver_device
{
public:
	v102_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mpsc(*this, "mpsc")
		, m_chargen(*this, "chargen")
	{ }

	void v102(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void hs_w(int state);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<upd7201_device> m_mpsc;
	required_region_ptr<u8> m_chargen;

	bool m_hs_state = false;
	bool m_kb_clock = false;
};


u32 v102_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void v102_state::hs_w(int state)
{
	if (state && !m_hs_state)
	{
		m_kb_clock = !m_kb_clock;
		m_mpsc->txca_w(m_kb_clock);
		m_mpsc->rxca_w(m_kb_clock);
	}

	m_hs_state = bool(state);
}


void v102_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x8000, 0x8fff).ram();
	map(0xa000, 0xafff).ram();
	map(0xb800, 0xb9ff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write));
}

void v102_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x3f).rw("vpac", FUNC(crt9007_device::read), FUNC(crt9007_device::write));
	map(0x40, 0x43).rw("mpsc", FUNC(upd7201_device::ba_cd_r), FUNC(upd7201_device::ba_cd_w));
	map(0x60, 0x61).rw("usart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x80, 0x83).w("pit", FUNC(pit8253_device::write));
	map(0xa0, 0xa3).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void v102_state::machine_start()
{
	m_hs_state = false;
	m_kb_clock = false;

	m_mpsc->ctsa_w(0);
	m_mpsc->ctsb_w(0);

	save_item(NAME(m_hs_state));
	save_item(NAME(m_kb_clock));
}

static INPUT_PORTS_START(v102)
INPUT_PORTS_END

void v102_state::v102(machine_config &config)
{
	Z80(config, m_maincpu, 18.575_MHz_XTAL / 5); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &v102_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &v102_state::io_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(18.575_MHz_XTAL, 970, 0, 800, 319, 0, 300);
	//screen.set_raw(18.575_MHz_XTAL, 948, 0, 792, 319, 0, 300);
	screen.set_screen_update(FUNC(v102_state::screen_update));

	crt9007_device &vpac(CRT9007(config, "vpac", 18.575_MHz_XTAL / 10));
	vpac.set_character_width(10); // 6 in 132-column mode
	vpac.int_callback().set("mainirq", FUNC(input_merger_device::in_w<2>));
	vpac.hs_callback().set(FUNC(v102_state::hs_w));
	vpac.set_screen("screen");

	EEPROM_2804(config, "eeprom");

	UPD7201(config, m_mpsc, 18.575_MHz_XTAL / 5); // divider not verified
	m_mpsc->out_int_callback().set("mainirq", FUNC(input_merger_device::in_w<0>));
	m_mpsc->out_txda_callback().set("keyboard", FUNC(v102_keyboard_device::write_rxd));
	m_mpsc->out_txdb_callback().set("aux", FUNC(rs232_port_device::write_txd));
	m_mpsc->out_dtrb_callback().set("aux", FUNC(rs232_port_device::write_dtr));
	m_mpsc->out_rtsb_callback().set("aux", FUNC(rs232_port_device::write_rts));

	i8251_device &usart(I8251(config, "usart", 18.575_MHz_XTAL / 5)); // divider not verified
	usart.rxrdy_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));
	usart.txd_handler().set("modem", FUNC(rs232_port_device::write_txd));
	usart.dtr_handler().set("modem", FUNC(rs232_port_device::write_dtr));
	usart.rts_handler().set("modem", FUNC(rs232_port_device::write_rts));

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	pit8253_device &pit(PIT8253(config, "pit", 0));
	pit.set_clk<0>(18.575_MHz_XTAL / 6);
	pit.set_clk<1>(18.575_MHz_XTAL / 6);
	pit.set_clk<2>(18.575_MHz_XTAL / 6);
	pit.out_handler<0>().set("usart", FUNC(i8251_device::write_txc));
	pit.out_handler<1>().set("usart", FUNC(i8251_device::write_rxc));
	pit.out_handler<2>().set(m_mpsc, FUNC(upd7201_device::txcb_w));
	pit.out_handler<2>().append(m_mpsc, FUNC(upd7201_device::rxcb_w));

	I8255(config, "ppi");

	v102_keyboard_device &keyboard(V102_KEYBOARD(config, "keyboard"));
	keyboard.txd_callback().set(m_mpsc, FUNC(upd7201_device::rxa_w));

	rs232_port_device &modem(RS232_PORT(config, "modem", default_rs232_devices, nullptr));
	modem.rxd_handler().set("usart", FUNC(i8251_device::write_rxd));
	modem.cts_handler().set("usart", FUNC(i8251_device::write_cts));
	modem.dcd_handler().set("usart", FUNC(i8251_device::write_dsr));

	rs232_port_device &aux(RS232_PORT(config, "aux", default_rs232_devices, nullptr));
	aux.rxd_handler().set(m_mpsc, FUNC(upd7201_device::rxb_w));
	aux.dcd_handler().set(m_mpsc, FUNC(upd7201_device::dcdb_w)); // DTR (printer busy)
}


/**************************************************************************************************************

Visual 102. (VT-102 clone plus graphics)
Chips: D780C-1 (Z80), CRT9021B-018, COM8251A, D8255AC-5, 2x CRT9006-135, CRT9007, M5L8253P-5, X2804AP-35, D7201C
Crystals: 18.575000
Keyboard: TMP8039P-6

***************************************************************************************************************/

ROM_START( v102 )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "245-001.u1",  0x0000, 0x4000, CRC(c36cc525) SHA1(a45e75ded10979c8e3ad262e2cf5818e08db762c) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "260-001.u50", 0x0000, 0x1000, CRC(732f5b99) SHA1(d105bf9f3ed41109d7181bcf0223bb280afe3f0a) )
ROM_END

} // anonymous namespace


COMP( 1984, v102, 0, 0, v102, v102, v102_state, empty_init, "Visual Technology", "Visual 102", MACHINE_IS_SKELETON )
