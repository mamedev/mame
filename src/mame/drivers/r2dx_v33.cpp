// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, David Haywood, ???
/*

Raiden 2 / DX V33 Version

Raiden 2 / DX checks if there's the string "RAIDEN" at start-up inside the eeprom, otherwise it dies.
Then it puts settings at 0x9e08 and 0x9e0a (bp 91acb)

    the 333 ROM is a 0x10000 byte table (bytes values?)
    followed by a 0x400 bytes (word values)?
    the remaining space is 0xff

    Notes:

    Zero Team 2000
     - EEPROM contains high scores, but they don't get restored? (original bug?)

    New Zero Team
     - 2 Player only? Service mode only shows 2 Players and I don't see a switch
     - Stages 3 and 1 are swapped, this is correct.

    Raiden 2 New / Raiden DX
     - This is a 2-in-1 board.  The current game to boot is stored in the EEPROM.
       If you wish to change game then on powerup you must hold down all 4 (P1) joystick
       directions simultaneously along with either button 1 or button 2.
       Obviously this is impossible with a real joystck!

       It is also impossible in MAME unless you enable -joystick_contradictory
       to disable MAME from preventing opposing joystick directions being pressed
       and you'll most likely have to remap some keys too because 5 buttons at the
       same time is beyond the limits of most keyboards.

       We currently use a default EEPROM for each game as this is most likely how
       it shipped, the game changing is an undocumented secret.

     - The sound is awful, this is just how it is.

     - In Raiden 2 New stages 5 and 1 are swapped, and the intro is missing, this is
       correct.

*/

/* Rom structure notes

 Raiden 2 New/DX (hardware can bank upper/lower half of rom to switch fixed areas)

 000000-02ffff : 0xff fill, inaccessible by hardware
 030000-0fffff : 'Fixed' ROM data for Raiden 2
 100000-1fffff : Banked ROM data for Raiden 2 (16x 0x10000 banks)

 200000-22ffff : 0xff fill, inaccessible by hardware
 230000-2fffff : 'Fixed' ROM data for Raiden DX
 300000-3fffff : Banked ROM data for Raiden DX (16x 0x10000 banks)

 New Zero Team /Zero

 000000-01ffff : 0xff fill, inaccessible by hardware?
 020000-0fffff : Fixed ROM data for Zero Team
 (no banking)

*/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "includes/raiden2.h"


class r2dx_v33_state : public raiden2_state
{
public:
	r2dx_v33_state(const machine_config &mconfig, device_type type, std::string tag)
		: raiden2_state(mconfig, type, tag),
		m_eeprom(*this, "eeprom"),
		m_math(*this, "math"),
		m_r2dxbank(0),
		m_r2dxgameselect(0)
	{ }

	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_region_ptr<UINT8> m_math;

	DECLARE_WRITE16_MEMBER(r2dx_angle_w);
	DECLARE_WRITE16_MEMBER(r2dx_dx_w);
	DECLARE_WRITE16_MEMBER(r2dx_dy_w);
	DECLARE_WRITE16_MEMBER(r2dx_sdistl_w);
	DECLARE_WRITE16_MEMBER(r2dx_sdisth_w);
	DECLARE_READ16_MEMBER(r2dx_angle_r);
	DECLARE_READ16_MEMBER(r2dx_dist_r);
	DECLARE_READ16_MEMBER(r2dx_sin_r);
	DECLARE_READ16_MEMBER(r2dx_cos_r);

	DECLARE_WRITE16_MEMBER(tile_bank_w);
	DECLARE_READ16_MEMBER(rdx_v33_unknown_r);
	DECLARE_WRITE16_MEMBER(mcu_xval_w);
	DECLARE_WRITE16_MEMBER(mcu_yval_w);
	DECLARE_WRITE16_MEMBER(mcu_table_w);
	DECLARE_WRITE16_MEMBER(mcu_table2_w);
	DECLARE_WRITE16_MEMBER(mcu_prog_w);
	DECLARE_WRITE16_MEMBER(mcu_prog_w2);
	DECLARE_WRITE16_MEMBER(mcu_prog_offs_w);
	DECLARE_WRITE16_MEMBER(rdx_v33_eeprom_w);
	DECLARE_WRITE16_MEMBER(zerotm2k_eeprom_w);
	DECLARE_WRITE16_MEMBER(r2dx_rom_bank_w);
	DECLARE_DRIVER_INIT(rdx_v33);
	DECLARE_DRIVER_INIT(nzerotea);
	DECLARE_DRIVER_INIT(zerotm2k);

	DECLARE_WRITE16_MEMBER(r2dx_tilemapdma_w);
	DECLARE_WRITE16_MEMBER(r2dx_paldma_w);
	DECLARE_READ16_MEMBER(r2dx_debug_r);

	void r2dx_setbanking(void);

	DECLARE_MACHINE_RESET(r2dx_v33);
	DECLARE_MACHINE_RESET(nzeroteam);

