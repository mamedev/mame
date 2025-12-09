// license:BSD-3-Clause
// copyright-holders: Angelo Salese, David Haywood

/**************************************************************************************************

Popo Bear (c) 2000 BMC

TODO:
- BGM seems quite off;
- timer chip (controls auto-animation on title screen + something else during gameplay?);
- complete I/Os (BMC-A00211 PCB has 4 banks of DIPs but only 1 is hooked up?);
- Identify what's on $600000 & $620000;
- Uses tas opcode to sync to irq, from VDP?
- magkengo: doesn't boot, same as popobear would do without the 0x620000 work-around, but it doesn't
  read there.
- qiwang: uses unhandled GFX features, needs correct I/O

===================================================================================================

Qi Wang (2010, Herb Home). English title is 'Chess King'
Popo Bear (2000, Bao Ma Technology Co., LTD)
Hardware Info by Guru
---------------------

BMC-A00211
|---------------------------------------------|
| 42MHz TR5116258  241024 241024 T518 556  SC |
|       TR5116258  241024 241024           SW |
|                                 U10         |
|  |--------| |-------|        6264     6264  |
|  |  BMC   | |  BMC  |                       |
|  |AIA90423| |AIA90610                       |
|J |        | |       |       2         1     |
|A |        | |-------|                       |
|M |--------|              MJ-04.U4  A-3.U3   |
|M                  PLCC44                    |
|A                         MJ-68.U6  MJ-57.U5 |
|                                             |
|                             8         7     |
| ULN2003                                     |
|                          MJ-09.U9 DIP3  DIP1|
|            7805   6295                      |
| TDA2003    VOL    LM324  UM3567   DIP4  DIP2|
|---------------------------------------------|
Notes:
      AIA90610 - Rebadged 68000 CPU. Clock 10.500MHz [42/4]
      AIA90423 - BMC Custom Graphics Chip
        UM3567 - Yamaha YM2413-compatible Sound Chip. Clock 3.500MHz [42/12]
          6295 - Oki M6295 ADPCM Sample Player. Clock 1.000MHz [42/42]. Pin 7 HIGH.
     TR5116258 - 256kB x16-bit DRAM, equivalent to HM514260 \
        241024 - Winbond W241024 128kB x8-bit SRAM          / connected to graphics chip
          6264 - 8kB x8-bit SRAM (68000 RAM). Both RAMs are tied to the Super Cap.
        PLCC44 - 44 pin CPLD (surface scratched)
       1,2,7,8 - Empty Sockets
          T518 - Mitsumi PST518A Master Reset IC
           556 - NE556 Dual Timer
            SC - 5.5V 0.1F Super Capacitor
            SW - Reset Switch / NVRAM Memory Clear
           U10 - Unpopulated position for a 32Mbit/64Mbit TSOP48 Flash ROM (jumper-selectable)
          ROMs - U5/U6 are 27C801 EPROM, other ROMs are 27C010.
                 ROM U5 is unique to this set (only ROM fill change), other ROMs match existing qiwang dump.
        DIP1-4 - 8-position DIP Switch
       TDA2003 - ST TDA2003 10W Audio Amplifier
         LM324 - LM324 Quad Op-Amp
          7805 - LM7805 5V Linear Regulator
       ULN2003 - 7-Channel Darlington Transistor Array
       V-Sync 59.6377Hz
       H-Sync 15.6248kHz

JAMMA CONNECTOR
Component Side   A   B   Solder Side
           GND   1   1   GND
           GND   2   2   GND
           +5v   3   3   +5v
           +5v   4   4   +5v
                 5   5
          +12v   6   6   +12v
                 7   7
    Coin Meter   8   8
                 9   9
       Speaker  10   10  GND
                11   11
           Red  12   12  Green
          Blue  13   13  Syn
           GND  14   14
          Test  15   15
         Coin1  16   16  Coin2
      1P Start  17   17  2P Start
         1P Up  18   18  2P Up
       1P Down  19   19  2P Down
       1P Left  20   20  2P Left
      1P Right  21   21  2P Right
          1P A  22   22  2P A
          1P B  23   23  2P B
          1P C  24   24  2P C
                25   25
                26   26
           GND  27   27  GND
           GND  28   28  GND
**************************************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class popobear_state : public driver_device
{
public:
	popobear_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_spriteram(*this, "spriteram")
		, m_vram(*this, "vram")
		, m_vregs(*this, "vregs")
	{
		m_tilemap_base[0] = 0xf0000;
		m_tilemap_base[1] = 0xf4000;
		m_tilemap_base[2] = 0xf8000;
		m_tilemap_base[3] = 0xfc000;
	}

	void popobear(machine_config &config) ATTR_COLD;
	void qiwang(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_vram;
	required_shared_ptr<uint16_t> m_vregs;

	std::vector<uint16_t> m_vram_rearranged;
	int m_tilemap_base[4]{};
	tilemap_t *m_bg_tilemap[4]{};

	template <unsigned N> TILE_GET_INFO_MEMBER(get_tile_info);

	void irq_ack_w(uint8_t data);
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <uint8_t Mystery_value> uint8_t _620000_r();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);

	void main_map(address_map &map) ATTR_COLD;
	void qiwang_main_map(address_map &map) ATTR_COLD;
};


void popobear_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);

	// HACK: the graphic data for the tiles is in a strange order
	// rearrange it so that we can use it as tiles..
	const u32 swapped_offset = bitswap<19>(offset,
		18,17,16,15,14,13,12,
		8,7,6,5,4,3,2,
		11,10,9, // y tile address bits
		1,0 // x tile address bits
	);

	COMBINE_DATA(&m_vram_rearranged[swapped_offset]);
	m_gfxdecode->gfx(0)->mark_dirty((swapped_offset) / 32);

	// unfortunately tilemaps and tilegfx share the same RAM so we're always dirty if we write to RAM
	m_bg_tilemap[0]->mark_all_dirty();
	m_bg_tilemap[1]->mark_all_dirty();
	m_bg_tilemap[2]->mark_all_dirty();
	m_bg_tilemap[3]->mark_all_dirty();
}

static const gfx_layout char_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ STEP8(0, 8) },
	{ STEP8(0, 64) },
	8*64
};

GFXDECODE_START(gfx_popobear)
	GFXDECODE_RAM( "vram", 0, char_layout, 0, 1 )
GFXDECODE_END

template <unsigned N> TILE_GET_INFO_MEMBER(popobear_state::get_tile_info)
{
	int const base = m_tilemap_base[N];
	int const tileno = m_vram[base / 2 + tile_index];
	int const flipyx = (tileno >> 14);
	tileinfo.set(0, tileno & 0x3fff, 0, TILE_FLIPYX(flipyx));
}

void popobear_state::video_start()
{
	m_vram_rearranged.resize(0x100000 / 2);

	m_gfxdecode->gfx(0)->set_source(reinterpret_cast<uint8_t *>(&m_vram_rearranged[0]));

	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(popobear_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 128, 64);
	m_bg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(popobear_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 128, 64);
	m_bg_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(popobear_state::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 8, 128, 64);
	m_bg_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(popobear_state::get_tile_info<3>)), TILEMAP_SCAN_ROWS, 8, 8, 128, 64);

	m_bg_tilemap[0]->set_transparent_pen(0);
	m_bg_tilemap[1]->set_transparent_pen(0);
	m_bg_tilemap[2]->set_transparent_pen(0);
	m_bg_tilemap[3]->set_transparent_pen(0);

	save_item(NAME(m_vram_rearranged));
}



/*
 * ???? ---- ---- ---- unused?
 * ---- xxxx ---- ---- priority (against other sprites! used to keep the line of characters following you in order)
 * ---- ---- x--- ---- Y direction
 * ---- ---- -x-- ---- X direction
 * ---- ---- --xx ---- size (height & width)
 * ---- ---- ---- xx-- color bank
 * ---- ---- ---- --x- NOT set on the enemy character / characters in your line
 * ---- ---- ---- ---x set on opposite to above?
 */
