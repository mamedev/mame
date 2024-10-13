// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, Philip Bennett
/***************************************************************************

    Tap a Tune

    driver by Mariusz Wojcieszek and Phil Bennett


    PCB Notes:

    Top board notable (video)

    - Hitachi HD68HC000-12 68000 CPU (24 MHz crystal)
    - Hitachi HD46505SP-2 CRTC
    - 2x Sony CXK581000P-10L RAM
    - 2x Mosel MS6264L-10PC RAM
    - 2x Mosel MS62256L-10PC RAM
    - rom0.u3 / rom1.u12 - 68000 program
    - rom2.u4 / rom3.u13 / rom4.u5 / rom5.u14 - graphics

    Bottom board notable (main / sound / io)

    - Zilog Z0840006PSC Z80 CPU (24 MHz crystal)
    - BSMT2000 custom audio IC
    - rom.u8 Z80 program
    - arom1.u16 BSMT2000 samples
    - 2 banks of 8-position DIP switches
    - red/green/yellow LEDs
    - many connectors for I/O

    The sound and I/O board is used by other redemption games such as
    Colorama, Wheel 'Em In, Super Rock and Bowl, Feed Big Bertha and Sonic
    the Hedgehog (Redemption). The CPU location is marked "68A09" on the
    PCB; some games have a 68B09E here, but others use a Z80 instead.

****************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/bsmt2000.h"
#include "video/mc6845.h"

#include "screen.h"
#include "speaker.h"


namespace {

/*************************************
 *
 *  Driver state
 *
 *************************************/

class tapatune_state : public driver_device
{
public:
	tapatune_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videocpu(*this, "videocpu"),
		m_bsmt(*this, "bsmt"),
		m_videoram(*this, "videoram")
	{
	}

