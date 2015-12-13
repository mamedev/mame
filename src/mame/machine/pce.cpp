// license:BSD-3-Clause
// copyright-holders:Charles MacDonald, Wilbert Pol, Angelo Salese
/************************************************************

PC Engine CD HW notes:

TODO:
- Dragon Ball Z: ADPCM dies after the first upload;
- Dragon Slayer - The Legend of Heroes: black screen;
- Mirai Shonen Conan: dies at new game selection;
- Snatcher: black screen after Konami logo, tries set up CD-DA
            while transferring data?
- Steam Heart's: needs transfer ready irq to get past the
                 gameplay hang, don't know exactly when it should fire
- Steam Heart's: bad ADPCM irq, dialogue is cutted due of it;

=============================================================

CD Interface Register 0x00 - CDC status
x--- ---- busy signal
-x-- ---- request signal
---x ---- cd signal
---- x--- i/o signal

CD Interface Register 0x03 - BRAM lock / CD status
-x-- ---- acknowledge signal
--x- ---- done signal
---x ---- bram signal
---- x--- ADPCM 2
---- -x-- ADPCM 1
---- --x- CDDA left/right speaker select

CD Interface Register 0x05 - CD-DA Volume low 8-bit port

CD Interface Register 0x06 - CD-DA Volume high 8-bit port

CD Interface Register 0x07 - BRAM unlock / CD status
x--- ---- Enables BRAM

CD Interface Register 0x0c - ADPCM status
x--- ---- ADPCM is reading data
---- x--- ADPCM playback (0) stopped (1) currently playing
---- -x-- pending ADPCM data write
---- ---x ADPCM playback (1) stopped (0) currently playing

CD Interface Register 0x0d - ADPCM address control
x--- ---- ADPCM reset
-x-- ---- ADPCM play
--x- ---- ADPCM repeat
---x ---- ADPCM set length
---- x--- ADPCM set read address
---- --xx ADPCM set write address
(note: some games reads bit 5 and wants it to be low otherwise they hangs, surely NOT an ADPCM repeat flag read because it doesn't make sense)

CD Interface Register 0x0e - ADPCM playback rate

CD Interface Register 0x0f - ADPCM fade in/out register
---- xxxx command setting:
0x00 ADPCM/CD-DA Fade-in
0x01 CD-DA fade-in
0x08 CD-DA fade-out (short) ADPCM fade-in
0x09 CD-DA fade-out (long)
0x0a ADPCM fade-out (long)
0x0c CD-DA fade-out (short) ADPCM fade-in
0x0d CD-DA fade-out (short)
0x0e ADPCM fade-out (short)

*************************************************************/

#include "emu.h"
#include "cpu/h6280/h6280.h"
#include "includes/pce.h"


/* joystick related data*/

#define JOY_CLOCK   0x01
#define JOY_RESET   0x02



DRIVER_INIT_MEMBER(pce_state,mess_pce)
{
	m_io_port_options = PCE_JOY_SIG | CONST_SIG;
}

DRIVER_INIT_MEMBER(pce_state,tg16)
{
	m_io_port_options = TG_16_JOY_SIG | CONST_SIG;
}

DRIVER_INIT_MEMBER(pce_state,sgx)
{
	m_io_port_options = PCE_JOY_SIG | CONST_SIG;
}

MACHINE_START_MEMBER(pce_state,pce)
{
	if (m_cd)
		m_cd->late_setup();

	// saving is only partially supported: it should be fine with cart games
	// OTOH CD states are saved but not correctly restored!
	save_item(NAME(m_io_port_options));
	save_item(NAME(m_acard));
	save_item(NAME(m_joystick_port_select));
	save_item(NAME(m_joystick_data_select));
	save_item(NAME(m_joy_6b_packet));
}

