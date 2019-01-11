// license:BSD-3-Clause
// copyright-holders:smf,AJR
/***************************************************************************

    NEC µPD4701A 2-Axis Incremental Encoder Counter

****************************************************************************
                              _____   _____
                      Xa   1 |*    \_/     | 24  Vdd
                      Xb   2 |             | 23  D7
                 RESET X   3 |             | 22  D6
                      Ya   4 |             | 21  D5
                      Yb   5 |             | 20  D4
                 RESET Y   6 |             | 19  D3
                  _RIGHT   7 |   uPD4701A  | 18  D2
                   _LEFT   8 |             | 17  D1
                 _MIDDLE   9 |             | 16  D0
                     _SF  10 |             | 15  _CS
                     _CF  11 |             | 14  _X/Y
                     Vss  12 |_____________| 13  U/_L

***************************************************************************/

#ifndef MAME_MACHINE_UPD4701_H
#define MAME_MACHINE_UPD4701_H

#pragma once


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define MCFG_UPD4701_PORTX(_tag) \
	downcast<upd4701_device &>(*device).set_portx_tag(_tag);
#define MCFG_UPD4701_PORTY(_tag) \
	downcast<upd4701_device &>(*device).set_porty_tag(_tag);
#define MCFG_UPD4701_CF_CALLBACK(_devcb) \
	devcb = downcast<upd4701_device &>(*device).set_cf_cb(DEVCB_##_devcb);
#define MCFG_UPD4701_SF_CALLBACK(_devcb) \
	devcb = downcast<upd4701_device &>(*device).set_sf_cb(DEVCB_##_devcb);

class upd4701_device : public device_t
{
public:
	upd4701_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	void set_portx_tag(const char *tag) { m_portx.set_tag(tag); }
	void set_porty_tag(const char *tag) { m_porty.set_tag(tag); }
	template<class Object> devcb_base &set_cf_cb(Object &&cb) { return m_cf_cb.set_callback(std::forward<Object>(cb)); }
	template<class Object> devcb_base &set_sf_cb(Object &&cb) { return m_sf_cb.set_callback(std::forward<Object>(cb)); }

	void x_add(s16 data);
	void y_add(s16 data);

	DECLARE_WRITE_LINE_MEMBER(cs_w);
	DECLARE_WRITE_LINE_MEMBER(xy_w);
	DECLARE_WRITE_LINE_MEMBER(ul_w);
	DECLARE_WRITE_LINE_MEMBER(resetx_w);
	DECLARE_WRITE_LINE_MEMBER(resety_w);
	DECLARE_READ8_MEMBER(reset_x);
	DECLARE_WRITE8_MEMBER(reset_x);
	DECLARE_READ8_MEMBER(reset_y);
	DECLARE_WRITE8_MEMBER(reset_y);
	DECLARE_READ8_MEMBER(reset_xy);
	DECLARE_WRITE8_MEMBER(reset_xy);

	DECLARE_READ8_MEMBER(d_r);
	DECLARE_READ8_MEMBER(read_x);
	DECLARE_READ8_MEMBER(read_y);
	DECLARE_READ8_MEMBER(read_xy);

	DECLARE_WRITE_LINE_MEMBER(left_w);
	DECLARE_WRITE_LINE_MEMBER(right_w);
	DECLARE_WRITE_LINE_MEMBER(middle_w);

	DECLARE_READ_LINE_MEMBER(cf_r);
	DECLARE_READ_LINE_MEMBER(sf_r);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// internal helpers
	void analog_update();
	void switch_update(u8 mask, bool state);

	// control lines
	bool m_cs;                      // chip select (active low)
	bool m_xy;                      // counter select (L = X, H = Y)
	bool m_ul;                      // byte select (L = lower, H = upper)
	bool m_resetx;                  // X-axis counter reset (active high)
	bool m_resety;                  // Y-axis counter reset (active high)

	// counter state
	optional_ioport m_portx;
	optional_ioport m_porty;
	s16 m_latchx;
	s16 m_latchy;
	s16 m_startx;
	s16 m_starty;
	s16 m_x;
	s16 m_y;

	// switch state
	u8 m_switches;
	u8 m_latchswitches;

	// flag outputs and callbacks
	bool m_cf;
	devcb_write_line m_cf_cb;
	devcb_write_line m_sf_cb;
};

DECLARE_DEVICE_TYPE(UPD4701A, upd4701_device)

#endif // MAME_MACHINE_UPD4701_H