	void tapatune(machine_config &config);
	void tapatune_base(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_videocpu;
	required_device<bsmt2000_device> m_bsmt;

	optional_shared_ptr<uint16_t> m_videoram;

	uint8_t m_paletteram[0x300]{};
	uint16_t m_palette_write_addr = 0;
	rgb_t m_pens[0x100];
	uint8_t m_controls_mux = 0;
	uint8_t m_z80_to_68k_index = 0;
	uint8_t m_z80_to_68k_data = 0;
	uint8_t m_68k_to_z80_index = 0;
	uint8_t m_68k_to_z80_data = 0;
	uint8_t m_z80_data_available = 0;
	uint8_t m_68k_data_available = 0;
	uint8_t m_bsmt_data_l = 0;
	uint8_t m_bsmt_data_h = 0;
	bool m_bsmt_reset = false;

	void crtc_vsync(int state);

	void palette_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t read_from_z80();
	void write_to_z80(uint16_t data);

	uint8_t sound_irq_clear();
	void controls_mux(uint8_t data);
	uint8_t controls_r();
	void write_index_to_68k(uint8_t data);
	void write_data_to_68k(uint8_t data);
	uint8_t read_index_from_68k();
	uint8_t read_data_from_68k();
	void lamps_w(uint8_t data);
	uint8_t status_r();
	void bsmt_data_lo_w(uint8_t data);
	void bsmt_data_hi_w(uint8_t data);
	void bsmt_reg_w(uint8_t data);
	uint8_t special_r();

	MC6845_BEGIN_UPDATE(crtc_begin_update);
	MC6845_UPDATE_ROW(crtc_update_row);

	void maincpu_io_map(address_map &map) ATTR_COLD;
	void maincpu_map(address_map &map) ATTR_COLD;
	void video_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Initialization & interrupts
 *
 *************************************/

void tapatune_state::machine_start()
{
	save_item(NAME(m_paletteram));
	save_item(NAME(m_palette_write_addr));
	save_item(NAME(m_pens));
	save_item(NAME(m_controls_mux));
	save_item(NAME(m_z80_to_68k_index));
	save_item(NAME(m_z80_to_68k_data));
	save_item(NAME(m_68k_to_z80_index));
	save_item(NAME(m_68k_to_z80_data));
	save_item(NAME(m_z80_data_available));
	save_item(NAME(m_68k_data_available));
	save_item(NAME(m_bsmt_data_l));
	save_item(NAME(m_bsmt_data_h));
	save_item(NAME(m_bsmt_reset));
}


void tapatune_state::machine_reset()
{
	// The BSMT2000 is held in reset until the Z80 writes to P1
	m_bsmt_reset = true;
	m_z80_data_available = 0;
	m_68k_data_available = 0;
}


/*************************************
 *
 *  Video hardware
 *
 *************************************/

MC6845_BEGIN_UPDATE( tapatune_state::crtc_begin_update )
{
	// Create the pens
	for (uint32_t i = 0; i < 0x100; i++)
	{
		int r = m_paletteram[3 * i + 0];
		int g = m_paletteram[3 * i + 1];
		int b = m_paletteram[3 * i + 2];

		r = pal6bit(r);
		g = pal6bit(g);
		b = pal6bit(b);

		m_pens[i] = rgb_t(r, g, b);
	}
}


MC6845_UPDATE_ROW( tapatune_state::crtc_update_row )
{
	uint32_t *const dest = &bitmap.pix(y);
	offs_t offs = (ma*2 + ra*0x40)*4;

	uint8_t const *const videoram = reinterpret_cast<uint8_t *>(m_videoram.target());

	for (uint32_t x = 0; x < x_count*4; x++)
	{
		uint8_t pix = videoram[BYTE_XOR_BE(offs + x)];
		dest[2*x] = m_pens[((pix >> 4) & 0x0f)];
		dest[2*x + 1] = m_pens[(pix & 0x0f)];
	}
}


void tapatune_state::palette_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//logerror("Palette write: offset = %02x, data = %04x, mask = %04x\n", offset, data, mem_mask );
	switch(offset)
	{
		case 0: // address register
			m_palette_write_addr = ((data >> 8) & 0xff) * 3;
			break;
		case 1: // palette data
			m_paletteram[m_palette_write_addr++] = (data >> 8) & 0xff;
			break;
		case 2: // unknown?
			break;
	}
}


void tapatune_state::crtc_vsync(int state)
{
	m_videocpu->set_input_line(2, state ? HOLD_LINE : CLEAR_LINE);
}


/*************************************
 *
 *  68000 <-> Z80 comms
 *
 *************************************/

uint16_t tapatune_state::read_from_z80()
{
	m_z80_data_available = 0;
	return ((uint16_t)m_z80_to_68k_data << 8) | (m_z80_to_68k_index);
}


void tapatune_state::write_to_z80(uint16_t data)
{
	m_68k_to_z80_index = data & 0xff;
	m_68k_to_z80_data = (data >> 8) & 0xff;

	if (m_68k_data_available)
	{
		fatalerror("68000 overwrote old Z80 data");
	}
	m_68k_data_available = 1;
}


/*************************************
 *
 *  Z80 <-> 68000 comms
 *
 *************************************/

void tapatune_state::write_index_to_68k(uint8_t data)
{
	m_z80_to_68k_index = data;
}


void tapatune_state::write_data_to_68k(uint8_t data)
{
	// todo, use callback as this will hook up elsewhere on non-video games
	if (m_videocpu)
	{
		m_z80_to_68k_data = data;
		m_z80_data_available = 1;
		m_videocpu->set_input_line(1, HOLD_LINE);
	}
}


uint8_t tapatune_state::read_index_from_68k()
{
	return m_68k_to_z80_index;
}


uint8_t tapatune_state::read_data_from_68k()
{
	m_68k_data_available = 0;

	return m_68k_to_z80_data;
}


/*************************************
 *
 *  Memory maps
 *
 *************************************/

void tapatune_state::video_map(address_map &map)
{
	map(0x000000, 0x2fffff).rom();
	map(0x300000, 0x31ffff).ram().share("videoram");
	map(0x320000, 0x33ffff).ram();
	map(0x400000, 0x400003).rw(FUNC(tapatune_state::read_from_z80), FUNC(tapatune_state::write_to_z80));
	map(0x400010, 0x400011).noprw(); // Watchdog?
	map(0x600000, 0x600005).w(FUNC(tapatune_state::palette_w));
	map(0x800000, 0x800000).w("crtc", FUNC(mc6845_device::address_w));
	map(0x800002, 0x800002).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
}


void tapatune_state::maincpu_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().nopw();
	map(0xe000, 0xffff).ram().share("nvram");
}


