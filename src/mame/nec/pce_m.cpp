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
	// Install HuCard ROM
	if ((m_cartslot->get_type() != PCE_ACARD_DUO))
	{
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x000000, 0x0fffff, read8sm_delegate(m_cartslot, FUNC(pce_cart_slot_device::read_cart)), write8sm_delegate(m_cartslot, FUNC(pce_cart_slot_device::write_cart)));
	}

	// Install Super System Card registers
	if ((m_cartslot->get_type() == PCE_CDSYS3J)
		|| (m_cartslot->get_type() == PCE_CDSYS3U)
		|| (m_cartslot->get_type() == PCE_ACARD_PRO))
	{
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1ff8c0, 0x1ff8c7, 0, 0x330, 0, read8sm_delegate(m_cartslot, FUNC(pce_cart_slot_device::read_ex)));
	}

	// Install Arcade Card registers
	if ((m_cartslot->get_type() == PCE_ACARD_DUO)
		|| (m_cartslot->get_type() == PCE_ACARD_PRO))
	{
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x080000, 0x087fff, read8sm_delegate(m_cartslot, FUNC(pce_cart_slot_device::read_ram)), write8sm_delegate(m_cartslot, FUNC(pce_cart_slot_device::write_ram)));
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1ffa00, 0x1ffbff, read8sm_delegate(m_cartslot, FUNC(pce_cart_slot_device::peripheral_r)), write8sm_delegate(m_cartslot, FUNC(pce_cart_slot_device::peripheral_w)));
	}

	if (m_cd)
		m_cd->late_setup();

	// saving is only partially supported: it should be fine with cart games
	// OTOH CD states are saved but not correctly restored!
	save_item(NAME(m_io_port_options));
}

void pce_state::machine_reset()
{
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

	m_cd->intf_w(offset, data);

	m_cd->update();
}

u8 pce_state::cd_intf_r(offs_t offset)
{
	m_cd->update();

	return m_cd->intf_r(offset);
}
