// license:BSD-3-Clause
// copyright-holders: Bryan McPhail, David Graves

/****************************************************************************

    Gunbuster (c) 1992 Taito

    Driver by Bryan McPhail & David Graves.

    Board Info:

        CPU   : 68EC020 68000
        SOUND : Ensoniq ES5505 + ES5510
        OSC.  : 40.000MHz 16.000MHz 30.47618MHz

        * This board (K11J0717A) uses following chips:
          - TC0470LIN
          - TC0480SCP
          - TC0570SPC
          - TC0260DAR
          - TC0510NIO

    Gunbuster uses a slightly enhanced sprite system from the one
    in Taito Z games.

    The key feature remains the use of a sprite map ROM which allows
    the sprite hardware to create many large zoomed sprites on screen
    while minimizing the main CPU load.

    This feature makes the SZ system complementary to the F3 system
    which, owing to its F2 sprite hardware, is not very well suited to
    3d games. (Taito abandoned the SZ system once better 3d hardware
    platforms were available in the mid 1990s.)

    Gunbuster also uses the TC0480SCP tilemap chip (like the last Taito
    Z game, Double Axle).

    Todo:

        FLIPX support in the video chips is not quite correct - the Taito logo is wrong,
        and the floor in the Doom levels has horizontal scrolling where it shouldn't.

        No networked machine support

***************************************************************************/

#include "emu.h"

#include "taito_en.h"
#include "taitoio.h"
#include "tc0480scp.h"

#include "cpu/m68000/m68020.h"
#include "machine/eepromser.h"
#include "sound/es5506.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


// configurable logging
#define LOG_BADSPRITES     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_BADSPRITES)

#include "logmacro.h"

#define LOGBADSPRITES(...)     LOGMASKED(LOG_BADSPRITES,     __VA_ARGS__)


namespace {

class gunbustr_state : public driver_device
{
public:
	gunbustr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_tc0480scp(*this, "tc0480scp"),
		m_ram(*this,"ram"),
		m_spriteram(*this,"spriteram"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spritemap(*this, "spritemap"),
		m_io_light_x(*this, "LIGHT%u_X", 0U),
		m_io_light_y(*this, "LIGHT%u_Y", 0U),
		m_gun_recoil_pl(*this, "Player%u_Gun_Recoil", 1U),
		m_hit_lamp(*this, "Hit_lamp")

	{
		m_coin_lockout = true;
	}

	void gunbustr(machine_config &config);

	void init_gunbustrj();
	void init_gunbustr();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void prg_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(trigger_irq5);

	void motor_control_w(u32 data);
	u32 gun_r();
	void gun_w(u32 data);
	u32 main_cycle_r();
	void coin_word_w(u8 data);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(gunbustr_interrupt);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, const u32 *primasks, int x_offs, int y_offs);

	required_device<cpu_device> m_maincpu;
	required_device<tc0480scp_device> m_tc0480scp;
	required_shared_ptr<u32> m_ram;
	required_shared_ptr<u32> m_spriteram;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_region_ptr<u16> m_spritemap;

	required_ioport_array<2> m_io_light_x;
	required_ioport_array<2> m_io_light_y;
	output_finder<2> m_gun_recoil_pl;
	output_finder<> m_hit_lamp;

	struct gb_tempsprite
	{
		int gfx = 0;
		int code = 0, color = 0;
		int flipx,flipy = 0;
		int x = 0, y = 0;
		int zoomx = 0, zoomy = 0;
		int primask = 0;
	};