void popobear_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t* vram = reinterpret_cast<uint8_t *>(m_spriteram.target());

	for (int drawpri = 0xf; drawpri >= 0x0; drawpri--)
	{
		// 0x106 = 8 x 8
		// 0x*29 = 32 x 32
		for (int i = 0x800 - 8; i >= 0; i -= 8)
		{
			uint16_t *sprdata = &m_spriteram[(0x7f800 + i) / 2];

			int const param = sprdata[0];
			int const pri = (param & 0x0f00) >> 8;

			// we do this because it's sprite<->sprite priority,
			if (pri != drawpri)
				continue;

			int y = sprdata[1];
			int x = sprdata[2];
			int spr_num = sprdata[3];

			int const width = 8 << ((param & 0x30) >> 4);
			int const height = width; // sprites are always square?

			int color_bank = ((param & 0xc) >> 2);
			int const x_dir = param & 0x40;
			int const y_dir = param & 0x80;

			if (x & 0x8000) x -= 0x10000;
			if (y & 0x8000) y -= 0x10000;

			if (param & 0xf000) color_bank = (machine().rand() & 0x3);



			int add_it = 0;

			// this isn't understood, not enough evidence.
			switch (param & 3)
			{
				case 0x0: // girls in the intro (qiwang)
				//color_bank = (machine().rand() & 0x3);
				add_it = color_bank * 0x40;
				break;

				case 0x1: // butterflies in intro, enemy characters, line of characters, stage start text
				//color_bank = (machine().rand() & 0x3);
				add_it = color_bank * 0x40;
				break;

				case 0x2: // characters in intro, main player, powerups, timer, large dancing chars between levels (0x3f?)
				//color_bank = (machine().rand() & 0x3);
				add_it = color_bank * 0x40;
				break;

				case 0x3: // letters on GAME OVER need this..
				add_it = color_bank * 0x40;
				add_it += 0x20;
				break;
			}

			if (param == 0) // this avoids some glitches during the intro in popobear, when the panda gets stunned
				continue;

			spr_num <<= 3;

			for (int yi = 0; yi < height; yi++)
			{
				int const y_draw = (y_dir) ? y + ((height - 1) - yi) : y + yi;

				for (int xi = 0; xi < width; xi++)
				{
					uint8_t const pix = vram[BYTE_XOR_BE(spr_num)];
					int const x_draw = (x_dir) ? x + ((width - 1) - xi) : x + xi;

					if (cliprect.contains(x_draw, y_draw))
					{
						// this is a bit strange, pix data is basically 8-bit
						// but we have to treat 0x00, 0x40, 0x80, 0xc0
						// see scores when you collect an item, must be at least steps of 0x40 or one of the female panda gfx between levels breaks.. might depend on lower bits?
						// granularity also means colour bank is applied *0x40
						// and we have 2 more possible colour bank bits
						// colours on game over screen are still wrong without the weird param kludge above
						if (pix & 0x3f)
						{
							bitmap.pix(y_draw, x_draw) = m_palette->pen(((pix + (add_it)) & 0xff) + 0x100);
						}
					}

					spr_num++;
				}
			}
		}
	}
}

