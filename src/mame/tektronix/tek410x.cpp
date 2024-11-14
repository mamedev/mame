// license:BSD-3-Clause
// copyright-holders:Curt Coder, AJR
/***************************************************************************

    Tektronix 4107A/4109A

    Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/i8255.h"
#include "machine/mc68681.h"
#include "tek410x_kbd.h"
#include "video/crt9007.h"
#include "emupal.h"
#include "screen.h"


namespace {

class tek4107a_state : public driver_device
{
public:
	tek4107a_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_duart(*this, "duart%u", 0U)
		, m_keyboard(*this, "keyboard")
		, m_vpac(*this, "vpac")
		, m_ppi_pc(0)
		, m_kb_rdata(true)
		, m_kb_tdata(true)
		, m_kb_rclamp(false)
		, m_graphics_control(0)
		, m_alpha_control(0)
		, m_x_position(0)
		, m_y_position(0)
		, m_x_cursor(0)
		, m_y_cursor(0)
	{ }

	void tek4109a(machine_config &config);
	void tek4107a(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u8 vpac_r(offs_t offset);

	u16 nmi_enable_r();
	u16 nmi_disable_r();
	u16 system_reset_r();

	void ppi_pc_w(u8 data);
	void kb_rdata_w(int state);
	void kb_tdata_w(int state);
	void kb_rclamp_w(int state);

	void xpos_w(u16 data);
	void ypos_w(u16 data);
	void xcur_w(u16 data);
	void ycur_w(u16 data);
	void tbwin_w(u16 data);
	u16 gcntl_r();
	void gcntl_w(u16 data);
	u16 acntl_r();
	void acntl_w(u16 data);
	u16 test_r();
	u8 font_r();

	void tek4107a_io(address_map &map) ATTR_COLD;
	void tek4107a_mem(address_map &map) ATTR_COLD;

	required_device_array<scn2681_device, 2> m_duart;
	required_device<tek410x_keyboard_device> m_keyboard;
	required_device<crt9007_device> m_vpac;

	u8 m_ppi_pc;
	bool m_kb_rdata;
	bool m_kb_tdata;
	bool m_kb_rclamp;

	u16 m_graphics_control;
	u16 m_alpha_control;
	u16 m_x_position;
	u16 m_y_position;
	u16 m_x_cursor;
	u16 m_y_cursor;
};

u8 tek4107a_state::vpac_r(offs_t offset)
{
	return m_vpac->read(offset + 0x20);
}

u16 tek4107a_state::nmi_enable_r()
{
	// TODO
	return 0;
}

u16 tek4107a_state::nmi_disable_r()
{
	// TODO
	return 0;
}

u16 tek4107a_state::system_reset_r()
{
	// TODO
	return 0;
}

void tek4107a_state::ppi_pc_w(u8 data)
{
	if (!m_kb_rclamp && BIT(m_ppi_pc, 2) != BIT(data, 2))
		m_keyboard->kdo_w(!BIT(data, 2) || m_kb_tdata);

	m_ppi_pc = data;
}

void tek4107a_state::kb_rdata_w(int state)
{
	m_kb_rdata = state;
	if (!m_kb_rclamp)
		m_duart[0]->rx_a_w(state);
}

void tek4107a_state::kb_rclamp_w(int state)
{
	if (m_kb_rclamp != !state)
	{
		m_kb_rclamp = !state;

		// Clamp RXDA to 1 and KBRDATA to 0 when DUART asserts RxRDYA
		if (m_kb_tdata || !BIT(m_ppi_pc, 2))
			m_keyboard->kdo_w(state);
		m_duart[0]->rx_a_w(state ? m_kb_rdata : 1);
	}
}

void tek4107a_state::kb_tdata_w(int state)
{
	if (m_kb_tdata != state)
	{
		m_kb_tdata = state;

		m_duart[0]->ip4_w(!state);
		if (BIT(m_ppi_pc, 2) && m_kb_rdata && !m_kb_rclamp)
			m_keyboard->kdo_w(state);
	}
}

void tek4107a_state::xpos_w(u16 data)
{
	m_x_position = data & 0x03ff;
}

void tek4107a_state::ypos_w(u16 data)
{
	m_y_position = data & 0x01ff;
}

void tek4107a_state::xcur_w(u16 data)
{
	m_x_cursor = data & 0x03ff;
}

void tek4107a_state::ycur_w(u16 data)
{
	m_y_cursor = data & 0x01ff;
}

void tek4107a_state::tbwin_w(u16 data)
{
	// TODO
}

u16 tek4107a_state::gcntl_r()
{
	return m_graphics_control;
}

void tek4107a_state::gcntl_w(u16 data)
{
	m_graphics_control = data;
}

u16 tek4107a_state::acntl_r()
{
	return m_alpha_control;
}

void tek4107a_state::acntl_w(u16 data)
{
	m_alpha_control = data;
}

u16 tek4107a_state::test_r()
{
	// TODO
	return 0;
}

u8 tek4107a_state::font_r()
{
	// TODO
	return 0;
}


/* Memory Maps */

