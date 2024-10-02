// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, David Haywood
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
     - 2 Player only. Service mode only shows 2 Players, and the code confirms that the options
       for three and four players have been removed from this version.
     - Stages 3 and 1 are swapped, this is correct.

    Raiden 2 New / Raiden DX
     - This is a 2-in-1 board.  The current game to boot is stored in the EEPROM.
       If you wish to change game then on powerup you must hold down all 4 (P1) joystick
       directions simultaneously along with either button 1 or button 2.
       Obviously this is impossible with a real joystick!

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

 New Zero Team /Zero Team 2000

 000000-01ffff : 0xff fill, inaccessible by hardware?
 020000-0fffff : Fixed ROM data for Zero Team
 (no banking)

*/

#include "emu.h"
#include "raiden2.h"

#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "r2crypt.h"

#include "speaker.h"


namespace {

class r2dx_v33_state : public raiden2_state
{
public:
	r2dx_v33_state(const machine_config &mconfig, device_type type, const char *tag) :
		raiden2_state(mconfig, type, tag),
		m_r2dxbank(0),
		m_r2dxgameselect(0),
		m_eeprom(*this, "eeprom"),
		m_math(*this, "math"),
		m_okibank(*this, "okibank")
	{
	}

	void nzerotea(machine_config &config);
	void rdx_v33(machine_config &config);
	void zerotm2k(machine_config &config);

	void init_rdx_v33();
	void init_nzerotea();
	void init_zerotm2k();

private:

	void angle_w(offs_t offset, u16 data, u16 mem_mask);
	void dx_w(offs_t offset, u16 data, u16 mem_mask);
	void dy_w(offs_t offset, u16 data, u16 mem_mask);
	void sdistl_w(offs_t offset, u16 data, u16 mem_mask);
	void sdisth_w(offs_t offset, u16 data, u16 mem_mask);
	u16 angle_r();
	u16 dist_r();
	u16 sin_r();
	u16 cos_r();

	void tile_bank_w(u8 data);
	[[maybe_unused]] u16 rdx_v33_unknown_r();
	[[maybe_unused]] void mcu_xval_w(u16 data);
	[[maybe_unused]] void mcu_yval_w(u16 data);
	[[maybe_unused]] void mcu_table_w(offs_t offset, u16 data);
	[[maybe_unused]] void mcu_table2_w(offs_t offset, u16 data);
	void mcu_prog_w(u16 data);
	void mcu_prog_w2(u16 data);
	void mcu_prog_offs_w(u16 data);
	void rdx_v33_eeprom_w(u8 data);
	void zerotm2k_eeprom_w(u16 data);
	void r2dx_rom_bank_w(u16 data);

	void tilemapdma_w(address_space &space, u16 data);
	void paldma_w(address_space &space, u16 data);
	u16 r2dx_debug_r();

	DECLARE_MACHINE_RESET(r2dx_v33);
	DECLARE_MACHINE_RESET(nzeroteam);

	void nzerotea_map(address_map &map) ATTR_COLD;
	void nzeroteam_base_map(address_map &map) ATTR_COLD;
	void r2dx_oki_map(address_map &map) ATTR_COLD;
	void rdx_v33_map(address_map &map) ATTR_COLD;
	void zerotm2k_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;

	void r2dx_setbanking();

	int m_r2dxbank;
	int m_r2dxgameselect;

	u16 m_dx, m_dy, m_angle;
	u32 m_sdist;
	u16 m_mcu_prog[0x800];
	int m_mcu_prog_offs;
	u16 m_mcu_xval, m_mcu_yval;
	u16 m_mcu_data[9];

	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_region_ptr<u8> m_math;

