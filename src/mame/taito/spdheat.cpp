// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Super Dead Heat hardware

    driver by Phil Bennett

    TODO:
      * Sound filters

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/input_merger.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/flt_vol.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "spdheat.lh"


namespace {

/*************************************
 *
 *  Machine class
 *
 *************************************/

class spdheat_state : public driver_device
{
public:
	spdheat_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_audiocpu(*this, "audiocpu"),
		m_audio_irq(*this, "audio_irq"),
		m_fg_ram(*this, "fg_ram%u", 0U),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette0(*this, "palette0"),
		m_palette1(*this, "palette1"),
		m_palette2(*this, "palette2"),
		m_palette3(*this, "palette3"),
		m_dac(*this, "dac")
	{ }

	void spdheat(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<input_merger_any_high_device> m_audio_irq;
	required_shared_ptr_array<uint16_t, 4> m_fg_ram;
	required_shared_ptr<uint16_t> m_spriteram;
	tilemap_t *m_fg_tilemap[4]{};

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette0;
	required_device<palette_device> m_palette1;
	required_device<palette_device> m_palette2;
	required_device<palette_device> m_palette3;
	required_device<dac_byte_interface> m_dac;

	uint32_t m_sound_data[4]{};
	uint32_t m_sound_status = 0;
	uint32_t m_sub_data = 0;
	uint32_t m_sub_status = 0;

	void main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
	void sub_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	void sub_dac_w(uint8_t data);
	void sub_nmi_w(uint8_t data);
	void sub_status_w(uint8_t data);
	uint8_t sub_snd_r();
	uint8_t soundstatus_r();
	uint8_t sub_status_r();
	uint16_t sound_status_r();
	template<int screen> void sound_w(uint16_t data);
	template<int screen> uint8_t sndcpu_sound_r();
	void ym1_port_a_w(uint8_t data);
	void ym1_port_b_w(uint8_t data);
	void ym2_port_a_w(uint8_t data);
	void ym2_port_b_w(uint8_t data);
	void ym3_port_a_w(uint8_t data);
	void ym3_port_b_w(uint8_t data);
	void ym4_port_a_w(uint8_t data);
	void ym4_port_b_w(uint8_t data);

	template<int screen> void text_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template<int screen> TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t xo, uint32_t yo);
	template<int which> uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*************************************
 *
 *  Machine initialization
 *
 *************************************/

void spdheat_state::machine_start()
{
	save_item(NAME(m_sound_data));
	save_item(NAME(m_sound_status));
	save_item(NAME(m_sub_data));
	save_item(NAME(m_sub_status));
}

void spdheat_state::machine_reset()
{
	m_sound_status = 0x000f;
	m_sub_data = 0;
}


/*************************************
 *
 *  Video reset
 *
 *************************************/

void spdheat_state::video_start()
{
	m_fg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(spdheat_state::get_fg_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(spdheat_state::get_fg_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(spdheat_state::get_fg_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(spdheat_state::get_fg_tile_info<3>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


/*************************************
 *
 *  Video memory access
 *
 *************************************/

template<int screen>
void spdheat_state::text_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fg_ram[screen][offset]);
	m_fg_tilemap[screen]->mark_tile_dirty(offset);
}


template<int screen>
TILE_GET_INFO_MEMBER(spdheat_state::get_fg_tile_info)
{
	uint16_t data = m_fg_ram[screen][tile_index];
	uint16_t code = data & 0x07ff;
	uint16_t color = (data & 0x3800) >> 12;
	tileinfo.set(0, code, color, TILE_FLIPYX((data & 0xc000) >> 14));
}


/*************************************
 *
 *  Drawing routines
 *
 *************************************/

void spdheat_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t xo, uint32_t yo)
{
	gfx_element *gfx = m_gfxdecode->gfx(1);

	/*
	 Sprite RAM format:

	 0:  .... .... .... .... = ?

	 1:  .... .... xxxx xxxx = Y position
	     .... ..x. .... .... = Y position
	     x... .... .... .... = 0 = 16x16, 1 = 16x32

	 2:  .... ..xx xxxx xxxx = Code
	     ..xx xx.. .... .... = Color
	     .x.. .... .... .... = X flip
	     x... .... .... .... = Y flip

	 3:  .... ..xx xxxx xxxx = X position
	 */

	for (int offs = (0x800 / 2) - 4; offs >= 0; offs -= 4)
	{
		int code = (m_spriteram[offs + 2] & 0x3ff) ^ 0x200;
		int color = (m_spriteram[offs + 2] >> 10) & 0x0f;

		int y = 256 - ((m_spriteram[offs + 1] & 0x0ff));
		int x = (m_spriteram[offs + 3] & 0x3ff);

		int flipx = BIT(m_spriteram[offs + 2], 14);
		int flipy = BIT(m_spriteram[offs + 2], 15);

		if (xo == 1)
		{
			x -= 0x200;
		}
		else
		{
			if (x & 0x200)
				x -= 0x400;
		}

		if (yo != BIT(m_spriteram[offs + 1], 9))
			continue;

		if ((m_spriteram[offs + 1] & 0x8000) == 0)
		{
			if (flipy)
			{
				// 16 x 32 mode
				gfx->transpen(bitmap, cliprect, code, color, flipx, flipy, x, y, 0);
				gfx->transpen(bitmap, cliprect, code + 1, color, flipx, flipy, x, y - 16, 0);
			}
			else
			{
				// 16 x 16 mode
				gfx->transpen(bitmap, cliprect, code, color, flipx, flipy, x, y - 16, 0);
				gfx->transpen(bitmap, cliprect, code + 1, color, flipx, flipy, x, y, 0);
			}
		}
		else
		{
			gfx->transpen(bitmap, cliprect, code, color, flipx, flipy, x, y, 0);
		}
	}
}


template<int which>
uint32_t spdheat_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_fg_tilemap[which]->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, which & 1, (which & 2) >> 1);
	return 0;
}


/*************************************
 *
 *  Main memory map
 *
 *************************************/

void spdheat_state::main_map(address_map &map)
{
	map(0x000000, 0x02ffff).rom();
	map(0x080000, 0x083fff).ram();
	map(0x088000, 0x0887ff).ram().w(FUNC(spdheat_state::text_w<0>)).share("fg_ram0"); // IC11?
	map(0x088800, 0x088fff).ram().w(FUNC(spdheat_state::text_w<1>)).share("fg_ram1"); // IC27?
	map(0x08c000, 0x08c7ff).ram().w(FUNC(spdheat_state::text_w<2>)).share("fg_ram2"); // IC16?
	map(0x08c800, 0x08cfff).ram().w(FUNC(spdheat_state::text_w<3>)).share("fg_ram3"); // IC32?
	map(0x094000, 0x094001).rw(FUNC(spdheat_state::sound_status_r), FUNC(spdheat_state::sound_w<0>));
	map(0x094002, 0x094003).w(FUNC(spdheat_state::sound_w<1>));
	map(0x094004, 0x094005).w(FUNC(spdheat_state::sound_w<2>));
	map(0x094006, 0x094007).w(FUNC(spdheat_state::sound_w<3>));
	map(0x090000, 0x0907ff).ram().share("spriteram");
	map(0x098000, 0x0987ff).ram().w(m_palette0, FUNC(palette_device::write16)).share("palette0");
	map(0x099000, 0x0997ff).ram().w(m_palette1, FUNC(palette_device::write16)).share("palette1");
	map(0x09a000, 0x09a7ff).ram().w(m_palette2, FUNC(palette_device::write16)).share("palette2");
	map(0x09b000, 0x09b7ff).ram().w(m_palette3, FUNC(palette_device::write16)).share("palette3");
	map(0x09c000, 0x09c001).portr("HANDLE1");
	map(0x09c002, 0x09c003).portr("ACCEL1");
	map(0x09c004, 0x09c005).portr("HANDLE2");
	map(0x09c006, 0x09c007).portr("ACCEL2");
	map(0x09c008, 0x09c009).portr("HANDLE3");
	map(0x09c00a, 0x09c00b).portr("ACCEL3");
	map(0x09c00c, 0x09c00d).portr("HANDLE4");
	map(0x09c00e, 0x09c00f).portr("ACCEL4");
	map(0x09c010, 0x09c011).portr("IN0");
	map(0x09c012, 0x09c013).portr("IN1");
	map(0x09c014, 0x09c015).portr("SYSTEM");
	map(0x09c016, 0x09c017).portr("IN2");
	map(0x09c018, 0x09c019).portr("DSWA");
	map(0x09c01a, 0x09c01b).portr("DSWB");
	map(0x09c01c, 0x09c01d).portr("DSWC");
	map(0x09c01e, 0x09c01f).portr("DSWD");
	map(0x09c020, 0x09c021).nopw();
	map(0x09c022, 0x09c023).nopw();
	map(0x09c024, 0x09c025).nopw();
	map(0x09c026, 0x09c027).nopw();
}


/*************************************
 *
 *  Sound CPU memory map
 *
 *************************************/

void spdheat_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xd000, 0xd001).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xd100, 0xd101).rw("ym2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xd200, 0xd200).w("ym3", FUNC(ay8910_device::address_w));
	map(0xd201, 0xd201).rw("ym3", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xd300, 0xd300).w("ym4", FUNC(ay8910_device::address_w));
	map(0xd301, 0xd301).rw("ym4", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xd400, 0xd400).nopw(); // Filter?
	map(0xd420, 0xd420).nopw(); // Filter?
	map(0xd440, 0xd440).nopw(); // Filter?
	map(0xd460, 0xd460).nopw(); // Filter?
	map(0xd480, 0xd480).nopw(); // Filter?
	map(0xd4a0, 0xd4a0).nopw(); // Filter?
	map(0xd4e0, 0xd4e0).nopw(); // Filter?
	map(0xd500, 0xd500).nopw(); // Filter?
	map(0xd520, 0xd520).nopw(); // Filter?
	map(0xd540, 0xd540).nopw(); // Filter?
	map(0xd560, 0xd560).nopw(); // Filter?
	map(0xd580, 0xd580).nopw(); // Filter?
	map(0xd5a0, 0xd5a0).nopw(); // Filter?
	map(0xd5e0, 0xd5e0).nopw(); // Filter?
	map(0xd600, 0xd600).r(FUNC(spdheat_state::soundstatus_r));
	map(0xd700, 0xd700).ram(); // ? Read lots
	map(0xd800, 0xd800).rw(FUNC(spdheat_state::sndcpu_sound_r<0>), FUNC(spdheat_state::sub_nmi_w));
	map(0xd801, 0xd801).r(FUNC(spdheat_state::sndcpu_sound_r<1>));
	map(0xd802, 0xd802).r(FUNC(spdheat_state::sndcpu_sound_r<2>));
	map(0xd803, 0xd803).r(FUNC(spdheat_state::sndcpu_sound_r<3>));
	map(0xd807, 0xd807).r(FUNC(spdheat_state::sub_status_r));
	map(0xe000, 0xffff).rom(); // Diagnostic ROM
}


