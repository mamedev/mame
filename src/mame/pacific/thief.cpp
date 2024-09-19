// license:BSD-3-Clause
// copyright-holders: Victor Trucco, Mike Balfour, Phil Stroffolino

/******************************************************************

Shark Attack
(C) 1980 PACIFIC NOVELTY MFG. INC.

Thief
(C) 1981 PACIFIC NOVELTY MFG. INC.

NATO Defense
(C) 1982 PACIFIC NOVELTY MFG. INC.

Credits:
    Shark Driver by Victor Trucco and Mike Balfour
    Driver for Thief and NATO Defense by Phil Stroffolino

- minor blitting glitches in playfield of Thief (XOR vs copy?)

- Nato Defense gfx ROMs may be hooked up wrong;
    see screenshots from flyers

- Coprocessor needs identification and actual emulation

******************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "sound/samples.h"
#include "video/tms9927.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class sharkatt_state : public driver_device
{
public:
	sharkatt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_screen(*this, "screen"),
		m_tms(*this, "tms"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram", 0x2000 * 4 * 2, ENDIANNESS_LITTLE),
		m_ioport(*this, { "DSW1", "DSW2", "P1", "P2" })
	{ }

	void sharkatt(machine_config &config);

	void slam_w(int state);

protected:
	virtual void video_start() override ATTR_COLD;

	uint8_t videoram_r(offs_t offset);
	void videoram_w(offs_t offset, uint8_t data);

	void main_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;
	required_device<screen_device> m_screen;

private:
	required_device<tms9927_device> m_tms;
	required_device<palette_device> m_palette;

	memory_share_creator<uint8_t> m_videoram;

	required_ioport_array<4> m_ioport;

	uint8_t m_input_select = 0;
	uint8_t m_read_mask = 0;
	uint8_t m_write_mask = 0;
	uint8_t m_video_control = 0;

	void input_select_w(uint8_t data);
	uint8_t io_r();
	void video_control_w(uint8_t data);
	void color_map_w(offs_t offset, uint8_t data);
	void color_plane_w(uint8_t data);
	void tape_control_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	IRQ_CALLBACK_MEMBER(iack);
	void tape_set_audio(int track, int bon);
	void tape_set_motor(int bon);

	void io_map(address_map &map) ATTR_COLD;
};

class thief_state : public sharkatt_state
{
public:
	thief_state(const machine_config &mconfig, device_type type, const char *tag) :
		sharkatt_state(mconfig, type, tag) ,
		m_blitrom(*this, "blitter"),
		m_coprocessor(*this)
	{ }

	void natodef(machine_config &config);
	void thief(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_region_ptr<uint8_t> m_blitrom;

	struct coprocessor
	{
		coprocessor(device_t &host) :
			context_ram(host, "context_ram", 0x400, ENDIANNESS_LITTLE),
			bank(0),
			image_ram(host, "image_ram", 0x2000, ENDIANNESS_LITTLE),
			param{}
		{ }

		memory_share_creator<uint8_t> context_ram;
		uint8_t bank;
		memory_share_creator<uint8_t> image_ram;
		uint8_t param[0x9];
	};

	coprocessor m_coprocessor;

	uint8_t context_ram_r(offs_t offset);
	void context_ram_w(offs_t offset, uint8_t data);
	void context_bank_w(uint8_t data);
	void blit_w(uint8_t data);
	uint8_t coprocessor_r(offs_t offset);
	void coprocessor_w(offs_t offset, uint8_t data);

	uint16_t fetch_image_addr();

	void main_map(address_map &map) ATTR_COLD;
};


enum {
	IMAGE_ADDR_LO,      //0xe000
	IMAGE_ADDR_HI,      //0xe001
	SCREEN_XPOS,        //0xe002
	SCREEN_YPOS,        //0xe003
	BLIT_WIDTH,         //0xe004
	BLIT_HEIGHT,        //0xe005
	GFX_PORT,           //0xe006
	BARL_PORT,          //0xe007
	BLIT_ATTRIBUTES     //0xe008
};

/***************************************************************************/

uint8_t thief_state::context_ram_r(offs_t offset)
{
	return m_coprocessor.context_ram[0x40 * m_coprocessor.bank + offset];
}

void thief_state::context_ram_w(offs_t offset, uint8_t data)
{
	m_coprocessor.context_ram[0x40 * m_coprocessor.bank + offset] = data;
}

void thief_state::context_bank_w(uint8_t data)
{
	m_coprocessor.bank = data & 0xf;
}

/***************************************************************************/

void sharkatt_state::video_control_w(uint8_t data)
{
	m_video_control = data;
/*
    bit 0: screen flip
    bit 1: working page
    bit 2: visible page
    bit 3: mirrors bit 1
    bit 4: mirrors bit 2
*/
}

void sharkatt_state::color_map_w(offs_t offset, uint8_t data)
{
/*
    --xx----    blue
    ----xx--    green
    ------xx    red
*/
	static const uint8_t intensity[4] = {0x00, 0x55, 0xaa, 0xff};
	int const r = intensity[(data & 0x03) >> 0];
	int const g = intensity[(data & 0x0c) >> 2];
	int const b = intensity[(data & 0x30) >> 4];

	m_palette->set_pen_color(offset, rgb_t(r, g, b));
}

