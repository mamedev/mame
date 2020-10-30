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
	- pinpoint exact timing generation (free counter or based on host screen 
	  beams)
	- interface with multiple screens is a mystery, cfr. dual screen bnstars,
	  stepping stage HW;
	- network irq?
	- actual chip name;

	BTANBs:
	- in p47aces v1.0 (p47acesa) code messes up the programmable irq timer 
	  setup, causing SFX overloads by using Spitfire ship with 30 Hz autofire 
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
	, device_video_interface(mconfig, *this)
	, m_flip_screen_cb(*this)
	, m_vblank_cb(*this)
	, m_field_cb(*this)
	, m_prg_timer_cb(*this)
	, m_sound_reset_cb(*this)
{
}

void jaleco_ms32_sysctrl_device::amap(address_map& map)
{
//	0xba0000 in 68k, 0xfce00000 in MS32 mapped at lower 16-bits mask
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
//	map(0x1e, 0x1f).w // unknown reset signal
//	map(0x24, 0x27).w // sound comms bidirectional acks?
	map(0x28, 0x29).nopw(); // watchdog
	map(0x2c, 0x2d).w(FUNC(jaleco_ms32_sysctrl_device::field_ack_w));
	map(0x2e, 0x2f).w(FUNC(jaleco_ms32_sysctrl_device::vblank_ack_w));
}

//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

void jaleco_ms32_sysctrl_device::device_add_mconfig(machine_config &config)
{
	//DEVICE(config, ...);
	// TODO: at least watchdog sub
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void jaleco_ms32_sysctrl_device::device_start()
{
	m_flip_screen_cb.resolve();
	m_vblank_cb.resolve();
	m_field_cb.resolve();
	m_prg_timer_cb.resolve();
	m_sound_reset_cb.resolve();

	save_item(NAME(m_dotclock_setting));
	save_item(NAME(m_crtc.horz_blank));
	save_item(NAME(m_crtc.horz_display));
	save_item(NAME(m_crtc.vert_blank));
	save_item(NAME(m_crtc.vert_display));
	save_item(NAME(m_flip_screen_state));
	save_item(NAME(m_timer.irq_enable));
	
	m_timer.prg_irq = timer_alloc(PRG_TIMER);
	m_vblank_timer = timer_alloc(VBLANK_TIMER);
	m_field_timer = timer_alloc(FIELD_TIMER);
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
	m_vblank_timer->adjust(attotime::never);
	m_field_timer->adjust(attotime::never);
}

void jaleco_ms32_sysctrl_device::flush_prg_timer()
{
	/* hayaosi1 needs at least 12 IRQ 0 per frame to work (see code at FFE02289) <- hayaosi1 is a megasys1 game ... -AS
	   kirarast needs it too, at least 8 per frame, but waits for a variable amount
	   suchie2 needs ?? per frame (otherwise it hangs when you lose)
	   in different points. Could this be a raster interrupt?
	   Other games using it but not needing it to work:
	   desertwr
	   p47aces
	*/
	attotime step;
	if (m_timer.irq_enable == true)
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

void jaleco_ms32_sysctrl_device::flush_vblank_timer()
{
	m_vblank_timer->adjust(screen().time_until_pos(m_crtc.vert_display));	
}

void jaleco_ms32_sysctrl_device::flush_field_timer()
{
	// 30 Hz irq
	// TODO: unknown vertical position where this happens
	m_field_timer->adjust(screen().time_until_pos(0));
}

void jaleco_ms32_sysctrl_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case VBLANK_TIMER:
			m_vblank_cb(1);
			flush_vblank_timer();
			break;
		case FIELD_TIMER:
			if (screen().frame_number() & 1)
				m_field_cb(1);
			flush_field_timer();
			break;
		case PRG_TIMER:
			m_prg_timer_cb(1);
			flush_prg_timer();
			break;
	}
}

//*****************************************************************************
//  READ/WRITE HANDLERS
//*****************************************************************************

inline u16 jaleco_ms32_sysctrl_device::clamp_to_12bits_neg(u16 raw_data)
{
	// each write has a 12 bit resolution, for both CRTC and timer interval
	// TODO: nndmseal sets up bit 12, reason?
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

inline void jaleco_ms32_sysctrl_device::crtc_refresh_screen_params()
{
	rectangle visarea;
	const u16 htotal = m_crtc.horz_blank + m_crtc.horz_display;
	const u16 vtotal = m_crtc.vert_blank + m_crtc.vert_display;
	const attoseconds_t refresh = HZ_TO_ATTOSECONDS(get_dotclock_frequency()) * htotal * vtotal;
	visarea.set(0, m_crtc.horz_display - 1, 0, m_crtc.vert_display - 1);
	screen().configure(htotal, vtotal, visarea, refresh);
	flush_vblank_timer();
	flush_field_timer();
}

void jaleco_ms32_sysctrl_device::control_w(u16 data)
{
	/* 
	 * ---- x--- programmable irq timer enable (1->0 in P47 Aces)
	 * ---- -x-- used by f1superb, network irq enable?
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
	// data shouldn't matter
	m_sound_reset_cb(1);
}

void jaleco_ms32_sysctrl_device::vblank_ack_w(u16 data)
{
	m_vblank_cb(0);
}

void jaleco_ms32_sysctrl_device::field_ack_w(u16 data)
{
	m_field_cb(0);
}