/*************************************
 *
 *  ADPCM CPU memory map
 *
 *************************************/

void spdheat_state::sub_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
	map(0x8000, 0x8000).w(FUNC(spdheat_state::sub_dac_w));
	map(0xf800, 0xffff).ram();
}

void spdheat_state::sub_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xff, 0xff).rw(FUNC(spdheat_state::sub_snd_r), FUNC(spdheat_state::sub_status_w));
}


/*************************************
 *
 *  Sound communications
 *
 *************************************/

uint16_t spdheat_state::sound_status_r()
{
	// 1s mean not busy?
	return m_sound_status;
}

template<int screen>
void spdheat_state::sound_w(uint16_t data)
{
	m_sound_data[screen] = data;
	m_sound_status &= ~(1 << screen);
}

void spdheat_state::sub_dac_w(uint8_t data)
{
	m_dac->write(data);
}

uint8_t spdheat_state::soundstatus_r()
{
	return m_sound_status ^ 0xf;
}

template<int screen>
uint8_t spdheat_state::sndcpu_sound_r()
{
	m_sound_status |= 1 << screen;
	return m_sound_data[screen];
}

uint8_t spdheat_state::sub_status_r()
{
	return m_sub_status ? 0x80 : 0;
}

uint8_t spdheat_state::sub_snd_r()
{
	return m_sub_data;
}

