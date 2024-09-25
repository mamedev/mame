// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/****************************************

    "Universal System 16" Hardware (c) 1983/1986 Namco

    driver by Angelo Salese,
    original "wiped off due of not anymore licenseable" driver by Edgardo E. Contini Salvan.

    TODO:
    - PAL is presumably inverted with address bit 11 (0x800) for 0x6000-0x7fff area
      between Libble Rabble and Toy Pop.
    - Proper sprite DMA.
    - Flip Screen;
    - Remaining outputs;

    Notes:
    ------
    - Libble Rabble Easter egg:
     - enter service mode
     - turn off the service mode switch, and turn it on again quickly to remain
       on the monitor test grid
     - Enter the following sequence using the right joystick:
       9xU 2xR 9xD 2xL
    (c) 1983 NAMCO LTD. will appear on the screen.

****************************************/

#include "emu.h"
#include "namcoio.h"

#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"
#include "machine/timer.h"
#include "sound/namco.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define MASTER_CLOCK XTAL(6'144'000)

class namcos16_state : public driver_device
{
public:
	namcos16_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_master_cpu(*this,"maincpu"),
		m_slave_cpu(*this, "slave"),
		m_sound_cpu(*this, "audiocpu"),
		m_namco15xx(*this, "namco"),
		m_namco58xx(*this, "58xx"),
		m_namco56xx_1(*this, "56xx_1"),
		m_namco56xx_2(*this, "56xx_2"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_master_workram(*this, "master_workram"),
		m_slave_sharedram(*this, "slave_sharedram"),
		m_bgvram(*this, "bgvram"),
		m_fgvram(*this, "fgvram"),
		m_fgattr(*this, "fgattr")
	{
	}

	void toypop(machine_config &config);
	void liblrabl(machine_config &config);

private:
	required_device<cpu_device> m_master_cpu;
	required_device<cpu_device> m_slave_cpu;
	required_device<cpu_device> m_sound_cpu;

	required_device<namco_15xx_device> m_namco15xx;
	required_device<namco58xx_device> m_namco58xx;
	required_device<namco56xx_device> m_namco56xx_1;
	required_device<namco56xx_device> m_namco56xx_2;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_master_workram;
	required_shared_ptr<uint8_t> m_slave_sharedram;
	required_shared_ptr<uint16_t> m_bgvram;
	required_shared_ptr<uint8_t> m_fgvram;
	required_shared_ptr<uint8_t> m_fgattr;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(master_scanline);
	void slave_vblank_irq(int state);

	uint8_t irq_enable_r();
	void irq_disable_w(uint8_t data);
	void irq_ctrl_w(offs_t offset, uint8_t data);
	void toypop_palette(palette_device &palette) const;
	uint8_t dipA_l();
	uint8_t dipA_h();
	uint8_t dipB_l();
	uint8_t dipB_h();
	//void out_coin0(uint8_t data);
	//void out_coin1(uint8_t data);
	void pal_bank_w(offs_t offset, uint8_t data);
	void flip(uint8_t data);
	void slave_halt_ctrl_w(offs_t offset, uint8_t data);
	uint8_t slave_shared_r(offs_t offset);
	void slave_shared_w(offs_t offset, uint8_t data);
	void slave_irq_enable_w(offs_t offset, uint16_t data);
	void sound_halt_ctrl_w(offs_t offset, uint8_t data);
	uint8_t bg_rmw_r(offs_t offset);
	void bg_rmw_w(offs_t offset, uint8_t data);

	void master_liblrabl_map(address_map &map) ATTR_COLD;
	void master_toypop_map(address_map &map) ATTR_COLD;
	void namcos16_master_base_map(address_map &map) ATTR_COLD;
	void slave_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	// driver_device overrides
//  virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

//  virtual void video_start() override ATTR_COLD;

	bool m_master_irq_enable = false;
	bool m_slave_irq_enable = false;
	uint8_t m_pal_bank = 0;

	void legacy_bg_draw(bitmap_ind16 &bitmap,const rectangle &cliprect,bool flip);
	void legacy_fg_draw(bitmap_ind16 &bitmap,const rectangle &cliprect,bool flip);
	void legacy_obj_draw(bitmap_ind16 &bitmap,const rectangle &cliprect,bool flip);
};

void namcos16_state::toypop_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		bit3 = BIT(color_prom[i], 3);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// green component
		bit0 = BIT(color_prom[i + 0x100], 0);
		bit1 = BIT(color_prom[i + 0x100], 1);
		bit2 = BIT(color_prom[i + 0x100], 2);
		bit3 = BIT(color_prom[i + 0x100], 3);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// blue component
		bit0 = BIT(color_prom[i + 0x200], 0);
		bit1 = BIT(color_prom[i + 0x200], 1);
		bit2 = BIT(color_prom[i + 0x200], 2);
		bit3 = BIT(color_prom[i + 0x200], 3);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	for (int i = 0; i < 256; i++)
	{
		// characters
		palette.set_pen_indirect(i + 0*256, (color_prom[i + 0x300] & 0x0f) | 0x70);
		palette.set_pen_indirect(i + 1*256, (color_prom[i + 0x300] & 0x0f) | 0xf0);

		// sprites
		uint8_t const entry = color_prom[i + 0x500];
		palette.set_pen_indirect(i + 2*256, entry);
	}

