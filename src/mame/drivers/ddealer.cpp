// license:LGPL-2.1+
// copyright-holders:Angelo Salese, David Haywood
/*********************************************************************************************************

    Double Dealer (c)NMK 1991

    driver by Angelo Salese & David Haywood, based on early work by Tomasz Slanina

    Appears to be a down-grade of the nmk16 HW

    TODO:
    -When you use the "gun card" the game gives "minus" points,but points are always added,inaccurate protection?
    -Understand better the video emulation and convert it to tilemaps;
    -Coinage settings based on those listed at www.crazykong.com/dips/DoubleDealer.txt - coin/credit simulation need fixing;
    -Decap + emulate MCU, required if the random number generation is going to be accurate;

==========================================================================================================
    --

    pcb marked  GD91071

    68000P10
    YM2203C
    91071-3 (Mask ROM)
    NMK-110 8131 ( Mitsubishi M50747 MCU ?)
    NMK 901
    NMK 902
    NMK 903 x2
    82S135N ("5")
    82S129N ("6")
    xtals 16.000 MHz and  6.000 MHz
    DSW x2

    --

    Few words about protection:

    - Work RAM at $fe000 - $fffff is shared with MCU . Maybe whole $f0000-$fffff is shared ...
    - After boot, game writes random-looking data to work RAM:

    00052C: 33FC 1234 000F E086        move.w  #$1234, $fe086.l
    000534: 33FC 5678 000F E164        move.w  #$5678, $fe164.l
    00053C: 33FC 9CA3 000F E62E        move.w  #$9ca3, $fe62e.l
    000544: 33FC ABA2 000F E734        move.w  #$aba2, $fe734.l
    00054C: 33FC B891 000F E828        move.w  #$b891, $fe828.l
    000554: 33FC C760 000F E950        move.w  #$c760, $fe950.l
    00055C: 33FC D45F 000F EA7C        move.w  #$d45f, $fea7c.l
    000564: 33FC E32E 000F ED4A        move.w  #$e32e, $fed4a.l

    Some (or maybe all ?) of above enables random generator at $fe010 - $fe017

    - There's also MCU response (write/read/test) test just after these writes.
      (probably data used in the check depends on above writes). It's similar to
      jalmah.c tests, but num of responses is different, and  shared ram is
      used to communicate with MCU

    - After last check (or maybe durning tests ... no idea)
      MCU writes $4ef900000604 (jmp $604) to $fe000 and game jumps to this address.

    - code at $604  writes $20.w to $fe018 and $1.w to $fe01e.
      As result shared ram $fe000 - $fe007 is cleared.

    Also many, many other reads/writes  from/to shared mem.
    Few checks every interrupt:

    interrupt, lvl1

    000796: 007C 0700                  ori     #$700, SR
    00079A: 48E7 FFFE                  movem.l D0-D7/A0-A6, -(A7)
    00079E: 33FC 0001 000F E006        move.w  #$1, $fe006.l ; shared ram W (watchdog ?)
    0007A6: 4A79 000F E000             tst.w   $fe000.l ; shared ram R
    0007AC: 6600 0012                  bne     $7c0
    0007B0: 4A79 000F 02FE             tst.w   $f02fe.l
    0007B6: 6600 0008                  bne     $7c0
    0007BA: 4279 000F C880             clr.w   $fc880.l
+-0007C0: 6100 0236                  bsr     $9f8
| 0007C4: 4EB9 0003 0056             jsr     $30056.l
|   0007CA: 33FC 00FF 000F C880        move.w  #$ff, $fc880.l
|   0007D2: 007C 2000                  ori     #$2000, SR
|   0007D6: 4CDF 7FFF                  movem.l (A7)+, D0-D7/A0-A6
|   0007DA: 4E73                       rte
|
|
+->0009F8: 4A79 000F 02C0             tst.w   $f02c0.l
     0009FE: 6700 0072                  beq     $a72
     000A02: 4A79 000F 02F6             tst.w   $f02f6.l
     000A08: 6600 0068                  bne     $a72
     000A0C: 3439 000F E002             move.w  $fe002.l, D2 ; shared ram R
     000A12: 3602                       move.w  D2, D3
     000A14: 0242 00FF                  andi.w  #$ff, D2
     000A18: 0243 FF00                  andi.w  #$ff00, D3
     000A1C: E04B                       lsr.w   #8, D3
     000A1E: 3039 000F E000             move.w  $fe000.l, D0  ; shared ram R
     000A24: 3239 000F 0010             move.w  $f0010.l, D1
     000A2A: B041                       cmp.w   D1, D0
     000A2C: 6200 002A                  bhi     $a58
     000A30: 6600 002E                  bne     $a60
     000A34: 3039 000F 0012             move.w  $f0012.l, D0
     000A3A: B440                       cmp.w   D0, D2
     000A3C: 6200 001A                  bhi     $a58
     000A40: 6600 001E                  bne     $a60
     000A44: 3039 000F 0014             move.w  $f0014.l, D0
     000A4A: B640                       cmp.w   D0, D3
     000A4C: 6200 000A                  bhi     $a58
     000A50: 6600 000E                  bne     $a60
     000A54: 6000 001C                  bra     $a72
     000A58: 33FC 0007 000F C880        move.w  #$7, $fc880.l ; used later in the code...
     000A60: 33C0 000F 0010             move.w  D0, $f0010.l ;update mem, used in next test
     000A66: 33C2 000F 0012             move.w  D2, $f0012.l
     000A6C: 33C3 000F 0014             move.w  D3, $f0014.l
     000A72: 4E75                       rts

*********************************************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/2203intf.h"

class ddealer_state : public driver_device
{
public:
	ddealer_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_vregs(*this, "vregs"),
		m_left_fg_vram_top(*this, "left_fg_vratop"),
		m_right_fg_vram_top(*this, "right_fg_vratop"),
		m_left_fg_vram_bottom(*this, "left_fg_vrabot"),
		m_right_fg_vram_bottom(*this, "right_fg_vrabot"),
		m_back_vram(*this, "back_vram"),
		m_work_ram(*this, "work_ram"),
		m_mcu_shared_ram(*this, "mcu_shared_ram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_vregs;
	required_shared_ptr<UINT16> m_left_fg_vram_top;
	required_shared_ptr<UINT16> m_right_fg_vram_top;
	required_shared_ptr<UINT16> m_left_fg_vram_bottom;
	required_shared_ptr<UINT16> m_right_fg_vram_bottom;
	required_shared_ptr<UINT16> m_back_vram;
	required_shared_ptr<UINT16> m_work_ram;
	required_shared_ptr<UINT16> m_mcu_shared_ram;

	/* video-related */
	tilemap_t  *m_back_tilemap;
	int      m_respcount;
	int      m_flipscreen;

	/* misc */
	UINT8    m_input_pressed;
	UINT16   m_coin_input;
	DECLARE_WRITE16_MEMBER(ddealer_flipscreen_w);
	DECLARE_WRITE16_MEMBER(back_vram_w);
	DECLARE_WRITE16_MEMBER(ddealer_vregs_w);
	DECLARE_WRITE16_MEMBER(ddealer_mcu_shared_w);
	DECLARE_READ16_MEMBER(ddealer_mcu_r);
	DECLARE_DRIVER_INIT(ddealer);
	TILE_GET_INFO_MEMBER(get_back_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_ddealer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(ddealer_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(ddealer_mcu_sim);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void ddealer_draw_video_layer( UINT16* vreg_base, UINT16* top, UINT16* bottom, bitmap_ind16 &bitmap, const rectangle &cliprect, int flipy);
};



WRITE16_MEMBER(ddealer_state::ddealer_flipscreen_w)
{
	m_flipscreen = data & 0x01;
}

TILE_GET_INFO_MEMBER(ddealer_state::get_back_tile_info)
{
	int code = m_back_vram[tile_index];
	SET_TILE_INFO_MEMBER(0,
			code & 0xfff,
			code >> 12,
			0);
}

void ddealer_state::video_start()
{
	m_flipscreen = 0;
	m_back_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ddealer_state::get_back_tile_info),this), TILEMAP_SCAN_COLS, 8, 8, 64, 32);
}