void spdheat_state::sub_status_w(uint8_t data)
{
	m_sub_status = 0;
}

// Write command to sub CPU
void spdheat_state::sub_nmi_w(uint8_t data)
{
	// Sub data is cleared by /NMI?
	m_sub_data = data;
	m_subcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	// It is busy
	m_sub_status = 1;
}


/*************************************
 *
 *  Sound I/O
 *
 *************************************/

/*

     YM2203 IC62
     ===========

     PORT A
     .... xxxx  PGA[3:0]
     xxxx ....  PC010SA IC58 'VR'

     PORT B
     .... xxxx  PC010SA IC58 'BASS'
     xxxx ....  PC010SA IC58 TREBL

     PSG OUTPUT -> PC010SA IC58   (PSGA)


     YM2203 IC61
     ===========

     PORT A
     .... xxxx  PGB[3:0]
     xxxx ....  PC010SA IC59 'VR'

     PORT B
     .... xxxx  PC010SA IC59 'BASS'
     xxxx ....  PC010SA IC59 TREBL

     PSG OUTPUT -> PC010SA IC59   (PSGB)


     YM2149 IC57
     ===========

     PORT A
     .... xxxx  PGC[3:0]
     xxxx ....  PC010SA IC54 'VR'

     PORT B
     .... xxxx  PC010SA IC54 'BASS'
     xxxx ....  PC010SA IC54 TREBL

     PSG OUTPUT -> PC010SA IC54   (PSGC)


     YM2149 IC56
     ===========

     PORT A
     .... xxxx  PGD[3:0]
     xxxx ....  PC010SA IC55 'VR'

     PORT B
     .... xxxx  PC010SA IC55 'BASS'
     xxxx ....  PC010SA IC55 TREBL

     PSG OUTPUT -> PC010SA IC55   (PSGD)


    PGC[3:0], PGD[3:0] = FMB BAL1
    PGA[3:0], PGB[3:0] = FMB VR1
 */

