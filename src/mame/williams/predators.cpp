// license:BSD-3-Clause
// copyright-holders: Angelo Salese, David Haywood, Lysiwyg, Philip Bennett
/*

Williams The Predators (prototype)

*/

#include "emu.h"

#include "cpu/m6800/m6800.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/hc55516.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include <algorithm>


DECLARE_DEVICE_TYPE(WILLIAMS_PREDATORS_SOUND, williams_pred_sound_device)

class williams_pred_sound_device :  public device_t, public device_mixer_interface
{
public:
	williams_pred_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);

	void sound_w(uint16_t data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void bank_w(uint8_t data);

	void memmap(address_map &map) ATTR_COLD;

	required_device<m6802_cpu_device> m_cpu;
	required_device<pia6821_device> m_pia;
	required_device<dac_device_base> m_dac;
	memory_bank_array_creator<2> m_bank;
};

void williams_pred_sound_device::bank_w(uint8_t data)
{
	m_bank[1]->set_entry(BIT(data, 0));
	m_bank[0]->set_entry(BIT(data, 1));
}

void williams_pred_sound_device::memmap(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x1000, 0x1000).w(FUNC(williams_pred_sound_device::bank_w));
	map(0x2000, 0x2003).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8000, 0xbfff).bankr(m_bank[0]);
	map(0xc000, 0xffff).bankr(m_bank[1]);
}

DEFINE_DEVICE_TYPE(WILLIAMS_PREDATORS_SOUND, williams_pred_sound_device, "williams_predators_sound", "Williams Predators Sound Board")

williams_pred_sound_device::williams_pred_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WILLIAMS_PREDATORS_SOUND, tag, owner, clock),
	device_mixer_interface(mconfig, *this),
	m_cpu(*this, "cpu"),
	m_pia(*this, "pia"),
	m_dac(*this, "dac"),
	m_bank(*this, { "low", "high" })
{
}

void williams_pred_sound_device::device_add_mconfig(machine_config &config)
{
	M6808(config, m_cpu, 4'000'000);
	m_cpu->set_addrmap(AS_PROGRAM, &williams_pred_sound_device::memmap);
	m_cpu->set_ram_enable(false);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_cpu, M6802_IRQ_LINE);

	PIA6821(config, m_pia);
	m_pia->writepb_handler().set(m_dac, FUNC(dac_byte_interface::data_w));
	m_pia->cb2_handler().set("cvsd", FUNC(hc55516_device::digit_w));
	m_pia->ca2_handler().set("cvsd", FUNC(hc55516_device::clock_w));
	m_pia->irqa_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));
	m_pia->irqb_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	HC55516(config, "cvsd").add_route(ALL_OUTPUTS, *this, 0.8);

	MC1408(config, m_dac, 0).add_route(ALL_OUTPUTS, *this, 0.1);
}

void williams_pred_sound_device::device_start()
{
	m_bank[0]->configure_entries(0, 2, memregion("cpu")->base(), 0x4000);
	m_bank[1]->configure_entries(0, 2, memregion("cpu")->base() + 0x8000, 0x4000);
}

void williams_pred_sound_device::device_reset()
{
	m_bank[0]->set_entry(0);
	m_bank[1]->set_entry(1);
}

void williams_pred_sound_device::sound_w(uint16_t data)
{
	m_pia->porta_w(data);
	m_pia->ca1_w(0);
	m_pia->ca1_w(1);
}

namespace {

class predators_state : public driver_device
{
public:
	predators_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_i8035(*this, "i8035"),
		m_screen(*this, { "hawk", "falcon", "eagle", "condor" }), // show screen names
		m_palette(*this, "palette%u", 0U),
		m_ptm(*this, "ptm"),
		m_blit_length(*this, "blitlength"),
		m_blit_yoffs(*this, "blityoffs"),
		m_blit_attr(*this, "blitattr"),
		m_blit_xoffs(*this, "blitxoffs"),
		m_blit_color(*this, "blitcolor"),
		m_proms(*this, "proms%u", 0U),
		m_sincos(*this, "sincos%u", 0U),
		m_stickx(*this, "stickx%u", 0),
		m_sticky(*this, "sticky%u", 0),
		m_general(*this, "general%u", 0),
		m_predsnd(*this, { "hawk0", "falcon0", "eagle0", "condor0", "hawk1", "falcon1", "eagle1", "condor1" }), // show channel names in mixer
		m_vram(*this, "vram%u", 0U, 0x20000U, ENDIANNESS_LITTLE)
	{ }

