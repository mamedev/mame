// license:BSD-3-Clause
// copyright-holders:Charles MacDonald, Wilbert Pol, Angelo Salese
/*****************************************************************************
 *
 * includes/pce.h
 *
 * NEC PC Engine/TurboGrafx16
 *
 ****************************************************************************/

#ifndef PCE_H_
#define PCE_H_

#include "cdrom.h"
#include "cpu/h6280/h6280.h"
#include "bus/pce/pce_slot.h"
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
		m_user_ram(*this, "user_ram"),
		m_huc6260(*this, "huc6260"),
		m_cartslot(*this, "cartslot"),
		m_cd(*this, "pce_cd"),
		m_joy(*this, "JOY_P"),
		m_joy6b(*this, "JOY6B_P"),
		m_joy_type(*this, "JOY_TYPE"),
		m_a_card(*this, "A_CARD")
	{ }

	required_device<h6280_device> m_maincpu;
	required_shared_ptr<UINT8> m_cd_ram;
	required_shared_ptr<UINT8> m_user_ram;
	optional_device<huc6260_device> m_huc6260;
	required_device<pce_cart_slot_device> m_cartslot;
	optional_device<pce_cd_device> m_cd;
	required_ioport_array<5> m_joy;
	required_ioport_array<5> m_joy6b;
	required_ioport m_joy_type;
	required_ioport m_a_card;

	UINT8 m_io_port_options;
	UINT8 m_sys3_card;
	UINT8 m_acard;
	int m_joystick_port_select;
	int m_joystick_data_select;
	UINT8 m_joy_6b_packet[5];
	DECLARE_WRITE8_MEMBER(mess_pce_joystick_w);
	DECLARE_READ8_MEMBER(mess_pce_joystick_r);
	DECLARE_WRITE8_MEMBER(pce_cd_intf_w);
	DECLARE_READ8_MEMBER(pce_cd_intf_r);
	DECLARE_READ8_MEMBER(pce_cd_acard_wram_r);
	DECLARE_WRITE8_MEMBER(pce_cd_acard_wram_w);
	DECLARE_DRIVER_INIT(sgx);
	DECLARE_DRIVER_INIT(tg16);
	DECLARE_DRIVER_INIT(mess_pce);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_MACHINE_START(pce);
	DECLARE_MACHINE_RESET(mess_pce);
	DECLARE_WRITE_LINE_MEMBER(pce_irq_changed);
};

#endif /* PCE_H_ */