void ddealer_state::ddealer_draw_video_layer( UINT16* vreg_base, UINT16* top, UINT16* bottom, bitmap_ind16 &bitmap, const rectangle &cliprect, int flipy)
{
	gfx_element *gfx = m_gfxdecode->gfx(1);

	INT16 sx, sy;
	int x,y, count;
	UINT16* src;

	sx =  ((vreg_base[0x4 / 2] & 0xff));
	sx |= ((vreg_base[0x2 / 2] & 0xff) << 8);

	sx &= 0x7ff;
	if (sx & 0x400) sx -= 0x800;

	sy =  ((vreg_base[0x8 / 2] & 0xff));
	sy |= ((vreg_base[0x6 / 2] & 0xff) << 8);

	if (!flipy)
	{
		sx -= 64; // video shift

		/* the tilemaps seems to be split into top / bottom pieces */
		count = 0;
		src = top;
		for (x = 0; x < 128; x++)
		{
			for (y = 0; y < 16; y++)
			{
				UINT16 tile = (src[count] & 0x0fff);
				UINT16 colr = (src[count] & 0xf000) >> 12;
				count++;
					gfx->transpen(bitmap,cliprect, tile, colr, 0, flipy, (x * 16) - sx, (y * 16) - sy, 15);
			}
		}
		count = 0;
		src = bottom;
		sy -= 256;
		for (x = 0; x < 128; x++)
		{
			for (y = 0; y < 16; y++)
			{
				UINT16 tile = (src[count] & 0x0fff);
				UINT16 colr = (src[count] & 0xf000) >> 12;
				count++;
					gfx->transpen(bitmap,cliprect, tile, colr, 0, flipy, (x * 16) - sx, (y * 16) - sy, 15);
			}
		}
	}
	else
	{
		sx -= 0x6d0;
		sy -= 16;

		/* the tilemaps seems to be split into top / bottom pieces */
		count = 0;
		src = top;
		for (x = 128; x > 0; x--)
		{
			for (y = 16; y > 0; y--)
			{
				UINT16 tile = (src[count] & 0x0fff);
				UINT16 colr = (src[count] & 0xf000) >> 12;
				count++;
					gfx->transpen(bitmap,cliprect, tile, colr, flipy, flipy, (x * 16) + sx, (y * 16) + sy, 15);
			}
		}
		count = 0;
		src = bottom;
		sy -= 256;
		for (x = 128; x > 0; x--)
		{
			for (y = 16; y > 0; y--)
			{
				UINT16 tile = (src[count] & 0x0fff);
				UINT16 colr = (src[count] & 0xf000) >> 12;
				count++;
					gfx->transpen(bitmap,cliprect, tile, colr, flipy, flipy, (x * 16) + sx, (y * 16) + sy, 15);
			}
		}
	}
}


