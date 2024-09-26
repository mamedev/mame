// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Kitco Crowns Golf hardware

    driver by Aaron Giles

    Games supported:
        * Crowns Golf (4 sets)
        * Crowns Golf in Hawaii

    Known bugs:
        * not sure if the analog inputs are handled correctly

    Text Strings in sound CPU ROM read:
    ARIES ELECA
    1984JAN15 V-0

    Text Strings in the bootleg sound CPU ROM read:
    WHO AM I?      (In place of "ARIES ELECA")
    1984JULY1 V-1  (In place of "1984JAN15 V-0")
    1984 COPYRIGHT BY WHO

    2008-08
    Dip locations and factory settings verified with manual

    PAL16L8 @ E3 on cpu/sound board - Provided by Kevin Eshbach

    Pin 1  - Pin 21 of H1 (Z80 on cpu/sound board), Input
    Pin 2  - Ground, Not Used
    Pin 3  - Pin 9 of A4 (74LS367 on video board), Input
    Pin 4  - Pin 5 of E4 (TC40H000P on cpu/sound board), Input
    Pin 5  - Pin 2 of D2 (74LS174 on cpu/sound board), Input
    Pin 6  - Pin 5 of D2 (74LS174 on cpu/sound board), Input
    Pin 7  - Pin 31 of H1 (Z80 on cpu/sound board), Input
    Pin 8  - Pin 30 of H1 (Z80 on cpu/sound board), Input
    Pin 9  - Not Used
    Pin 10 - Ground
    Pin 11 - Not Used
    Pin 12 - Not Used
    Pin 13 - Pin 4 of D2 (74LS174 on cpu/sound board), Output
    Pin 14 - Pin 3 of D2 (74LS174 on cpu/sound board), Output
    Pin 15 - Pin 23 of B1 (AY-3-8910 on cpu/sound board), Output
    Pin 16 - Pin 11 of B4 (74LS04 on video board), Output
    Pin 17 - Pin 26 of H1 (Z80 on cpu/sound board), Output
    Pin 18 - Pin 3 of H3 (74LS74 on cpu/sound board), Output
    Pin 19 - Pin 22 of F1 (2764 on cpu/sound board), Output
    Pin 20 - VCC

    -------------------------------------------------------------------

    Master's Golf is a different PCB, but appears to operate in a similar way


            PCB X-081-PC-A

            contains a large box marked


        |-----------------------\_/--------------------|
        |                                   NASCO-9000 |
        |                                              |
        |                  /-  NASCO  -\               |
        |       /\         |  ORIGINAL |               |
        |  NASCO\/YUVO     \- 0001941 -/               |
        |                                              |
        |                      PAT.P                   |
        |   |---------------------------------------|  |
        |   |        MASTER'S GOLF vers JAPAN       |  |
        |   |                                       |  |
        |   |            CUSTOM BOARD               |  |
        |   |---------------------------------------|  |
        |                                              |
        |                 YUVO CO., LTD                |
        |-----------------------------------------------

         next to ROM M-GF_A10.12K
         the box must contain at least a Z80

DASM Notes:
- Main CPU currently stalls with a RAM buffer check ($63fe), then it
  tries to see if $612c onward has a "MASTERJ" string on it, resets itself
  otherwise.
- During irq routines it also checks if bit 7 is active for $640a-$6415,
  modifies this area if condition is true.
- Afterwards it seems to expect RAM "blitting" tasks at $6000-$63ff;
- Neither of above matches what we have in the ROM data banks, so something
  must provide those;
