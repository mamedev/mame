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