UINT32 ddealer_state::screen_update_ddealer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_back_tilemap->set_scrollx(0, m_flipscreen ? -192 : -64);
	m_back_tilemap->set_flip(m_flipscreen ? TILEMAP_FLIPY | TILEMAP_FLIPX : 0);
	m_back_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* the fg tilemap handling is a little hacky right now,
	   i'm not sure if it should be a single tilemap with
	   rowscroll / linescroll, or two tilemaps which can be
	   combined, the flipscreen case makes things more
	   difficult to understand */

	if (!m_flipscreen)
	{
		if (m_vregs[0xcc / 2] & 0x80)
		{
			ddealer_draw_video_layer(&m_vregs[0x1e0 / 2], m_left_fg_vram_top, m_left_fg_vram_bottom, bitmap, cliprect, m_flipscreen);
			ddealer_draw_video_layer(&m_vregs[0xcc / 2], m_right_fg_vram_top, m_right_fg_vram_bottom, bitmap, cliprect, m_flipscreen);
		}
		else
		{
			ddealer_draw_video_layer(&m_vregs[0x1e0 / 2], m_left_fg_vram_top, m_left_fg_vram_bottom, bitmap, cliprect, m_flipscreen);
		}
	}
	else
	{
		if (m_vregs[0xcc / 2] & 0x80)
		{
			ddealer_draw_video_layer(&m_vregs[0xcc / 2], m_left_fg_vram_top, m_left_fg_vram_bottom, bitmap, cliprect, m_flipscreen);
			ddealer_draw_video_layer(&m_vregs[0x1e0 / 2], m_right_fg_vram_top, m_right_fg_vram_bottom, bitmap, cliprect, m_flipscreen);
		}
		else
		{
			ddealer_draw_video_layer(&m_vregs[0x1e0 / 2], m_left_fg_vram_top, m_left_fg_vram_bottom, bitmap, cliprect, m_flipscreen);
		}

	}

	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(ddealer_state::ddealer_mcu_sim)
{
	/*coin/credit simulation*/
	/*$fe002 is used,might be for multiple coins for one credit settings.*/
	m_coin_input = (~(ioport("IN0")->read()));

	if (m_coin_input & 0x01)//coin 1
	{
		if((m_input_pressed & 0x01) == 0)
			m_mcu_shared_ram[0x000 / 2]++;
		m_input_pressed = (m_input_pressed & 0xfe) | 1;
	}
	else
		m_input_pressed = (m_input_pressed & 0xfe);

	if (m_coin_input & 0x02)//coin 2
	{
		if ((m_input_pressed & 0x02) == 0)
			m_mcu_shared_ram[0x000 / 2]++;
		m_input_pressed = (m_input_pressed & 0xfd) | 2;
	}
	else
		m_input_pressed = (m_input_pressed & 0xfd);

	if (m_coin_input & 0x04)//service 1
	{
		if ((m_input_pressed & 0x04) == 0)
			m_mcu_shared_ram[0x000 / 2]++;
		m_input_pressed = (m_input_pressed & 0xfb) | 4;
	}
	else
		m_input_pressed = (m_input_pressed & 0xfb);

	/*0x104/2 is some sort of "start-lock",i.e. used on the girl selection.
	  Without it,the game "steals" one credit if you press the start button on that.*/
	if (m_mcu_shared_ram[0x000 / 2] > 0 && m_work_ram[0x104 / 2] & 1)
	{
		if (m_coin_input & 0x08)//start 1
		{
			if ((m_input_pressed & 0x08) == 0 && (~(m_work_ram[0x100 / 2] & 1)))
				m_mcu_shared_ram[0x000 / 2]--;
			m_input_pressed = (m_input_pressed & 0xf7) | 8;
		}
		else
			m_input_pressed = (m_input_pressed & 0xf7);

		if (m_coin_input & 0x10)//start 2
		{
			if((m_input_pressed & 0x10) == 0 && (~(m_work_ram[0x100 / 2] & 2)))
				m_mcu_shared_ram[0x000 / 2]--;
			m_input_pressed = (m_input_pressed & 0xef) | 0x10;
		}
		else
			m_input_pressed = (m_input_pressed & 0xef);
	}

	/*random number generators,controls order of cards*/
	m_mcu_shared_ram[0x10 / 2] = machine().rand() & 0xffff;
	m_mcu_shared_ram[0x12 / 2] = machine().rand() & 0xffff;
	m_mcu_shared_ram[0x14 / 2] = machine().rand() & 0xffff;
	m_mcu_shared_ram[0x16 / 2] = machine().rand() & 0xffff;
}



