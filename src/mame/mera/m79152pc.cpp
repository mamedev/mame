// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,AJR
/***************************************************************************

    Mera-Elzab 79152pc

    This system provides a half-featured emulation of the ADM-3A or similar
    terminals by TeleVideo and Wyse. The “half-featured” part is that some
    commands are not recognized at all and others are merely filtered out.

    The 8035 here serves as a soft CRTC, counting horizontal scans and
    outputting row addresses (dependent on scrolling) and vertical sync
    pulses. The present emulation produces incorrect video output at the
    vertical margins and is extremely prone to desyncing.

    “PC Shadow” is the name of the software this terminal either runs or
    interfaces with. The actual keyboard is unknown, but is almost
    certainly PC-XT compatible. The character set is a nonstandard variant
    of CP 437 that incorporates a few Polish letters.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/i8212.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "sound/beep.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class m79152pc_state : public driver_device
{
public:
	m79152pc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_videoram(*this, "videoram")
		, m_attributes(*this, "attributes")
		, m_chargen(*this, "chargen")
		, m_maincpu(*this, "maincpu")
		, m_mcu(*this, "mcu")
		, m_uart(*this, "uart")
		, m_screen(*this, "screen")
		, m_beep(*this, "beep")
	{ }

	void m79152pc(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void beep_w(offs_t offset, uint8_t data);
	void latch_full_w(int state);
	int mcu_t0_r();
	int mcu_t1_r();
	void mcu_p1_w(u8 data);
	void mcu_p2_w(u8 data);
	void lc_reset_w(u8 data);

	TIMER_CALLBACK_MEMBER(hsync_on);
	TIMER_CALLBACK_MEMBER(hsync_off);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_draw_line(bitmap_ind16 &bitmap, unsigned y);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mcu_map(address_map &map) ATTR_COLD;
	void mcu_io_map(address_map &map) ATTR_COLD;

	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_attributes;
	required_region_ptr<u8> m_chargen;

	required_device<z80_device> m_maincpu;
	required_device<mcs48_cpu_device> m_mcu;
	required_device<z80sio_device> m_uart;
	required_device<screen_device> m_screen;
	required_device<beep_device> m_beep;

	u8 m_line_base = 0;
	u8 m_line_count = 0;
	bool m_latch_full = false;
	u8 m_mcu_p2 = 0;
	bool m_hsync = false;

	emu_timer *m_hsync_on_timer = nullptr;
	emu_timer *m_hsync_off_timer = nullptr;
};

void m79152pc_state::beep_w(offs_t offset, uint8_t data)
{
	m_beep->set_state(BIT(offset, 2));
}

void m79152pc_state::latch_full_w(int state)
{
	m_latch_full = state == ASSERT_LINE;
}

int m79152pc_state::mcu_t0_r()
{
	return m_latch_full ? 0 : 1;
}

int m79152pc_state::mcu_t1_r()
{
	return m_hsync ? 0 : 1;
}

void m79152pc_state::mcu_p1_w(u8 data)
{
	m_line_base = data;
}

void m79152pc_state::mcu_p2_w(u8 data)
{
	m_mcu_p2 = data;
}

void m79152pc_state::lc_reset_w(u8 data)
{
	m_line_count = (data >> 4) & 0xf;
}

void m79152pc_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0x8fff).ram().share("videoram");
	map(0x9000, 0x9fff).ram().share("attributes");
}

void m79152pc_state::io_map(address_map &map)
{
	//map.unmap_value_high();
	map.global_mask(0xff);
	map(0x40, 0x43).rw(m_uart, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x44, 0x47).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x48, 0x4b).w("pit", FUNC(pit8253_device::write));
	map(0x4c, 0x4c).w("mculatch", FUNC(i8212_device::strobe));
	map(0x54, 0x57).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x58, 0x58).select(4).w(FUNC(m79152pc_state::beep_w));
}

void m79152pc_state::mcu_map(address_map &map)
{
	map(0x000, 0x7ff).rom().region("mcu", 0);
}

void m79152pc_state::mcu_io_map(address_map &map)
{
	map(0x00, 0x00).mirror(0xff).r("mculatch", FUNC(i8212_device::read)).w(FUNC(m79152pc_state::lc_reset_w));
}

/* Input ports */
static INPUT_PORTS_START( m79152pc )
INPUT_PORTS_END


