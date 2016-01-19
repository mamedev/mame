// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Aaron Giles, R. Belmont, Jonathan Gevaryahu
/***************************************************************************

    Berzerk hardware

    Driver by Zsolt Vasvari
    Original sound driver by Alex Judd
    New sound driver by Aaron Giles, R. Belmont and Lord Nightmare

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "audio/exidy.h"
#include "machine/74181.h"
#include "machine/nvram.h"
#include "sound/s14001a.h"
#include "video/resnet.h"


class berzerk_state : public driver_device
{
public:
	berzerk_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_s14001a(*this, "speech"),
		m_ls181_10c(*this, "ls181_10c"),
		m_ls181_12c(*this, "ls181_12c"),
		m_custom(*this, "exidy"),
		m_screen(*this, "screen"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<s14001a_device> m_s14001a;
	required_device<ttl74181_device> m_ls181_10c;
	required_device<ttl74181_device> m_ls181_12c;
	required_device<exidy_sound_device> m_custom;
	required_device<screen_device> m_screen;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	UINT8 m_magicram_control;
	UINT8 m_last_shift_data;
	UINT8 m_intercept;
	emu_timer *m_irq_timer;
	emu_timer *m_nmi_timer;
	UINT8 m_irq_enabled;
	UINT8 m_nmi_enabled;
	int m_p1_counter_74ls161;
	int m_p1_direction;
	int m_p2_counter_74ls161;
	int m_p2_direction;

	DECLARE_READ8_MEMBER(led_on_r);
	DECLARE_WRITE8_MEMBER(led_on_w);
	DECLARE_READ8_MEMBER(led_off_r);
	DECLARE_WRITE8_MEMBER(led_off_w);
	DECLARE_WRITE8_MEMBER(irq_enable_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_WRITE8_MEMBER(nmi_disable_w);
	DECLARE_READ8_MEMBER(nmi_enable_r);
	DECLARE_READ8_MEMBER(nmi_disable_r);
	DECLARE_WRITE8_MEMBER(magicram_w);
	DECLARE_WRITE8_MEMBER(magicram_control_w);
	DECLARE_READ8_MEMBER(intercept_v256_r);
	DECLARE_WRITE8_MEMBER(audio_w);
	DECLARE_READ8_MEMBER(audio_r);
	DECLARE_READ8_MEMBER(moonwarp_p1_r);
	DECLARE_READ8_MEMBER(moonwarp_p2_r);

	DECLARE_DRIVER_INIT(moonwarp);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void sound_reset() override;
	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	TIMER_CALLBACK_MEMBER(irq_callback);
	TIMER_CALLBACK_MEMBER(nmi_callback);
	void vpos_to_vsync_chain_counter(int vpos, UINT8 *counter, UINT8 *v256);
	int vsync_chain_counter_to_vpos(UINT8 counter, UINT8 v256);
	void create_irq_timer();
	void start_irq_timer();
	void create_nmi_timer();
	void start_nmi_timer();
	void get_pens(rgb_t *pens);
};


#define MASTER_CLOCK                (XTAL_10MHz)
#define MAIN_CPU_CLOCK              (MASTER_CLOCK / 4)
#define PIXEL_CLOCK                 (MASTER_CLOCK / 2)
#define S14001_CLOCK                (MASTER_CLOCK / 4)
#define HTOTAL                      (0x140)
#define HBEND                       (0x000)
#define HBSTART                     (0x100)
#define VTOTAL                      (0x106)
#define VBEND                       (0x020)
#define VBSTART                     (0x100)
#define VCOUNTER_START_NO_VBLANK    (0x020)
#define VCOUNTER_START_VBLANK       (0x0da)
#define IRQS_PER_FRAME              (2)
#define NMIS_PER_FRAME              (8)

static const UINT8 irq_trigger_counts[IRQS_PER_FRAME] = { 0x80, 0xda };
static const UINT8 irq_trigger_v256s [IRQS_PER_FRAME] = { 0x00, 0x01 };

static const UINT8 nmi_trigger_counts[NMIS_PER_FRAME] = { 0x30, 0x50, 0x70, 0x90, 0xb0, 0xd0, 0xf0, 0xf0 };
static const UINT8 nmi_trigger_v256s [NMIS_PER_FRAME] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };


/*************************************
 *
 *  Start LED handling
 *
 *************************************/

READ8_MEMBER(berzerk_state::led_on_r)
{
	output().set_led_value(0, 1);

	return 0;
}


WRITE8_MEMBER(berzerk_state::led_on_w)
{
	output().set_led_value(0, 1);
}


READ8_MEMBER(berzerk_state::led_off_r)
{
	output().set_led_value(0, 0);

	return 0;
}


WRITE8_MEMBER(berzerk_state::led_off_w)
{
	output().set_led_value(0, 0);
}



/*************************************
 *
 *  Convert to/from our line counting
 *  to the hardware's vsync chain
 *
 *************************************/

void berzerk_state::vpos_to_vsync_chain_counter(int vpos, UINT8 *counter, UINT8 *v256)
{
	/* convert from a vertical position to the actual values on the vertical sync counters */
	*v256 = ((vpos < VBEND) || (vpos >= VBSTART));

	if (*v256)
	{
		int temp = vpos - VBSTART + VCOUNTER_START_VBLANK;

		if (temp < 0)
			*counter = temp + VTOTAL;
		else
			*counter = temp;
	}
	else
		*counter = vpos;
}


int berzerk_state::vsync_chain_counter_to_vpos(UINT8 counter, UINT8 v256)
{
	/* convert from the vertical sync counters to an actual vertical position */
	int vpos;

	if (v256)
	{
		vpos = counter - VCOUNTER_START_VBLANK + VBSTART;

		if (vpos >= VTOTAL)
			vpos = vpos - VTOTAL;
	}
	else
		vpos = counter;

	return vpos;
}



/*************************************
 *
 *  IRQ generation
 *
 *  There are two IRQ's per frame
 *
 *************************************/

WRITE8_MEMBER(berzerk_state::irq_enable_w)
{
	m_irq_enabled = data & 0x01;
}


TIMER_CALLBACK_MEMBER(berzerk_state::irq_callback)
{
	int irq_number = param;
	UINT8 next_counter;
	UINT8 next_v256;
	int next_vpos;
	int next_irq_number;

	/* set the IRQ line if enabled */
	if (m_irq_enabled)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xfc);

	/* set up for next interrupt */
	next_irq_number = (irq_number + 1) % IRQS_PER_FRAME;
	next_counter = irq_trigger_counts[next_irq_number];
	next_v256 = irq_trigger_v256s[next_irq_number];

	next_vpos = vsync_chain_counter_to_vpos(next_counter, next_v256);
	m_irq_timer->adjust(m_screen->time_until_pos(next_vpos), next_irq_number);
}