	optional_memory_bank m_okibank;
};

void r2dx_v33_state::machine_start()
{
	common_save_state();

	save_item(NAME(m_r2dxbank));
	save_item(NAME(m_r2dxgameselect));
	save_item(NAME(m_dx));
	save_item(NAME(m_dy));
	save_item(NAME(m_angle));
	save_item(NAME(m_sdist));
	save_item(NAME(m_mcu_prog));
	save_item(NAME(m_mcu_prog_offs));
	save_item(NAME(m_mcu_xval));
	save_item(NAME(m_mcu_yval));
	save_item(NAME(m_mcu_data));
}

void r2dx_v33_state::tile_bank_w(u8 data)
{
	int new_bank;
	new_bank = ((data & 0x10)>>4);
	if (new_bank != m_bg_bank)
	{
		m_bg_bank = new_bank;
		m_background_layer->mark_all_dirty();
	}

	new_bank = 2 + ((data & 0x20)>>5);
	if (new_bank != m_mid_bank)
	{
		m_mid_bank = new_bank;
		m_midground_layer->mark_all_dirty();
	}

	new_bank = 4 | (data & 3);
	if (new_bank != m_fg_bank)
	{
		m_fg_bank = new_bank;
		m_foreground_layer->mark_all_dirty();
	}
}

void r2dx_v33_state::r2dx_setbanking()
{
	m_mainbank[0]->set_entry(m_r2dxgameselect*0x10 + m_r2dxbank);
	m_mainbank[1]->set_entry(m_r2dxgameselect);
}

void r2dx_v33_state::rdx_v33_eeprom_w(u8 data)
{
	m_eeprom->clk_write((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->di_write((data & 0x20) >> 5);
	m_eeprom->cs_write((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);

	// 0x40 - coin counter 1?
	// 0x80 - coin counter 2?

	// 0x04 is active in Raiden DX mode, it could be part of the rom bank (which half of the rom to use) or the FG tile bank (or both?)
	// the bit gets set if it reads RAIDENDX from the EEPROM
	m_r2dxgameselect = (data & 0x04) >> 2;

	m_tx_bank = m_r2dxgameselect;
	m_text_layer->mark_all_dirty();

	r2dx_setbanking();

	m_okibank->set_entry(data&3);
}

/* new zero team uses the copd3 protection... and uploads a 0x400 byte table, probably the mcu code, encrypted */

void r2dx_v33_state::mcu_prog_w(u16 data)
{
	m_mcu_prog[m_mcu_prog_offs*2] = data;
}

void r2dx_v33_state::mcu_prog_w2(u16 data)
{
	m_mcu_prog[m_mcu_prog_offs*2+1] = data;

	// both new zero team and raiden2/dx v33 version upload the same table..
#if 0
	{
		char tmp[64];
		FILE *fp;
		sprintf(tmp,"cop3_%s.data", machine().system().name);

		fp=fopen(tmp, "w+b");
		if (fp)
		{
			fwrite(m_mcu_prog, 0x400, 2, fp);
			fclose(fp);
		}
	}
#endif
}

void r2dx_v33_state::mcu_prog_offs_w(u16 data)
{
	m_mcu_prog_offs = data;
}

u16 r2dx_v33_state::rdx_v33_unknown_r()
{
	return machine().rand();
}


/* something sent to the MCU for X/Y global screen calculating ... */
void r2dx_v33_state::mcu_xval_w(u16 data)
{
	m_mcu_xval = data;
	//popmessage("%04x %04x",m_mcu_xval,m_mcu_yval);
}

void r2dx_v33_state::mcu_yval_w(u16 data)
{
	m_mcu_yval = data;
	//popmessage("%04x %04x",m_mcu_xval,m_mcu_yval);
}


/* 0x400-0x407 seems some DMA hook-up, 0x420-0x427 looks like some x/y sprite calculation routine */
void r2dx_v33_state::mcu_table_w(offs_t offset, u16 data)
{
	m_mcu_data[offset] = data;

	//popmessage("%04x %04x %04x %04x | %04x %04x %04x %04x",m_mcu_data[0/2],m_mcu_data[2/2],m_mcu_data[4/2],m_mcu_data[6/2],m_mcu_data[8/2],m_mcu_data[0xa/2],m_mcu_data[0xc/2],m_mcu_data[0xe/2]);
}

void r2dx_v33_state::mcu_table2_w(offs_t offset, u16 data)
{
//  printf("mcu_table2_w %04x\n", data);

	m_mcu_data[offset+4] = data;

	//popmessage("%04x %04x %04x %04x | %04x %04x %04x %04x",m_mcu_data[0/2],m_mcu_data[2/2],m_mcu_data[4/2],m_mcu_data[6/2],m_mcu_data[8/2],m_mcu_data[0xa/2],m_mcu_data[0xc/2],m_mcu_data[0xe/2]);
}

void r2dx_v33_state::r2dx_rom_bank_w(u16 data)
{
	//printf("rom bank %04x\n", data);
	m_r2dxbank = data & 0xf;
	r2dx_setbanking();

}

void r2dx_v33_state::angle_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_angle);
}

void r2dx_v33_state::dx_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_dx);
}

void r2dx_v33_state::dy_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_dy);
}

u16 r2dx_v33_state::angle_r()
{
	return m_math[((m_dy & 0xff) << 8) | (m_dx & 0xff)];
}

u16 r2dx_v33_state::dist_r()
{
	return sqrt(double(m_sdist));
}

u16 r2dx_v33_state::sin_r()
{
	int off = 65536 + (m_angle & 0xff)*4;
	return (m_math[off+0]) | (m_math[off+1] << 8);
}

u16 r2dx_v33_state::cos_r()
{
	int off = 65536 + (m_angle & 0xff)*4;
	return (m_math[off+2]) | (m_math[off+3] << 8);
}

void r2dx_v33_state::sdistl_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_sdist = (m_sdist & (0xffff0000 | u16(~mem_mask))) | (data & mem_mask);
}

void r2dx_v33_state::sdisth_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_sdist = (m_sdist & (0x0000ffff | (u16(~mem_mask)) << 16)) | ((data & mem_mask) << 16);
}