	for (int i = 0; i < 16; i++)
	{
		// background
		palette.set_pen_indirect(i + 3*256 + 0*16, 0x60 + i);
		palette.set_pen_indirect(i + 3*256 + 1*16, 0xe0 + i);
	}
}

void namcos16_state::legacy_bg_draw(bitmap_ind16 &bitmap,const rectangle &cliprect,bool flip)
{
	uint16_t const pal_base = 0x300 + (m_pal_bank << 4);
	uint32_t const src_base = 0x200/2;
	uint16_t const src_pitch = 288 / 2;

	for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		uint16_t const *src = &m_bgvram[y * src_pitch + cliprect.min_x + src_base];
		uint16_t *dst = &bitmap.pix(flip ? (cliprect.max_y - y) : y, flip ? cliprect.max_x : cliprect.min_x);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x += 2)
		{
			uint32_t const srcpix = *src++;
			int const idx1 = ((srcpix >> 8) & 0xf) + pal_base;
			int const idx2 = (srcpix & 0xf) + pal_base;
			if (!flip)
			{
				*dst++ = m_palette->pen(idx1);
				*dst++ = m_palette->pen(idx2);
			}
			else
			{
				*dst-- = m_palette->pen(idx1);
				*dst-- = m_palette->pen(idx2);
			}
		}
	}
}

void namcos16_state::legacy_fg_draw(bitmap_ind16 &bitmap,const rectangle &cliprect,bool flip)
{
	gfx_element *const gfx_0 = m_gfxdecode->gfx(0);

	for (int count = 0; count < 32*32; count++)
	{
		int const xoffs(count >> 5);
		int const yoffs(count & 0x1f);
		int x;// = (count % 32);
		int y; //= count / 32;

		if (count < 64)
		{
			x = 34 + xoffs;
			y = yoffs - 2;
		}
		else if (count >= 32*30)
		{
			x = xoffs - 30;
			y = yoffs - 2;
		}
		else
		{
			x = 2 + yoffs;
			y = xoffs - 2;
		}

		uint16_t tile = m_fgvram[count];
		uint8_t color = (m_fgattr[count] & 0x3f) + (m_pal_bank << 6);

		gfx_0->transpen(bitmap, cliprect, tile, color, flip, flip, (flip ? 35-x : x)*8, (flip ? 27-y : y)*8, 0);
	}
}

