// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/*

Witch / Pinball Champ '95


            Witch: press F1 to initialize NVRAM

Pinball Champ '95: Seems to be a simple mod with the following differences:
                   -The title screen is changed
                   -The sample saying "witch" is not played (obviously)
                   -Different configuration values (time limit, etc)
                   -Auto-initialization on NVRAM error(?)
                   -Stars keep falling at the title screen


ES-9104 PCB:
+-------------------------------------+
|        12.00MHz                5.1A |
|                                     |
|   6116                         6116 |
|   6116                         6116 |
|          24S10N                     |
|                                     |
|       SW2         Z80A     6116     |
|SW1 8255     SW5   Z80A              |
|                                     |
|      SW3                       6116 |
|      SW4 YM2203                     |
|          YM2203    U_5B.5U   3.3U   |
|X2 M5202  1.10V     HM6264           |
|  VR1     ES8712    BAT1             |
+-------------------------------------+

       Z80A: Two Z80A CPUs frequency unknown (4MHz? 12MHz/3) (CPU2 used mainly for sound effects)
     YM2203: Two Yamaha YM2203+YM3014B sound chip combos. Frequency unknown (music + sound effects + video scrolling access)
      M5202: OKI M5202 ADPCM Speech Synthesis IC @ 384kHz
     ES8712: Excellent System ES-8712 Streaming single channel ADPCM, frequency unknown (samples)
       8255: M5M82C255ASP Programmable Peripheral Interface
        OSC: 12.000MHz
         X2: 384kHz Resonator to drive M5202
        VR1: Volume pot
       BAT1: 3.6V battery
        SW1: Service push button
      other: 8-position dipswitch x 4 (labeled SW2 through SW5)
             Standard 8-liner harness connectors (36x2 edge connector + 10x2 edge connector).


This is so far what could be reverse-engineered from the code.
BEWARE : these are only suppositions, not facts.


GFX

    2 gfx layers accessed by cpu1 (& cpu2 for scrolling) + 1 sprite layer

    In (assumed) order of priority :
        - Top layer @0xc000-0xc3ff(vram) + 0xc400-0xc7ff(cram) apparently not scrollable (gfx0)
            Uses tiles from "gfx2"

            tileno =    vram | ((cram & 0xe0) << 3)
            color  =    cram & 0x0f
            priority =  cram & 0x10 (0x10 = under sprites, 0x00 = over sprites)

        - Sprites @0xd000-0xd7ff + 0xd800-0xdfff
                One sprite every 0x20 bytes
                0x40 sprites
                Tiles are from "gfx2"
                Seems to be only 16x16 sprites (2x2 tiles)
                xflip and yflip available

                tileno                = sprite_ram[i*0x20] << 2 | (( sprite_ram[i*0x20+0x800] & 0x07 ) << 10 );
                sx                    = sprite_ram[i*0x20+1];
                sy              = sprite_ram[i*0x20+2];
                flags+colors    = sprite_ram[i*0x20+3];

        - Background layer @0xc800-0xcbff(vram) + 0xcc00-0xcfff(cram) (gfx1)
                Uses tiles from "gfx1"
                    tileno = vram | ((cram & 0xf0) << 4),
                    color  = cram & 0x0f

                The background is scrolled via 2 registers accessed through one of the ym2203, port A&B
                The scrolling is set by CPU2 in its interrupt handler.
                CPU1 doesn't seem to offset vram accesses for the scrolling, so it's assumed to be done
                in hardware.
                This layer looks misaligned with the sprites, but the top layer is not. This is perhaps
                due to the weird handling of the scrolling. For now we just offset it by 7 pixels.


Palette

    3*0x100 palette banks @ 0xe000-0xe300 & 0xe800-0xe8ff (xBBBBBGGGGGRRRRR_split format?)
    Bank 1 is used for gfx0 (top layer) and sprites
    Bank 2 is for gfx1 (background layer)

    Could not find any use of bank 0 ; I'm probably missing a flag somewhere.


Sound

    Mainly handled by CPU2

    2xYM2203

    0x8000-0x8001 : Mainly used for sound effects & to read dipswitches
    0x8008-0x8009 : Music & scrolling

    1xES8712

    Mapped @0x8010-0x8016
    Had to patch es8712.c to start playing on 0x8016 write and to prevent continuous looping.
    There's a test on bit1 at offset 0 (0x8010), so this may be a "read status" kind of port.
    For now reading at 8010 always reports ready.


