// license:BSD-3-Clause
// copyright-holders: Bryan McPhail, David Graves

/***************************************************************************

    Ground Effects / Super Ground FX                    (c) 1993 Taito

    Driver by Bryan McPhail & David Graves.

    Board Info:

    K1100744A
    J1100316A
    Sticker - K11J0744A
    ------------------------------------------------------------------------------------------------
    | 2018  2088        43256      43256   MC68EC020RP25   DIP-SW (8) COM20020  SLIDE-SW  LAN-IN   |           Connector G
    | 2018  2088        D51-21     D51-22              D51-14                                      |           -----------
    | 2018  2088        43256      43256               (PAL16L8)                                   |       Parts    |   Solder
    |                                                  D51-15              ADC0809        LAN-OUT  |       ------------------
    | 2018              D51-23     D51-24              (PAL16L8)                                   |        GND    1 A   GND
    |                                                  D51-16-1                          JP4       |        GND    2 B   GND
    | 2018  TC0570SPC                                  (PAL16L8)            EEPROM.164   1-2       |        +12    3 C   +12
    | 2018                                                                               2-3       |         +5    4 D   +5
    |                                                                 TC0510NIO          2-3     --|         +5    5 E   +5
    |    D51-13       D51-03                                                             2-3     |           +5    6 F   +5
    |                 D51-04                             43256                           1-2     |                 7 H
    |    TC0470LIN    D51-05                                                             1-2     --|        +13    8 J   +13
    |                 D51-06                             43256   TC0650FCA               1-2       |        +13    9 K   +13
    |                 D51-07                                                             2-3       |      SPK_R+  10 L  SPK_R-
    |    TC0580PIV              43256                    43256      2018                 2-3       |      SPK_L+  11 M  SPK_L-
    |                           43256     D51-17                                                   |   SPK_REAR-  12 N  SPK_REAR-
    | 514256          TC0480SCP           (PAL16L8)                                                |              13 P
    | 514256                                                                                      G|         RED  14 R  GREEN
    | 514256                              D51-10                                                   |        BLUE  15 S  SYNC
    | 514256   D51-09                     D51-11          TC0620SCC                                |       V-GND  16 T
    | 514256   D51-08                     D51012                                                   |      METER1  17 U  METER2
    | 514256          43256                                                                        |    LOCKOUT1  18 V  LOCKOUT2
    |          MB8421                                        43256                                 |              19 W
    | D51-18   MB8421             ENSONIC                                                        --|       COIN1  20 X  COIN2
    | (PAL20L8)       D51-29      OTISR2                     43256      MB87078                  |       SERVICE  21 Y  TEST
    |                         D51-01                                                             |                22 Z
    | D51-19          43256           ENSONIC     ENSONIC                                        --|    SHIFT UP  23 a  BRAKE
    | (PAL20L8)               D51-02  SUPER GLU   ESP-R6    TC511664    TL074  TL074               |   HANDLE VR  24 b  ACCEL VR
    |                 D51-30                                                                       |    SHIFT DN  25 c
    | MC68000P12F16MHz                                                  TDA1543 TDA1543            |              26 d
    |                 40MHz    16MHz  30.476MHz    MC68681                                         |         GND  27 e  GND
    | D51-20                                                                                       |         GND  28 f  GND
    | (PAL20L8)                                                                                    |
    ------------------------------------------------------------------------------------------------


    Ground Effects combines the sprite system used in Taito Z games with
    the TC0480SCP tilemap chip plus some features from the Taito F3 system.
    It has an extra TC0620SCC tilemap chip which is a 6bpp version of the
    TC0100SCN (check the inits), like Under Fire.

    Ground Effects is effectively a 30Hz game - though the vblank interrupts
    still come in at 60Hz, the game uses a hardware frame counter to limit
    itself to 30Hz (it only updates things like the video registers every
    other vblank).  There isn't enough CPU power in a 20MHz 68020 to run
    this game at 60Hz.

    Ground Effects has a network mode - probably uses IRQ 6 and the unknown
    ports at 0xc00000.

***************************************************************************/

#include "emu.h"