/***************************************************************************/

void sharkatt_state::color_plane_w(uint8_t data)
{
/*
    --xx----    selects bitplane to read from (0..3)
    ----xxxx    selects bitplane(s) to write to (0x0 = none, 0xf = all)
*/
	m_write_mask = data & 0xf;
	m_read_mask = (data >> 4) & 3;
}

uint8_t sharkatt_state::videoram_r(offs_t offset)
{
	uint8_t *source = &m_videoram[offset];

	if (m_video_control & 0x02)
		source += 0x2000 * 4; // foreground / background

	return source[m_read_mask * 0x2000];
}

void sharkatt_state::videoram_w(offs_t offset, uint8_t data)
{
	uint8_t *dest = &m_videoram[offset];

	if (m_video_control & 0x02)
		dest += 0x2000 * 4; // foreground / background

	for (int i = 0; i < 4; i++)
		if (BIT(m_write_mask, i))
			dest[0x2000 * i] = data;
}

/***************************************************************************/

void sharkatt_state::video_start()
{
	save_item(NAME(m_input_select));
	save_item(NAME(m_read_mask));
	save_item(NAME(m_write_mask));
	save_item(NAME(m_video_control));
}

void thief_state::video_start()
{
	sharkatt_state::video_start();

	save_item(NAME(m_coprocessor.bank));
	save_item(NAME(m_coprocessor.param));
}

uint32_t sharkatt_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int const flipscreen = m_video_control & 1;
	const uint8_t *source = m_videoram.target();

	if (m_tms->screen_reset())
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	if (m_video_control & 4) // visible page
		source += 0x2000 * 4;

	for (uint32_t offs = 0; offs < 0x2000; offs++)
	{
		int const ypos = offs / 32;
		int const xpos = (offs % 32) * 8;
		int const plane0 = source[0x2000 * 0 + offs];
		int const plane1 = source[0x2000 * 1 + offs];
		int const plane2 = source[0x2000 * 2 + offs];
		int const plane3 = source[0x2000 * 3 + offs];
		if (flipscreen)
		{
			for (int bit = 0; bit < 8; bit++)
			{
				bitmap.pix(0xff - ypos, 0xff - (xpos + bit)) =
						(((plane0 << bit) & 0x80) >> 7) |
						(((plane1 << bit) & 0x80) >> 6) |
						(((plane2 << bit) & 0x80) >> 5) |
						(((plane3 << bit) & 0x80) >> 4);
			}
		}
		else
		{
			for (int bit = 0; bit < 8; bit++)
			{
				bitmap.pix(ypos, xpos + bit) =
						(((plane0 << bit) & 0x80) >> 7) |
						(((plane1 << bit) & 0x80) >> 6) |
						(((plane2 << bit) & 0x80) >> 5) |
						(((plane3 << bit) & 0x80) >> 4);
			}
		}
	}
	return 0;
}

/***************************************************************************/

uint16_t thief_state::fetch_image_addr()
{
	int const addr = m_coprocessor.param[IMAGE_ADDR_LO] + 256 * m_coprocessor.param[IMAGE_ADDR_HI];

	// auto-increment
	m_coprocessor.param[IMAGE_ADDR_LO]++;

	if (m_coprocessor.param[IMAGE_ADDR_LO] == 0x00)
		m_coprocessor.param[IMAGE_ADDR_HI]++;

	return addr;
}

void thief_state::blit_w(uint8_t data)
{
	uint8_t x = m_coprocessor.param[SCREEN_XPOS];
	uint8_t y = m_coprocessor.param[SCREEN_YPOS];
	uint8_t const width = m_coprocessor.param[BLIT_WIDTH];
	uint8_t height = m_coprocessor.param[BLIT_HEIGHT];
	uint8_t attributes = m_coprocessor.param[BLIT_ATTRIBUTES];

	int xor_blit = data;
		/* making the xor behavior selectable fixes score display,
		but causes minor glitches on the playfield */

	x -= width * 8;
	int const xoffset = x & 7;
	int dy;

	if (attributes & 0x10)
	{
		y += 7 - height;
		dy = 1;
	}
	else
		dy = -1;

	height++;

	while (height--)
	{
		for (int i = 0; i <= width; i++)
		{
			int addr = fetch_image_addr();

			if (addr < 0x2000)
				data = m_coprocessor.image_ram[addr];
			else
			{
				addr -= 0x2000;
				if (addr < 0x2000 * 3)
					data = m_blitrom[addr];
			}

			int offs = (y * 32 + x / 8 + i) & 0x1fff;
			uint8_t old_data = videoram_r(offs);

			if (xor_blit)
				videoram_w(offs, old_data ^ (data >> xoffset));
			else
				videoram_w(offs, (old_data & (0xff00 >> xoffset)) | (data >> xoffset));

			offs = (offs + 1) & 0x1fff;
			old_data = videoram_r(offs);

			if (xor_blit)
				videoram_w(offs, old_data ^ ((data << (8 - xoffset)) & 0xff));
			else
				videoram_w(offs, (old_data & (0xff >> xoffset)) | ((data << (8 - xoffset)) & 0xff));
		}

		y += dy;
	}
}