// TODO: this is likely to be a lot more complex, and maybe is per scanline too
void namcos16_state::legacy_obj_draw(bitmap_ind16 &bitmap,const rectangle &cliprect,bool flip)
{
	gfx_element *gfx_1 = m_gfxdecode->gfx(1);
	int count;
	uint8_t *base_spriteram = m_master_workram;
	const uint16_t bank1 = 0x0800;
	const uint16_t bank2 = 0x1000;


	for (count=0x780;count<0x800;count+=2)
	{
		bool enabled = (base_spriteram[count+bank2+1] & 2) == 0;

		if(enabled == false)
			continue;

		static const int gfx_offs[2][2] =
		{
			{ 0, 1 },
			{ 2, 3 }
		};
		uint8_t tile = base_spriteram[count];
		uint8_t color = base_spriteram[count+1];
		int x = base_spriteram[count+bank1+1] + (base_spriteram[count+bank2+1] << 8);
		x -= 71;

		int y = base_spriteram[count+bank1+0];
		y += 7;
		// TODO: actually m_screen.height()
		y = 224 - y;

		bool fx = (base_spriteram[count+bank2] & 1) == 1;
		bool fy = (base_spriteram[count+bank2] & 2) == 2;
		uint8_t width = ((base_spriteram[count+bank2] & 4) >> 2) + 1;
		uint8_t height = ((base_spriteram[count+bank2] & 8) >> 3) + 1;

		tile &= ~(width - 1);
		tile &= ~((height - 1) << 1);

		if (flip)
		{
			fx ^= 1;
			fy ^= 1;
		}

		if (height == 2)
			y -=16;

		for (int yi=0; yi<height; yi++)
		{
			for(int xi=0; xi<width; xi++)
			{
				uint16_t sprite_offs = tile + gfx_offs[yi ^ ((height - 1) * fy)][xi ^ ((width - 1) * fx)];
				gfx_1->transmask(bitmap,cliprect,sprite_offs,color,fx,fy,x + xi*16,y + yi *16,m_palette->transpen_mask(*gfx_1, color, 0xff));
			}
		}
	}
}

uint32_t namcos16_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	bool const flip = flip_screen();
	legacy_bg_draw(bitmap,cliprect,flip);
	legacy_fg_draw(bitmap,cliprect,flip);
	legacy_obj_draw(bitmap,cliprect,flip);
	return 0;
}

uint8_t namcos16_state::irq_enable_r()
{
	m_master_irq_enable = true;
	return 0;
}

void namcos16_state::irq_disable_w(uint8_t data)
{
	m_master_irq_enable = false;
}


void namcos16_state::irq_ctrl_w(offs_t offset, uint8_t data)
{
	m_master_irq_enable = (offset & 0x0800) ? false : true;
}

void namcos16_state::slave_halt_ctrl_w(offs_t offset, uint8_t data)
{
	m_slave_cpu->set_input_line(INPUT_LINE_RESET,offset & 0x800 ? ASSERT_LINE : CLEAR_LINE);
}