WRITE16_MEMBER(ddealer_state::back_vram_w)
{
	COMBINE_DATA(&m_back_vram[offset]);
	m_back_tilemap->mark_tile_dirty(offset);
}


WRITE16_MEMBER(ddealer_state::ddealer_vregs_w)
{
	COMBINE_DATA(&m_vregs[offset]);
}

/******************************************************************************************************

Protection handling,identical to Hacha Mecha Fighter / Thunder Dragon with different vectors.

******************************************************************************************************/

#define PROT_JSR(_offs_,_protvalue_,_pc_) \
	if(m_mcu_shared_ram[(_offs_)/2] == _protvalue_) \
	{ \
		m_mcu_shared_ram[(_offs_)/2] = 0xffff;  /*(MCU job done)*/ \
		m_mcu_shared_ram[(_offs_+2-0x10)/2] = 0x4ef9;/*JMP*/\
		m_mcu_shared_ram[(_offs_+4-0x10)/2] = 0x0000;/*HI-DWORD*/\
		m_mcu_shared_ram[(_offs_+6-0x10)/2] = _pc_;  /*LO-DWORD*/\
	}
#define PROT_INPUT(_offs_,_protvalue_,_protinput_,_input_) \
	if(m_mcu_shared_ram[_offs_] == _protvalue_) \
	{\
		m_mcu_shared_ram[_protinput_] = ((_input_ & 0xffff0000)>>16);\
		m_mcu_shared_ram[_protinput_+1] = (_input_ & 0x0000ffff);\
	}