	void predators(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// 4 mono sounds better than 4 stereo
	static constexpr bool USE_MONO_SPEAKERS = true;

	// move into a screen device?
	struct screen_state
	{
		// blitter register
		uint8_t     m_reg0_trigger;
		uint16_t    m_reg1_xmax;
		uint16_t    m_reg2_xmin;
		uint8_t     m_reg3_blit_size;
		uint16_t    m_reg4_blit_src;

		// working
		uint16_t m_xmin;
		uint16_t m_xmax;
		uint8_t m_vram_page;
		uint8_t m_busy;
		uint8_t m_swap;
	};

	screen_state m_screen_state[4];
	uint8_t m_screen_irq_mask;

	emu_timer *m_blit_done_timer;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_i8035;
	required_device_array<screen_device, 4> m_screen;
	required_device_array<palette_device, 4> m_palette;
	required_device<ptm6840_device> m_ptm;
	required_region_ptr<uint8_t> m_blit_length;
	required_region_ptr<uint8_t> m_blit_yoffs;
	required_region_ptr<uint8_t> m_blit_attr;
	required_region_ptr<uint8_t> m_blit_xoffs;
	required_region_ptr<uint8_t> m_blit_color;
	required_region_ptr_array<u8, 4> m_proms;
	required_region_ptr_array<u8, 4> m_sincos;
	required_ioport_array<4> m_stickx;
	required_ioport_array<4> m_sticky;
	required_ioport_array<4> m_general;
	required_device_array<williams_pred_sound_device, 8> m_predsnd;
	memory_share_array_creator<u8, 4> m_vram;

	template <int N>
	void vblank_write_line(int state);

	TIMER_CALLBACK_MEMBER(draw_complete);

	void blit_param_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void blit_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void update_main_irqs(void);
	void request_vram_swap(int screen);
	void clear_vram(int screen);
	void set_clipping_region(int screen);
	void do_blit(int screen);
	void execute_draw_cmd(int screen);
	void draw_line(int screen, int startx, int starty, int valctab, int valstab, int length, int color);
	void write_4bpp_pixel(int screen, int x, int y, uint8_t pix, int wrap);
	void draw_cmd_finished(int screen);
	void screen_start(int screen) ATTR_COLD;
	uint16_t player_input_r(offs_t offset);
	void sound_w(offs_t offset, uint16_t data);

	void main_map(address_map &map) ATTR_COLD;
	void slave_map(address_map &map) ATTR_COLD;
	void slave_io_map(address_map &map) ATTR_COLD;

	template <int N>
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

template <int N>
uint32_t predators_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int color_base = 16;
	uint8_t const *const vram = m_vram[N] + (m_screen_state[N].m_vram_page ? 0 : 0x10000);
	pen_t const *const paldata = m_palette[N]->pens();

	for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		uint8_t const *src = &vram[y * 512/2 + cliprect.min_x / 2];
		uint32_t *dest = &bitmap.pix(y, cliprect.min_x & ~1);

		for (int x = cliprect.min_x & ~1; x <= cliprect.max_x; x += 2)
		{
			uint8_t const color = *src++;

			*dest++ = paldata[((color >> 4) & 0x0f) + color_base];
			*dest++ = paldata[((color >> 0) & 0x0f) + color_base];
		}
	}
	return 0;
}

template <int N>
void predators_state::vblank_write_line(int state)
{
	if (!state)
	{
		screen_state &s_state = m_screen_state[N];

		if (s_state.m_swap)
		{
			s_state.m_vram_page ^= 1;
			s_state.m_swap = 0;
			draw_cmd_finished(N);
		}
	}
}

void predators_state::draw_cmd_finished(int screen)
{
	m_screen_state[screen].m_busy = 0;
	update_main_irqs();
}

TIMER_CALLBACK_MEMBER(predators_state::draw_complete)
{
	draw_cmd_finished(param);
}

void predators_state::write_4bpp_pixel(int screen, int x, int y, uint8_t pix, int wrap)
{
	constexpr int width = 512;
	constexpr int height = 256;
	screen_state &s_state = m_screen_state[screen];

	x -= 256;
	y -= 256;

	if (y < 0 || x < 0)
		return;

	if (y >= height || x >= width)
		return;

	if (x >= (s_state.m_xmin * 2) && x <= (s_state.m_xmax * 2))
	{
		pix &= 0xf;

		uint8_t *const vram = &m_vram[screen][s_state.m_vram_page ? 0x10000 : 0];

		int const offs = (y * width + x) / 2;

		if (x & 1)
			vram[offs] = (vram[offs] & 0xf0) | pix;
		else
			vram[offs] = (vram[offs] & 0x0f) | (pix << 4);
	}
}

/* TODO: This is inaccurate */
void predators_state::draw_line(int screen, int startx, int starty, int valctab, int valstab, int length, int color)
{
	// pre-increment?
	int drawx = valctab & 0x7f, drawy = valstab & 0x7f;

	for (int j = 0; j < length; ++j)
	{
		int const xscreen = (valctab & 0x80) ? (startx - (drawx >> 6)) : (startx + (drawx >> 6));
		int const yscreen = (valstab & 0x80) ? (starty - (drawy >> 6)) : (starty + (drawy >> 6));

		write_4bpp_pixel(screen, xscreen, yscreen, color, 0);

		drawx += valctab & 0x7f;
		drawy += valstab & 0x7f;

	}
}

void predators_state::request_vram_swap(int screen)
{
	screen_state &s_state = m_screen_state[screen];
	/*
	    VRAM bank swap on VBLANK?

	    Valid parameters:

	    * Size (always 1)
	*/
	s_state.m_swap = 1;
	s_state.m_busy = 1;
}

void predators_state::clear_vram(int screen)
{
	screen_state &s_state = m_screen_state[screen];
	/*
	    VRAM clear?

	    Valid parameters:

	    * X
	    * Size (always 1)
	    * Source (always 0)
	*/

	uint8_t *const vram = &m_vram[screen][s_state.m_vram_page ? 0x10000 : 0];

	/* Clear the active video page with respect to the clipping region */
	// TODO: Understand X parameter
	for (int y = 0; y < 256; ++y)
	{
		int bytes = (s_state.m_xmax - s_state.m_xmin) + 1;
		std::fill_n(&vram[y * 512 / 2 + s_state.m_xmin], bytes, 0);
	}

	s_state.m_busy = 1;

	m_blit_done_timer->adjust(attotime::from_usec(5), screen);
}

void predators_state::set_clipping_region(int screen)
{
	screen_state &s_state = m_screen_state[screen];
	/*
	    Set clipping region?

	    Valid parameters:

	    * X
	    * Y
	    * Size (always 1)
	*/
	s_state.m_xmax = s_state.m_reg1_xmax;
	s_state.m_xmin = s_state.m_reg2_xmin;

	s_state.m_busy = 1;
	m_blit_done_timer->adjust(attotime::from_usec(1), screen);
}

void predators_state::do_blit(int screen)
{
	screen_state &s_state = m_screen_state[screen];
	int blit_x = s_state.m_reg1_xmax;
	int blit_y = s_state.m_reg2_xmin;
	uint16_t blit_size = s_state.m_reg3_blit_size;
	uint16_t blit_src = s_state.m_reg4_blit_src;

	for (int i = 0; i < blit_size; ++i)
	{
		int const offset = blit_src + i;
		uint8_t const length = m_blit_length[offset];
		int8_t const extray = m_blit_yoffs[offset];
		uint8_t const attr = m_blit_attr[offset];
		int8_t const extrax = m_blit_xoffs[offset];
		uint8_t color;

		/* Colour data is packed into half sized rom with 2 * 4 bits in each byte */
		if ((blit_src + i) & 1)
		{
			color = m_blit_color[offset >> 1] & 0xf0;
			color >>= 4;
		}
		else
		{
			color = m_blit_color[offset >> 1] & 0x0f;
		}

		/* (relative to vertical orientation)
		        line drawing.. it's an actual angle!! upper 4 bits shown in diagram below

		    e     f    0   1      2
		    . -        -        -
		    .   \      |      /
		    d     \    |    /     3
		    .       \  |  /
		    m         \|/
		    c <------- o -------> 4
		    .         /|\
		    .       /  |  \
		    b     /    |    \     5
		    .   /      |      \
		    . -        -        -
		    a    9    8    7     6

		    note, the length attribute is NOT in pixels(!) it's the real line length
		*/

		int const valctab = m_sincos[screen][(attr & 0xff)];
		int const valstab = m_sincos[screen][(attr & 0xff) + 0x100];

		draw_line(screen, blit_x + extrax, blit_y + extray, valctab, valstab, length, color);
	}

	s_state.m_busy = 1;
	m_blit_done_timer->adjust(attotime::from_usec(5), screen);
}

void predators_state::execute_draw_cmd(int screen)
{
	screen_state &s_state = m_screen_state[screen];
	uint8_t const trigger = s_state.m_reg0_trigger;

	// 4 cases for each command but screen comes from offset
	// 4 work units?
	switch (trigger)
	{
	case 0x12: case 0x1a: case 0x22: case 0x2a:
		clear_vram(screen);
		break;

	case 0x32: case 0x37: case 0x3c: case 0x41:
		request_vram_swap(screen);
		break;

	case 0x46: case 0x48: case 0x4a: case 0x4c:
		set_clipping_region(screen);
		break;

	case 0x4e: case 0x56: case 0x5e: case 0x66:
		do_blit(screen);
		break;
	}
}

void predators_state::blit_param_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/*
	    Each screen has its own blit registers:

	    40,2,4,6 = Command and trigger (Byte)
	    50,2,4,6 = Dest. Y (Word)
	    60,2,4,6 = Dest. X (Word)
	    70,2,4,6 = Size (Byte)
	    80,2,4,6 = Image address (Word)
	*/