Ports

    0xA000-0xA00f : Various ports yet to figure out...

      - 0xA000 : unknown ; seems muxed with a002
      - 0xA002 : banking?
                         bank number = bits 7&6 (swapped?)
                 mapped 0x0800-0x7fff?
                 0x0000-0x07ff ignored?
                 see code @ 61d
                 lower bits seems to mux port A000 reads
      - 0xA003 : ?
      - 0xA004 : dipswitches
      - 0xA005 : dipswitches
      - 0xA006 : bit1(out) = release coin?
      - 0xA007 : ?
      - 0xA008 : cpu1 sets it to 0x80 on reset ; cleared in interrupt handler
                             cpu2 sets it to 0x40 on reset ; cleared in interrupt handler
      - 0xA00C : bit0 = payout related?
                         bit3 = reset? (see cpu2 code @14C)
      - 0xA00E : ?


Memory

    RAM:
        Considering that
            -CPU1 busy loops on fd00 and that CPU2 modifies fd00 once it is initialized
            -CPU1 writes to fd01-fd05 and CPU2 reads there and plays sounds accordingly
            -CPU1 writes to f208-f209 and CPU2 forwards this to the scrolling registers
        we can assume that the 0xf2xx and 0fdxx segments are shared.

        From the fact that
            -CPU1's SP is set to 0xf100 on reset
            -CPU2's SP is set to 0xf080 on reset
        we may suppose that this memory range (0xf000-0xf0ff) is shared too.

        Moreover, range 0xf100-0xf17f is checked after reset without prior initialization and
        is being reset ONLY by changing a particular port bit whose modification ends up with
        a soft reboot. This looks like a good candidate for an NVRAM segment.
        Whether CPU2 can access the NVRAM or not is still a mystery considering that it never
        attempts to do so.

        From these we consider that the 0xfxxx segment, except for the NVRAM range, is shared
        between the two CPUs.

  CPU1:
      The ROM segment (0x0000-0x7fff) is banked, but the 0x0000-0x07ff region does not look
      like being affected (the SEGA Master System did something similar IIRC). A particular
      bank is selected by changing the two most significant bits of port 0xa002 (swapped?).

  CPU2:
            No banking
        Doesn't seem to be banking going on. However there's a strange piece of code @0x021a:
        Protection(?) check @ $21a


Interesting memory locations

        +f180-f183 : dipswitches stored here (see code@2746). Beware, all values are "CPL"ed!
            *f180   : kkkbbppp / A005
                             ppp  = PAY OUT | 60 ; 65 ; 70 ; 75 ; 80 ; 85 ; 90 ; 95
                             bb   = MAX BET | 20 ; 30 ; 40 ; 60
                             kkk  = KEY IN  | 1-10 ; 1-20 ; 1-40 ; 1-50 ; 1-100 ; 1-200 ; 1-250 ; 1-500

            *f181   : ccccxxxd / A004
                             d    = DOUBLE UP | ON ; OFF
                             cccc = COIN IN1 | 1-1 ; 1-2 ; 1-3 ; 1-4 ; 1-5 ; 1-6 ; 1-7 ; 1-8 ; 1-9 ; 1-10 ; 1-15 ; 1-20 ; 1-25 ; 1-30 ; 1-40 ; 1-50

            *f182   : sttpcccc / portA
                             cccc = COIN IN2 | 1-1 ; 1-2 ; 1-3 ; 1-4 ; 1-5 ; 1-6 ; 1-7 ; 1-8 ; 1-9 ; 1-10 ; 2-1 ; 3-1 ; 4-1 ; 5-1 ; 6-1 ; 10-1
                             p    = PAYOUT SWITCH | ON ; OFF
                             tt   = TIME | 40 ; 45 ; 50 ; 55
                             s    = DEMO SOUND | ON ; OFF
            *f183 : xxxxhllb / portB
                             b    = AUTO BET | ON ; OFF
                             ll   = GAME LIMIT | 500 ; 1000 ; 5000 ; 990000
                             h    = HOPPER ACTIVE | LOW ; HIGH


        +f15c-f15e : MAX WIN
        +f161      : JACK POT
        +f166-f168 : DOUBLE UP
        +f16b-f16d : MAX D-UP WIN

        +f107-f109 : TOTAL IN
        +f10c-f10e : TOTAL OUT

        +f192-f194 : credits (bcd)

        +fd00 = cpu2 ready
        +f211 = input port cache?

    CPU2 Commands :
        -0xfd01 start music
        -0xfd02 play sound effect
        -0xfd03 play sample on the ES8712
        -0xfd04 ?
        -0xfd05 ?


TODO :
    - Figure out the ports for the "PayOut" stuff (a006/a00c?)
    - Hook up the OKI M5202
*/