void tapatune_state::maincpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(tapatune_state::bsmt_data_lo_w));
	map(0x08, 0x08).w(FUNC(tapatune_state::bsmt_data_hi_w));
	map(0x10, 0x10).w(FUNC(tapatune_state::bsmt_reg_w));
	map(0x18, 0x18).w(FUNC(tapatune_state::controls_mux));
	map(0x20, 0x20).r(FUNC(tapatune_state::sound_irq_clear));
	map(0x28, 0x28).r(FUNC(tapatune_state::status_r));
	map(0x30, 0x30).r(FUNC(tapatune_state::controls_r));
	map(0x38, 0x38).portr("COINS");
	map(0x60, 0x60).w(FUNC(tapatune_state::write_index_to_68k));
	map(0x61, 0x61).w(FUNC(tapatune_state::write_data_to_68k));
	map(0x63, 0x63).w(FUNC(tapatune_state::lamps_w));
	map(0x68, 0x68).r(FUNC(tapatune_state::read_index_from_68k));
	map(0x69, 0x69).r(FUNC(tapatune_state::read_data_from_68k));
	map(0x6b, 0x6b).r(FUNC(tapatune_state::special_r));
}


/*************************************
 *
 *  I/O
 *
 *************************************/

uint8_t tapatune_state::sound_irq_clear()
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return 0;
}


void tapatune_state::controls_mux(uint8_t data)
{
	/*
	    Input multiplexer select and outputs:

	    76543210
	    .......x    Mux A
	    ......x.    Mux B (/Red LED)
	    .....x..    Mux C (/Yellow LED)
	    ....x...    Mux D (/Green LED)
	    ...x....    DOUT1 - High current driver 0
	    ..x.....    DOUT2 - High current driver 1
	    .x......    DOUT3 - High current driver 2
	    x.......    DOUT4 - Ticket dispenser
	*/

	m_controls_mux = data;
}


uint8_t tapatune_state::controls_r()
{
	switch (m_controls_mux & 0xf)
	{
		case 0x07: return ioport("SW4")->read();
		case 0x08: return ioport("SW5")->read();
		case 0x09: return ioport("IN0")->read();
		default:
			return 0xff;
	}
}


uint8_t tapatune_state::special_r()
{
	// Not sure if this is actually correct
	if (m_z80_data_available)
		return m_68k_data_available ? 0x80 : 0;
	else
		return ioport("BUTTONS")->read();
}


void tapatune_state::lamps_w(uint8_t data)
{
	/*
	    Button Lamps:

	    7654 3210
	    .... ...x   Pink
	    .... ..x.   Purple
	    .... .x..   Blue
	    .... x...   Dark Green
	    ...x ....   Light Green
	    ..x. ....   Yellow
	    .x.. ....   Orange
	    x... ....   Red
	*/
}


/*************************************
 *
 *  BSMT communications
 *
 *************************************/

uint8_t tapatune_state::status_r()
{
	return !m_bsmt->read_status() << 7;
}


void tapatune_state::bsmt_data_lo_w(uint8_t data)
{
	m_bsmt_data_l = data;
}


void tapatune_state::bsmt_data_hi_w(uint8_t data)
{
	m_bsmt_data_h = data;

	if (m_bsmt_reset)
	{
		m_bsmt->reset();
		m_bsmt_reset = false;
	}
}


void tapatune_state::bsmt_reg_w(uint8_t data)
{
	m_bsmt->write_reg(data);
	m_bsmt->write_data((m_bsmt_data_h << 8) | m_bsmt_data_l);
}


/*************************************
 *
 *  Input definitions
 *
 *************************************/