void tek4107a_state::tek4107a_mem(address_map &map)
{
	map(0x00000, 0x3ffff).ram();
	map(0x40000, 0x7ffff).ram().share("gfxram");
	map(0x80000, 0xbffff).rom().region("firmware", 0);
	map(0xf0000, 0xfffff).rom().region("firmware", 0x30000);
}

void tek4107a_state::tek4107a_io(address_map &map)
{
	map(0x0000, 0x001f).rw(m_duart[0], FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0x00ff);
	map(0x0000, 0x001f).rw(m_duart[1], FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0xff00);
	map(0x0080, 0x00bf).r(FUNC(tek4107a_state::vpac_r)).w(m_vpac, FUNC(crt9007_device::write)).umask16(0x00ff);
	map(0x00c0, 0x00c1).w(FUNC(tek4107a_state::xpos_w));
	map(0x00c2, 0x00c3).w(FUNC(tek4107a_state::ypos_w));
	map(0x00c4, 0x00c5).w(FUNC(tek4107a_state::xcur_w));
	map(0x00c6, 0x00c7).w(FUNC(tek4107a_state::ycur_w));
	map(0x00c8, 0x00c9).r(FUNC(tek4107a_state::test_r));
	map(0x00ca, 0x00ca).r(FUNC(tek4107a_state::font_r));
	map(0x00ca, 0x00cb).w(FUNC(tek4107a_state::tbwin_w));
	map(0x00cc, 0x00cd).rw(FUNC(tek4107a_state::gcntl_r), FUNC(tek4107a_state::gcntl_w));
	map(0x00ce, 0x00cf).rw(FUNC(tek4107a_state::acntl_r), FUNC(tek4107a_state::acntl_w));
	map(0x0100, 0x0107).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0xff00);
	map(0x0200, 0x0201).r(FUNC(tek4107a_state::nmi_enable_r));
	map(0x0280, 0x0281).r(FUNC(tek4107a_state::system_reset_r));
	map(0x0300, 0x0301).r(FUNC(tek4107a_state::nmi_disable_r));
}

/* Input Ports */

static INPUT_PORTS_START( tek4107a )
INPUT_PORTS_END

/* Video */

void tek4107a_state::video_start()
{
	save_item(NAME(m_graphics_control));
	save_item(NAME(m_alpha_control));
	save_item(NAME(m_x_position));
	save_item(NAME(m_y_position));
	save_item(NAME(m_x_cursor));
	save_item(NAME(m_y_cursor));
}

u32 tek4107a_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static const gfx_layout tek4107a_charlayout =
{
	8, 15,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8 },
	16*8
};

static GFXDECODE_START( gfx_tek4107a )
	GFXDECODE_ENTRY( "chargen", 0x0000, tek4107a_charlayout, 0, 1 )
GFXDECODE_END

/* Machine Initialization */

void tek4107a_state::machine_start()
{
	save_item(NAME(m_ppi_pc));
	save_item(NAME(m_kb_rdata));
	save_item(NAME(m_kb_tdata));
	save_item(NAME(m_kb_rclamp));
}

/* Machine Driver */