- Otherwise HW more or less matches base crgolf, including VRAM
  banking, screen enable/disable etc. It doesn't seem worth of a driver split;

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class crgolf_state : public driver_device
{
public:
	crgolf_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_palette(*this, "palette"),
		m_videoram(*this, "vram%u", 0U, 0x6000U, ENDIANNESS_LITTLE),
		m_mainbank(*this, "mainbank"),
		m_stick(*this, "STICK%u", 0U),
		m_ioport(*this, { "IN0", "IN1", "P1", "P2", "DSW", "UNUSED0", "UNUSED1" })
	{ }

	void crgolf(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<palette_device> m_palette;

	void sound_map(address_map &map) ATTR_COLD;

private:
	// memory pointers
	memory_share_array_creator<uint8_t, 2> m_videoram;
	required_memory_bank m_mainbank;

	required_ioport_array<2> m_stick;
	required_ioport_array<7> m_ioport;

	bool m_color_select = false;
	bool m_screen_flip = false;
	bool m_screen_enable[2] = { false, false };

	// misc
	uint8_t m_port_select = 0U;

	void rom_bank_select_w(uint8_t data);
	uint8_t switch_input_r();
	uint8_t analog_input_r();
	void switch_input_select_w(uint8_t data);
	void unknown_w(uint8_t data);
	void color_select_w(int state);
	void screen_flip_w(int state);
	void screen_select_w(int state);
	template <uint8_t Which> void screen_enable_w(int state);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
};

class crgolfhi_state : public crgolf_state
{
public:
	crgolfhi_state(const machine_config &mconfig, device_type type, const char *tag) :
		crgolf_state(mconfig, type, tag),
		m_adpcm_rom(*this, "adpcm"),
		m_msm(*this, "msm")
	{ }

	void crgolfhi(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// memory pointers
	required_region_ptr<uint8_t> m_adpcm_rom;

	// devices
	required_device<msm5205_device> m_msm;

	// misc
	uint16_t m_sample_offset = 0U;
	uint8_t m_sample_count = 0U;

	void sample_w(offs_t offset, uint8_t data);
	void vck_callback(int state);

	void sound_map(address_map &map) ATTR_COLD;
};

class mastrglf_state : public crgolfhi_state
{
public:
	mastrglf_state(const machine_config &mconfig, device_type type, const char *tag) :
		crgolfhi_state(mconfig, type, tag)
	{ }

	void mastrglf(machine_config &config);

private:
	uint8_t unk_sound_02_r();
	uint8_t unk_sound_05_r();
	uint8_t unk_sound_07_r();
	void unk_sound_0c_w(uint8_t data);
	void palette(palette_device &palette) const;
	void main_io_map(address_map &map) ATTR_COLD;
	void main_prg_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_prg_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Video startup
 *
 *************************************/

void crgolf_state::palette(palette_device &palette) const
{
	uint8_t const *const prom = memregion("proms")->base();
	static constexpr uint8_t NUM_PENS = 0x20;

	for (offs_t offs = 0; offs < NUM_PENS; offs++)
	{
		uint8_t const data = prom[offs];

		// red component
		int bit0 = BIT(data, 0);
		int bit1 = BIT(data, 1);
		int bit2 = BIT(data, 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(data, 3);
		bit1 = BIT(data, 4);
		bit2 = BIT(data, 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = BIT(data, 6);
		bit1 = BIT(data, 7);
		int const b = 0x4f * bit0 + 0xa8 * bit1;

		m_palette->set_pen_color(offs, r, g, b);
	}
}

void mastrglf_state::palette(palette_device &palette) const
{
	// TODO: once PROMs are dumped
}

/*************************************
 *
 *  Video update
 *
 *************************************/

uint32_t crgolf_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int flip = m_screen_flip;
	static constexpr uint16_t VIDEORAM_SIZE = 0x2000 * 3;

	// for each byte in the video RAM
	for (offs_t offs = 0; offs < VIDEORAM_SIZE / 3; offs++)
	{
		uint8_t y = (offs & 0x1fe0) >> 5;
		uint8_t x = (offs & 0x001f) << 3;

		uint8_t data_a0 = m_videoram[0][0x2000 | offs];
		uint8_t data_a1 = m_videoram[0][0x0000 | offs];
		uint8_t data_a2 = m_videoram[0][0x4000 | offs];
		uint8_t data_b0 = m_videoram[1][0x2000 | offs];
		uint8_t data_b1 = m_videoram[1][0x0000 | offs];
		uint8_t data_b2 = m_videoram[1][0x4000 | offs];

		if (flip)
		{
			y = ~y;
			x = ~x;
		}

		// for each pixel in the byte
		for (int i = 0; i < 8; i++)
		{
			offs_t color;
			uint8_t data_b = 0;
			uint8_t data_a = 0;

			if (!m_screen_enable[0])
				data_a = ((data_a0 & 0x80) >> 7) | ((data_a1 & 0x80) >> 6) | ((data_a2 & 0x80) >> 5);

			if (!m_screen_enable[1])
				data_b = ((data_b0 & 0x80) >> 7) | ((data_b1 & 0x80) >> 6) | ((data_b2 & 0x80) >> 5);

			// screen A has priority over B
			if (data_a)
				color = data_a;
			else
				color = data_b | 0x08;

			// add HI bit if enabled
			if (m_color_select)
				color = color | 0x10;

			bitmap.pix(y, x) = color;

			// next pixel
			data_a0 = data_a0 << 1;
			data_a1 = data_a1 << 1;
			data_a2 = data_a2 << 1;
			data_b0 = data_b0 << 1;
			data_b1 = data_b1 << 1;
			data_b2 = data_b2 << 1;

			if (flip)
				x = x - 1;
			else
				x = x + 1;
		}
	}

	return 0;
}


/*************************************
 *
 *  ROM banking
 *
 *************************************/

void crgolf_state::rom_bank_select_w(uint8_t data)
{
	m_mainbank->set_entry(data & 15); // TODO: mastrglf has more banks
}


void crgolf_state::machine_start()
{
	uint32_t size = memregion("maindata")->bytes();

	// configure the banking
	m_mainbank->configure_entries(0, size / 0x2000, memregion("maindata")->base(), 0x2000);
	m_mainbank->set_entry(0);
	membank("vrambank")->configure_entry(0, m_videoram[0]);
	membank("vrambank")->configure_entry(1, m_videoram[1]);

	// register for save states
	save_item(NAME(m_port_select));
	save_item(NAME(m_color_select));
	save_item(NAME(m_screen_flip));
	save_item(NAME(m_screen_enable));
}

void crgolfhi_state::machine_start()
{
	crgolf_state::machine_start();

	save_item(NAME(m_sample_offset));
	save_item(NAME(m_sample_count));
}

void crgolf_state::machine_reset()
{
	m_port_select = 0;
}

void crgolfhi_state::machine_reset()
{
	crgolf_state::machine_reset();

	m_sample_offset = 0;
	m_sample_count = 0;
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

uint8_t crgolf_state::switch_input_r()
{
	return m_ioport[m_port_select]->read();
}


uint8_t crgolf_state::analog_input_r()
{
	return ((m_stick[0]->read() >> 4) | (m_stick[1]->read() & 0xf0)) ^ 0x88;
}


void crgolf_state::switch_input_select_w(uint8_t data)
{
	for (int i = 6; i >= 0; i--)
		if (!(BIT(data, i)))
			m_port_select = i;
}


void crgolf_state::unknown_w(uint8_t data)
{
	logerror("%04X:unknown_w = %02X\n", m_audiocpu->pc(), data);
}



/*************************************
 *
 *  Hawaii auto-sample player
 *
 *************************************/

void crgolfhi_state::vck_callback(int state)
{
	// only play back if we have data remaining
	if (m_sample_count != 0xff)
	{
		uint8_t data = m_adpcm_rom[m_sample_offset >> 1];

		// write the next nibble and advance
		m_msm->data_w((data >> (4 * (~m_sample_offset & 1))) & 0x0f);
		m_sample_offset++;

		// every 256 clocks, we decrement the length
		if (!(m_sample_offset & 0xff))
		{
			m_sample_count--;

			// if we hit 0xff, automatically turn off playback
			if (m_sample_count == 0xff)
				m_msm->reset_w(1);
		}
	}
}


void crgolfhi_state::sample_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		// offset 0 holds the MSM5205 in reset
		case 0:
			m_msm->reset_w(1);
			break;

		// offset 1 is the length/256 nibbles
		case 1:
			m_sample_count = data;
			break;

		// offset 2 is the offset/256 nibbles
		case 2:
			m_sample_offset = data << 8;
			break;

		// offset 3 turns on playback
		case 3:
			m_msm->reset_w(0);
			break;
	}
}


void crgolf_state::color_select_w(int state)
{
	m_color_select = state;
}


void crgolf_state::screen_flip_w(int state)
{
	m_screen_flip = state;
}


template <uint8_t Which>
void crgolf_state::screen_enable_w(int state)
{
	m_screen_enable[Which] = state;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void crgolf_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x5fff).ram();
	map(0x6000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0x8007).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0x8800, 0x8800).r("soundlatch2", FUNC(generic_latch_8_device::read));
	map(0x8800, 0x8800).w("soundlatch1", FUNC(generic_latch_8_device::write));
	map(0x9000, 0x9000).w(FUNC(crgolf_state::rom_bank_select_w));
	map(0xa000, 0xffff).bankrw("vrambank");
}


/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

void crgolf_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xc000, 0xc001).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0xc002, 0xc002).nopw();
	map(0xe000, 0xe000).rw(FUNC(crgolf_state::switch_input_r), FUNC(crgolf_state::switch_input_select_w));
	map(0xe001, 0xe001).rw(FUNC(crgolf_state::analog_input_r), FUNC(crgolf_state::unknown_w));
	map(0xe003, 0xe003).r("soundlatch1", FUNC(generic_latch_8_device::read));
	map(0xe003, 0xe003).w("soundlatch2", FUNC(generic_latch_8_device::write));
}

