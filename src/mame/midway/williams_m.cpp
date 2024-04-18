// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Williams 6809 system

***************************************************************************/

#include "emu.h"
#include "williams.h"


/*************************************
 *
 *  Older Williams interrupts
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(williams_state::va11_callback)
{
	int const scanline = param;

	// must not fire at line 256
	if (scanline == 256)
		return;

	/* the IRQ signal comes into CB1, and is set to VA11 */
	m_pia[1]->cb1_w(BIT(scanline, 5));
}


TIMER_DEVICE_CALLBACK_MEMBER(williams_state::count240_callback)
{
	int const scanline = param;

	// the COUNT240 signal comes into CA1, and is set to the logical AND of VA10-VA13
	m_pia[1]->ca1_w(scanline >= 240 ? 1 : 0);
}


/*************************************
 *
 *  Older Williams initialization
 *
 *************************************/

void williams_state::machine_reset()
{
	m_rom_view.disable();
}


/*************************************
 *
 *  Newer Williams interrupts
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(williams2_state::va11_callback)
{
	int const scanline = param;
	if (scanline == 256)
		return;

	// the IRQ signal comes into CB1, and is set to VA11
	m_pia[0]->cb1_w(BIT(scanline, 5));
	m_pia[1]->ca1_w(BIT(scanline, 5));
}


TIMER_DEVICE_CALLBACK_MEMBER(williams2_state::endscreen_callback)
{
	int const scanline = param;

	// the /ENDSCREEN signal comes into CA1
	m_pia[0]->ca1_w(scanline >= 254 ? 0 : 1);
}



/*************************************
 *
 *  Newer Williams initialization
 *
 *************************************/

void williams2_state::machine_start()
{
	williams_state::machine_start();

	/* configure memory banks */
	m_mainbank->configure_entries(0, 4, memregion("maincpu")->base() + 0x10000, 0x8000);
}


void williams2_state::machine_reset()
{
	williams_state::machine_reset();

	/* make sure our banking is reset */
	bank_select_w(0);
}



/*************************************
 *
 *  VRAM/ROM banking
 *
 *************************************/

void williams_state::vram_select_w(u8 data)
{
	/* VRAM/ROM banking from bit 0 */
	if (BIT(data, 0))
		m_rom_view.select(0);
	else
		m_rom_view.disable();

	/* cocktail flip from bit 1 */
	m_cocktail = BIT(data, 1);
}


void williams2_state::bank_select_w(u8 data)
{
	/* the low two bits control the paging */
	switch (data & 0x03)
	{
		/* page 0 is video ram */
		case 0:
			m_rom_view.disable();
			m_palette_view.disable();
			break;

		/* pages 1 and 2 are ROM */
		case 1:
		case 2:
			m_mainbank->set_entry((data & 6) >> 1);
			m_rom_view.select(0);
			m_palette_view.disable();
			break;

		/* page 3 accesses palette RAM; the remaining areas are as if page 1 ROM was selected */
		case 3:
			m_mainbank->set_entry((data & 4) >> 1);
			m_rom_view.select(0);
			m_palette_view.select(0);
			break;
	}
}



/*************************************
 *
 *  Sound commands
 *
 *************************************/

template <unsigned A, unsigned... B>
TIMER_CALLBACK_MEMBER(williams_state::deferred_snd_cmd_w)
{
	m_pia[A]->portb_w(param);
	m_pia[A]->cb1_w((param == 0xff) ? 0 : 1);

	if constexpr (sizeof...(B) > 0)
		deferred_snd_cmd_w<B...>(param);
}

void williams_state::snd_cmd_w(u8 data)
{
	// the high two bits are set externally, and should be 1
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(williams_state::deferred_snd_cmd_w<2>), this), data | 0xc0);
}

void williams_state::playball_snd_cmd_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(williams_state::deferred_snd_cmd_w<2>), this), data);
}

TIMER_CALLBACK_MEMBER(williams2_state::deferred_snd_cmd_w)
{
	m_pia[2]->porta_w(param);
}

void williams2_state::snd_cmd_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(williams2_state::deferred_snd_cmd_w),this), data);
}



