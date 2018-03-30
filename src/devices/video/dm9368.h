// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Fairchild DM9368 7-Segment Decoder/Driver/Latch emulation

**********************************************************************
                            _____   _____
                    A1   1 |*    \_/     | 16  Vcc
                    A2   2 |             | 15  F
                   _LE   3 |             | 14  G
                  _RBO   4 |   DM9368    | 13  A
                  _RBI   5 |             | 12  B
                    A3   6 |             | 11  C
                    A0   7 |             | 10  D
                   GND   8 |_____________| 9   E

**********************************************************************/

#ifndef MAME_VIDEO_DM9368_H
#define MAME_VIDEO_DM9368_H

#pragma once



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_DM9368_UPDATE_CALLBACK(cb) \
		devcb = &downcast<dm9368_device &>(*device).set_update_callback(DEVCB_##cb);

#define MCFG_DM9368_RBO_CALLBACK(cb) \
		devcb = &downcast<dm9368_device &>(*device).set_rbo_callback(DEVCB_##cb);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dm9368_device

class dm9368_device : public device_t
{
public:
	template <typename Obj> devcb_base &set_update_callback(Obj &&cb) { return m_update_cb.set_callback(std::forward<Obj>(cb)); }
	template <typename Obj> devcb_base &set_rbo_callback(Obj &&cb) { return m_rbo_cb.set_callback(std::forward<Obj>(cb)); }

	// construction/destruction
	dm9368_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void a_w(u8 data);

	DECLARE_WRITE_LINE_MEMBER( rbi_w ) { m_rbi = state; }
	DECLARE_READ_LINE_MEMBER( rbo_r ) { return m_rbo; }

protected:
	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;

	void update();

private:
	devcb_write8       m_update_cb;
	devcb_write_line   m_rbo_cb;

	int m_rbi;
	int m_rbo;

	static const u8 s_segment_data[16];
};


// device type definition
DECLARE_DEVICE_TYPE(DM9368, dm9368_device)

#endif // MAME_VIDEO_DM9368_H