	int const screen = offset & 7;

	if (screen > 3)
	{
		logerror("invalid blit screen! %d\n", screen);
		return;
	}

	screen_state &s_state = m_screen_state[screen];

	int const byte_offset = (offset & 0xf8) << 1;

	switch (byte_offset)
	{
		case 0x40:
			s_state.m_reg0_trigger = data;
			execute_draw_cmd(screen);
			break;

		case 0x50:
			s_state.m_reg1_xmax = data;
			break;

		case 0x60:
			s_state.m_reg2_xmin = data;
			break;

		case 0x70:
			s_state.m_reg3_blit_size = data;
			break;

		case 0x80:
			s_state.m_reg4_blit_src = data;
			break;

		default:
			logerror("%04x %04x %04x\n", offset * 2, data, mem_mask);
			break;
	}
}

void predators_state::blit_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_screen_irq_mask = (data >> 8) & 0xf;
	update_main_irqs();
}

void predators_state::update_main_irqs(void)
{
	m_maincpu->set_input_line(2, ((m_screen_irq_mask & 1) && !m_screen_state[0].m_busy) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(3, ((m_screen_irq_mask & 2) && !m_screen_state[1].m_busy) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(4, ((m_screen_irq_mask & 4) && !m_screen_state[2].m_busy) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(5, ((m_screen_irq_mask & 8) && !m_screen_state[3].m_busy) ? ASSERT_LINE : CLEAR_LINE);
}



static INPUT_PORTS_START( predators )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Audit 4 (Condor)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Audit 3 (Eagle)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Audit 2 (Falcon)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Audit 1 (Hawk)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "MemLock" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "HighScore" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_BIT(           0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2)
	PORT_DIPNAME( 0x80, 0x80, "Auto Manual" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	// Hawk
	PORT_START("stickx0") // read at 0x800008, see williams_49way_port_hawk_r
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)  PORT_PLAYER(1) PORT_NAME("Player 1 (Hawk) Stick X")
	PORT_START("sticky0") // read at 0x800008, see williams_49way_port_hawk_r
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE  PORT_PLAYER(1) PORT_NAME("Player 1 (Hawk) Stick Y")

	PORT_START("general0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("%p (Hawk) Fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("%p (Hawk) Pod")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("%p (Hawk) Thrust")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // unused?
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN9 ) PORT_NAME("Player 1 (Hawk) Right Coin")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_NAME("Player 1 (Hawk) Center Coin")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Player 1 (Hawk) Left Coin")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT ) PORT_NAME("Player 1 (Hawk) Slam") // slam??

	// Falcon
	PORT_START("stickx1") // read at 0x80000c, see williams_49way_port_falcon_r
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)  PORT_PLAYER(2) PORT_NAME("Player 2 (Falcon) Stick X")
	PORT_START("sticky1") // read at 0x80000c, see williams_49way_port_falcon_r
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE  PORT_PLAYER(2) PORT_NAME("Player 2 (Falcon) Stick Y")

	PORT_START("general1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("%p (Falcon) Fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("%p (Falcon) Pod")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("%p (Falcon) Thrust")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // unused?
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN10) PORT_NAME("Player 2 (Falcon) Right Coin")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN6 ) PORT_NAME("Player 2 (Falcon) Center Coin")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("Player 2 (Falcon) Left Coin")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT ) PORT_NAME("Player 2 (Falcon) Slam") // slam??

	// Eagle
	PORT_START("stickx2") // read at 0x800010, see williams_49way_port_eagle_r
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)  PORT_PLAYER(3) PORT_NAME("Player 3 (Eagle) Stick X")
	PORT_START("sticky2") // read at 0x800010, see williams_49way_port_eagle_r
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE  PORT_PLAYER(3) PORT_NAME("Player 3 (Eagle) Stick Y")

	PORT_START("general2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("%p (Eagle) Fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("%p (Eagle) Pod")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(3) PORT_NAME("%p (Eagle) Thrust")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // unused?
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN11) PORT_NAME("Player 3 (Eagle) Right Coin")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN7 ) PORT_NAME("Player 3 (Eagle) Center Coin")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("Player 3 (Eagle) Left Coin")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT ) PORT_NAME("Player 3 (Eagle) Slam") // slam??

	// Condor
	PORT_START("stickx3") // read at 0x800014, see williams_49way_port_condor_r
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)  PORT_PLAYER(4) PORT_NAME("Player 4 (Condor) Stick X")
	PORT_START("sticky3") // read at 0x800014, see williams_49way_port_condor_r
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(4) PORT_NAME("Player 4 (Condor) Stick Y")

	PORT_START("general3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4) PORT_NAME("%p (Condor) Fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4) PORT_NAME("%p (Condor) Pod")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(4) PORT_NAME("%p (Condor) Thrust")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // unused?
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN12) PORT_NAME("Player 4 (Condor) Right Coin")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN8 ) PORT_NAME("Player 4 (Condor) Center Coin")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("Player 4 (Condor) Left Coin")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT ) PORT_NAME("Player 4 (Condor) Slam") // slam??
INPUT_PORTS_END