void crgolfhi_state::sound_map(address_map &map)
{
	crgolf_state::sound_map(map);

	map(0xa000, 0xa003).w(FUNC(crgolfhi_state::sample_w));
}




void mastrglf_state::main_prg_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x5fff).bankr("mainbank");
	map(0x6000, 0x8fff).ram(); // maybe RAM and ROM here?
	map(0x9000, 0x9fff).ram();
	map(0xa000, 0xffff).bankrw("vrambank");

}


void mastrglf_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x07).w("mainlatch", FUNC(ls259_device::write_d0));
//  map(0x20, 0x20).w(FUNC(crgolf_state::rom_bank_select_w));
	map(0x40, 0x40).w("soundlatch1", FUNC(generic_latch_8_device::write));
	map(0xa0, 0xa0).r("soundlatch2", FUNC(generic_latch_8_device::read));
}



void mastrglf_state::sound_prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
}


uint8_t mastrglf_state::unk_sound_02_r()
{
	return 0x00;
}

uint8_t mastrglf_state::unk_sound_05_r()
{
	return 0x00;
}

uint8_t mastrglf_state::unk_sound_07_r()
{
	return 0x00;
}

void mastrglf_state::unk_sound_0c_w(uint8_t data)
{
}


void mastrglf_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("soundlatch1", FUNC(generic_latch_8_device::read)).nopw();
	map(0x02, 0x02).r(FUNC(mastrglf_state::unk_sound_02_r));
	map(0x05, 0x05).r(FUNC(mastrglf_state::unk_sound_05_r));
	map(0x06, 0x06).nopr();
	map(0x07, 0x07).r(FUNC(mastrglf_state::unk_sound_07_r));
	map(0x08, 0x08).w("soundlatch2", FUNC(generic_latch_8_device::write));
	map(0x0c, 0x0c).w(FUNC(mastrglf_state::unk_sound_0c_w));
	map(0x10, 0x11).w("aysnd", FUNC(ay8910_device::address_data_w));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( crgolf )
	PORT_START("IN0")   // CREDIT
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")   // SELECT
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START4 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1)            // club select
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)            // backward address
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)            // forward address
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)            // open stance
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1)            // closed stance
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)  // direction left
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) // direction right
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)            // shot switch

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_COCKTAIL     // club select
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL     // backward address
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL     // forward address
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_COCKTAIL     // open stance
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_COCKTAIL     // closed stance
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL   // direction left
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL  // direction right
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL     // shot switch

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x06, 0x04, "Half-Round Play" ) PORT_DIPLOCATION("SW:1,4")
	PORT_DIPSETTING(    0x00, "4 Credits" )
	PORT_DIPSETTING(    0x02, "5 Credits" )
	PORT_DIPSETTING(    0x04, "6 Credits" )
	PORT_DIPSETTING(    0x06, "10 Credits" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x10, 0x00, "Clear High Scores" ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW:8" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "SW:7" )

	PORT_START("UNUSED0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("UNUSED1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STICK0")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(16) PORT_REVERSE

	PORT_START("STICK1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(16) PORT_REVERSE PORT_COCKTAIL
INPUT_PORTS_END


static INPUT_PORTS_START(crgolfa)
	PORT_INCLUDE(crgolf)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x20, 0x20, "Price To Start" ) PORT_DIPLOCATION( "SW:5" )
	PORT_DIPSETTING( 0x20, "2 Credits" )
	PORT_DIPSETTING( 0x00, "1 Credit" )
INPUT_PORTS_END

static INPUT_PORTS_START(crgolfb)
	PORT_INCLUDE(crgolf)

	PORT_MODIFY("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW:5" )
	PORT_DIPNAME( 0x06, 0x04, "Half-Round Play" ) PORT_DIPLOCATION( "SW:1,4" )
	PORT_DIPSETTING( 0x00, "5 Credits" )
	PORT_DIPSETTING( 0x02, "8 Credits" )
	PORT_DIPSETTING( 0x04, "10 Credits" )
	PORT_DIPSETTING( 0x06, "15 Credits" )
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void crgolf_state::crgolf(machine_config &config)
{
	static constexpr XTAL MASTER_CLOCK = XTAL(18'432'000);

	// basic machine hardware
	Z80(config, m_maincpu, MASTER_CLOCK / 3 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &crgolf_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(crgolf_state::irq0_line_hold));

	Z80(config, m_audiocpu, MASTER_CLOCK / 3 / 2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &crgolf_state::sound_map);
	m_audiocpu->set_vblank_int("screen", FUNC(crgolf_state::irq0_line_hold));

	config.set_maximum_quantum(attotime::from_hz(6000));

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 1H
	mainlatch.q_out_cb<3>().set(FUNC(crgolf_state::color_select_w));
	mainlatch.q_out_cb<4>().set(FUNC(crgolf_state::screen_flip_w));
	mainlatch.q_out_cb<5>().set_membank("vrambank");
	mainlatch.q_out_cb<6>().set(FUNC(crgolf_state::screen_enable_w<1>));
	mainlatch.q_out_cb<7>().set(FUNC(crgolf_state::screen_enable_w<0>));

	GENERIC_LATCH_8(config, "soundlatch1").data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	GENERIC_LATCH_8(config, "soundlatch2").data_pending_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	PALETTE(config, m_palette, FUNC(crgolf_state::palette), 0x20);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(256, 256);
	screen.set_visarea(0, 255, 8, 247);
	screen.set_screen_update(FUNC(crgolf_state::screen_update));
	screen.set_palette(m_palette);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	AY8910(config, "aysnd", MASTER_CLOCK / 3 / 2 / 2).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void crgolfhi_state::crgolfhi(machine_config &config)
{
	crgolf(config);

	m_audiocpu->set_addrmap(AS_PROGRAM, &crgolfhi_state::sound_map);

	MSM5205(config, m_msm, 384000);
	m_msm->vck_legacy_callback().set(FUNC(crgolfhi_state::vck_callback));
	m_msm->set_prescaler_selector(msm5205_device::S64_4B);
	m_msm->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void mastrglf_state::mastrglf(machine_config &config)
{
	crgolfhi(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &mastrglf_state::main_prg_map);
	m_maincpu->set_addrmap(AS_IO, &mastrglf_state::main_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(mastrglf_state::irq0_line_hold));

	m_audiocpu->set_addrmap(AS_PROGRAM, &mastrglf_state::sound_prg_map);
	m_audiocpu->set_addrmap(AS_IO, &mastrglf_state::sound_io_map);
	m_audiocpu->set_vblank_int("screen", FUNC(mastrglf_state::irq0_line_hold));

	PALETTE(config.replace(), m_palette, FUNC(mastrglf_state::palette), 0x100);
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( crgolf ) // 834-5419-04
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "crwnc1.1c",  0x00000, 0x2000, CRC(3246e405) SHA1(f6018029317ac96df5866ca6a2bb2135edbd7e77) )
	ROM_LOAD( "crwna1.1a",  0x02000, 0x2000, CRC(b9a936e2) SHA1(cebf67d9c42627fbb39648674012a6cf8cb287b5) )

	ROM_REGION( 0x20000, "maindata", 0 )
	ROM_LOAD( "epr-5880.6b", 0x00000, 0x2000, CRC(4d6d8dad) SHA1(1530f81ad0097eadc75884ff8690b60b85ae451b) )
	ROM_LOAD( "epr-5885.5e", 0x0e000, 0x2000, CRC(fac6d56c) SHA1(67dc1918d5ab2443e967359e51d49dd134cdf25d) )
	ROM_LOAD( "epr-5881.6f", 0x10000, 0x2000, CRC(dd48dc1f) SHA1(d4560a88d872bd5f401344e3adb25f8486caca11) )
	ROM_LOAD( "epr-5886.5f", 0x12000, 0x2000, CRC(a09b27b8) SHA1(8b2d8322b633f6c7174bdb1fff0f6cef2d5a86de) )
	ROM_LOAD( "epr-5882.6h", 0x14000, 0x2000, CRC(fb86a168) SHA1(a679c9f50ac952da6c65f6593dce805023b8fc45) )
	ROM_LOAD( "epr-5887.5h", 0x16000, 0x2000, CRC(981f03ef) SHA1(42f686b970902bc42ac0f81bd2fc93dbdf766b1a) )
	ROM_LOAD( "epr-5883.6j", 0x18000, 0x2000, CRC(e64125ff) SHA1(ae2014d1039f4ed02c55053519bdeddd2f60a77a) )
	ROM_LOAD( "epr-5888.5j", 0x1a000, 0x2000, CRC(efc0e15a) SHA1(ba5772830f921004a2d9c90f557c04c799c755b9) )
	ROM_LOAD( "epr-5884.6k", 0x1c000, 0x2000, CRC(eb455966) SHA1(14278b598ac1d4007d5357cb40899c92a052417f) )
	ROM_LOAD( "epr-5889.5k", 0x1e000, 0x2000, CRC(88357391) SHA1(afdb5ed6555adf60bd64808413fc72fa5c67b6ec) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr-6198.1f",  0x0000, 0x1000, CRC(388c33d6) SHA1(42fd19c4b4ec7538d6c437552efb258bf2dcebc0) )
	ROM_LOAD( "epr-5892.1e",  0x2000, 0x2000, CRC(608dc2e2) SHA1(d906537cffd3e055f52f37a0490b3bb63107b2f9) )
	ROM_LOAD( "epr-5891a.1d", 0x4000, 0x2000, CRC(f353b585) SHA1(f09dcd0240131f872ceef5ddc9c89ab2fc92d117) )
	ROM_LOAD( "epr-5890a.1c", 0x6000, 0x2000, CRC(b737c2e8) SHA1(8596abbdff74300230b5ec5bf8acfe222eb3414f) )

	ROM_REGION( 0x0020,  "proms", 0 )
	ROM_LOAD( "pr5877.1s", 0x0000, 0x0020, CRC(f880b95d) SHA1(5ad0ee39e2b9befaf3895ec635d5865b7b1e562b) )

	ROM_REGION( 0x0200, "plds", 0 ) // pal16l8
	ROM_LOAD( "cg.3e.bin",  0x0000, 0x0104, CRC(beef5560) SHA1(cd7462dea015151cf29029e2275e10b949537cd2) ) // PAL is read protected
ROM_END

ROM_START( crgolfa ) // 834-5419-03
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "epr-6143.1c", 0x00000, 0x2000, CRC(4b301360) SHA1(2a7dd4876f4448b4b59b6dd02e55eb2d0126b777) )
	ROM_LOAD( "epr-6142.1a", 0x02000, 0x2000, CRC(8fc5e67f) SHA1(6563db94c55cfc7d2270daccaab57fc7b422b9f9) )

	ROM_REGION( 0x20000, "maindata", 0 )
	ROM_LOAD( "epr-5880.6b", 0x00000, 0x2000, CRC(4d6d8dad) SHA1(1530f81ad0097eadc75884ff8690b60b85ae451b) )
	ROM_LOAD( "epr-5885.5e", 0x0e000, 0x2000, CRC(fac6d56c) SHA1(67dc1918d5ab2443e967359e51d49dd134cdf25d) )
	ROM_LOAD( "epr-5881.6f", 0x10000, 0x2000, CRC(dd48dc1f) SHA1(d4560a88d872bd5f401344e3adb25f8486caca11) )
	ROM_LOAD( "epr-5886.5f", 0x12000, 0x2000, CRC(a09b27b8) SHA1(8b2d8322b633f6c7174bdb1fff0f6cef2d5a86de) )
	ROM_LOAD( "epr-5882.6h", 0x14000, 0x2000, CRC(fb86a168) SHA1(a679c9f50ac952da6c65f6593dce805023b8fc45) )
	ROM_LOAD( "epr-5887.5h", 0x16000, 0x2000, CRC(981f03ef) SHA1(42f686b970902bc42ac0f81bd2fc93dbdf766b1a) )
	ROM_LOAD( "epr-5883.6j", 0x18000, 0x2000, CRC(e64125ff) SHA1(ae2014d1039f4ed02c55053519bdeddd2f60a77a) )
	ROM_LOAD( "epr-5888.5j", 0x1a000, 0x2000, CRC(efc0e15a) SHA1(ba5772830f921004a2d9c90f557c04c799c755b9) )
	ROM_LOAD( "epr-5884.6k", 0x1c000, 0x2000, CRC(eb455966) SHA1(14278b598ac1d4007d5357cb40899c92a052417f) )
	ROM_LOAD( "epr-5889.5k", 0x1e000, 0x2000, CRC(88357391) SHA1(afdb5ed6555adf60bd64808413fc72fa5c67b6ec) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr-6144.1f",  0x0000, 0x1000, CRC(3fdc8cd6) SHA1(01d118d56a0e363af66a36ba583c4cbce86ee1d1) )
	ROM_LOAD( "epr-5892.1e",  0x2000, 0x2000, CRC(608dc2e2) SHA1(d906537cffd3e055f52f37a0490b3bb63107b2f9) )
	ROM_LOAD( "epr-5891a.1d", 0x4000, 0x2000, CRC(f353b585) SHA1(f09dcd0240131f872ceef5ddc9c89ab2fc92d117) )
	ROM_LOAD( "epr-5890a.1c", 0x6000, 0x2000, CRC(b737c2e8) SHA1(8596abbdff74300230b5ec5bf8acfe222eb3414f) )

	ROM_REGION( 0x0020,  "proms", 0 )
	ROM_LOAD( "pr5877.1s", 0x0000, 0x0020, CRC(f880b95d) SHA1(5ad0ee39e2b9befaf3895ec635d5865b7b1e562b) )

	ROM_REGION( 0x0200, "plds", 0 ) // pal16l8
	ROM_LOAD( "cg.3e.bin",  0x0000, 0x0104, CRC(beef5560) SHA1(cd7462dea015151cf29029e2275e10b949537cd2) ) // PAL is read protected
ROM_END


ROM_START( crgolfb )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "epr-5879b.1c", 0x00000, 0x2000, CRC(927be359) SHA1(d534f7e3ef4ced8eea882ae2b8425df4c5842833) ) // 5879b.
	ROM_LOAD( "epr-5878.1a",  0x02000, 0x2000, CRC(65fd0fa0) SHA1(de95ff95c9f981cd9eadf8b028ee5373bc69007b) ) // 5878.

	ROM_REGION( 0x20000, "maindata", 0 )
	ROM_LOAD( "epr-5880.6b",  0x00000, 0x2000, CRC(4d6d8dad) SHA1(1530f81ad0097eadc75884ff8690b60b85ae451b) ) // crnsgolf.c
	ROM_LOAD( "epr-5885.5e",  0x0e000, 0x2000, CRC(fac6d56c) SHA1(67dc1918d5ab2443e967359e51d49dd134cdf25d) ) // crnsgolf.h
	ROM_LOAD( "epr-5881.6f",  0x10000, 0x2000, CRC(dd48dc1f) SHA1(d4560a88d872bd5f401344e3adb25f8486caca11) ) // crnsgolf.d
	ROM_LOAD( "epr-5886.5f",  0x12000, 0x2000, CRC(a09b27b8) SHA1(8b2d8322b633f6c7174bdb1fff0f6cef2d5a86de) ) // crnsgolf.i
	ROM_LOAD( "epr-5882.6h",  0x14000, 0x2000, CRC(fb86a168) SHA1(a679c9f50ac952da6c65f6593dce805023b8fc45) ) // crnsgolf.e
	ROM_LOAD( "epr-5887.5h",  0x16000, 0x2000, CRC(981f03ef) SHA1(42f686b970902bc42ac0f81bd2fc93dbdf766b1a) ) // crnsgolf.j
	ROM_LOAD( "epr-5883.6j",  0x18000, 0x2000, CRC(e64125ff) SHA1(ae2014d1039f4ed02c55053519bdeddd2f60a77a) ) // crnsgolf.f
	ROM_LOAD( "epr-5888.5j",  0x1a000, 0x2000, CRC(efc0e15a) SHA1(ba5772830f921004a2d9c90f557c04c799c755b9) ) // crnsgolf.k
	ROM_LOAD( "epr-5884.6k",  0x1c000, 0x2000, CRC(eb455966) SHA1(14278b598ac1d4007d5357cb40899c92a052417f) ) // crnsgolf.g
	ROM_LOAD( "epr-5889.5k",  0x1e000, 0x2000, CRC(88357391) SHA1(afdb5ed6555adf60bd64808413fc72fa5c67b6ec) ) // crnsgolf.l

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr-5893c.1f", 0x0000, 0x1000, CRC(5011646d) SHA1(1bbf83107396d69c17580d4b1b38d93f741a608f) ) // 5893c.
	ROM_LOAD( "epr-5892.1e",  0x2000, 0x2000, CRC(608dc2e2) SHA1(d906537cffd3e055f52f37a0490b3bb63107b2f9) ) // 5892.
	ROM_LOAD( "epr-5891a.1d", 0x4000, 0x2000, CRC(f353b585) SHA1(f09dcd0240131f872ceef5ddc9c89ab2fc92d117) ) // 5890a.
	ROM_LOAD( "epr-5890a.1c", 0x6000, 0x2000, CRC(b737c2e8) SHA1(8596abbdff74300230b5ec5bf8acfe222eb3414f) ) // 5891a.

	ROM_REGION( 0x0020,  "proms", 0 )
	ROM_LOAD( "pr5877.1s",   0x0000, 0x0020, CRC(f880b95d) SHA1(5ad0ee39e2b9befaf3895ec635d5865b7b1e562b) ) // golfprom.
ROM_END


ROM_START( crgolfc )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "15.1a",      0x00000, 0x2000, CRC(e6194356) SHA1(78eec53a0658b552e6a8af109d9c9754e4ddadcb) )
	ROM_LOAD( "16.1c",      0x02000, 0x2000, CRC(f50412e2) SHA1(5a50fb1edfc26072e921447bd157fe996f707e05) )

	ROM_REGION( 0x20000, "maindata", 0 )
	ROM_LOAD( "cg.1",        0x00000, 0x2000, CRC(ad7d537a) SHA1(deff74074a8b16ea91a0fa72d97ec36336c87b97) ) //  1.6a
	ROM_LOAD( "epr-5885.5e", 0x0e000, 0x2000, CRC(fac6d56c) SHA1(67dc1918d5ab2443e967359e51d49dd134cdf25d) ) //  6.5a
	ROM_LOAD( "epr-5881.6f", 0x10000, 0x2000, CRC(dd48dc1f) SHA1(d4560a88d872bd5f401344e3adb25f8486caca11) ) //  2.6b
	ROM_LOAD( "epr-5886.5f", 0x12000, 0x2000, CRC(a09b27b8) SHA1(8b2d8322b633f6c7174bdb1fff0f6cef2d5a86de) ) //  7.5b
	ROM_LOAD( "3.6c",        0x14000, 0x2000, CRC(b7fcee1a) SHA1(47e9a2cee945c5f59490b73c475ec2512ea0f559) )
	ROM_LOAD( "epr-5887.5h", 0x16000, 0x2000, CRC(981f03ef) SHA1(42f686b970902bc42ac0f81bd2fc93dbdf766b1a) ) //  8.5c
	ROM_LOAD( "epr-5883.6j", 0x18000, 0x2000, CRC(e64125ff) SHA1(ae2014d1039f4ed02c55053519bdeddd2f60a77a) ) //  4.6d
	ROM_LOAD( "epr-5888.5j", 0x1a000, 0x2000, CRC(efc0e15a) SHA1(ba5772830f921004a2d9c90f557c04c799c755b9) ) //  9.5d
	ROM_LOAD( "epr-5884.6k", 0x1c000, 0x2000, CRC(eb455966) SHA1(14278b598ac1d4007d5357cb40899c92a052417f) ) //  5.6e
	ROM_LOAD( "epr-5889.5k", 0x1e000, 0x2000, CRC(88357391) SHA1(afdb5ed6555adf60bd64808413fc72fa5c67b6ec) ) // 10.5e

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "11.1e",       0x0000, 0x1000, CRC(53295a1a) SHA1(ec6c4df9f32e4b3ffe48e823d90a9e6a671e6027) )
	ROM_LOAD( "epr-5892.1e",  0x2000, 0x2000, CRC(608dc2e2) SHA1(d906537cffd3e055f52f37a0490b3bb63107b2f9) ) // 12.1d
	ROM_LOAD( "epr-5891a.1d", 0x4000, 0x2000, CRC(f353b585) SHA1(f09dcd0240131f872ceef5ddc9c89ab2fc92d117) ) // 13.1c
	ROM_LOAD( "epr-5890a.1c", 0x6000, 0x2000, CRC(b737c2e8) SHA1(8596abbdff74300230b5ec5bf8acfe222eb3414f) ) // 14.1b

	ROM_REGION( 0x0020,  "proms", 0 )
	ROM_LOAD( "pr5877.1s", 0x0000, 0x0020, CRC(f880b95d) SHA1(5ad0ee39e2b9befaf3895ec635d5865b7b1e562b) )
ROM_END


ROM_START( crgolfbt )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "epr-5879b.1c", 0x00000, 0x2000, CRC(927be359) SHA1(d534f7e3ef4ced8eea882ae2b8425df4c5842833) )
	ROM_LOAD( "epr-5878.1a",  0x02000, 0x2000, CRC(65fd0fa0) SHA1(de95ff95c9f981cd9eadf8b028ee5373bc69007b) )

	ROM_REGION( 0x20000, "maindata", 0 )
	ROM_LOAD( "cg.1",         0x00000, 0x2000, CRC(ad7d537a) SHA1(deff74074a8b16ea91a0fa72d97ec36336c87b97) )
	ROM_LOAD( "epr-5885.5e",  0x0e000, 0x2000, CRC(fac6d56c) SHA1(67dc1918d5ab2443e967359e51d49dd134cdf25d) ) // cg.6
	ROM_LOAD( "epr-5881.6f",  0x10000, 0x2000, CRC(dd48dc1f) SHA1(d4560a88d872bd5f401344e3adb25f8486caca11) ) // cg.2
	ROM_LOAD( "epr-5886.5f",  0x12000, 0x2000, CRC(a09b27b8) SHA1(8b2d8322b633f6c7174bdb1fff0f6cef2d5a86de) ) // cg.7
	ROM_LOAD( "epr-5882.6h",  0x14000, 0x2000, CRC(fb86a168) SHA1(a679c9f50ac952da6c65f6593dce805023b8fc45) ) // cg.3
	ROM_LOAD( "epr-5887.5h",  0x16000, 0x2000, CRC(981f03ef) SHA1(42f686b970902bc42ac0f81bd2fc93dbdf766b1a) ) // cg.8
	ROM_LOAD( "epr-5883.6j",  0x18000, 0x2000, CRC(e64125ff) SHA1(ae2014d1039f4ed02c55053519bdeddd2f60a77a) ) // cg.4
	ROM_LOAD( "epr-5888.5j",  0x1a000, 0x2000, CRC(efc0e15a) SHA1(ba5772830f921004a2d9c90f557c04c799c755b9) ) // cg.9
	ROM_LOAD( "epr-5884.6k",  0x1c000, 0x2000, CRC(eb455966) SHA1(14278b598ac1d4007d5357cb40899c92a052417f) ) // cg.5
	ROM_LOAD( "epr-5889.5k",  0x1e000, 0x2000, CRC(88357391) SHA1(afdb5ed6555adf60bd64808413fc72fa5c67b6ec) ) // cg.10

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cg.14",       0x0000, 0x1000, CRC(07156cd9) SHA1(8907cf9228d6de117b24969d4e039cee330f9b1e) )
	ROM_LOAD( "epr-5892.1e",  0x2000, 0x2000, CRC(608dc2e2) SHA1(d906537cffd3e055f52f37a0490b3bb63107b2f9) ) // cg.13
	ROM_LOAD( "epr-5891a.1d", 0x4000, 0x2000, CRC(f353b585) SHA1(f09dcd0240131f872ceef5ddc9c89ab2fc92d117) ) // cg.12
	ROM_LOAD( "epr-5890a.1c", 0x6000, 0x2000, CRC(b737c2e8) SHA1(8596abbdff74300230b5ec5bf8acfe222eb3414f) ) // cg.11

	ROM_REGION( 0x0020,  "proms", 0 )
	ROM_LOAD( "pr5877.1s",   0x0000, 0x0020, CRC(f880b95d) SHA1(5ad0ee39e2b9befaf3895ec635d5865b7b1e562b) )
ROM_END


ROM_START( crgolfhi )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "cpu.c1",  0x00000, 0x2000, CRC(8b101085) SHA1(a59c369be3e7e645d8b20032998a778a2056b7d7) )
	ROM_LOAD( "cpu.a1",  0x02000, 0x2000, CRC(f48a8ee8) SHA1(cc07c7258caf251e9cb52f12be779cb02fca0b0a) )

	ROM_REGION( 0x20000, "maindata", 0 )
	ROM_LOAD( "main.b6", 0x00000, 0x2000, CRC(5b0336c6) SHA1(86e2c197f23a2f2f7666448b74611150ca15a2af) )
	ROM_LOAD( "main.b5", 0x02000, 0x2000, CRC(7b80149a) SHA1(c802a79b1430b15d166f5fca11d2ed4e65bc65a9) )
	ROM_LOAD( "main.c6", 0x04000, 0x2000, CRC(7804cb1c) SHA1(487f979f47a0f40fa35331c71a66dc8428387a26) )
	ROM_LOAD( "main.c5", 0x06000, 0x2000, CRC(7721efc5) SHA1(9f3fb6845e5815ada1535da7800e175769fd46b1) )
	ROM_LOAD( "main.d6", 0x08000, 0x2000, CRC(f3ccdfaa) SHA1(c266737caf7222a971d0297b944c5710d3ec12be) )
	ROM_LOAD( "main.d5", 0x0a000, 0x2000, CRC(bef85c95) SHA1(516615975207209a4c649df7ffd451167fc40c45) )
	ROM_LOAD( "main.e6", 0x0c000, 0x2000, CRC(aa75e849) SHA1(226e7712e65f86422a1caebf3b95abcf39af2277) )
	ROM_LOAD( "main.e5", 0x0e000, 0x2000, CRC(e8eefbc4) SHA1(02393d3c0a1234ec51348d755725562cc7861285) )
	ROM_LOAD( "main.f6", 0x10000, 0x2000, CRC(e1130eec) SHA1(26a68f8af543983fcae73db59d075b11ee101ca8) )
	ROM_LOAD( "main.f5", 0x12000, 0x2000, CRC(090c21e3) SHA1(e5e0fc1e4ffd2a9c344cfc70a9e8e7cebb0821cc) )
	ROM_LOAD( "main.h6", 0x14000, 0x2000, CRC(33b8ada4) SHA1(73192108daa0724c30c1deea7d52538a49bfdf8f) )
	ROM_LOAD( "main.h5", 0x16000, 0x2000, CRC(16e5a26c) SHA1(7bb6e5d852f352331953058c17e753fee04d1cf9) )
	ROM_LOAD( "main.j6", 0x18000, 0x2000, CRC(22db8cce) SHA1(cd646830129bfdd2f5f10c8f6732e76f8a15b74f) )
	ROM_LOAD( "main.j5", 0x1a000, 0x2000, CRC(f757de30) SHA1(38330f10051735683f41ed425900b9f0f9ee01be) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "main.f1",  0x0000, 0x2000, CRC(e7c471de) SHA1(b953807bc714496363ca33ad0fc11a2d30aa7b7e) )

	ROM_REGION( 0x8000, "adpcm", 0 )
	ROM_LOAD( "sub.r1", 0x0000, 0x2000, CRC(9be85e38) SHA1(a108fe812d0518e7bef32fd76998c0c70b70723e) )
	ROM_LOAD( "sub.r2", 0x2000, 0x2000, CRC(d65b8e3a) SHA1(de6acffbe2d7078f0598857a6a3b2179e5c82a34) )
	ROM_LOAD( "sub.r3", 0x4000, 0x2000, CRC(65967250) SHA1(7620560ea57b8e5d259ea8881fb8d8ca46228014) )
	ROM_LOAD( "sub.r4", 0x6000, 0x2000, CRC(d3716776) SHA1(7e38437d255c5f28aac24f0943c10fc1ce998b60) )

	ROM_REGION( 0x0020,  "proms", 0 )
	ROM_LOAD( "prom.s1", 0x0000, 0x0020, CRC(014427df) SHA1(85a5e660f9667e032b80152bbde351007e5c88df) )