static INPUT_PORTS_START( tapatune )
	PORT_START("SW4")
	PORT_DIPNAME( 0x01, 0x01, "Dispense" ) PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x01, "Tickets" )
	PORT_DIPSETTING(    0x00, "Capsules" )
	PORT_DIPNAME( 0x02, 0x02, "Award per" ) PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x02, "Game" )
	PORT_DIPSETTING(    0x00, "Tune" )
	PORT_DIPNAME( 0x1c, 0x1c, "Tickets/capsules per game/tune" ) PORT_DIPLOCATION("SW4:3,4,5")
	PORT_DIPSETTING(    0x1c, "Disabled" )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x14, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x04, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x60, 0x60, "Tune bonus tickets/capsules" ) PORT_DIPLOCATION("SW4:6,7")
	PORT_DIPSETTING(    0x60, "Disabled" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW5")
	PORT_DIPNAME( 0x07, 0x03, "Tune Timer" ) PORT_DIPLOCATION("SW5:1,2,3")
	PORT_DIPSETTING(    0x07, "5 secs" )
	PORT_DIPSETTING(    0x06, "10 secs" )
	PORT_DIPSETTING(    0x05, "15 secs" )
	PORT_DIPSETTING(    0x04, "20 secs" )
	PORT_DIPSETTING(    0x03, "25 secs" )
	PORT_DIPSETTING(    0x02, "30 secs" )
	PORT_DIPSETTING(    0x01, "35 secs" )
	PORT_DIPSETTING(    0x00, "40 secs" )
	PORT_DIPNAME( 0x18, 0x10, "Bad note penalty" ) PORT_DIPLOCATION("SW5:4,5")
	PORT_DIPSETTING(    0x18, "0 secs" )
	PORT_DIPSETTING(    0x10, "1 secs" )
	PORT_DIPSETTING(    0x08, "5 secs" )
	PORT_DIPSETTING(    0x00, "10 secs" )
	PORT_DIPNAME( 0x20, 0x20, "Coins per game" ) PORT_DIPLOCATION("SW5:6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW5:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Burn in Mode" ) PORT_DIPLOCATION("SW5:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Advance (SW3)") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service (SW2)")
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Pink")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Purple")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Blue")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Dark Green")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Light Green")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Yellow")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Orange")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Red")
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void tapatune_state::tapatune_base(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(24'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &tapatune_state::maincpu_map);
	m_maincpu->set_addrmap(AS_IO, &tapatune_state::maincpu_io_map);
	m_maincpu->set_periodic_int(FUNC(tapatune_state::irq0_line_assert), attotime::from_ticks(4 * 4096, XTAL(24'000'000) / 4));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	TICKET_DISPENSER(config, "ticket", attotime::from_msec(100));

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	BSMT2000(config, m_bsmt, XTAL(24'000'000));
	m_bsmt->add_route(0, "lspeaker", 1.0);
	m_bsmt->add_route(1, "rspeaker", 1.0);
}

void tapatune_state::tapatune(machine_config &config)
{
	tapatune_base(config);

	M68000(config, m_videocpu, XTAL(24'000'000) / 2);
	m_videocpu->set_addrmap(AS_PROGRAM, &tapatune_state::video_map);

	config.set_perfect_quantum(m_videocpu);

	hd6845s_device &crtc(HD6845S(config, "crtc", XTAL(24'000'000) / 16));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(5);
	crtc.set_begin_update_callback(FUNC(tapatune_state::crtc_begin_update));
	crtc.set_update_row_callback(FUNC(tapatune_state::crtc_update_row));
	crtc.out_vsync_callback().set(FUNC(tapatune_state::crtc_vsync));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(24'000'000) / 16 * 5, 500, 0, 320, 250, 0, 240);
	screen.set_screen_update("crtc", FUNC(hd6845s_device::screen_update));
}

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( tapatune )
	ROM_REGION( 0x300000, "videocpu", 0 )
	ROM_LOAD16_BYTE("rom1.u12", 0x000000, 0x80000, CRC(1d3ed3f9) SHA1(a42997dffcd4a3e83a5cec75d5bb0c295bd18a8d) )
	ROM_LOAD16_BYTE("rom0.u3",  0x000001, 0x80000, CRC(d76e5dec) SHA1(11b5fa6019e8b891550d37667aa156b113266b9b) )
	ROM_LOAD16_BYTE("rom3.u13", 0x100000, 0x80000, CRC(798e004a) SHA1(8e40ea7bf6e9a67d1c182c3b132a71d1334e1ae8) )
	ROM_LOAD16_BYTE("rom2.u4",  0x100001, 0x80000, CRC(3f073ff7) SHA1(6c34103c49c6c04dad51415037fa9a0c63d98cc2) )
	ROM_LOAD16_BYTE("rom5.u14", 0x280000, 0x40000, CRC(5d3b8765) SHA1(049a115eb28554cc3f5ca813441c42d6b834fc6f) )
	ROM_LOAD16_BYTE("rom4.u5",  0x280001, 0x40000, CRC(2a2eda6a) SHA1(ce86f0da2a41e23a842b3aa1659aad4817de333f) )

	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom.u8", 0x0000, 0x10000, CRC(f5c571d7) SHA1(cb5ef3b2bce9a579b54678962082d0e2fc0f1cd9) )

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD( "arom1.u16",  0x000000, 0x20000, CRC(e51696bc) SHA1(b002f8705ad1877f91a860dddb0ae16b2e73dd15) )
	ROM_CONTINUE(           0x040000, 0x20000 )
	ROM_CONTINUE(           0x080000, 0x20000 )
	ROM_CONTINUE(           0x0c0000, 0x20000 )
	// U21 is not populated