uint8_t thief_state::coprocessor_r(offs_t offset)
{
	switch (offset)
	{
		case SCREEN_XPOS:
		case SCREEN_YPOS:
		{
			// XLAT: given (x,y) coordinate, return byte address in videoram
			int const addr = m_coprocessor.param[SCREEN_XPOS] + 256 * m_coprocessor.param[SCREEN_YPOS];
			int const result = 0xc000 | (addr >> 3);

			return (offset == 0x03) ? (result >> 8) : (result & 0xff);
		}

		case GFX_PORT:
		{
			int addr = fetch_image_addr();

			if (addr < 0x2000)
				return m_coprocessor.image_ram[addr];
			else
			{
				addr -= 0x2000;

				if (addr < 0x6000)
					return m_blitrom[addr];
			}
		}
			break;

		case BARL_PORT:
		{
			// return bitmask for addressed pixel
			int const dx = m_coprocessor.param[SCREEN_XPOS] & 0x7;

			if (m_coprocessor.param[BLIT_ATTRIBUTES] & 0x01)
				return 0x01 << dx; // flipx
			else
				return 0x80 >> dx; // no flip
		}
	}

	return m_coprocessor.param[offset];
}

void thief_state::coprocessor_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case GFX_PORT:
	{
		int const addr = fetch_image_addr();

		if (addr < 0x2000)
			m_coprocessor.image_ram[addr] = data;
	}
		break;

	default:
		m_coprocessor.param[offset] = data;
		break;
	}
}


void sharkatt_state::slam_w(int state)
{
	// SLAM switch causes an NMI if it's pressed
	m_maincpu->set_input_line(INPUT_LINE_NMI, state ? CLEAR_LINE : ASSERT_LINE);
}

IRQ_CALLBACK_MEMBER(sharkatt_state::iack)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return 0xff;
}

/**********************************************************/


/*  Following is an attempt to simulate the behavior of the
**  cassette tape used in several Pacific Novelty games.
**
**  It is a leaderless tape that is constructed so that it will
**  loop continuously.  The IO controller can start and stop the
**  tape player's motor, and enable/disable each of two audio
**  tracks.
*/

enum
{
	ktalktrack, kcrashtrack
};

void sharkatt_state::tape_set_audio(int track, int bon)
{
	m_samples->set_volume(track, bon ? 1.0 : 0.0);
}

void sharkatt_state::tape_set_motor(int bon)
{
	if (bon)
	{
		// If talk track is not playing, start it.
		if (!m_samples->playing(ktalktrack))
			m_samples->start(0, ktalktrack, true);

		// Resume playback of talk track.
		m_samples->pause(ktalktrack, false);


		// If crash track is not playing, start it.
		if (!m_samples->playing(kcrashtrack))
			m_samples->start(1, kcrashtrack, true);

		// Resume playback of crash track.
		m_samples->pause(kcrashtrack, false);
	}
	else
	{
		// Pause both the talk and crash tracks.
		m_samples->pause(ktalktrack, true);
		m_samples->pause(kcrashtrack, true);
	}
}

/***********************************************************/

void sharkatt_state::input_select_w(uint8_t data)
{
	m_input_select = data;
}

void sharkatt_state::tape_control_w(uint8_t data)
{
	// avoid bogus coin counts after reset
	if (data == 0x00)
		return;

	// 7---32-0  not used
	// -6------  speaker right (crash track)
	// --5-----  tape motor
	// ---4----  speaker left (talk track)
	// ------1-  coin meter

	machine().bookkeeping().coin_counter_w(0, BIT(data, 1) ? 0 : 1);

	tape_set_audio(ktalktrack, BIT(data, 4) ? 0 : 1);
	tape_set_motor(BIT(data, 5) ? 0 : 1);
	tape_set_audio(kcrashtrack, BIT(data, 6) ? 0 : 1);
}

uint8_t sharkatt_state::io_r()
{
	uint8_t data = 0xff;

	for (int i = 0; i < 4; i++)
		if (BIT(m_input_select, i))
			data &= m_ioport[i]->read();

	return data;
}

void sharkatt_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram();     // 2114
	map(0xc000, 0xdfff).rw(FUNC(sharkatt_state::videoram_r), FUNC(sharkatt_state::videoram_w));   // 4116
}

void thief_state::main_map(address_map &map)
{
	sharkatt_state::main_map(map);

	map(0x0000, 0x0000).w(FUNC(thief_state::blit_w));
	map(0xa000, 0xafff).rom();     // NATO Defense diagnostic ROM
	map(0xe000, 0xe008).rw(FUNC(thief_state::coprocessor_r), FUNC(thief_state::coprocessor_w));
	map(0xe010, 0xe02f).rom().region("copro", 0x290);
	map(0xe080, 0xe0bf).rw(FUNC(thief_state::context_ram_r), FUNC(thief_state::context_ram_w));
	map(0xe0c0, 0xe0c0).w(FUNC(thief_state::context_bank_w));
}