void namcos16_state::sound_halt_ctrl_w(offs_t offset, uint8_t data)
{
	m_sound_cpu->set_input_line(INPUT_LINE_RESET,offset & 0x800 ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t namcos16_state::slave_shared_r(offs_t offset)
{
	return m_slave_sharedram[offset];
}

void namcos16_state::slave_shared_w(offs_t offset, uint8_t data)
{
	m_slave_sharedram[offset] = data;
}

void namcos16_state::slave_irq_enable_w(offs_t offset, uint16_t data)
{
	m_slave_irq_enable = (offset & 0x40000) ? false : true;
}

uint8_t namcos16_state::bg_rmw_r(offs_t offset)
{
	uint8_t res;

	res = 0;
	// note: following offset is written as offset * 2
	res |= (m_bgvram[offset] & 0x0f00) >> 4;
	res |= (m_bgvram[offset] & 0x000f);
	return res;
}

void namcos16_state::bg_rmw_w(offs_t offset, uint8_t data)
{
	// note: following offset is written as offset * 2
	m_bgvram[offset] = (data & 0xf) | ((data & 0xf0) << 4);
}

uint8_t namcos16_state::dipA_l() { return ioport("DSW1")->read(); }                // dips A
uint8_t namcos16_state::dipA_h() { return ioport("DSW1")->read() >> 4; }           // dips A
uint8_t namcos16_state::dipB_l() { return ioport("DSW2")->read(); }                // dips B
uint8_t namcos16_state::dipB_h() { return ioport("DSW2")->read() >> 4; }           // dips B

void namcos16_state::flip(uint8_t data)
{
	flip_screen_set(data & 1);
}

void namcos16_state::pal_bank_w(offs_t offset, uint8_t data)
{
	m_pal_bank = offset & 1;
}

void namcos16_state::namcos16_master_base_map(address_map &map)
{
	map(0x0000, 0x03ff).ram().share("fgvram");
	map(0x0400, 0x07ff).ram().share("fgattr");
	map(0x0800, 0x1fff).ram().share("master_workram");
	map(0x2800, 0x2fff).ram().share("slave_sharedram");

	// 0x6000 - 0x7fff i/o specific, guessing PAL controlled.

	map(0x8000, 0x8fff).w(FUNC(namcos16_state::slave_halt_ctrl_w));
	map(0x9000, 0x9fff).w(FUNC(namcos16_state::sound_halt_ctrl_w));
	map(0xa000, 0xa001).w(FUNC(namcos16_state::pal_bank_w));
	map(0x8000, 0xffff).rom().region("master_rom", 0);
}

void namcos16_state::master_liblrabl_map(address_map &map)
{
	namcos16_master_base_map(map);
	map(0x6000, 0x63ff).rw(m_namco15xx, FUNC(namco_15xx_device::sharedram_r), FUNC(namco_15xx_device::sharedram_w));
	map(0x6800, 0x680f).rw(m_namco58xx, FUNC(namco58xx_device::read), FUNC(namco58xx_device::write));
	map(0x6810, 0x681f).rw(m_namco56xx_1, FUNC(namco56xx_device::read), FUNC(namco56xx_device::write));
	map(0x6820, 0x682f).rw(m_namco56xx_2, FUNC(namco56xx_device::read), FUNC(namco56xx_device::write));
	map(0x7000, 0x7fff).nopr().w(FUNC(namcos16_state::irq_ctrl_w));
}

void namcos16_state::master_toypop_map(address_map &map)
{
	namcos16_master_base_map(map);
	map(0x6000, 0x600f).rw(m_namco58xx, FUNC(namco58xx_device::read), FUNC(namco58xx_device::write));
	map(0x6010, 0x601f).rw(m_namco56xx_1, FUNC(namco56xx_device::read), FUNC(namco56xx_device::write));
	map(0x6020, 0x602f).rw(m_namco56xx_2, FUNC(namco56xx_device::read), FUNC(namco56xx_device::write));
	map(0x6800, 0x6bff).rw(m_namco15xx, FUNC(namco_15xx_device::sharedram_r), FUNC(namco_15xx_device::sharedram_w));
	map(0x7000, 0x7000).rw(FUNC(namcos16_state::irq_enable_r), FUNC(namcos16_state::irq_disable_w));
}

void namcos16_state::slave_map(address_map &map)
{
	map(0x000000, 0x007fff).rom().region("slave_rom", 0);
	map(0x080000, 0x0bffff).ram();
	map(0x100000, 0x100fff).rw(FUNC(namcos16_state::slave_shared_r), FUNC(namcos16_state::slave_shared_w)).umask16(0x00ff);
	map(0x180000, 0x187fff).rw(FUNC(namcos16_state::bg_rmw_r), FUNC(namcos16_state::bg_rmw_w));
	map(0x190000, 0x1dffff).ram().share("bgvram");
	map(0x300000, 0x3fffff).w(FUNC(namcos16_state::slave_irq_enable_w));
}

void namcos16_state::sound_map(address_map &map)
{
	map(0x0000, 0x03ff).rw(m_namco15xx, FUNC(namco_15xx_device::sharedram_r), FUNC(namco_15xx_device::sharedram_w));
	map(0xe000, 0xffff).rom().region("sound_rom", 0);
}



static INPUT_PORTS_START( liblrabl )
	/* The inputs are not memory mapped, they are handled by three I/O chips. */
	PORT_START("P1_RIGHT")  /* 58XX #0 pins 22-29 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY

	PORT_START("P2_RIGHT")  /* 58XX #0 pins 22-29 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("P1_LEFT")   /* 56XX #2 pins 22-29 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY

	PORT_START("P2_LEFT")   /* 56XX #2 pins 22-29 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("BUTTONS")   /* 58XX #0 pins 30-33 and 38-41 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("COINS") /* 58XX #0 pins 30-33 and 38-41 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("DSW1")  /* 56XX #1 pins 22-29 */
	/* default setting: all OFF */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWA:8,7")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWA:6,5,4")
	// bonus scores for common
	PORT_DIPSETTING(    0x1c, "40k 120k 200k 400k 600k 1m" )
	PORT_DIPSETTING(    0x0c, "40k 140k 250k 400k 700k 1m" )
	// bonus scores for 1, 2 or 3 lives
	PORT_DIPSETTING(    0x14, "50k 150k 300k 500k 700k 1m" ) PORT_CONDITION("DSW1", 0x03, NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x04, "40k 120k and every 120k" )    PORT_CONDITION("DSW1", 0x03, NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x18, "40k 150k and every 150k" )    PORT_CONDITION("DSW1", 0x03, NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x08, "50k 150k 300k" )              PORT_CONDITION("DSW1", 0x03, NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x10, "40k 120k 200k" )              PORT_CONDITION("DSW1", 0x03, NOTEQUALS, 0x01)
	// bonus scores for 5 lives
	PORT_DIPSETTING(    0x14, "40k 120k" )                   PORT_CONDITION("DSW1", 0x03, EQUALS, 0x01)
	PORT_DIPSETTING(    0x04, "50k 150k" )                   PORT_CONDITION("DSW1", 0x03, EQUALS, 0x01)
	PORT_DIPSETTING(    0x18, "50k 150k and every 150k" )    PORT_CONDITION("DSW1", 0x03, EQUALS, 0x01)
	PORT_DIPSETTING(    0x08, "60k 200k and every 200k" )    PORT_CONDITION("DSW1", 0x03, EQUALS, 0x01)
	PORT_DIPSETTING(    0x10, "50k" )                        PORT_CONDITION("DSW1", 0x03, EQUALS, 0x01)
	// bonus scores for common
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWA:3,2,1")
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")  /* 56XX #1 pins 30-33 and 38-41 */
	PORT_DIPNAME( 0x01, 0x01, "Freeze" ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Rack Test" ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWB:5,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x20, 0x20, "Practice" ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:2,1")
	PORT_DIPSETTING(    0xc0, "A" )
	PORT_DIPSETTING(    0x40, "B" )
	PORT_DIPSETTING(    0x80, "C" )
	PORT_DIPSETTING(    0x00, "D" )

	PORT_START("SERVICE")   /* 56XX #2 pins 30-33 */
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( toypop )
	/* The inputs are not memory mapped, they are handled by three I/O chips. */
	PORT_START("P1_RIGHT")  /* 58XX #0 pins 22-29 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY

	PORT_START("P2_RIGHT")  /* 58XX #0 pins 22-29 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)

	PORT_START("BUTTONS")   /* 58XX #0 pins 30-33 and 38-41 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("COINS") /* 58XX #0 pins 30-33 and 38-41 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("DSW1")  /* 56XX #1 pins 22-29 */
	/* default setting: all OFF */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWA:8,7")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWA:6,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWA:4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, 0x80, "SWA:1" )

	PORT_START("DSW2")  /* 56XX #1 pins 30-33 and 38-41 */
	PORT_DIPNAME( 0x01, 0x01, "Freeze" ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Level_Select ) ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "2 Players Game" ) PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x00, "1 Credit" )
	PORT_DIPSETTING(    0x04, "2 Credits" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Entering" ) PORT_DIPLOCATION("SWB:4")    // buy in
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:3,2")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, "Every 15000 points" )
	PORT_DIPSETTING(    0x00, "Every 20000 points" )

	PORT_START("P1_LEFT")   /* 56XX #2 pins 22-29 */
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_LEFT")   /* 56XX #2 pins 22-29 */
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE")   /* 56XX #2 pins 30-33 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) // would be Cabinet, but this game has no cocktail mode
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )    // service mode again
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 8*8+0, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8, 8*8+1, 8*8+2, 8*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
	24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	32 * 8, 33 * 8, 34 * 8, 35 * 8, 36 * 8, 37 * 8, 38 * 8, 39 * 8 },
	64*8
};