	int m_r2dxbank;
	int m_r2dxgameselect;
	INT16 m_r2dx_angle;

	UINT16 r2dx_i_dx, r2dx_i_dy, r2dx_i_angle;
	UINT32 r2dx_i_sdist;

	INTERRUPT_GEN_MEMBER(rdx_v33_interrupt);

protected:
	virtual void machine_start() override;
};

void r2dx_v33_state::machine_start()
{
	raiden2_state::machine_start();

	save_item(NAME(m_r2dxbank));
	save_item(NAME(m_r2dxgameselect));
	save_item(NAME(m_r2dx_angle));
	save_item(NAME(r2dx_i_dx));
	save_item(NAME(r2dx_i_dy));
	save_item(NAME(r2dx_i_angle));
	save_item(NAME(r2dx_i_sdist));
}

WRITE16_MEMBER(r2dx_v33_state::tile_bank_w)
{
	if(ACCESSING_BITS_0_7) {
		int new_bank;
		new_bank = ((data & 0x10)>>4);
		if(new_bank != bg_bank) {
			bg_bank = new_bank;
			background_layer->mark_all_dirty();
		}

		new_bank = 2 + ((data & 0x20)>>5);
		if(new_bank != mid_bank) {
			mid_bank = new_bank;
			midground_layer->mark_all_dirty();
		}

		new_bank = 4 | (data & 3);
		if(new_bank != fg_bank) {
			fg_bank = new_bank;
			foreground_layer->mark_all_dirty();
		}
	}
}

void r2dx_v33_state::r2dx_setbanking(void)
{
	membank("bank1")->set_entry(m_r2dxgameselect*0x20 + m_r2dxbank + 16);
	membank("bank3")->set_entry(m_r2dxgameselect);
}

