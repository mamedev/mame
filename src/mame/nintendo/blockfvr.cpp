// license:BSD-3-Clause
// copyright-holders:smf
/*
CPU Board TBF-CPU
+--------------------------------------------------------------+
|  1    2    3    4       5    6    7           8        9     |
|                                                              |
|       10  11  12  13  14  15  16  17  18  19 XTAL  20    21  |
|                                                              |
|       22  23  24  25  26  27  28  29      30  31  32  33  34 |
|                                                              |
| P8    35  DSW 36      37  38  39  40  41  42  43  44  45  46 |
|                                                              |
| P7    47  48      49  50  51  52  53  54  55  56  57  58  59 |
|                                                              |
| P6    60  61  62  63  64  65  66  67  68  69  70  71  72  73 |
|                                                              |
|   P5  P4  P3  P2      P1          74  75  76  77  78  79  80 |
+--------------------------------------------------------------+

  1-4                         Mitsubishi M5L2078S M58732S - 8k EPROM (A1-D1, socketed)
  5-7                         NEC UPB8212C - Eight-Bit Input/Output Port
  8                           NEC D8085AC
  9/20/21                     NEC M3624 - 4096 bit PROM (MR005/MR006/unpopulated)
  10/11/12/22/23/24/38/39/52  Texas Instruments SN74LS125AN
  13/14/25/26                 Mitsubishi M58723P - 1024-Bit (256-Word by 4-Bit) Static Random-Access Memory
  15/16/27/28                 NEC D2111AL-4 1Kx4bit 200ns Static Ram
  17/51/75                    Fairchild MB74LS10
  18/37/50/64                 Signetics 74LS74N
  19/31/32/65/76              Texas Instruments SN74LS04N
  XTAL                        6.144 MHZ
  29/71/72/79                 Fairchild MB74LS00
  30                          Mitsubishi M74LS139P
  33                          Texas Instruments SN74LS194AN
  34                          Texas Instruments SN74LS124AN
  35/43/44/56/57/70           Fairchild MB74LS175
  DSW                         OMRON DIP SW8
  36                          National Semiconductor ADC0800PN (socketed)
  40/53/73/80                 Texas Instruments SN74LS93N
  41/46/54/59                 Texas Instruments SN74LS86N
  42/45/55/58                 Texas Instruments SN74LS266N
  47/60                       Fairchild MB434
  48                          Texas Instruments SN74121N
  49                          Mitsubishi M74L139P
  61                          Hitachi HD74LS14P
  62                          National Semiconductor LM380N (socketed)
  63                          Motorola MC14086B
  66/69                       Fairchild MB74LS107
  67/68/74                    Fairchild MB74LS164
  77                          Texas Instruments SN74LS08N
  78                          Texas Instruments SN74LS03N
  P1                          Power Supply Unit
  P2                          Game Counter
  P3
  P4                          Coin Switch
  P5                          TV/Speaker
  P6                          Sub Switch Unit
  P7/P8                       Main Switch Unit
*/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "sound/beep.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"
#include "blockfvr.lh"