/*
 *  Williams 49-way joystick
 *
 *  The joystick works on a 7x7 grid system:
 *
 *      + + + | + + +
 *      + + + | + + +
 *      + + + | + + +
 *      ------+------
 *      + + + | + + +
 *      + + + | + + +
 *      + + + | + + +
 *
 *  Each axis has 7 positions, reported as follows
 *  in 4 bits/axis:
 *
 *      0000 = left/up full
 *      0100 = left/up 2/3
 *      0110 = left/up 1/3
 *      0111 = center
 *      1011 = right/down 1/3
 *      1001 = right/down 2/3
 *      1000 = right/down full
 */

u8 williams_state::port_0_49way_r()
{
	static const u8 translate49[7] = { 0x0, 0x4, 0x6, 0x7, 0xb, 0x9, 0x8 };
	return (translate49[m_49way_x->read() >> 4] << 4) | translate49[m_49way_y->read() >> 4];
}



/*************************************
 *
 *  CMOS access
 *
 *************************************/

void williams_state::cmos_4bit_w(offs_t offset, u8 data)
{
	// only 4 bits are valid
	m_nvram[offset] = data | 0xf0;
}



/*************************************
 *
 *  Watchdog
 *
 *************************************/

void williams_state::watchdog_reset_w(u8 data)
{
	/* yes, the data bits are checked for this specific value */
	if (data == 0x39)
		m_watchdog->watchdog_reset();
}


void williams2_state::watchdog_reset_w(u8 data)
{
	/* yes, the data bits are checked for this specific value */
	if ((data & 0x3f) == 0x14)
		m_watchdog->watchdog_reset();
}



/*************************************
 *
 *  Diagnostic controls
 *
 *************************************/

void williams2_state::segments_w(u8 data)
{
	int n;
	char dot;

	switch (data & 0x7F)
	{
		case 0x40:  n = 0; break;
		case 0x79:  n = 1; break;
		case 0x24:  n = 2; break;
		case 0x30:  n = 3; break;
		case 0x19:  n = 4; break;
		case 0x12:  n = 5; break;
		case 0x02:  n = 6; break;
		case 0x03:  n = 6; break;
		case 0x78:  n = 7; break;
		case 0x00:  n = 8; break;
		case 0x18:  n = 9; break;
		case 0x10:  n = 9; break;
		default:    n =-1; break;
	}

	if ((data & 0x80) == 0x00)
		dot = '.';
	else
		dot = ' ';

	if (n == -1)
		logerror("[ %c]\n", dot);
	else
		logerror("[%d%c]\n", n, dot);
}



/*************************************
 *
 *  Defender-specific routines
 *
 *************************************/

void defender_state::machine_start()
{
	williams_state::machine_start();

	memory_region *const banked_rom = memregion("banked");
	unsigned const banks = banked_rom->bytes() / 0x1000;
	for (unsigned i = 0; 15 > i; ++i)
	{
		if ((i < 9) && (i < banks))
			m_rom_view[i + 1].install_rom(0xc000, 0xcfff, banked_rom->base() + (i * 0x1000));
		else
			m_rom_view[i + 1];
	}
}


void defender_state::machine_reset()
{
	williams_state::machine_reset();

	bank_select_w(0);
}


void defender_state::video_control_w(u8 data)
{
	m_cocktail = BIT(data, 0);
}


void defender_state::bank_select_w(u8 data)
{
	m_rom_view.select(data & 0x0f);
}



/*************************************
 *
 *  Mayday-specific routines
 *
 *************************************/

u8 mayday_state::protection_r(offs_t offset)
{
	/* Mayday does some kind of protection check that is not currently understood  */
	/* However, the results of that protection check are stored at $a190 and $a191 */
	/* These are compared against $a193 and $a194, respectively. Thus, to prevent  */
	/* the protection from resetting the machine, we just return $a193 for $a190,  */
	/* and $a194 for $a191. */
	return m_videoram[0xa193 + offset];
}



/*************************************
 *
 *  Sinistar-specific routines
 *
 *************************************/

