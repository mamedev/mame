// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    2013-09-10 Skeleton driver for Televideo TV950
    2016-07-30 Preliminary not-so-skeleton driver

    TODO:
    - VIA T2 counter mode emulation
    - CRTC reset and drawing
    - Bidirectional communications mode

    Hardware:
    6502A CPU
    6545 CRTC
    6522A VIA, wired to count HSYNCs and to enable the 6502 to pull RESET on the CRTC
    3x 6551 ACIA  1 for the keyboard, 1 for the modem port, 1 for the printer port

    VIA hookup (see schematics):
    PA3 = beep?
    PA5 = inverse video
    PA6 = IRQ in
    PA7 = force blank
    PB6 = Hblank in
    PB7 = out speaker
    CA1 = reset CRTC in
    CA2 = reset CRTC out
    CB2 = blink timer

    IRQ = ACIAs (all 3 ORed together)
    NMI = 6522 VIA's IRQ line

    http://www.bitsavers.org/pdf/televideo/950/Model_950_Terminal_Theory_of_Operation_26Jan1981.pdf
    http://www.bitsavers.org/pdf/televideo/950/2002100_Model_950_Maintenance_Manual_Nov1983.pdf
    http://www.bitsavers.org/pdf/televideo/950/B300002-001_Model_950_Operators_Manual_Feb81.pdf

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6502/m6502.h"
#include "machine/input_merger.h"
#include "machine/6522via.h"
#include "machine/mos6551.h"
#include "tv950kb.h"
#include "video/mc6845.h"
#include "screen.h"


namespace {

#define MASTER_CLOCK XTAL(23'814'000)

class tv950_state : public driver_device
{
public:
	tv950_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_via(*this, "via")
		, m_crtc(*this, "crtc")
		, m_uart(*this, "a%uuart", 49U)
		, m_keyboard(*this, "keyboard")
		, m_vram(*this, "vram")
		, m_gfx(*this, "graphics")
		, m_dsw(*this, "DSW%u", 0U)
	{ }

	void tv950(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void via_a_w(uint8_t data);
	void via_b_w(uint8_t data);
	uint8_t via_b_r();
	void crtc_vs_w(int state);
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr);

	void row_addr_w(uint8_t data);
	void via_crtc_reset_w(int state);

	void tv950_mem(address_map &map) ATTR_COLD;

	uint8_t m_via_row = 0;
	uint8_t m_attr_row = 0;
	uint8_t m_attr_screen = 0;

	required_device<m6502_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<r6545_1_device> m_crtc;
	required_device_array<mos6551_device, 3> m_uart;
	required_device<tv950kb_device> m_keyboard;
	required_shared_ptr<uint8_t> m_vram;
	required_region_ptr<uint16_t> m_gfx;
	required_ioport_array<4> m_dsw;

	int m_row_addr = 0;
	int m_row = 0;
};

void tv950_state::machine_start()
{
	m_uart[0]->write_dcd(0);
	m_uart[0]->write_dsr(0);
	m_uart[2]->write_cts(0);
}

void tv950_state::tv950_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x3fff).ram().share("vram"); // VRAM
	map(0x8100, 0x8100).rw(m_crtc, FUNC(r6545_1_device::status_r), FUNC(r6545_1_device::address_w));
	map(0x8101, 0x8101).rw(m_crtc, FUNC(r6545_1_device::register_r), FUNC(r6545_1_device::register_w));
	map(0x9000, 0x9000).w(FUNC(tv950_state::row_addr_w));
	map(0x9300, 0x9303).rw(m_uart[0], FUNC(mos6551_device::read), FUNC(mos6551_device::write)); // CS0 = AB9
	map(0x9500, 0x9503).rw(m_uart[2], FUNC(mos6551_device::read), FUNC(mos6551_device::write)); // CS0 = AB10
	map(0x9900, 0x9903).rw(m_uart[1], FUNC(mos6551_device::read), FUNC(mos6551_device::write)); // CS0 = AB11
	map(0xb100, 0xb10f).m(m_via, FUNC(via6522_device::map));
	map(0xe000, 0xffff).rom().region("maincpu", 0);
}