uint32_t popobear_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	const rectangle &visarea = screen.visible_area();
	rectangle clip = visarea;

	//popmessage("%04x",m_vregs[0/2]);
	uint16_t* vreg = m_vregs;

	// popmessage("%04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x",vreg[0x00],vreg[0x01],vreg[0x02],vreg[0x03],vreg[0x04],vreg[0x05],vreg[0x06], vreg[0x07],vreg[0x08],vreg[0x09],vreg[0x0a],vreg[0x0b],m_vregs[0x0c],m_vregs[0x0d],vreg[0x0e],vreg[0x0f]);

	// vreg[0x00] also looks like it could be some enable registers
	// 0x82ff - BMC logo
	// 0x8aff - some attract scenes (no sprites)
	// 0x8bff - game attract scenes etc. (sprites)

	// vreg[0x01] is always
	// 0xfefb



	// these are more than just enable, they get written with 0x0d and 0x1f (and 0x00 when a layer is off)
	// seems to be related to the linescroll mode at least? maybe sizes?
	int const enable0 = (m_vregs[0x0c] & 0xff00) >> 8;
	int const enable1 = (m_vregs[0x0c] & 0x00ff) >> 0;
	int const enable2 = (m_vregs[0x0d] & 0xff00) >> 8;
	int const enable3 = (m_vregs[0x0d] & 0x00ff) >> 0;

	if ((enable0 != 0x00) && (enable0 != 0x0d) && (enable0 != 0x1f)) logerror("unknown enable0 value %02x\n", enable0);
	if ((enable1 != 0x00) && (enable1 != 0x0d) && (enable1 != 0x1f)) logerror("unknown enable1 value %02x\n", enable1);
	if ((enable2 != 0x00) && (enable2 != 0x0d)) logerror("unknown enable2 value %02x\n", enable2);
	if ((enable3 != 0x00) && (enable3 != 0x0d)) logerror("unknown enable3 value %02x\n", enable3);


	// for popobear, the lower 2 tilemaps use regular scrolling. qiwang doesn't seem to agree
	m_bg_tilemap[2]->set_scrollx(0, vreg[0x07]);
	m_bg_tilemap[2]->set_scrolly(0, vreg[0x08]);

	m_bg_tilemap[3]->set_scrollx(0, vreg[0x09]);
	m_bg_tilemap[3]->set_scrolly(0, vreg[0x0a]);

	if (enable3) m_bg_tilemap[3]->draw(screen, bitmap, cliprect, 0, 0);
	if (enable2) m_bg_tilemap[2]->draw(screen, bitmap, cliprect, 0, 0);

	// the upper 2 tilemaps have a lineselect / linescroll logic

	int scrollbase;
	int scrollbase2;

	if (enable1 == 0x1f)
	{
		scrollbase = 0xdf600;
		scrollbase2 = 0xdf800;

		for (int line = 0; line < 240; line++)
		{
			uint16_t const val = m_vram[scrollbase / 2 + line];
			uint16_t const upper = (m_vram[scrollbase2 / 2 + line] & 0xff00) >> 8;

			clip.sety(line, line);

			m_bg_tilemap[1]->set_scrollx(0, (val & 0x00ff) | (upper << 8));
			m_bg_tilemap[1]->set_scrolly(0, ((val & 0xff00) >> 8) - line);

			m_bg_tilemap[1]->draw(screen, bitmap, clip, 0, 0);
		}
	}
	else if (enable1 != 0x00)
	{
		m_bg_tilemap[1]->set_scrollx(0, 0);
		m_bg_tilemap[1]->set_scrolly(0, 0);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	}

	if (enable0 == 0x1f)
	{
		scrollbase = 0xdf400;
		scrollbase2 = 0xdf800;

		for (int line = 0; line < 240; line++)
		{
			uint16_t const val = m_vram[scrollbase / 2 + line];
			uint16_t const upper = (m_vram[scrollbase2 / 2 + line] & 0x00ff) >> 0;

			clip.sety(line, line);

			m_bg_tilemap[0]->set_scrollx(0, (val & 0x00ff) | (upper << 8));
			m_bg_tilemap[0]->set_scrolly(0, ((val & 0xff00) >> 8) - line);

			m_bg_tilemap[0]->draw(screen, bitmap, clip, 0, 0);
		}
	}
	else if (enable0 != 0x00)
	{
		m_bg_tilemap[0]->set_scrollx(0, 0);
		m_bg_tilemap[0]->set_scrolly(0, 0);
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	}

	if (BIT(m_vregs[0x00], 8))
		draw_sprites(bitmap, cliprect);

	return 0;
}