// these DMA operations seem to use hardcoded addresses on this hardware
void r2dx_v33_state::tilemapdma_w(address_space &space, u16 data)
{
	int src = 0xd000;

	for (int i = 0; i < 0x2800 / 2; i++)
	{
		u16 tileval = space.read_word(src);
		src += 2;
		m_videoram_private_w(i, tileval);
	}
}

void r2dx_v33_state::paldma_w(address_space &space, u16 data)
{
	int src = 0x1f000;

	for (int i = 0; i < 0x1000 / 2; i++)
	{
		u16 palval = space.read_word(src);
		src += 2;
		m_palette->write16(i, palval);
	}
}

u16 r2dx_v33_state::r2dx_debug_r()
{
	// read once on startup, needed for player collisions to work
	return 0xffff;
}

void r2dx_v33_state::rdx_v33_map(address_map &map)
{
	map(0x00000, 0x003ff).ram(); // vectors copied here

	map(0x00400, 0x00401).w(FUNC(r2dx_v33_state::tilemapdma_w)); // tilemaps to private buffer
	map(0x00402, 0x00403).w(FUNC(r2dx_v33_state::paldma_w));  // palettes to private buffer

	map(0x00404, 0x00405).w(FUNC(r2dx_v33_state::r2dx_rom_bank_w));
	map(0x00406, 0x00406).w(FUNC(r2dx_v33_state::tile_bank_w));

	map(0x00420, 0x00421).w(FUNC(r2dx_v33_state::dx_w));
	map(0x00422, 0x00423).w(FUNC(r2dx_v33_state::dy_w));
	map(0x00424, 0x00425).w(FUNC(r2dx_v33_state::sdistl_w));
	map(0x00426, 0x00427).w(FUNC(r2dx_v33_state::sdisth_w));
	map(0x00428, 0x00429).w(FUNC(r2dx_v33_state::angle_w));

	map(0x00430, 0x00431).r(FUNC(r2dx_v33_state::angle_r));
	map(0x00432, 0x00433).r(FUNC(r2dx_v33_state::dist_r));
	map(0x00434, 0x00435).r(FUNC(r2dx_v33_state::sin_r));
	map(0x00436, 0x00437).r(FUNC(r2dx_v33_state::cos_r));

	map(0x00600, 0x0063f).rw("crtc", FUNC(seibu_crtc_device::read), FUNC(seibu_crtc_device::write));
	//map(0x00640, 0x006bf).rw("obj", FUNC(seibu_encrypted_sprite_device::read), FUNC(seibu_encrypted_sprite_device::write));
	map(0x0068e, 0x0068f).w(m_spriteram, FUNC(buffered_spriteram16_device::write));
	map(0x006b0, 0x006b1).w(FUNC(r2dx_v33_state::mcu_prog_w)); // could be encryption key uploads just like raiden2.cpp ?
	map(0x006b2, 0x006b3).w(FUNC(r2dx_v33_state::mcu_prog_w2));
//  map(0x006b4, 0x006b5).nopw();
//  map(0x006b6, 0x006b7).nopw();
	map(0x006bc, 0x006bd).w(FUNC(r2dx_v33_state::mcu_prog_offs_w));
//  map(0x006be, 0x006bf).nopw();

	// sprite protection not 100% verified as the same
	map(0x006c0, 0x006c1).rw(FUNC(r2dx_v33_state::sprite_prot_off_r), FUNC(r2dx_v33_state::sprite_prot_off_w));
	map(0x006c2, 0x006c3).rw(FUNC(r2dx_v33_state::sprite_prot_src_seg_r), FUNC(r2dx_v33_state::sprite_prot_src_seg_w));
	map(0x006c6, 0x006c7).w(FUNC(r2dx_v33_state::sprite_prot_dst1_w));

	map(0x006d8, 0x006d9).w(FUNC(r2dx_v33_state::sprite_prot_x_w));
	map(0x006da, 0x006db).w(FUNC(r2dx_v33_state::sprite_prot_y_w));
	map(0x006dc, 0x006dd).rw(FUNC(r2dx_v33_state::sprite_prot_maxx_r), FUNC(r2dx_v33_state::sprite_prot_maxx_w));
	map(0x006de, 0x006df).w(FUNC(r2dx_v33_state::sprite_prot_src_w));

	map(0x00700, 0x00700).w(FUNC(r2dx_v33_state::rdx_v33_eeprom_w));
	map(0x00740, 0x00741).r(FUNC(r2dx_v33_state::r2dx_debug_r));
	map(0x00744, 0x00745).portr("INPUT");
	map(0x0074c, 0x0074d).portr("SYSTEM");
	map(0x00762, 0x00763).r(FUNC(r2dx_v33_state::sprite_prot_dst1_r));

	map(0x00780, 0x00780).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // single OKI chip on this version

	map(0x00800, 0x00fff).ram(); // copies eeprom here?
	map(0x01000, 0x0bfff).ram();

	map(0x0c000, 0x0cfff).ram().share("spriteram");
	map(0x0d000, 0x0d7ff).ram(); //.w(FUNC(r2dx_v33_state::background_w)).share("back_data");
	map(0x0d800, 0x0dfff).ram(); //.w(FUNC(r2dx_v33_state::foreground_w).share("fore_data");
	map(0x0e000, 0x0e7ff).ram(); //.w(FUNC(r2dx_v33_state::midground_w).share("mid_data");
	map(0x0e800, 0x0f7ff).ram(); //.w(FUNC(r2dx_v33_state::text_w).share("text_data");
	map(0x0f800, 0x0ffff).ram(); /* Stack area */
	map(0x10000, 0x1efff).ram();
	map(0x1f000, 0x1ffff).ram(); //.w(m_palette, FUNC(palette_device::write16)).share("palette");

	map(0x20000, 0x2ffff).bankr("mainbank1").nopw();
	map(0x30000, 0xfffff).bankr("mainbank2").nopw();
}