void sharkatt_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).nopw(); // watchdog
	map(0x10, 0x10).w(FUNC(sharkatt_state::video_control_w));
	map(0x30, 0x33).mirror(0x0c).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x41).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x41, 0x41).r("ay1", FUNC(ay8910_device::data_r));
	map(0x42, 0x43).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x43, 0x43).r("ay2", FUNC(ay8910_device::data_r));
	map(0x50, 0x50).w(FUNC(sharkatt_state::color_plane_w));
	map(0x60, 0x6f).rw(m_tms, FUNC(tms9927_device::read), FUNC(tms9927_device::write));
	map(0x70, 0x7f).w(FUNC(sharkatt_state::color_map_w));
}


/**********************************************************/

static INPUT_PORTS_START( sharkatt )
	PORT_START("DSW1")  // IN0
	PORT_DIPNAME( 0x7f, 0x7f, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x7f, DEF_STR( 1C_1C ) ) // if any are set
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("DSW2")  // IN1
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("P1")    // IN2
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("P2")    // IN3
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, sharkatt_state, slam_w)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( thief )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x000, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "7" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00|0x0c, "10K" )
	PORT_DIPSETTING(    0x01|0x0c, "20K" )
	PORT_DIPSETTING(    0x02|0x0c, "30K" )
	PORT_DIPSETTING(    0x03|0x0c, "40K" )
	PORT_DIPSETTING(    0x00|0x08, "10K 10K" )
	PORT_DIPSETTING(    0x01|0x08, "20K 20K" )
	PORT_DIPSETTING(    0x02|0x08, "30K 30K" )
	PORT_DIPSETTING(    0x03|0x08, "40K 40K" )
	PORT_DIPSETTING(    0x00,      DEF_STR( None ) )
	PORT_DIPNAME( 0xf0, 0x00, "Mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x70, "Display Options" )
	PORT_DIPSETTING(    0x80|0x00, "Burn-in Test" )
	PORT_DIPSETTING(    0x80|0x10, "Color Bar Test" )
	PORT_DIPSETTING(    0x80|0x20, "Cross Hatch" )
	PORT_DIPSETTING(    0x80|0x30, "Color Map" )
	PORT_DIPSETTING(    0x80|0x40, "VIDSEL Test" )
	PORT_DIPSETTING(    0x80|0x50, "VIDBIT Test" )
	PORT_DIPSETTING(    0x80|0x60, "I/O Board Test" )
	PORT_DIPSETTING(    0x80|0x70, "Reserved" )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, thief_state, slam_w)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( natodef )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x000, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "7" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, "Add a Coin?" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0b, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "10K" )
	PORT_DIPSETTING(    0x09, "20K" )
	PORT_DIPSETTING(    0x0a, "30K" )
	PORT_DIPSETTING(    0x0b, "40K" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0xf0, 0x00, "Mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x70, "Display Options" )
	PORT_DIPSETTING(    0x80|0x00, "Burn-in Test" )
	PORT_DIPSETTING(    0x80|0x10, "Color Bar Test" )
	PORT_DIPSETTING(    0x80|0x20, "Cross Hatch" )
	PORT_DIPSETTING(    0x80|0x30, "Color Map" )
	PORT_DIPSETTING(    0x80|0x40, "VIDSEL Test" )
	PORT_DIPSETTING(    0x80|0x50, "VIDBIT Test" )
	PORT_DIPSETTING(    0x80|0x60, "I/O Board Test" )
	PORT_DIPSETTING(    0x80|0x70, "Reserved" )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, thief_state, slam_w)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/***********************************************************/