static GFXDECODE_START( gfx_toypop )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,       0, 128 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 128*4,  64 )
GFXDECODE_END

void namcos16_state::machine_reset()
{
	m_master_irq_enable = false;
	m_slave_irq_enable = false;
	m_slave_cpu->set_input_line(INPUT_LINE_RESET,ASSERT_LINE);
	m_sound_cpu->set_input_line(INPUT_LINE_RESET,ASSERT_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(namcos16_state::master_scanline)
{
	int scanline = param;

	if(scanline == 224 && m_master_irq_enable == true)
		m_master_cpu->set_input_line(M6809_IRQ_LINE,HOLD_LINE);

	// TODO: definitely can't fire from this, presume that a command send has a timing response ...
	if(scanline == 0)
	{
		if (!m_namco58xx->read_reset_line())
			m_namco58xx->customio_run();

		if (!m_namco56xx_1->read_reset_line())
			m_namco56xx_1->customio_run();

		if (!m_namco56xx_2->read_reset_line())
			m_namco56xx_2->customio_run();
	}
}

void namcos16_state::slave_vblank_irq(int state)
{
	if (state && m_slave_irq_enable == true)
		m_slave_cpu->set_input_line(6, HOLD_LINE);
}

void namcos16_state::liblrabl(machine_config &config)
{
	MC6809E(config, m_master_cpu, MASTER_CLOCK/4);
	m_master_cpu->set_addrmap(AS_PROGRAM, &namcos16_state::master_liblrabl_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(namcos16_state::master_scanline), "screen", 0, 1);

	M68000(config, m_slave_cpu, MASTER_CLOCK);
	m_slave_cpu->set_addrmap(AS_PROGRAM, &namcos16_state::slave_map);

	MC6809E(config, m_sound_cpu, MASTER_CLOCK/4);
	m_sound_cpu->set_addrmap(AS_PROGRAM, &namcos16_state::sound_map);
	m_sound_cpu->set_periodic_int(FUNC(namcos16_state::irq0_line_hold), attotime::from_hz(60));

	NAMCO_58XX(config, m_namco58xx, 0);
	m_namco58xx->in_callback<0>().set_ioport("COINS");
	m_namco58xx->in_callback<1>().set_ioport("P1_RIGHT");
	m_namco58xx->in_callback<2>().set_ioport("P2_RIGHT");
	m_namco58xx->in_callback<3>().set_ioport("BUTTONS");

	NAMCO_56XX(config, m_namco56xx_1, 0);
	m_namco56xx_1->in_callback<0>().set(FUNC(namcos16_state::dipA_h));
	m_namco56xx_1->in_callback<1>().set(FUNC(namcos16_state::dipB_l));
	m_namco56xx_1->in_callback<2>().set(FUNC(namcos16_state::dipB_h));
	m_namco56xx_1->in_callback<3>().set(FUNC(namcos16_state::dipA_l));
	m_namco56xx_1->out_callback<0>().set(FUNC(namcos16_state::flip));

	NAMCO_56XX(config, m_namco56xx_2, 0);
	m_namco56xx_2->in_callback<1>().set_ioport("P1_LEFT");
	m_namco56xx_2->in_callback<2>().set_ioport("P2_LEFT");
	m_namco56xx_2->in_callback<3>().set_ioport("SERVICE");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(MASTER_CLOCK,384,0,288,264,0,224); // derived from Galaxian HW, 60.606060
	screen.set_screen_update(FUNC(namcos16_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(namcos16_state::slave_vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_toypop);
	PALETTE(config, m_palette, FUNC(namcos16_state::toypop_palette), 128*4 + 64*4 + 16*2, 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	NAMCO_15XX(config, m_namco15xx, 24000);
	m_namco15xx->set_voices(8);
	m_namco15xx->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void namcos16_state::toypop(machine_config &config)
{
	liblrabl(config);
	m_master_cpu->set_addrmap(AS_PROGRAM, &namcos16_state::master_toypop_map);
}


ROM_START( liblrabl )
	ROM_REGION( 0x8000, "master_rom", 0 )
	ROM_LOAD( "5b.rom",   0x0000, 0x4000, CRC(da7a93c2) SHA1(fe4a02cdab66722eb7b8cf58825f899b1949a6a2) )
	ROM_LOAD( "5c.rom",   0x4000, 0x4000, CRC(6cae25dc) SHA1(de74317a7d5de1865d096c377923a764be5e6879) )

	ROM_REGION( 0x2000, "sound_rom", 0 )
	ROM_LOAD( "2c.rom",   0x0000, 0x2000, CRC(7c09e50a) SHA1(5f004d60bbb7355e008a9cda137b28bc2192b8ef) )

	ROM_REGION16_BE( 0x8000, "slave_rom", 0 )
	ROM_LOAD16_BYTE("8c.rom",    0x0000, 0x4000, CRC(a00cd959) SHA1(cc5621103c31cfbc65941615cab391db0f74e6ce) )
	ROM_LOAD16_BYTE("10c.rom",   0x0001, 0x4000, CRC(09ce209b) SHA1(2ed46d6592f8227bac8ab54963d9a300706ade47) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5p.rom",   0x0000, 0x2000, CRC(3b4937f0) SHA1(06d9de576f1c2262c34aeb91054e68c9298af688) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "9t.rom",   0x0000, 0x4000, CRC(a88e24ca) SHA1(eada133579f19de09255084dcdc386311606a335) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "lr1-3.1r", 0x0000, 0x0100, CRC(f3ec0d07) SHA1(b0aad1fb6df79f202889600f486853995352f9c2) )
	ROM_LOAD( "lr1-2.1s", 0x0100, 0x0100, CRC(2ae4f702) SHA1(838fdca9e91fea4f64a59880ac47c48973bb8fbf) )
	ROM_LOAD( "lr1-1.1t", 0x0200, 0x0100, CRC(7601f208) SHA1(572d070ca387b780030ed5de38a8970b7cc14349) )
	ROM_LOAD( "lr1-5.5l", 0x0300, 0x0100, CRC(940f5397) SHA1(825a7bd78a8a08d30bad2e4890ae6e9ad88b36b8) )
	ROM_LOAD( "lr1-6.2p", 0x0400, 0x0200, CRC(a6b7f850) SHA1(7cfde16dfd5c4d5b876b4fbe4f924f1385932a93) )

	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "lr1-4.3d", 0x0000, 0x0100, CRC(16a9166a) SHA1(847cbaf7c88616576c410177e066ae1d792ac0ba) )
ROM_END

ROM_START( toypop )
	ROM_REGION( 0x8000, "master_rom", 0 )
	ROM_LOAD( "tp1-2.5b", 0x0000, 0x4000, CRC(87469620) SHA1(2ee257486c9c044386ac7d0cd4a90583eaeb3e97) )
	ROM_LOAD( "tp1-1.5c", 0x4000, 0x4000, CRC(dee2fd6e) SHA1(b2c12008d6d3e7544ba3c12a52a6abf9181842c8) )

	ROM_REGION( 0x2000, "sound_rom", 0 )
	ROM_LOAD( "tp1-3.2c", 0x0000, 0x2000, CRC(5f3bf6e2) SHA1(d1b3335661b9b23cb10001416c515b77b5e783e9) )

	ROM_REGION16_BE( 0x8000, "slave_rom", 0 )
	ROM_LOAD16_BYTE("tp1-4.8c",  0x0000, 0x4000, CRC(76997db3) SHA1(5023a2f20a5f2c9baff130f6832583493c71f883) )
	ROM_LOAD16_BYTE("tp1-5.10c", 0x0001, 0x4000, CRC(37de8786) SHA1(710365e34c05d01815844c414518f93234b6160b) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tp1-7.5p", 0x0000, 0x2000, CRC(95076f9e) SHA1(1e3d32b21f6d46591ec3921aba51f672d64a9023) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "tp1-6.9t", 0x0000, 0x4000, CRC(481ffeaf) SHA1(c51735ad3a1dbb46ad414408b54554e9223b2219) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "tp1-3.1r", 0x0000, 0x0100, CRC(cfce2fa5) SHA1(b42aa0f34d885389d2650bf7a0531b95703b8a28) )
	ROM_LOAD( "tp1-2.1s", 0x0100, 0x0100, CRC(aeaf039d) SHA1(574560526100d38635aecd71eb73499c4f57d586) )
	ROM_LOAD( "tp1-1.1t", 0x0200, 0x0100, CRC(08e7cde3) SHA1(5261aca6834d635d17f8afaa8e35848930030ba4) )
	ROM_LOAD( "tp1-4.5l", 0x0300, 0x0100, CRC(74138973) SHA1(2e21dbb1b19dd089da52e70fcb0ca91336e004e6) )
	ROM_LOAD( "tp1-5.2p", 0x0400, 0x0200, CRC(4d77fa5a) SHA1(2438910314b23ecafb553230244f3931861ad2da) )

	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "tp1-6.3d", 0x0000, 0x0100, CRC(16a9166a) SHA1(847cbaf7c88616576c410177e066ae1d792ac0ba) )
ROM_END

} // anonymous namespace


GAME( 1983, liblrabl, 0,     liblrabl, liblrabl, namcos16_state, empty_init, ROT0,   "Namco", "Libble Rabble", 0 )
GAME( 1986, toypop,   0,     toypop,   toypop,   namcos16_state, empty_init, ROT180, "Namco", "Toypop",        0 )