void r2dx_v33_state::nzeroteam_base_map(address_map &map)
{
	map(0x00000, 0x003ff).ram(); //stack area

	map(0x00400, 0x00401).w(FUNC(r2dx_v33_state::tilemapdma_w)); // tilemaps to private buffer
	map(0x00402, 0x00403).w(FUNC(r2dx_v33_state::paldma_w));  // palettes to private buffer
	// 0x404 is bank on r2dx, this doesn't need it
	// map(0x00406, 0x00406).w(FUNC(r2dx_v33_state::tile_bank_w)); // not the same?

	map(0x00406, 0x00407).noprw(); // always 6022, supposed to be the tile bank but ignores the actual value???

	map(0x00420, 0x00421).w(FUNC(r2dx_v33_state::dx_w));
	map(0x00422, 0x00423).w(FUNC(r2dx_v33_state::dy_w));
	map(0x00424, 0x00425).w(FUNC(r2dx_v33_state::sdistl_w));
	map(0x00426, 0x00427).w(FUNC(r2dx_v33_state::sdisth_w));
	map(0x00428, 0x00429).w(FUNC(r2dx_v33_state::angle_w));

	map(0x00430, 0x00431).r(FUNC(r2dx_v33_state::angle_r));
	map(0x00432, 0x00433).r(FUNC(r2dx_v33_state::dist_r));
	map(0x00434, 0x00435).r(FUNC(r2dx_v33_state::sin_r));
	map(0x00436, 0x00437).r(FUNC(r2dx_v33_state::cos_r));

	map(0x00600, 0x0063f).rw("crtc", FUNC(seibu_crtc_device::read), FUNC(seibu_crtc_device::write));
	//map(0x00640, 0x006bf)rw("obj", FUNC(seibu_encrypted_sprite_device::read), FUNC(seibu_encrypted_sprite_device::write));
	map(0x0068e, 0x0068f).w(m_spriteram, FUNC(buffered_spriteram16_device::write));
	map(0x006b0, 0x006b1).w(FUNC(r2dx_v33_state::mcu_prog_w));
	map(0x006b2, 0x006b3).w(FUNC(r2dx_v33_state::mcu_prog_w2));
//  map(0x006b4, 0x006b5).nopw();
//  map(0x006b6, 0x006b7).nopw();
	map(0x006bc, 0x006bd).w(FUNC(r2dx_v33_state::mcu_prog_offs_w));
//  map(0x006d8, 0x006d9).w(FUNC(r2dx_v33_state::bbbbll_w)); // scroll?
//  map(0x006dc, 0x006dd).r(FUNC(r2dx_v33_state::nzerotea_unknown_r));
//  map(0x006de, 0x006df).w(FUNC(r2dx_v33_state::mcu_unkaa_w)); // mcu command related?
//  map(0x00700, 0x00700).w(FUNC(r2dx_v33_state::rdx_v33_eeprom_w));

//  map(0x00762, 0x00763).r(FUNC(r2dx_v33_state::nzerotea_unknown_r));

	map(0x00780, 0x0079f).lrw8(
							   NAME([this] (offs_t offset) { return m_seibu_sound->main_r(offset >> 1); }),
							   NAME([this] (offs_t offset, u8 data) { m_seibu_sound->main_w(offset >> 1, data); })).umask16(0x00ff);

	map(0x00800, 0x00fff).ram();
	map(0x01000, 0x0bfff).ram();

	map(0x0c000, 0x0cfff).ram().share("spriteram");
	map(0x0d000, 0x0d7ff).ram(); //.w(FUNC(r2dx_v33_state::background_w)).share("back_data");
	map(0x0d800, 0x0dfff).ram(); //.w(FUNC(r2dx_v33_state::foreground_w)).share("fore_data");
	map(0x0e000, 0x0e7ff).ram(); //.w(FUNC(r2dx_v33_state::midground_w)).share("mid_data");
	map(0x0e800, 0x0f7ff).ram(); //.w(FUNC(r2dx_v33_state::text_w)).share("text_data");
	map(0x0f800, 0x0ffff).ram(); /* Stack area */
	map(0x10000, 0x1efff).ram();
	map(0x1f000, 0x1ffff).ram(); //.w("palette", FUNC(palette_device::write)).share("palette");

	map(0x20000, 0xfffff).rom().region("maincpu", 0x20000);
}