	bool m_coin_lockout;
	std::unique_ptr<gb_tempsprite[]> m_spritelist{};
	emu_timer *m_interrupt5_timer = nullptr;
};


/************************************************************/

void gunbustr_state::video_start()
{
	m_spritelist = std::make_unique<gb_tempsprite[]>(0x4000);

	m_gun_recoil_pl.resolve();
	m_hit_lamp.resolve();
}

/************************************************************
            SPRITE DRAW ROUTINES

We draw a series of small tiles ("chunks") together to
create each big sprite. The spritemap ROM provides the lookup
table for this. The game hardware looks up 16x16 sprite chunks
from the spritemap ROM, creating a 64x64 sprite like this:

     0  1  2  3
     4  5  6  7
     8  9 10 11
    12 13 14 15

(where the number is the word offset into the spritemap ROM).
It can also create 32x32 sprites.

NB: unused portions of the spritemap ROM contain hex FF's.
It is a useful coding check to warn in the log if these
are being accessed. [They can be inadvertently while
spriteram is being tested, take no notice of that.]

Heavy use is made of sprite zooming.

        ***

    Sprite table layout (4 long words per entry)

    ------------------------------------------
     0 | ........ x....... ........ ........ | Flip X
     0 | ........ .xxxxxxx ........ ........ | ZoomX
     0 | ........ ........ .xxxxxxx xxxxxxxx | Sprite Tile
       |                                     |
     2 | ........ ....xx.. ........ ........ | Sprite/tile priority [*]
     2 | ........ ......xx xxxxxx.. ........ | Palette bank
     2 | ........ ........ ......xx xxxxxxxx | X position
       |                                     |
     3 | ........ .....x.. ........ ........ | Sprite size (0=32x32, 1=64x64)
     3 | ........ ......x. ........ ........ | Flip Y
     3 | ........ .......x xxxxxx.. ........ | ZoomY
     3 | ........ ........ ......xx xxxxxxxx | Y position
    ------------------------------------------

    [* 00=over BG1; 01=BG2; 10=BG3; 11=over text ???]

********************************************************/

void gunbustr_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, const u32 *primasks, int x_offs, int y_offs)
{
	int sprites_flipscreen = 0;

	/* pdrawgfx() needs us to draw sprites front to back, so we have to build a list
	   while processing sprite ram and then draw them all at the end */
	struct gb_tempsprite *sprite_ptr = m_spritelist.get();

	for (int offs = (m_spriteram.bytes() / 4 - 4); offs >= 0; offs -= 4)
	{
		u32 data = m_spriteram[offs + 0];
		int flipx =          (data & 0x00800000) >> 23;
		int zoomx =          (data & 0x007f0000) >> 16;
		const u32 tilenum =  (data & 0x00007fff);

		data = m_spriteram[offs + 2];
		const int priority = (data & 0x000c0000) >> 18;
		int color =          (data & 0x0003fc00) >> 10;
		int x =              (data & 0x000003ff);

		data = m_spriteram[offs + 3];
		const int dblsize =  (data & 0x00040000) >> 18;
		int flipy =          (data & 0x00020000) >> 17;
		int zoomy =          (data & 0x0001fc00) >> 10;
		int y =              (data & 0x000003ff);

		color |= 0x80;

		if (!tilenum) continue;

		flipy = !flipy;
		zoomx += 1;
		zoomy += 1;

		y += y_offs;

		// treat coords as signed
		if (x > 0x340) x -= 0x400;
		if (y > 0x340) y -= 0x400;

		x -= x_offs;

		int bad_chunks = 0;
		const int dimension = ((dblsize * 2) + 2);  // 2 or 4
		const int total_chunks = ((dblsize * 3) + 1) << 2;  // 4 or 16
		const int map_offset = tilenum << 2;

		for (int sprite_chunk = 0; sprite_chunk < total_chunks; sprite_chunk++)
		{
			const int j = sprite_chunk / dimension;   // rows
			const int k = sprite_chunk % dimension;   // chunks per row

			int px = k;
			int py = j;
			// pick tiles back to front for x and y flips
			if (flipx)  px = dimension - 1 - k;
			if (flipy)  py = dimension - 1 - j;

			const u32 code = m_spritemap[map_offset + px + (py << (dblsize + 1))];

			if (code == 0xffff)
			{
				bad_chunks += 1;
				continue;
			}

			int curx = x + ((k * zoomx) / dimension);
			int cury = y + ((j * zoomy) / dimension);

			const int zx = x + (((k + 1) * zoomx) / dimension) - curx;
			const int zy = y + (((j + 1) * zoomy) / dimension) - cury;

			if (sprites_flipscreen)
			{
				/* -zx/y is there to fix zoomed sprite coords in screenflip.
				   drawgfxzoom does not know to draw from flip-side of sprites when
				   screen is flipped; so we must correct the coords ourselves. */

				curx = 320 - curx - zx;
				cury = 256 - cury - zy;
				flipx = !flipx;
				flipy = !flipy;
			}

			sprite_ptr->gfx = 0;
			sprite_ptr->code = code;
			sprite_ptr->color = color;
			sprite_ptr->flipx = !flipx;
			sprite_ptr->flipy = flipy;
			sprite_ptr->x = curx;
			sprite_ptr->y = cury;
			sprite_ptr->zoomx = zx << 12;
			sprite_ptr->zoomy = zy << 12;

			if (primasks)
			{
				sprite_ptr->primask = primasks[priority];

				sprite_ptr++;
			}
			else
			{
				m_gfxdecode->gfx(sprite_ptr->gfx)->zoom_transpen(bitmap, cliprect,
						sprite_ptr->code,
						sprite_ptr->color,
						sprite_ptr->flipx, sprite_ptr->flipy,
						sprite_ptr->x, sprite_ptr->y,
						sprite_ptr->zoomx, sprite_ptr->zoomy, 0);
			}
		}

		if (bad_chunks)
			LOGBADSPRITES("Sprite number %04x had %02x invalid chunks\n", tilenum, bad_chunks);
	}

	// this happens only if primsks != nullptr
	while (sprite_ptr != m_spritelist.get())
	{
		sprite_ptr--;

		m_gfxdecode->gfx(sprite_ptr->gfx)->prio_zoom_transpen(bitmap, cliprect,
				sprite_ptr->code,
				sprite_ptr->color,
				sprite_ptr->flipx, sprite_ptr->flipy,
				sprite_ptr->x, sprite_ptr->y,
				sprite_ptr->zoomx, sprite_ptr->zoomy,
				screen.priority(), sprite_ptr->primask, 0);
	}
}