uint16_t predators_state::player_input_r(offs_t offset)
{
	if (offset & 1)
	{
		return m_general[offset >> 1]->read();
	}
	else
	{
		constexpr uint8_t translate49[7] = { 0x0, 0x1, 0x2, 0x7, 0xa, 0x9, 0x8 };
		return (translate49[m_sticky[offset >> 1]->read() >> 4] << 4) | translate49[m_stickx[offset >> 1]->read() >> 4];
	}
}

void predators_state::sound_w(offs_t offset, uint16_t data)
{
	m_predsnd[offset]->sound_w(data);
}

void predators_state::main_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();

	map(0x400000, 0x40000f).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write)).umask16(0x00ff);

	map(0x600000, 0x6000ff).w(FUNC(predators_state::blit_param_w));

	map(0x800000, 0x800001).portr("DSW");

	map(0x800008, 0x800017).r(FUNC(predators_state::player_input_r));

	map(0x800020, 0x80002f).w(FUNC(predators_state::sound_w));

	map(0xa00000, 0xa00001).w(FUNC(predators_state::blit_ctrl_w));

	map(0xc00000, 0xc003ff).ram().share("nvram");
	map(0xfe0000, 0xffffff).ram();
}

void predators_state::slave_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
}

void predators_state::slave_io_map(address_map &map)
{
	map(0x0000, 0x000f).ram();
}