WRITE16_MEMBER(r2dx_v33_state::rdx_v33_eeprom_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_eeprom->clk_write((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->di_write((data & 0x20) >> 5);
		m_eeprom->cs_write((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);

		// 0x40 - coin counter 1?
		// 0x80 - coin counter 2?

		// 0x04 is active in Raiden DX mode, it could be part of the rom bank (which half of the rom to use) or the FG tile bank (or both?)
		// the bit gets set if it reads RAIDENDX from the EEPROM
		m_r2dxgameselect = (data & 0x04) >> 2;

		tx_bank = m_r2dxgameselect;
		text_layer->mark_all_dirty();

		r2dx_setbanking();

		membank("okibank")->set_entry(data&3);

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
//  printf("mcu_table2_w %04x %04x\n", data, mem_mask);

	mcu_data[offset+4] = data;

	//popmessage("%04x %04x %04x %04x | %04x %04x %04x %04x",mcu_data[0/2],mcu_data[2/2],mcu_data[4/2],mcu_data[6/2],mcu_data[8/2],mcu_data[0xa/2],mcu_data[0xc/2],mcu_data[0xe/2]);
}

WRITE16_MEMBER(r2dx_v33_state::r2dx_rom_bank_w)
{
	//printf("rom bank %04x %04x\n", data, mem_mask);
	m_r2dxbank = data & 0xf;
	r2dx_setbanking();

}

WRITE16_MEMBER(r2dx_v33_state::r2dx_angle_w)
{
	COMBINE_DATA(&r2dx_i_angle);
}

WRITE16_MEMBER(r2dx_v33_state::r2dx_dx_w)
{
	COMBINE_DATA(&r2dx_i_dx);
}

WRITE16_MEMBER(r2dx_v33_state::r2dx_dy_w)
{
	COMBINE_DATA(&r2dx_i_dy);
}

READ16_MEMBER(r2dx_v33_state::r2dx_angle_r)
{
	return m_math[((r2dx_i_dy & 0xff) << 8) | (r2dx_i_dx & 0xff)];
}

READ16_MEMBER(r2dx_v33_state::r2dx_dist_r)
{
	return sqrt(double(r2dx_i_sdist));
}

READ16_MEMBER(r2dx_v33_state::r2dx_sin_r)
{
	int off = 65536 + (r2dx_i_angle & 0xff)*4;
	return (m_math[off+0]) | (m_math[off+1] << 8);
}

READ16_MEMBER(r2dx_v33_state::r2dx_cos_r)
{
	int off = 65536 + (r2dx_i_angle & 0xff)*4;
	return (m_math[off+2]) | (m_math[off+3] << 8);
}

WRITE16_MEMBER(r2dx_v33_state::r2dx_sdistl_w)
{
	r2dx_i_sdist = (r2dx_i_sdist & (0xffff0000 | UINT16(~mem_mask))) | (data & mem_mask);
}

WRITE16_MEMBER(r2dx_v33_state::r2dx_sdisth_w)
{
	r2dx_i_sdist = (r2dx_i_sdist & (0x0000ffff | (UINT16(~mem_mask)) << 16)) | ((data & mem_mask) << 16);
}

// these DMA operations seem to use hardcoded addresses on this hardware
WRITE16_MEMBER(r2dx_v33_state::r2dx_tilemapdma_w)
{
	int src = 0xd000;

	for (int i = 0; i < 0x2800 / 2; i++)
	{
		UINT16 tileval = space.read_word(src);
		src += 2;
		m_videoram_private_w(space, i, tileval, 0xffff);
	}
}

WRITE16_MEMBER(r2dx_v33_state::r2dx_paldma_w)
{
	int src = 0x1f000;

	for (int i = 0; i < 0x1000 / 2; i++)
	{
		UINT16 palval = space.read_word(src);
		src += 2;
		m_palette->set_pen_color(i, pal5bit(palval >> 0), pal5bit(palval >> 5), pal5bit(palval >> 10));
	}
}

READ16_MEMBER(r2dx_v33_state::r2dx_debug_r)
{
	// read once on startup, needed for player collisions to work
	return 0xffff;
}

static ADDRESS_MAP_START( rdx_v33_map, AS_PROGRAM, 16, r2dx_v33_state )
	AM_RANGE(0x00000, 0x003ff) AM_RAM // vectors copied here

	AM_RANGE(0x00400, 0x00401) AM_WRITE(r2dx_tilemapdma_w) // tilemaps to private buffer
	AM_RANGE(0x00402, 0x00403) AM_WRITE(r2dx_paldma_w)  // palettes to private buffer


	AM_RANGE(0x00404, 0x00405) AM_WRITE(r2dx_rom_bank_w)
	AM_RANGE(0x00406, 0x00407) AM_WRITE(tile_bank_w)

	AM_RANGE(0x00420, 0x00421) AM_WRITE(r2dx_dx_w)
	AM_RANGE(0x00422, 0x00423) AM_WRITE(r2dx_dy_w)
	AM_RANGE(0x00424, 0x00425) AM_WRITE(r2dx_sdistl_w)
	AM_RANGE(0x00426, 0x00427) AM_WRITE(r2dx_sdisth_w)
	AM_RANGE(0x00428, 0x00429) AM_WRITE(r2dx_angle_w)

	AM_RANGE(0x00430, 0x00431) AM_READ(r2dx_angle_r)
	AM_RANGE(0x00432, 0x00433) AM_READ(r2dx_dist_r)
	AM_RANGE(0x00434, 0x00435) AM_READ(r2dx_sin_r)
	AM_RANGE(0x00436, 0x00437) AM_READ(r2dx_cos_r)

	AM_RANGE(0x00600, 0x0064f) AM_DEVREADWRITE("crtc", seibu_crtc_device, read, write)
//  AM_RANGE(0x00650, 0x0068f) AM_RAM //???

	AM_RANGE(0x0068e, 0x0068f) AM_WRITENOP // maybe a watchdog?
	AM_RANGE(0x006b0, 0x006b1) AM_WRITE(mcu_prog_w) // could be encryption key uploads just like raiden2.c ?
	AM_RANGE(0x006b2, 0x006b3) AM_WRITE(mcu_prog_w2)
//  AM_RANGE(0x006b4, 0x006b5) AM_WRITENOP
//  AM_RANGE(0x006b6, 0x006b7) AM_WRITENOP
	AM_RANGE(0x006bc, 0x006bd) AM_WRITE(mcu_prog_offs_w)
//  AM_RANGE(0x006be, 0x006bf) AM_WRITENOP

	// sprite protection not 100% verified as the same
	AM_RANGE(0x006c0, 0x006c1) AM_READWRITE(sprite_prot_off_r, sprite_prot_off_w)
	AM_RANGE(0x006c2, 0x006c3) AM_READWRITE(sprite_prot_src_seg_r, sprite_prot_src_seg_w)
	AM_RANGE(0x006c6, 0x006c7) AM_WRITE(sprite_prot_dst1_w)

	AM_RANGE(0x006d8, 0x006d9) AM_WRITE(sprite_prot_x_w)
	AM_RANGE(0x006da, 0x006db) AM_WRITE(sprite_prot_y_w)
	AM_RANGE(0x006dc, 0x006dd) AM_READWRITE(sprite_prot_maxx_r, sprite_prot_maxx_w)
	AM_RANGE(0x006de, 0x006df) AM_WRITE(sprite_prot_src_w)


	AM_RANGE(0x00700, 0x00701) AM_WRITE(rdx_v33_eeprom_w)
	AM_RANGE(0x00740, 0x00741) AM_READ(r2dx_debug_r)
	AM_RANGE(0x00744, 0x00745) AM_READ_PORT("INPUT")
	AM_RANGE(0x0074c, 0x0074d) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x00762, 0x00763) AM_READ(sprite_prot_dst1_r)

	AM_RANGE(0x00780, 0x00781) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff) // single OKI chip on this version

	AM_RANGE(0x00800, 0x00fff) AM_RAM // copies eeprom here?
	AM_RANGE(0x01000, 0x0bfff) AM_RAM

	AM_RANGE(0x0c000, 0x0c7ff) AM_RAM AM_SHARE("sprites")
	AM_RANGE(0x0c800, 0x0cfff) AM_RAM
	AM_RANGE(0x0d000, 0x0d7ff) AM_RAM //_WRITE(raiden2_background_w) AM_SHARE("back_data")
	AM_RANGE(0x0d800, 0x0dfff) AM_RAM //_WRITE(raiden2_foreground_w) AM_SHARE("fore_data")
	AM_RANGE(0x0e000, 0x0e7ff) AM_RAM //_WRITE(raiden2_midground_w)  AM_SHARE("mid_data")
	AM_RANGE(0x0e800, 0x0f7ff) AM_RAM //_WRITE(raiden2_text_w) AM_SHARE("text_data")
	AM_RANGE(0x0f800, 0x0ffff) AM_RAM /* Stack area */
	AM_RANGE(0x10000, 0x1efff) AM_RAM
	AM_RANGE(0x1f000, 0x1ffff) AM_RAM //_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")

	AM_RANGE(0x20000, 0x2ffff) AM_ROMBANK("bank1") AM_WRITENOP
	AM_RANGE(0x30000, 0xfffff) AM_ROMBANK("bank3") AM_WRITENOP
ADDRESS_MAP_END


static ADDRESS_MAP_START( nzeroteam_base_map, AS_PROGRAM, 16, r2dx_v33_state )
	AM_RANGE(0x00000, 0x003ff) AM_RAM //stack area

	AM_RANGE(0x00400, 0x00401) AM_WRITE(r2dx_tilemapdma_w) // tilemaps to private buffer
	AM_RANGE(0x00402, 0x00403) AM_WRITE(r2dx_paldma_w)  // palettes to private buffer
	// 0x404 is bank on r2dx, this doesn't need it
	// AM_RANGE(0x00406, 0x00407) AM_WRITE(tile_bank_w) // not the same?

	AM_RANGE(0x00406, 0x00407) AM_NOP // always 6022, supposed to be the tile bank but ignroes the actual value???

	AM_RANGE(0x00420, 0x00421) AM_WRITE(r2dx_dx_w)
	AM_RANGE(0x00422, 0x00423) AM_WRITE(r2dx_dy_w)
	AM_RANGE(0x00424, 0x00425) AM_WRITE(r2dx_sdistl_w)
	AM_RANGE(0x00426, 0x00427) AM_WRITE(r2dx_sdisth_w)
	AM_RANGE(0x00428, 0x00429) AM_WRITE(r2dx_angle_w)

	AM_RANGE(0x00430, 0x00431) AM_READ(r2dx_angle_r)
	AM_RANGE(0x00432, 0x00433) AM_READ(r2dx_dist_r)
	AM_RANGE(0x00434, 0x00435) AM_READ(r2dx_sin_r)
	AM_RANGE(0x00436, 0x00437) AM_READ(r2dx_cos_r)

	AM_RANGE(0x00600, 0x0064f) AM_DEVREADWRITE("crtc", seibu_crtc_device, read, write)

	AM_RANGE(0x0068e, 0x0068f) AM_WRITENOP // synch for the MCU?
	AM_RANGE(0x006b0, 0x006b1) AM_WRITE(mcu_prog_w)
	AM_RANGE(0x006b2, 0x006b3) AM_WRITE(mcu_prog_w2)
//  AM_RANGE(0x006b4, 0x006b5) AM_WRITENOP
//  AM_RANGE(0x006b6, 0x006b7) AM_WRITENOP
	AM_RANGE(0x006bc, 0x006bd) AM_WRITE(mcu_prog_offs_w)
//  AM_RANGE(0x006d8, 0x006d9) AM_WRITE(bbbbll_w) // scroll?
//  AM_RANGE(0x006dc, 0x006dd) AM_READ(nzerotea_unknown_r)
//  AM_RANGE(0x006de, 0x006df) AM_WRITE(mcu_unkaa_w) // mcu command related?
	//AM_RANGE(0x00700, 0x00701) AM_WRITE(rdx_v33_eeprom_w)

//  AM_RANGE(0x00762, 0x00763) AM_READ(nzerotea_unknown_r)

	AM_RANGE(0x00780, 0x0079f) AM_READWRITE(raiden2_sound_comms_r,raiden2_sound_comms_w)

	AM_RANGE(0x00800, 0x00fff) AM_RAM
	AM_RANGE(0x01000, 0x0bfff) AM_RAM

	AM_RANGE(0x0c000, 0x0c7ff) AM_RAM AM_SHARE("sprites")
	AM_RANGE(0x0c800, 0x0cfff) AM_RAM
	AM_RANGE(0x0d000, 0x0d7ff) AM_RAM //_WRITE(raiden2_background_w) AM_SHARE("back_data")
	AM_RANGE(0x0d800, 0x0dfff) AM_RAM //_WRITE(raiden2_foreground_w) AM_SHARE("fore_data")
	AM_RANGE(0x0e000, 0x0e7ff) AM_RAM //_WRITE(raiden2_midground_w)  AM_SHARE("mid_data")
	AM_RANGE(0x0e800, 0x0f7ff) AM_RAM //_WRITE(raiden2_text_w) AM_SHARE("text_data")
	AM_RANGE(0x0f800, 0x0ffff) AM_RAM /* Stack area */
	AM_RANGE(0x10000, 0x1efff) AM_RAM
	AM_RANGE(0x1f000, 0x1ffff) AM_RAM //_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")

	AM_RANGE(0x20000, 0xfffff) AM_ROM AM_REGION("maincpu", 0x20000 )
ADDRESS_MAP_END

static ADDRESS_MAP_START( nzerotea_map, AS_PROGRAM, 16, r2dx_v33_state )
	AM_IMPORT_FROM( nzeroteam_base_map )
	AM_RANGE(0x00740, 0x00741) AM_READ_PORT("DSW")
	AM_RANGE(0x00744, 0x00745) AM_READ_PORT("INPUT")
	AM_RANGE(0x0074c, 0x0074d) AM_READ_PORT("SYSTEM")
ADDRESS_MAP_END

WRITE16_MEMBER(r2dx_v33_state::zerotm2k_eeprom_w)
{
//  printf("zerotm2k_eeprom_w %04x %04x\n", data, mem_mask);

	m_eeprom->clk_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->di_write((data & 0x04) >> 2);
	m_eeprom->cs_write((data & 0x01) ? ASSERT_LINE : CLEAR_LINE);
}

static ADDRESS_MAP_START( zerotm2k_map, AS_PROGRAM, 16, r2dx_v33_state )
	AM_IMPORT_FROM( nzeroteam_base_map )
	AM_RANGE(0x00740, 0x00741) AM_READ_PORT("P3_P4")
	AM_RANGE(0x00744, 0x00745) AM_READ_PORT("INPUT")
	AM_RANGE(0x0074c, 0x0074d) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x00748, 0x00749) AM_WRITE(zerotm2k_eeprom_w)