/**************************************************************
                SCREEN REFRESH
**************************************************************/

u32 gunbustr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[5];
	static const u32 primasks[4] = {0xfffc, 0xfff0, 0xff00, 0x0};

	m_tc0480scp->tilemap_update();

	u16 const priority = m_tc0480scp->get_bg_priority();
	layer[0] = (priority & 0xf000) >> 12;   // tells us which bg layer is bottom
	layer[1] = (priority & 0x0f00) >>  8;
	layer[2] = (priority & 0x00f0) >>  4;
	layer[3] = (priority & 0x000f) >>  0;   // tells us which is top
	layer[4] = 4;   // text layer always over bg layers

	screen.priority().fill(0, cliprect);

	/* We have to assume 2nd to bottom layer is always underneath
	   sprites as pdrawgfx cannot yet cope with more than 4 layers */

#ifdef MAME_DEBUG
	if (!machine().input().code_pressed (KEYCODE_Z)) m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 0);
	if (!machine().input().code_pressed (KEYCODE_X)) m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 1);
	if (!machine().input().code_pressed (KEYCODE_C)) m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 2);
	if (!machine().input().code_pressed (KEYCODE_V)) m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[3], 0, 4);
	if (!machine().input().code_pressed (KEYCODE_B)) m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[4], 0, 8);
	if (!machine().input().code_pressed (KEYCODE_N)) draw_sprites(screen, bitmap, cliprect, primasks, 48, -116);
#else
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 0);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 1);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 2);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[3], 0, 4);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[4], 0, 8);    // text layer
	draw_sprites(screen, bitmap, cliprect, primasks, 48, -116);
#endif
	return 0;
}


/*********************************************************************/