void predators_state::screen_start(int screen)
{
	uint8_t *proms = m_proms[screen];

	for (int i = 0; i < 32; ++i)
	{
		int bit0, bit1, bit2, bit3;

		bit0 = BIT(proms[i], 0);
		bit1 = BIT(proms[i], 1);
		bit2 = BIT(proms[i], 2);
		bit3 = BIT(proms[i], 3);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = BIT(proms[i], 4);
		bit1 = BIT(proms[i], 5);
		bit2 = BIT(proms[i], 6);
		bit3 = BIT(proms[i], 7);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = BIT(proms[i + 0x20], 0);
		bit1 = BIT(proms[i + 0x20], 1);
		bit2 = BIT(proms[i + 0x20], 2);
		bit3 = BIT(proms[i + 0x20], 3);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		m_palette[screen]->set_pen_color(i, rgb_t(r, g, b));
	}
}


void predators_state::machine_start()
{
	save_item(STRUCT_MEMBER(m_screen_state, m_reg0_trigger));
	save_item(STRUCT_MEMBER(m_screen_state, m_reg1_xmax));
	save_item(STRUCT_MEMBER(m_screen_state, m_reg2_xmin));
	save_item(STRUCT_MEMBER(m_screen_state, m_reg3_blit_size));
	save_item(STRUCT_MEMBER(m_screen_state, m_reg4_blit_src));
	save_item(STRUCT_MEMBER(m_screen_state, m_xmin));
	save_item(STRUCT_MEMBER(m_screen_state, m_xmax));
	save_item(STRUCT_MEMBER(m_screen_state, m_vram_page));
	save_item(STRUCT_MEMBER(m_screen_state, m_busy));
	save_item(STRUCT_MEMBER(m_screen_state, m_swap));

	save_item(NAME(m_screen_irq_mask));

	screen_start(0);
	screen_start(1);
	screen_start(2);
	screen_start(3);

	m_blit_done_timer = timer_alloc(FUNC(predators_state::draw_complete), this);
}

void predators_state::machine_reset()
{
	for (auto &s_state : m_screen_state)
	{
		s_state.m_reg0_trigger = 0;
		s_state.m_reg1_xmax = 0;
		s_state.m_reg2_xmin = 0;
		s_state.m_reg3_blit_size = 0;
		s_state.m_reg4_blit_src = 0;
		s_state.m_xmin = 0;
		s_state.m_xmax = 0;
		s_state.m_vram_page = 0;
		s_state.m_busy = 0;
		s_state.m_swap = 0;
	}

	for (auto &vram : m_vram)
		std::fill(std::begin(vram), std::end(vram), 0);

	m_screen_irq_mask = 0;
}