#define MAIN_CLOCK        XTAL_12MHz
#define CPU_CLOCK         MAIN_CLOCK / 4
#define YM2203_CLOCK      MAIN_CLOCK / 4
#define ES8712_CLOCK      8000              // 8Khz, it's the only clock for sure (pin13) it come from pin14 of M5205.


#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/es8712.h"
#include "sound/2203intf.h"
#include "machine/nvram.h"


class witch_state : public driver_device
{
public:
	witch_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_gfx0_vram(*this, "gfx0_vram"),
		m_gfx0_cram(*this, "gfx0_cram"),
		m_gfx1_vram(*this, "gfx1_vram"),
		m_gfx1_cram(*this, "gfx1_cram"),
		m_sprite_ram(*this, "sprite_ram"),
		m_palette(*this, "palette")  { }

	tilemap_t *m_gfx0a_tilemap;
	tilemap_t *m_gfx0b_tilemap;
	tilemap_t *m_gfx1_tilemap;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<UINT8> m_gfx0_vram;
	required_shared_ptr<UINT8> m_gfx0_cram;
	required_shared_ptr<UINT8> m_gfx1_vram;
	required_shared_ptr<UINT8> m_gfx1_cram;
	required_shared_ptr<UINT8> m_sprite_ram;
	required_device<palette_device> m_palette;

	int m_scrollx;
	int m_scrolly;
	UINT8 m_reg_a002;
	DECLARE_WRITE8_MEMBER(gfx0_vram_w);
	DECLARE_WRITE8_MEMBER(gfx0_cram_w);
	DECLARE_WRITE8_MEMBER(gfx1_vram_w);
	DECLARE_WRITE8_MEMBER(gfx1_cram_w);
	DECLARE_READ8_MEMBER(gfx1_vram_r);
	DECLARE_READ8_MEMBER(gfx1_cram_r);
	DECLARE_READ8_MEMBER(read_a00x);
	DECLARE_WRITE8_MEMBER(write_a00x);
	DECLARE_READ8_MEMBER(prot_read_700x);
	DECLARE_READ8_MEMBER(read_8010);
	DECLARE_WRITE8_MEMBER(xscroll_w);
	DECLARE_WRITE8_MEMBER(yscroll_w);
	DECLARE_DRIVER_INIT(witch);
	TILE_GET_INFO_MEMBER(get_gfx0b_tile_info);
	TILE_GET_INFO_MEMBER(get_gfx0a_tile_info);
	TILE_GET_INFO_MEMBER(get_gfx1_tile_info);
	virtual void video_start() override;
	UINT32 screen_update_witch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};


#define UNBANKED_SIZE 0x800


TILE_GET_INFO_MEMBER(witch_state::get_gfx0b_tile_info)
{
	int code  = m_gfx0_vram[tile_index];
	int color = m_gfx0_cram[tile_index];

	code=code | ((color & 0xe0) << 3);

	if(color&0x10)
	{
		code=0;
	}

	SET_TILE_INFO_MEMBER(1,
			code,   //tiles beyond 0x7ff only for sprites?
			color & 0x0f,
			0);
}

TILE_GET_INFO_MEMBER(witch_state::get_gfx0a_tile_info)
{
	int code  = m_gfx0_vram[tile_index];
	int color = m_gfx0_cram[tile_index];

	code=code | ((color & 0xe0) << 3);

	if((color&0x10)==0)
	{
		code=0;
	}

	SET_TILE_INFO_MEMBER(1,
			code,//tiles beyond 0x7ff only for sprites?
			color & 0x0f,
			0);
}

TILE_GET_INFO_MEMBER(witch_state::get_gfx1_tile_info)
{
	int code  = m_gfx1_vram[tile_index];
	int color = m_gfx1_cram[tile_index];

	SET_TILE_INFO_MEMBER(0,
			code | ((color & 0xf0) << 4),
			(color>>0) & 0x0f,
			0);
}

WRITE8_MEMBER(witch_state::gfx0_vram_w)
{
	m_gfx0_vram[offset] = data;
	m_gfx0a_tilemap->mark_tile_dirty(offset);
	m_gfx0b_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(witch_state::gfx0_cram_w)
{
	m_gfx0_cram[offset] = data;
	m_gfx0a_tilemap->mark_tile_dirty(offset);
	m_gfx0b_tilemap->mark_tile_dirty(offset);
}

#define FIX_OFFSET() do { \
	offset=(((offset + ((m_scrolly & 0xf8) << 2) ) & 0x3e0)+((offset + (m_scrollx >> 3) ) & 0x1f)+32)&0x3ff; } while(0)