ADDRESS_MAP_END



INTERRUPT_GEN_MEMBER(r2dx_v33_state::rdx_v33_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0xc0/4);   /* VBL */
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
	GFXDECODE_ENTRY( "gfx1", 0x00000, rdx_v33_charlayout,   0x700, 128 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, rdx_v33_tilelayout,   0x400, 128 )
	GFXDECODE_ENTRY( "gfx3", 0x00000, rdx_v33_spritelayout, 0x000, 4096 )
GFXDECODE_END

static INPUT_PORTS_START( rdx_v33 )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_DIPLOC(  0x0040, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
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
	SEIBU_COIN_INPUTS_INVERT    /* coin inputs read through sound cpu */

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

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

	PORT_START("DSW") // taken from zeroteam, except last dip is service mode
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:!4,!5,!6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Starting Coin" ) PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "X 2" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(      0x0c00, "2" )
	PORT_DIPSETTING(      0x0800, "4" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(      0x3000, "1000000" )
	PORT_DIPSETTING(      0x2000, "2000000" )
	PORT_DIPSETTING(      0x1000, "Every 1000000" )
	PORT_DIPSETTING(      0x0000, "No Extend" )
	PORT_DIPNAME( 0x4000, 0x4000, "Demo Sound" ) PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW2:!8" )
INPUT_PORTS_END

static INPUT_PORTS_START( zerotm2k )
	SEIBU_COIN_INPUTS_INVERT    /* coin inputs read through sound cpu */

	PORT_MODIFY("COIN")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(4)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0020, IP_ACTIVE_LOW )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUT")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START("P3_P4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


MACHINE_RESET_MEMBER(r2dx_v33_state,r2dx_v33)
{
	common_reset();
}

MACHINE_RESET_MEMBER(r2dx_v33_state,nzeroteam)
{
	common_reset();

	bg_bank = 0;
	fg_bank = 2;
	mid_bank = 1;
}

static ADDRESS_MAP_START( r2dx_oki_map, AS_0, 8, r2dx_v33_state )
	AM_RANGE(0x00000, 0x3ffff) AM_ROMBANK("okibank")
ADDRESS_MAP_END

static MACHINE_CONFIG_START( rdx_v33, r2dx_v33_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V33, 32000000/2 ) // ?
	MCFG_CPU_PROGRAM_MAP(rdx_v33_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", r2dx_v33_state,  rdx_v33_interrupt)

	MCFG_MACHINE_RESET_OVERRIDE(r2dx_v33_state,r2dx_v33)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(55.47)    /* verified on pcb */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(500) /* not accurate */)
	MCFG_SCREEN_SIZE(44*8, 34*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(raiden2_state, screen_update_raiden2)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", rdx_v33)
	MCFG_PALETTE_ADD("palette", 2048)
	//MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(raiden2_state,raiden2)

	MCFG_DEVICE_ADD("crtc", SEIBU_CRTC, 0)
	MCFG_SEIBU_CRTC_LAYER_EN_CB(WRITE16(raiden2_state, tilemap_enable_w))
	MCFG_SEIBU_CRTC_LAYER_SCROLL_CB(WRITE16(raiden2_state, tile_scroll_w))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_28_63636MHz/28, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, r2dx_oki_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( nzerotea, r2dx_v33_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V33,XTAL_32MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(nzerotea_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", r2dx_v33_state,  rdx_v33_interrupt)

	MCFG_MACHINE_RESET_OVERRIDE(r2dx_v33_state,nzeroteam)


	//  SEIBU2_RAIDEN2_SOUND_SYSTEM_CPU(14318180/4)
	SEIBU_NEWZEROTEAM_SOUND_SYSTEM_CPU(14318180/4)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(55.47)    /* verified on pcb */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(500) /* not accurate */)
	MCFG_SCREEN_SIZE(44*8, 34*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(raiden2_state, screen_update_raiden2)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", rdx_v33)
	MCFG_PALETTE_ADD("palette", 2048)
	//MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(raiden2_state,raiden2)

	MCFG_DEVICE_ADD("crtc", SEIBU_CRTC, 0)
	MCFG_SEIBU_CRTC_LAYER_EN_CB(WRITE16(raiden2_state, tilemap_enable_w))
	MCFG_SEIBU_CRTC_LAYER_SCROLL_CB(WRITE16(raiden2_state, tile_scroll_w))

	/* sound hardware */
//  SEIBU_SOUND_SYSTEM_YM2151_RAIDEN2_INTERFACE(28636360/8,28636360/28,1,2)
	SEIBU_SOUND_SYSTEM_YM3812_INTERFACE(14318180/4,1320000)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( zerotm2k, nzerotea )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(zerotm2k_map)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(r2dx_v33_state,rdx_v33)
{
	init_blending(raiden_blended_colors);
	static const int spri[5] = { 0, 1, 2, 3, -1 };
	cur_spri = spri;

	membank("bank1")->configure_entries(0, 0x40, memregion("maincpu")->base(), 0x10000);

	membank("bank3")->configure_entry(0, memregion("maincpu")->base()+0x030000); // 0x30000 - 0xfffff bank for Raiden 2
	membank("bank3")->configure_entry(1, memregion("maincpu")->base()+0x230000); // 0x30000 - 0xfffff bank for Raiden DX


	raiden2_decrypt_sprites(machine());

//  sensible defaults if booting as R2
	membank("bank1")->set_entry(0);
	membank("bank3")->set_entry(0);


	membank("okibank")->configure_entries(0, 4, memregion("oki")->base(), 0x40000);
	membank("okibank")->set_entry(0);

}

DRIVER_INIT_MEMBER(r2dx_v33_state,nzerotea)
{
	init_blending(zeroteam_blended_colors);
	static const int spri[5] = { -1, 0, 1, 2, 3 };
	cur_spri = spri;

	zeroteam_decrypt_sprites(machine());
}

DRIVER_INIT_MEMBER(r2dx_v33_state,zerotm2k)
{
	init_blending(zeroteam_blended_colors);
	static const int spri[5] = { -1, 0, 1, 2, 3 };
	cur_spri = spri;

	// no sprite encryption(!)

	// BG tile rom has 2 lines swapped
	UINT8 *src = memregion("gfx2")->base()+0x100000;
	int len = 0x080000;

	dynamic_buffer buffer(len);
	int i;
	for (i = 0; i < len; i ++)
		buffer[i] = src[BITSWAP32(i,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,5,6,4,3,2,1,0)];
	memcpy(src, &buffer[0], len);
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
        Atmel 93C46 EEPROM (SOIC8)
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
	ROM_REGION( 0x400000, "maincpu", 0 ) /* v33 main cpu */
	ROM_LOAD("prg.223", 0x000000, 0x400000, CRC(b3dbcf98) SHA1(30d6ec2090531c8c579dff74c4898889902d7d87) )

	ROM_REGION( 0x040000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "fix.613", 0x000000, 0x040000, CRC(3da27e39) SHA1(3d446990bf36dd0a3f8fadb68b15bed54904c8b5) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "bg.612", 0x000000, 0x400000, CRC(162c61e9) SHA1(bd0a6a29804b84196ba6bf3402e9f30a25da9269) )

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "obj1.724", 0x000000, 0x400000, CRC(7d218985) SHA1(777241a533defcbea3d7e735f309478d260bad52) )
	ROM_LOAD32_WORD( "obj2.725", 0x000002, 0x400000, CRC(891b24d6) SHA1(74f89b47b1ba6b84ddd96d1fae92fddad0ace342) )

	ROM_REGION( 0x100000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "pcm.099", 0x00000, 0x100000, CRC(97ca2907) SHA1(bfe8189300cf72089d0beaeab8b1a0a1a4f0a5b6) )

	ROM_REGION( 0x20000, "math", 0 ) /* SEI333 (AKA COPX-D3) data */
	ROM_LOAD( "copx_d3.357", 0x00000, 0x20000, CRC(fa2cf3ad) SHA1(13eee40704d3333874b6e3da9ee7d969c6dc662a) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "raidendx_eeprom-r2dx_v33.bin", 0x0000, 0x0080, CRC(0b34c0ca) SHA1(20612d5a1d819d3997ea47e8de7a194ec61b537d) ) // for booting as Raiden DX
ROM_END

ROM_START( r2dx_v33_r2 )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* v33 main cpu */
	ROM_LOAD("prg.223", 0x000000, 0x400000, CRC(b3dbcf98) SHA1(30d6ec2090531c8c579dff74c4898889902d7d87) )

	ROM_REGION( 0x040000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "fix.613", 0x000000, 0x040000, CRC(3da27e39) SHA1(3d446990bf36dd0a3f8fadb68b15bed54904c8b5) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "bg.612", 0x000000, 0x400000, CRC(162c61e9) SHA1(bd0a6a29804b84196ba6bf3402e9f30a25da9269) )

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "obj1.724", 0x000000, 0x400000, CRC(7d218985) SHA1(777241a533defcbea3d7e735f309478d260bad52) )
	ROM_LOAD32_WORD( "obj2.725", 0x000002, 0x400000, CRC(891b24d6) SHA1(74f89b47b1ba6b84ddd96d1fae92fddad0ace342) )


	ROM_REGION( 0x100000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "pcm.099", 0x00000, 0x100000, CRC(97ca2907) SHA1(bfe8189300cf72089d0beaeab8b1a0a1a4f0a5b6) )

	ROM_REGION( 0x20000, "math", 0 ) /* SEI333 (AKA COPX-D3) data */
	ROM_LOAD( "copx_d3.357", 0x00000, 0x20000, CRC(fa2cf3ad) SHA1(13eee40704d3333874b6e3da9ee7d969c6dc662a) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "raidenii_eeprom-r2dx_v33.bin", 0x0000, 0x0080, CRC(ba454777) SHA1(101c5364e8664d17bfb1e759515d135a2673d67e) ) // for booting as Raiden 2
ROM_END

// uses dipswitches
ROM_START( nzeroteam ) /* V33 SYSTEM TYPE_B hardware, uses SEI333 (AKA COPX-D3) for protection  */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("prg1", 0x000000, 0x80000, CRC(3c7d9410) SHA1(25f2121b6c2be73f11263934266901ed5d64d2ee) )
	ROM_LOAD16_BYTE("prg2", 0x000001, 0x80000, CRC(6cba032d) SHA1(bf5d488cd578fff09e62e3650efdee7658033e3f) )

	ROM_REGION( 0x20000, "math", 0 ) /* SEI333 (AKA COPX-D3) data */
	ROM_LOAD( "copx-d3.bin", 0x00000, 0x20000, CRC(fa2cf3ad) SHA1(13eee40704d3333874b6e3da9ee7d969c6dc662a) ) /* Not from this set, but same data as Zero Team 2000 & Raiden II New */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "sound",       0x000000, 0x08000, CRC(7ec1fbc3) SHA1(48299d6530f641b18764cc49e283c347d0918a47) ) /* Same as some of other Zero Team sets */
	ROM_CONTINUE(            0x010000, 0x08000 )    /* banked stuff */
	ROM_COPY( "audiocpu", 0x0000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "fix1", 0x000000, 0x010000, CRC(0c4895b0) SHA1(f595dbe5a19edb8a06ea60105ee26b95db4a2619) )
	ROM_LOAD16_BYTE( "fix2", 0x000001, 0x010000, CRC(07d8e387) SHA1(52f54a6a4830592784cdf643a5f255aa3db53e50) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "back-1", 0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) ) /* Same as "MUSHA BACK-1" of other Zero Team sets */
	ROM_LOAD( "back-2", 0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) ) /* Same as "MUSHA BACK-2" of other Zero Team sets */

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "obj-1", 0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) ) /* Same as "MUSHA OBJ-1" of other Zero Team sets */
	ROM_LOAD32_WORD( "obj-2", 0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) ) /* Same as "MUSHA OBJ-2" of other Zero Team sets */

	ROM_REGION( 0x100000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "6.pcm", 0x00000, 0x40000, CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) /* Same as other Zero Team sets */