void predators_state::predators(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(8'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &predators_state::main_map);

	PTM6840(config, m_ptm, XTAL(8'000'000)/10);
	m_ptm->irq_callback().set_inputline("maincpu", 1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	for (auto &screen : m_screen)
	{
		SCREEN(config, screen, SCREEN_TYPE_RASTER);
		screen->set_refresh_hz(60);
		screen->set_size(512, 256);
		screen->set_visarea(48, 436 - 1, 20, 252 - 1);
	}

	for (auto &palette : m_palette)
		PALETTE(config, palette).set_entries(32);

	m_screen[0]->set_screen_update(FUNC(predators_state::screen_update<0>));
	m_screen[0]->screen_vblank().set(FUNC(predators_state::vblank_write_line<0>));
	m_screen[1]->set_screen_update(FUNC(predators_state::screen_update<1>));
	m_screen[1]->screen_vblank().set(FUNC(predators_state::vblank_write_line<1>));
	m_screen[2]->set_screen_update(FUNC(predators_state::screen_update<2>));
	m_screen[2]->screen_vblank().set(FUNC(predators_state::vblank_write_line<2>));
	m_screen[3]->set_screen_update(FUNC(predators_state::screen_update<3>));
	m_screen[3]->screen_vblank().set(FUNC(predators_state::vblank_write_line<3>));

	if (USE_MONO_SPEAKERS)
	{
		SPEAKER(config, "hawk_mono").front_center();
		SPEAKER(config, "falcon_mono").front_center();
		SPEAKER(config, "eagle_mono").front_center();
		SPEAKER(config, "condor_mono").front_center();

		WILLIAMS_PREDATORS_SOUND(config, m_predsnd[0]).add_route(ALL_OUTPUTS, "hawk_mono", 1.0, 0); // hawk0
		WILLIAMS_PREDATORS_SOUND(config, m_predsnd[1]).add_route(ALL_OUTPUTS, "falcon_mono", 1.0, 0); // falcon0
		WILLIAMS_PREDATORS_SOUND(config, m_predsnd[2]).add_route(ALL_OUTPUTS, "eagle_mono", 1.0, 0); // eagle0
		WILLIAMS_PREDATORS_SOUND(config, m_predsnd[3]).add_route(ALL_OUTPUTS, "condor_mono", 1.0, 0); // condor0
		WILLIAMS_PREDATORS_SOUND(config, m_predsnd[4]).add_route(ALL_OUTPUTS, "hawk_mono", 1.0, 0); // hawk1
		WILLIAMS_PREDATORS_SOUND(config, m_predsnd[5]).add_route(ALL_OUTPUTS, "falcon_mono", 1.0, 0); // falcon1
		WILLIAMS_PREDATORS_SOUND(config, m_predsnd[6]).add_route(ALL_OUTPUTS, "eagle_mono", 1.0, 0); // eagle1
		WILLIAMS_PREDATORS_SOUND(config, m_predsnd[7]).add_route(ALL_OUTPUTS, "condor_mono", 1.0, 0); // condor1
	}
	else
	{
		SPEAKER(config, "hawk_stereo", 2).front();
		SPEAKER(config, "falcon_stereo", 2).front();
		SPEAKER(config, "eagle_stereo", 2).front();
		SPEAKER(config, "condor_stereo", 2).front();

		WILLIAMS_PREDATORS_SOUND(config, m_predsnd[0]).add_route(ALL_OUTPUTS, "hawk_stereo", 1.0, 0); // hawk0
		WILLIAMS_PREDATORS_SOUND(config, m_predsnd[1]).add_route(ALL_OUTPUTS, "falcon_stereo", 1.0, 0); // falcon0
		WILLIAMS_PREDATORS_SOUND(config, m_predsnd[2]).add_route(ALL_OUTPUTS, "eagle_stereo", 1.0, 0); // eagle0
		WILLIAMS_PREDATORS_SOUND(config, m_predsnd[3]).add_route(ALL_OUTPUTS, "condor_stereo", 1.0, 0); // condor0
		WILLIAMS_PREDATORS_SOUND(config, m_predsnd[4]).add_route(ALL_OUTPUTS, "hawk_stereo", 1.0, 1); // hawk1
		WILLIAMS_PREDATORS_SOUND(config, m_predsnd[5]).add_route(ALL_OUTPUTS, "falcon_stereo", 1.0, 1); // falcon1
		WILLIAMS_PREDATORS_SOUND(config, m_predsnd[6]).add_route(ALL_OUTPUTS, "eagle_stereo", 1.0, 1); // eagle1
		WILLIAMS_PREDATORS_SOUND(config, m_predsnd[7]).add_route(ALL_OUTPUTS, "condor_stereo", 1.0, 1); // condor1
	}

	I8035(config, m_i8035, XTAL(8'000'000)/4);
	m_i8035->set_addrmap(AS_PROGRAM, &predators_state::slave_map);
	m_i8035->set_addrmap(AS_IO, &predators_state::slave_io_map);
}


ROM_START( predtors )

	/**********************************************************************************************************************
	  ROMs from Main board below
	  Contains the 68k, Am2910, NVRAM etc.  drives all game logic etc.
	 *********************************************************************************************************************/

	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "u218.bin",   0x000000, 0x8000, CRC(5ed16dc8) SHA1(d50550d7d6e51c0c81972829f4d0a7e4cc20878d) )
	ROM_LOAD16_BYTE( "u219.bin",   0x000001, 0x8000, CRC(752c326b) SHA1(99afbc96c0984e0964937c753efcacf2ff2ade32) )
	ROM_LOAD16_BYTE( "u220.bin",   0x010000, 0x8000, CRC(e1371d17) SHA1(f7c3e9aeb35192d00d83c886a9bf8dc1ec2a5c15) )
	ROM_LOAD16_BYTE( "u237.bin",   0x010001, 0x8000, CRC(8714876b) SHA1(239ad5f28c45c2a2a995e5ed1712fd4da90bb9dc) )

	ROM_REGION( 0x10000, "blitlength", 0 )
	ROM_LOAD( "u201.bin",   0x0000, 0x8000, CRC(2ea40e11) SHA1(73512af40c6739c4061409ee12cf47b7b03b683f) )
	ROM_LOAD( "u203.bin",   0x8000, 0x8000, CRC(9982e4bd) SHA1(a2bbd9883e21bbf8e6d614d4e1ef60d66892236d) )

	ROM_REGION( 0x10000, "blityoffs", 0 )
	ROM_LOAD( "u202.bin",   0x0000, 0x8000, CRC(8deefb05) SHA1(0c1fbf9e1e846dc9bc68a0f8ad505bdec9c08df5) )
	ROM_LOAD( "u204.bin",   0x8000, 0x8000, CRC(0a24750a) SHA1(b40237c0b5165ed7fa30c40bfd42711441c1b8c7) )

	ROM_REGION( 0x10000, "blitattr", 0 )
	ROM_LOAD( "u221.bin",   0x0000, 0x8000, CRC(ce8d42ed) SHA1(7bc014fdfe64c7648ce550e969b23c46aaabace0) )
	ROM_LOAD( "u223.bin",   0x8000, 0x8000, CRC(9b66c1ca) SHA1(d70464fc457671682711eb47523c595ece5cf488) )

	ROM_REGION( 0x10000, "blitxoffs", 0 )
	ROM_LOAD( "u222.bin",   0x0000, 0x8000, CRC(b795c4c2) SHA1(7a6528516f6e28670f25068a293a12f360dc78a1) )
	ROM_LOAD( "u224.bin",   0x8000, 0x8000, CRC(bdba0ed9) SHA1(066f3ee60c1922b6b5f75a9d6083eddf7b673c9b) )

	ROM_REGION( 0x8000, "blitcolor", 0 )
	ROM_LOAD( "u240.bin",   0x0000, 0x8000, CRC(3828b6bc) SHA1(db985977f5d17b45f61c67e8f190581ddf8fee70) )

	//ROM_REGION( 0x117, "pal", 0 )
	//ROM_LOAD( "u254",   0x0000, 0x117, CRC(8ce4ff25) SHA1(9cbdd85b83eda7f1814e4c0930c4d47ec920a1bf) )

	ROM_REGION( 0x20000, "am2910", 0 ) // microcode for the am2910??
	ROM_LOAD( "u280.bin",   0x0000, 0x200, CRC(835d733a) SHA1(2ea0fe5698a34206e8fc0166e2af4e066d864866) )
	ROM_LOAD( "u281.bin",   0x0000, 0x200, CRC(700b24c1) SHA1(790aa04717e86828974afaa2ac743f37afa11021) )
	ROM_LOAD( "u282.bin",   0x0000, 0x200, CRC(a5005f9a) SHA1(b33ccdc433d299fe3495f5fb2a5e604d8712881f) )
	ROM_LOAD( "u283.bin",   0x0000, 0x200, CRC(8368a869) SHA1(86b2562106d894b63a529ae11c6d7a27347d3654) )

	/**********************************************************************************************************************
	  Roms from SC board below
	  Contains a single rom along with the i8035 and some other bare-bones componenets
	 *********************************************************************************************************************/

	ROM_REGION( 0x1000, "i8035", 0 )
	ROM_LOAD( "io_slave", 0x0000, 0x1000, CRC(ccd186b0) SHA1(822602f5141e326df8ed18e0436a0ec585b5cb9e) )


	/**********************************************************************************************************************
	  ROMs from Slave boards below
	  All slave boards appear to be identical, I've prefixed ROM names with the name of the player each board is for to
	  aid clarity here.
	 *********************************************************************************************************************/

	/* Sound Section */

	/* there are 8 of these.. 2 pairs on each of the 4 slave boards, all running the same program */
	ROM_REGION( 0x10000, "hawk0:cpu",   0 )
	ROM_LOAD( "hawk_u84.bin",     0x0000, 0x8000, CRC(49bf0180) SHA1(40ea3049dd35dc9b306fc516c7b814be025cd39f) )
	ROM_LOAD( "hawk_u83.bin",     0x8000, 0x8000, CRC(c34704ce) SHA1(853434087df664ed17e7043804fa9e961c5fe97f) )

	ROM_REGION( 0x10000, "hawk1:cpu",   0 )
	ROM_LOAD( "hawk_u95.bin",     0x0000, 0x8000, CRC(49bf0180) SHA1(40ea3049dd35dc9b306fc516c7b814be025cd39f) )
	ROM_LOAD( "hawk_u96.bin",     0x8000, 0x8000, CRC(c34704ce) SHA1(853434087df664ed17e7043804fa9e961c5fe97f) )

	ROM_REGION( 0x10000, "falcon0:cpu", 0 )
	ROM_LOAD( "falcon_u84.bin",   0x0000, 0x8000, CRC(49bf0180) SHA1(40ea3049dd35dc9b306fc516c7b814be025cd39f) )
	ROM_LOAD( "falcon_u83.bin",   0x8000, 0x8000, CRC(c34704ce) SHA1(853434087df664ed17e7043804fa9e961c5fe97f) )

	ROM_REGION( 0x10000, "falcon1:cpu", 0 )
	ROM_LOAD( "falcon_u95.bin",   0x0000, 0x8000, CRC(49bf0180) SHA1(40ea3049dd35dc9b306fc516c7b814be025cd39f) )
	ROM_LOAD( "falcon_u96.bin",   0x8000, 0x8000, CRC(c34704ce) SHA1(853434087df664ed17e7043804fa9e961c5fe97f) )

	ROM_REGION( 0x10000, "eagle0:cpu", 0 )
	ROM_LOAD( "eagle_u84.bin",   0x0000, 0x8000, CRC(49bf0180) SHA1(40ea3049dd35dc9b306fc516c7b814be025cd39f) )
	ROM_LOAD( "eagle_u83.bin",   0x8000, 0x8000, CRC(c34704ce) SHA1(853434087df664ed17e7043804fa9e961c5fe97f) )

	ROM_REGION( 0x10000, "eagle1:cpu", 0 )
	ROM_LOAD( "eagle_u95.bin",   0x0000, 0x8000, CRC(49bf0180) SHA1(40ea3049dd35dc9b306fc516c7b814be025cd39f) )
	ROM_LOAD( "eagle_u96.bin",   0x8000, 0x8000, CRC(c34704ce) SHA1(853434087df664ed17e7043804fa9e961c5fe97f) )

	ROM_REGION( 0x10000, "condor0:cpu", 0 )
	ROM_LOAD( "condor_u84.bin",   0x0000, 0x8000, CRC(49bf0180) SHA1(40ea3049dd35dc9b306fc516c7b814be025cd39f) )
	ROM_LOAD( "condor_u83.bin",   0x8000, 0x8000, CRC(c34704ce) SHA1(853434087df664ed17e7043804fa9e961c5fe97f) )

	ROM_REGION( 0x10000, "condor1:cpu", 0 )
	ROM_LOAD( "condor_u95.bin",   0x0000, 0x8000, CRC(49bf0180) SHA1(40ea3049dd35dc9b306fc516c7b814be025cd39f) )
	ROM_LOAD( "condor_u96.bin",   0x8000, 0x8000, CRC(c34704ce) SHA1(853434087df664ed17e7043804fa9e961c5fe97f) )

	/* Video section */

	/* color PROMs are on the slave boards, they're the same on all of them, but we load them once for each board anyway */
	ROM_REGION( 0x40, "proms0",   0 )
	ROM_LOAD( "hawk_u77.bin",     0x0000, 0x020, CRC(c9bc35f1) SHA1(78e76cbe72157b775cd24f36e1814e312d56af30) )
	ROM_LOAD( "hawk_u78.bin",     0x0020, 0x020, CRC(3fb02480) SHA1(ace4eeb3a9b81d0890d2eb5c4e3cc2e874cfa88c) )

	ROM_REGION( 0x40, "proms1", 0 )
	ROM_LOAD( "falcon_u77.bin",   0x0000, 0x020, CRC(c9bc35f1) SHA1(78e76cbe72157b775cd24f36e1814e312d56af30) )
	ROM_LOAD( "falcon_u78.bin",   0x0020, 0x020, CRC(3fb02480) SHA1(ace4eeb3a9b81d0890d2eb5c4e3cc2e874cfa88c) )

	ROM_REGION( 0x40, "proms2", 0 )
	ROM_LOAD( "eagle_u77.bin",    0x0000, 0x020, CRC(c9bc35f1) SHA1(78e76cbe72157b775cd24f36e1814e312d56af30) )
	ROM_LOAD( "eagle_u78.bin",    0x0020, 0x020, CRC(3fb02480) SHA1(ace4eeb3a9b81d0890d2eb5c4e3cc2e874cfa88c) )

	ROM_REGION( 0x40, "proms3", 0 )
	ROM_LOAD( "condor_u77.bin",   0x0000, 0x020, CRC(c9bc35f1) SHA1(78e76cbe72157b775cd24f36e1814e312d56af30) )
	ROM_LOAD( "condor_u78.bin",   0x0020, 0x020, CRC(3fb02480) SHA1(ace4eeb3a9b81d0890d2eb5c4e3cc2e874cfa88c) )

	/* these PROMs are are also on the slave boards, they're the same on all of them, but we load them once for each board anyway */
	ROM_REGION( 0x400, "sincos0",   0 )
	ROM_LOAD( "hawk_u4.bin",     0x0000, 0x200, CRC(f8aeaa23) SHA1(f1245705a65795be1c15423fda99b3cf2c09200a) )
	ROM_LOAD( "hawk_u5.bin",     0x0200, 0x200, CRC(f8aeaa23) SHA1(f1245705a65795be1c15423fda99b3cf2c09200a) )

	ROM_REGION( 0x400, "sincos1", 0 )
	ROM_LOAD( "falcon_u4.bin",   0x0000, 0x200, CRC(f8aeaa23) SHA1(f1245705a65795be1c15423fda99b3cf2c09200a) )
	ROM_LOAD( "falcon_u5.bin",   0x0200, 0x200, CRC(f8aeaa23) SHA1(f1245705a65795be1c15423fda99b3cf2c09200a) )

	ROM_REGION( 0x400, "sincos2", 0 )
	ROM_LOAD( "eagle_u4.bin",    0x0000, 0x200, CRC(f8aeaa23) SHA1(f1245705a65795be1c15423fda99b3cf2c09200a) )
	ROM_LOAD( "eagle_u5.bin",    0x0200, 0x200, CRC(f8aeaa23) SHA1(f1245705a65795be1c15423fda99b3cf2c09200a) )

	ROM_REGION( 0x400, "sincos3", 0 )
	ROM_LOAD( "condor_u4.bin",   0x0000, 0x200, CRC(f8aeaa23) SHA1(f1245705a65795be1c15423fda99b3cf2c09200a) )
	ROM_LOAD( "condor_u5.bin",   0x0200, 0x200, CRC(f8aeaa23) SHA1(f1245705a65795be1c15423fda99b3cf2c09200a) )
ROM_END

} // anonymous namespace

GAME( 1986, predtors, 0, predators, predators, predators_state, empty_init, ROT270, "Williams / Q Video System", "The Predators (prototype)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