WRITE8_MEMBER(witch_state::gfx1_vram_w)
{
	FIX_OFFSET();
	m_gfx1_vram[offset] = data;
	m_gfx1_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(witch_state::gfx1_cram_w)
{
	FIX_OFFSET();
	m_gfx1_cram[offset] = data;
	m_gfx1_tilemap->mark_tile_dirty(offset);
}
READ8_MEMBER(witch_state::gfx1_vram_r)
{
	FIX_OFFSET();
	return m_gfx1_vram[offset];
}

READ8_MEMBER(witch_state::gfx1_cram_r)
{
	FIX_OFFSET();
	return m_gfx1_cram[offset];
}

READ8_MEMBER(witch_state::read_a00x)
{
	switch(offset)
	{
		case 0x02: return m_reg_a002;
		case 0x04: return ioport("A004")->read();
		case 0x05: return ioport("A005")->read();
		case 0x0c: return ioport("SERVICE")->read();    // stats / reset
		case 0x0e: return ioport("A00E")->read();       // coin/reset
	}

	if(offset == 0x00) //muxed with A002?
	{
		switch(m_reg_a002 & 0x3f)
		{
		case 0x3b:
			return ioport("UNK")->read();   //bet10 / pay out
		case 0x3e:
			return ioport("INPUTS")->read();    //TODO : trace f564
		case 0x3d:
			return ioport("A005")->read();
		default:
			logerror("A000 read with mux=0x%02x\n", m_reg_a002 & 0x3f);
		}
	}
	return 0xff;
}

WRITE8_MEMBER(witch_state::write_a00x)
{
	switch(offset)
	{
		case 0x02: //A002 bit 7&6 = m_bank ????
		{
			m_reg_a002 = data;
			
			membank("bank1")->set_entry((data>>6)&3);
		}
		break;

		case 0x06: // bit 1 = coin lockout/counter ?
		break;

		case 0x08: //A008
			space.device().execute().set_input_line(0,CLEAR_LINE);
		break;
	}
}

READ8_MEMBER(witch_state::prot_read_700x)
{
/*
    Code @$21a looks like simple protection check.

    - write 7,6,0 to $700f
    - read 5 bytes from $7000-$7004 ( bit 1 of $700d is data "READY" status)

    Data @ $7000 must differs from data @$7001-04.
    Otherwise later in game some I/O (controls) reads are skipped.
*/

	switch(space.device().safe_pc())
	{
	case 0x23f:
	case 0x246:
	case 0x24c:
	case 0x252:
	case 0x258:
	case 0x25e:
		return offset;//enough to pass...
	}
	return memregion("sub")->base()[0x7000+offset];
}

/*
 * Status from ES8712?
 * BIT1 is zero when no sample is playing?
 */
READ8_MEMBER(witch_state::read_8010){   return 0x00; }

WRITE8_MEMBER(witch_state::xscroll_w)
{
	m_scrollx=data;
}
WRITE8_MEMBER(witch_state::yscroll_w)
{
	m_scrolly=data;
}

static ADDRESS_MAP_START( map_main, AS_PROGRAM, 8, witch_state )
	AM_RANGE(0x0000, UNBANKED_SIZE-1) AM_ROM
	AM_RANGE(UNBANKED_SIZE, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x8001) AM_DEVREADWRITE("ym1", ym2203_device, read, write)
	AM_RANGE(0x8008, 0x8009) AM_DEVREADWRITE("ym2", ym2203_device, read, write)
	AM_RANGE(0xa000, 0xa00f) AM_READWRITE(read_a00x, write_a00x)
	AM_RANGE(0xc000, 0xc3ff) AM_RAM AM_WRITE(gfx0_vram_w) AM_SHARE("gfx0_vram")
	AM_RANGE(0xc400, 0xc7ff) AM_RAM AM_WRITE(gfx0_cram_w) AM_SHARE("gfx0_cram")
	AM_RANGE(0xc800, 0xcbff) AM_READWRITE(gfx1_vram_r, gfx1_vram_w) AM_SHARE("gfx1_vram")
	AM_RANGE(0xcc00, 0xcfff) AM_READWRITE(gfx1_cram_r, gfx1_cram_w) AM_SHARE("gfx1_cram")
	AM_RANGE(0xd000, 0xdfff) AM_RAM AM_SHARE("sprite_ram")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xe800, 0xefff) AM_RAM_DEVWRITE("palette", palette_device, write_ext) AM_SHARE("palette_ext")
	AM_RANGE(0xf000, 0xf0ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xf100, 0xf17f) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xf180, 0xffff) AM_RAM AM_SHARE("share2")
ADDRESS_MAP_END