#include "taito_en.h"
#include "taitoio.h"
#include "tc0100scn.h"
#include "tc0480scp.h"

#include "cpu/m68000/m68020.h"
#include "machine/adc0808.h"
#include "machine/eepromser.h"
#include "sound/es5506.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class groundfx_state : public driver_device
{
public:
	groundfx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_ram(*this, "ram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_tc0620scc(*this, "tc0620scc"),
		m_tc0480scp(*this, "tc0480scp"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spritemap(*this, "spritemap")
	{ }

	void groundfx(machine_config &config);
	void init_groundfx();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<u32> m_ram;
	required_shared_ptr<u32> m_spriteram;

	required_device<cpu_device> m_maincpu;
	required_device<tc0620scc_device> m_tc0620scc;
	required_device<tc0480scp_device> m_tc0480scp;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_region_ptr<u16> m_spritemap;

	struct gfx_tempsprite
	{
		u8 gfx = 0U;
		u32 code = 0U, color = 0U;
		bool flipx = false, flipy = false;
		int x = 0, y = 0;
		int zoomx = 0, zoomy = 0;
		u8 pri = 0;
	};

	u16 m_frame_counter = 0U;
	u8 m_port_sel = 0U;
	std::unique_ptr<gfx_tempsprite[]> m_spritelist{};
	u16 m_rotate_ctrl[8]{};
	rectangle m_hack_cliprect{};

	void rotate_control_w(offs_t offset, u16 data);
	void motor_control_w(u32 data);
	u32 irq_speedup_r();
	int frame_counter_r();
	void coin_word_w(u8 data);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int do_hack, int x_offs, int y_offs);

	void prg_map(address_map &map) ATTR_COLD;
};


/******************************************************************/

void groundfx_state::video_start()
{
	m_spritelist = std::make_unique<gfx_tempsprite[]>(0x4000);

	save_item(NAME(m_frame_counter));
	save_item(NAME(m_port_sel));
	save_item(NAME(m_rotate_ctrl));

	// Hack
	m_hack_cliprect.set(69, 250, 24 + 5, 24 + 44);
}

/***************************************************************
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

    [*  00=over BG0, 01=BG1, 10=BG2, 11=BG3 ]
    [or 00=over BG1, 01=BG2, 10=BG3, 11=BG3 ]

***************************************************************/

void groundfx_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int do_hack, int x_offs, int y_offs)
{
	int sprites_flipscreen = 0;
	static const u32 primasks[4] = {0xffff, 0xfffc, 0xfff0, 0xff00 };

	/* pdrawgfx() needs us to draw sprites front to back, so we have to build a list
	   while processing sprite ram and then draw them all at the end */
	struct gfx_tempsprite *sprite_ptr = m_spritelist.get();

	for (int offs = (m_spriteram.bytes() / 4 - 4); offs >= 0; offs -= 4)
	{
		u32 data = m_spriteram[offs + 0];
		int flipx =          (data & 0x00800000) >> 23;
		int zoomx =          (data & 0x007f0000) >> 16;
		const u32 tilenum =  (data & 0x00007fff);

		data = m_spriteram[offs + 2];
		const int priority = (data & 0x000c0000) >> 18;
		u32 color =          (data & 0x0003fc00) >> 10;
		int x =              (data & 0x000003ff);

		data = m_spriteram[offs + 3];
		const int dblsize =  (data & 0x00040000) >> 18;
		int flipy =          (data & 0x00020000) >> 17;
		int zoomy =          (data & 0x0001fc00) >> 10;
		int y =              (data & 0x000003ff);

//      color |= (0x100 + (priority << 6));     // priority bits select color bank
		color /= 2;     // as sprites are 5bpp
		flipy = !flipy;
		y = (-y & 0x3ff);

		if (!tilenum) continue;

		flipy = !flipy;
		zoomx += 1;
		zoomy += 1;

		y += y_offs;

		// treat coords as signed
		if (x > 0x340) x -= 0x400;
		if (y > 0x340) y -= 0x400;

		x -= x_offs;

		const int dimension = ((dblsize * 2) + 2);  // 2 or 4
		const int total_chunks = ((dblsize * 3) + 1) << 2;  // 4 or 16
		const int map_offset = tilenum << 2;

		{
			for (int sprite_chunk = 0; sprite_chunk < total_chunks; sprite_chunk++)
			{
				const int j = sprite_chunk / dimension;   // rows
				const int k = sprite_chunk % dimension;   // chunks per row

				int px = k;
				int py = j;
				// pick tiles back to front for x and y flips
				if (flipx)  px = dimension - 1 - k;
				if (flipy)  py = dimension - 1 - j;

				const u16 code = m_spritemap[map_offset + px + (py << (dblsize + 1))];

				if (code == 0xffff)
				{
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
				sprite_ptr->pri = priority;
				sprite_ptr++;
			}
		}
	}

	// this happens only if primsks != nullptr
	while (sprite_ptr != m_spritelist.get())
	{
		const rectangle *clipper;

		sprite_ptr--;

		if (do_hack && sprite_ptr->pri == 1 && sprite_ptr->y < 100)
			clipper = &m_hack_cliprect;
		else
			clipper = &cliprect;

		m_gfxdecode->gfx(sprite_ptr->gfx)->prio_zoom_transpen(bitmap, *clipper,
				sprite_ptr->code,
				sprite_ptr->color,
				sprite_ptr->flipx, sprite_ptr->flipy,
				sprite_ptr->x, sprite_ptr->y,
				sprite_ptr->zoomx, sprite_ptr->zoomy,
				screen.priority(), primasks[sprite_ptr->pri], 0);
	}
}

/**************************************************************
                SCREEN REFRESH
**************************************************************/

u32 groundfx_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[5];
	u8 scclayer[3];

	m_tc0620scc->tilemap_update();
	m_tc0480scp->tilemap_update();

	const u16 priority = m_tc0480scp->get_bg_priority();

	layer[0] = (priority & 0xf000) >> 12;   // tells us which bg layer is bottom
	layer[1] = (priority & 0x0f00) >>  8;
	layer[2] = (priority & 0x00f0) >>  4;
	layer[3] = (priority & 0x000f) >>  0;   // tells us which is top
	layer[4] = 4;   // text layer always over bg layers

	scclayer[0] = m_tc0620scc->bottomlayer();
	scclayer[1] = scclayer[0] ^ 1;
	scclayer[2] = 2;

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);   // wrong color?

	m_tc0620scc->tilemap_draw(screen, bitmap, cliprect, scclayer[0], TILEMAP_DRAW_OPAQUE, 0);
	m_tc0620scc->tilemap_draw(screen, bitmap, cliprect, scclayer[1], 0, 0);

	/*  BIG HACK!

	    The rear view mirror is a big priority trick - the text
	    layer of TC0100SCN is used as a stencil to display
	    the bottom layer of TC0480SCP and a particular sprite
	    priority.  These never appear outside of the stencil.

	    I'm not sure how the game turns this effect on/off
	    (the 480 layer is used normally in the frontend
	    of the game).

	    I haven't implemented it properly yet, instead I'm
	    doing a hacky cliprect around the rearview and drawing
	    its contents the usual way.

	*/
	if (m_tc0620scc->ram_r(0x4090 / 2) || m_tc0620scc->ram_r(0x4092 / 2) ||
			((m_tc0480scp->ram_r(0x20 / 2) == 0x24) && (m_tc0480scp->ram_r(0x22 / 2) == 0x0866)))  // Anything in text layer - really stupid hack
	{
		m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);
		m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);
		m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[3], 0, 8);

		//m_tc0620scc->tilemap_draw(screen, bitmap, cliprect, 0, scclayer[2], 0, 0);

		if ((m_tc0480scp->ram_r(0x20 / 2) != 0x24) && (m_tc0480scp->ram_r(0x22 / 2) != 0x0866)) // Stupid hack for start of race
			m_tc0480scp->tilemap_draw(screen, bitmap, m_hack_cliprect, layer[0], 0, 0);
		draw_sprites(screen, bitmap, cliprect, 1, 44, -574);
	}
	else
	{
		m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[0], 0, 1);
		m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);
		m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);
		m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[3], 0, 8);

		m_tc0620scc->tilemap_draw(screen, bitmap, cliprect, scclayer[2], 0, 0);

		draw_sprites(screen, bitmap, cliprect, 0, 44, -574);
	}

	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[4], 0, 0);    // TC0480SCP text layer
	return 0;
}


