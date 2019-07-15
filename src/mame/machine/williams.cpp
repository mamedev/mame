// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Williams 6809 system

***************************************************************************/

#include "emu.h"
#include "includes/williams.h"


/*************************************
 *
 *  Older Williams interrupts
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(williams_state::williams_va11_callback)
{
	int scanline = param;

	// must not fire at line 256
	if (scanline == 256)
		return;

	/* the IRQ signal comes into CB1, and is set to VA11 */
	m_pia[1]->cb1_w(BIT(scanline, 5));
}


TIMER_DEVICE_CALLBACK_MEMBER(williams_state::williams_count240_callback)
{
	int scanline = param;

	// the COUNT240 signal comes into CA1, and is set to the logical AND of VA10-VA13
	m_pia[1]->ca1_w(scanline >= 240 ? 1 : 0);
}


/*************************************
 *
 *  Older Williams initialization
 *
 *************************************/

MACHINE_START_MEMBER(williams_state,williams_common)
{
	/* configure the memory bank */
	m_mainbank->configure_entry(1, memregion("maincpu")->base() + 0x10000);
	m_mainbank->configure_entry(0, m_videoram);
}


MACHINE_START_MEMBER(williams_state,williams)
{
	MACHINE_START_CALL_MEMBER(williams_common);
}



/*************************************
 *
 *  Newer Williams interrupts
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(williams2_state::williams2_va11_callback)
{
	int scanline = param;
	if (scanline == 256)
		return;

	// the IRQ signal comes into CB1, and is set to VA11
	m_pia[0]->cb1_w(BIT(scanline, 5));
	m_pia[1]->ca1_w(BIT(scanline, 5));
}


TIMER_DEVICE_CALLBACK_MEMBER(williams2_state::williams2_endscreen_callback)
{
	int scanline = param;

	// the /ENDSCREEN signal comes into CA1
	m_pia[0]->ca1_w(scanline >= 254 ? 0 : 1);
}



/*************************************
 *
 *  Newer Williams initialization
 *
 *************************************/

MACHINE_START_MEMBER(williams2_state,williams2)
{
	/* configure memory banks */
	m_mainbank->configure_entries(1, 4, memregion("maincpu")->base() + 0x10000, 0x10000);
	m_mainbank->configure_entry(0, m_videoram);
	membank("vram8000")->set_base(&m_videoram[0x8000]);
}


MACHINE_RESET_MEMBER(williams2_state,williams2)
{
	/* make sure our banking is reset */
	williams2_bank_select_w(0);
}



/*************************************
 *
 *  VRAM/ROM banking
 *
 *************************************/

void williams_state::williams_vram_select_w(u8 data)
{
	/* VRAM/ROM banking from bit 0 */
	m_mainbank->set_entry(data & 0x01);

	/* cocktail flip from bit 1 */
	m_cocktail = data & 0x02;
}


void williams2_state::williams2_bank_select_w(u8 data)
{
	/* the low two bits control the paging */
	switch (data & 0x03)
	{
		/* page 0 is video ram */
		case 0:
			m_mainbank->set_entry(0);
			m_bank8000->set_bank(0);
			break;

		/* pages 1 and 2 are ROM */
		case 1:
		case 2:
			m_mainbank->set_entry(1 + ((data & 6) >> 1));
			m_bank8000->set_bank(0);
			break;

		/* page 3 accesses palette RAM; the remaining areas are as if page 1 ROM was selected */
		case 3:
			m_mainbank->set_entry(1 + ((data & 4) >> 1));
			m_bank8000->set_bank(1);
			break;
	}
}



/*************************************
 *
 *  Sound commands
 *
 *************************************/

TIMER_CALLBACK_MEMBER(williams_state::williams_deferred_snd_cmd_w)
{
	m_pia[2]->write_portb(param);
	m_pia[2]->cb1_w((param == 0xff) ? 0 : 1);
}

void williams_state::williams_snd_cmd_w(u8 data)
{
	/* the high two bits are set externally, and should be 1 */
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(williams_state::williams_deferred_snd_cmd_w),this), data | 0xc0);
}

void williams_state::playball_snd_cmd_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(williams_state::williams_deferred_snd_cmd_w),this), data);
}

TIMER_CALLBACK_MEMBER(williams2_state::williams2_deferred_snd_cmd_w)
{
	m_pia[2]->write_porta(param);
}

void williams2_state::williams2_snd_cmd_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(williams2_state::williams2_deferred_snd_cmd_w),this), data);
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

u8 williams_state::williams_49way_port_0_r()
{
	static const uint8_t translate49[7] = { 0x0, 0x4, 0x6, 0x7, 0xb, 0x9, 0x8 };
	return (translate49[ioport("49WAYX")->read() >> 4] << 4) | translate49[ioport("49WAYY")->read() >> 4];
}



/*************************************
 *
 *  CMOS access
 *
 *************************************/

void williams_state::williams_cmos_w(offs_t offset, u8 data)
{
	/* only 4 bits are valid */
	m_nvram[offset] = data | 0xf0;
}


void williams_state::bubbles_cmos_w(offs_t offset, u8 data)
{
	/* bubbles has additional CMOS for a full 8 bits */
	m_nvram[offset] = data;
}



/*************************************
 *
 *  Watchdog
 *
 *************************************/

WRITE8_MEMBER(williams_state::williams_watchdog_reset_w)
{
	/* yes, the data bits are checked for this specific value */
	if (data == 0x39)
		m_watchdog->watchdog_reset();
}


WRITE8_MEMBER(williams2_state::williams2_watchdog_reset_w)
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