void spdheat_state::ym1_port_a_w(uint8_t data)
{

}

void spdheat_state::ym1_port_b_w(uint8_t data)
{

}

void spdheat_state::ym2_port_a_w(uint8_t data)
{

}

void spdheat_state::ym2_port_b_w(uint8_t data)
{

}

void spdheat_state::ym3_port_a_w(uint8_t data)
{

}

void spdheat_state::ym3_port_b_w(uint8_t data)
{

}

void spdheat_state::ym4_port_a_w(uint8_t data)
{

}

void spdheat_state::ym4_port_b_w(uint8_t data)
{

}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( spdheat )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_COIN4 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x0001, 0x0001, "Screen Stop" ) PORT_DIPLOCATION("DSWA:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Timer Stop" ) PORT_DIPLOCATION("DSWA:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("DSWA:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0008, IP_ACTIVE_LOW ) PORT_DIPLOCATION("DSWA:4")
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Color Bar Display" ) PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Round Skip (With Select Buttons)" ) PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "CPU Keeps Collected Upgrades Between Races" ) PORT_DIPLOCATION("DSWA:8") // "Functions of CPU-Controlled Cars"
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) ) // "Not Continuous"
	PORT_DIPSETTING(      0x0080, DEF_STR( Yes ) ) // "Continuous"

	PORT_START("DSWB")
	PORT_DIPNAME( 0x000F, 0x0000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSWB:1,2,3,4")
	PORT_DIPSETTING(      0x000F, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(      0x000E, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(      0x000D, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(      0x000C, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x000B, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x000A, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x0003, 0x0003, "Race Number of Setting Car Functions" ) PORT_DIPLOCATION("DSWC:1,2")
	PORT_DIPSETTING(      0x0003, "1st Race" )
	PORT_DIPSETTING(      0x0002, "2nd Race" )
	PORT_DIPSETTING(      0x0001, "3rd Race" )
	PORT_DIPSETTING(      0x0000, "4th Race" )
	PORT_DIPNAME( 0x000C, 0x0008, "Timer Setting" ) PORT_DIPLOCATION("DSWC:3,4")
	PORT_DIPSETTING(      0x000C, DEF_STR( Easy ) ) // 210 on initial race
	PORT_DIPSETTING(      0x0008, DEF_STR( Normal ) ) // 200
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) ) // 190
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) ) // 180
	PORT_DIPNAME( 0x0010, 0x0010, "Display Coins per Credit" ) PORT_DIPLOCATION("DSWC:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Language ) ) PORT_DIPLOCATION("DSWC:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSWC:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Coin System" ) PORT_DIPLOCATION("DSWC:8") // what does this do?
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSWD")
	PORT_DIPNAME( 0x0001, 0x0001, "Irrespective of Joining In Halfway" ) PORT_DIPLOCATION("DSWD:1")
	PORT_DIPSETTING(      0x0001, "Game Over After Final" )
	PORT_DIPSETTING(      0x0000, "All Races Playable" )
	PORT_DIPNAME( 0x0002, 0x0002, "Drag Race" ) PORT_DIPLOCATION("DSWD:2")
	PORT_DIPSETTING(      0x0002, "First One Only" )
	PORT_DIPSETTING(      0x0000, "Twice" )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSWD:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSWD:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSWD:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSWD:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSWD:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSWD:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_TOGGLE PORT_NAME("P1 Shift") PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED ) // if this is ACTIVE_HIGH gear in Service mode will always show low gear, but your start will be slow as if in high gear regardless of above
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("Reserved 2")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("Reserved 1")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_TOGGLE PORT_NAME("P2 Shift") PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED ) // see above
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_TOGGLE PORT_NAME("P3 Shift") PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED ) // see above
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("Reserved 3")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_TOGGLE PORT_NAME("P4 Shift") PORT_PLAYER(4)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED ) // see above
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE4 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START("HANDLE1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("ACCEL1")
	PORT_BIT( 0xff, 0x40, IPT_PEDAL ) PORT_MINMAX(0x40,0xFF) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)  PORT_PLAYER(1)

	PORT_START("HANDLE2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("ACCEL2")
	PORT_BIT( 0xff, 0x40, IPT_PEDAL ) PORT_MINMAX(0x40,0xFF) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)  PORT_PLAYER(2)

	PORT_START("HANDLE3")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(3)

	PORT_START("ACCEL3")
	PORT_BIT( 0xff, 0x40, IPT_PEDAL ) PORT_MINMAX(0x40,0xFF) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)  PORT_PLAYER(3)

	PORT_START("HANDLE4")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(4)

	PORT_START("ACCEL4")
	PORT_BIT( 0xff, 0x40, IPT_PEDAL ) PORT_MINMAX(0x40,0xFF) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)  PORT_PLAYER(4)