/**********************************************************
            GAME INPUTS
**********************************************************/

int groundfx_state::frame_counter_r()
{
	return m_frame_counter;
}

void groundfx_state::coin_word_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x08);
}

void groundfx_state::rotate_control_w(offs_t offset, u16 data) // only a guess that it's rotation
{
	if (offset & 1)
	{
		m_rotate_ctrl[m_port_sel] = data;
		return;
	}
	else
	{
		m_port_sel = data & 0x7;
	}
}

void groundfx_state::motor_control_w(u32 data)
{
/*
    Standard value poked is 0x00910200 (we ignore LSB and MSB
    which seem to be always zero)

    0x0, 0x8000 and 0x9100 are written at startup

    Two bits are written in test mode to this middle word
    to test gun vibration:

    ........ .x......   P1 gun vibration
    ........ x.......   P2 gun vibration
*/
}


/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

void groundfx_state::prg_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x200000, 0x21ffff).ram().share(m_ram); // main CPUA RAM
	map(0x300000, 0x303fff).ram().share(m_spriteram);
	map(0x400000, 0x400003).w(FUNC(groundfx_state::motor_control_w));  // gun vibration
	map(0x500000, 0x500007).rw("tc0510nio", FUNC(tc0510nio_device::read), FUNC(tc0510nio_device::write));
	map(0x600000, 0x600007).rw("adc", FUNC(adc0808_device::data_r), FUNC(adc0808_device::address_offset_start_w)).umask32(0xffffffff);
	map(0x700000, 0x7007ff).rw("taito_en:dpram", FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w));
	map(0x800000, 0x80ffff).rw(m_tc0480scp, FUNC(tc0480scp_device::ram_r), FUNC(tc0480scp_device::ram_w));      // tilemaps
	map(0x830000, 0x83002f).rw(m_tc0480scp, FUNC(tc0480scp_device::ctrl_r), FUNC(tc0480scp_device::ctrl_w));  // debugging
	map(0x900000, 0x90ffff).rw(m_tc0620scc, FUNC(tc0620scc_device::ram_r), FUNC(tc0620scc_device::ram_w));    // 6bpp tilemaps
	map(0x920000, 0x92000f).rw(m_tc0620scc, FUNC(tc0620scc_device::ctrl_r), FUNC(tc0620scc_device::ctrl_w));
	map(0xa00000, 0xa0ffff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0xb00000, 0xb003ff).ram();                     // ?? single bytes, blending ??
	map(0xc00000, 0xc00007).nopr(); // Network?
	map(0xd00000, 0xd00003).w(FUNC(groundfx_state::rotate_control_w)); // perhaps port based rotate control?
	// f00000 is seat control?
}