void r2dx_v33_state::nzerotea_map(address_map &map)
{
	nzeroteam_base_map(map);
	map(0x00740, 0x00741).portr("DSW");
	map(0x00744, 0x00745).portr("INPUT");
	map(0x0074c, 0x0074d).portr("SYSTEM");
}

void r2dx_v33_state::zerotm2k_eeprom_w(u16 data)
{
//  printf("zerotm2k_eeprom_w %04x\n", data);

	m_eeprom->clk_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->di_write((data & 0x04) >> 2);
	m_eeprom->cs_write((data & 0x01) ? ASSERT_LINE : CLEAR_LINE);
}

void r2dx_v33_state::zerotm2k_map(address_map &map)
{
	nzeroteam_base_map(map);
	map(0x00740, 0x00741).portr("P3_P4");
	map(0x00744, 0x00745).portr("INPUT");
	map(0x0074c, 0x0074d).portr("SYSTEM");
	map(0x00748, 0x00749).w(FUNC(r2dx_v33_state::zerotm2k_eeprom_w));
}



static INPUT_PORTS_START( rdx_v33 )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
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

	// The P3_P4 port is never read in this version

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
	PORT_DIPSETTING(      0x0100, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0c00, "2" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPSETTING(      0x0800, "4" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(      0x2000, "1000000" )
	PORT_DIPSETTING(      0x3000, "2000000" )
	PORT_DIPSETTING(      0x1000, "3000000" )
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
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

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
	bank_reset(0,6,2,0);
}

MACHINE_RESET_MEMBER(r2dx_v33_state,nzeroteam)
{
	bank_reset(0,2,1,0);
}

void r2dx_v33_state::r2dx_oki_map(address_map &map)
{
	map(0x00000, 0x3ffff).bankr("okibank");
}

void r2dx_v33_state::rdx_v33(machine_config &config)
{
	/* basic machine hardware */
	V33(config, m_maincpu, 32000000/2); // ?
	m_maincpu->set_addrmap(AS_PROGRAM, &r2dx_v33_state::rdx_v33_map);
	m_maincpu->set_vblank_int("screen", FUNC(r2dx_v33_state::interrupt));

	MCFG_MACHINE_RESET_OVERRIDE(r2dx_v33_state,r2dx_v33)

	EEPROM_93C46_16BIT(config, m_eeprom);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	screen.set_refresh_hz(55.47);    /* verified on pcb */
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(500)); /* not accurate */
	screen.set_size(44*8, 34*8);
	screen.set_visarea(0*8, 40*8-1, 0, 30*8-1);
	screen.set_screen_update(FUNC(r2dx_v33_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, r2dx_v33_state::gfx_raiden2);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);

	seibu_crtc_device &crtc(SEIBU_CRTC(config, "crtc", 0));
	crtc.layer_en_callback().set(FUNC(r2dx_v33_state::tilemap_enable_w));
	crtc.layer_scroll_callback().set(FUNC(r2dx_v33_state::tile_scroll_w));

	BUFFERED_SPRITERAM16(config, m_spriteram);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(28'636'363)/28, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.set_addrmap(0, &r2dx_v33_state::r2dx_oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 0.5);
}

void r2dx_v33_state::nzerotea(machine_config &config)
{
	/* basic machine hardware */
	V33(config, m_maincpu, XTAL(32'000'000)/2); /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &r2dx_v33_state::nzerotea_map);
	m_maincpu->set_vblank_int("screen", FUNC(r2dx_v33_state::interrupt));

	MCFG_MACHINE_RESET_OVERRIDE(r2dx_v33_state,nzeroteam)

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(28'636'363)/8));
	audiocpu.set_addrmap(AS_PROGRAM, &r2dx_v33_state::zeroteam_sound_map);
	audiocpu.set_irq_acknowledge_callback("seibu_sound", FUNC(seibu_sound_device::im0_vector_cb));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	screen.set_refresh_hz(55.47);    /* verified on pcb */
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(500)); /* not accurate */
	screen.set_size(44*8, 34*8);
	screen.set_visarea(0*8, 40*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(r2dx_v33_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, r2dx_v33_state::gfx_raiden2);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);

	seibu_crtc_device &crtc(SEIBU_CRTC(config, "crtc", 0));
	crtc.layer_en_callback().set(FUNC(r2dx_v33_state::tilemap_enable_w));
	crtc.layer_scroll_callback().set(FUNC(r2dx_v33_state::tile_scroll_w));

	BUFFERED_SPRITERAM16(config, m_spriteram);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(28'636'363)/8));
	ymsnd.irq_handler().set("seibu_sound", FUNC(seibu_sound_device::fm_irqhandler));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(28'636'363)/28, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "mono", 0.40);

	SEIBU_SOUND(config, m_seibu_sound, 0);
	m_seibu_sound->int_callback().set_inputline("audiocpu", 0);
	m_seibu_sound->set_rom_tag("audiocpu");
	m_seibu_sound->set_rombank_tag("seibu_bank1");
	m_seibu_sound->ym_read_callback().set("ymsnd", FUNC(ym3812_device::read));
	m_seibu_sound->ym_write_callback().set("ymsnd", FUNC(ym3812_device::write));
}