static ADDRESS_MAP_START( map_sub, AS_PROGRAM, 8, witch_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8001) AM_DEVREADWRITE("ym1", ym2203_device, read, write)
	AM_RANGE(0x8008, 0x8009) AM_DEVREADWRITE("ym2", ym2203_device, read, write)
	AM_RANGE(0x8010, 0x8016) AM_READ(read_8010) AM_DEVWRITE("essnd", es8712_device, es8712_w)
	AM_RANGE(0xa000, 0xa00f) AM_READWRITE(read_a00x, write_a00x)
	AM_RANGE(0xf000, 0xf0ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xf180, 0xffff) AM_RAM AM_SHARE("share2")
ADDRESS_MAP_END

static INPUT_PORTS_START( witch )
	PORT_START("SERVICE")   /* DSW */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("NVRAM Init") PORT_CODE(KEYCODE_F1)
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Stats")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("A00E")  /* DSW */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Key In")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Reset ?")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("UNK")   /* Not a DSW */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("INPUTS")    /* Inputs */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Flipper")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Big")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Small")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Take")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Double Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Bet")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Right Flipper")

/*
F180 kkkbbppp ; Read onPORT 0xA005
 ppp  = PAY OUT | 60 ; 65 ; 70 ; 75 ; 80 ; 85 ; 90 ; 95
 bb   = MAX BET | 20 ; 30 ; 40 ; 60
 kkk  = KEY IN  | 1-10 ; 1-20 ; 1-40 ; 1-50 ; 1-100 ; 1-200 ; 1-250 ; 1-500
*/
	PORT_START("A005")  /* DSW "SW2" */
	PORT_DIPNAME( 0x07, 0x07, "PAY OUT" )   PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, "60" )
	PORT_DIPSETTING(    0x06, "65" )
	PORT_DIPSETTING(    0x05, "70" )
	PORT_DIPSETTING(    0x04, "75" )
	PORT_DIPSETTING(    0x03, "80" )
	PORT_DIPSETTING(    0x02, "85" )
	PORT_DIPSETTING(    0x01, "90" )
	PORT_DIPSETTING(    0x00, "95" )
	PORT_DIPNAME( 0x18, 0x00, "MAX BET" )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20" )
	PORT_DIPSETTING(    0x10, "30" )
	PORT_DIPSETTING(    0x08, "40" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPNAME( 0xe0, 0xe0, "KEY IN" )    PORT_DIPLOCATION("SW2:6,7,8")
	PORT_DIPSETTING(    0xE0, "1-10"  )
	PORT_DIPSETTING(    0xC0, "1-20"  )
	PORT_DIPSETTING(    0xA0, "1-40"  )
	PORT_DIPSETTING(    0x80, "1-50"  )
	PORT_DIPSETTING(    0x60, "1-100" )
	PORT_DIPSETTING(    0x40, "1-200" )
	PORT_DIPSETTING(    0x20, "1-250" )
	PORT_DIPSETTING(    0x00, "1-500" )
/*
*f181   : ccccxxxd ; Read onPORT 0xA004
 d    = DOUBLE UP | ON ; OFF
 cccc = COIN IN1 | 1-1 ; 1-2 ; 1-3 ; 1-4 ; 1-5 ; 1-6 ; 1-7 ; 1-8 ; 1-9 ; 1-10 ; 1-15 ; 1-20 ; 1-25 ; 1-30 ; 1-40 ; 1-50
*/
	PORT_START("A004")  /* DSW "SW3" Switches 2-4 not defined in manual */
	PORT_DIPNAME( 0x01, 0x00, "DOUBLE UP" )     PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off  ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPNAME( 0xf0, 0xf0, "COIN IN1" )      PORT_DIPLOCATION("SW3:5,6,7,8")
	PORT_DIPSETTING(    0xf0, "1-1" )
	PORT_DIPSETTING(    0xe0, "1-2" )
	PORT_DIPSETTING(    0xd0, "1-3" )
	PORT_DIPSETTING(    0xc0, "1-4" )
	PORT_DIPSETTING(    0xb0, "1-5" )
	PORT_DIPSETTING(    0xa0, "1-6" )
	PORT_DIPSETTING(    0x90, "1-7" )
	PORT_DIPSETTING(    0x80, "1-8" )
	PORT_DIPSETTING(    0x70, "1-9" )
	PORT_DIPSETTING(    0x60, "1-10" )
	PORT_DIPSETTING(    0x50, "1-15" )
	PORT_DIPSETTING(    0x40, "1-20" )
	PORT_DIPSETTING(    0x30, "1-25" )
	PORT_DIPSETTING(    0x20, "1-30" )
	PORT_DIPSETTING(    0x10, "1-40" )
	PORT_DIPSETTING(    0x00, "1-50" )

/*
*f182   : sttpcccc ; Read onPORT A of YM2203 @ 0x8001
 cccc = COIN IN2 | 1-1 ; 1-2 ; 1-3 ; 1-4 ; 1-5 ; 1-6 ; 1-7 ; 1-8 ; 1-9 ; 1-10 ; 2-1 ; 3-1 ; 4-1 ; 5-1 ; 6-1 ; 10-1
 p    = PAYOUT SWITCH | ON ; OFF
 tt   = TIME | 40 ; 45 ; 50 ; 55
 s    = DEMO SOUND | ON ; OFF
*/
	PORT_START("YM_PortA")  /* DSW "SW4" */
	PORT_DIPNAME( 0x0f, 0x0f, "COIN IN2" )      PORT_DIPLOCATION("SW4:1,2,3,4")
	PORT_DIPSETTING(    0x0f, "1-1" )
	PORT_DIPSETTING(    0x0e, "1-2" )
	PORT_DIPSETTING(    0x0d, "1-3" )
	PORT_DIPSETTING(    0x0c, "1-4" )
	PORT_DIPSETTING(    0x0b, "1-5" )
	PORT_DIPSETTING(    0x0a, "1-6" )
	PORT_DIPSETTING(    0x09, "1-7" )
	PORT_DIPSETTING(    0x08, "1-8" )
	PORT_DIPSETTING(    0x07, "1-9" )
	PORT_DIPSETTING(    0x06, "1-10" )
	PORT_DIPSETTING(    0x05, "2-1" )
	PORT_DIPSETTING(    0x04, "3-1" )
	PORT_DIPSETTING(    0x03, "4-1" )
	PORT_DIPSETTING(    0x02, "5-1" )
	PORT_DIPSETTING(    0x01, "6-1" )
	PORT_DIPSETTING(    0x00, "10-1" )
	PORT_DIPNAME( 0x10, 0x00, "PAYOUT SWITCH" ) PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off  ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x00, "TIME" )      PORT_DIPLOCATION("SW4:6,7")
	PORT_DIPSETTING(    0x60, "40" )
	PORT_DIPSETTING(    0x40, "45" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x00, "55" )
	PORT_DIPNAME( 0x80, 0x00, "DEMO SOUND" )    PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off  ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