void berzerk_state::create_irq_timer()
{
	m_irq_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(berzerk_state::irq_callback),this));
}


void berzerk_state::start_irq_timer()
{
	int vpos = vsync_chain_counter_to_vpos(irq_trigger_counts[0], irq_trigger_v256s[0]);
	m_irq_timer->adjust(m_screen->time_until_pos(vpos));
}



/*************************************
 *
 *  NMI generation
 *
 *  An NMI is asserted roughly every
 *  32 scanlines when V16 clocks HI.
 *  The NMI is cleared 2 pixels later.
 *  Since this happens so quickly, I am
 *  not emulating it, just pulse
 *  the line instead.
 *
 *************************************/

WRITE8_MEMBER(berzerk_state::nmi_enable_w)
{
	m_nmi_enabled = 1;
}


WRITE8_MEMBER(berzerk_state::nmi_disable_w)
{
	m_nmi_enabled = 0;
}


READ8_MEMBER(berzerk_state::nmi_enable_r)
{
	m_nmi_enabled = 1;

	return 0;
}


READ8_MEMBER(berzerk_state::nmi_disable_r)
{
	m_nmi_enabled = 0;

	return 0;
}


TIMER_CALLBACK_MEMBER(berzerk_state::nmi_callback)
{
	int nmi_number = param;
	UINT8 next_counter;
	UINT8 next_v256;
	int next_vpos;
	int next_nmi_number;

	/* pulse the NMI line if enabled */
	if (m_nmi_enabled)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);

	/* set up for next interrupt */
	next_nmi_number = (nmi_number + 1) % NMIS_PER_FRAME;
	next_counter = nmi_trigger_counts[next_nmi_number];
	next_v256 = nmi_trigger_v256s[next_nmi_number];

	next_vpos = vsync_chain_counter_to_vpos(next_counter, next_v256);
	m_nmi_timer->adjust(m_screen->time_until_pos(next_vpos), next_nmi_number);
}


void berzerk_state::create_nmi_timer()
{
	m_nmi_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(berzerk_state::nmi_callback),this));
}


void berzerk_state::start_nmi_timer()
{
	int vpos = vsync_chain_counter_to_vpos(nmi_trigger_counts[0], nmi_trigger_v256s[0]);
	m_nmi_timer->adjust(m_screen->time_until_pos(vpos));
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

void berzerk_state::machine_start()
{
	create_irq_timer();
	create_nmi_timer();

	/* register for state saving */
	save_item(NAME(m_magicram_control));
	save_item(NAME(m_last_shift_data));
	save_item(NAME(m_intercept));
	save_item(NAME(m_irq_enabled));
	save_item(NAME(m_nmi_enabled));
}



/*************************************
 *
 *  Machine reset
 *
 *************************************/

void berzerk_state::machine_reset()
{
	m_irq_enabled = 0;
	m_nmi_enabled = 0;
	output().set_led_value(0, 0);
	m_magicram_control = 0;

	start_irq_timer();
	start_nmi_timer();
}



/*************************************
 *
 *  Video system
 *
 *************************************/

void berzerk_state::video_start()
{
	m_ls181_10c->mode_w(1);
	m_ls181_12c->mode_w(1);
}


WRITE8_MEMBER(berzerk_state::magicram_w)
{
	UINT8 alu_output;

	UINT8 current_video_data = m_videoram[offset];

	/* shift data towards LSB.  MSB bits are filled by data from last_shift_data.
	   The shifter consists of 5 74153 devices @ 7A, 8A, 9A, 10A and 11A,
	   followed by 4 more 153's at 11B, 10B, 9B and 8B, which optionally
	   reverse the order of the resulting bits */
	UINT8 shift_flop_output = (((UINT16)m_last_shift_data << 8) | data) >> (m_magicram_control & 0x07);

	if (m_magicram_control & 0x08)
		shift_flop_output = BITSWAP8(shift_flop_output, 0, 1, 2, 3, 4, 5, 6, 7);

	/* collision detection - AND gate output goes to the K pin of the flip-flop,
	   while J is LO, therefore, it only resets, never sets */
	if (shift_flop_output & current_video_data)
		m_intercept = 0;

	/* perform ALU step */
	m_ls181_12c->input_a_w(shift_flop_output >> 0);
	m_ls181_10c->input_a_w(shift_flop_output >> 4);
	m_ls181_12c->input_b_w(current_video_data >> 0);
	m_ls181_10c->input_b_w(current_video_data >> 4);
	m_ls181_12c->select_w(m_magicram_control >> 4);
	m_ls181_10c->select_w(m_magicram_control >> 4);

	alu_output = m_ls181_10c->function_r() << 4 | m_ls181_12c->function_r();

	m_videoram[offset] = alu_output ^ 0xff;

	/* save data for next time */
	m_last_shift_data = data & 0x7f;
}


WRITE8_MEMBER(berzerk_state::magicram_control_w)
{
	/* save the control byte, clear the shift data latch,
	   and set the intercept flip-flop */
	m_magicram_control = data;
	m_last_shift_data = 0;
	m_intercept = 1;
}


READ8_MEMBER(berzerk_state::intercept_v256_r)
{
	UINT8 counter;
	UINT8 v256;

	vpos_to_vsync_chain_counter(m_screen->vpos(), &counter, &v256);

	return (m_intercept^1) << 7 | v256;
}


void berzerk_state::get_pens(rgb_t *pens)
{
	static const int resistances_wg[] = { 750, 0 };
	static const int resistances_el[] = { static_cast<int>(1.0 / ((1.0 / 750.0) + (1.0 / 360.0))), 0 };

	double color_weights[2];

	if (ioport("MONITOR_TYPE")->read() == 0)
		compute_resistor_weights(0, 0xff, -1.0,
									2, resistances_wg, color_weights, 0, 270,
									2, resistances_wg, color_weights, 0, 270,
									2, resistances_wg, color_weights, 0, 270);
	else
		compute_resistor_weights(0, 0xff, -1.0,
									2, resistances_el, color_weights, 0, 270,
									2, resistances_el, color_weights, 0, 270,
									2, resistances_el, color_weights, 0, 270);

	for (int color = 0; color < 0x10; color++)
	{
		UINT8 r_bit = (color >> 0) & 0x01;
		UINT8 g_bit = (color >> 1) & 0x01;
		UINT8 b_bit = (color >> 2) & 0x01;
		UINT8 i_bit = (color >> 3) & 0x01;

		UINT8 r = combine_2_weights(color_weights, r_bit & i_bit, r_bit);
		UINT8 g = combine_2_weights(color_weights, g_bit & i_bit, g_bit);
		UINT8 b = combine_2_weights(color_weights, b_bit & i_bit, b_bit);

		pens[color] = rgb_t(r, g, b);
	}
}


UINT32 berzerk_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rgb_t pens[0x10];
	get_pens(pens);

	for (int offs = 0; offs < m_videoram.bytes(); offs++)
	{
		int i;

		UINT8 data = m_videoram[offs];
		UINT8 color = m_colorram[((offs >> 2) & 0x07e0) | (offs & 0x001f)];

		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		for (i = 0; i < 4; i++)
		{
			rgb_t pen = (data & 0x80) ? pens[color >> 4] : rgb_t::black;
			bitmap.pix32(y, x) = pen;

			x = x + 1;
			data = data << 1;
		}

		for (; i < 8; i++)
		{
			rgb_t pen = (data & 0x80) ? pens[color & 0x0f] : rgb_t::black;
			bitmap.pix32(y, x) = pen;

			x = x + 1;
			data = data << 1;
		}
	}

	return 0;
}



/*************************************
 *
 *  Audio system
 *
 *************************************/

WRITE8_MEMBER(berzerk_state::audio_w)
{
	switch (offset)
	{
	/* offset 4 writes to the S14001A */
	case 4:
		switch (data >> 6)
		{
		/* write data to the S14001 */
		case 0:
			/* only if not busy */
			if (!m_s14001a->bsy_r())
			{
				m_s14001a->reg_w(data & 0x3f);

				/* clock the chip -- via a 555 timer */
				m_s14001a->rst_w(1);
				m_s14001a->rst_w(0);
			}

			break;

		case 1:
		{
			/* volume */
			m_s14001a->set_volume(((data & 0x38) >> 3) + 1);

			/* clock control - the first LS161 divides the clock by 9 to 16, the 2nd by 8,
			   giving a final clock from 19.5kHz to 34.7kHz */
			int clock_divisor = 16 - (data & 0x07);

			m_s14001a->set_clock(S14001_CLOCK / clock_divisor / 8);
			break;
		}

		default: break; /* 2 and 3 are not connected */
		}

		break;

	/* offset 6 writes to the sfxcontrol latch */
	case 6:
		m_custom->sfxctrl_w(space, data >> 6, data);
		break;

	/* everything else writes to the 6840 */
	default:
		m_custom->sh6840_w(space, offset, data);
		break;
	}
}


READ8_MEMBER(berzerk_state::audio_r)
{
	switch (offset)
	{
	/* offset 4 reads from the S14001A */
	case 4:
		return (!m_s14001a->bsy_r()) ? 0x40 : 0x00;
	/* offset 6 is open bus */
	case 6:
		logerror("attempted read from berzerk audio reg 6 (sfxctrl)!\n");
		return 0;
	/* everything else reads from the 6840 */
	default:
		return m_custom->sh6840_r(space, offset);
	}
}



void berzerk_state::sound_reset()
{
	address_space &space = m_maincpu->space(AS_IO);
	/* clears the flip-flop controlling the volume and freq on the speech chip */
	audio_w(space, 4, 0x40);
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( berzerk_map, AS_PROGRAM, 8, berzerk_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x0800, 0x0bff) AM_MIRROR(0x0400) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x1000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x5fff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x6000, 0x7fff) AM_RAM_WRITE(magicram_w) AM_SHARE("videoram")
	AM_RANGE(0x8000, 0x87ff) AM_MIRROR(0x3800) AM_RAM AM_SHARE("colorram")
	AM_RANGE(0xc000, 0xffff) AM_NOP
ADDRESS_MAP_END


static ADDRESS_MAP_START( frenzy_map, AS_PROGRAM, 8, berzerk_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x5fff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x6000, 0x7fff) AM_RAM_WRITE(magicram_w) AM_SHARE("videoram")
	AM_RANGE(0x8000, 0x87ff) AM_MIRROR(0x3800) AM_RAM AM_SHARE("colorram")
	AM_RANGE(0xc000, 0xcfff) AM_ROM
	AM_RANGE(0xf800, 0xfbff) AM_MIRROR(0x0400) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END



/*************************************
 *
 *  Port handlers
 *
 *************************************/

static ADDRESS_MAP_START( berzerk_io_map, AS_IO, 8, berzerk_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x3f) AM_NOP
	AM_RANGE(0x40, 0x47) AM_READWRITE(audio_r, audio_w)
	AM_RANGE(0x48, 0x48) AM_READ_PORT("P1") AM_WRITENOP
	AM_RANGE(0x49, 0x49) AM_READ_PORT("SYSTEM") AM_WRITENOP
	AM_RANGE(0x4a, 0x4a) AM_READ_PORT("P2") AM_WRITENOP
	AM_RANGE(0x4b, 0x4b) AM_READNOP AM_WRITE(magicram_control_w)
	AM_RANGE(0x4c, 0x4c) AM_READWRITE(nmi_enable_r, nmi_enable_w)
	AM_RANGE(0x4d, 0x4d) AM_READWRITE(nmi_disable_r, nmi_disable_w)
	AM_RANGE(0x4e, 0x4e) AM_READ(intercept_v256_r) AM_WRITENOP // note reading from here should clear pending frame interrupts, see zfb-1.tiff 74ls74 at 3D pin 13 /CLR
	AM_RANGE(0x4f, 0x4f) AM_READNOP AM_WRITE(irq_enable_w)
	AM_RANGE(0x50, 0x57) AM_NOP /* second sound board, initialized but not used */
	AM_RANGE(0x58, 0x5f) AM_NOP
	AM_RANGE(0x60, 0x60) AM_MIRROR(0x18) AM_READ_PORT("F3") AM_WRITENOP
	AM_RANGE(0x61, 0x61) AM_MIRROR(0x18) AM_READ_PORT("F2") AM_WRITENOP
	AM_RANGE(0x62, 0x62) AM_MIRROR(0x18) AM_READ_PORT("F6") AM_WRITENOP
	AM_RANGE(0x63, 0x63) AM_MIRROR(0x18) AM_READ_PORT("F5") AM_WRITENOP
	AM_RANGE(0x64, 0x64) AM_MIRROR(0x18) AM_READ_PORT("F4") AM_WRITENOP
	AM_RANGE(0x65, 0x65) AM_MIRROR(0x18) AM_READ_PORT("SW2") AM_WRITENOP
	AM_RANGE(0x66, 0x66) AM_MIRROR(0x18) AM_READWRITE(led_off_r, led_off_w)
	AM_RANGE(0x67, 0x67) AM_MIRROR(0x18) AM_READWRITE(led_on_r, led_on_w)
	AM_RANGE(0x80, 0xff) AM_NOP
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

#define BERZERK_COINAGE(CHUTE, DIPBANK) \
	PORT_DIPNAME( 0x0f, 0x00, "Coin "#CHUTE )  PORT_DIPLOCATION(#DIPBANK":1,2,3,4") \
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x0d, DEF_STR( 4C_3C ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x0e, DEF_STR( 4C_5C ) ) \
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(    0x0f, DEF_STR( 4C_7C ) ) \
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_5C ) ) \
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_7C ) ) \
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) ) \
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) ) \
	PORT_DIPSETTING(    0x07, "1 Coin/10 Credits" ) \
	PORT_DIPSETTING(    0x08, "1 Coin/14 Credits" )


static INPUT_PORTS_START( joystick ) // used on all games except moonwarp
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x60, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( common ) // used on all games
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	/* fake port for monitor type */
	PORT_START("MONITOR_TYPE")
	PORT_CONFNAME( 0x01, 0x00, "Monitor Type" )
	PORT_CONFSETTING(    0x00, "Wells-Gardner" )
	PORT_CONFSETTING(    0x01, "Electrohome" )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SW2")
	/* port for the 'bookkeeping reset' and 'bookkeeping' buttons;
	 * The 'bookkeeping reset' button is an actual button on the zpu-1000 and
	 * zpu-1001 pcbs, labeled 'S2' or 'SW2'. It is wired to bit 0.
	 * * pressing it while high scores are displayed will give a free game
	 *   without adding any coin info to the bookkeeping info in nvram.
	 * The 'bookkeeping' button is wired to the control panel, usually hidden
	 * underneath or only accessible through the coin door. Wired to bit 7.
	 * * It displays various bookkeeping statistics when pressed sequentially.
	 *   Pressing P1 fire (according to the manual) when stats are displayed
	 *   will clear the stat shown on screen.
	 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Free Game (not logged in bookkeeping)")
	PORT_BIT( 0x7e, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("Bookkeeping") PORT_CODE(KEYCODE_F1)
INPUT_PORTS_END

static INPUT_PORTS_START( berzerk )
	PORT_INCLUDE( joystick )
	PORT_INCLUDE( common )

	PORT_START("F2")
	PORT_DIPNAME( 0x03, 0x00, "Color Test" ) PORT_CODE(KEYCODE_F5) PORT_TOGGLE PORT_DIPLOCATION("F2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x03, DEF_STR( On ) )
	PORT_BIT( 0x3c, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("F2:7,8")
	PORT_DIPSETTING(    0xc0, "5000 and 10000" )
	PORT_DIPSETTING(    0x40, "5000" )
	PORT_DIPSETTING(    0x80, "10000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_START("F3")
	PORT_DIPNAME( 0x01, 0x00, "Input Test Mode" ) PORT_CODE(KEYCODE_F2) PORT_TOGGLE PORT_DIPLOCATION("F3:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Crosshair Pattern" ) PORT_CODE(KEYCODE_F4) PORT_TOGGLE PORT_DIPLOCATION("F3:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0x3c, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("F3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( German ) )
	PORT_DIPSETTING(    0x80, DEF_STR( French ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Spanish ) )

	PORT_START("F4")
	BERZERK_COINAGE(1, F4)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("F5")
	BERZERK_COINAGE(2, F5)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("F6")
	BERZERK_COINAGE(3, F6)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END

// this set has French speech roms, so default the language to French
static INPUT_PORTS_START( berzerkf )
	PORT_INCLUDE( berzerk )

	PORT_MODIFY("F3")
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Language ) ) PORT_DIPLOCATION("F3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( German ) )
	PORT_DIPSETTING(    0x80, DEF_STR( French ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Spanish ) )
INPUT_PORTS_END

// this set has German speech roms, so default the language to German
static INPUT_PORTS_START( berzerkg )
	PORT_INCLUDE( berzerk )

	PORT_MODIFY("F3")
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Language ) ) PORT_DIPLOCATION("F3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( German ) )
	PORT_DIPSETTING(    0x80, DEF_STR( French ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Spanish ) )
INPUT_PORTS_END

// this set has a Spanish speech rom, so default the language to Spanish
static INPUT_PORTS_START( berzerks )
	PORT_INCLUDE( berzerk )

	PORT_MODIFY("F3")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Language ) ) PORT_DIPLOCATION("F3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( German ) )
	PORT_DIPSETTING(    0x80, DEF_STR( French ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Spanish ) )
INPUT_PORTS_END

static INPUT_PORTS_START( frenzy )
	PORT_INCLUDE( joystick )
	PORT_INCLUDE( common )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) // frenzy lacks coin 3

	PORT_START("F2")
	/* Bit 0 does some more hardware tests. According to the manual, both bit 0 & 1 must be:
	   - ON for Signature Analysis (S.A.)
	   - OFF for game operation     */
	PORT_DIPNAME( 0x03, 0x00, "Hardware Tests" ) PORT_DIPLOCATION("F2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, "Color test" )
	PORT_DIPSETTING(    0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x03, "Signature Analysis" )
	PORT_DIPNAME( 0x04, 0x00, "Input Test Mode" ) PORT_CODE(KEYCODE_F2) PORT_TOGGLE PORT_DIPLOCATION("F2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Crosshair Pattern" ) PORT_CODE(KEYCODE_F4) PORT_TOGGLE PORT_DIPLOCATION("F2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_DIPLOCATION("F2:5,6,7,8")

	PORT_START("F3")
	PORT_DIPNAME( 0x0f, 0x03, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("F3:1,2,3,4")
	PORT_DIPSETTING(    0x01, "1000" )
	PORT_DIPSETTING(    0x02, "2000" )
	PORT_DIPSETTING(    0x03, "3000" )
	PORT_DIPSETTING(    0x04, "4000" )
	PORT_DIPSETTING(    0x05, "5000" )
	PORT_DIPSETTING(    0x06, "6000" )
	PORT_DIPSETTING(    0x07, "7000" )
	PORT_DIPSETTING(    0x08, "8000" )
	PORT_DIPSETTING(    0x09, "9000" )
	PORT_DIPSETTING(    0x0a, "10000" )
	PORT_DIPSETTING(    0x0b, "11000" )
	PORT_DIPSETTING(    0x0c, "12000" )
	PORT_DIPSETTING(    0x0d, "13000" )
	PORT_DIPSETTING(    0x0e, "14000" )
	PORT_DIPSETTING(    0x0f, "15000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_DIPLOCATION("F3:5,6")
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("F3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( German ) )
	PORT_DIPSETTING(    0x80, DEF_STR( French ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Spanish ) )

	/* The following 3 ports use all 8 bits, but I didn't feel like adding all 256 values :-) */
	PORT_START("F4")
	PORT_DIPNAME( 0xff, 0x01, "Coin Multiplier" ) PORT_DIPLOCATION("F4:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x09, "9" )
	PORT_DIPSETTING(    0x0a, "10" )
	PORT_DIPSETTING(    0x0b, "11" )
	PORT_DIPSETTING(    0x0c, "12" )
	PORT_DIPSETTING(    0x0d, "13" )
	PORT_DIPSETTING(    0x0e, "14" )
	PORT_DIPSETTING(    0x0f, "15" )
	PORT_DIPSETTING(    0xff, "255" )

	PORT_START("F5")
	PORT_DIPNAME( 0xff, 0x01, "Coins/Credit A" ) PORT_DIPLOCATION("F5:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(    0x00, "0 (invalid)" ) // Can't insert coins
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x09, "9" )
	PORT_DIPSETTING(    0x0a, "10" )
	PORT_DIPSETTING(    0x0b, "11" )
	PORT_DIPSETTING(    0x0c, "12" )
	PORT_DIPSETTING(    0x0d, "13" )
	PORT_DIPSETTING(    0x0e, "14" )
	PORT_DIPSETTING(    0x0f, "15" )
	PORT_DIPSETTING(    0xff, "255" )

	PORT_START("F6")
	PORT_DIPNAME( 0xff, 0x01, "Coins/Credit B" ) PORT_DIPLOCATION("F6:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(    0x00, "0 (invalid)" ) // Can't insert coins
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x09, "9" )
	PORT_DIPSETTING(    0x0a, "10" )
	PORT_DIPSETTING(    0x0b, "11" )
	PORT_DIPSETTING(    0x0c, "12" )
	PORT_DIPSETTING(    0x0d, "13" )
	PORT_DIPSETTING(    0x0e, "14" )
	PORT_DIPSETTING(    0x0f, "15" )
	PORT_DIPSETTING(    0xff, "255" )
INPUT_PORTS_END

READ8_MEMBER(berzerk_state::moonwarp_p1_r)
{
	// This seems to be the same type of dial as the later 'moon war 2' set uses
	// see http://www.cityofberwyn.com/schematics/stern/MoonWar_opto.tiff for schematic
	// I.e. a 74ls161 counts from 0 to 15 which is the absolute number of bars passed on the quadrature
	// one difference is it lacks the strobe input (does it?), which if not active causes
	// the dial input to go open bus. This is used in moon war 2 to switch between player 1
	// and player 2 dials, which share a single port. moonwarp uses separate ports for the dials.
	signed char dialread = ioport("P1_DIAL")->read();
	UINT8 ret;
	UINT8 buttons = (ioport("P1")->read()&0xe0);
	if (dialread < 0) m_p1_direction = 0;
	else if (dialread > 0) m_p1_direction = 0x10;
	m_p1_counter_74ls161 += abs(dialread);
	m_p1_counter_74ls161 &= 0xf;
	ret = m_p1_counter_74ls161 | m_p1_direction | buttons;
	//fprintf(stderr, "dialread1: %02x, p1_counter_74ls161: %02x, spinner ret is %02x\n", dialread, m_p1_counter_74ls161, ret);
	return ret;
}

READ8_MEMBER(berzerk_state::moonwarp_p2_r)
{
	// same as above, but for player 2 in cocktail mode
	signed char dialread = ioport("P2_DIAL")->read();
	UINT8 ret;
	UINT8 buttons = (ioport("P2")->read()&0xe0);
	if (dialread < 0) m_p2_direction = 0;
	else if (dialread > 0) m_p2_direction = 0x10;
	m_p2_counter_74ls161 += abs(dialread);
	m_p2_counter_74ls161 &= 0xf;
	ret = m_p2_counter_74ls161 | m_p2_direction | buttons;
	//fprintf(stderr, "dialread2: %02x, p2_counter_74ls161: %02x, spinner ret is %02x\n", dialread, m_p2_counter_74ls161, ret);
	return ret;
}

static INPUT_PORTS_START( moonwarp )
	PORT_INCLUDE( common )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // is wired high for upright harness, low for cocktail; if high, cocktail dipswitch is ignored.
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) // Hyper flip button is common for both players in cocktail mode.

	PORT_START("P1")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_SPECIAL ) // spinner/dial
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("P1_DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_RESET

	PORT_START("P2")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_SPECIAL ) // spinner/dial(cocktail)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("P2_DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_COCKTAIL PORT_RESET

	PORT_START("F2")
	PORT_DIPNAME( 0x03, 0x00, "Hardware Tests" ) PORT_DIPLOCATION("F2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, "Color test" )
	PORT_DIPSETTING(    0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x03, "Signature Analysis" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("F2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xF8, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("F2:4,5,6,7,8")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x08, "15000" )
	PORT_DIPSETTING(    0x10, "20000" )
	PORT_DIPSETTING(    0x20, "30000" )
	PORT_DIPSETTING(    0x40, "40000" )
	PORT_DIPSETTING(    0x80, "50000" )
	PORT_DIPSETTING(    0x18, "15000 (duplicate)" )
	PORT_DIPSETTING(    0x28, "15000 (duplicate)" )
	PORT_DIPSETTING(    0x30, "20000 (duplicate)" )
	PORT_DIPSETTING(    0x38, "15000 (duplicate)" )
	PORT_DIPSETTING(    0x48, "15000 (duplicate)" )
	PORT_DIPSETTING(    0x50, "20000 (duplicate)" )
	PORT_DIPSETTING(    0x58, "15000 (duplicate)" )
	PORT_DIPSETTING(    0x60, "30000 (duplicate)" )
	PORT_DIPSETTING(    0x68, "15000 (duplicate)" )
	PORT_DIPSETTING(    0x70, "20000 (duplicate)" )
	PORT_DIPSETTING(    0x78, "15000 (duplicate)" )
	PORT_DIPSETTING(    0x88, "15000 (duplicate)" )
	PORT_DIPSETTING(    0x90, "20000 (duplicate)" )
	PORT_DIPSETTING(    0x98, "15000 (duplicate)" )
	PORT_DIPSETTING(    0xa0, "30000 (duplicate)" )
	PORT_DIPSETTING(    0xa8, "15000 (duplicate)" )
	PORT_DIPSETTING(    0xb0, "20000 (duplicate)" )
	PORT_DIPSETTING(    0xb8, "15000 (duplicate)" )
	PORT_DIPSETTING(    0xc0, "40000 (duplicate)" )
	PORT_DIPSETTING(    0xc8, "15000 (duplicate)" )
	PORT_DIPSETTING(    0xd0, "20000 (duplicate)" )
	PORT_DIPSETTING(    0xd8, "15000 (duplicate)" )
	PORT_DIPSETTING(    0xe0, "30000 (duplicate)" )
	PORT_DIPSETTING(    0xe8, "15000 (duplicate)" )
	PORT_DIPSETTING(    0xf0, "20000 (duplicate)" )
	PORT_DIPSETTING(    0xf8, "15000 (duplicate)" )

	PORT_START("F3")
	PORT_DIPNAME( 0x01, 0x00, "Input Test Mode" ) PORT_CODE(KEYCODE_F2) PORT_TOGGLE PORT_DIPLOCATION("F3:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Crosshair Pattern" ) PORT_CODE(KEYCODE_F4) PORT_TOGGLE PORT_DIPLOCATION("F3:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("F3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Spinner Orientation" ) PORT_DIPLOCATION("F3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Reverse ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Standard ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("F3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("F3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("F3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( German ) )
	PORT_DIPSETTING(    0x80, DEF_STR( French ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Spanish ) )

	PORT_START("F4")
	BERZERK_COINAGE(1, F4)
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("F4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("F4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("F4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("F4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("F5")
	BERZERK_COINAGE(2, F5)
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("F5:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("F5:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("F5:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("F5:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("F6")
	BERZERK_COINAGE(3, F6)
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("F6:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("F6:6") // enemy spawn rate?
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("F6:7,8") // Difficulty select effectively chooses what level to start on, level 1, 2, 3, or 4 for very easy, easy, normal, and hard. Number here is added to the current level count (base 1) at $43be.
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hard ) )
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( berzerk, berzerk_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MAIN_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(berzerk_map)
	MCFG_CPU_IO_MAP(berzerk_io_map)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_TTL74181_ADD("ls181_10c")
	MCFG_TTL74181_ADD("ls181_12c")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(berzerk_state, screen_update)

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("speech", S14001A, S14001_CLOCK/16/8) /* placeholder - the clock is software controllable */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_ADD("exidy", EXIDY, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( frenzy, berzerk )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(frenzy_map)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

/*

Berzerk program labels follow this format:

BERZERK (C)    <-- rom socket silkscreened ROM0
RC31 1C
1980 STERN

So PCB location and silkscreen are as follows as they appear on an actual PCB:

1C <--> ROM0      1D <--> ROM1

3C <--> ROM6      3D <--> ROM2

5C <--> ROM5      5D <--> ROM3

                  6D <--> ROM4

NOTE: No known set uses ROM6

Sound roms for Berzerk / Frenzy have been found labeled as:

BERZERK        BERZERK
R VO 1C        R VO 2C
1980  STERN    1980  STERN

as well as

E169-1CVO      E169-2CVO
RVO 1C (-9)    RVO 2C (-9)
1982  STERN    1982  STERN

Both sets of roms contain the same data.

We need to have actual verfied rom labels for all the sets below (other then voice roms :-)

*/

ROM_START( berzerk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "berzerk_rc31_1c.rom0.1c", 0x0000, 0x0800, CRC(ca566dbc) SHA1(fae2647f12f1cd82826db61b53b116a5e0c9f995) )
	ROM_LOAD( "berzerk_rc31_1d.rom1.1d", 0x1000, 0x0800, CRC(7ba69fde) SHA1(69af170c4a39a3494dcd180737e5c87b455f9203) )
	ROM_LOAD( "berzerk_rc31_3d.rom2.3d", 0x1800, 0x0800, CRC(a1d5248b) SHA1(a0b7842f6a5f86c16d80d78e7012c78b3ea11d1d) )
	ROM_LOAD( "berzerk_rc31_5d.rom3.5d", 0x2000, 0x0800, CRC(fcaefa95) SHA1(07f849aa39f1e3db938187ffde4a46a588156ddc) )
	ROM_LOAD( "berzerk_rc31_6d.rom4.6d", 0x2800, 0x0800, CRC(1e35b9a0) SHA1(5a5e549ec0e4803ab2d1eac6b3e7171aedf28244) )
	ROM_LOAD( "berzerk_rc31_5c.rom5.5c", 0x3000, 0x0800, CRC(c8c665e5) SHA1(e9eca4b119549e0061384abf52327c14b0d56624) )
	ROM_FILL(                            0x3800, 0x0800, 0xff ) /* rom socket ROM6 at 3C is unpopulated */

	ROM_REGION( 0x01000, "speech", 0 ) /* voice data */
	ROM_LOAD( "berzerk_r_vo_1c.1c", 0x0000, 0x0800, CRC(2cfe825d) SHA1(f12fed8712f20fa8213f606c4049a8144bfea42e) )  /* VSU-1000 board */
	ROM_LOAD( "berzerk_r_vo_2c.2c", 0x0800, 0x0800, CRC(d2b6324e) SHA1(20a6611ad6ec19409ac138bdae7bdfaeab6c47cf) )  /* ditto */
ROM_END

ROM_START( berzerk1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom0.1c", 0x0000, 0x0800, CRC(5b7eb77d) SHA1(8de488e279036fe40d6fb4c0dde16075309342fd) ) /* is this set RC32 ? */
	ROM_LOAD( "rom1.1d", 0x1000, 0x0800, CRC(e58c8678) SHA1(a11f08448b457d690b270512c9f02fcf1e41d9e0) )
	ROM_LOAD( "rom2.3d", 0x1800, 0x0800, CRC(705bb339) SHA1(845191df90cd7d80f8fed3d2b69305301d921549) )
	ROM_LOAD( "rom3.5d", 0x2000, 0x0800, CRC(6a1936b4) SHA1(f1635e9d2f25514c35559d2a247c3bc4b4034c19) )
	ROM_LOAD( "rom4.6d", 0x2800, 0x0800, CRC(fa5dce40) SHA1(b3a3ee52bf65bbb3a20f905d3e4ebdf6871dcb5d) )
	ROM_LOAD( "rom5.5c", 0x3000, 0x0800, CRC(2579b9f4) SHA1(890f0237afbb194166eae88c98de81989f408548) )
	ROM_FILL(            0x3800, 0x0800, 0xff ) /* rom socket ROM6 at 3C is unpopulated */

	ROM_REGION( 0x01000, "speech", 0 ) /* voice data */
	ROM_LOAD( "berzerk_r_vo_1c.1c", 0x0000, 0x0800, CRC(2cfe825d) SHA1(f12fed8712f20fa8213f606c4049a8144bfea42e) )  /* VSU-1000 board */
	ROM_LOAD( "berzerk_r_vo_2c.2c", 0x0800, 0x0800, CRC(d2b6324e) SHA1(20a6611ad6ec19409ac138bdae7bdfaeab6c47cf) )  /* ditto */
ROM_END

ROM_START( berzerkf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "berzerk_rc31f_1c.rom0.1c", 0x0000, 0x0800, CRC(3ba6e56e) SHA1(f2b02dcdc3fe1de28cace39055a88f6aa0798fd1) )
	ROM_LOAD( "berzerk_rc31f_1d.rom1.1d", 0x1000, 0x0800, CRC(a1de2a3e) SHA1(86ac3717ec26aeb2632583a65de6a0c2e7ea7419) )
	ROM_LOAD( "berzerk_rc31f_3d.rom2.3d", 0x1800, 0x0800, CRC(bc31c478) SHA1(906d0acdee208a0bf714bd06be99321722b531c6) )
	ROM_LOAD( "berzerk_rc31f_5d.rom3.5d", 0x2000, 0x0800, CRC(316192b5) SHA1(50f4ba2b59423a48c1d51fc6e4d9ea098d6f3743) )
	ROM_LOAD( "berzerk_rc31f_6d.rom4.6d", 0x2800, 0x0800, CRC(cd51238c) SHA1(f0b65bdd1f225c151a93ea62812b4bb64969acac) )
	ROM_LOAD( "berzerk_rc31f_5c.rom5.5c", 0x3000, 0x0800, CRC(563b13b6) SHA1(f8d137cd26535efe92780560d2f69f12d3f0fa42) )
	ROM_FILL(                             0x3800, 0x0800, 0xff ) /* rom socket ROM6 at 3C is unpopulated */

	ROM_REGION( 0x01000, "speech", 0 ) /* voice data */
	ROM_LOAD( "berzerk_rvof_1c.1c", 0x0000, 0x0800, CRC(d7bfaca2) SHA1(b8c22db0f6e86d90f3c2ac9ff9e9d0ccff314919) )    /* VSU-1000 board */
	ROM_LOAD( "berzerk_rvof_2c.2c", 0x0800, 0x0800, CRC(7bdc3573) SHA1(f346f0ac9813812f2e3fe68ebbf79151975babcb) )    /* ditto */
ROM_END

ROM_START( berzerkg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cpu rom 00.1c", 0x0000, 0x0800, CRC(77923a9e) SHA1(3760800b7aa1245f2141897b2406f0f5af5a8d71) )
	ROM_LOAD( "cpu rom 01.1d", 0x1000, 0x0800, CRC(19bb3aac) SHA1(11341521fd880d55ea01bceb4a321ec571f0b759) )
	ROM_LOAD( "cpu rom 02.3d", 0x1800, 0x0800, CRC(b0888ff7) SHA1(ac76400482fe37b6c8e309cd9b10855dac86ed24) )
	ROM_LOAD( "cpu rom 03.5d", 0x2000, 0x0800, CRC(e23239a9) SHA1(a0505efdee4cb1962243638c641e94983673f70f) )
	ROM_LOAD( "cpu rom 04.6d", 0x2800, 0x0800, CRC(651b31b7) SHA1(890f424a5a73a95af642435c1b0cca78a9413aae) )
	ROM_LOAD( "cpu rom 05.5c", 0x3000, 0x0800, CRC(8a403bba) SHA1(686a9b58a245df6c947d14991a2e4cbaf511e2ca) )
	ROM_FILL(                  0x3800, 0x0800, 0xff ) /* rom socket ROM6 at 3C is unpopulated */

	ROM_REGION( 0x01000, "speech", 0 ) /* voice data */
	ROM_LOAD( "berzerk_rvog_1c.1c", 0x0000, 0x0800, CRC(fc1da15f) SHA1(f759a017d9e95acf0e1d35b16d8820acee7d7e3d) )    /* VSU-1000 board */
	ROM_LOAD( "berzerk_rvog_2c.2c", 0x0800, 0x0800, CRC(7f6808fb) SHA1(8a9c43597f924221f68d1b31e033f1dc492cddc5) )    /* ditto */
ROM_END

ROM_START( berzerks )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "berzerk_rc32_1c.rom0.1c",  0x0000, 0x0800, CRC(77923a9e) SHA1(3760800b7aa1245f2141897b2406f0f5af5a8d71) ) /* Same as the German set */
	ROM_LOAD( "berzerk_rc32_1d.rom1.1d",  0x1000, 0x0800, CRC(19bb3aac) SHA1(11341521fd880d55ea01bceb4a321ec571f0b759) ) /* Same as the German set */
	ROM_LOAD( "berzerk_rc32_3d.rom2.3d",  0x1800, 0x0800, CRC(5423ea87) SHA1(c49f81f0dee4479965023aac15daac10bbbefe65) )
	ROM_LOAD( "berzerk_rc32_5d.rom3.5d",  0x2000, 0x0800, CRC(e23239a9) SHA1(a0505efdee4cb1962243638c641e94983673f70f) ) /* Same as the German set */
	ROM_LOAD( "berzerk_rc32_6d.rom4.6d",  0x2800, 0x0800, CRC(959efd86) SHA1(3401f86ed6202e8790cef00c73af29cc282d322e) )
	ROM_LOAD( "berzerk_rc32s_5c.rom5.5c", 0x3000, 0x0800, CRC(9ad80e4e) SHA1(f79a86dd3dee5d53c2a60eda5b5181816bd73bc3) )
	ROM_FILL(                             0x3800, 0x0800, 0xff ) /* rom socket ROM6 at 3C is unpopulated */

	ROM_REGION( 0x01000, "speech", 0 ) /* voice data */
	ROM_LOAD( "berzerk_rvos_1c.1c", 0x0000, 0x0800, CRC(0b51409c) SHA1(75333853a82029f080e3db61441ba6091c1aab55) )    /* VSU-1000 board */
	/* rom socket 2C is unpopulated */
ROM_END


ROM_START( frenzy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1c-0",         0x0000, 0x1000, CRC(abdd25b8) SHA1(e6a3ab826b51b2c6ddd63d55681848fccad800dd) )
	ROM_LOAD( "1d-1",         0x1000, 0x1000, CRC(536e4ae8) SHA1(913385c43b8902d3d3ad2194a3137e19e61c6573) )
	ROM_LOAD( "3d-2",         0x2000, 0x1000, CRC(3eb9bc9b) SHA1(1e43e76ae0606a6d41d9006005d6001bdee48694) )
	ROM_LOAD( "5d-3",         0x3000, 0x1000, CRC(e1d3133c) SHA1(2af4a9bc2b29735a548ae770f872127bc009cc42) )
	ROM_LOAD( "6d-4",         0xc000, 0x1000, CRC(5581a7b1) SHA1(1f633c1c29d3b64f701c601feba26da66a6c6f23) )

	ROM_REGION( 0x01000, "speech", 0 ) /* voice data */
	ROM_LOAD( "e169-1cvo.1c", 0x0000, 0x0800, CRC(2cfe825d) SHA1(f12fed8712f20fa8213f606c4049a8144bfea42e) )    /* VSU-1000 board */
	ROM_LOAD( "e169-2cvo.2c", 0x0800, 0x0800, CRC(d2b6324e) SHA1(20a6611ad6ec19409ac138bdae7bdfaeab6c47cf) )    /* ditto */

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "prom.6e",      0x0000, 0x0020, CRC(4471ca5d) SHA1(ba8dca2ec076818f8ad8c17b15c77965e36fa05e) ) /* address decoder/rom select prom (N82S123N) */
ROM_END


/*
   The original / prototype version of moon war runs on Frenzy Hardware.

   The more common version of Moon War runs on modified Super Cobra (scobra.c) hardware and is often called
   'Moon War 2' because it is the second version, and many of the PCBs are labeled as such.

   So far only 2 original boards of this have been found, one with only the sound roms on it, and the other
   with only the program roms on it.  This set is a combination of dumps from those two boards, so there
   is a small chance they could be mismatched.
*/
ROM_START( moonwarp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1d.bin",      0x0000, 0x1000, CRC(75470634) SHA1(1a811fef39724fd227e06b694841d3dad5659622) )
	ROM_LOAD( "3d.bin",      0x1000, 0x1000, CRC(a9d046dc) SHA1(88afccd09d2809cafd12dd40ab3be77e3707cfc5) )
	ROM_LOAD( "5d.bin",      0x2000, 0x1000, CRC(bf671737) SHA1(cdfae1eb8995c2251813cc5633fc809aa9e6a36f) )
	ROM_LOAD( "6d.bin",      0x3000, 0x1000, CRC(cef2d697) SHA1(5c31c6e7002f0d944b3028d1b804480acf3af042) )
	ROM_LOAD( "5c.bin",      0xc000, 0x1000, CRC(a3d551ab) SHA1(a32352727b5475a6ec6c495c55f01ccd6e024f98) )

	ROM_REGION( 0x01000, "speech", 0 ) /* voice data */
	ROM_LOAD( "moonwar.1c.bin",           0x0000, 0x0800, CRC(9e9a653f) SHA1(cf49a38ef343ace271ba1e5dde38bd8b9c0bd876) )    /* VSU-1000 board */
	ROM_LOAD( "moonwar.2c.bin",           0x0800, 0x0800, CRC(73fd988d) SHA1(08a2aeb4d87eee58e38e4e3f749a95f2308aceb0) )    /* ditto */

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "n82s123 po. e6 select decoder",      0x0000, 0x0020, CRC(4471ca5d) SHA1(ba8dca2ec076818f8ad8c17b15c77965e36fa05e) ) /* address decoder/rom select prom - from board with prg roms, same as Frenzy*/
	ROM_LOAD( "prom.6e",        0x0000, 0x0020, CRC(56bffba3) SHA1(c8e24f6361c50bcb4c9d3f39cdaf4172c2a2b318) ) /* address decoder/rom select prom - from the sound rom only set, is it bad? */
ROM_END

DRIVER_INIT_MEMBER(berzerk_state,moonwarp)
{
	address_space &io = m_maincpu->space(AS_IO);
	io.install_read_handler (0x48, 0x48, read8_delegate(FUNC(berzerk_state::moonwarp_p1_r), this));
	io.install_read_handler (0x4a, 0x4a, read8_delegate(FUNC(berzerk_state::moonwarp_p2_r), this));

	save_item(NAME(m_p1_counter_74ls161));
	save_item(NAME(m_p1_direction));
	save_item(NAME(m_p2_counter_74ls161));
	save_item(NAME(m_p2_direction));
}

/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1980, berzerk,  0,       berzerk, berzerk,  driver_device,        0, ROT0, "Stern Electronics", "Berzerk (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, berzerk1, berzerk, berzerk, berzerk,  driver_device,        0, ROT0, "Stern Electronics", "Berzerk (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, berzerkf, berzerk, berzerk, berzerkf, driver_device,        0, ROT0, "Stern Electronics", "Berzerk (French Speech)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, berzerkg, berzerk, berzerk, berzerkg, driver_device,        0, ROT0, "Stern Electronics", "Berzerk (German Speech)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, berzerks, berzerk, berzerk, berzerks, driver_device,        0, ROT0, "Stern Electronics (Sonic License)", "Berzerk (Spanish Speech)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, frenzy,   0,       frenzy,  frenzy,   driver_device,        0, ROT0, "Stern Electronics", "Frenzy", MACHINE_SUPPORTS_SAVE )
GAME( 1981, moonwarp, 0,       frenzy,  moonwarp, berzerk_state, moonwarp, ROT0, "Stern Electronics", "Moon War (prototype on Frenzy hardware)", MACHINE_SUPPORTS_SAVE )