void popobear_state::irq_ack_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
	{
		if (BIT(data, i))
			m_maincpu->set_input_line(i, CLEAR_LINE);
	}
}

template <uint8_t Mystery_value>
uint8_t popobear_state::_620000_r()
{
	// TODO: checked while flipping bit 0 in clock select fashion at POST
	// refuses to boot with either bits 1-2 high.
	return Mystery_value;
}

void popobear_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x03ffff).rom();
	map(0x210000, 0x21ffff).ram();
	map(0x280000, 0x2fffff).ram().share(m_spriteram); // unknown boundaries
	map(0x300000, 0x3fffff).ram().w(FUNC(popobear_state::vram_w)).share(m_vram); // tile definitions + tilemaps

	// TODO: is the 48xxxx block entirely from AIA90423?
	map(0x480000, 0x48001f).ram().share(m_vregs);
	map(0x480031, 0x480031).w(FUNC(popobear_state::irq_ack_w));
	map(0x480034, 0x480035).nopr(); // uses bset/bclr to write, which causes a read (ignored)
	map(0x480035, 0x480035).lw8(
		NAME([this] (offs_t offset, uint8_t data) {
			// "coin meter" in pinout
			machine().bookkeeping().coin_counter_w(0, BIT(data, 1));
		})
	);
	map(0x48003a, 0x48003b).lr16(
		NAME([this] (offs_t offset) -> u16 {
			// TODO: absolute guess, controls title screen faces animation
			// Most likely Free Running Timer
			// - PC=039532 latches in $2100b5-b2 with $48003a / $480022 (upper bytes)
			// - PC=039f4a Reads $48003a (word), writes contents of $480022 (word),
			//   ANDs with 0xf for title screen anim.
			const u8 frame_count = (m_maincpu->total_cycles() >> 8) & 0xf;
			return frame_count;
		})
	);

	map(0x480400, 0x4807ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

	map(0x500000, 0x500001).portr("IN0");
	map(0x520000, 0x520001).portr("IN1");
	map(0x540001, 0x540001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x550000, 0x550003).w("ymsnd", FUNC(ym2413_device::write)).umask16(0x00ff);

//  map(0x600000, 0x600001).nopw(); // activated during transitions, bits 0-3
	map(0x620000, 0x620000).r(FUNC(popobear_state::_620000_r<0x09>));
	map(0x800000, 0xbfffff).rom().region("gfx_data", 0);
}