INPUT_PORTS_END


static INPUT_PORTS_START( spdheatj )
	PORT_INCLUDE( spdheat )

	PORT_MODIFY("DSWC")
	// Default to Japanese as all the speech is in Japanese
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Language ) ) PORT_DIPLOCATION("DSWC:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
INPUT_PORTS_END


/*************************************
 *
 *  GFX decoding
 *
 *************************************/

static const gfx_layout sprite_layout =
{
	16,16,    /* 16*16 characters */
	1024,   /* 1024 characters */
	4,      /* 4 bits per pixel */
	{ 3*1024*16*16, 2*1024*16*16, 1024*16*16, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	8*8*4 /* every char takes 32 consecutive bytes */
};

static const gfx_layout char_layout =
{
	8,8,    /* 8*8 characters */
	2048,   /* 2048 characters */
	4,      /* 4 bits per pixel */
	{ 3*2048*8*8, 2*2048*8*8, 2048*8*8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};


static GFXDECODE_START( gfx_spdheat )
	GFXDECODE_ENTRY( "gfx1", 0, char_layout, 256, 4 )
	GFXDECODE_ENTRY( "gfx2", 0, sprite_layout, 0, 16 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void spdheat_state::spdheat(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = 16_MHz_XTAL;
	constexpr XTAL SOUND_CLOCK = 4_MHz_XTAL;
	constexpr XTAL FM_CLOCK = SOUND_CLOCK / 2;

	/* basic machine hardware */
	M68000(config, m_maincpu, MASTER_CLOCK / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &spdheat_state::main_map);
	m_maincpu->set_vblank_int("screen0", FUNC(spdheat_state::irq1_line_hold));

	Z80(config, m_audiocpu, SOUND_CLOCK);
	m_audiocpu->set_addrmap(AS_PROGRAM, &spdheat_state::sound_map);

	Z80(config, m_subcpu, SOUND_CLOCK);
	m_subcpu->set_addrmap(AS_PROGRAM, &spdheat_state::sub_map);
	m_subcpu->set_addrmap(AS_IO, &spdheat_state::sub_io_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, m_palette0, gfx_spdheat); // TODO?
	PALETTE(config, m_palette0).set_format(palette_device::xBGR_555, 1024);
	PALETTE(config, m_palette1).set_format(palette_device::xBGR_555, 1024);
	PALETTE(config, m_palette2).set_format(palette_device::xBGR_555, 1024);
	PALETTE(config, m_palette3).set_format(palette_device::xBGR_555, 1024);
	config.set_default_layout(layout_spdheat);

	screen_device &screen0(SCREEN(config, "screen0", SCREEN_TYPE_RASTER));
	screen0.set_refresh_hz(60);
	screen0.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen0.set_size(256, 256);
	screen0.set_visarea(0, 256-1, 16, 256-16-1);
	screen0.set_screen_update(FUNC(spdheat_state::screen_update<0>));
	screen0.set_palette(m_palette0);

	screen_device &screen1(SCREEN(config, "screen1", SCREEN_TYPE_RASTER));
	screen1.set_refresh_hz(60);
	screen1.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen1.set_size(256, 256);
	screen1.set_visarea(0, 256-1, 16, 256-16-1);
	screen1.set_screen_update(FUNC(spdheat_state::screen_update<1>));
	screen1.set_palette(m_palette1);

	screen_device &screen2(SCREEN(config, "screen2", SCREEN_TYPE_RASTER));
	screen2.set_refresh_hz(60);
	screen2.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen2.set_size(256, 256);
	screen2.set_visarea(0, 256-1, 16, 256-16-1);
	screen2.set_screen_update(FUNC(spdheat_state::screen_update<2>));
	screen2.set_palette(m_palette2);

	screen_device &screen3(SCREEN(config, "screen3", SCREEN_TYPE_RASTER));
	screen3.set_refresh_hz(60);
	screen3.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen3.set_size(256, 256);
	screen3.set_visarea(0, 256-1, 16, 256-16-1);
	screen3.set_screen_update(FUNC(spdheat_state::screen_update<3>));
	screen3.set_palette(m_palette3);

	/* sound hardware */
	// TODO: there are multiple speakers
	SPEAKER(config, "mono").front_center();

	INPUT_MERGER_ANY_HIGH(config, m_audio_irq).output_handler().set_inputline(m_audiocpu, INPUT_LINE_IRQ0);

	ym2203_device &ym1(YM2203(config, "ym1", FM_CLOCK));
	ym1.irq_handler().set(m_audio_irq, FUNC(input_merger_any_high_device::in_w<0>));
	ym1.port_a_write_callback().set(FUNC(spdheat_state::ym1_port_a_w));
	ym1.port_b_write_callback().set(FUNC(spdheat_state::ym1_port_b_w));
	ym1.add_route(0, "mono", 0.3);
	ym1.add_route(1, "mono", 0.3);
	ym1.add_route(2, "mono", 0.3);
	ym1.add_route(3, "mono", 0.3);

	ym2203_device &ym2(YM2203(config, "ym2", FM_CLOCK));
	ym2.irq_handler().set(m_audio_irq, FUNC(input_merger_any_high_device::in_w<1>));
	ym2.port_a_write_callback().set(FUNC(spdheat_state::ym2_port_a_w));
	ym2.port_b_write_callback().set(FUNC(spdheat_state::ym2_port_b_w));
	ym2.add_route(0, "mono", 0.3);
	ym2.add_route(1, "mono", 0.3);
	ym2.add_route(2, "mono", 0.3);
	ym2.add_route(3, "mono", 0.3);

	ym2149_device &ym3(YM2149(config, "ym3", SOUND_CLOCK));
	ym3.port_a_write_callback().set(FUNC(spdheat_state::ym3_port_a_w));
	ym3.port_b_write_callback().set(FUNC(spdheat_state::ym3_port_b_w));
	ym3.add_route(ALL_OUTPUTS, "mono", 0.3);

	ym2149_device &ym4(YM2149(config, "ym4", SOUND_CLOCK));
	ym4.port_a_write_callback().set(FUNC(spdheat_state::ym4_port_a_w));
	ym4.port_b_write_callback().set(FUNC(spdheat_state::ym4_port_b_w));
	ym4.add_route(ALL_OUTPUTS, "mono", 0.3);

	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 0.3);
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( spdheat )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a55-04.ic37", 0x00000, 0x08000, CRC(bb1dd6b2) SHA1(f0d45f82e643fbc5cf6ec75fd8d7ae4c4b7d8b23) )
	ROM_LOAD16_BYTE( "a55-01.ic26", 0x00001, 0x08000, CRC(7ee12547) SHA1(79395cc622a315c3b3c3000224e150311da8073d) )
	ROM_LOAD16_BYTE( "a55-05.ic36", 0x10000, 0x08000, CRC(b396ef0b) SHA1(7a7f59faf4478c417f0928b22fb81462cb2628c2) )
	ROM_LOAD16_BYTE( "a55-02.ic25", 0x10001, 0x08000, CRC(4b76870a) SHA1(6bc1a594e37e2e3f2f0b4f23ba9bd7c87c3a27d9) )
	ROM_LOAD16_BYTE( "a55-06.ic34", 0x20000, 0x08000, CRC(900ecd44) SHA1(352b196bfe3b61cfbcdde72a03d3064be05be41e) )
	ROM_LOAD16_BYTE( "a55-03.ic23", 0x20001, 0x08000, CRC(6a5d2fe5) SHA1(595cc6028dbd0ceaf519b9a5eddfeffa74e2d27c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a55-17.ic11",  0x00000, 0x08000, CRC(43c2318f) SHA1(472e9cc68bb8ff3c5c3d4ec475491ad1a97261e7) )

	ROM_REGION( 0x10000, "subcpu", 0 ) // TODO: What are the correct labels for these?
	ROM_LOAD( "a55-15.ic5", 0x00000, 0x08000, CRC(c43b85ee) SHA1(7d7ed6b5f3e48a38b3e387f2dbc2f2bb0662db94) )
	ROM_LOAD( "a55-16.ic6", 0x08000, 0x08000, CRC(8f45edbd) SHA1(29a696691bd199b6fff0fe0e9fd9241cec9f3fbe) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "a55-07.ic6",   0x00000, 0x04000, CRC(cf85d3a0) SHA1(8ad330fd33b94b7bc0eb49edc4e5eafd2df54010) )
	ROM_LOAD( "a55-08.ic5",   0x04000, 0x04000, CRC(9ce4214d) SHA1(ea461d00af87bb0618604a02bd72338bcfb31f5b) )
	ROM_LOAD( "a55-09.ic22",  0x08000, 0x04000, CRC(00d7fba1) SHA1(ff0418856f469aa0b570c7a9c9af6cd3442e9b97) )
	ROM_LOAD( "a55-10.ic21",  0x0c000, 0x04000, CRC(743a04c5) SHA1(b878f4cdf1585eedddb8d18453474996a10b0804) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "a55-11.ic55",  0x00000, 0x08000, CRC(db979542) SHA1(a857e2ad12b07ccedd4453819fcb8f946893eedf) )
	ROM_LOAD( "a55-12.ic53",  0x08000, 0x08000, CRC(3d8211c2) SHA1(587caaf5775001a9aa2f266b3d084bd93fa0d575) )
	ROM_LOAD( "a55-13.ic52",  0x10000, 0x08000, CRC(38085e40) SHA1(5e4d6f9ce39a95bdddf5b2f4504fe3c34b5a8585) )
	ROM_LOAD( "a55-14.ic36",  0x18000, 0x08000, CRC(31c38779) SHA1(42ce3441a540644d17f27e84f8c5693cbee3e9f1) )
ROM_END


ROM_START( spdheatj )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a55-04.ic37", 0x00000, 0x08000, CRC(bb1dd6b2) SHA1(f0d45f82e643fbc5cf6ec75fd8d7ae4c4b7d8b23) )
	ROM_LOAD16_BYTE( "a55-01.ic26", 0x00001, 0x08000, CRC(7ee12547) SHA1(79395cc622a315c3b3c3000224e150311da8073d) )
	ROM_LOAD16_BYTE( "a55-05.ic36", 0x10000, 0x08000, CRC(b396ef0b) SHA1(7a7f59faf4478c417f0928b22fb81462cb2628c2) )
	ROM_LOAD16_BYTE( "a55-02.ic25", 0x10001, 0x08000, CRC(4b76870a) SHA1(6bc1a594e37e2e3f2f0b4f23ba9bd7c87c3a27d9) )
	ROM_LOAD16_BYTE( "a55-06.ic34", 0x20000, 0x08000, CRC(900ecd44) SHA1(352b196bfe3b61cfbcdde72a03d3064be05be41e) )
	ROM_LOAD16_BYTE( "a55-03.ic23", 0x20001, 0x08000, CRC(6a5d2fe5) SHA1(595cc6028dbd0ceaf519b9a5eddfeffa74e2d27c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a55-17.ic11",  0x00000, 0x08000, CRC(43c2318f) SHA1(472e9cc68bb8ff3c5c3d4ec475491ad1a97261e7) )

	ROM_REGION( 0x10000, "subcpu", 0 )
	ROM_LOAD( "a55-15.ic5",   0x00000, 0x08000, CRC(d076c1b7) SHA1(925390d09471f946b31a0a65da7d62eda4c05ec0) )
	ROM_LOAD( "a55-16.ic6",   0x08000, 0x08000, CRC(0b091bdb) SHA1(f86d2cc32eb8008b27035d7622f16ce7fb4daa61) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "a55-07.ic6",   0x00000, 0x04000, CRC(cf85d3a0) SHA1(8ad330fd33b94b7bc0eb49edc4e5eafd2df54010) )
	ROM_LOAD( "a55-08.ic5",   0x04000, 0x04000, CRC(9ce4214d) SHA1(ea461d00af87bb0618604a02bd72338bcfb31f5b) )
	ROM_LOAD( "a55-09.ic22",  0x08000, 0x04000, CRC(00d7fba1) SHA1(ff0418856f469aa0b570c7a9c9af6cd3442e9b97) )
	ROM_LOAD( "a55-10.ic21",  0x0c000, 0x04000, CRC(743a04c5) SHA1(b878f4cdf1585eedddb8d18453474996a10b0804) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "a55-11.ic55",  0x00000, 0x08000, CRC(db979542) SHA1(a857e2ad12b07ccedd4453819fcb8f946893eedf) )
	ROM_LOAD( "a55-12.ic53",  0x08000, 0x08000, CRC(3d8211c2) SHA1(587caaf5775001a9aa2f266b3d084bd93fa0d575) )
	ROM_LOAD( "a55-13.ic52",  0x10000, 0x08000, CRC(38085e40) SHA1(5e4d6f9ce39a95bdddf5b2f4504fe3c34b5a8585) )
	ROM_LOAD( "a55-14.ic36",  0x18000, 0x08000, CRC(31c38779) SHA1(42ce3441a540644d17f27e84f8c5693cbee3e9f1) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1985, spdheat,  0,       spdheat, spdheat,  spdheat_state, empty_init, ROT0, "Taito Corporation", "Super Dead Heat (World)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1985, spdheatj, spdheat, spdheat, spdheatj, spdheat_state, empty_init, ROT0, "Taito Corporation", "Super Dead Heat (Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