TIMER_CALLBACK_MEMBER(m79152pc_state::hsync_on)
{
	m_screen->update_now();
	m_mcu->set_input_line(MCS48_INPUT_IRQ, ASSERT_LINE);
	m_hsync = true;
	m_line_count = (m_line_count + 1) & 0xf;
}

TIMER_CALLBACK_MEMBER(m79152pc_state::hsync_off)
{
	unsigned vpos = m_screen->vpos();
	m_mcu->set_input_line(MCS48_INPUT_IRQ, CLEAR_LINE);
	m_hsync_on_timer->adjust(m_screen->time_until_pos(vpos, 640));
	m_hsync = false;
}

void m79152pc_state::screen_draw_line(bitmap_ind16 &bitmap, unsigned y)
{
	const u16 ma = u16(m_line_base) << 4;
	const u8 ra = m_line_count & 0xf;

	u16 *p = &bitmap.pix(y++);

	for (u16 x = ma; x < ma + 80; x++)
	{
		// BIT(attr, 3) should probably be blinking
		// BIT(attr, 1) may be used for high-intensity text
		u8 chr = m_videoram[x];
		u8 attr = m_attributes[x];
		u8 gfx = m_chargen[(chr << 4) | (BIT(attr, 2) && ra == 15 ? 3 : ra)];
		if (BIT(attr, 0))
			gfx ^= 0xff;

		*p++ = BIT(gfx, 7);
		*p++ = BIT(gfx, 6);
		*p++ = BIT(gfx, 5);
		*p++ = BIT(gfx, 4);
		*p++ = BIT(gfx, 3);
		*p++ = BIT(gfx, 2);
		*p++ = BIT(gfx, 1);
		*p++ = BIT(gfx, 0);
	}
}

u32 m79152pc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (unsigned y = cliprect.top(); y <= cliprect.bottom(); y++)
		screen_draw_line(bitmap, y);

	return 0;
}

/* F4 Character Displayer */
static const gfx_layout m79152pc_charlayout =
{
	8, 12,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_m79152pc )
	GFXDECODE_ENTRY( "chargen", 0x0000, m79152pc_charlayout, 0, 1 )
GFXDECODE_END


void m79152pc_state::machine_start()
{
	m_latch_full = false;
	m_mcu_p2 = 0xff;
	m_line_base = 0;
	m_line_count = 0;
	m_hsync = false;

	m_hsync_on_timer = timer_alloc(FUNC(m79152pc_state::hsync_on), this);
	m_hsync_off_timer = timer_alloc(FUNC(m79152pc_state::hsync_off), this);
	m_hsync_off_timer->adjust(m_screen->time_until_pos(9, 0), 0, m_screen->scan_period());

	save_item(NAME(m_latch_full));
	save_item(NAME(m_mcu_p2));
	save_item(NAME(m_line_base));
	save_item(NAME(m_line_count));
	save_item(NAME(m_hsync));
}

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ "uart" },
	{ nullptr }
};