namespace {

class blockfvr_state :
	public driver_device
{
public:
	blockfvr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_tilemap(*this, "tilemap"),
		m_beep(*this, "beep%u", 0U),
		m_in(*this, "IN%u", 0U),
		m_adc0800pn(*this, "ADC0800PN_%u", 0U),
		m_start(*this, "led%u", 0U),
		m_ball_x(0x00),
		m_ball_y(0x00)
	{
	}

	void blockfvr(machine_config &config)
	{
		I8085A(config, m_maincpu, 6.144_MHz_XTAL);
		m_maincpu->set_addrmap(AS_PROGRAM, &blockfvr_state::mem_map);

		SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
		m_screen->set_raw(6.144_MHz_XTAL, 384, 0, 256, 264, 0, 192);
		m_screen->set_screen_update(FUNC(blockfvr_state::screen_update));

		GFXDECODE(config, m_gfxdecode, m_palette, gfxdecode);
		PALETTE(config, m_palette, palette_device::MONOCHROME_INVERTED);
		TILEMAP(config, m_tilemap, m_gfxdecode, 1, 8, 12, TILEMAP_SCAN_ROWS, 32, 16).set_info_callback(FUNC(blockfvr_state::tile_info));

		SPEAKER(config, "mono").front_center();
		BEEP(config, m_beep[0], 1332).add_route(ALL_OUTPUTS, "mono", 0.25); // bounce
		BEEP(config, m_beep[1], 666).add_route(ALL_OUTPUTS, "mono", 0.25); // block
		BEEP(config, m_beep[2], 333).add_route(ALL_OUTPUTS, "mono", 0.25); // flashing block
	}

protected:
	virtual void device_resolve_objects() override ATTR_COLD
	{
		m_start.resolve();
	}

	virtual void driver_start() override ATTR_COLD
	{
		save_item(NAME(m_ball_x));
		save_item(NAME(m_ball_y));
	}

	void mem_map(address_map &map) ATTR_COLD
	{
		map.unmap_value_high();
		map(0x0000, 0x0fff).rom();
		map(0x1000, 0x11ff).ram();
		map(0x2000, 0x21ff).ram().w(m_tilemap, FUNC(tilemap_device::write8)).share("tilemap");
		map(0x8000, 0x8000).w(FUNC(blockfvr_state::sound_w));
		map(0x8001, 0x8001).w(FUNC(blockfvr_state::lamp_w));
		map(0x8002, 0x8002).w(FUNC(blockfvr_state::ball_x_w));
		map(0x8003, 0x8003).w(FUNC(blockfvr_state::ball_y_w));
		map(0x8004, 0x8004).r(FUNC(blockfvr_state::vblank_r));
		map(0x8005, 0x8005).r(FUNC(blockfvr_state::adc_r));
		map(0x8006, 0x8006).r(FUNC(blockfvr_state::in_r));
		map(0x8007, 0x8007).portr("DIPSW");
	}

	void sound_w(uint8_t data)
	{
		m_beep[0]->set_state(BIT(data, 0));
		m_beep[1]->set_state(BIT(data, 1));
		m_beep[2]->set_state(BIT(data, 2));
		flip_screen_set(BIT(data, 3));
	}

	void lamp_w(uint8_t data)
	{
		// BIT(data, 0); // TODO: this is set when replay reached (force with rb@0EC3=3a)

		m_start[0] = BIT(data, 1);
		m_start[1] = BIT(data, 2);
	}

	void ball_x_w(uint8_t data)
	{
		m_ball_x = data;
	}

	void ball_y_w(uint8_t data)
	{
		m_ball_y = data;
	}

	uint8_t vblank_r()
	{
		return 0xfe | m_screen->vblank();
	}

	uint8_t adc_r()
	{
		return m_adc0800pn[!flip_screen()]->read();
	}

	uint8_t in_r()
	{
		uint8_t data = (m_in[!flip_screen()]->read() & 1) | (m_in[0]->read() & ~1);

		machine().bookkeeping().coin_counter_w(0, !BIT(data, 3));

		return data;
	}

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
	{
		m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

		int ball_y = (BIT(m_ball_y, 4, 4) * 12) + BIT(m_ball_y, 0, 4);
		bitmap.plot_box(flip_screen() ? 255 - m_ball_x : m_ball_x, flip_screen() ? 191 - ball_y : ball_y, 4, 4, rgb_t::white());
		return 0;
	}

	static constexpr gfx_layout tile_layout =
	{
		8, 12, // 8x12 tiles
		128,   // 128 tiles
		1,     // 1 bits per pixel
		{ 0 }, // no bitplanes
		// x offsets
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		// y offsets
		{ 0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8, 8 * 8, 9 * 8, 10 * 8, 11 * 8 },
		16 * 8 // every tile takes 16 bytes
	};

	static constexpr GFXDECODE_START(gfxdecode)
		GFXDECODE_ENTRY("tiles", 0x0000, tile_layout, 0, 1)
	GFXDECODE_END

	TILE_GET_INFO_MEMBER(tile_info)
	{
		uint32_t code = m_tilemap->basemem_read(tile_index) & 0x7f;

		tileinfo.set(0, code, 1, 0);
	}

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tilemap_device> m_tilemap;
	required_device_array<beep_device, 3> m_beep;
	required_ioport_array<2> m_in;
	required_ioport_array<2> m_adc0800pn;
	output_finder<2> m_start;
	uint8_t m_ball_x;
	uint8_t m_ball_y;
};

static INPUT_PORTS_START( blockfvr )
	PORT_START("DIPSW") // The manual is rotated by 180 degrees
	PORT_DIPNAME(0x01, 0x01, "Replay") PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPNAME(0x06, 0x06, "Replay Points") PORT_DIPLOCATION("DSW:7,6")
	PORT_DIPSETTING(0x00, "1600")
	PORT_DIPSETTING(0x02, "1200")
	PORT_DIPSETTING(0x04, "800")
	PORT_DIPSETTING(0x06, "400")
	PORT_DIPNAME(0x18, 0x18, DEF_STR(Lives)) PORT_DIPLOCATION("DSW:5,4")
	PORT_DIPSETTING(0x00, "6")
	PORT_DIPSETTING(0x08, "5")
	PORT_DIPSETTING(0x10, "4")
	PORT_DIPSETTING(0x18, "3")
	PORT_DIPNAME(0x20, 0x20, "Game A First Rack Flashing Blocks") PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(0x00, "4")
	PORT_DIPSETTING(0x20, "2")
	PORT_DIPNAME(0x40, 0x40, "Game A Blocks Descend") PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(0x00, "8")
	PORT_DIPSETTING(0x40, "4")
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "DSW:1")

	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_START2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_CONFNAME(0x70, 0x60, "Game")
	PORT_CONFSETTING(0x60, "A")
	PORT_CONFSETTING(0x50, "B")
	PORT_CONFSETTING(0x30, "C")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)

	PORT_START("ADC0800PN_0")
	PORT_BIT(0xff, 0x80, IPT_PADDLE) PORT_MINMAX(0x00, 0xff) PORT_INVERT PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CENTERDELTA(0)

	PORT_START("ADC0800PN_1")
	PORT_BIT(0xff, 0x80, IPT_PADDLE) PORT_MINMAX(0x00, 0xff) PORT_INVERT PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CENTERDELTA(0) PORT_PLAYER(2)