ROM_END

ROM_START( srockbwl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "super_rnb.u8", 0x0000, 0x10000, CRC(ad264e73) SHA1(9f950e7e8ac02619d86d6144e187eddbe299580d) )

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD( "super_rnb_v1.1_4-7-94.u16",  0x000000, 0x20000, CRC(2fe5d1e2) SHA1(5ca18bf369a3021d84a776ce62db44c498119fc9) )
	ROM_CONTINUE(           0x040000, 0x20000 )
	ROM_CONTINUE(           0x080000, 0x20000 )
	ROM_CONTINUE(           0x0c0000, 0x20000 )
	// U21 is not populated
ROM_END


/*

Smart Toss 'Em

romset was marked "Smart Toss 'em"

one of the roms contains the string

SMART INDUSTRIES SMARTBALL   V2.0

a reference to Smart Toss 'em can be found at
http://www.museumofplay.org/online-collections/22/67/109.17072

also contains

"Creative Electronics Software"

*/


ROM_START( smartoss )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s-tossem.u8", 0x0000, 0x10000,CRC(ce07c837) SHA1(5a474cc9d3163385a3dac6da935bc77af67ac85a) )

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD( "s-tossem.u16",  0x000000, 0x20000, CRC(633222dc) SHA1(f95713902920737c9d20aafbdeb975daf98c7078) )
	ROM_CONTINUE(           0x040000, 0x20000 )
	ROM_CONTINUE(           0x080000, 0x20000 )
	ROM_CONTINUE(           0x0c0000, 0x20000 )
	// U21 is not populated?

	// for a different sub-board maybe? or banked CPU data?
	ROM_REGION( 0x40000, "unkdata", 0 )
	ROM_LOAD( "s-tossem.u27", 0x0000, 0x40000, CRC(703d19b7) SHA1(75641d885885a67bd66afa38577c7907fa505b0b) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME(1994, tapatune, 0, tapatune,      tapatune, tapatune_state, empty_init, ROT0, "Moloney Manufacturing Inc. / Creative Electronics and Software", "Tap a Tune", MACHINE_SUPPORTS_SAVE )

// below appear to be mechanical games with the same Z80 board as the above
GAME(1994, srockbwl, 0, tapatune_base, tapatune, tapatune_state, empty_init, ROT0, "Bromley",                                                        "Super Rock and Bowl (V1.1)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(199?, smartoss, 0, tapatune_base, tapatune, tapatune_state, empty_init, ROT0, "Smart Industries / Creative Electronics and Software",           "Smart Toss 'em / Smartball (Ver 2.0)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