void r2dx_v33_state::zerotm2k(machine_config &config)
{
	nzerotea(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &r2dx_v33_state::zerotm2k_map);

	EEPROM_93C46_16BIT(config, m_eeprom);
}

void r2dx_v33_state::init_rdx_v33()
{
	init_blending(raiden_blended_colors);
	static const int spri[5] = { 0, 1, 2, 3, -1 };
	m_cur_spri = spri;

	m_mainbank[0]->configure_entries(   0, 0x10, memregion("maincpu")->base() + 0x100000, 0x010000); // 0x20000 - 0x2ffff bank for Raiden 2
	m_mainbank[0]->configure_entries(0x10, 0x10, memregion("maincpu")->base() + 0x300000, 0x010000); // 0x20000 - 0x2ffff bank for Raiden DX
	m_mainbank[1]->configure_entries(   0,    2, memregion("maincpu")->base() + 0x030000, 0x200000);

	raiden2_decrypt_sprites(machine());

//  sensible defaults if booting as R2
	m_mainbank[0]->set_entry(0);
	m_mainbank[1]->set_entry(0);

	m_okibank->configure_entries(0, 4, memregion("oki")->base(), 0x40000);
	m_okibank->set_entry(0);

}

void r2dx_v33_state::init_nzerotea()
{
	init_blending(zeroteam_blended_colors);
	static const int spri[5] = { -1, 0, 1, 2, 3 };
	m_cur_spri = spri;

	zeroteam_decrypt_sprites(machine());
}

void r2dx_v33_state::init_zerotm2k()
{
	init_blending(zeroteam_blended_colors);
	static const int spri[5] = { -1, 0, 1, 2, 3 };
	m_cur_spri = spri;

	// no sprite encryption(!)

	// BG tile rom has 2 lines swapped
	u8 *src = memregion("gfx2")->base()+0x100000;
	const int len = 0x080000;

	std::vector<u8> buffer(len);
	for (int i = 0; i < len; i ++)
		buffer[i] = src[bitswap<32>(i,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,5,6,4,3,2,1,0)];
	memcpy(src, &buffer[0], len);
}

/*

Raiden DX
Seibu Kaihatsu, 1993/1996

Note! PCB seems like an updated version. It uses _entirely_ SMD technology and
is smaller than the previous hardware. I guess the game is still popular, so
Seibu re-manufactured it using newer technology to meet demand.
Previous version hardware is similar to Heated Barrel/Legionnaire/Seibu Cup Soccer etc.
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
	ROM_LOAD16_BYTE("seibu_1.u0224", 0x000000, 0x80000, CRC(ce1bcaf4) SHA1(1c340575e440b716caca8605cc5e1221060e3714) )
	ROM_LOAD16_BYTE("seibu_2.u0226", 0x000001, 0x80000, CRC(03f6e32d) SHA1(5363f20d515ff84346aa15f7b9d95c5805d81285) )

	ROM_REGION( 0x20000, "math", 0 ) /* SEI333 (AKA COPX-D3) data */
	ROM_LOAD( "copx-d3.bin", 0x00000, 0x20000, CRC(fa2cf3ad) SHA1(13eee40704d3333874b6e3da9ee7d969c6dc662a) ) /* Not from this set, but same data as Zero Team 2000 & Raiden II New */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "seibu_3.u01019", 0x000000, 0x08000, CRC(7ec1fbc3) SHA1(48299d6530f641b18764cc49e283c347d0918a47) ) /* Same as some of other Zero Team sets */
	ROM_CONTINUE(               0x010000, 0x08000 )    /* banked stuff */
	ROM_COPY( "audiocpu",       0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "seibu_5.u0616", 0x000000, 0x010000, CRC(ce68ba3c) SHA1(52830533711ec906bf4fe9d06e065ec80b25b4da) )
	ROM_LOAD16_BYTE( "seibu_6.u0617", 0x000001, 0x010000, CRC(cf44aea7) SHA1(e8d622fd5c10133fa563402daf0690fdff297f94) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "back-1", 0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) ) /* Same as "MUSHA BACK-1" of other Zero Team sets */
	ROM_LOAD( "back-2", 0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) ) /* Same as "MUSHA BACK-2" of other Zero Team sets */

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "obj-1", 0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) ) /* Same as "MUSHA OBJ-1" of other Zero Team sets */
	ROM_LOAD32_WORD( "obj-2", 0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) ) /* Same as "MUSHA OBJ-2" of other Zero Team sets */

	ROM_REGION( 0x100000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "seibu_4.u099", 0x00000, 0x40000, CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) /* Same as other Zero Team sets */

	ROM_REGION( 0x200, "pld", 0 ) /* PLDs */
	ROM_LOAD( "sysv33b-1.u0222.bin", 0x000, 0x117, CRC(f514a11f) SHA1(dd83ee1f511915d3d5f65375f34583be7fa1158b) )
	ROM_LOAD( "sysv33b-2.u0227.bin", 0x000, 0x117, CRC(d9f4612f) SHA1(0c507b28dc0f50a67cc12d63092067dc3f7f4679) )
