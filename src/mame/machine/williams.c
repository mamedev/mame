// license:???
// copyright-holders:Michael Soderstrom, Marc LaFontaine, Aaron Giles
/***************************************************************************

    Williams 6809 system

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "machine/ticket.h"
#include "includes/williams.h"
#include "sound/dac.h"
#include "sound/hc55516.h"


/*************************************
 *
 *  Older Williams interrupts
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(williams_state::williams_va11_callback)
{
	pia6821_device *pia_1 = machine().device<pia6821_device>("pia_1");
	int scanline = param;

	/* the IRQ signal comes into CB1, and is set to VA11 */
	pia_1->cb1_w(scanline & 0x20);

	/* set a timer for the next update */
	scanline += 0x20;
	if (scanline >= 256) scanline = 0;
	timer.adjust(m_screen->time_until_pos(scanline), scanline);
}


TIMER_CALLBACK_MEMBER(williams_state::williams_count240_off_callback)
{
	pia6821_device *pia_1 = machine().device<pia6821_device>("pia_1");

	/* the COUNT240 signal comes into CA1, and is set to the logical AND of VA10-VA13 */
	pia_1->ca1_w(0);
}


TIMER_DEVICE_CALLBACK_MEMBER(williams_state::williams_count240_callback)
{
	pia6821_device *pia_1 = machine().device<pia6821_device>("pia_1");

	/* the COUNT240 signal comes into CA1, and is set to the logical AND of VA10-VA13 */
	pia_1->ca1_w(1);

	/* set a timer to turn it off once the scanline counter resets */
	machine().scheduler().timer_set(m_screen->time_until_pos(0), timer_expired_delegate(FUNC(williams_state::williams_count240_off_callback),this));

	/* set a timer for next frame */
	timer.adjust(m_screen->time_until_pos(240));
}