/* Input ports */
static INPUT_PORTS_START( tv950 )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, "S01")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPNAME( 0x02, 0x00, "S02")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPNAME( 0x04, 0x00, "S03")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPNAME( 0x08, 0x00, "S04")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPNAME( 0x10, 0x00, "S05")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPNAME( 0x20, 0x20, "S06")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "S07")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPNAME( 0x02, 0x00, "S08")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPNAME( 0x04, 0x00, "S09")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPNAME( 0x08, 0x00, "S10")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPNAME( 0x10, 0x00, "S11")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPNAME( 0x20, 0x20, "S12")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "S13")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPNAME( 0x02, 0x00, "S14")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPNAME( 0x04, 0x00, "S15")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPNAME( 0x08, 0x00, "S16")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPNAME( 0x10, 0x00, "S17")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPNAME( 0x20, 0x20, "S18")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "S19")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPNAME( 0x02, 0x00, "S20")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


void tv950_state::machine_reset()
{
	m_row = 0;
	m_via_row = 0;
	m_attr_row = 0;
	m_attr_screen = 0;
}

void tv950_state::crtc_vs_w(int state)
{
	m_attr_screen = 0;
}

void tv950_state::via_crtc_reset_w(int state)
{
	//printf("via_crtc_reset_w: %d\n", state);
	m_via->write_ca1(state);

	if (!state)
	{
		//m_crtc->device_reset();
	}
}

void tv950_state::row_addr_w(uint8_t data)
{
	m_row_addr = data;
}

void tv950_state::via_a_w(uint8_t data)
{
	m_via_row = ~data & 15;
	// PA4, 5, 7 to do
}

void tv950_state::via_b_w(uint8_t data)
{
	// bit 3 of m_via_row must be active as well?
	m_keyboard->rx_w(!BIT(data, 7));
}

uint8_t tv950_state::via_b_r()
{
	uint8_t data = 0xff;
	for (int n = 0; n < 4; n++)
		if (BIT(m_via_row, n))
			data &= m_dsw[n]->read();
	return data;
}

MC6845_ON_UPDATE_ADDR_CHANGED( tv950_state::crtc_update_addr )
{
}

MC6845_UPDATE_ROW( tv950_state::crtc_update_row )
{
	if (ra)
		m_attr_row = m_attr_screen;
	else
		m_attr_screen = m_attr_row;

	uint32_t *p = &bitmap.pix(m_row);
	rgb_t fg(255,255,255,255);
	rgb_t bg(0,0,0,0);

	for(uint8_t x = 0; x < x_count; x++)
	{
		uint8_t chr = m_vram[ma + x];
		if ((chr & 0x90)==0x90)
			m_attr_row = chr & 15;
		uint16_t data = m_gfx[chr * 16 + (m_row % 10)];
		if (x == cursor_x)
			data ^= 0xff;
		// apply attributes...

		for (uint8_t i = 0; i < 14; i++)
			*p++ = BIT( data, 13-i ) ? fg : bg;
	}
	m_row = (m_row + 1) % 250;
}

