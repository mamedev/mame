/*

Raiden 2 / DX V33 Version

Temporary split from raiden2.c, it'll be re-merged at some point.

Note:
Please don't do any state machine refactoring of this.

0x430-0x433
1) appears to control where the first enemies turns, if clockwise or anticlockwise
2) ...

Raiden 2 / DX checks if there's the string "RAIDEN" at start-up inside the eeprom, otherwise it dies.
Then it puts settings at 0x9e08 and 0x9e0a (bp 91acb)

*/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "audio/seibu.h"
#include "machine/eeprom.h"
#include "sound/okim6295.h"
#include "includes/raiden2.h"


class r2dx_v33_state : public driver_device
{
public:
	r2dx_v33_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_spriteram;
	DECLARE_WRITE16_MEMBER(rdx_bg_vram_w);
	DECLARE_WRITE16_MEMBER(rdx_md_vram_w);
	DECLARE_WRITE16_MEMBER(rdx_fg_vram_w);
	DECLARE_WRITE16_MEMBER(rdx_tx_vram_w);
	DECLARE_READ16_MEMBER(rdx_v33_unknown_r);
	DECLARE_WRITE16_MEMBER(mcu_xval_w);
	DECLARE_WRITE16_MEMBER(mcu_yval_w);
	DECLARE_WRITE16_MEMBER(mcu_table_w);
	DECLARE_WRITE16_MEMBER(mcu_table2_w);
	DECLARE_READ16_MEMBER(nzerotea_sound_comms_r);
	DECLARE_WRITE16_MEMBER(nzerotea_sound_comms_w);
	DECLARE_WRITE16_MEMBER(mcu_prog_w);
	DECLARE_WRITE16_MEMBER(mcu_prog_w2);
	DECLARE_WRITE16_MEMBER(mcu_prog_offs_w);
};


static UINT16 *seibu_crtc_regs;
static UINT16 *bg_vram,*md_vram,*fg_vram,*tx_vram;
static tilemap_t *bg_tilemap,*md_tilemap,*fg_tilemap,*tx_tilemap;

static TILE_GET_INFO( get_bg_tile_info )
{
	int tile = bg_vram[tile_index];
	int color = (tile>>12)&0xf;

	tile &= 0xfff;

	SET_TILE_INFO(1,tile + 0x0000,color,0);
}

static TILE_GET_INFO( get_md_tile_info )
{
	int tile = md_vram[tile_index];
	int color = (tile>>12)&0xf;

	tile &= 0xfff;

	SET_TILE_INFO(2,tile + 0x2000,color,0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int tile = fg_vram[tile_index];
	int color = (tile>>12)&0xf;

	tile &= 0xfff;

	SET_TILE_INFO(3,tile + 0x1000,color,0);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	int tile = tx_vram[tile_index];
	int color = (tile>>12)&0xf;

	tile &= 0xfff;

	SET_TILE_INFO(4,tile,color,0);
}

/* copied from Legionnaire */
static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect,int pri)
{
	r2dx_v33_state *state = machine.driver_data<r2dx_v33_state>();
	UINT16 *spriteram16 = state->m_spriteram;
	int offs,fx,fy,x,y,color,sprite;
//  int cur_pri;
	int dx,dy,ax,ay;

	for (offs = 0x400-4;offs >= 0;offs -= 4)
	{
		UINT16 data = spriteram16[offs];
		//if (!(data &0x8000)) continue;

		//cur_pri = (spriteram16[offs+1] & 0xc000) >> 14;
		//if (cur_pri!=pri) continue;

		sprite = spriteram16[offs+1];

		//sprite &= 0x7fff;
		//if(data & 0x8000) sprite |= 0x4000;
		//if(spriteram16[offs+3] & 0x8000) sprite |= 0x8000;//tile banking?,used in Denjin Makai

		y = spriteram16[offs+3];
		x = spriteram16[offs+2];

		x&=0xfff;
		y&=0xfff;

		if (x&0x8000) x-=0x10000;
		if (y&0x8000) y-=0x10000;

		color = (data &0x3f);
		fx =  (data &0x8000) >> 15;
		fy =  (data &0x0800) >> 11;
		dx = ((data &0x0700) >> 8)  + 1;
		dy = ((data &0x7000) >> 12) + 1;

		if (!fx)
		{
			if(!fy)
			{
				for (ax=0; ax<dx; ax++)
					for (ay=0; ay<dy; ay++)
					{
						drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
						sprite++,
						color,fx,fy,x+ax*16,y+ay*16,15);
					}
			}
			else
			{
				for (ax=0; ax<dx; ax++)
					for (ay=0; ay<dy; ay++)
					{
						drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
						sprite++,
						color,fx,fy,x+ax*16,y+(dy-ay-1)*16,15);
					}
			}
		}
		else
		{
			if(!fy)
			{
				for (ax=0; ax<dx; ax++)
					for (ay=0; ay<dy; ay++)
					{
						drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
						sprite++,
						color,fx,fy,x+(dx-ax-1)*16,y+ay*16,15);
					}
			}
			else
			{
				for (ax=0; ax<dx; ax++)
					for (ay=0; ay<dy; ay++)
					{
						drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
						sprite++,
						color,fx,fy,x+(dx-ax-1)*16,y+(dy-ay-1)*16,15);
					}
			}
		}
	}
}