WRITE16_MEMBER(ddealer_state::ddealer_mcu_shared_w)
{
	COMBINE_DATA(&m_mcu_shared_ram[offset]);

	switch(offset)
	{
		case 0x086/2: PROT_INPUT(0x086/2,0x1234,0x100/2,0x80000); break;
		case 0x164/2: PROT_INPUT(0x164/2,0x5678,0x104/2,0x80002); break;
		case 0x62e/2: PROT_INPUT(0x62e/2,0x9ca3,0x108/2,0x80008); break;
		case 0x734/2: PROT_INPUT(0x734/2,0xaba2,0x10c/2,0x8000a); break;
/*These enables something for sure,maybe the random number generator?*/
	//00054C: 33FC B891 000F E828        move.w  #$b891, $fe828.l
	//000554: 33FC C760 000F E950        move.w  #$c760, $fe950.l
	//00055C: 33FC D45F 000F EA7C        move.w  #$d45f, $fea7c.l
	//000564: 33FC E32E 000F ED4A        move.w  #$e32e, $fed4a.l

		case 0x40e/2: PROT_JSR(0x40e,0x8011,0x6992); break;//score
		case 0x41e/2: break;//unused
		case 0x42e/2: PROT_JSR(0x42e,0x8007,0x6004); break;//cards on playfield/hand (ram side)
		case 0x43e/2: PROT_JSR(0x43e,0x801d,0x6176); break;//second player sub-routine
		case 0x44e/2: PROT_JSR(0x44e,0x8028,0x6932); break;//"gun card" logic
		case 0x45e/2: PROT_JSR(0x45e,0x803e,0x6f90); break;//card delete
		case 0x46e/2: PROT_JSR(0x46e,0x8033,0x93c2); break;//card movements
		case 0x47e/2: PROT_JSR(0x47e,0x8026,0x67a0); break;//cards on playfield (vram side)
		case 0x48e/2: PROT_JSR(0x48e,0x8012,0x6824); break;//cards on hand (vram side)
		case 0x49e/2: PROT_JSR(0x49e,0x8004,0x9696); break;//write to text layer
		case 0x4ae/2: PROT_JSR(0x4ae,0x8035,0x95fe); break;//write to scroll layer
		case 0x4be/2: PROT_JSR(0x4be,0x8009,0x9634); break;//show girls sub-routine
		case 0x4ce/2: PROT_JSR(0x4ce,0x802a,0x9656); break;
		case 0x4de/2: PROT_JSR(0x4de,0x803b,0x96c2); break;
		case 0x4ee/2: PROT_JSR(0x4ee,0x800c,0x5ca4); break;//palette ram buffer
		case 0x4fe/2: PROT_JSR(0x4fe,0x8018,0x9818); break;
		/*Start-up vector,I think that only the first ram address can be written by the main CPU,or it is a whole sequence.*/
		case 0x000/2:
			if (m_mcu_shared_ram[0x000 / 2] == 0x60fe)
			{
				m_mcu_shared_ram[0x000 / 2] = 0x0000;//coin counter
				m_mcu_shared_ram[0x002 / 2] = 0x0000;//coin counter "decimal point"
				m_mcu_shared_ram[0x004 / 2] = 0x4ef9;
			}
			break;
		case 0x002/2:
		case 0x004/2:
			if (m_mcu_shared_ram[0x002 / 2] == 0x0000 && m_mcu_shared_ram[0x004 / 2] == 0x0214)
				m_mcu_shared_ram[0x004 / 2] = 0x4ef9;
			break;
		case 0x008/2:
			if (m_mcu_shared_ram[0x008 / 2] == 0x000f)
				m_mcu_shared_ram[0x008 / 2] = 0x0604;
			break;
		case 0x00c/2:
			if (m_mcu_shared_ram[0x00c / 2] == 0x000f)
				m_mcu_shared_ram[0x00c / 2] = 0x0000;
			break;
	}
}

