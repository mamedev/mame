// license:BSD-3-Clause
// copyright-holders:Charles MacDonald, Wilbert Pol, Angelo Salese
/*****************************************************************************
 *
 * includes/pce.h
 *
 * NEC PC Engine/TurboGrafx16
 *
 ****************************************************************************/

#ifndef MAME_NEC_PCE_H
#define MAME_NEC_PCE_H

#include "pce_cd.h"

#include "cpu/h6280/h6280.h"
#include "bus/pce/pce_slot.h"
#include "bus/pce_ctrl/pcectrl.h"
#include "video/huc6260.h"

#include "cdrom.h"


class pce_state : public driver_device
{
public:
	pce_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_huc6260(*this, "huc6260"),
		m_cartslot(*this, "cartslot"),
		m_cd(*this, "pce_cd"),
		m_port_ctrl(*this, "ctrl")
	{ }

	void init_tg16();
	void init_pce();

	void pce_common(machine_config &config);
	void pce(machine_config &config);
	void tg16(machine_config &config);
	void sgx(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<h6280_device> m_maincpu;
	required_device<huc6260_device> m_huc6260;
	required_device<pce_cart_slot_device> m_cartslot;
	optional_device<pce_cd_device> m_cd;
	required_device<pce_control_port_device> m_port_ctrl;

	u8 m_io_port_options = 0;
	void controller_w(u8 data);
	u8 controller_r();
	void cd_intf_w(offs_t offset, u8 data);
	u8 cd_intf_r(offs_t offset);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void pce_io(address_map &map) ATTR_COLD;
	void pce_mem(address_map &map) ATTR_COLD;
	void sgx_io(address_map &map) ATTR_COLD;
	void sgx_mem(address_map &map) ATTR_COLD;
};

#endif // MAME_NEC_PCE_H