/***********************************************************
             INPUT PORTS (dips in EPROM)
***********************************************************/

static INPUT_PORTS_START( groundfx )
	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Shift Hi")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Brake")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Shift Low")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("WHEEL")
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("ACCEL")
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_PLAYER(1)
INPUT_PORTS_END

/**********************************************************
                GFX DECODING
**********************************************************/

static const gfx_layout tile16x16_layout =
{
	16,16,  // 16*16 sprites
	RGN_FRAC(1,5),
	5,  // 5 bits per pixel
	{ 0, RGN_FRAC(1,5), RGN_FRAC(2,5), RGN_FRAC(3,5), RGN_FRAC(4,5) },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16   // every sprite takes 128 consecutive bytes
};

static GFXDECODE_START( gfx_groundfx )
	GFXDECODE_ENTRY( "sprites", 0x0, tile16x16_layout, 4096, 512 )
GFXDECODE_END


/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

INTERRUPT_GEN_MEMBER(groundfx_state::interrupt)
{
	m_frame_counter ^= 1;
	device.execute().set_input_line(4, HOLD_LINE);
}

void groundfx_state::groundfx(machine_config &config)
{
	// basic machine hardware
	M68EC020(config, m_maincpu, XTAL(40'000'000) / 2); // 20MHz - verified
	m_maincpu->set_addrmap(AS_PROGRAM, &groundfx_state::prg_map);
	m_maincpu->set_vblank_int("screen", FUNC(groundfx_state::interrupt));

	EEPROM_93C46_16BIT(config, "eeprom");

	adc0809_device &adc(ADC0809(config, "adc", 500'000)); // unknown clock
	adc.eoc_ff_callback().set_inputline("maincpu", 5);
	adc.in_callback<0>().set_constant(0); // unknown
	adc.in_callback<1>().set_constant(0); // unknown (used to be labeled 'volume' - but doesn't seem to affect it)
	adc.in_callback<2>().set_ioport("WHEEL");
	adc.in_callback<3>().set_ioport("ACCEL");

	tc0510nio_device &tc0510nio(TC0510NIO(config, "tc0510nio", 0));
	tc0510nio.read_2_callback().set_ioport("BUTTONS");
	tc0510nio.read_3_callback().set("eeprom", FUNC(eeprom_serial_93cxx_device::do_read)).lshift(7);
	tc0510nio.read_3_callback().append(FUNC(groundfx_state::frame_counter_r)).lshift(0);
	tc0510nio.write_3_callback().set("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write)).bit(5);
	tc0510nio.write_3_callback().append("eeprom", FUNC(eeprom_serial_93cxx_device::di_write)).bit(6);
	tc0510nio.write_3_callback().append("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write)).bit(4);
	tc0510nio.write_4_callback().set(FUNC(groundfx_state::coin_word_w));
	tc0510nio.read_7_callback().set_ioport("SYSTEM");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0, 40*8-1, 3*8, 32*8-1);
	screen.set_screen_update(FUNC(groundfx_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_groundfx);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_888, 16384);

	TC0620SCC(config, m_tc0620scc, 0);
	m_tc0620scc->set_offsets(50, 8);
	m_tc0620scc->set_palette(m_palette);

	TC0480SCP(config, m_tc0480scp, 0);
	m_tc0480scp->set_palette(m_palette);
	m_tc0480scp->set_offsets(0x24, 0);
	m_tc0480scp->set_offsets_tx(-1, 0);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	taito_en_device &taito_en(TAITO_EN(config, "taito_en", 0));
	taito_en.add_route(0, "speaker", 1.0, 0);
	taito_en.add_route(1, "speaker", 1.0, 1);
}

/***************************************************************************
                    DRIVERS
***************************************************************************/

ROM_START( groundfx )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68020 code (CPU A)
	ROM_LOAD32_BYTE( "d51-24.79", 0x00000, 0x80000, CRC(5caaa031) SHA1(03e727e26df701e3f5e16c5f933d5b29a528945a) )
	ROM_LOAD32_BYTE( "d51-23.61", 0x00001, 0x80000, CRC(462e3c9b) SHA1(7f116ee755748497b911868a948d3e3b5134e475) )
	ROM_LOAD32_BYTE( "d51-22.77", 0x00002, 0x80000, CRC(b6b04d88) SHA1(58685ee8fd788dcbfe318f1e3c06d93e2128034c) )
	ROM_LOAD32_BYTE( "d51-21.59", 0x00003, 0x80000, CRC(21ecde2b) SHA1(c6d3738f34c8e24346e7784b14aeff300ae2d225) )

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )
	ROM_LOAD16_BYTE( "d51-29.54", 0x100000, 0x40000,  CRC(4b64f41d) SHA1(040427668d13f7320d23805098d6d0e1aa8d121e) )
	ROM_LOAD16_BYTE( "d51-30.56", 0x100001, 0x40000,  CRC(45f339fe) SHA1(cc7adfb2b86070f5bb426542e3b7ed2a50b3c39e) )

	ROM_REGION( 0x400000, "tc0480scp", 0 ) // SCR 16x16 tiles
	ROM_LOAD32_WORD( "d51-08.35", 0x000000, 0x200000, CRC(835b7a0f) SHA1(0131fceabd73b0045b5d4ae0bb2f03efdd407962) )
	ROM_LOAD32_WORD( "d51-09.34", 0x000002, 0x200000, CRC(6dabd83d) SHA1(3dbd7ea36b9900faa6420af1f1600efe295db74c) )

	ROM_REGION( 0xa00000, "sprites", 0 ) // OBJ 16x16 tiles
	ROM_LOAD16_WORD_SWAP( "d51-03.47", 0x000000, 0x200000, CRC(629a5c99) SHA1(cfc1c0b07ecefd6eddb83edcbcf710e8b8de19e4) )
	ROM_LOAD16_WORD_SWAP( "d51-04.48", 0x200000, 0x200000, CRC(f49b14b7) SHA1(31129771159c1295a074c8311344ece525302289) )
	ROM_LOAD16_WORD_SWAP( "d51-05.49", 0x400000, 0x200000, CRC(3a2e2cbf) SHA1(ed2c1ca9211b1d70b4767a54e08263a3e4867199) )
	ROM_LOAD16_WORD_SWAP( "d51-06.50", 0x600000, 0x200000, CRC(d33ce2a0) SHA1(92c4504344672ea798cd6dd34f4b46848bf9f82b) )
	ROM_LOAD16_WORD_SWAP( "d51-07.51", 0x800000, 0x200000, CRC(24b2f97d) SHA1(6980e67b435d189ce897c0301e0411763410ab47) )

	ROM_REGION( 0x200000, "tc0620scc", 0 ) // SCC 8x8 tiles, 4bpp
	ROM_LOAD16_BYTE( "d51-10.95", 0x000001, 0x100000, CRC(d5910604) SHA1(8efe13884cfdef208394ddfe19f43eb1b9f78ff3) )
	ROM_LOAD16_BYTE( "d51-11.96", 0x000000, 0x100000, CRC(fee5f5c6) SHA1(1be88747f9c71c348dd61a8f0040007df3a3e6a6) )

	ROM_REGION( 0x100000, "tc0620scc:hi_gfx", 0 ) // SCC 8x8 tiles, 2bpp
	ROM_LOAD       ( "d51-12.97", 0x000000, 0x100000, CRC(d630287b) SHA1(2fa09e1821b7280d193ca9a2a270759c3c3189d1) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 ) // STY
	ROM_LOAD16_WORD( "d51-13.7", 0x00000,  0x80000,  CRC(36921b8b) SHA1(2130120f78a3b984618a53054fc937cf727177b9) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "d51-01.73", 0x000000, 0x200000, CRC(92f09155) SHA1(8015e1997818bb480174394eb43840bf26679bcf) )
	ROM_LOAD16_BYTE( "d51-02.74", 0xc00000, 0x200000, CRC(20a9428f) SHA1(c9033d02a49c72f704808f5f899101617d5814e5) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.164", 0x0000, 0x0080, CRC(6f58851d) SHA1(33bd4478f097dca6b5d222adb89699c6d35ed009) )
ROM_END

/*The World version of Ground Effects is not dumped and has the following EPROM labels:
'D51 28' @ IC59
'D51 31' @ IC77
'D51 32' @ IC61
'D51 33' @ IC79*/

u32 groundfx_state::irq_speedup_r()
{
	int ptr;
	offs_t sp = m_maincpu->state_int(M68K_SP);
	if ((sp & 2) == 0) ptr = m_ram[(sp & 0x1ffff) / 4];
	else ptr = (((m_ram[(sp & 0x1ffff) / 4]) & 0x1ffff) << 16) |
	(m_ram[((sp & 0x1ffff) / 4) + 1] >> 16);

	if (m_maincpu->pc() == 0x1ece && ptr == 0x1b9a)
		m_maincpu->spin_until_interrupt();

	return m_ram[0xb574 / 4];
}


void groundfx_state::init_groundfx()
{
	// Speedup handlers
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x20b574, 0x20b577, read32smo_delegate(*this, FUNC(groundfx_state::irq_speedup_r)));
}

} // anonymous namespace


GAME( 1992, groundfx, 0, groundfx, groundfx, groundfx_state, init_groundfx, ROT0, "Taito Corporation", "Ground Effects / Super Ground Effects (Japan)", MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE )
