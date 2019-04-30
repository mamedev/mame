// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood, Mike Green
/*******************************************************************************************

Diamond Derby - G4001 board (c) 1986 Electrocoin

driver by David Haywood,Mike Green & Angelo Salese

Notes:
-Press Collect Button to "get the money";

TODO:
-Enters into Service Mode (?) if you let it go in attract mode after some time;
-Fix remaining graphic issues;
-Fix colors (check bar test on the first Service Mode menu);
============================================================================================

G4001
Diamond Derby - Electrocoin on an SNK board

SWP gambling game base on horse racing

SNK/Electrocoin 1986
Re-released in 1994 (see dderbya) for changed uk gaming rules.

--------------------------------------------
G4001UP01

SWA SWB   C1        DD1
                    DD2

                6116              DD4
                          Z80     DD5
                                  DD6
                          DD3     6116
             8910  Z80    6116    6116

---------------------------------------------
G4001UP02

DD7  DD11  DD15           K1
DD8  DD12  DD16                   DD19
DD9  DD13  DD17                   DD20
DD10 DD14  DD18     H5            DD21


                                  DD22
                                  DD23
 2114                  2148 2148
 2114              H10

*******************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/resnet.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class dmndrby_state : public driver_device
{
public:
	dmndrby_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_scroll_ram(*this, "scroll_ram"),
		m_sprite_ram(*this, "sprite_ram"),
		m_dderby_vidchars(*this, "vidchars"),
		m_dderby_vidattribs(*this, "vidattribs"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void dderby(machine_config &config);

protected:
	virtual void video_start() override;

private:
	required_shared_ptr<uint8_t> m_scroll_ram;
	required_shared_ptr<uint8_t> m_sprite_ram;
	required_shared_ptr<uint8_t> m_dderby_vidchars;
	required_shared_ptr<uint8_t> m_dderby_vidattribs;
	uint8_t *m_racetrack_tilemap_rom;
	tilemap_t *m_racetrack_tilemap;
	uint8_t m_io_port[8];
	int m_bg;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	DECLARE_WRITE8_MEMBER(dderby_sound_w);
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(output_w);
	TILE_GET_INFO_MEMBER(get_dmndrby_tile_info);
	void dmndrby_palette(palette_device &palette) const;
	uint32_t screen_update_dderby(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(dderby_irq);
	INTERRUPT_GEN_MEMBER(dderby_timer_irq);

	void dderby_sound_map(address_map &map);
	void memmap(address_map &map);
};


WRITE8_MEMBER(dmndrby_state::dderby_sound_w)
{
	m_soundlatch->write(data);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}


READ8_MEMBER(dmndrby_state::input_r)
{
	switch(offset & 7)
	{
		case 0: return ioport("IN0")->read();
		case 1: return ioport("IN1")->read();
		case 2: return ioport("IN2")->read();
		case 3: return ioport("IN3")->read();
		case 4: return ioport("IN4")->read();
		case 5: return ioport("IN5")->read();
		case 6: return ioport("IN6")->read();
		case 7: return ioport("IN7")->read();
	}

	return 0xff;
}

WRITE8_MEMBER(dmndrby_state::output_w)
{
	/*
	---- x--- refill meter [4]
	---- x--- token out meter [5]
	---- x--- token in meter [6]
	---- x--- cash out meter [7]
	---- -x-- coin out (meter) [0-3]
	---- -x-- coin lockout token [4]
	---- -x-- coin counter (meter) [5]
	---- --x- coin lockout [0-3]
	---- ---x lamp [0-6]
	*/
	m_io_port[offset] = data;
//  popmessage("%02x|%02x|%02x|%02x|%02x|%02x|%02x|%02x|",m_io_port[0],m_io_port[1],m_io_port[2],m_io_port[3],m_io_port[4],m_io_port[5],m_io_port[6],m_io_port[7]);
}

void dmndrby_state::memmap(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x8000, 0x8fff).ram().share("nvram");
	map(0xc000, 0xc007).r(FUNC(dmndrby_state::input_r));
	map(0xc000, 0xc007).w(FUNC(dmndrby_state::output_w));
	map(0xc802, 0xc802).portr("DSW1");
	map(0xc803, 0xc803).portr("DSW2");
	map(0xca00, 0xca00).nopw();//(vblank_irq_w) //???
	map(0xca01, 0xca01).nopw(); //watchdog
	map(0xca02, 0xca02).ram().w(FUNC(dmndrby_state::dderby_sound_w));
	map(0xca03, 0xca03).nopw();//(timer_irq_w) //???
	map(0xcc00, 0xcc05).ram().share("scroll_ram");
	map(0xce08, 0xce1f).ram().share("sprite_ram"); // horse sprites
	map(0xd000, 0xd3ff).ram().share("vidchars"); // char ram
	map(0xd400, 0xd7ff).ram().share("vidattribs"); // colours/ attrib ram
}

void dmndrby_state::dderby_sound_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x1000).ram(); //???
	map(0x4000, 0x4001).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x4000, 0x4000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x4001, 0x4001).r("ay1", FUNC(ay8910_device::data_r));
	map(0x6000, 0x67ff).ram();
}

static INPUT_PORTS_START( dderby )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_DIPNAME( 0x0002, 0x0002, "Out Coin 1" )//out coin 1
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("BET Horse 5")  PORT_CODE(KEYCODE_B)
	PORT_BIT( 0xf4, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_DIPNAME( 0x0002, 0x0002, "Out Coin 2" )//out coin 2
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("BET Horse 6")  PORT_CODE(KEYCODE_N)
	PORT_BIT( 0xf4, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x0002, 0x0002, "Out Coin 3" )//out coin 3
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Collect")  PORT_CODE(KEYCODE_2_PAD) //to get coins
	PORT_BIT( 0xf4, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Refill Key")  PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("BET Horse 4")  PORT_CODE(KEYCODE_V)
	PORT_BIT( 0xf5, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN5")
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("BET Horse 3")  PORT_CODE(KEYCODE_C)
	PORT_BIT( 0xf5, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN6")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Back Door")  PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("BET Horse 2")  PORT_CODE(KEYCODE_X)
	PORT_BIT( 0xf5, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN7")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Token Coin")  PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("BET Horse 1")  PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0xf5, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, " Unknown 1-1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, " Unknown 1-2" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x00, "Max Prize" )
	PORT_DIPSETTING(    0x06, "240p (cash)" )
	PORT_DIPSETTING(    0x02, "300p (cash)" )
	PORT_DIPSETTING(    0x04, "480p (cash + tokens)" )
	PORT_DIPSETTING(    0x00, "600p (cash + tokens)" )
	PORT_DIPNAME( 0x30, 0x00, "Percentage Payout" )
	PORT_DIPSETTING(    0x00, "76%" )   PORT_CONDITION("DSW1", 0xc0, LESSTHAN, 0x80)
	PORT_DIPSETTING(    0x10, "80%" )   PORT_CONDITION("DSW1", 0xc0, LESSTHAN, 0x80)
	PORT_DIPSETTING(    0x20, "86%" )   PORT_CONDITION("DSW1", 0xc0, LESSTHAN, 0x80)
	PORT_DIPSETTING(    0x30, "88%" )   PORT_CONDITION("DSW1", 0xc0, LESSTHAN, 0x80)
	PORT_DIPSETTING(    0x00, "78%" )   PORT_CONDITION("DSW1", 0xc0, NOTLESSTHAN, 0x80)
	PORT_DIPSETTING(    0x10, "82%" )   PORT_CONDITION("DSW1", 0xc0, NOTLESSTHAN, 0x80)
	PORT_DIPSETTING(    0x20, "86%" )   PORT_CONDITION("DSW1", 0xc0, NOTLESSTHAN, 0x80)
	PORT_DIPSETTING(    0x30, "90%" )   PORT_CONDITION("DSW1", 0xc0, NOTLESSTHAN, 0x80)
	PORT_DIPNAME( 0xc0, 0x80, "Price Per Game" )
	PORT_DIPSETTING(    0x00, "2p" )
	PORT_DIPSETTING(    0x40, "5p" )
	PORT_DIPSETTING(    0x80, "10p" )
	PORT_DIPSETTING(    0xc0, "20p" )

	PORT_START("DSW2")  /* 8bit */
	PORT_DIPNAME( 0x01, 0x01, "Show Results")
	PORT_DIPSETTING(    0x01, "Last Race" )
	PORT_DIPSETTING(    0x00, "Last 6 Races" )
	PORT_DIPNAME( 0x02, 0x02, " Unknown 2-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, " Unknown 2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, " Unknown 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, " Unknown 2-5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, " Unknown 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, " Unknown 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, " Unknown 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dderbya )
	PORT_INCLUDE( dderby )
	PORT_MODIFY("DSW1") /* 8bit */
	PORT_DIPNAME( 0x01, 0x01, "Price Per Play")
	PORT_DIPSETTING(    0x01, "5p" )
	PORT_DIPSETTING(    0x00, "10p" )
	PORT_DIPNAME( 0x06, 0x02, "Max Prize" )
	PORT_DIPSETTING(    0x00, "400p (cash + tokens)" )
	PORT_DIPSETTING(    0x04, "300p (cash + tokens)" )
	PORT_DIPSETTING(    0x02, "200p (cash)" )
	PORT_DIPSETTING(    0x06, "150p (cash)" )
	PORT_DIPNAME( 0x18, 0x08, "Percentage" )
	PORT_DIPSETTING(    0x00, "76%" )
	PORT_DIPSETTING(    0x08, "80%" )
	PORT_DIPSETTING(    0x10, "84%" )
	PORT_DIPSETTING(    0x18, "88%" )
	PORT_DIPNAME( 0x20, 0x20, " Unknown 1-2" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, " Unknown 1-3" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, " Unknown 1-4" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	3,
	{ RGN_FRAC(1,2),4,0 },
	{ 0, 1, 2, 3,8,9,10,11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tiles16x16_layout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,3),  /* 256 sprites */
	3,      /* 3 bits per pixel */
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },    // the three bitplanes are separated
	{ 0, 1, 2, 3, 4, 5, 6, 7,  16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    // every sprite takes 32 consecutive bytes

};

static const gfx_layout tiles8x8_layout2 =
{
	8,8, // 8x8 chars
	RGN_FRAC(1,3),
	3,
	{ 0,RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8},
	8*8,
};

static GFXDECODE_START( gfx_dmndrby )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 32*16, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout2, 0, 8)
	GFXDECODE_ENTRY( "gfx3", 0, tiles16x16_layout, 16*16, 32 )
GFXDECODE_END

TILE_GET_INFO_MEMBER(dmndrby_state::get_dmndrby_tile_info)
{
	int code = m_racetrack_tilemap_rom[tile_index];
	int attr = m_racetrack_tilemap_rom[tile_index+0x2000];

	int col = attr&0x1f;
	int flipx = (attr&0x40)>>6;


	SET_TILE_INFO_MEMBER(2,
			code,
			col,
			TILE_FLIPYX(flipx) );
}


void dmndrby_state::video_start()
{
	m_bg = 0;

	m_racetrack_tilemap_rom = memregion("user1")->base();
	m_racetrack_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(dmndrby_state::get_dmndrby_tile_info),this),TILEMAP_SCAN_ROWS,16,16, 16, 512);
	m_racetrack_tilemap->mark_all_dirty();

}

uint32_t dmndrby_state::screen_update_dderby(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y,count;
	int off,scrolly;
	gfx_element *gfx = m_gfxdecode->gfx(0);
	gfx_element *sprites = m_gfxdecode->gfx(1);
	gfx_element *track = m_gfxdecode->gfx(2);

	bitmap.fill(m_palette->black_pen(), cliprect);


/* Draw racetrack

racetrack seems to be stored in 4th and 5th prom.
can we draw it with the tilemap? maybe not, the layout is a litle strange

*/
//  base = m_scroll_ram[0];

	off=0x1900-(m_bg*0x100)+(m_scroll_ram[1])*0x100;
	scrolly = 0xff-(m_scroll_ram[0]);
	if(m_scroll_ram[1]==0xff) off=0x1800;
	for(x=0;x<16;x++) {
		for(y=0;y<16;y++) {
			int chr = m_racetrack_tilemap_rom[off];
			int col = m_racetrack_tilemap_rom[off+0x2000]&0x1f;
			int flipx = m_racetrack_tilemap_rom[off+0x2000]&0x40;
			track->opaque(bitmap,cliprect,chr,col,flipx,0,y*16+scrolly,x*16);
			// draw another bit of track
			// a rubbish way of doing it
			chr = m_racetrack_tilemap_rom[off-0x100];
			col = m_racetrack_tilemap_rom[off+0x1f00]&0x1f;
			flipx = m_racetrack_tilemap_rom[off+0x1f00]&0x40;
			track->opaque(bitmap,cliprect,chr,col,flipx,0,y*16-256+scrolly,x*16);
			off++;
		}
	}


//return 0;

/* draw sprites

 guess work  again! seems to work fine and horse labels match up
wouldnt like to say its the most effective way though...
 -- maybe they should be decoded as 'big sprites' instead?

*/
	for (count=5;count>=0;count-- )
	{
		int a=0;
		int b=0;
		int base = count*4;
		int sprx=m_sprite_ram[base+3];
		int spry=m_sprite_ram[base+2];
		//m_sprite_ram[base+1];
		int col = (m_sprite_ram[base+1]&0x1f);
		int anim = (m_sprite_ram[base]&0x3)*0x40; // animation frame - probably wrong but seems right
		int horse = (m_sprite_ram[base+1]&0x7)*8+7;  // horse label from 1 - 6

		for (a=0;a<8 ;a++)
		{
			for(b=0;b<7;b++) {
				sprites->transpen(bitmap,cliprect,anim+a*8+b,col,0,0,sprx+a*8,spry+b*8,0);
			}
		}
		// draw the horse number
		a=3;
		b=3;
		sprites->transpen(bitmap,cliprect,anim+horse,col,0,0,sprx+a*8,spry+b*8,0);


	}

	/*TODO: Fix / understand how the transparency works properly. */
	count=0;
	for (y=0;y<32;y++)
	{
		for(x=0;x<32;x++)
		{
			int tileno,bank,color;
			tileno=m_dderby_vidchars[count];
			bank=(m_dderby_vidattribs[count]&0x20)>>5;
			tileno|=(bank<<8);
			color=((m_dderby_vidattribs[count])&0x1f);

			gfx->transpen(bitmap,cliprect,tileno,color,0,0,x*8,y*8,(tileno == 0x38) ? 0 : -1);

			count++;
		}
	}


	return 0;
}

// copied from elsewhere. surely incorrect
void dmndrby_state::dmndrby_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	// compute the color output resistor weights
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights, 470, 0,
			3, &resistances_rg[0], gweights, 470, 0,
			2, &resistances_b[0],  bweights, 470, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;

		// red component */
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(rweights, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(gweights, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(bweights, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom = memregion("proms2")->base();

	// normal tiles use colors 0-15
	for (int i = 0x000; i < 0x300; i++)
	{
		uint8_t ctabentry = color_prom[i];
		palette.set_pen_indirect(i, ctabentry);
	}
}

/*Main Z80 is IM 0,HW-latched irqs. */
INTERRUPT_GEN_MEMBER(dmndrby_state::dderby_irq)
{
	m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7); /* Z80 - RST 10h */
}

INTERRUPT_GEN_MEMBER(dmndrby_state::dderby_timer_irq)
{
	m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xcf); /* Z80 - RST 08h */
}

void dmndrby_state::dderby(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4000000);         /* ? MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &dmndrby_state::memmap);
	m_maincpu->set_vblank_int("screen", FUNC(dmndrby_state::dderby_irq));
	m_maincpu->set_periodic_int(FUNC(dmndrby_state::dderby_timer_irq), attotime::from_hz(244/2));

	Z80(config, m_audiocpu, 4000000);  /* verified on schematics */
	m_audiocpu->set_addrmap(AS_PROGRAM, &dmndrby_state::dderby_sound_map);

	config.m_minimum_quantum = attotime::from_hz(6000);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 256-16-1);
	screen.set_screen_update(FUNC(dmndrby_state::screen_update_dderby));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_dmndrby);
	PALETTE(config, m_palette, FUNC(dmndrby_state::dmndrby_palette), 0x300, 0x20);

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8910(config, "ay1", 1789750).add_route(ALL_OUTPUTS, "mono", 0.35); // frequency guessed
}