WRITE_LINE_MEMBER(williams_state::williams_main_irq)
{
	pia6821_device *pia_1 = machine().device<pia6821_device>("pia_1");
	int combined_state = pia_1->irq_a_state() | pia_1->irq_b_state();

	/* IRQ to the main CPU */
	m_maincpu->set_input_line(M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}


WRITE_LINE_MEMBER(williams_state::williams_main_firq)
{
	/* FIRQ to the main CPU */
	m_maincpu->set_input_line(M6809_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}


WRITE_LINE_MEMBER(williams_state::williams_snd_irq)
{
	pia6821_device *pia_2 = machine().device<pia6821_device>("pia_2");
	int combined_state = pia_2->irq_a_state() | pia_2->irq_b_state();

	/* IRQ to the sound CPU */
	m_soundcpu->set_input_line(M6800_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}
/* Same as above, but for second sound board */
WRITE_LINE_MEMBER(blaster_state::williams_snd_irq_b)
{
	pia6821_device *pia_2 = machine().device<pia6821_device>("pia_2b");
	int combined_state = pia_2->irq_a_state() | pia_2->irq_b_state();

	/* IRQ to the sound CPU */
	m_soundcpu_b->set_input_line(M6800_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Newer Williams interrupts
 *
 *************************************/

WRITE_LINE_MEMBER(williams2_state::mysticm_main_irq)
{
	pia6821_device *pia_0 = machine().device<pia6821_device>("pia_0");
	pia6821_device *pia_1 = machine().device<pia6821_device>("pia_1");
	int combined_state = pia_0->irq_b_state() | pia_1->irq_a_state() | pia_1->irq_b_state();

	/* IRQ to the main CPU */
	m_maincpu->set_input_line(M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}


WRITE_LINE_MEMBER(williams2_state::tshoot_main_irq)
{
	pia6821_device *pia_0 = machine().device<pia6821_device>("pia_0");
	pia6821_device *pia_1 = machine().device<pia6821_device>("pia_1");
	int combined_state = pia_0->irq_a_state() | pia_0->irq_b_state() | pia_1->irq_a_state() | pia_1->irq_b_state();

	/* IRQ to the main CPU */
	m_maincpu->set_input_line(M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Older Williams initialization
 *
 *************************************/

MACHINE_START_MEMBER(williams_state,williams_common)
{
	/* configure the memory bank */
	membank("bank1")->configure_entry(1, memregion("maincpu")->base() + 0x10000);
	membank("bank1")->configure_entry(0, m_videoram);
}


MACHINE_RESET_MEMBER(williams_state,williams_common)
{
	/* set a timer to go off every 16 scanlines, to toggle the VA11 line and update the screen */
	timer_device *scan_timer = machine().device<timer_device>("scan_timer");
	scan_timer->adjust(m_screen->time_until_pos(0));

	/* also set a timer to go off on scanline 240 */
	timer_device *l240_timer = machine().device<timer_device>("240_timer");
	l240_timer->adjust(m_screen->time_until_pos(240));
}


MACHINE_START_MEMBER(williams_state,williams)
{
	MACHINE_START_CALL_MEMBER(williams_common);
}


MACHINE_RESET_MEMBER(williams_state,williams)
{
	MACHINE_RESET_CALL_MEMBER(williams_common);
}



/*************************************
 *
 *  Newer Williams interrupts
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(williams2_state::williams2_va11_callback)
{
	pia6821_device *pia_0 = machine().device<pia6821_device>("pia_0");
	pia6821_device *pia_1 = machine().device<pia6821_device>("pia_1");
	int scanline = param;

	/* the IRQ signal comes into CB1, and is set to VA11 */
	pia_0->cb1_w(scanline & 0x20);
	pia_1->ca1_w(scanline & 0x20);

	/* set a timer for the next update */
	scanline += 0x20;
	if (scanline >= 256) scanline = 0;
	timer.adjust(m_screen->time_until_pos(scanline), scanline);
}


TIMER_CALLBACK_MEMBER(williams2_state::williams2_endscreen_off_callback)
{
	pia6821_device *pia_0 = machine().device<pia6821_device>("pia_0");

	/* the /ENDSCREEN signal comes into CA1 */
	pia_0->ca1_w(1);
}


TIMER_DEVICE_CALLBACK_MEMBER(williams2_state::williams2_endscreen_callback)
{
	pia6821_device *pia_0 = machine().device<pia6821_device>("pia_0");

	/* the /ENDSCREEN signal comes into CA1 */
	pia_0->ca1_w(0);

	/* set a timer to turn it off once the scanline counter resets */
	machine().scheduler().timer_set(m_screen->time_until_pos(8), timer_expired_delegate(FUNC(williams2_state::williams2_endscreen_off_callback),this));

	/* set a timer for next frame */
	timer.adjust(m_screen->time_until_pos(254));
}



/*************************************
 *
 *  Newer Williams initialization
 *
 *************************************/

MACHINE_START_MEMBER(williams2_state,williams2)
{
	/* configure memory banks */
	membank("bank1")->configure_entries(1, 4, memregion("maincpu")->base() + 0x10000, 0x10000);
	membank("bank1")->configure_entry(0, m_videoram);
	membank("vram8000")->set_base(&m_videoram[0x8000]);
}


MACHINE_RESET_MEMBER(williams2_state,williams2)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* make sure our banking is reset */
	williams2_bank_select_w(space, 0, 0);

	/* set a timer to go off every 16 scanlines, to toggle the VA11 line and update the screen */
	timer_device *scan_timer = machine().device<timer_device>("scan_timer");
	scan_timer->adjust(m_screen->time_until_pos(0));

	/* also set a timer to go off on scanline 254 */
	timer_device *l254_timer = machine().device<timer_device>("254_timer");
	l254_timer->adjust(m_screen->time_until_pos(254));
}



/*************************************
 *
 *  VRAM/ROM banking
 *
 *************************************/

WRITE8_MEMBER(williams_state::williams_vram_select_w)
{
	/* VRAM/ROM banking from bit 0 */
	membank("bank1")->set_entry(data & 0x01);

	/* cocktail flip from bit 1 */
	m_cocktail = data & 0x02;
}


WRITE8_MEMBER(williams2_state::williams2_bank_select_w)
{
	/* the low two bits control the paging */
	switch (data & 0x03)
	{
		/* page 0 is video ram */
		case 0:
			membank("bank1")->set_entry(0);
			m_bank8000->set_bank(0);
			break;

		/* pages 1 and 2 are ROM */
		case 1:
		case 2:
			membank("bank1")->set_entry(1 + ((data & 6) >> 1));
			m_bank8000->set_bank(0);
			break;

		/* page 3 accesses palette RAM; the remaining areas are as if page 1 ROM was selected */
		case 3:
			membank("bank1")->set_entry(1 + ((data & 4) >> 1));
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
	pia6821_device *pia_2 = machine().device<pia6821_device>("pia_2");

	pia_2->portb_w(param);
	pia_2->cb1_w((param == 0xff) ? 0 : 1);
}

WRITE8_MEMBER(williams_state::williams_snd_cmd_w)
{
	/* the high two bits are set externally, and should be 1 */
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(williams_state::williams_deferred_snd_cmd_w),this), data | 0xc0);
}

WRITE8_MEMBER(williams_state::playball_snd_cmd_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(williams_state::williams_deferred_snd_cmd_w),this), data);
}

TIMER_CALLBACK_MEMBER(williams2_state::williams2_deferred_snd_cmd_w)
{
	pia6821_device *pia_2 = machine().device<pia6821_device>("pia_2");

	pia_2->porta_w(param);
}

WRITE8_MEMBER(williams2_state::williams2_snd_cmd_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(williams2_state::williams2_deferred_snd_cmd_w),this), data);
}



/*************************************
 *
 *  General input port handlers
 *
 *************************************/

WRITE_LINE_MEMBER(williams_state::williams_port_select_w)
{
	m_port_select = state;
}

CUSTOM_INPUT_MEMBER(williams_state::williams_mux_r)
{
	const char *tag = (const char *)param;

	if (m_port_select != 0)
		tag += strlen(tag) + 1;

	return ioport(tag)->read();
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

READ8_MEMBER(williams_state::williams_49way_port_0_r)
{
	static const UINT8 translate49[7] = { 0x0, 0x4, 0x6, 0x7, 0xb, 0x9, 0x8 };
	return (translate49[ioport("49WAYX")->read() >> 4] << 4) | translate49[ioport("49WAYY")->read() >> 4];
}


READ8_MEMBER(williams_state::williams_input_port_49way_0_5_r)
{
	if (m_port_select)
		return williams_49way_port_0_r(space, 0);
	else
		return ioport("IN3")->read();
}



/*************************************
 *
 *  CMOS access
 *
 *************************************/

WRITE8_MEMBER(williams_state::williams_cmos_w)
{
	/* only 4 bits are valid */
	m_nvram[offset] = data | 0xf0;
}


WRITE8_MEMBER(williams_state::bubbles_cmos_w)
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
		watchdog_reset_w(space,0,0);
}


WRITE8_MEMBER(williams2_state::williams2_watchdog_reset_w)
{
	/* yes, the data bits are checked for this specific value */
	if ((data & 0x3f) == 0x14)
		watchdog_reset_w(space,0,0);
}



/*************************************
 *
 *  Diagnostic controls
 *
 *************************************/

WRITE8_MEMBER(williams2_state::williams2_7segment_w)
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
	address_space &space = m_maincpu->space(AS_PROGRAM);

	MACHINE_RESET_CALL_MEMBER(williams_common);

	defender_bank_select_w(space, 0, 0);
}