static VIDEO_START( rdx_v33 )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,16,16,32,32);
	md_tilemap = tilemap_create(machine, get_md_tile_info, tilemap_scan_rows,16,16,32,32);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,16,16,32,32);
	tx_tilemap = tilemap_create(machine, get_tx_tile_info, tilemap_scan_rows,8, 8, 64,32);

	bg_tilemap->set_transparent_pen(15);
	md_tilemap->set_transparent_pen(15);
	fg_tilemap->set_transparent_pen(15);
	tx_tilemap->set_transparent_pen(15);
}

static SCREEN_UPDATE_IND16( rdx_v33 )
{
	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	bg_tilemap->draw(bitmap, cliprect, 0, 0);
	md_tilemap->draw(bitmap, cliprect, 0, 0);
	fg_tilemap->draw(bitmap, cliprect, 0, 0);

	draw_sprites(screen.machine(),bitmap,cliprect,0);

	tx_tilemap->draw(bitmap, cliprect, 0, 0);

	/* debug DMA processing */
	if(0)
	{
		static UINT32 src_addr = 0x100000;
		static int frame;
		address_space *space = screen.machine().device("maincpu")->memory().space(AS_PROGRAM);

		//if(screen.machine().input().code_pressed_once(KEYCODE_A))
		//  src_addr+=0x800;

		//if(screen.machine().input().code_pressed_once(KEYCODE_S))
		//  src_addr-=0x800;

		frame++;

		popmessage("%08x 0",src_addr);

		//if(screen.machine().input().code_pressed_once(KEYCODE_Z))
		if(frame == 5)
		{
			int i,data;
			static UINT8 *rom = space->machine().region("mainprg")->base();

			for(i=0;i<0x800;i+=2)
			{
				data = rom[src_addr+i+0];
				space->write_byte(i+0xd000+0, data);
				data = rom[src_addr+i+1];
				space->write_byte(i+0xd000+1, data);
			}

			popmessage("%08x 1",src_addr);
			bg_tilemap->mark_all_dirty();
			frame = 0;
			src_addr+=0x800;
		}
	}
	return 0;
}