TIMER_CALLBACK_MEMBER(gunbustr_state::trigger_irq5)
{
	m_maincpu->set_input_line(5, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(gunbustr_state::gunbustr_interrupt)
{
	m_interrupt5_timer->adjust(m_maincpu->cycles_to_attotime(200000 - 500));
	device.execute().set_input_line(4, HOLD_LINE);
}

void gunbustr_state::coin_word_w(u8 data)
{
	if (m_coin_lockout)
	{
		machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
		machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
	}

	// game does not write a separate counter for coin 2! maybe in linked mode?
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x04);
}

void gunbustr_state::motor_control_w(u32 data)
{
	// Standard value poked into MSW is 0x3c00
	// (0x2000 and zero are written at startup)
	m_gun_recoil_pl[0] = BIT(data, 24);
	m_gun_recoil_pl[1] = BIT(data, 16);
	m_hit_lamp = BIT(data, 18);
}



u32 gunbustr_state::gun_r()
{
	return (m_io_light_x[0]->read() << 24) | (m_io_light_y[0]->read() << 16) |
			(m_io_light_x[1]->read() << 8)  |  m_io_light_y[1]->read();
}

void gunbustr_state::gun_w(u32 data)
{
	// 10000 cycle delay is arbitrary
	m_interrupt5_timer->adjust(m_maincpu->cycles_to_attotime(10000));
}


/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

void gunbustr_state::prg_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x21ffff).ram().share(m_ram); // main CPUA RAM
	map(0x300000, 0x301fff).ram().share(m_spriteram);
	map(0x380000, 0x380003).w(FUNC(gunbustr_state::motor_control_w)); // motor, lamps etc.
	map(0x390000, 0x3907ff).rw("taito_en:dpram", FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w)); // Sound shared RAM
	map(0x400000, 0x400007).rw("tc0510nio", FUNC(tc0510nio_device::read), FUNC(tc0510nio_device::write));
	map(0x500000, 0x500003).rw(FUNC(gunbustr_state::gun_r), FUNC(gunbustr_state::gun_w));
	map(0x800000, 0x80ffff).rw(m_tc0480scp, FUNC(tc0480scp_device::ram_r), FUNC(tc0480scp_device::ram_w));
	map(0x830000, 0x83002f).rw(m_tc0480scp, FUNC(tc0480scp_device::ctrl_r), FUNC(tc0480scp_device::ctrl_w));
	map(0x900000, 0x901fff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0xc00000, 0xc03fff).ram(); // network RAM ?
}

/***********************************************************
             INPUT PORTS (dips in eprom)
***********************************************************/