WRITE8_MEMBER(williams_state::defender_video_control_w)
{
	m_cocktail = data & 0x01;
}


WRITE8_MEMBER(williams_state::defender_bank_select_w)
{
	m_bankc000->set_bank(data & 0x0f);
}



/*************************************
 *
 *  Mayday-specific routines
 *
 *************************************/

READ8_MEMBER(williams_state::mayday_protection_r)
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

WRITE8_MEMBER(williams_state::sinistar_vram_select_w)
{
	/* low two bits are standard */
	williams_vram_select_w(space, offset, data);

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
	membank("bank1")->configure_entries(1, 16, memregion("maincpu")->base() + 0x18000, 0x4000);
	membank("bank1")->configure_entry(0, m_videoram);

	membank("bank2")->configure_entries(1, 16, memregion("maincpu")->base() + 0x10000, 0x0000);
	membank("bank2")->configure_entry(0, &m_videoram[0x4000]);

	/* register for save states */
	save_item(NAME(m_vram_bank));
	save_item(NAME(m_rom_bank));
}


MACHINE_RESET_MEMBER(blaster_state,blaster)
{
	MACHINE_RESET_CALL_MEMBER(williams_common);
}


inline void blaster_state::update_blaster_banking()
{
	membank("bank1")->set_entry(m_vram_bank * (m_rom_bank + 1));
	membank("bank2")->set_entry(m_vram_bank * (m_rom_bank + 1));
}