void m79152pc_state::m79152pc(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4'000'000); // UA880D
	m_maincpu->set_addrmap(AS_PROGRAM, &m79152pc_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &m79152pc_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	I8035(config, m_mcu, 6'000'000); // NEC D8035HLC
	m_mcu->set_addrmap(AS_PROGRAM, &m79152pc_state::mcu_map);
	m_mcu->set_addrmap(AS_IO, &m79152pc_state::mcu_io_map);
	m_mcu->t0_in_cb().set(FUNC(m79152pc_state::mcu_t0_r));
	m_mcu->t1_in_cb().set(FUNC(m79152pc_state::mcu_t1_r));
	m_mcu->p1_out_cb().set(FUNC(m79152pc_state::mcu_p1_w));
	m_mcu->p2_out_cb().set(FUNC(m79152pc_state::mcu_p2_w));
	m_mcu->p2_out_cb().append("ctc", FUNC(z80ctc_device::trg0)).bit(6); // determines beep duration
	m_mcu->p2_out_cb().append("ctc", FUNC(z80ctc_device::trg3)).bit(6);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(50 * 720 * 324, 720, 0, 640, 324, 0, 250);
	m_screen->set_screen_update(FUNC(m79152pc_state::screen_update));
	m_screen->set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_m79152pc);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	pit8253_device &pit(PIT8253(config, "pit", 0)); // КР580ВИ53
	pit.set_clk<1>(921600);
	pit.set_clk<2>(921600);
	pit.out_handler<1>().set(m_uart, FUNC(z80sio_device::txcb_w));
	pit.out_handler<2>().set(m_uart, FUNC(z80sio_device::rxcb_w));

	i8212_device &mculatch(I8212(config, "mculatch")); // CEMI UCY74S412
	mculatch.md_rd_callback().set_constant(0);
	mculatch.int_wr_callback().set(m_uart, FUNC(z80sio_device::ctsb_w)).invert();
	mculatch.int_wr_callback().append(FUNC(m79152pc_state::latch_full_w));

	i8255_device &ppi(I8255A(config, "ppi")); // NEC D8255AD-2
	ppi.out_pb_callback().set("printer", FUNC(centronics_device::write_data0)).bit(0);
	ppi.out_pb_callback().append("printer", FUNC(centronics_device::write_data1)).bit(1);
	ppi.out_pb_callback().append("printer", FUNC(centronics_device::write_data2)).bit(2);
	ppi.out_pb_callback().append("printer", FUNC(centronics_device::write_data3)).bit(3);
	ppi.out_pb_callback().append("printer", FUNC(centronics_device::write_data4)).bit(4);
	ppi.out_pb_callback().append("printer", FUNC(centronics_device::write_data5)).bit(5);
	ppi.out_pb_callback().append("printer", FUNC(centronics_device::write_data6)).bit(6);
	ppi.out_pb_callback().append("printer", FUNC(centronics_device::write_data7)).bit(7);
	ppi.out_pc_callback().set("printer", FUNC(centronics_device::write_strobe)).bit(1);

	centronics_device &printer(CENTRONICS(config, "printer", centronics_devices, nullptr));
	printer.ack_handler().set("ppi", FUNC(i8255_device::pc2_w));

	z80ctc_device &ctc(Z80CTC(config, "ctc", 4'000'000));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc.set_clk<2>(921600);
	ctc.zc_callback<2>().set(m_uart, FUNC(z80sio_device::txca_w));
	ctc.zc_callback<2>().append(m_uart, FUNC(z80sio_device::rxca_w));

	// FIXME: Channel A should be the modem channel. Channel B should be a PC keyboard
	// that outputs XT scancodes, which are then rebroadcast through channel A!
	Z80SIO(config, m_uart, 4'000'000); // UB8560D
	m_uart->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_uart->out_txda_callback().set("keyboard", FUNC(rs232_port_device::write_txd));
	m_uart->out_dtra_callback().set("keyboard", FUNC(rs232_port_device::write_dtr));
	m_uart->out_rtsa_callback().set("keyboard", FUNC(rs232_port_device::write_rts));
	m_uart->out_txdb_callback().set("modem", FUNC(rs232_port_device::write_txd));
	m_uart->out_dtrb_callback().set("modem", FUNC(rs232_port_device::write_dtr));
	m_uart->out_rtsb_callback().set("modem", FUNC(rs232_port_device::write_rts));

	rs232_port_device &keyboard(RS232_PORT(config, "keyboard", default_rs232_devices, "keyboard"));
	keyboard.rxd_handler().set(m_uart, FUNC(z80sio_device::rxa_w));
	keyboard.cts_handler().set(m_uart, FUNC(z80sio_device::ctsa_w));
	rs232_port_device &modem(RS232_PORT(config, "modem", default_rs232_devices, nullptr));
	modem.rxd_handler().set(m_uart, FUNC(z80sio_device::rxb_w));
	//modem.cts_handler().set(m_uart, FUNC(z80sio_device::ctsb_w));

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 1000);
	m_beep->add_route(ALL_OUTPUTS, "mono", 0.50);
}

/* ROM definition */
ROM_START( m79152pc )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "left.bin", 0x0000, 0x4000, CRC(8cd677fc) SHA1(7ad28f3ba984383f24a36639ca27fc1eb5a5d002))

	ROM_REGION( 0x1000, "chargen", ROMREGION_INVERT )
	ROM_LOAD( "right.bin", 0x0000, 0x1000, CRC(93f83fdc) SHA1(e8121b3d175c46c02828f43ec071a7d9c62e7c26)) // chargen

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "char.bin",  0x0000, 0x0800, CRC(da3792a5) SHA1(b4a4f0d61d8082b7909a346a5b01494c53cf8d05))

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "7641apc.bin", 0x0000, 0x0200, NO_DUMP)
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY       FULLNAME         FLAGS
COMP( 198?, m79152pc, 0,      0,      m79152pc, m79152pc, m79152pc_state, empty_init, "Mera-Elzab", "MERA 79152 PC", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