ROM_END

// uses a 93c46a eeprom
ROM_START( zerotm2k ) /* V33 SYSTEM TYPE_C VER2 hardware, uses SEI333 (AKA COPX-D3) for protection  */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD( "mt28f800b1.u0230", 0x000000, 0x100000, CRC(6ab49d8c) SHA1(d94ec9a46ff98a76c3372369246733268474de99) ) /* SMT rom, PCB silkscreened PRG01 */
	/* PCB has unpopulated socket space for two 27C040 at u0224 silkscreened PRG0 & u0226 silkscreened PRG1) */

	ROM_REGION( 0x20000, "math", 0 ) /* SEI333 (AKA COPX-D3) data */
	ROM_LOAD( "mx27c1000mc.u0366", 0x00000, 0x20000, CRC(fa2cf3ad) SHA1(13eee40704d3333874b6e3da9ee7d969c6dc662a) ) /* PCB silkscreened 333ROM */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "syz-02.u019",  0x000000, 0x08000, CRC(55371073) SHA1(f6e182fa64630595dc8c25ac820e12983cfbed12) ) /* PCB silkscreened SOUND */
	ROM_CONTINUE(             0x010000, 0x08000 )   /* banked stuff */
	ROM_COPY( "audiocpu", 0x0000,  0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "syz-04.u0616", 0x000000, 0x010000, CRC(3515a45f) SHA1(a25a7e23a5d9cf5a95a0d0e828848a8d223bdf51) ) /* PCB silkscreened FIX E */
	ROM_LOAD16_BYTE( "syz-03.u0617", 0x000001, 0x010000, CRC(02fbf9d7) SHA1(6eb4db1f89c9b003e7eed7bf39e6065b1c99447f) ) /* PCB silkscreened FIX O */

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "szy-05.u0614",     0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) ) /* PCB silkscreened BG12, Same as "MUSHA BACK-1" */
	ROM_LOAD( "mt28f400b1.u0619", 0x100000, 0x080000, CRC(266acee6) SHA1(2a9da66c313a7536c7fb393134b9df0bb122cb2b) ) /* SMT rom, PCB silkscreened BG3 */
	/* PCB has an unpopulated socket rom space for a LH535A00D at u0615 for alt BG3 location */

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (NOT encrypted) */
	ROM_LOAD32_WORD( "musha_obj-1a.u0729", 0x000000, 0x200000, CRC(9b2cf68c) SHA1(cd8cb277091bfa125fd0f68410de39f72f1c7047) ) /* PCB silkscreened OBJ1 */
	ROM_LOAD32_WORD( "musha_obj-2a.u0730", 0x000002, 0x200000, CRC(fcabee05) SHA1(b2220c0311b3bd2fd44fb56fff7c27bed0816fe9) ) /* PCB silkscreened OBJ2 */
	/* PCB has unpopulated rom space for two SMT roms at u0734 & u0736 for alt OBJ1 & OBJ2 locations) */

	ROM_REGION( 0x100000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "szy-01.u099", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) /* PCB silkscreened PCM, Same as other Zero Team sets */