void popobear_state::qiwang_main_map(address_map &map)
{
	main_map(map);

	map(0x500000, 0x500001).lr16(NAME([] () -> uint16_t { return 0x0000; })); // TODO: while booting, it specifically checks for this not to return 0xffff. Any other value is ok
	map(0x620000, 0x620000).r(FUNC(popobear_state::_620000_r<0x01>));
}


// TODO: unconfirmed diplocations
static INPUT_PORTS_START( popobear )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x0e, "Coin_A" ) PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x1e00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x2000, 0x2000, "Service?" ) // hangs if flipped on during attract
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("DSW2") // TODO: where are this read?
	PORT_DIPNAME( 0x01, 0x00, "Arrow" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW2:2" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW2:3" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW2:4" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW2:5" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DSW2:6" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW2:7" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW2:8" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


TIMER_DEVICE_CALLBACK_MEMBER(popobear_state::scanline_cb)
{
	int const scanline = param;

	// vblank-in
	// Order is trusted, 5 as vblank-in makes title mosaic-esque rotation to draw incorrectly
	if (scanline == 240)
		m_maincpu->set_input_line(3, ASSERT_LINE);

	// vblank-out
	if (scanline == 0)
		m_maincpu->set_input_line(5, ASSERT_LINE);

	// TODO: actually a timer irq, tied with YM2413 sound chip (controls BGM tempo)
	// the YM2413 doesn't have interrupts?
	if (scanline == 64 || scanline == 192)
		m_maincpu->set_input_line(2, ASSERT_LINE);
}

void popobear_state::popobear(machine_config &config)
{
	M68000(config, m_maincpu, 42_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &popobear_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(popobear_state::scanline_cb), "screen", 0, 1);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(59.64);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	m_screen->set_screen_update(FUNC(popobear_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->set_size(128*8, 32*8);
	m_screen->set_visarea(0, 479, 0, 239);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 512);

	SPEAKER(config, "mono").front_center();

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_popobear);

	YM2413(config, "ymsnd", 42_MHz_XTAL / 12).add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, "oki", 42_MHz_XTAL / 42, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void popobear_state::qiwang(machine_config &config)
{
	popobear(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &popobear_state::qiwang_main_map);
}

ROM_START( popobear )
	ROM_REGION( 0x040000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "popobear_en-a-401_1.6.u4", 0x000000, 0x020000, CRC(0568af9c) SHA1(920531dbc4bbde2d1db062bd5c48b97dd50b7185) )
	ROM_LOAD16_BYTE( "popobear_en-a-301_1.6.u3", 0x000001, 0x020000, CRC(b934adf6) SHA1(93431c7a19af812b549aad35cc1176a81805ffab) )

	ROM_REGION16_BE( 0x400000, "gfx_data", 0 )
	ROM_LOAD16_BYTE( "popobear_en-a-601.u6",     0x000000, 0x100000, CRC(84fa9f3f) SHA1(34dd7873f88b0dae5fb81fe84e82d2b6b49f7332) )
	ROM_LOAD16_BYTE( "popobear_en-a-501.u5",     0x000001, 0x100000, CRC(185901a9) SHA1(7ff82b5751645df53435eaa66edce589684cc5c7) )
	ROM_LOAD16_BYTE( "popobear_en-a-801.u8",     0x200000, 0x100000, CRC(2760f2e6) SHA1(58af59f486c9df930f7c124f89154f8f389a5bd7) )
	ROM_LOAD16_BYTE( "popobear_en-a-701.u7",     0x200001, 0x100000, CRC(45eba6d0) SHA1(0278602ed57ac45040619d590e6cc85e2cfeed31) )

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "popobear_ta-a-901.u9", 0x00000, 0x40000,  CRC(f1e94926) SHA1(f4d6f5b5811d90d0069f6efbb44d725ff0d07e1c) )
ROM_END

// exactly same PCB as popobear.
// All labels have 棋王 prepended to what's below, with the exception of mj-57 which has a sticker 'BMC 棋王' on the upper part of the label
ROM_START( qiwang )
	ROM_REGION( 0x040000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mj-04.u4", 0x000000, 0x020000, CRC(03a0d290) SHA1(d8fb1e6780d31ebf8cdc6ae14301d1f8c25380c6) )
	ROM_LOAD16_BYTE( "mj-03.u3", 0x000001, 0x020000, CRC(3cf3ff12) SHA1(dd4347b44a45822e7bfddffb0afadd65d398bea6) )
	// u1 and u2 not populated

	ROM_REGION16_BE( 0x400000, "gfx_data", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "mj-68.u6", 0x000000, 0x100000, CRC(9692bb92) SHA1(6282054a41eda1b1fc1de5096bc2440a386d9f99) )
	ROM_LOAD16_BYTE( "mj-57.u5", 0x000001, 0x100000, CRC(d442e579) SHA1(86d840ce9cd2623f13658f75336847f5df306531) )
	// u7 and u8 not populated

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "mj-09.u9", 0x00000, 0x20000, CRC(dc6326bf) SHA1(88bb77f46f04cc8824a6ac52a2eee32cbe813a26) )
ROM_END

// HERBHOME 20A23-1 PCB. Mostly same components as popobear's (BMC AIA90423, BMC AIA90610, OKI6295, 42 MHz XTAL, etc.), different layout.
// Has a Altera MAX EPM7064SLC84-10 and only 1 8-DIP bank
// Has Herb Home 2003 copyright in ROM but stickers have 2005. TODO: Verify date on title screen when it boots.
ROM_START( magkengo )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "magical_kengo_2005_l1_a_201_u27.u27", 0x000000, 0x020000, CRC(eddfd8b1) SHA1(9eaeff2c0798b2b6b727ad6f3749cc427a52db41) ) // M27C1001
	ROM_LOAD16_BYTE( "magical_kengo_2005_l1_a_101_u26.u26", 0x000001, 0x020000, CRC(449ad8ab) SHA1(625bd2491eb95aa8e39f75db0d0cd01ec649f5b8) ) // M27C1001

	ROM_REGION16_BE( 0x400000, "gfx_data", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "magical_kengo_2005_l8_a_401_u6.u6", 0x000000, 0x100000, CRC(857e34e6) SHA1(17681a684f451c92f577d6cb3443a3801b192df3) ) // M27C801
	ROM_LOAD16_BYTE( "magical_kengo_2005_l8_a_301_u5.u5", 0x000001, 0x100000, CRC(ca03b9fd) SHA1(b9fb407760ed1fc276f42a6dd326edbf97c1660d) ) // M27C801
	// u3 and u4 empty not populated

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "magical_kengo_2005_l4_a_701_u20.u20", 0x00000, 0x80000, CRC(a2c563a7) SHA1(ce33e95887cdf76cb4c5f944bbdd219de1b2c7c0) ) // AMD AM27C040
ROM_END