ROM_END

ROM_START( nzeroteama ) /* V33 SYSTEM TYPE_B hardware, uses SEI333 (AKA COPX-D3) for protection  */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("seibu_1.u0224", 0x000000, 0x80000, CRC(cb277b46) SHA1(86e99f14f7349bfabb13d1f540d6fa46748379dc) ) // sldh
	ROM_LOAD16_BYTE("seibu_2.u0226", 0x000001, 0x80000, CRC(6debbf78) SHA1(92d6f3bb59f72b40d3bc6d731866105a135574fb) ) // sldh

	ROM_REGION( 0x20000, "math", 0 ) /* SEI333 (AKA COPX-D3) data */
	ROM_LOAD( "copx-d3.bin", 0x00000, 0x20000, CRC(fa2cf3ad) SHA1(13eee40704d3333874b6e3da9ee7d969c6dc662a) ) /* Not from this set, but same data as Zero Team 2000 & Raiden II New */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "seibu_3.u01019", 0x000000, 0x08000, CRC(7ec1fbc3) SHA1(48299d6530f641b18764cc49e283c347d0918a47) ) /* Same as some of other Zero Team sets */
	ROM_CONTINUE(               0x010000, 0x08000 )    /* banked stuff */
	ROM_COPY( "audiocpu",       0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "seibu_5.u0616", 0x000000, 0x010000, CRC(ce68ba3c) SHA1(52830533711ec906bf4fe9d06e065ec80b25b4da) )
	ROM_LOAD16_BYTE( "seibu_6.u0617", 0x000001, 0x010000, CRC(cf44aea7) SHA1(e8d622fd5c10133fa563402daf0690fdff297f94) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "back-1", 0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) ) /* Same as "MUSHA BACK-1" of other Zero Team sets */
	ROM_LOAD( "back-2", 0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) ) /* Same as "MUSHA BACK-2" of other Zero Team sets */

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "obj-1", 0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) ) /* Same as "MUSHA OBJ-1" of other Zero Team sets */
	ROM_LOAD32_WORD( "obj-2", 0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) ) /* Same as "MUSHA OBJ-2" of other Zero Team sets */

	ROM_REGION( 0x100000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "seibu_4.u099", 0x00000, 0x40000, CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) /* Same as other Zero Team sets */

	ROM_REGION( 0x200, "pld", 0 ) /* PLDs */
	ROM_LOAD( "sysv33b-1.u0222.bin", 0x000, 0x117, CRC(f514a11f) SHA1(dd83ee1f511915d3d5f65375f34583be7fa1158b) )
	ROM_LOAD( "sysv33b-2.u0227.bin", 0x000, 0x117, CRC(d9f4612f) SHA1(0c507b28dc0f50a67cc12d63092067dc3f7f4679) )
ROM_END

ROM_START( nzeroteamb ) /* V33 SYSTEM TYPE_B hardware, uses SEI333 (AKA COPX-D3) for protection  */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("prg1", 0x000000, 0x80000, CRC(3c7d9410) SHA1(25f2121b6c2be73f11263934266901ed5d64d2ee) )
	ROM_LOAD16_BYTE("prg2", 0x000001, 0x80000, CRC(6cba032d) SHA1(bf5d488cd578fff09e62e3650efdee7658033e3f) )

	ROM_REGION( 0x20000, "math", 0 ) /* SEI333 (AKA COPX-D3) data */
	ROM_LOAD( "copx-d3.bin", 0x00000, 0x20000, CRC(fa2cf3ad) SHA1(13eee40704d3333874b6e3da9ee7d969c6dc662a) ) /* Not from this set, but same data as Zero Team 2000 & Raiden II New */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "sound",    0x000000, 0x08000, CRC(7ec1fbc3) SHA1(48299d6530f641b18764cc49e283c347d0918a47) ) /* Same as some of other Zero Team sets */
	ROM_CONTINUE(         0x010000, 0x08000 )    /* banked stuff */
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

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

	ROM_REGION( 0x200, "pld", 0 ) /* PLDs */
	ROM_LOAD( "sysv33b-1.u0222.bin", 0x000, 0x117, CRC(f514a11f) SHA1(dd83ee1f511915d3d5f65375f34583be7fa1158b) )
	ROM_LOAD( "sysv33b-2.u0227.bin", 0x000, 0x117, CRC(d9f4612f) SHA1(0c507b28dc0f50a67cc12d63092067dc3f7f4679) )
