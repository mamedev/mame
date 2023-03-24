// license:BSD-3-Clause
// copyright-holders:Charles MacDonald, Wilbert Pol, Angelo Salese
/**************************************************************************************************

PC Engine legacy middle ground file, to be removed ...

**************************************************************************************************/

#include "emu.h"
#include "cpu/h6280/h6280.h"
#include "pce.h"


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