WRITE16_DEVICE_HANDLER( rdx_v33_eeprom_w )
{
	if (ACCESSING_BITS_0_7)
	{
		eeprom_device *eeprom = downcast<eeprom_device *>(device);
		eeprom->set_clock_line((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);
		eeprom->write_bit(data & 0x20);
		eeprom->set_cs_line((data & 0x08) ? CLEAR_LINE : ASSERT_LINE);

		if (data&0xc7) logerror("eeprom_w extra bits used %04x\n",data);
	}
	else
	{
		logerror("eeprom_w MSB used %04x",data);
	}
}

/* new zero team uses the copd3 protection... and uploads a 0x400 byte table, probably the mcu code, encrypted */


static UINT16 mcu_prog[0x800];
static int mcu_prog_offs = 0;

WRITE16_MEMBER(r2dx_v33_state::mcu_prog_w)
{
	mcu_prog[mcu_prog_offs*2] = data;
}

WRITE16_MEMBER(r2dx_v33_state::mcu_prog_w2)
{
	mcu_prog[mcu_prog_offs*2+1] = data;

	// both new zero team and raiden2/dx v33 version upload the same table..
#if 0
    {
		char tmp[64];
        FILE *fp;
	    sprintf(tmp,"cop3_%s.data", machine().system().name);

		fp=fopen(tmp, "w+b");
        if (fp)
        {
            fwrite(mcu_prog, 0x400, 2, fp);
            fclose(fp);
        }
    }
#endif
}

WRITE16_MEMBER(r2dx_v33_state::mcu_prog_offs_w)
{
	mcu_prog_offs = data;
}

WRITE16_MEMBER(r2dx_v33_state::rdx_bg_vram_w)
{
	COMBINE_DATA(&bg_vram[offset]);
	bg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(r2dx_v33_state::rdx_md_vram_w)
{
	COMBINE_DATA(&md_vram[offset]);
	md_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(r2dx_v33_state::rdx_fg_vram_w)
{
	COMBINE_DATA(&fg_vram[offset]);
	fg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(r2dx_v33_state::rdx_tx_vram_w)
{
	COMBINE_DATA(&tx_vram[offset]);
	tx_tilemap->mark_tile_dirty(offset);
}

READ16_MEMBER(r2dx_v33_state::rdx_v33_unknown_r)
{
	return machine().rand();
}


static UINT16 mcu_xval,mcu_yval;

/* something sent to the MCU for X/Y global screen calculating ... */
WRITE16_MEMBER(r2dx_v33_state::mcu_xval_w)
{
	mcu_xval = data;
	//popmessage("%04x %04x",mcu_xval,mcu_yval);
}

WRITE16_MEMBER(r2dx_v33_state::mcu_yval_w)
{
	mcu_yval = data;
	//popmessage("%04x %04x",mcu_xval,mcu_yval);
}

static UINT16 mcu_data[9];

/* 0x400-0x407 seems some DMA hook-up, 0x420-0x427 looks like some x/y sprite calculation routine */
WRITE16_MEMBER(r2dx_v33_state::mcu_table_w)
{
	mcu_data[offset] = data;

	//popmessage("%04x %04x %04x %04x | %04x %04x %04x %04x",mcu_data[0/2],mcu_data[2/2],mcu_data[4/2],mcu_data[6/2],mcu_data[8/2],mcu_data[0xa/2],mcu_data[0xc/2],mcu_data[0xe/2]);
}

WRITE16_MEMBER(r2dx_v33_state::mcu_table2_w)
{
	mcu_data[offset+4] = data;

	//popmessage("%04x %04x %04x %04x | %04x %04x %04x %04x",mcu_data[0/2],mcu_data[2/2],mcu_data[4/2],mcu_data[6/2],mcu_data[8/2],mcu_data[0xa/2],mcu_data[0xc/2],mcu_data[0xe/2]);
}


static ADDRESS_MAP_START( rdx_v33_map, AS_PROGRAM, 16, r2dx_v33_state )
	AM_RANGE(0x00000, 0x003ff) AM_RAM // vectors copied here

	AM_RANGE(0x00400, 0x00407) AM_WRITE(mcu_table_w)
	AM_RANGE(0x00420, 0x00429) AM_WRITE(mcu_table2_w)

	/* results from cop? */
	AM_RANGE(0x00430, 0x00431) AM_READ(rdx_v33_unknown_r)
	AM_RANGE(0x00432, 0x00433) AM_READ(rdx_v33_unknown_r)
	AM_RANGE(0x00434, 0x00435) AM_READ(rdx_v33_unknown_r)
	AM_RANGE(0x00436, 0x00437) AM_READ(rdx_v33_unknown_r)

	AM_RANGE(0x00600, 0x0064f) AM_RAM AM_BASE_LEGACY(&seibu_crtc_regs)
	AM_RANGE(0x00650, 0x0068f) AM_RAM //???

	AM_RANGE(0x0068e, 0x0068f) AM_WRITENOP // synch for the MCU?
	AM_RANGE(0x006b0, 0x006b1) AM_WRITE(mcu_prog_w)
	AM_RANGE(0x006b2, 0x006b3) AM_WRITE(mcu_prog_w2)
//  AM_RANGE(0x006b4, 0x006b5) AM_WRITENOP
//  AM_RANGE(0x006b6, 0x006b7) AM_WRITENOP
	AM_RANGE(0x006bc, 0x006bd) AM_WRITE(mcu_prog_offs_w)
	AM_RANGE(0x006be, 0x006bf) AM_WRITENOP // MCU program related
	AM_RANGE(0x006d8, 0x006d9) AM_WRITE(mcu_xval_w)
	AM_RANGE(0x006da, 0x006db) AM_WRITE(mcu_yval_w)
//  AM_RANGE(0x006dc, 0x006dd) AM_READ_LEGACY(rdx_v33_unknown2_r)
//  AM_RANGE(0x006de, 0x006df) AM_WRITE_LEGACY(mcu_unkaa_w) // mcu command related?

	AM_RANGE(0x00700, 0x00701) AM_DEVWRITE_LEGACY("eeprom", rdx_v33_eeprom_w)
//  AM_RANGE(0x00740, 0x00741) AM_READ_LEGACY(rdx_v33_unknown2_r)
	AM_RANGE(0x00744, 0x00745) AM_READ_PORT("INPUT")
	AM_RANGE(0x0074c, 0x0074d) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x00762, 0x00763) AM_READNOP

	AM_RANGE(0x00780, 0x00781) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff) // single OKI chip on this version

	AM_RANGE(0x00800, 0x00fff) AM_RAM // copies eeprom here?
	AM_RANGE(0x01000, 0x0bfff) AM_RAM

	AM_RANGE(0x0c000, 0x0c7ff) AM_RAM AM_BASE(m_spriteram)
	AM_RANGE(0x0c800, 0x0cfff) AM_RAM
	AM_RANGE(0x0d000, 0x0d7ff) AM_RAM_WRITE(rdx_bg_vram_w) AM_BASE_LEGACY(&bg_vram)
	AM_RANGE(0x0d800, 0x0dfff) AM_RAM_WRITE(rdx_md_vram_w) AM_BASE_LEGACY(&md_vram)
	AM_RANGE(0x0e000, 0x0e7ff) AM_RAM_WRITE(rdx_fg_vram_w) AM_BASE_LEGACY(&fg_vram)
	AM_RANGE(0x0e800, 0x0f7ff) AM_RAM_WRITE(rdx_tx_vram_w) AM_BASE_LEGACY(&tx_vram)
	AM_RANGE(0x0f800, 0x0ffff) AM_RAM /* Stack area */
	AM_RANGE(0x10000, 0x1efff) AM_RAM
	AM_RANGE(0x1f000, 0x1ffff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_SHARE("paletteram")

	AM_RANGE(0x20000, 0x3ffff) AM_ROMBANK("bank1")
	AM_RANGE(0x40000, 0xfffff) AM_ROM AM_REGION("mainprg", 0x40000 )
ADDRESS_MAP_END

READ16_MEMBER(r2dx_v33_state::nzerotea_sound_comms_r)
{
	switch(offset+0x780)
	{
		case (0x788/2):	return seibu_main_word_r(&space,2,0xffff);
		case (0x78c/2):	return seibu_main_word_r(&space,3,0xffff);
		case (0x794/2): return seibu_main_word_r(&space,5,0xffff);
	}

	return 0xffff;
}


WRITE16_MEMBER(r2dx_v33_state::nzerotea_sound_comms_w)
{
	switch(offset+0x780)
	{
		case (0x780/2): { seibu_main_word_w(&space,0,data,0x00ff); break; }
		case (0x784/2): { seibu_main_word_w(&space,1,data,0x00ff); break; }
		//case (0x790/2): { seibu_main_word_w(&space,4,data,0x00ff); break; }
		case (0x794/2): { seibu_main_word_w(&space,4,data,0x00ff); break; }
		case (0x798/2): { seibu_main_word_w(&space,6,data,0x00ff); break; }
	}
}

static ADDRESS_MAP_START( nzerotea_map, AS_PROGRAM, 16, r2dx_v33_state )
	AM_RANGE(0x00000, 0x003ff) AM_RAM //stack area

	/* results from cop? */
	AM_RANGE(0x00430, 0x00431) AM_READ(rdx_v33_unknown_r)
	AM_RANGE(0x00432, 0x00433) AM_READ(rdx_v33_unknown_r)
	AM_RANGE(0x00434, 0x00435) AM_READ(rdx_v33_unknown_r)
	AM_RANGE(0x00436, 0x00437) AM_READ(rdx_v33_unknown_r)

	AM_RANGE(0x00400, 0x00407) AM_WRITE(mcu_table_w)
	AM_RANGE(0x00420, 0x00427) AM_WRITE(mcu_table2_w)

	AM_RANGE(0x00600, 0x0064f) AM_RAM AM_BASE_LEGACY(&seibu_crtc_regs)

	AM_RANGE(0x0068e, 0x0068f) AM_WRITENOP // synch for the MCU?
	AM_RANGE(0x006b0, 0x006b1) AM_WRITE(mcu_prog_w)
	AM_RANGE(0x006b2, 0x006b3) AM_WRITE(mcu_prog_w2)
//  AM_RANGE(0x006b4, 0x006b5) AM_WRITENOP
//  AM_RANGE(0x006b6, 0x006b7) AM_WRITENOP
	AM_RANGE(0x006bc, 0x006bd) AM_WRITE(mcu_prog_offs_w)
//  AM_RANGE(0x006d8, 0x006d9) AM_WRITE_LEGACY(bbbbll_w) // scroll?
//  AM_RANGE(0x006dc, 0x006dd) AM_READ_LEGACY(nzerotea_unknown_r)
//  AM_RANGE(0x006de, 0x006df) AM_WRITE_LEGACY(mcu_unkaa_w) // mcu command related?
	//AM_RANGE(0x00700, 0x00701) AM_DEVWRITE_LEGACY("eeprom", rdx_v33_eeprom_w)
	AM_RANGE(0x00740, 0x00741) AM_READ_PORT("DSW")
	AM_RANGE(0x00744, 0x00745) AM_READ_PORT("INPUT")
	AM_RANGE(0x0074c, 0x0074d) AM_READ_PORT("SYSTEM")
//  AM_RANGE(0x00762, 0x00763) AM_READ_LEGACY(nzerotea_unknown_r)

	AM_RANGE(0x00780, 0x0079f) AM_READWRITE(nzerotea_sound_comms_r,nzerotea_sound_comms_w)

	AM_RANGE(0x00800, 0x00fff) AM_RAM
	AM_RANGE(0x01000, 0x0bfff) AM_RAM

	AM_RANGE(0x0c000, 0x0c7ff) AM_RAM AM_BASE(m_spriteram)
	AM_RANGE(0x0c800, 0x0cfff) AM_RAM
	AM_RANGE(0x0d000, 0x0d7ff) AM_RAM_WRITE(rdx_bg_vram_w) AM_BASE_LEGACY(&bg_vram)
	AM_RANGE(0x0d800, 0x0dfff) AM_RAM_WRITE(rdx_md_vram_w) AM_BASE_LEGACY(&md_vram)
	AM_RANGE(0x0e000, 0x0e7ff) AM_RAM_WRITE(rdx_fg_vram_w) AM_BASE_LEGACY(&fg_vram)
	AM_RANGE(0x0e800, 0x0f7ff) AM_RAM_WRITE(rdx_tx_vram_w) AM_BASE_LEGACY(&tx_vram)
	AM_RANGE(0x0f800, 0x0ffff) AM_RAM /* Stack area */
	AM_RANGE(0x10000, 0x1efff) AM_RAM
	AM_RANGE(0x1f000, 0x1ffff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_SHARE("paletteram")

	AM_RANGE(0x20000, 0x3ffff) AM_ROMBANK("bank1")
	AM_RANGE(0x40000, 0xfffff) AM_ROM AM_REGION("mainprg", 0x40000 )
ADDRESS_MAP_END

static INTERRUPT_GEN( rdx_v33_interrupt )
{
	device_set_input_line_and_vector(device, 0, HOLD_LINE, 0xc0/4);	/* VBL */
}

static const gfx_layout rdx_v33_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 8,12,0,4 },
	{ 3,2,1,0,19,18,17,16 },
	{ STEP8(0,32) },
	32*8
};


static const gfx_layout rdx_v33_tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 8,12,0,4 },
	{
		3,2,1,0,
		19,18,17,16,
		3+64*8, 2+64*8, 1+64*8, 0+64*8,
		19+64*8,18+64*8,17+64*8,16+64*8,
	},
	{ STEP16(0,32) },
	128*8
};