ROM_END


ROM_START( mastrglf )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "m-gf_a1.4a.27128", 0x00000, 0x04000, CRC(55b89e8f) SHA1(2860fd3f8e4241dc25bb9a14e8967cdcaf769432) )

	ROM_REGION( 0x30000, "maindata", 0 ) // 24 (0x18) * 0x2000 banks
	ROM_LOAD( "m-gf_a2.5a.27256.rom5",   0x00000, 0x08000, CRC(98aa20d8) SHA1(64007c4706f8e2e3b57c4a8467b37d44e8be9a01) )
	ROM_LOAD( "m-gf_a3.7a.27256.rom4",   0x08000, 0x08000, CRC(3f62b979) SHA1(90cc784230f6ed7fd3dd943e0808f0c3d722806a) )
	ROM_LOAD( "m-gf_a4.8a.27256.rom3",   0x10000, 0x08000, CRC(08a470d1) SHA1(4dabff8fc915406b1d4f7936d925378eec0df915) )
	ROM_LOAD( "m-gf_a5.10a.27256.rom2",  0x18000, 0x08000, CRC(4397c8a0) SHA1(deb9de1cf7ce6ddc69addf18ff5bf2f25ed11602) )
	ROM_LOAD( "m-gf_a6.12a.27256.rom1",  0x20000, 0x08000, CRC(b1fccecf) SHA1(8fb5e40f34596d9faa73255afc2c2635e9008954) )
	ROM_LOAD( "m-gf_a7.13a.27256.rom0",  0x28000, 0x08000, CRC(06075e41) SHA1(3426f4ede8449288519e25bc8a1d679bb5137279) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // next to large module
	ROM_LOAD( "m-gf_a10.12k.27256", 0x00000, 0x08000, CRC(d145b144) SHA1(52370d56106f0280c52266b5a727493a3396a8e3) )

	ROM_REGION( 0x4000, "mcu", 0 ) // unknown part/device, living inside the epoxy blob
	ROM_LOAD( "epoxy.bin", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x10000, "adpcm", 0 ) // MSM5205 samples
	ROM_LOAD( "m-gf_a8.15a.27256",  0x00000, 0x08000, CRC(9ea9183b) SHA1(55f54575cd662b6194f69532baa25c9b2272760f) )
	ROM_LOAD( "m-gf_a9.16a.27256",  0x08000, 0x08000, CRC(61ab715f) SHA1(6b9cccaa83a9a9e44a46bae796e2f9eaa9f9c951) )

	ROM_REGION( 0x0300,  "proms", 0 )
	ROM_LOAD( "tbp24s10n.1", 0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "tbp24s10n.2", 0x0100, 0x0100, NO_DUMP )
	ROM_LOAD( "tbp24s10n.2", 0x0200, 0x0100, NO_DUMP )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1984, crgolf,   0,      crgolf,   crgolf,  crgolf_state,   empty_init, ROT0, "Nasco Japan", "Crowns Golf (834-5419-04)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, crgolfa,  crgolf, crgolf,   crgolfa, crgolf_state,   empty_init, ROT0, "Nasco Japan", "Crowns Golf (834-5419-03)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, crgolfb,  crgolf, crgolf,   crgolfb, crgolf_state,   empty_init, ROT0, "Nasco Japan", "Crowns Golf (set 3)",       MACHINE_SUPPORTS_SAVE )
GAME( 1984, crgolfc,  crgolf, crgolf,   crgolfb, crgolf_state,   empty_init, ROT0, "Nasco Japan", "Champion Golf",             MACHINE_SUPPORTS_SAVE )
GAME( 1984, crgolfbt, crgolf, crgolf,   crgolfb, crgolf_state,   empty_init, ROT0, "bootleg",     "Champion Golf (bootleg)",   MACHINE_SUPPORTS_SAVE )
GAME( 1985, crgolfhi, 0,      crgolfhi, crgolfa, crgolfhi_state, empty_init, ROT0, "Nasco Japan", "Crowns Golf in Hawaii",     MACHINE_SUPPORTS_SAVE )

GAME( 1985, mastrglf, 0,      mastrglf, crgolf,  mastrglf_state, empty_init, ROT0, "Nasco",       "Master's Golf",             MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION ) // shared RAM with an undumped device or CPU, cfr. notes on top