WRITE8_MEMBER(blaster_state::blaster_vram_select_w)
{
	/* VRAM/ROM banking from bit 0 */
	m_vram_bank = data & 0x01;
	update_blaster_banking();

	/* cocktail flip from bit 1 */
	m_cocktail = data & 0x02;

	/* window enable from bit 2 (clips to 0x9700) */
	m_blitter_window_enable = data & 0x04;
}


WRITE8_MEMBER(blaster_state::blaster_bank_select_w)
{
	m_rom_bank = data & 0x0f;
	update_blaster_banking();
}


TIMER_CALLBACK_MEMBER(blaster_state::blaster_deferred_snd_cmd_w)
{
	pia6821_device *pia_2l = machine().device<pia6821_device>("pia_2");
	pia6821_device *pia_2r = machine().device<pia6821_device>("pia_2b");
	UINT8 l_data = param | 0x80;
	UINT8 r_data = (param >> 1 & 0x40) | (param & 0x3f) | 0x80;

	pia_2l->portb_w(l_data); pia_2l->cb1_w((l_data == 0xff) ? 0 : 1);
	pia_2r->portb_w(r_data); pia_2r->cb1_w((r_data == 0xff) ? 0 : 1);
}


WRITE8_MEMBER(blaster_state::blaster_snd_cmd_w)
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
	coin_lockout_global_w(machine(), state & 1); /* bit 5 of PIC control port A */
}



/*************************************
 *
 *  Turkey Shoot-specific routines
 *
 *************************************/

READ8_MEMBER(williams2_state::tshoot_input_port_0_3_r)
{
	/* merge in the gun inputs with the standard data */
	int data = ioport("IN0")->read();
	int gun = (data & 0x3f) ^ ((data & 0x3f) >> 1);
	return (data & 0xc0) | gun;
}


WRITE_LINE_MEMBER(williams2_state::tshoot_maxvol_w)
{
	/* something to do with the sound volume */
	logerror("tshoot maxvol = %d (%s)\n", state, machine().describe_context());
}


WRITE8_MEMBER(williams2_state::tshoot_lamp_w)
{
	/* set the grenade lamp */
	output_set_value("Grenade_lamp", (~data & 0x4)>>2 );
	/* set the gun lamp */
	output_set_value("Gun_lamp", (~data & 0x8)>>3 );
	/* gun coil */
	output_set_value("Player1_Gun_Recoil", (data & 0x10)>>4 );
	/* feather coil */
	output_set_value("Feather_Blower", (data & 0x20)>>5 );
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
	pia6821_device *pia_2 = machine().device<pia6821_device>("pia_2");
	pia_2->porta_w(param & 0xff);
}


WRITE_LINE_MEMBER(joust2_state::joust2_pia_3_cb1_w)
{
	m_joust2_current_sound_data = (m_joust2_current_sound_data & ~0x100) | ((state << 8) & 0x100);
	m_cvsd_sound->write(machine().driver_data()->generic_space(), 0, m_joust2_current_sound_data);
}


WRITE8_MEMBER(joust2_state::joust2_snd_cmd_w)
{
	m_joust2_current_sound_data = (m_joust2_current_sound_data & ~0xff) | (data & 0xff);
	m_cvsd_sound->write(machine().driver_data()->generic_space(), 0, m_joust2_current_sound_data);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(joust2_state::joust2_deferred_snd_cmd_w),this), m_joust2_current_sound_data);
}