static const gfx_layout rdx_v33_spritelayout =
{
	16, 16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 4, 0, 12, 8, 20, 16, 28, 24, 36, 32, 44, 40, 52, 48, 60, 56 },
	{ STEP16(0,64) },
	16*16*4
};

static GFXDECODE_START( rdx_v33 )
	GFXDECODE_ENTRY( "gfx3", 0x00000, rdx_v33_spritelayout, 0x000, 0x40 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, rdx_v33_tilelayout,   0x400, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, rdx_v33_tilelayout,   0x500, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, rdx_v33_tilelayout,   0x600, 0x10 )
	GFXDECODE_ENTRY( "gfx1", 0x00000, rdx_v33_charlayout,   0x700, 0x10 )
	GFXDECODE_ENTRY( "gfx1", 0x00000, rdx_v33_tilelayout,   0x700, 0x10 ) // debugging, to be removed
GFXDECODE_END

static INPUT_PORTS_START( rdx_v33 )
   PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_device, read_bit)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0040, 0x0040, "Test Mode" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUT")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END


static INPUT_PORTS_START( nzerotea )

	SEIBU_COIN_INPUTS	/* coin inputs read through sound cpu */

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	//PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_device, read_bit)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0040, 0x0040, "Test Mode" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUT")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static MACHINE_CONFIG_START( rdx_v33, r2dx_v33_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V33, 32000000/2 ) // ?
	MCFG_CPU_PROGRAM_MAP(rdx_v33_map)
	MCFG_CPU_VBLANK_INT("screen", rdx_v33_interrupt)

	//MCFG_MACHINE_RESET(rdx_v33)

	MCFG_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_STATIC(rdx_v33)

	MCFG_GFXDECODE(rdx_v33)
	MCFG_PALETTE_LENGTH(2048)

	MCFG_VIDEO_START(rdx_v33)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( nzerotea, r2dx_v33_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V33,XTAL_32MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(nzerotea_map)
	MCFG_CPU_VBLANK_INT("screen", rdx_v33_interrupt)

	MCFG_MACHINE_RESET(seibu_sound)

//  SEIBU2_RAIDEN2_SOUND_SYSTEM_CPU(14318180/4)
	SEIBU_NEWZEROTEAM_SOUND_SYSTEM_CPU(14318180/4)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(55.47)    /* verified on pcb */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate *//2)
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(rdx_v33)
	MCFG_GFXDECODE(rdx_v33)
	MCFG_PALETTE_LENGTH(2048)

	MCFG_VIDEO_START(rdx_v33)

	/* sound hardware */
//  SEIBU_SOUND_SYSTEM_YM2151_RAIDEN2_INTERFACE(28636360/8,28636360/28,1,2)
	SEIBU_SOUND_SYSTEM_YM3812_INTERFACE(14318180/4,1320000)

MACHINE_CONFIG_END


static DRIVER_INIT(rdx_v33)
{
	memory_configure_bank(machine, "bank1", 0, 0x20, machine.region("mainprg")->base(), 0x20000);

	raiden2_decrypt_sprites(machine);

	memory_set_bank(machine, "bank1", 1);
}

static DRIVER_INIT(nzerotea)
{
	memory_configure_bank(machine, "bank1", 0, 2, machine.region("mainprg")->base(), 0x20000);

	raiden2_decrypt_sprites(machine);

	memory_set_bank(machine, "bank1", 1);

}

/*

Raiden DX
Seibu Kaihatsu, 1993/1996

Note! PCB seems like an updated version. It uses _entirely_ SMD technology and
is smaller than the previous hardware. I guess the game is still popular, so
Seibu re-manufactured it using newer technology to meet demand.
Previous version hardware is similar to Heated Barrel/Legionairre/Seibu Cup Soccer etc.
It's possible that the BG and OBJ ROMs from this set can be used to complete the
previous (incomplete) dump that runs on the V30 hardware, since most GFX chips are the same.

PCB ID: (C) 1996 JJ4-China-Ver2.0 SEIBU KAIHATSU INC., MADE IN JAPAN
CPU   : NEC 70136AL-16 (V33)
SOUND : Oki M6295
OSC   : 28.636360MHz
RAM   : CY7C199-15 (28 Pin SOIC, x11)
        Breakdown of RAM locations...
                                     (x2 near SIE150)
                                     (x3 near SEI252)
                                     (x2 near SEI0200)
                                     (x4 near SEI360)

DIPs  : 8 position (x1)
        1-6 OFF   (NOT USED)
        7   OFF = Normal Mode  , ON = Test/Setting Mode
        8   OFF = Normal Screen, ON = FLIP Screen

OTHER : Controls are 8-way + 3 Buttons
        Amtel 93C46 EEPROM (SOIC8)
        PALCE16V8 (x1, near BG ROM, SOIC20)
        SEIBU SEI360 SB06-1937   (160 pin PQFP)
        SEIBI SIE150             (100 pin PQFP, Note SIE, not a typo)
        SEIBU SEI252             (208 pin PQFP)
        SEIBU SEI333             (208 pin PQFP)
        SEIBU SEI0200 TC110G21AF (100 pin PQFP)

        Note: Most of the custom SEIBU chips are the same as the ones used on the
              previous version hardware.

ROMs  :   (filename is PCB label, extension is PCB 'u' location)

              ROM                ROM                 Probably               Byte
Filename      Label              Type                Used...        Note    C'sum
---------------------------------------------------------------------------------
PCM.099       RAIDEN-X SOUND     LH538100  (SOP32)   Oki Samples      0     8539h
FIX.613       RAIDEN-X FIX       LH532048  (SOP40)   ? (BG?)          1     182Dh
COPX_D3.357   RAIDEN-X 333       LH530800A (SOP32)   Protection?      2     CEE4h
PRG.223       RAIDEN-X CHR-4A1   MX23C3210 (SOP44)   V33 program      3     F276h
OBJ1.724      RAIDEN-X CHR1      MX23C3210 (SOP44)   Motion Objects   4     4148h
OBJ2.725      RAIDEN-X CHR2      MX23C3210 (SOP44)   Motion Objects   4     00C3h
BG.612        RAIDEN-X CHR3      MX23C3210 (SOP44)   Backgrounds      5     3280h


Notes
0. Located near Oki M6295
1. Located near SEI0200 and BG ROM
2. Located near SEI333
3. Located near V33 and SEI333
4. Located near V33 and SEI252
5. Located near FIX ROM and SEI0200

*/


ROM_START( r2dx_v33 )
	ROM_REGION( 0x400000, "mainprg", 0 ) /* v33 main cpu */
	ROM_LOAD("prg.223",   0x000000, 0x400000, CRC(b3dbcf98) SHA1(30d6ec2090531c8c579dff74c4898889902d7d87) )

	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF ) /* v33 main cpu */