static const char *const sharkatt_sample_names[] =
{
	"*sharkatt",
	"talk",
	"crash",
	nullptr   // end of array
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static const char *const thief_sample_names[] =
{
	"*thief",
	"talk",
	"crash",
	nullptr   // end of array
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static const char *const natodef_sample_names[] =
{
	"*natodef",
	"talk",
	"crash",
	nullptr   // end of array
};


void sharkatt_state::sharkatt(machine_config &config)
{
	Z80(config, m_maincpu, XTAL(8'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &sharkatt_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &sharkatt_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(sharkatt_state::iack));

	i8255_device &ppi(I8255A(config, "ppi"));
	ppi.out_pa_callback().set(FUNC(sharkatt_state::input_select_w));
	ppi.in_pb_callback().set(FUNC(sharkatt_state::io_r));
	ppi.out_pc_callback().set(FUNC(sharkatt_state::tape_control_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(20'000'000) / 4, 320, 0, 256, 272, 0, 192);
	m_screen->set_screen_update(FUNC(sharkatt_state::screen_update));
	m_screen->set_palette(m_palette);

	TMS9927(config, m_tms, XTAL(20'000'000) / 4 / 8);
	m_tms->set_char_width(8);
	m_tms->vsyn_callback().set_inputline("maincpu", 0, ASSERT_LINE);

	PALETTE(config, m_palette).set_entries(16);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay1", XTAL(8'000'000) / 2 / 4).add_route(ALL_OUTPUTS, "mono", 0.50);

	AY8910(config, "ay2", XTAL(8'000'000 )/ 2 / 4).add_route(ALL_OUTPUTS, "mono", 0.50);

	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(sharkatt_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void thief_state::thief(machine_config &config)
{
	sharkatt(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &thief_state::main_map);

	m_screen->set_visarea(0*8, 32*8-1, 0*8, 32*8-1);

	m_samples->set_samples_names(thief_sample_names);
}

void thief_state::natodef(machine_config &config)
{
	thief(config);

	m_samples->set_samples_names(natodef_sample_names);
}


/**********************************************************/

ROM_START( sharkatt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sharkatt.0",   0x0000, 0x800, CRC(c71505e9) SHA1(068c92e9d797918f281fa509f3c86578b3f0de3a) )
	ROM_LOAD( "sharkatt.1",   0x0800, 0x800, CRC(3e3abf70) SHA1(ef69e72db583a22093a3c32ba437a6eaef4b132a) )
	ROM_LOAD( "sharkatt.2",   0x1000, 0x800, CRC(96ded944) SHA1(e60db225111423b0a481e85fe38a85c3ea844351) )
	ROM_LOAD( "sharkatt.3",   0x1800, 0x800, CRC(007283ae) SHA1(1c311c03729573a4aa6656972e193024364a2f2a) )
	ROM_LOAD( "sharkatt.4a",  0x2000, 0x800, CRC(5cb114a7) SHA1(4240fe1bcc1501b22da133dfb42746b6752b3aea) )
	ROM_LOAD( "sharkatt.5",   0x2800, 0x800, CRC(1d88aaad) SHA1(c81f6d75d88af067f33ff84c417908c450e9e280) )
	ROM_LOAD( "sharkatt.6",   0x3000, 0x800, CRC(c164bad4) SHA1(d72e896bd4b5b0863f2ef8e621e78dd324f9d2c8) )
	ROM_LOAD( "sharkatt.7",   0x3800, 0x800, CRC(d78c4b8b) SHA1(c0371dccfb997331b31893b54fe3c749632dc171) )
	ROM_LOAD( "sharkatt.8",   0x4000, 0x800, CRC(5958476a) SHA1(2063a9721a6eec5049191c69089c3d8cc3064b69) )
	ROM_LOAD( "sharkatt.9",   0x4800, 0x800, CRC(4915eb37) SHA1(56ec2745241afd76aeaa30fb0010cedfd55f307b) )
	ROM_LOAD( "sharkatt.10",  0x5000, 0x800, CRC(9d07cb68) SHA1(528a42e8e7696452bb9d376222f3cbfcb238c01d) )
	ROM_LOAD( "sharkatt.11",  0x5800, 0x800, CRC(21edc962) SHA1(8af23e471b6eb11fc55f331ec97a94e2e6c8be80) )
	ROM_LOAD( "sharkatt.12a", 0x6000, 0x800, CRC(5dd8785a) SHA1(4eaceb781271757c4f4f6f9a4647d394d1912d72) )
ROM_END

ROM_START( thief )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Z80 code
	ROM_LOAD( "t8a0ah0a",   0x0000, 0x1000, CRC(edbbf71c) SHA1(9f13841c54fbe5449280c24954a45517014a834e) )
	ROM_LOAD( "t2662h2",    0x1000, 0x1000, CRC(85b4f6ff) SHA1(8e007bfff2f27809e7a9881bc3b2587bf35cff6d) )
	ROM_LOAD( "tc162h4",    0x2000, 0x1000, CRC(70478a82) SHA1(547bad88a44c63657bf8f65f2877ab1323515521) )
	ROM_LOAD( "t0cb4h6",    0x3000, 0x1000, CRC(29de0425) SHA1(6614f3ee314ebf2a6469481e8c69c32a93fa8eb5) )
	ROM_LOAD( "tc707h8",    0x4000, 0x1000, CRC(ea8dd847) SHA1(eab24621abe3735902f03463ee536a0cbfeb7407) )
	ROM_LOAD( "t857bh10",   0x5000, 0x1000, CRC(403c33b7) SHA1(d1422e74c9ecdadbc238b155f853294f6bb83992) )
	ROM_LOAD( "t606bh12",   0x6000, 0x1000, CRC(4ca2748b) SHA1(07df2fac63471d716923f859105421e22e5e970e) )
	ROM_LOAD( "tae4bh14",   0x7000, 0x1000, CRC(22e7dcc3) SHA1(fd4302688905bbd47dfdc1d7cdb55212a5e99f81) ) // diagnostics ROM

	ROM_REGION( 0x400, "copro", 0 ) // coprocessor
	ROM_LOAD( "b8",         0x000, 0x0200, CRC(fe865b2a) SHA1(b29144b05cb2846ea9c868ebf843d74d94c7bcc6) )
	// B8 is a function dispatch table for the coprocessor (unused)
	ROM_LOAD( "c8",         0x200, 0x0200, CRC(7ed5c923) SHA1(35757d50bfa9ea3cf916576a148064a0f9be8732) )
	// C8 is mapped (banked) in the coprocessor's address space; it contains Z80 code

	ROM_REGION( 0x6000, "blitter", 0 ) // image ROMs for coprocessor
	ROM_LOAD16_BYTE( "t079ahd4" ,  0x0001, 0x1000, CRC(928bd8ef) SHA1(3a2de005176ef012c0411d7752a69c03fb165b28) )
	ROM_LOAD16_BYTE( "tdda7hh4" ,  0x0000, 0x1000, CRC(b48f0862) SHA1(c62ccf407e819fe7fa94a4353a17da47b91f0606) )
	// next 0x4000 bytes are unmapped (used by Nato Defense)
ROM_END

/*

NATO Defense (Pacific Novelty, 6/21/82)

Labels formatted as:

 CPU PCB      COP PCB
+--------+   +--------+
| C 1982 |   | C 1982 |
| P.N.M. |   | P.N.M. |
| Inc.   |   | Inc.   |
| NATO   |   | NATO   |
| 062182 |   | 062182 |
| C317H  |   | 7058H  |
| CPU# 0 |   | COP#D4 |
+--------+   +--------+

BPROMS:
+----------+
| Nato     |
| COP # B8 |
+----------+

NOTE: ROMs at B4, C4, J4 & K4 are dated as 060982, all other ROMs dated 062182

*/
ROM_START( natodef )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Z80 code - PCB 'ROM' locations silkscreened as 0 through 15
	ROM_LOAD( "p.n.m._inc._nato_062182_c317h_cpu_0.n1",  0x0000, 0x1000, CRC(8397c787) SHA1(5957613f1ace7dc4612f28f6fba3a7374be905ac) ) // C 1982   P.N.M.  Inc.  NATO  062182  C317H  CPU# 0
	ROM_LOAD( "p.n.m._inc._nato_062182_21efh_cpu_2.l1",  0x1000, 0x1000, CRC(8cfbf26f) SHA1(a15f0d5d82cd96b80ee91dc91858b660c5895f34) ) // C 1982   P.N.M.  Inc.  NATO  062182  21EFH  CPU# 2
	ROM_LOAD( "p.n.m._inc._nato_062182_0a9eh_cpu_4.i1",  0x2000, 0x1000, CRC(b4c90fb2) SHA1(3ff4691415433863bfe74d51b9f3aa428f3bf88f) ) // C 1982   P.N.M.  Inc.  NATO  062182  0A9EH  CPU# 4
	ROM_LOAD( "p.n.m._inc._nato_062182_dcc8h_cpu_6.g1",  0x3000, 0x1000, CRC(c6d0d35e) SHA1(d4f34b4930be6dc67d77af691d14ee3b797ec29d) ) // C 1982   P.N.M.  Inc.  NATO  062182  DCC8H  CPU# 6
	ROM_LOAD( "p.n.m._inc._nato_062182_1a23h_cpu_8.n2",  0x4000, 0x1000, CRC(e4b6c21e) SHA1(cfdae66494bc2cc9ee414b9adcf8257b7c69bb40) ) // C 1982   P.N.M.  Inc.  NATO  062182  1A23H  CPU# 8
	ROM_LOAD( "p.n.m._inc._nato_062182_cbf8h_cpu_10.l2", 0x5000, 0x1000, CRC(888ecd42) SHA1(5af638d7e299046d5803d2764bf42ea44a80374c) ) // C 1982   P.N.M.  Inc.  NATO  062182  CBF8H  CPU# 10
	ROM_LOAD( "p.n.m._inc._nato_062182_8c95h_cpu_12.i2", 0x6000, 0x1000, CRC(cf713bc9) SHA1(0687755a6cfd76a920c210bf11530ef4c59d92b0) ) // C 1982   P.N.M.  Inc.  NATO  062182  8C95H  CPU# 12
	ROM_LOAD( "p.n.m._inc._nato_062182_bb32h_cpu_14.g2", 0x7000, 0x1000, CRC(4eef6bf4) SHA1(ab094198ea4d2267194ace5d382abb78d568983a) ) // C 1982   P.N.M.  Inc.  NATO  062182  BB32H  CPU# 14
	ROM_LOAD( "p.n.m._inc._nato_062182_b312h_cpu_5.h1",  0xa000, 0x1000, CRC(65c3601b) SHA1(c7bf31e6cb781405b3665b3aa93644ed57616256) ) // C 1982   P.N.M.  Inc.  NATO  062182  B312H  CPU# 5  - diagnostics ROM

	ROM_REGION( 0x400, "copro", 0 ) // coprocessor
	ROM_LOAD( "nato_cop_b8.b8", 0x000, 0x0200, CRC(fe865b2a) SHA1(b29144b05cb2846ea9c868ebf843d74d94c7bcc6) ) // Nato  COP # B8 - PCB silkscreened  PROM 2
	ROM_LOAD( "nato_cop_c8.c8", 0x200, 0x0200, CRC(7ed5c923) SHA1(35757d50bfa9ea3cf916576a148064a0f9be8732) ) // Nato  COP # C8 - PCB silkscreened  PROM 1
	// C8 is mapped (banked) in the coprocessor's address space; it contains Z80 code

	ROM_REGION( 0x6000, "blitter", 0 ) // image ROMs for coprocessor
	ROM_LOAD16_BYTE( "p.n.m._inc._nato_062182_7058h_cop_d4.d4", 0x0001, 0x1000, CRC(39a868f8) SHA1(870795f18cd8f831b714b809a380e30b5d323a5f) ) // C 1982   P.N.M.  Inc.  NATO  062182  7058H  COP#D4 - PCB silkscreened  O4
	ROM_LOAD16_BYTE( "p.n.m._inc._nato_062182_f67ah_cop_h4.h4", 0x0000, 0x1000, CRC(b6d1623d) SHA1(0aa15db0e1459a6cc7d2a5bc8e588fd514b71d85) ) // C 1982   P.N.M.  Inc.  NATO  062182  F67AH  COP#H4 - PCB silkscreened  E1
	ROM_LOAD16_BYTE( "p.n.m._inc._nato_060982_59aah_cop_c4.c4", 0x2001, 0x1000, CRC(77cc9cfd) SHA1(1bbed3cb834b844fb2d9d48a3a142edaeb33ccc6) ) // C 1982   P.N.M.  Inc.  NATO  060982  59AAH  COP#C4 - PCB silkscreened  O3
	ROM_LOAD16_BYTE( "p.n.m._inc._nato_060982_e987h_cop_j4.j4", 0x2000, 0x1000, CRC(5302410d) SHA1(e166c151d948f474c134802e3f891982bf370596) ) // C 1982   P.N.M.  Inc.  NATO  060982  E987H  COP#J4 - PCB silkscreened  E2
	ROM_LOAD16_BYTE( "p.n.m._inc._nato_060982_71fch_cop_b4.b4", 0x4001, 0x1000, CRC(b217909a) SHA1(a26eb5bf2c92d79a75376deb6278710426b34cc5) ) // C 1982   P.N.M.  Inc.  NATO  060982  71FCH  COP#B4 - PCB silkscreened  O2
	ROM_LOAD16_BYTE( "p.n.m._inc._nato_060982_629ch_cop_k4.k4", 0x4000, 0x1000, CRC(886c3f05) SHA1(306c8621455d2d6b7b2f545500b27e56a7159a1b) ) // C 1982   P.N.M.  Inc.  NATO  060982  629CH  COP#K4 - PCB silkscreened  E3
	// PCB locations silkscreened as O1 and E4 are unpopulated
ROM_END

ROM_START( natodefa )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Z80 code - PCB 'ROM' locations silkscreened as 0 through 15
	ROM_LOAD( "p.n.m._inc._nato_062182_c317h_cpu_0.n1",  0x0000, 0x1000, CRC(8397c787) SHA1(5957613f1ace7dc4612f28f6fba3a7374be905ac) ) // C 1982   P.N.M.  Inc.  NATO  062182  C317H  CPU# 0
	ROM_LOAD( "p.n.m._inc._nato_062182_21efh_cpu_2.l1",  0x1000, 0x1000, CRC(8cfbf26f) SHA1(a15f0d5d82cd96b80ee91dc91858b660c5895f34) ) // C 1982   P.N.M.  Inc.  NATO  062182  21EFH  CPU# 2
	ROM_LOAD( "p.n.m._inc._nato_062182_0a9eh_cpu_4.i1",  0x2000, 0x1000, CRC(b4c90fb2) SHA1(3ff4691415433863bfe74d51b9f3aa428f3bf88f) ) // C 1982   P.N.M.  Inc.  NATO  062182  0A9EH  CPU# 4
	ROM_LOAD( "p.n.m._inc._nato_062182_dcc8h_cpu_6.g1",  0x3000, 0x1000, CRC(c6d0d35e) SHA1(d4f34b4930be6dc67d77af691d14ee3b797ec29d) ) // C 1982   P.N.M.  Inc.  NATO  062182  DCC8H  CPU# 6
	ROM_LOAD( "p.n.m._inc._nato_062182_1a23h_cpu_8.n2",  0x4000, 0x1000, CRC(e4b6c21e) SHA1(cfdae66494bc2cc9ee414b9adcf8257b7c69bb40) ) // C 1982   P.N.M.  Inc.  NATO  062182  1A23H  CPU# 8
	ROM_LOAD( "p.n.m._inc._nato_062182_cbf8h_cpu_10.l2", 0x5000, 0x1000, CRC(888ecd42) SHA1(5af638d7e299046d5803d2764bf42ea44a80374c) ) // C 1982   P.N.M.  Inc.  NATO  062182  CBF8H  CPU# 10
	ROM_LOAD( "p.n.m._inc._nato_062182_8c95h_cpu_12.i2", 0x6000, 0x1000, CRC(cf713bc9) SHA1(0687755a6cfd76a920c210bf11530ef4c59d92b0) ) // C 1982   P.N.M.  Inc.  NATO  062182  8C95H  CPU# 12
	ROM_LOAD( "p.n.m._inc._nato_062182_bb32h_cpu_14.g2", 0x7000, 0x1000, CRC(4eef6bf4) SHA1(ab094198ea4d2267194ace5d382abb78d568983a) ) // C 1982   P.N.M.  Inc.  NATO  062182  BB32H  CPU# 14
	ROM_LOAD( "p.n.m._inc._nato_062182_b312h_cpu_5.h1",  0xa000, 0x1000, CRC(65c3601b) SHA1(c7bf31e6cb781405b3665b3aa93644ed57616256) ) // C 1982   P.N.M.  Inc.  NATO  062182  B312H  CPU# 5  - diagnostics ROM

	ROM_REGION( 0x400, "copro", 0 ) // coprocessor
	ROM_LOAD( "nato_cop_b8.b8", 0x000, 0x0200, CRC(fe865b2a) SHA1(b29144b05cb2846ea9c868ebf843d74d94c7bcc6) ) // Nato  COP # B8 - PCB silkscreened  PROM 2
	ROM_LOAD( "nato_cop_c8.c8", 0x200, 0x0200, CRC(7ed5c923) SHA1(35757d50bfa9ea3cf916576a148064a0f9be8732) ) // Nato  COP # C8 - PCB silkscreened  PROM 1
	// C8 is mapped (banked) in the coprocessor's address space; it contains Z80 code

	ROM_REGION( 0x6000, "blitter", 0 ) // image ROMs for coprocessor - same ROMs as natodef, but in a different oder to give different mazes
	ROM_LOAD16_BYTE( "p.n.m._inc._nato_062182_7058h_cop_d4.d4", 0x0001, 0x1000, CRC(39a868f8) SHA1(870795f18cd8f831b714b809a380e30b5d323a5f) ) // C 1982   P.N.M.  Inc.  NATO  062182  7058H  COP#D4 - PCB silkscreened  O4
	ROM_LOAD16_BYTE( "p.n.m._inc._nato_062182_f67ah_cop_h4.h4", 0x0000, 0x1000, CRC(b6d1623d) SHA1(0aa15db0e1459a6cc7d2a5bc8e588fd514b71d85) ) // C 1982   P.N.M.  Inc.  NATO  062182  F67AH  COP#H4 - PCB silkscreened  E1
	ROM_LOAD16_BYTE( "p.n.m._inc._nato_060982_71fch_cop_b4.b4", 0x2001, 0x1000, CRC(b217909a) SHA1(a26eb5bf2c92d79a75376deb6278710426b34cc5) ) // C 1982   P.N.M.  Inc.  NATO  060982  71FCH  COP#B4 - PCB silkscreened  O2
	ROM_LOAD16_BYTE( "p.n.m._inc._nato_060982_629ch_cop_k4.k4", 0x2000, 0x1000, CRC(886c3f05) SHA1(306c8621455d2d6b7b2f545500b27e56a7159a1b) ) // C 1982   P.N.M.  Inc.  NATO  060982  629CH  COP#K4 - PCB silkscreened  E3
	ROM_LOAD16_BYTE( "p.n.m._inc._nato_060982_59aah_cop_c4.c4", 0x4001, 0x1000, CRC(77cc9cfd) SHA1(1bbed3cb834b844fb2d9d48a3a142edaeb33ccc6) ) // C 1982   P.N.M.  Inc.  NATO  060982  59AAH  COP#C4 - PCB silkscreened  O3
	ROM_LOAD16_BYTE( "p.n.m._inc._nato_060982_e987h_cop_j4.j4", 0x4000, 0x1000, CRC(5302410d) SHA1(e166c151d948f474c134802e3f891982bf370596) ) // C 1982   P.N.M.  Inc.  NATO  060982  E987H  COP#J4 - PCB silkscreened  E2
	// PCB locations silkscreened as O1 and E4 are unpopulated
ROM_END

} // anonymous namespace


GAME( 1980, sharkatt, 0,       sharkatt, sharkatt, sharkatt_state, empty_init, ROT0, "Pacific Novelty", "Shark Attack",                    MACHINE_SUPPORTS_SAVE )
GAME( 1981, thief,    0,       thief,    thief,    thief_state,    empty_init, ROT0, "Pacific Novelty", "Thief",                           MACHINE_SUPPORTS_SAVE )
GAME( 1982, natodef,  0,       natodef,  natodef,  thief_state,    empty_init, ROT0, "Pacific Novelty", "NATO Defense" ,                   MACHINE_SUPPORTS_SAVE )
GAME( 1982, natodefa, natodef, natodef,  natodef,  thief_state,    empty_init, ROT0, "Pacific Novelty", "NATO Defense (alternate mazes)" , MACHINE_SUPPORTS_SAVE )