static ADDRESS_MAP_START( ddealer, AS_PROGRAM, 16, ddealer_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("IN0")
	AM_RANGE(0x080002, 0x080003) AM_READ_PORT("IN1")
	AM_RANGE(0x080008, 0x080009) AM_READ_PORT("DSW1")
	AM_RANGE(0x08000a, 0x08000b) AM_READ_PORT("UNK")
	AM_RANGE(0x084000, 0x084003) AM_DEVWRITE8("ymsnd", ym2203_device, write, 0x00ff) // ym ?
	AM_RANGE(0x088000, 0x0887ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x08c000, 0x08cfff) AM_RAM_WRITE(ddealer_vregs_w) AM_SHARE("vregs") // palette ram

	/* this might actually be 1 tilemap with some funky rowscroll / columnscroll enabled, I'm not sure */
	AM_RANGE(0x090000, 0x090fff) AM_RAM AM_SHARE("left_fg_vratop")
	AM_RANGE(0x091000, 0x091fff) AM_RAM AM_SHARE("right_fg_vratop")
	AM_RANGE(0x092000, 0x092fff) AM_RAM AM_SHARE("left_fg_vrabot")
	AM_RANGE(0x093000, 0x093fff) AM_RAM AM_SHARE("right_fg_vrabot")
	//AM_RANGE(0x094000, 0x094001) AM_NOP // always 0?
	AM_RANGE(0x098000, 0x098001) AM_WRITE(ddealer_flipscreen_w)
	AM_RANGE(0x09c000, 0x09cfff) AM_RAM_WRITE(back_vram_w) AM_SHARE("back_vram") // bg tilemap
	AM_RANGE(0x0f0000, 0x0fdfff) AM_RAM AM_SHARE("work_ram")
	AM_RANGE(0x0fe000, 0x0fefff) AM_RAM_WRITE(ddealer_mcu_shared_w) AM_SHARE("mcu_shared_ram")
	AM_RANGE(0x0ff000, 0x0fffff) AM_RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( ddealer )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) //used,"test" in service mode,unknown purpose
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	/*bits 0-7 are almost surely to be coinage.*/
	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW1:8" ) /* Listed as "Always Off" */
	PORT_DIPUNUSED_DIPLOC( 0x0002, IP_ACTIVE_LOW, "SW1:7" ) /* Listed as "Always Off" */
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:6,5,4")
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) ) /* Not listed in "dips" text, but current effect is FREE PLAY. Is this correct? */
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Language ) )     PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0400, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:5,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Lady Stripping" )        PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW2:2" ) /* Listed as "Always Off" */
	PORT_DIPUNUSED_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:1" ) /* Listed as "Always Off" */

	PORT_START("UNK")
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
			16*32+0*4, 16*32+1*4, 16*32+2*4, 16*32+3*4, 16*32+4*4, 16*32+5*4, 16*32+6*4, 16*32+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	32*32
};