//  ROM_REGION( 0x20000, "cpu1", ROMREGION_ERASE00 ) /* 64k code for sound Z80 */
	/* nothing?  no z80*/

	ROM_REGION( 0x040000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "fix.613",	0x000000,	0x040000,	CRC(3da27e39) SHA1(3d446990bf36dd0a3f8fadb68b15bed54904c8b5) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "bg.612",   0x000000, 0x400000, CRC(162c61e9) SHA1(bd0a6a29804b84196ba6bf3402e9f30a25da9269) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "obj1.724",  0x000000, 0x400000, CRC(7d218985) SHA1(777241a533defcbea3d7e735f309478d260bad52) )
	ROM_LOAD32_WORD( "obj2.725",  0x000002, 0x400000, CRC(b09434d9) SHA1(da75252b7693ab791fece4c10b8a4910edb76c88) )

	ROM_REGION( 0x100000, "oki", 0 )	/* ADPCM samples */
	ROM_LOAD( "pcm.099", 0x00000, 0x100000, CRC(97ca2907) SHA1(bfe8189300cf72089d0beaeab8b1a0a1a4f0a5b6) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	ROM_LOAD( "copx_d3.357",   0x00000, 0x20000, CRC(fa2cf3ad) SHA1(13eee40704d3333874b6e3da9ee7d969c6dc662a) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-r2dx_v33.bin", 0x0000, 0x0080, CRC(ba454777) SHA1(101c5364e8664d17bfb1e759515d135a2673d67e) )
ROM_END

/* Different hardware, uses COPX-D3 for protection  */
ROM_START( nzerotea )
	ROM_REGION( 0x100000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("prg1",   0x000000, 0x80000, CRC(3c7d9410) SHA1(25f2121b6c2be73f11263934266901ed5d64d2ee) )
	ROM_LOAD16_BYTE("prg2",   0x000001, 0x80000, CRC(6cba032d) SHA1(bf5d488cd578fff09e62e3650efdee7658033e3f) )

	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF ) /* v33 main cpu */

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	/* not from this set, assumed to be the same */
	ROM_LOAD( "copx-d3.bin",   0x00000, 0x20000, BAD_DUMP CRC(fa2cf3ad) SHA1(13eee40704d3333874b6e3da9ee7d969c6dc662a) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "sound",  0x000000, 0x08000, CRC(7ec1fbc3) SHA1(48299d6530f641b18764cc49e283c347d0918a47) )
	ROM_CONTINUE(			  0x010000, 0x08000 )	/* banked stuff */
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "fix1",	0x000000,	0x010000,	CRC(0c4895b0) SHA1(f595dbe5a19edb8a06ea60105ee26b95db4a2619) )
	ROM_LOAD16_BYTE( "fix2",	0x000001,	0x010000,	CRC(07d8e387) SHA1(52f54a6a4830592784cdf643a5f255aa3db53e50) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "back-1",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "back-2",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) (diff encrypt to raiden2? ) */
	ROM_LOAD32_WORD( "obj-1",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "obj-2",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )	/* ADPCM samples */
	ROM_LOAD( "6.pcm", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) )
ROM_END

// newer PCB, with V33 CPU and COPD3 protection, but weak sound hardware. - was marked as Raiden DX New in the rom dump, but boots as Raiden 2 New version, is it switchable?
GAME( 1996, r2dx_v33, 0,		 rdx_v33,  rdx_v33, rdx_v33,  ROT270, "Seibu Kaihatsu", "Raiden II / DX (newer V33 PCB)", GAME_NOT_WORKING|GAME_NO_SOUND)

// 'V33 system type_b' - uses V33 CPU, COPX-D3 external protection rom, but still has the proper sound system
GAME( 1997, nzerotea, zeroteam,  nzerotea, nzerotea,  nzerotea,  ROT0,   "Seibu Kaihatsu", "New Zero Team", GAME_NOT_WORKING|GAME_NO_SOUND) // this uses a v33 and COPD3