ROM_START( dmndrby )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main cpu */
	ROM_LOAD( "dd04.m6", 0x00000, 0x02000, CRC(a3cfd28e) SHA1(7ba14876fa4409634a699e049bce3bc457522932) )
	ROM_LOAD( "dd05.m7", 0x02000, 0x02000, CRC(16f7ac0b) SHA1(030b8c2b294a0287f3aaf72424304fc191315888) )
	ROM_LOAD( "dd06.m8", 0x04000, 0x02000, CRC(914ba8f5) SHA1(d1b3f3d5d2625e42ea6cb5c777942cec7faea58e) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* sound cpu */
	ROM_LOAD( "dd03.j9", 0x00000, 0x01000, CRC(660f0cae) SHA1(b3b414e52342de877a5c20886a87002a63310a94) )

	ROM_REGION( 0x04000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "dd01.e1", 0x00000, 0x02000, CRC(2e120288) SHA1(0ea29aff07e956b19080f05bd18b427195694ce8) )
	ROM_LOAD( "dd02.e2", 0x02000, 0x02000, CRC(ca028c8c) SHA1(f882eea2191cf1f3ea57d49fd6862f98401555be) )

	ROM_REGION( 0x4000, "user1", 0 ) // tracerack data
	ROM_LOAD( "dd22.n6", 0x00000, 0x02000, CRC(db6b13fc) SHA1(3415deb2ffa86679e4f8abb644b75963e5368ba0) )
	ROM_LOAD( "dd23.n7", 0x02000, 0x02000, CRC(595fdb9b) SHA1(133d227bb156be52337da974e37973b049722e49) )

	ROM_REGION( 0x18000, "gfx2", 0 ) /* horse sprites (kinda) */
	ROM_LOAD( "dd07.b1", 0x00000, 0x02000, CRC(207a534a) SHA1(ddbd292f79cc9fb7bd9f0ee9874da87909147789) )
	ROM_LOAD( "dd08.b2", 0x02000, 0x02000, CRC(f380e2c4) SHA1(860a6557ae8b81d310c353f88f9194e1ffd551ec) )
	ROM_LOAD( "dd09.b3", 0x04000, 0x02000, CRC(68ebf74c) SHA1(959ee6c4ce700cff86af39442063dc79b8f8913e) )
	ROM_LOAD( "dd10.b5", 0x06000, 0x01000, CRC(38b1568a) SHA1(f7e04db49708dfc8c8512026d3460af0f3fb6780) )
	ROM_LOAD( "dd11.d1", 0x08000, 0x02000, CRC(fe615561) SHA1(808f703d0ca1576feb78f21c380e4006dd634a9c) )
	ROM_LOAD( "dd12.d2", 0x0a000, 0x02000, CRC(4df63aae) SHA1(a0b224fb1157fc25c21f9f0664bb8385e94e5c77) )
	ROM_LOAD( "dd13.d4", 0x0c000, 0x02000, CRC(cace0dfc) SHA1(41902f3ee2fa18798e3b441ee18f7b953d977b93) )
	ROM_LOAD( "dd14.d5", 0x0e000, 0x01000, CRC(2c602cbe) SHA1(78ffe79e3f2c4a3e9c6adc8f4635ed1a93528dc8) )
	ROM_LOAD( "dd15.e1", 0x10000, 0x02000, CRC(2ce23b64) SHA1(5cbeabc015cb167c7fd485ab4d9f1329bc2e94b3) )
	ROM_LOAD( "dd16.e2", 0x12000, 0x02000, CRC(6af9796c) SHA1(4cd818d488ac85fd6f8732fdca80cc29db86d3f4) )
	ROM_LOAD( "dd17.e4", 0x14000, 0x02000, CRC(b451cde2) SHA1(1c7340cc39d9beca1640c88000112c898d3de941) )
	ROM_LOAD( "dd18.e5", 0x16000, 0x01000, CRC(56228aaf) SHA1(74e96ebefc1b69310b23e47a35affbb7cd7d9acc) )

	ROM_REGION( 0x6000, "gfx3", 0 )
	ROM_LOAD( "dd19.n2", 0x00000, 0x02000, CRC(fd536051) SHA1(556dfe064eeb9be1db751410ec128385c463e706) )
	ROM_LOAD( "dd20.n3", 0x02000, 0x02000, CRC(1497e52f) SHA1(f08c20c97c8d2148fcc705297cf1129bc65c9b83) )
	ROM_LOAD( "dd21.n4", 0x04000, 0x02000, CRC(87605d44) SHA1(c985fb15eac7bcc89e92909cf588b5982d0cabd0) )

	ROM_REGION( 0x020, "proms", 0 ) // palette
	ROM_LOAD( "ddprom3.h5", 0x0000, 0x0020, CRC(aea3cff6) SHA1(1139dd52c127436873a674be8a14527190091a82) )

	ROM_REGION( 0x300, "proms2", 0 ) // lookup
	ROM_LOAD( "ddprom4.h10",0x0000, 0x0100, CRC(7280f000) SHA1(bfad2b547b8abe6c67928bec08e7d00431c539d5) ) // HORSES
	ROM_LOAD( "ddprom5.k1", 0x0100, 0x0100, CRC(15edbdac) SHA1(298640afb24830d32f54c0c7c5960d777f51f2bd) ) // TILES?
	ROM_LOAD( "ddprom1.c1", 0x0200, 0x0100, CRC(e1c2fa1b) SHA1(6b8b0b2c1ac4b2796070452c923ba96dd8b29048) ) // ???

	ROM_REGION( 0x200, "proms3", 0 ) // other
	ROM_LOAD( "ddprom2.j5", 0x0000, 0x0100, CRC(3e5402dc) SHA1(2f497333f49064c54995cec8919f3aebdc17e977) )
	ROM_LOAD( "ddprom6.m12",0x0100, 0x0100, CRC(7f677b7d) SHA1(946014cb01f5954a3cb196796741ee174a0de641) )
ROM_END

ROM_START( dmndrbya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dd4", 0x00000, 0x02000, CRC(29b06e0f) SHA1(301fc2fe25ce47c2ad5112f0b795cd6bae605071) )
	ROM_LOAD( "dd5", 0x02000, 0x02000, CRC(5299d020) SHA1(678d338d2cee5250154454be97456d5f80bb4759) )
	ROM_LOAD( "dd6", 0x04000, 0x02000, CRC(f7e30ec0) SHA1(bf898987366ee9def190e3575108395b0aaef2d6) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* sound cpu */
	ROM_LOAD( "dd03.j9", 0x00000, 0x01000, CRC(660f0cae) SHA1(b3b414e52342de877a5c20886a87002a63310a94) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "dd1", 0x00000, 0x02000, CRC(7fe475a6) SHA1(008bbaff87baad7f4c2497e40bf5e3fc95f993b4) )
	ROM_LOAD( "dd2", 0x02000, 0x02000, CRC(54def3ee) SHA1(fb88852ada2b5b555c0e8c0a082ed9978ff27434) )

	ROM_REGION( 0x4000, "user1", 0 ) // tracerack data
	ROM_LOAD( "dd22.n6", 0x00000, 0x02000, CRC(db6b13fc) SHA1(3415deb2ffa86679e4f8abb644b75963e5368ba0) )
	ROM_LOAD( "dd23.n7", 0x02000, 0x02000, CRC(595fdb9b) SHA1(133d227bb156be52337da974e37973b049722e49) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "dd07.b1", 0x00000, 0x02000, CRC(207a534a) SHA1(ddbd292f79cc9fb7bd9f0ee9874da87909147789) )
	ROM_LOAD( "dd08.b2", 0x02000, 0x02000, CRC(f380e2c4) SHA1(860a6557ae8b81d310c353f88f9194e1ffd551ec) )
	ROM_LOAD( "dd09.b3", 0x04000, 0x02000, CRC(68ebf74c) SHA1(959ee6c4ce700cff86af39442063dc79b8f8913e) )
	ROM_LOAD( "dd10.b5", 0x06000, 0x01000, CRC(38b1568a) SHA1(f7e04db49708dfc8c8512026d3460af0f3fb6780) )
	ROM_LOAD( "dd11.d1", 0x08000, 0x02000, CRC(fe615561) SHA1(808f703d0ca1576feb78f21c380e4006dd634a9c) )
	ROM_LOAD( "dd12.d2", 0x0a000, 0x02000, CRC(4df63aae) SHA1(a0b224fb1157fc25c21f9f0664bb8385e94e5c77) )
	ROM_LOAD( "dd13.d4", 0x0c000, 0x02000, CRC(cace0dfc) SHA1(41902f3ee2fa18798e3b441ee18f7b953d977b93) )
	ROM_LOAD( "dd14.d5", 0x0e000, 0x01000, CRC(2c602cbe) SHA1(78ffe79e3f2c4a3e9c6adc8f4635ed1a93528dc8) )
	ROM_LOAD( "dd15.e1", 0x10000, 0x02000, CRC(2ce23b64) SHA1(5cbeabc015cb167c7fd485ab4d9f1329bc2e94b3) )
	ROM_LOAD( "dd16.e2", 0x12000, 0x02000, CRC(6af9796c) SHA1(4cd818d488ac85fd6f8732fdca80cc29db86d3f4) )
	ROM_LOAD( "dd17.e4", 0x14000, 0x02000, CRC(b451cde2) SHA1(1c7340cc39d9beca1640c88000112c898d3de941) )
	ROM_LOAD( "dd18.e5", 0x16000, 0x01000, CRC(56228aaf) SHA1(74e96ebefc1b69310b23e47a35affbb7cd7d9acc) )

	ROM_REGION( 0x6000, "gfx3", 0 )
	ROM_LOAD( "dd19.n2", 0x00000, 0x02000, CRC(fd536051) SHA1(556dfe064eeb9be1db751410ec128385c463e706) )
	ROM_LOAD( "dd20.n3", 0x02000, 0x02000, CRC(1497e52f) SHA1(f08c20c97c8d2148fcc705297cf1129bc65c9b83) )
	ROM_LOAD( "dd21.n4", 0x04000, 0x02000, CRC(87605d44) SHA1(c985fb15eac7bcc89e92909cf588b5982d0cabd0) )

	ROM_REGION( 0x020, "proms", 0 ) // palette
	ROM_LOAD( "ddprom3.h5", 0x0000, 0x0020, CRC(aea3cff6) SHA1(1139dd52c127436873a674be8a14527190091a82) )

	ROM_REGION( 0x300, "proms2", 0 ) // lookup
	ROM_LOAD( "ddprom4.h10",0x0000, 0x0100, CRC(7280f000) SHA1(bfad2b547b8abe6c67928bec08e7d00431c539d5) ) // HORSES
	ROM_LOAD( "ddprom5.k1", 0x0100, 0x0100, CRC(15edbdac) SHA1(298640afb24830d32f54c0c7c5960d777f51f2bd) ) // TILES?
	ROM_LOAD( "ddprom1.c1", 0x0200, 0x0100, CRC(e1c2fa1b) SHA1(6b8b0b2c1ac4b2796070452c923ba96dd8b29048) ) // ???

	ROM_REGION( 0x200, "proms3", 0 ) // other
	ROM_LOAD( "ddprom2.j5", 0x0000, 0x0100, CRC(3e5402dc) SHA1(2f497333f49064c54995cec8919f3aebdc17e977) )
	ROM_LOAD( "ddprom6.m12",0x0100, 0x0100, CRC(7f677b7d) SHA1(946014cb01f5954a3cb196796741ee174a0de641) )
ROM_END


//    YEAR, NAME,     PARENT,  MACHINE, INPUT,   STATE,         INIT,       MONITOR, COMPANY,       FULLNAME                    FLAGS
GAME( 1994, dmndrby,  0,       dderby,  dderby,  dmndrby_state, empty_init, ROT0,    "Electrocoin", "Diamond Derby (Newer)",    MACHINE_IMPERFECT_GRAPHICS|MACHINE_IMPERFECT_COLORS|MACHINE_NOT_WORKING ) // hack?
GAME( 1986, dmndrbya, dmndrby, dderby,  dderbya, dmndrby_state, empty_init, ROT0,    "Electrocoin", "Diamond Derby (Original)", MACHINE_IMPERFECT_GRAPHICS|MACHINE_IMPERFECT_COLORS|MACHINE_NOT_WORKING )
