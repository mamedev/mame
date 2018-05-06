// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/**  Konami 053252  **/
/* CRT and interrupt control unit */
#ifndef MAME_MACHINE_K053252_H
#define MAME_MACHINE_K053252_H

#pragma once


#include "screen.h"

#define MCFG_K053252_INT1_EN_CB(_devcb) \
	devcb = &downcast<k053252_device &>(*device).set_int1_en_callback(DEVCB_##_devcb);

#define MCFG_K053252_INT2_EN_CB(_devcb) \
	devcb = &downcast<k053252_device &>(*device).set_int2_en_callback(DEVCB_##_devcb);

#define MCFG_K053252_INT1_ACK_CB(_devcb) \
	devcb = &downcast<k053252_device &>(*device).set_int1_ack_callback(DEVCB_##_devcb);

#define MCFG_K053252_INT2_ACK_CB(_devcb) \
	devcb = &downcast<k053252_device &>(*device).set_int2_ack_callback(DEVCB_##_devcb);

#define MCFG_K053252_INT_TIME_CB(_devcb) \
	devcb = &downcast<k053252_device &>(*device).set_int_time_callback(DEVCB_##_devcb);

#define MCFG_K053252_OFFSETS(_offsx, _offsy) \
	downcast<k053252_device &>(*device).set_offsets(_offsx, _offsy);

#define MCFG_K053252_SET_SLAVE_SCREEN(_tag) \
	downcast<k053252_device &>(*device).set_slave_screen(_tag);


class k053252_device : public device_t, public device_video_interface
{
public:
	k053252_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_int1_en_callback(Object &&obj) { return m_int1_en_cb.set_callback(std::forward<Object>(obj)); }
	template <class Object> devcb_base &set_int2_en_callback(Object &&obj) { return m_int2_en_cb.set_callback(std::forward<Object>(obj)); }
	template <class Object> devcb_base &set_int1_ack_callback(Object &&obj) { return m_int1_ack_cb.set_callback(std::forward<Object>(obj)); }
	template <class Object> devcb_base &set_int2_ack_callback(Object &&obj) { return m_int2_ack_cb.set_callback(std::forward<Object>(obj)); }
	template <class Object> devcb_base &set_int_time_callback(Object &&obj) { return m_int_time_cb.set_callback(std::forward<Object>(obj)); }
	void set_offsets(int offsx, int offsy) { m_offsx = offsx; m_offsy = offsy; }

	DECLARE_READ8_MEMBER( read );  // CCU registers
	DECLARE_WRITE8_MEMBER( write );

	void res_change();

	void set_slave_screen(const char *tag) { m_slave_screen.set_tag(tag); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override { reset_internal_state(); }
	void reset_internal_state();

	private:
	// internal state
	uint8_t   m_regs[16];
	uint16_t  m_hc,m_hfp,m_hbp;
	uint16_t  m_vc,m_vfp,m_vbp;
	uint8_t   m_vsw,m_hsw;

	devcb_write_line   m_int1_en_cb;
	devcb_write_line   m_int2_en_cb;
	devcb_write_line   m_int1_ack_cb;
	devcb_write_line   m_int2_ack_cb;
	devcb_write8       m_int_time_cb;
	int                m_offsx;
	int                m_offsy;

	optional_device<screen_device> m_slave_screen;
};

DECLARE_DEVICE_TYPE(K053252, k053252_device)

#endif  // MAME_MACHINE_K053252_H
