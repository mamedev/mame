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



void pce_state::init_pce()
{
	m_io_port_options = PCE_JOY_SIG | CONST_SIG;
}

void pce_state::init_tg16()
{
	m_io_port_options = TG_16_JOY_SIG | CONST_SIG;
}

void pce_state::machine_start()
{
	if (m_cd)
		m_cd->late_setup();

	// saving is only partially supported: it should be fine with cart games
	// OTOH CD states are saved but not correctly restored!
	save_item(NAME(m_io_port_options));
	save_item(NAME(m_acard));
}

void pce_state::machine_reset()
{
	/* Note: Arcade Card BIOS contents are the same as System 3, only internal HW differs.
	   We use a category to select between modes (some games can be run in either S-CD or A-CD modes) */
	m_acard = m_a_card->read() & 1;

	if (m_cartslot->get_type() == PCE_CDSYS3J)
	{
		m_sys3_card = 1;
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x080000, 0x087fff, read8sm_delegate(*this, FUNC(pce_state::acard_wram_r)), write8sm_delegate(*this, FUNC(pce_state::acard_wram_w)));
	}

	if (m_cartslot->get_type() == PCE_CDSYS3U)
	{
		m_sys3_card = 3;
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x080000, 0x087fff, read8sm_delegate(*this, FUNC(pce_state::acard_wram_r)), write8sm_delegate(*this, FUNC(pce_state::acard_wram_w)));
	}
}

/* todo: how many input ports does the PCE have? */
void pce_state::controller_w(u8 data)
{
	m_port_ctrl->sel_w(BIT(data, 0));
	m_port_ctrl->clr_w(BIT(data, 1));
}

u8 pce_state::controller_r()
{
	u8 ret = (m_port_ctrl->port_r() & 0x0f) | m_io_port_options;
#ifdef UNIFIED_PCE
	ret &= ~0x40;
#endif

	return ret;
}


void pce_state::cd_intf_w(offs_t offset, u8 data)
{
	m_cd->update();

	if (offset & 0x200 && m_sys3_card && m_acard) // route Arcade Card handling ports
		return m_cd->acard_w(offset, data);

	m_cd->intf_w(offset, data);

	m_cd->update();
}

u8 pce_state::cd_intf_r(offs_t offset)
{
	m_cd->update();

	if (offset & 0x200 && m_sys3_card && m_acard) // route Arcade Card handling ports
		return m_cd->acard_r(offset);

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

	return m_cd->intf_r(offset);
}


u8 pce_state::acard_wram_r(offs_t offset)
{
	return cd_intf_r(0x200 | (offset & 0x6000) >> 9);
}

void pce_state::acard_wram_w(offs_t offset, u8 data)
{
	cd_intf_w(0x200 | (offset & 0x6000) >> 9, data);
}