// HERBHOME 20A23-2 PCB. Same as the 20A23-1 variant but with single interleaved ROM for GFX and the BMC customs scratched off and covered
// with  Global stickers.
// Has Herb Home 2003 copyright in ROM but stickers have 2005. TODO: Verify date on title screen when it boots.
ROM_START( magkengou )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "magical_kengo_2005_l1_b_201_u27.u27", 0x000000, 0x020000, CRC(26e118da) SHA1(7ec4c5e6f56566013033149746655567f10b968a) )
	ROM_LOAD16_BYTE( "magical_kengo_2005_l1_b_101_u26.u26", 0x000001, 0x020000, CRC(30a8c00e) SHA1(7e7ffdc165312103465d67375ba2bac0800d3df4) )

	ROM_REGION16_BE( 0x400000, "gfx_data", ROMREGION_ERASE00 )
	ROM_LOAD( "magical_kengo_2005_l8_a_301_u7.u7", 0x000000, 0x200000, CRC(58e3acba) SHA1(59e105507d336451c3e2517a5a1f853d5edc7375) )
	// u3 and u4 empty not populated

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "magical_kengo_2005_l4_b_701_u20.u20", 0x00000, 0x80000, CRC(f8fb7f0c) SHA1(b7b0d08774334b07c6faab024f05e3cf37934090) )
ROM_END

} // anonymous namespace


// BMC-A00211 PCB
GAME( 2000, popobear,  0,        popobear, popobear, popobear_state, empty_init, ROT0, "BMC",       "PoPo Bear",                          MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_SUPPORTS_SAVE )
GAME( 2010, qiwang,    0,        qiwang,   popobear, popobear_state, empty_init, ROT0, "Herb Home", "Qi Wang",                            MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_SUPPORTS_SAVE )

// HERBHOME 20A23-* PCB
GAME( 2005, magkengo,  0,        popobear, popobear, popobear_state, empty_init, ROT0, "Herb Home", "Magical Kengo 2005 (Ver. 1.2)",      MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_SUPPORTS_SAVE )
GAME( 2005, magkengou, magkengo, popobear, popobear, popobear_state, empty_init, ROT0, "Herb Home", "Magical Kengo 2005 (Ver. USA 1.10)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_SUPPORTS_SAVE )