MACHINE_RESET_MEMBER(pce_state,mess_pce)
{
	for (auto & elem : m_joy_6b_packet)
		elem = 0;

	/* Note: Arcade Card BIOS contents are the same as System 3, only internal HW differs.
	   We use a category to select between modes (some games can be run in either S-CD or A-CD modes) */
	m_acard = m_a_card->read() & 1;

	if (m_cartslot->get_type() == PCE_CDSYS3J)
	{
		m_sys3_card = 1;
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x080000, 0x087fff, read8_delegate(FUNC(pce_state::pce_cd_acard_wram_r),this),write8_delegate(FUNC(pce_state::pce_cd_acard_wram_w),this));
	}

	if (m_cartslot->get_type() == PCE_CDSYS3U)
	{
		m_sys3_card = 3;
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x080000, 0x087fff, read8_delegate(FUNC(pce_state::pce_cd_acard_wram_r),this),write8_delegate(FUNC(pce_state::pce_cd_acard_wram_w),this));
	}
}

/* todo: how many input ports does the PCE have? */
WRITE8_MEMBER(pce_state::mess_pce_joystick_w)
{
	int joy_i;
	UINT8 joy_type = m_joy_type->read();

	m_maincpu->io_set_buffer(data);

	/* bump counter on a low-to-high transition of bit 1 */
	if ((!m_joystick_data_select) && (data & JOY_CLOCK))
	{
		m_joystick_port_select = (m_joystick_port_select + 1) & 0x07;
	}

	/* do we want buttons or direction? */
	m_joystick_data_select = data & JOY_CLOCK;

	/* clear counter if bit 2 is set */
	if (data & JOY_RESET)
	{
		m_joystick_port_select = 0;

		for (joy_i = 0; joy_i < 5; joy_i++)
		{
			if (((joy_type >> (joy_i*2)) & 3) == 2)
				m_joy_6b_packet[joy_i] ^= 1;
		}
	}
}

READ8_MEMBER(pce_state::mess_pce_joystick_r)
{
	UINT8 joy_type = m_joy_type->read();
	UINT8 ret, data;

	if (m_joystick_port_select <= 4)
	{
		switch ((joy_type >> (m_joystick_port_select*2)) & 3)
		{
			case 0: //2-buttons pad
				data = m_joy[m_joystick_port_select]->read();
				break;
			case 2: //6-buttons pad
				/*
				Two packets:
				1st packet: directions + I, II, Run, Select
				2nd packet: 6 buttons "header" (high 4 bits active low) + III, IV, V, VI
				Note that six buttons pad just doesn't work with (almost?) every single 2-button-only games, it's really just an after-thought and it is like this
				on real HW.
				*/
				data = m_joy6b[m_joystick_port_select]->read() >> (m_joy_6b_packet[m_joystick_port_select]*8);
				break;
			default:
				data = 0xff;
				break;
		}
	}
	else
		data = 0xff;


	if (m_joystick_data_select)
		data >>= 4;

	ret = (data & 0x0f) | m_io_port_options;
#ifdef UNIFIED_PCE
	ret &= ~0x40;
#endif

	return (ret);
}


WRITE8_MEMBER(pce_state::pce_cd_intf_w)
{
	m_cd->update();

	if (offset & 0x200 && m_sys3_card && m_acard) // route Arcade Card handling ports
		return m_cd->acard_w(space, offset, data);

	m_cd->intf_w(space, offset, data);

	m_cd->update();
}

READ8_MEMBER(pce_state::pce_cd_intf_r)
{
	m_cd->update();

	if (offset & 0x200 && m_sys3_card && m_acard) // route Arcade Card handling ports
		return m_cd->acard_r(space, offset);

	if ((offset & 0xc0) == 0xc0 && m_sys3_card) //System 3 Card header handling
	{
		switch (offset & 0xcf)
		{
			case 0xc1: return 0xaa;
			case 0xc2: return 0x55;
			case 0xc3: return 0x00;
			case 0xc5: return (m_sys3_card & 2) ? 0x55 : 0xaa;
			case 0xc6: return (m_sys3_card & 2) ? 0xaa : 0x55;
			case 0xc7: return 0x03;
		}
	}

	return m_cd->intf_r(space, offset);
}


READ8_MEMBER(pce_state::pce_cd_acard_wram_r)
{
	return pce_cd_intf_r(space, 0x200 | (offset & 0x6000) >> 9);
}

WRITE8_MEMBER(pce_state::pce_cd_acard_wram_w)
{
	pce_cd_intf_w(space, 0x200 | (offset & 0x6000) >> 9, data);
}