void tv950_state::tv950(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, MASTER_CLOCK/14);
	m_maincpu->set_addrmap(AS_PROGRAM, &tv950_state::tv950_mem);

	input_merger_device &mainirq(INPUT_MERGER_ANY_HIGH(config, "mainirq")); // open collector
	mainirq.output_handler().set_inputline(m_maincpu, m6502_device::IRQ_LINE);
	mainirq.output_handler().append(m_via, FUNC(via6522_device::write_pa6)).invert();

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(MASTER_CLOCK, 1200, 0, 1120, 370, 0, 250);   // not real values
	screen.set_screen_update("crtc", FUNC(r6545_1_device::screen_update));

	// there are many 6845 CRTC submodels, the Theory of Operation manual references the Rockwell R6545-1 specificially.
	R6545_1(config, m_crtc, MASTER_CLOCK/14);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(14);
	m_crtc->set_update_row_callback(FUNC(tv950_state::crtc_update_row));
	m_crtc->set_on_update_addr_change_callback(FUNC(tv950_state::crtc_update_addr));
	m_crtc->out_hsync_callback().set(m_via, FUNC(via6522_device::write_pb6)).invert();
	m_crtc->out_vsync_callback().set(FUNC(tv950_state::crtc_vs_w));
	m_crtc->set_screen(nullptr);

	MOS6522(config, m_via, MASTER_CLOCK/14);
	//m_via->irq_handler().set_inputline(m_maincpu, M6502_NMI_LINE);
	m_via->writepa_handler().set(FUNC(tv950_state::via_a_w));
	m_via->writepb_handler().set(FUNC(tv950_state::via_b_w));
	m_via->readpb_handler().set(FUNC(tv950_state::via_b_r));
	m_via->ca2_handler().set(FUNC(tv950_state::via_crtc_reset_w));
	//m_via->cb2_handler().set(FUNC(tv950_state::via_blink_rate_w));

	MOS6551(config, m_uart[0], MASTER_CLOCK/14).set_xtal(MASTER_CLOCK/13); // for keyboard
	m_uart[0]->irq_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));

	MOS6551(config, m_uart[1], MASTER_CLOCK/14).set_xtal(MASTER_CLOCK/13); // for main port
	m_uart[1]->irq_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));
	m_uart[1]->dtr_handler().set("p3", FUNC(rs232_port_device::write_dtr));
	m_uart[1]->rts_handler().set("p3", FUNC(rs232_port_device::write_rts));
	m_uart[1]->txd_handler().set("p3", FUNC(rs232_port_device::write_txd));

	MOS6551(config, m_uart[2], MASTER_CLOCK/14).set_xtal(MASTER_CLOCK/13); // for printer port
	m_uart[2]->irq_handler().set("mainirq", FUNC(input_merger_device::in_w<2>));
	m_uart[2]->txd_handler().set("p4", FUNC(rs232_port_device::write_txd)); // to pin 3 (RXD)
	m_uart[2]->rts_handler().set("p4", FUNC(rs232_port_device::write_rts)); // to pin 5 (CTS)
	m_uart[2]->dtr_handler().set("p4", FUNC(rs232_port_device::write_dtr)); // to pin 6 (DSR)

	TV950_KEYBOARD(config, m_keyboard);
	m_keyboard->tx_cb().set(m_uart[0], FUNC(mos6551_device::write_rxd));

	rs232_port_device &p3(RS232_PORT(config, "p3", default_rs232_devices, nullptr)); // main port
	p3.dsr_handler().set(m_uart[1], FUNC(mos6551_device::write_dsr));
	p3.dcd_handler().set(m_uart[1], FUNC(mos6551_device::write_dcd));
	p3.rxd_handler().set(m_uart[1], FUNC(mos6551_device::write_rxd));
	p3.cts_handler().set(m_uart[1], FUNC(mos6551_device::write_cts));

	rs232_port_device &p4(RS232_PORT(config, "p4", default_rs232_devices, nullptr)); // printer port
	p4.dsr_handler().set(m_uart[2], FUNC(mos6551_device::write_dsr)); // from pin 20 (DTR)
	p4.rxd_handler().set(m_uart[2], FUNC(mos6551_device::write_rxd)); // from pin 2 (TXD)
}

/* ROM definition */
ROM_START( tv950 )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "180000-001a_a41_eb17.bin", 0x001000, 0x001000, CRC(b7187cc5) SHA1(41cc8fd51661314e03ee7e00cc1e206e9a694d92) )
	ROM_LOAD( "180000-007a_a42_67d3.bin", 0x000000, 0x001000, CRC(3ef2e6fb) SHA1(21ccfd2b50c37b715eed67671b82faa4d75fc6bb) )

	ROM_REGION16_LE(0x2000, "graphics", 0)
	ROM_LOAD16_BYTE( "180000-002a_a33_9294.bin", 0x000001, 0x001000, CRC(eaf4f346) SHA1(b4c531626846f3f055ddc086ac24fdb1b34f3f8e) )
	ROM_LOAD16_BYTE( "180000-003a_a32_7ebf.bin", 0x000000, 0x001000, CRC(783ca0b6) SHA1(1cec9a9a56ef5795809f7ca7cd2e3f61b27e698d) )

	ROM_REGION(0x10000, "user1", 0)
	// came with "tv950.zip"
	ROM_LOAD( "180000-43i.a25", 0x0000, 0x1000, CRC(ac6f0bfc) SHA1(2a3863700405fbb9e510613559d78fceee3544e8) )
	ROM_LOAD( "180000-44i.a20", 0x0000, 0x1000, CRC(db91a727) SHA1(e94724ed1a563fb846f4203ae6523ee6b4c6577f) )
	// came with "tv950kbd.zip"
	ROM_LOAD( "1800000-003a.a32", 0x0000, 0x1000, CRC(eaef0138) SHA1(7198851299fce07c95d18e32cbfbe936c0dbec2a) )
	ROM_LOAD( "1800000-002a.a33", 0x0000, 0x1000, CRC(856dd85c) SHA1(e2570017e098b0e1ead7749e9c2ac40be2367433) )
ROM_END

} // anonymous namespace


/* Driver */
//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME                            FLAGS
COMP( 1981, tv950, 0,      0,      tv950,   tv950, tv950_state, empty_init, "TeleVideo", "Model 950 Video Display Terminal", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