INPUT_PORTS_END

ROM_START( blockfvr )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "d1_.4",   0x000000, 0x000400, CRC(c3efd8e7) SHA1(54613d94e8674e711215a8a65a25c455e18bd37a) )
	ROM_LOAD( "c1_.3",   0x000400, 0x000400, CRC(c0540779) SHA1(4773deabdd3352467cc9eb69cc788b8bd22a78ff) )
	ROM_LOAD( "b1_.2",   0x000800, 0x000400, CRC(8ba0436d) SHA1(ae31ece035b1f5841068bfc02762200fd56dba5e) )
	ROM_LOAD( "a1_.1",   0x000c00, 0x000400, CRC(e6de8867) SHA1(9dad61fc26cd6208cc9a3020527f38cf85f41556) )

	ROM_REGION( 0x800, "tiles", 0 )
	ROM_LOAD( "mr005.9",  0x000000, 0x000200, CRC(0b2f27a5) SHA1(65c7f943898df65330b8e6dce866ed86507a62d8) )
	ROM_LOAD( "mr006.20", 0x000200, 0x000200, CRC(eadec4c5) SHA1(2ce6e6216163216097e57295f6dc54ff221382c0) )
	ROM_FILL( 0x400, 0x200, 0xff ) // unpopulated position 21
	ROM_FILL( 0x600, 0x200, 0xff )
ROM_END

} // anonymous namespace

GAMEL(1978, blockfvr, 0, blockfvr, blockfvr, blockfvr_state, empty_init, ROT90, "Nintendo", "Block Fever", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND, layout_blockfvr)