/*
*f183 : xxxxhllb ; Read onPORT B of YM2203 @ 0x8001
 b    = AUTO BET | ON ; OFF
 ll   = GAME LIMIT | 500 ; 1000 ; 5000 ; 990000
 h    = HOPPER ACTIVE | LOW ; HIGH
*/
	PORT_START("YM_PortB")  /* DSW "SW5" Switches 5, 6 & 8 undefined in manual */
	PORT_DIPNAME( 0x01, 0x01, "AUTO BET" )      PORT_DIPLOCATION("SW5:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off  ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "GAME LIMIT" )    PORT_DIPLOCATION("SW5:2,3")
	PORT_DIPSETTING(    0x06, "500" )
	PORT_DIPSETTING(    0x04, "1000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPSETTING(    0x00, "990000" ) /* 10000 as defined in the Excellent System version manual */
	PORT_DIPNAME( 0x08, 0x08, "HOPPER" )        PORT_DIPLOCATION("SW5:4")
	PORT_DIPSETTING(    0x08, DEF_STR(Low) )
	PORT_DIPSETTING(    0x00, DEF_STR(High) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW5:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW5:6" )
	PORT_DIPNAME( 0x40, 0x00, "Unknown Use" )   PORT_DIPLOCATION("SW5:7") /* As defined in the Excellent System version manual */
	PORT_DIPSETTING(    0x40, "Matrix" )
	PORT_DIPSETTING(    0x00, "Straight (Normal)" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW5:8" )
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2,3 },
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 8, 12, RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+12},
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static GFXDECODE_START( witch )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

void witch_state::video_start()
{
	m_gfx0a_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(witch_state::get_gfx0a_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32,32);
	m_gfx0b_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(witch_state::get_gfx0b_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32,32);
	m_gfx1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(witch_state::get_gfx1_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32,32);

	m_gfx0a_tilemap->set_transparent_pen(0);
	m_gfx0b_tilemap->set_transparent_pen(0);
	m_gfx0a_tilemap->set_palette_offset(0x100);
	m_gfx0b_tilemap->set_palette_offset(0x100);
	m_gfx1_tilemap->set_palette_offset(0x200);
	
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_reg_a002));
}