ROM_END

// newer PCB, with V33 CPU and COPD3 protection, but weak sound hardware. - was marked as Raiden DX New in the rom dump, but boots as Raiden 2 New version, the rom contains both
// is there a switching method? for now I've split it into 2 sets with different EEPROM, the game checks that on startup and runs different code depending on what it finds
GAME( 1996, r2dx_v33,    0,          rdx_v33,  rdx_v33, r2dx_v33_state,  rdx_v33,   ROT270, "Seibu Kaihatsu", "Raiden II New / Raiden DX (newer V33 PCB) (Raiden DX EEPROM)", MACHINE_SUPPORTS_SAVE)
GAME( 1996, r2dx_v33_r2, r2dx_v33,   rdx_v33,  rdx_v33, r2dx_v33_state,  rdx_v33,   ROT270, "Seibu Kaihatsu", "Raiden II New / Raiden DX (newer V33 PCB) (Raiden II EEPROM)", MACHINE_SUPPORTS_SAVE)

// 'V33 system type_b' - uses V33 CPU, COPX-D3 external protection rom, but still has the proper sound system, DSW for settings
GAME( 1997, nzeroteam, zeroteam,  nzerotea, nzerotea, r2dx_v33_state, nzerotea,  ROT0,   "Seibu Kaihatsu (Haoyunlai Trading Company license)", "New Zero Team (V33 SYSTEM TYPE_B hardware)", MACHINE_SUPPORTS_SAVE) // license text translated from title screen

// 'V33 SYSTEM TYPE_C' - uses V33 CPU, basically the same board as TYPE_C VER2
// there is a version of New Zero Team on "V33 SYSTEM TYPE_C" board with EEPROM rather than dipswitches like Zero Team 2000

// 'V33 SYSTEM TYPE_C VER2' - uses V33 CPU, COPX-D3 external protection rom, but still has the proper sound system, unencrypted sprites, EEPROM for settings.  PCB also seen without 'VER2', looks the same
GAME( 2000, zerotm2k,  zeroteam,  zerotm2k, zerotm2k, r2dx_v33_state, zerotm2k,  ROT0,   "Seibu Kaihatsu", "Zero Team 2000", MACHINE_SUPPORTS_SAVE)

// there is also a 'Raiden 2 2000' on unknown hardware.