void tek4107a_state::tek4107a(machine_config &config)
{
	/* basic machine hardware */
	i80186_cpu_device &maincpu(I80186(config, "maincpu", 14.7456_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &tek4107a_state::tek4107a_mem);
	maincpu.set_addrmap(AS_IO, &tek4107a_state::tek4107a_io);

	SCN2681(config, m_duart[0], 14.7456_MHz_XTAL / 4);
	m_duart[0]->irq_cb().set("maincpu", FUNC(i80186_cpu_device::int0_w));
	m_duart[0]->outport_cb().set_inputline("maincpu", INPUT_LINE_NMI).bit(5).invert(); // RxRDYB
	m_duart[0]->outport_cb().append(FUNC(tek4107a_state::kb_rclamp_w)).bit(4);
	m_duart[0]->outport_cb().append(m_keyboard, FUNC(tek410x_keyboard_device::reset_w)).bit(3);
	m_duart[0]->a_tx_cb().set(m_keyboard, FUNC(tek410x_keyboard_device::kdi_w));

	SCN2681(config, m_duart[1], 14.7456_MHz_XTAL / 4);
	m_duart[1]->irq_cb().set("maincpu", FUNC(i80186_cpu_device::int2_w));

	i8255_device &ppi(I8255(config, "ppi"));
	ppi.in_pb_callback().set_constant(0x30);
	ppi.out_pc_callback().set(FUNC(tek4107a_state::ppi_pc_w));

	TEK410X_KEYBOARD(config, m_keyboard);
	m_keyboard->tdata_callback().set(FUNC(tek4107a_state::kb_tdata_w));
	m_keyboard->rdata_callback().set(FUNC(tek4107a_state::kb_rdata_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.2_MHz_XTAL, 800, 0, 640, 525, 0, 480);
	screen.set_screen_update(FUNC(tek4107a_state::screen_update));

	CRT9007(config, m_vpac, 25.2_MHz_XTAL / 8);
	m_vpac->set_screen("screen");
	m_vpac->set_character_width(8);
	m_vpac->int_callback().set("maincpu", FUNC(i80186_cpu_device::int1_w));

	PALETTE(config, "palette").set_entries(64);
	GFXDECODE(config, "gfxdecode", "palette", gfx_tek4107a);
}

void tek4107a_state::tek4109a(machine_config &config)
{
	tek4107a(config);

	/* video hardware */
	subdevice<palette_device>("palette")->set_entries(4096);
}

/* ROMs */

ROM_START( tek4107a )
	ROM_REGION16_LE( 0x40000, "firmware", 0 )
	ROM_LOAD16_BYTE( "160-2379-03.u60",  0x00000, 0x8000, NO_DUMP )
	ROM_LOAD16_BYTE( "160-2380-03.u160", 0x00001, 0x8000, NO_DUMP )
	ROM_LOAD16_BYTE( "160-2377-03.u70",  0x10000, 0x8000, NO_DUMP )
	ROM_LOAD16_BYTE( "160-2378-03.u170", 0x10001, 0x8000, CRC(feac272f) SHA1(f2018dc9bb5bd6840b2843c709cbc24689054dc0) )
	ROM_LOAD16_BYTE( "160-2375-03.u80",  0x20000, 0x8000, NO_DUMP )
	ROM_LOAD16_BYTE( "160-2376-03.u180", 0x20001, 0x8000, CRC(35a44e75) SHA1(278c7f218105ad3473409b16a8d27a0b9f7a859e) )
	ROM_LOAD16_BYTE( "160-2373-03.u90",  0x30000, 0x8000, CRC(d5e89b15) SHA1(d6eb1a0a684348a8194238d641528fb96fb29087) )
	ROM_LOAD16_BYTE( "160-2374-03.u190", 0x30001, 0x8000, CRC(a3fef76e) SHA1(902845c4aa0cbc392b62c726ac746ca62567d91c) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "160-2381 v1.0.u855", 0x0000, 0x1000, CRC(ac7ca279) SHA1(4c9de06d1c346f83eb8d0d09a0eb32c40bd8014c) )
ROM_END

ROM_START( tek4109a )
	// another set with 160-32xx-03 v10.5 labels exists
	ROM_REGION16_LE( 0x40000, "firmware", 0 )
	ROM_LOAD16_BYTE( "160-3283-02 v8.2.u60",  0x00000, 0x8000, CRC(2a821db6) SHA1(b4d8b74bd9fe43885dcdc4efbdd1eebb96e32060) )
	ROM_LOAD16_BYTE( "160-3284-02 v8.2.u160", 0x00001, 0x8000, CRC(ee567b01) SHA1(67b1b0648cfaa28d57473bcc45358ff2bf986acf) )
	ROM_LOAD16_BYTE( "160-3281-02 v8.2.u70",  0x10000, 0x8000, CRC(e2713328) SHA1(b0bb3471539ef24d79b18d0e33bc148ed27d0ec4) )
	ROM_LOAD16_BYTE( "160-3282-02 v8.2.u170", 0x10001, 0x8000, CRC(c109a4f7) SHA1(762019105c1f82200a9c99ccfcfd8ee81d2ac4fe) )
	ROM_LOAD16_BYTE( "160-3279-02 v8.2.u80",  0x20000, 0x8000, CRC(00822078) SHA1(a82e61dafccbaea44e67efaa5940e52ec6d07d7d) )
	ROM_LOAD16_BYTE( "160-3280-02 v8.2.u180", 0x20001, 0x8000, CRC(eec9f70f) SHA1(7b65336219f5fa0d11f8be2b37040b564a53c52f) )
	ROM_LOAD16_BYTE( "160-3277-02 v8.2.u90",  0x30000, 0x8000, CRC(cf6ebc97) SHA1(298db473874c57bf4eec788818179748030a9ad8) )
	ROM_LOAD16_BYTE( "160-3278-02 v8.2.u190", 0x30001, 0x8000, CRC(d6124cd1) SHA1(f826aee5ec07cf5ac369697d93def0259ad225bb) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "160-3087 v1.0.u855", 0x0000, 0x1000, CRC(97479528) SHA1(e9e15f1f64b3b6bd139accd51950bae71fdc2193) )
ROM_END

} // anonymous namespace


/* System Drivers */

//    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY      FULLNAME           FLAGS
COMP( 1983, tek4107a, 0,        0,      tek4107a, tek4107a, tek4107a_state, empty_init, "Tektronix", "Tektronix 4107A", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1983, tek4109a, tek4107a, 0,      tek4109a, tek4107a, tek4107a_state, empty_init, "Tektronix", "Tektronix 4109A", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