ROM_END


// uses a 93c46a eeprom
ROM_START( zerotm2k ) /* V33 SYSTEM TYPE_C VER2 hardware, uses SEI333 (AKA COPX-D3) for protection  */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD( "mt28f800b1.u0230", 0x000000, 0x100000, CRC(6ab49d8c) SHA1(d94ec9a46ff98a76c3372369246733268474de99) ) /* SMT rom, PCB silkscreened PRG01 */
	/* PCB has unpopulated socket space for two 27C040 at u0224 silkscreened PRG0 & u0226 silkscreened PRG1) */

	ROM_REGION( 0x20000, "math", 0 ) /* SEI333 (AKA COPX-D3) data */
	ROM_LOAD( "mx27c1000mc.u0366", 0x00000, 0x20000, CRC(fa2cf3ad) SHA1(13eee40704d3333874b6e3da9ee7d969c6dc662a) ) /* PCB silkscreened 333ROM */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "syz-02.u019", 0x000000, 0x08000, CRC(55371073) SHA1(f6e182fa64630595dc8c25ac820e12983cfbed12) ) /* PCB silkscreened SOUND */
	ROM_CONTINUE(            0x010000, 0x08000 )   /* banked stuff */
	ROM_COPY( "audiocpu",    0x000000, 0x018000, 0x08000 )

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

} // anonymous namespace


// newer PCB, with V33 CPU and COPD3 protection, but weak sound hardware. - was marked as Raiden DX New in the rom dump, but boots as Raiden 2 New version, the rom contains both
// is there a switching method? for now I've split it into 2 sets with different EEPROM, the game checks that on startup and runs different code depending on what it finds
GAME( 1996, r2dx_v33,    0,        rdx_v33,  rdx_v33,  r2dx_v33_state, init_rdx_v33,   ROT270, "Seibu Kaihatsu", "Raiden II New / Raiden DX (newer V33 PCB) (Raiden DX EEPROM)", MACHINE_SUPPORTS_SAVE)
GAME( 1996, r2dx_v33_r2, r2dx_v33, rdx_v33,  rdx_v33,  r2dx_v33_state, init_rdx_v33,   ROT270, "Seibu Kaihatsu", "Raiden II New / Raiden DX (newer V33 PCB) (Raiden II EEPROM)", MACHINE_SUPPORTS_SAVE)

// 'V33 system type_b' - uses V33 CPU, COPX-D3 external protection rom, but still has the proper sound system, DSW for settings
GAME( 1997, nzeroteam,   zeroteam, nzerotea, nzerotea, r2dx_v33_state, init_nzerotea,  ROT0,   "Seibu Kaihatsu",                                     "New Zero Team (V33 SYSTEM TYPE_B hardware)", MACHINE_SUPPORTS_SAVE)
GAME( 1997, nzeroteama,  zeroteam, nzerotea, nzerotea, r2dx_v33_state, init_nzerotea,  ROT0,   "Seibu Kaihatsu (Zhongguo Shantou Yihuang license)",  "New Zero Team (V33 SYSTEM TYPE_B hardware, Zhongguo Shantou Yihuang license)", MACHINE_SUPPORTS_SAVE) // license text translated from title screen
GAME( 1997, nzeroteamb,  zeroteam, nzerotea, nzerotea, r2dx_v33_state, init_nzerotea,  ROT0,   "Seibu Kaihatsu (Haoyunlai Trading Company license)", "New Zero Team (V33 SYSTEM TYPE_B hardware, Haoyunlai Trading Company license)", MACHINE_SUPPORTS_SAVE) // license text translated from title screen

// 'V33 SYSTEM TYPE_C' - uses V33 CPU, basically the same board as TYPE_C VER2
// there is a version of New Zero Team on "V33 SYSTEM TYPE_C" board with EEPROM rather than dipswitches like Zero Team 2000
// 1998 release of New Zero team on this hardware also exists, but not dumped: https://youtu.be/8mnFjXCc9BI

// 'V33 SYSTEM TYPE_C VER2' - uses V33 CPU, COPX-D3 external protection rom, but still has the proper sound system, unencrypted sprites, EEPROM for settings.  PCB also seen without 'VER2', looks the same
GAME( 2000, zerotm2k,    zeroteam, zerotm2k, zerotm2k, r2dx_v33_state, init_zerotm2k,  ROT0,  "Seibu Kaihatsu", "Zero Team 2000", MACHINE_SUPPORTS_SAVE)

// there is also a 'Raiden 2 2000' on unknown hardware.