void witch_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i,sx,sy,tileno,flags,color;
	int flipx=0;
	int flipy=0;

	for(i=0;i<0x800;i+=0x20) {
		sx     = m_sprite_ram[i+1];
		if(sx!=0xF8) {
			tileno = (m_sprite_ram[i]<<2)  | (( m_sprite_ram[i+0x800] & 0x07 ) << 10 );

			sy     = m_sprite_ram[i+2];
			flags  = m_sprite_ram[i+3];

			flipx  = (flags & 0x10 ) ? 1 : 0;
			flipy  = (flags & 0x20 ) ? 1 : 0;

			color  =  flags & 0x0f;


			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				tileno, color,
				flipx, flipy,
				sx+8*flipx,sy+8*flipy,0);

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				tileno+1, color,
				flipx, flipy,
				sx+8-8*flipx,sy+8*flipy,0);

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				tileno+2, color,
				flipx, flipy,
				sx+8*flipx,sy+8-8*flipy,0);

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				tileno+3, color,
				flipx, flipy,
				sx+8-8*flipx,sy+8-8*flipy,0);

		}
	}

}

UINT32 witch_state::screen_update_witch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_gfx1_tilemap->set_scrollx(0, m_scrollx-7 ); //offset to have it aligned with the sprites
	m_gfx1_tilemap->set_scrolly(0, m_scrolly+8 );



	m_gfx1_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_gfx0a_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap, cliprect);
	m_gfx0b_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

static MACHINE_CONFIG_START( witch, witch_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, CPU_CLOCK)    /* 3 MHz */
	MCFG_CPU_PROGRAM_MAP(map_main)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", witch_state,  irq0_line_assert)

	/* 2nd z80 */
	MCFG_CPU_ADD("sub", Z80, CPU_CLOCK)    /* 3 MHz */
	MCFG_CPU_PROGRAM_MAP(map_sub)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", witch_state,  irq0_line_assert)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(8, 256-1-8, 8*4, 256-8*4-1)
	MCFG_SCREEN_UPDATE_DRIVER(witch_state, screen_update_witch)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", witch)
	MCFG_PALETTE_ADD("palette", 0x800)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_ES8712_ADD("essnd", ES8712_CLOCK)          /* 8Khz, it's the only clock for sure (pin13) it comes from pin14 of M5205 */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("ym1", YM2203, YM2203_CLOCK)     /* 3 MHz */
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("YM_PortA"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("YM_PortB"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_SOUND_ADD("ym2", YM2203, YM2203_CLOCK)     /* 3 MHz */
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(witch_state, xscroll_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(witch_state, yscroll_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

MACHINE_CONFIG_END

ROM_START( witch )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "u_5b.u5", 0x10000, 0x20000, CRC(5c9f685a) SHA1(b75950048009ffb8c3b356592b1c69f905a1a2bd) )
	ROM_COPY( "maincpu" , 0x10000, 0x0000, 0x8000 )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "6.s6", 0x00000, 0x08000, CRC(82460b82) SHA1(d85a9d77edaa67dfab8ff6ac4cb6273f0904b3c0) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "3.u3", 0x00000, 0x20000,  CRC(7007ced4) SHA1(6a0aac3ff9a4d5360c8ba1142f010add1b430ada) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "5.a1", 0x00000, 0x40000,  CRC(fc37a9c2) SHA1(940d8c53d47eaa93a85a91e4ecb92fc4912d331d) )

	ROM_REGION( 0x40000, "essnd", 0 )
	ROM_LOAD( "1.v10", 0x00000, 0x40000, CRC(62e42371) SHA1(5042abc2176d0c35fd6b698eca4145f93b0a3944) )

	ROM_REGION( 0x100, "prom", 0 )
	ROM_LOAD( "tbp24s10n.10k", 0x000, 0x100, CRC(ee7b9d8f) SHA1(3a7b75befab83bc37e4e403ad3632841c2d37707) ) /* Currently unused, unknown use */
ROM_END


/* Witch (With ranking) */
ROM_START( witchb )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "x.u5", 0x10000, 0x20000, CRC(d0818777) SHA1(a6232fef84bec3cfb4a6122a48e96e7b7950e013) )
	ROM_COPY( "maincpu" , 0x10000, 0x0000, 0x8000 )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "6.s6", 0x00000, 0x08000, CRC(82460b82) SHA1(d85a9d77edaa67dfab8ff6ac4cb6273f0904b3c0) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "3.u3", 0x00000, 0x20000,  CRC(7007ced4) SHA1(6a0aac3ff9a4d5360c8ba1142f010add1b430ada) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "5.a1", 0x00000, 0x40000,  CRC(fc37a9c2) SHA1(940d8c53d47eaa93a85a91e4ecb92fc4912d331d) )

	ROM_REGION( 0x40000, "essnd", 0 )
	ROM_LOAD( "1.v10", 0x00000, 0x40000, CRC(62e42371) SHA1(5042abc2176d0c35fd6b698eca4145f93b0a3944) )

	ROM_REGION( 0x100, "prom", 0 )
	ROM_LOAD( "tbp24s10n.10k", 0x000, 0x100, CRC(ee7b9d8f) SHA1(3a7b75befab83bc37e4e403ad3632841c2d37707) ) /* Currently unused, unknown use */
