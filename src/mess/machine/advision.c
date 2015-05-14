// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

  advision.c

  Machine file to handle emulation of the AdventureVision.

***************************************************************************/

#include "emu.h"
#include "includes/advision.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/dac.h"

/*
    8048 Ports:

    P1  Bit 0..1  - RAM bank select
        Bit 3..7  - Keypad input

    P2  Bit 0..3  - A8-A11
        Bit 4..7  - Sound control/Video write address

    T1  Mirror sync pulse
*/

/* Machine Initialization */

void advision_state::machine_start()
{
	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	/* configure EA banking */
	m_bank1->configure_entry(0, memregion(I8048_TAG)->base());
	m_bank1->configure_entry(1, m_cart_rom->base());
	m_maincpu->space(AS_PROGRAM).install_readwrite_bank(0x0000, 0x03ff, "bank1");
	m_bank1->set_entry(0);

	/* allocate external RAM */
	m_ext_ram.resize(0x400);
	save_item(NAME(m_ext_ram));

	save_item(NAME(m_ea_bank));
	save_item(NAME(m_rambank));
	save_item(NAME(m_frame_count));
	save_item(NAME(m_frame_start));
	save_item(NAME(m_video_enable));
	save_item(NAME(m_video_bank));
	save_item(NAME(m_led_latch));
	save_item(NAME(m_sound_cmd));
	save_item(NAME(m_sound_d));
	save_item(NAME(m_sound_g));
}

void advision_state::machine_reset()
{
	m_ea_bank = 0;
	m_rambank = 0x300;
	m_frame_start = 0;
	m_video_enable = 0;
	m_sound_cmd = 0;

	/* enable internal ROM */
	m_maincpu->set_input_line(MCS48_INPUT_EA, CLEAR_LINE);
	m_bank1->set_entry(m_ea_bank);

	/* reset sound CPU */
	m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

/* Bank Switching */

WRITE8_MEMBER( advision_state::bankswitch_w )
{
	m_ea_bank = BIT(data, 2);
	m_rambank = (data & 0x03) << 8;

	m_maincpu->set_input_line(MCS48_INPUT_EA, m_ea_bank ? ASSERT_LINE : CLEAR_LINE);
	if (m_cart_rom)
		m_bank1->set_entry(m_ea_bank);
}

/* External RAM */

READ8_MEMBER( advision_state::ext_ram_r )
{
	UINT8 data = m_ext_ram[m_rambank + offset];

	if (!m_video_enable)
	{
		/* the video hardware interprets reads as writes */
		vh_write(data);
	}

	if (m_video_bank == 0x06)
	{
		m_soundcpu->set_input_line(INPUT_LINE_RESET, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
	}

	return data;
}

WRITE8_MEMBER( advision_state::ext_ram_w )
{
	m_ext_ram[m_rambank + offset] = data;
}

/* Sound */

READ8_MEMBER( advision_state::sound_cmd_r )
{
	return m_sound_cmd;
}

void advision_state::update_dac()
{
	if (m_sound_g == 0 && m_sound_d == 0)
		m_dac->write_unsigned8(0xff);
	else if (m_sound_g == 1 && m_sound_d == 1)
		m_dac->write_unsigned8(0x80);
	else
		m_dac->write_unsigned8(0x00);
}

WRITE8_MEMBER( advision_state::sound_g_w )
{
	m_sound_g = data & 0x01;

	update_dac();
}

WRITE8_MEMBER( advision_state::sound_d_w )
{
	m_sound_d = data & 0x01;

	update_dac();
}

/* Video */

WRITE8_MEMBER( advision_state::av_control_w )
{
	m_sound_cmd = data >> 4;

	if ((m_video_enable == 0x00) && (data & 0x10))
	{
		vh_update(m_video_hpos);

		m_video_hpos++;

		if (m_video_hpos > 255)
		{
			m_video_hpos = 0;
			logerror("HPOS OVERFLOW\n");
		}
	}

	m_video_enable = data & 0x10;
	m_video_bank = (data & 0xe0) >> 5;
}

READ8_MEMBER( advision_state::vsync_r )
{
	if (m_frame_start)
	{
		m_frame_start = 0;

		return 0;
	}
	else
	{
		return 1;
	}
}

/* Input */

READ8_MEMBER( advision_state::controller_r )
{
	// Get joystick switches
	UINT8 in = m_joy->read();
	UINT8 data = in | 0x0f;

	// Get buttons
	if (in & 0x02) data = data & 0xf7; /* Button 3 */
	if (in & 0x08) data = data & 0xcf; /* Button 1 */
	if (in & 0x04) data = data & 0xaf; /* Button 2 */
	if (in & 0x01) data = data & 0x6f; /* Button 4 */

	return data & 0xf8;
}
