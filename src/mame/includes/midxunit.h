// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Midway X-unit games.

**************************************************************************/

#include "machine/midwayic.h"

class midxunit_state : public midtunit_state
{
public:
	midxunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: midtunit_state(mconfig, type, tag),
			m_nvram(*this, "nvram"),
			m_midway_serial_pic(*this, "serial_pic") { }

	required_shared_ptr<UINT16> m_nvram;
	required_device<midway_serial_pic_device> m_midway_serial_pic;
	UINT8 m_cmos_write_enable;
	UINT16 m_iodata[8];
	UINT8 m_ioshuffle[16];
	UINT8 m_analog_port;
	UINT8 m_uart[8];
	UINT8 m_security_bits;
	DECLARE_READ16_MEMBER(midxunit_cmos_r);
	DECLARE_WRITE16_MEMBER(midxunit_cmos_w);
	DECLARE_WRITE16_MEMBER(midxunit_io_w);
	DECLARE_WRITE16_MEMBER(midxunit_unknown_w);
	DECLARE_READ16_MEMBER(midxunit_io_r);
	DECLARE_READ16_MEMBER(midxunit_analog_r);
	DECLARE_WRITE16_MEMBER(midxunit_analog_select_w);
	DECLARE_READ16_MEMBER(midxunit_status_r);
	DECLARE_READ16_MEMBER(midxunit_uart_r);
	DECLARE_WRITE16_MEMBER(midxunit_uart_w);
	DECLARE_READ16_MEMBER(midxunit_security_r);
	DECLARE_WRITE16_MEMBER(midxunit_security_w);
	DECLARE_WRITE16_MEMBER(midxunit_security_clock_w);
	DECLARE_READ16_MEMBER(midxunit_sound_r);
	DECLARE_READ16_MEMBER(midxunit_sound_state_r);
	DECLARE_WRITE16_MEMBER(midxunit_sound_w);
	DECLARE_WRITE_LINE_MEMBER(midxunit_dcs_output_full);
	DECLARE_DRIVER_INIT(revx);
	DECLARE_MACHINE_RESET(midxunit);
	DECLARE_VIDEO_START(midxunit);
	void register_state_saving();
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);
};