void williams_state::sinistar_vram_select_w(u8 data)
{
	// low two bits are standard
	vram_select_w(data);

	// window enable from bit 2 (clips to 0x7400)
	m_blitter_window_enable = BIT(data, 2);
}


void williams_state::cockpit_snd_cmd_w(u8 data)
{
	// the high two bits are set externally, and should be 1
	machine().scheduler().synchronize(timer_expired_delegate(NAME((&williams_state::deferred_snd_cmd_w<2, 3>)), this), data | 0xc0);
}



/*************************************
 *
 *  Blaster-specific routines
 *
 *************************************/

void blaster_state::machine_start()
{
	williams_state::machine_start();

	/* banking is different for blaster */
	m_mainbank->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x4000);
}

void blaster_state::machine_reset()
{
	williams_state::machine_reset();

	m_mainbank->set_entry(0);
}


void blaster_state::vram_select_w(u8 data)
{
	/* VRAM/ROM banking from bit 0 */
	if (BIT(data, 0))
		m_rom_view.select(0);
	else
		m_rom_view.disable();

	/* cocktail flip from bit 1 */
	m_cocktail = BIT(data, 1);

	/* window enable from bit 2 (clips to 0x9700) */
	m_blitter_window_enable = BIT(data, 2);
}


void blaster_state::bank_select_w(u8 data)
{
	m_mainbank->set_entry(data & 0x0f);
}


TIMER_CALLBACK_MEMBER(blaster_state::deferred_snd_cmd_w)
{
	u8 const l_data = param | 0x80;
	u8 const r_data = (param >> 1 & 0x40) | (param & 0x3f) | 0x80;

	m_pia[2]->portb_w(l_data); m_pia[2]->cb1_w((l_data == 0xff) ? 0 : 1);
	m_pia[3]->portb_w(r_data); m_pia[3]->cb1_w((r_data == 0xff) ? 0 : 1);
}


void blaster_state::blaster_snd_cmd_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(blaster_state::deferred_snd_cmd_w), this), data);
}



/*************************************
 *
 *  Williams 2nd-gen-specific routines
 *
 *************************************/

void williams2_state::video_control_w(u8 data)
{
	m_cocktail = BIT(data, 0);
}



/*************************************
 *
 *  Mystic Marathon-specific routines
 *
 *************************************/

void mysticm_state::machine_start()
{
	williams2_state::machine_start();

	save_item(NAME(m_bg_color));
}



/*************************************
 *
 *  Turkey Shoot-specific routines
 *
 *************************************/

void tshoot_state::machine_start()
{
	williams2_state::machine_start();

	m_grenade_lamp.resolve();
	m_gun_lamp.resolve();
	m_p1_gun_recoil.resolve();
	m_feather_blower.resolve();
}


void tshoot_state::maxvol_w(int state)
{
	/* something to do with the sound volume */
	logerror("tshoot maxvol = %d (%s)\n", state, machine().describe_context());
}


void tshoot_state::lamp_w(u8 data)
{
	/* set the grenade lamp */
	m_grenade_lamp   = BIT(~data, 2);
	/* set the gun lamp */
	m_gun_lamp       = BIT(~data, 3);
	/* gun coil */
	m_p1_gun_recoil  = BIT(data, 4);
	/* feather coil */
	m_feather_blower = BIT(data, 5);
}



/*************************************
 *
 *  Joust 2-specific routines
 *
 *************************************/

void joust2_state::machine_start()
{
	williams2_state::machine_start();

	save_item(NAME(m_current_sound_data));
}


TIMER_CALLBACK_MEMBER(joust2_state::deferred_snd_cmd_w)
{
	m_pia[2]->porta_w(param & 0xff);
}


void joust2_state::pia_s11_bg_strobe_w(int state)
{
	m_current_sound_data = (m_current_sound_data & ~0x100) | ((state << 8) & 0x100);
	m_bg->ctrl_w(state);
}


void joust2_state::snd_cmd_w(u8 data)
{
	m_current_sound_data = (m_current_sound_data & ~0xff) | (data & 0xff);
	m_bg->data_w(data);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(joust2_state::deferred_snd_cmd_w),this), m_current_sound_data);
}
