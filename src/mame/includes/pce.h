// license:BSD-3-Clause
// copyright-holders:Charles MacDonald, Wilbert Pol, Angelo Salese
/*****************************************************************************
 *
 * includes/pce.h
 *
 * NEC PC Engine/TurboGrafx16
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_PCE_H
#define MAME_INCLUDES_PCE_H

#include "cdrom.h"
#include "cpu/h6280/h6280.h"
#include "bus/pce/pce_slot.h"
#include "bus/pce_ctrl/pcectrl.h"
#include "machine/pce_cd.h"
#include "video/huc6260.h"

#define C6280_TAG           "c6280"

#define MAIN_CLOCK      21477270

#define TG_16_JOY_SIG       0x00
#define PCE_JOY_SIG         0x40
#define NO_CD_SIG           0x80
#define CD_SIG              0x00
/* these might be used to indicate something, but they always seem to return 1 */
#define CONST_SIG           0x30



class pce_state : public driver_device
{
public:
	pce_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cd_ram(*this, "cd_ram"),
		m_huc6260(*this, "huc6260"),
		m_cartslot(*this, "cartslot"),
		m_cd(*this, "pce_cd"),
		m_port_ctrl(*this, "ctrl"),
		m_a_card(*this, "A_CARD")
	{ }

	void init_tg16();
	void init_pce();

	void pce_common(machine_config &config);
	void pce(machine_config &config);
	void tg16(machine_config &config);
	void sgx(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<h6280_device> m_maincpu;
	required_shared_ptr<u8> m_cd_ram;
	required_device<huc6260_device> m_huc6260;
	required_device<pce_cart_slot_device> m_cartslot;
	optional_device<pce_cd_device> m_cd;
	required_device<pce_control_port_device> m_port_ctrl;
	required_ioport m_a_card;

	u8 m_io_port_options;
	u8 m_sys3_card;
	u8 m_acard;
	void controller_w(u8 data);
	u8 controller_r();
	void cd_intf_w(offs_t offset, u8 data);
	u8 cd_intf_r(offs_t offset);
	u8 acard_wram_r(offs_t offset);
	void acard_wram_w(offs_t offset, u8 data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void pce_io(address_map &map);
	void pce_mem(address_map &map);
	void sgx_io(address_map &map);
	void sgx_mem(address_map &map);
};

#endif // MAME_INCLUDES_PCE_H