void williams2_state::williams2_7segment_w(u8 data)
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

MACHINE_START_MEMBER(williams_state,defender)
{
}


MACHINE_RESET_MEMBER(williams_state,defender)
{
	defender_bank_select_w(0);
}


void williams_state::defender_video_control_w(u8 data)
{
	m_cocktail = data & 0x01;
}


void williams_state::defender_bank_select_w(u8 data)
{
	m_bankc000->set_bank(data & 0x0f);
}



/*************************************
 *
 *  Mayday-specific routines
 *
 *************************************/

u8 williams_state::mayday_protection_r(offs_t offset)
{
	/* Mayday does some kind of protection check that is not currently understood  */
	/* However, the results of that protection check are stored at $a190 and $a191 */
	/* These are compared against $a193 and $a194, respectively. Thus, to prevent  */
	/* the protection from resetting the machine(), we just return $a193 for $a190,  */
	/* and $a194 for $a191. */
	return m_mayday_protection[offset + 3];
}



/*************************************
 *
 *  Sinistar-specific routines
 *
 *************************************/

void williams_state::sinistar_vram_select_w(u8 data)
{
	/* low two bits are standard */
	williams_vram_select_w(data);

	/* window enable from bit 2 (clips to 0x7400) */
	m_blitter_window_enable = data & 0x04;
}



/*************************************
 *
 *  Blaster-specific routines
 *
 *************************************/

MACHINE_START_MEMBER(blaster_state,blaster)
{
	/* banking is different for blaster */
	m_mainbank->configure_entries(1, 16, memregion("maincpu")->base() + 0x18000, 0x4000);
	m_mainbank->configure_entry(0, m_videoram);

	m_blaster_bankb->configure_entries(1, 16, memregion("maincpu")->base() + 0x10000, 0x0000);
	m_blaster_bankb->configure_entry(0, &m_videoram[0x4000]);

	/* register for save states */
	save_item(NAME(m_vram_bank));
	save_item(NAME(m_rom_bank));
}


inline void blaster_state::update_blaster_banking()
{
	m_mainbank->set_entry(m_vram_bank * (m_rom_bank + 1));
	m_blaster_bankb->set_entry(m_vram_bank * (m_rom_bank + 1));
}


void blaster_state::blaster_vram_select_w(u8 data)
{
	/* VRAM/ROM banking from bit 0 */
	m_vram_bank = data & 0x01;
	update_blaster_banking();

	/* cocktail flip from bit 1 */
	m_cocktail = data & 0x02;

	/* window enable from bit 2 (clips to 0x9700) */
	m_blitter_window_enable = data & 0x04;
}


void blaster_state::blaster_bank_select_w(u8 data)
{
	m_rom_bank = data & 0x0f;
	update_blaster_banking();
}


TIMER_CALLBACK_MEMBER(blaster_state::blaster_deferred_snd_cmd_w)
{
	uint8_t l_data = param | 0x80;
	uint8_t r_data = (param >> 1 & 0x40) | (param & 0x3f) | 0x80;

	m_pia[2]->write_portb(l_data); m_pia[2]->cb1_w((l_data == 0xff) ? 0 : 1);
	m_pia[3]->write_portb(r_data); m_pia[3]->cb1_w((r_data == 0xff) ? 0 : 1);
}


void blaster_state::blaster_snd_cmd_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(blaster_state::blaster_deferred_snd_cmd_w),this), data);
}



/*************************************
 *
 *  Lotto Fun-specific routines
 *
 *************************************/

WRITE_LINE_MEMBER(williams_state::lottofun_coin_lock_w)
{
	machine().bookkeeping().coin_lockout_global_w(state & 1); /* bit 5 of PIC control port A */
}



/*************************************
 *
 *  Turkey Shoot-specific routines
 *
 *************************************/

MACHINE_START_MEMBER(tshoot_state,tshoot)
{
	MACHINE_START_CALL_MEMBER(williams2);
	m_grenade_lamp.resolve();
	m_gun_lamp.resolve();
	m_p1_gun_recoil.resolve();
	m_feather_blower.resolve();
}


CUSTOM_INPUT_MEMBER(tshoot_state::gun_r)
{
	int data = m_gun[(uintptr_t)param]->read();
	return (data & 0x3f) ^ ((data & 0x3f) >> 1);
}


WRITE_LINE_MEMBER(tshoot_state::maxvol_w)
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

MACHINE_START_MEMBER(joust2_state,joust2)
{
	MACHINE_START_CALL_MEMBER(williams2);
	save_item(NAME(m_joust2_current_sound_data));
}


MACHINE_RESET_MEMBER(joust2_state,joust2)
{
	MACHINE_RESET_CALL_MEMBER(williams2);
}


TIMER_CALLBACK_MEMBER(joust2_state::joust2_deferred_snd_cmd_w)
{
	m_pia[2]->write_porta(param & 0xff);
}


WRITE_LINE_MEMBER(joust2_state::joust2_pia_3_cb1_w)
{
	m_joust2_current_sound_data = (m_joust2_current_sound_data & ~0x100) | ((state << 8) & 0x100);
	m_cvsd_sound->write(m_joust2_current_sound_data);
}


WRITE8_MEMBER(joust2_state::joust2_snd_cmd_w)
{
	m_joust2_current_sound_data = (m_joust2_current_sound_data & ~0xff) | (data & 0xff);
	m_cvsd_sound->write(m_joust2_current_sound_data);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(joust2_state::joust2_deferred_snd_cmd_w),this), m_joust2_current_sound_data);
}