static INPUT_PORTS_START( gunbustr )
	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)  // Freeze input
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// Light gun inputs

	PORT_START("LIGHT0_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("LIGHT1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_PLAYER(2)

	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_REVERSE PORT_PLAYER(2)
INPUT_PORTS_END


/***********************************************************
                GFX DECODING
**********************************************************/

static const gfx_layout tile16x16_layout =
{
	16,16,  // 16*16 sprites
	RGN_FRAC(1,1),
	4,  // 4 bits per pixel
	{ STEP4(0,16) },
	{ STEP16(0,1) },
	{ STEP16(0,16*4) },
	16*16*4   // every sprite takes 128 consecutive bytes
};

static GFXDECODE_START( gfx_gunbustr )
	GFXDECODE_ENTRY( "sprites", 0x0, tile16x16_layout,  0, 256 )
GFXDECODE_END


/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

void gunbustr_state::gunbustr(machine_config &config)
{
	// basic machine hardware
	M68EC020(config, m_maincpu, XTAL(16'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &gunbustr_state::prg_map);
	m_maincpu->set_vblank_int("screen", FUNC(gunbustr_state::gunbustr_interrupt)); // VBL

	EEPROM_93C46_16BIT(config, "eeprom");

	tc0510nio_device &tc0510nio(TC0510NIO(config, "tc0510nio", 0));
	tc0510nio.read_0_callback().set_ioport("EXTRA");
	tc0510nio.read_2_callback().set_ioport("INPUTS");
	tc0510nio.read_3_callback().set_ioport("SPECIAL");
	tc0510nio.write_3_callback().set("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write)).bit(5);
	tc0510nio.write_3_callback().append("eeprom", FUNC(eeprom_serial_93cxx_device::di_write)).bit(6);
	tc0510nio.write_3_callback().append("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write)).bit(4);
	tc0510nio.write_4_callback().set(FUNC(gunbustr_state::coin_word_w));
	tc0510nio.read_7_callback().set_ioport("SYSTEM");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0, 40*8-1, 2*8, 32*8-1);
	screen.set_screen_update(FUNC(gunbustr_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gunbustr);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 4096);

	TC0480SCP(config, m_tc0480scp, 0);
	m_tc0480scp->set_palette(m_palette);
	m_tc0480scp->set_offsets(0x20, 0x07);
	m_tc0480scp->set_offsets_tx(-1, -1);
	m_tc0480scp->set_offsets_flip(-1, 0);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	taito_en_device &taito_en(TAITO_EN(config, "taito_en", 0));
	taito_en.add_route(0, "speaker", 1.0, 0);
	taito_en.add_route(1, "speaker", 1.0, 1);
}

/***************************************************************************/

ROM_START( gunbustr )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68020 code (CPU A)
	ROM_LOAD32_BYTE( "d27-23.bin", 0x00000, 0x40000, CRC(cd1037cc) SHA1(8005a6a84081ce609e7a605ec8e00e740bfc6846) )
	ROM_LOAD32_BYTE( "d27-22.bin", 0x00001, 0x40000, CRC(475949fc) SHA1(3d5aa3411d2618004902f9d05dff61d9af01ff35) )
	ROM_LOAD32_BYTE( "d27-21.bin", 0x00002, 0x40000, CRC(60950a8a) SHA1(a0336bf6970baa6eaa998a112db840a7fd0452d7) )
	ROM_LOAD32_BYTE( "d27-27.bin", 0x00003, 0x40000, CRC(fd7d3d4c) SHA1(df42e135b1e9b7e371971ba7c8a2e161f3623aa3) )

	ROM_REGION( 0x140000, "taito_en:audiocpu", 0 )
	ROM_LOAD16_BYTE( "d27-25.bin", 0x100000, 0x20000, CRC(c88203cf) SHA1(a918d395b471acdce56dacabd7a1e1e023948365) )
	ROM_LOAD16_BYTE( "d27-24.bin", 0x100001, 0x20000, CRC(084bd8bd) SHA1(93229bc7de4550ead1bb12f666ddbacbe357488d) )

	ROM_REGION( 0x100000, "tc0480scp", 0 ) // SCR 16x16 tiles
	ROM_LOAD32_WORD( "d27-01.bin", 0x00000, 0x80000, CRC(f41759ce) SHA1(30789f43dd09b56399e1dfdb8c6a1e01a21562bd) )
	ROM_LOAD32_WORD( "d27-02.bin", 0x00002, 0x80000, CRC(92ab6430) SHA1(28ed80391c732b09d10c74ed6b78ac76cb62e083) )

	ROM_REGION( 0x400000, "sprites", 0 ) // OBJ 16x16 tiles: each ROM has 1 bitplane
	ROM_LOAD64_WORD_SWAP( "d27-04.bin", 0x000006, 0x100000, CRC(ff8b9234) SHA1(6095b7daf9b7e9a22b0d44d9d6a642ddecb2bd29) )
	ROM_LOAD64_WORD_SWAP( "d27-05.bin", 0x000004, 0x100000, CRC(96d7c1a5) SHA1(93b6a7aea397280a5a778e736d433a85cb7da52c) )
	ROM_LOAD64_WORD_SWAP( "d27-06.bin", 0x000002, 0x100000, CRC(bbb934db) SHA1(9e9b5cf05b9275f1182f5b499b8ee897c4f25b96) )
	ROM_LOAD64_WORD_SWAP( "d27-07.bin", 0x000000, 0x100000, CRC(8ab4854e) SHA1(bd2750cdaa2918e56f8aef3732875952a1eeafea) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 ) // STY, used to create big sprites on the fly
	ROM_LOAD16_WORD( "d27-03.bin", 0x00000, 0x80000, CRC(23bf2000) SHA1(49b29e771a47fcd7e6cd4e2704b217f9727f8299) )

	ROM_REGION16_BE( 0x800000, "taito_en:ensoniq" , ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "d27-08.bin", 0x000000, 0x100000, CRC(7c147e30) SHA1(b605045154967050ec06391798da4afe3686a6e1) ) // C8, C9
	ROM_RELOAD(0x400000,0x100000)
	ROM_LOAD16_BYTE( "d27-09.bin", 0x200000, 0x100000, CRC(3e060304) SHA1(c4da4a94c168c3a454409d758c3ed45babbab170) ) // CA, CB
	ROM_LOAD16_BYTE( "d27-10.bin", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-gunbustr.bin", 0x0000, 0x0080, CRC(ef3685a1) SHA1(899b4b6dd2fd78be3a2ce00a2ef1840de9f122c3) )
ROM_END

ROM_START( gunbustru )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68020 code (CPU A)
	ROM_LOAD32_BYTE( "d27-23.bin", 0x00000, 0x40000, CRC(cd1037cc) SHA1(8005a6a84081ce609e7a605ec8e00e740bfc6846) )
	ROM_LOAD32_BYTE( "d27-22.bin", 0x00001, 0x40000, CRC(475949fc) SHA1(3d5aa3411d2618004902f9d05dff61d9af01ff35) )
	ROM_LOAD32_BYTE( "d27-21.bin", 0x00002, 0x40000, CRC(60950a8a) SHA1(a0336bf6970baa6eaa998a112db840a7fd0452d7) )
	ROM_LOAD32_BYTE( "d27-26.bin", 0x00003, 0x40000, CRC(8a7a0dda) SHA1(59ee7c391c170ab05a3d3d940d833c65e265d9b3) )

	ROM_REGION( 0x140000, "taito_en:audiocpu", 0 )
	ROM_LOAD16_BYTE( "d27-25.bin", 0x100000, 0x20000, CRC(c88203cf) SHA1(a918d395b471acdce56dacabd7a1e1e023948365) )
	ROM_LOAD16_BYTE( "d27-24.bin", 0x100001, 0x20000, CRC(084bd8bd) SHA1(93229bc7de4550ead1bb12f666ddbacbe357488d) )

	ROM_REGION( 0x100000, "tc0480scp", 0 ) // SCR 16x16 tiles
	ROM_LOAD32_WORD( "d27-01.bin", 0x00000, 0x80000, CRC(f41759ce) SHA1(30789f43dd09b56399e1dfdb8c6a1e01a21562bd) )
	ROM_LOAD32_WORD( "d27-02.bin", 0x00002, 0x80000, CRC(92ab6430) SHA1(28ed80391c732b09d10c74ed6b78ac76cb62e083) )

	ROM_REGION( 0x400000, "sprites", 0 ) // OBJ 16x16 tiles: each ROM has 1 bitplane
	ROM_LOAD64_WORD_SWAP( "d27-04.bin", 0x000006, 0x100000, CRC(ff8b9234) SHA1(6095b7daf9b7e9a22b0d44d9d6a642ddecb2bd29) )
	ROM_LOAD64_WORD_SWAP( "d27-05.bin", 0x000004, 0x100000, CRC(96d7c1a5) SHA1(93b6a7aea397280a5a778e736d433a85cb7da52c) )
	ROM_LOAD64_WORD_SWAP( "d27-06.bin", 0x000002, 0x100000, CRC(bbb934db) SHA1(9e9b5cf05b9275f1182f5b499b8ee897c4f25b96) )
	ROM_LOAD64_WORD_SWAP( "d27-07.bin", 0x000000, 0x100000, CRC(8ab4854e) SHA1(bd2750cdaa2918e56f8aef3732875952a1eeafea) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 ) // STY, used to create big sprites on the fly
	ROM_LOAD16_WORD( "d27-03.bin", 0x00000, 0x80000, CRC(23bf2000) SHA1(49b29e771a47fcd7e6cd4e2704b217f9727f8299) )

	ROM_REGION16_BE( 0x800000, "taito_en:ensoniq" , ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "d27-08.bin", 0x000000, 0x100000, CRC(7c147e30) SHA1(b605045154967050ec06391798da4afe3686a6e1) ) // C8, C9
	ROM_RELOAD(0x400000,0x100000)
	ROM_LOAD16_BYTE( "d27-09.bin", 0x200000, 0x100000, CRC(3e060304) SHA1(c4da4a94c168c3a454409d758c3ed45babbab170) ) // CA, CB
	ROM_LOAD16_BYTE( "d27-10.bin", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-gunbustr.bin", 0x0000, 0x0080, CRC(ef3685a1) SHA1(899b4b6dd2fd78be3a2ce00a2ef1840de9f122c3) )
ROM_END

ROM_START( gunbustrj )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68020 code (CPU A)
	ROM_LOAD32_BYTE( "d27-23.bin", 0x00000, 0x40000, CRC(cd1037cc) SHA1(8005a6a84081ce609e7a605ec8e00e740bfc6846) )
	ROM_LOAD32_BYTE( "d27-22.bin", 0x00001, 0x40000, CRC(475949fc) SHA1(3d5aa3411d2618004902f9d05dff61d9af01ff35) )
	ROM_LOAD32_BYTE( "d27-21.bin", 0x00002, 0x40000, CRC(60950a8a) SHA1(a0336bf6970baa6eaa998a112db840a7fd0452d7) )
	ROM_LOAD32_BYTE( "d27-20.bin", 0x00003, 0x40000, CRC(13735c60) SHA1(65b762b28d51b295f6fe190420af566b1b3d4a82) )

	ROM_REGION( 0x140000, "taito_en:audiocpu", 0 )
	ROM_LOAD16_BYTE( "d27-25.bin", 0x100000, 0x20000, CRC(c88203cf) SHA1(a918d395b471acdce56dacabd7a1e1e023948365) )
	ROM_LOAD16_BYTE( "d27-24.bin", 0x100001, 0x20000, CRC(084bd8bd) SHA1(93229bc7de4550ead1bb12f666ddbacbe357488d) )

	ROM_REGION( 0x100000, "tc0480scp", 0 ) // SCR 16x16 tiles
	ROM_LOAD32_WORD( "d27-01.bin", 0x00000, 0x80000, CRC(f41759ce) SHA1(30789f43dd09b56399e1dfdb8c6a1e01a21562bd) )
	ROM_LOAD32_WORD( "d27-02.bin", 0x00002, 0x80000, CRC(92ab6430) SHA1(28ed80391c732b09d10c74ed6b78ac76cb62e083) )

	ROM_REGION( 0x400000, "sprites", 0 ) // OBJ 16x16 tiles: each ROM has 1 bitplane
	ROM_LOAD64_WORD_SWAP( "d27-04.bin", 0x000006, 0x100000, CRC(ff8b9234) SHA1(6095b7daf9b7e9a22b0d44d9d6a642ddecb2bd29) )
	ROM_LOAD64_WORD_SWAP( "d27-05.bin", 0x000004, 0x100000, CRC(96d7c1a5) SHA1(93b6a7aea397280a5a778e736d433a85cb7da52c) )
	ROM_LOAD64_WORD_SWAP( "d27-06.bin", 0x000002, 0x100000, CRC(bbb934db) SHA1(9e9b5cf05b9275f1182f5b499b8ee897c4f25b96) )
	ROM_LOAD64_WORD_SWAP( "d27-07.bin", 0x000000, 0x100000, CRC(8ab4854e) SHA1(bd2750cdaa2918e56f8aef3732875952a1eeafea) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 ) // STY, used to create big sprites on the fly
	ROM_LOAD16_WORD( "d27-03.bin", 0x00000, 0x80000, CRC(23bf2000) SHA1(49b29e771a47fcd7e6cd4e2704b217f9727f8299) )

	ROM_REGION16_BE( 0x800000, "taito_en:ensoniq" , ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "d27-08.bin", 0x000000, 0x100000, CRC(7c147e30) SHA1(b605045154967050ec06391798da4afe3686a6e1) ) // C8, C9
	ROM_RELOAD(0x400000,0x100000)
	ROM_LOAD16_BYTE( "d27-09.bin", 0x200000, 0x100000, CRC(3e060304) SHA1(c4da4a94c168c3a454409d758c3ed45babbab170) ) // CA, CB
	ROM_LOAD16_BYTE( "d27-10.bin", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-gunbustr.bin", 0x0000, 0x0080, CRC(ef3685a1) SHA1(899b4b6dd2fd78be3a2ce00a2ef1840de9f122c3) )
ROM_END

u32 gunbustr_state::main_cycle_r()
{
	if (m_maincpu->pc() == 0x55a && (m_ram[0x3acc / 4] & 0xff000000) == 0)
		m_maincpu->spin_until_interrupt();

	return m_ram[0x3acc/4];
}

void gunbustr_state::init_gunbustr()
{
	// Speedup handler
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x203acc, 0x203acf, read32smo_delegate(*this, FUNC(gunbustr_state::main_cycle_r)));

	m_interrupt5_timer = timer_alloc(FUNC(gunbustr_state::trigger_irq5), this);
}

void gunbustr_state::init_gunbustrj()
{
	init_gunbustr();

	// no coin lockout, perhaps this was a prototype version without proper coin handling?
	m_coin_lockout = false;
}

} // anonymous namespace


GAME( 1992, gunbustr,  0,        gunbustr, gunbustr, gunbustr_state, init_gunbustr, ORIENTATION_FLIP_X, "Taito Corporation Japan",   "Gunbuster (World)", MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE )
GAME( 1992, gunbustru, gunbustr, gunbustr, gunbustr, gunbustr_state, init_gunbustr, ORIENTATION_FLIP_X, "Taito America Corporation", "Gunbuster (US)",    MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE )
GAME( 1992, gunbustrj, gunbustr, gunbustr, gunbustr, gunbustr_state, init_gunbustrj,ORIENTATION_FLIP_X, "Taito Corporation",         "Gunbuster (Japan)", MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE )
