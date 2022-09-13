// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Alex Marshall
/******************************************************************************

    Jaleco MS32 System Control Unit

    A simple system controller for late 90s Jaleco HWs

    Known features:
    - CRTC & screen(s?) control;
    - dot clock control;
    - irq/reset controller;
    - programmable timer;
    - watchdog;

    First use in MS32, then their later (?) 68k revision.

    TODO:
    - pinpoint exact timing generation for programmable irq
      (free counter or based on host screen beams)
    - interface with multiple screens is a mystery,
      cfr. dual screen bnstars, stepping stage HW.
      Most likely former effectively controls both screens in demux while
      latter has no way to set the other screen(s)?
    - watchdog timing;
    - upper address line seems unconnected by 68k,
      and is it a mystery how watchdog is supposed to route here and assuming
      it is and not actually disabled by pin.
    - network/COPROs irq connections, specifically for f1superb;
    - actual chip name;

    BTANBs:
    - in p47aces v1.0 (p47acesa) code messes up the prg irq timer setup,
      causing SFX overloads by using Spitfire ship with 30 Hz autofire
      and shooting at point blank range over walls/enemies.
      This has been fixed in v1.1

*******************************************************************************/

#include "emu.h"
#include "jaleco_ms32_sysctrl.h"

//*****************************************************************************
//  GLOBAL VARIABLES
//*****************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(JALECO_MS32_SYSCTRL, jaleco_ms32_sysctrl_device, "jaleco_ms32_sysctrl", "Jaleco MS32 System Control Unit")


//*****************************************************************************
//  LIVE DEVICE
//*****************************************************************************

//-------------------------------------------------
//  jaleco_ms32_sysctrl_device - constructor
//-------------------------------------------------

jaleco_ms32_sysctrl_device::jaleco_ms32_sysctrl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, JALECO_MS32_SYSCTRL, tag, owner, clock)
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_flip_screen_cb(*this)
	, m_vblank_cb(*this)
	, m_field_cb(*this)
	, m_prg_timer_cb(*this)
	, m_sound_ack_cb(*this)
	, m_sound_reset_cb(*this)
	, m_invert_vblank_lines(false)
{
}

void jaleco_ms32_sysctrl_device::amap(address_map& map)
{
//  0xba0000 in 68k, 0xfce00000 in MS32 mapped at lower 16-bits mask
	map(0x00, 0x01).w(FUNC(jaleco_ms32_sysctrl_device::control_w));
	map(0x02, 0x03).w(FUNC(jaleco_ms32_sysctrl_device::hblank_w));
	map(0x04, 0x05).w(FUNC(jaleco_ms32_sysctrl_device::hdisplay_w));
	map(0x06, 0x07).w(FUNC(jaleco_ms32_sysctrl_device::hbp_w));
	map(0x08, 0x09).w(FUNC(jaleco_ms32_sysctrl_device::hfp_w));
	map(0x0a, 0x0b).w(FUNC(jaleco_ms32_sysctrl_device::vblank_w));
	map(0x0c, 0x0d).w(FUNC(jaleco_ms32_sysctrl_device::vdisplay_w));
	map(0x0e, 0x0f).w(FUNC(jaleco_ms32_sysctrl_device::vbp_w));
	map(0x10, 0x11).w(FUNC(jaleco_ms32_sysctrl_device::vfp_w));
	map(0x18, 0x19).w(FUNC(jaleco_ms32_sysctrl_device::timer_interval_w));
	map(0x1a, 0x1b).w(FUNC(jaleco_ms32_sysctrl_device::timer_ack_w));
	map(0x1c, 0x1d).w(FUNC(jaleco_ms32_sysctrl_device::sound_reset_w));
	map(0x1e, 0x1f).w(FUNC(jaleco_ms32_sysctrl_device::irq_ack_w));
//  map(0x24, 0x27).w // sound comms bidirectional acks?
	map(0x26, 0x27).w(FUNC(jaleco_ms32_sysctrl_device::sound_ack_w));
	map(0x28, 0x29).nopw(); // watchdog on MS32
	map(0x2c, 0x2d).w(FUNC(jaleco_ms32_sysctrl_device::field_ack_w));
	map(0x2e, 0x2f).w(FUNC(jaleco_ms32_sysctrl_device::vblank_ack_w));
}

//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration additions
//-------------------------------------------------

void jaleco_ms32_sysctrl_device::device_add_mconfig(machine_config &config)
{
//  TIMER(config, "scantimer").configure_scanline(FUNC(jaleco_ms32_sysctrl_device::scanline_cb), m_screen, 0, 1);

	// TODO: watchdog
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void jaleco_ms32_sysctrl_device::device_resolve_objects()
{
	m_flip_screen_cb.resolve();
	m_vblank_cb.resolve();
	m_field_cb.resolve();
	m_prg_timer_cb.resolve();
	m_sound_reset_cb.resolve();
	m_sound_ack_cb.resolve();
}

void jaleco_ms32_sysctrl_device::device_start()
{
	save_item(NAME(m_dotclock_setting));
	save_item(NAME(m_crtc.horz_blank));
	save_item(NAME(m_crtc.horz_display));
	save_item(NAME(m_crtc.vert_blank));
	save_item(NAME(m_crtc.vert_display));
	save_item(NAME(m_flip_screen_state));
	save_item(NAME(m_timer.irq_enable));
	save_item(NAME(m_timer.interval));

	m_timer.prg_irq = timer_alloc(FUNC(jaleco_ms32_sysctrl_device::prg_timer_tick), this);
	m_timer_scanline = timer_alloc(FUNC(jaleco_ms32_sysctrl_device::flush_scanline_timer), this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void jaleco_ms32_sysctrl_device::device_reset()
{
	m_dotclock_setting = 0;
	m_crtc.horz_blank = 64;
	m_crtc.horz_display = 320;
	m_crtc.vert_blank = 39;
	m_crtc.vert_display = 224;
	m_timer.irq_enable = false;
	m_timer.interval = 1;
	flush_prg_timer();
	// bnstars1 never ever set up the CRTC at boot, causing no irq.
	// we currently compensate by basically giving one frame of time,
	// ofc on the real thing the first vblank is really when screen sync occurs.
	flush_scanline_timer(m_crtc.vert_display-1);
//  m_timer_scanline->adjust(attotime::never);
	// put flipping in a default state
	m_flip_screen_state = false;
	m_flip_screen_cb(0);
}

void jaleco_ms32_sysctrl_device::flush_prg_timer()
{
	attotime step;
	if (m_timer.irq_enable)
	{
		//const u16 htotal = m_crtc.horz_blank + m_crtc.horz_display;
		// TODO: unknown actual timings, with interval = 2 it should fire an irq every 16 scanlines in p47aces v1.1
		//       (excluding times where it is disabled) -> ~1000 Hz?
		step = attotime::from_nsec(500000) * m_timer.interval;
	}
	else
	{
		step = attotime::never;
		m_prg_timer_cb(0);
	}
	m_timer.prg_irq->adjust(step);
}

TIMER_CALLBACK_MEMBER(jaleco_ms32_sysctrl_device::flush_scanline_timer)
{
	int current_scanline = param;

	// in typical Jaleco fashion (cfr. mega system 1), both irqs are somehow configurable (a pin?).
	// Examples are tp2ms32 and wpksocv2, wanting vblank as vector 9 and field as 10 otherwise they runs
	// at half speed, but then their config can't possibly work with p47aces (i.e. wants 10 and 9 respectively),
	// plus bnstars that locks up off the bat if the wrong irq runs at 60 Hz.
	// We currently hardwire via an init time setter here, making the irq acks to trigger properly as well.

	if (current_scanline == m_crtc.vert_display)
		m_invert_vblank_lines ? m_field_cb(1) : m_vblank_cb(1);

	// 30 Hz irq
	// TODO: unknown mechanics where this happens, is it even tied to scanline?
	if (current_scanline == 0 && m_screen->frame_number() & 1)
		m_invert_vblank_lines ? m_vblank_cb(1) : m_field_cb(1);

	uint32_t next_scanline = (current_scanline + 1) % crtc_vtotal();
	m_timer_scanline->adjust(m_screen->time_until_pos(next_scanline), next_scanline);
}

TIMER_CALLBACK_MEMBER(jaleco_ms32_sysctrl_device::prg_timer_tick)
{
	m_prg_timer_cb(1);
	flush_prg_timer();
}

//*****************************************************************************
//  READ/WRITE HANDLERS
//*****************************************************************************

static constexpr u16 clamp_to_12bits_neg(u16 raw_data)
{
	// each write has a 12 bit resolution, for both CRTC and timer interval
	// TODO: nndmseal: on POST it sets up a 0x1000 for vblank, possibly for screen disable?
	// TODO: rockn2: on POST it sets up a vertical size of 487, is it trying to setup an interlace setting?
	return 0x1000 - (raw_data & 0xfff);
}

// ============================================================================
// CRTC
// ============================================================================

inline u32 jaleco_ms32_sysctrl_device::get_dotclock_frequency()
{
	const u8 dot_divider = m_dotclock_setting & 1 ? 6 : 8;
	return clock() / dot_divider;
}

inline u16 jaleco_ms32_sysctrl_device::crtc_vtotal()
{
	return m_crtc.vert_blank + m_crtc.vert_display;
}

inline void jaleco_ms32_sysctrl_device::crtc_refresh_screen_params()
{
	rectangle visarea;
	const u16 htotal = m_crtc.horz_blank + m_crtc.horz_display;
	const u16 vtotal = crtc_vtotal();
	const attoseconds_t refresh = HZ_TO_ATTOSECONDS(get_dotclock_frequency()) * htotal * vtotal;
	visarea.set(0, m_crtc.horz_display - 1, 0, m_crtc.vert_display - 1);
	logerror("%s: CRTC setup total: %d x %d display: %d x %d\n", this->tag(), htotal, vtotal, m_crtc.horz_display, m_crtc.vert_display);
	m_screen->configure(htotal, vtotal, visarea, refresh);
	m_timer_scanline->adjust(m_screen->time_until_pos(vtotal), vtotal);
}

void jaleco_ms32_sysctrl_device::control_w(u16 data)
{
	/*
	 * ---- x--- programmable irq timer enable
	 * ---- -x-- used by f1superb, stepstag, bnstars
	 * ---- --x- flip screen
	 * ---- ---x dotclock select (1) 8 MHz (0) 6 MHz
	 */
	if (BIT(data, 0) != m_dotclock_setting)
	{
		m_dotclock_setting = BIT(data, 0);
		crtc_refresh_screen_params();
	}

	const bool current_flip = bool(BIT(data, 1));
	if (current_flip != m_flip_screen_state)
	{
		m_flip_screen_state = current_flip;
		m_flip_screen_cb(m_flip_screen_state ? ASSERT_LINE : CLEAR_LINE);
	}
	if (data & 0xf4)
		logerror("%s: enabled unknown bit in control_w %02x\n", this->tag(), data & 0xf4);

	m_timer.irq_enable = bool(BIT(data, 3));
	flush_prg_timer();
}

void jaleco_ms32_sysctrl_device::hblank_w(u16 data)
{
	m_crtc.horz_blank = clamp_to_12bits_neg(data);
	crtc_refresh_screen_params();
}

void jaleco_ms32_sysctrl_device::hdisplay_w(u16 data)
{
	m_crtc.horz_display = clamp_to_12bits_neg(data);
	crtc_refresh_screen_params();
}

void jaleco_ms32_sysctrl_device::hbp_w(u16 data)
{
	logerror("%s: HSYNC back porch %d\n", this->tag(), 0x1000 - data);
}

void jaleco_ms32_sysctrl_device::hfp_w(u16 data)
{
	logerror("%s: HSYNC front porch %d\n", this->tag(), 0x1000 - data);
}

void jaleco_ms32_sysctrl_device::vblank_w(u16 data)
{
	m_crtc.vert_blank = clamp_to_12bits_neg(data);
	crtc_refresh_screen_params();
}

void jaleco_ms32_sysctrl_device::vdisplay_w(u16 data)
{
	m_crtc.vert_display = clamp_to_12bits_neg(data);
	crtc_refresh_screen_params();
}

void jaleco_ms32_sysctrl_device::vbp_w(u16 data)
{
	logerror("%s: VSYNC back porch %d\n", this->tag(), clamp_to_12bits_neg(data));
}

void jaleco_ms32_sysctrl_device::vfp_w(u16 data)
{
	logerror("%s: VSYNC front porch %d\n", this->tag(), clamp_to_12bits_neg(data));
}

// ============================================================================
// Timer
// ============================================================================

void jaleco_ms32_sysctrl_device::timer_interval_w(u16 data)
{
	m_timer.interval = clamp_to_12bits_neg(data);
	flush_prg_timer();
}

void jaleco_ms32_sysctrl_device::timer_ack_w(u16 data)
{
	m_prg_timer_cb(0);
}

// ============================================================================
// ACK lines
// ============================================================================

void jaleco_ms32_sysctrl_device::sound_reset_w(u16 data)
{
	m_sound_reset_cb(data & 1);
	logerror("%s: sound_reset_w %02x\n", this->tag(), data);
}

void jaleco_ms32_sysctrl_device::vblank_ack_w(u16 data)
{
	m_vblank_cb(0);
}

void jaleco_ms32_sysctrl_device::field_ack_w(u16 data)
{
	m_field_cb(0);
}

void jaleco_ms32_sysctrl_device::sound_ack_w(u16 data)
{
	m_sound_ack_cb(1);
}

void jaleco_ms32_sysctrl_device::irq_ack_w(u16 data)
{
	// guess: 68k games calls this in vblank routine instead of
	// the designated line, maybe it's a 68k version difference
	// or maybe this is right
	m_vblank_cb(0);
	m_field_cb(0);
	// TODO: f1superb definitely clears comms irq with this
}