static GFXDECODE_START( ddealer )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 0x100, 16 )
GFXDECODE_END

void ddealer_state::machine_start()
{
	save_item(NAME(m_respcount));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_input_pressed));
	save_item(NAME(m_coin_input));
}

void ddealer_state::machine_reset()
{
	m_respcount = 0;
	m_flipscreen = 0;
	m_input_pressed = 0;
	m_coin_input = 0;
}

INTERRUPT_GEN_MEMBER(ddealer_state::ddealer_interrupt)
{
	device.execute().set_input_line(4, HOLD_LINE);
}

static MACHINE_CONFIG_START( ddealer, ddealer_state )

	MCFG_CPU_ADD("maincpu" , M68000, XTAL_16MHz/2) /* 8MHz */
	MCFG_CPU_PROGRAM_MAP(ddealer)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", ddealer_state,  ddealer_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(ddealer_state, irq1_line_hold,  90)//guess,controls music tempo,112 is way too fast

	// M50747 or NMK-110 8131 MCU


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ddealer)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(ddealer_state, screen_update_ddealer)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x400)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("coinsim", ddealer_state, ddealer_mcu_sim, attotime::from_hz(10000))

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_6MHz / 8) /* 7.5KHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_CONFIG_END



READ16_MEMBER(ddealer_state::ddealer_mcu_r)
{
	static const int resp[] =
	{
		0x00, /* performs a clr.l when doing the ram test, triggering a read here */
		0x93, 0xc7, 0x00, 0x8000,
		0x2d, 0x6d, 0x00, 0x8000,
		0x99, 0xc7, 0x00, 0x8000,
		0x2a, 0x6a, 0x00, 0x8000,
		0x8e, 0xc7, 0x00, 0x8000,
	-1};

	int res;

	res = resp[m_respcount++];
	if (resp[m_respcount] < 0)
			m_respcount = 0;

	return res;
}

DRIVER_INIT_MEMBER(ddealer_state,ddealer)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xfe01c, 0xfe01d, read16_delegate(FUNC(ddealer_state::ddealer_mcu_r), this));
}

ROM_START( ddealer )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "1.ic6",  0x00001, 0x20000, CRC(ce0dff50) SHA1(2d7a03f6b9609aea7511a4dc49560a901b0b9f19) )
	ROM_LOAD16_BYTE( "2.ic28", 0x00000, 0x20000, CRC(f00c346f) SHA1(bd73efb19d5f9efc88210d92a82a3f4595b41097) )

	ROM_REGION( 0x40000, "mcu", 0 ) /* M50747? MCU Code */
	ROM_LOAD( "mcu", 0x0000, 0x1000, NO_DUMP ) // might be NMK-110 8131 chip

	ROM_REGION( 0x20000, "gfx1", 0 ) /* BG0 */
	ROM_LOAD( "4.ic65", 0x00000, 0x20000, CRC(4939ff1b) SHA1(af2f2feeef5520d775731a58cbfc8fcc913b7348) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* BG1 */
	ROM_LOAD( "3.ic64", 0x00000, 0x80000, CRC(660e367c) SHA1(54827a8998c58c578c594126d5efc18a92363eaa))

	ROM_REGION( 0x200, "user1", 0 ) /* Proms */
	ROM_LOAD( "5.ic67", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "6.ic86", 0x100, 0x100, NO_DUMP )
ROM_END

GAME( 1991, ddealer,  0, ddealer, ddealer, ddealer_state, ddealer,  ROT0, "NMK", "Double Dealer", MACHINE_SUPPORTS_SAVE )