ROM_END


ROM_START( witchs ) /* this set has (c)1992 Sega / Vic Tokai in the roms */
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "rom.u5", 0x10000, 0x20000, CRC(348fccb8) SHA1(947defd86c4a597fbfb9327eec4903aa779b3788) )
	ROM_COPY( "maincpu" , 0x10000, 0x0000, 0x8000 )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "6.s6", 0x00000, 0x08000, CRC(82460b82) SHA1(d85a9d77edaa67dfab8ff6ac4cb6273f0904b3c0) ) /* Same data as the Witch set */

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "3.u3", 0x00000, 0x20000,  CRC(7007ced4) SHA1(6a0aac3ff9a4d5360c8ba1142f010add1b430ada) ) /* Same data as the Witch set */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "rom.a1", 0x00000, 0x40000,  CRC(512300a5) SHA1(1e9ba58d1ddbfb8276c68f6d5c3591e6b77abf21) )

	ROM_REGION( 0x40000, "essnd", 0 )
	ROM_LOAD( "1.v10", 0x00000, 0x40000, CRC(62e42371) SHA1(5042abc2176d0c35fd6b698eca4145f93b0a3944) ) /* Same data as the Witch set */

	ROM_REGION( 0x100, "prom", 0 )
	ROM_LOAD( "tbp24s10n.10k", 0x000, 0x100, CRC(ee7b9d8f) SHA1(3a7b75befab83bc37e4e403ad3632841c2d37707) ) /* Currently unused, unknown use */
ROM_END


ROM_START( pbchmp95 ) /* Licensed for Germany? */
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "3.bin", 0x10000, 0x20000, CRC(e881aa05) SHA1(10d259396cac4b9a1b72c262c11ffa5efbdac433) )
	ROM_COPY( "maincpu" , 0x10000, 0x0000, 0x8000 )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "4.bin", 0x00000, 0x08000, CRC(82460b82) SHA1(d85a9d77edaa67dfab8ff6ac4cb6273f0904b3c0) ) /* Same data as the Witch set */

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "2.bin", 0x00000, 0x20000,  CRC(7007ced4) SHA1(6a0aac3ff9a4d5360c8ba1142f010add1b430ada) ) /* Same data as the Witch set */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x40000,  CRC(f6cf7ed6) SHA1(327580a17eb2740fad974a01d97dad0a4bef9881) )

	ROM_REGION( 0x40000, "essnd", 0 )
	ROM_LOAD( "5.bin", 0x00000, 0x40000, CRC(62e42371) SHA1(5042abc2176d0c35fd6b698eca4145f93b0a3944) ) /* Same data as the Witch set */

	ROM_REGION( 0x100, "prom", 0 )
	ROM_LOAD( "tbp24s10n.10k", 0x000, 0x100, CRC(ee7b9d8f) SHA1(3a7b75befab83bc37e4e403ad3632841c2d37707) ) /* Currently unused, unknown use */
ROM_END

DRIVER_INIT_MEMBER(witch_state,witch)
{
	membank("bank1")->configure_entries(0, 4, memregion("maincpu")->base() + 0x10000 + UNBANKED_SIZE, 0x8000);
	membank("bank1")->set_entry(0);

	m_subcpu->space(AS_PROGRAM).install_read_handler(0x7000, 0x700f, read8_delegate(FUNC(witch_state::prot_read_700x), this));
}

GAME( 1992, witch,    0,     witch, witch, witch_state, witch, ROT0, "Excellent System",     "Witch",                MACHINE_SUPPORTS_SAVE )
GAME( 1992, witchb,   witch, witch, witch, witch_state, witch, ROT0, "Excellent System",     "Witch (With ranking)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, witchs,   witch, witch, witch, witch_state, witch, ROT0, "Sega / Vic Tokai",     "Witch (Sega License)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, pbchmp95, witch, witch, witch, witch_state, witch, ROT0, "Veltmeijer Automaten", "Pinball Champ '95",    MACHINE_SUPPORTS_SAVE )
