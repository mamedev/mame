// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "machine/vt1682_io.h"

#define LOG_IO     (1U << 1)

#define LOG_ALL           ( LOG_IO )

#define VERBOSE             (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(VT_VT1682_IO, vrt_vt1682_io_device, "vt1682io", "VRT VT1682 I/O")

vrt_vt1682_io_device::vrt_vt1682_io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VT_VT1682_IO, tag, owner, clock),
	m_porta_out(*this),
	m_portb_out(*this),
	m_portc_out(*this),
	m_portd_out(*this),
	m_porta_in(*this),
	m_portb_in(*this),
	m_portc_in(*this),
	m_portd_in(*this)
{
}

void vrt_vt1682_io_device::device_start()
{
	m_porta_out.resolve_safe();
	m_portb_out.resolve_safe();
	m_portc_out.resolve_safe();
	m_portd_out.resolve_safe();
	m_porta_in.resolve_safe(0);
	m_portb_in.resolve_safe(0);
	m_portc_in.resolve_safe(0);
	m_portd_in.resolve_safe(0);

	save_item(NAME(m_210d_ioconfig));
}

void vrt_vt1682_io_device::device_reset()
{
	m_210d_ioconfig = 0;
}


/*
    Address 0x210d r/w (MAIN CPU)

    0x80 - IOD ENB
    0x40 - IOD OE
    0x20 - IOC ENB
    0x10 - IOC OE
    0x08 - IOB ENB
    0x04 - IOB OE
    0x02 - IOA ENB
    0x01 - IOA OE
*/

READ8_MEMBER(vrt_vt1682_io_device::vt1682_210d_ioconfig_r)
{
	uint8_t ret = m_210d_ioconfig;
	LOGMASKED(LOG_IO, "%s: vt1682_210d_ioconfig_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

WRITE8_MEMBER(vrt_vt1682_io_device::vt1682_210d_ioconfig_w)
{
	LOGMASKED(LOG_IO, "%s: vt1682_210d_ioconfig_w writing: %02x ( IOD_ENB:%1x IOD_OE:%1x | IOC_ENB:%1x IOC_OE:%1x | IOB_ENB:%1x IOB_OE:%1x | IOA_ENB:%1x IOA_OE:%1x )\n", machine().describe_context(), data,
		(data & 0x80) ? 1 : 0,
		(data & 0x40) ? 1 : 0,
		(data & 0x20) ? 1 : 0,
		(data & 0x10) ? 1 : 0,
		(data & 0x08) ? 1 : 0,
		(data & 0x04) ? 1 : 0,
		(data & 0x02) ? 1 : 0,
		(data & 0x01) ? 1 : 0);

	m_210d_ioconfig = data;
}

/*
    Address 0x210e r/w (MAIN CPU)

    0x80 - IOB:3
    0x40 - IOB:2
    0x20 - IOB:1
    0x10 - IOB:0
    0x08 - IOA:3
    0x04 - IOA:2
    0x02 - IOA:1
    0x01 - IOA:0
*/

READ8_MEMBER(vrt_vt1682_io_device::vt1682_210e_io_ab_r)
{
	uint8_t ret = (m_porta_in(0, 0x0f) & 0x0f) | ((m_portb_in(0, 0x0f) & 0x0f)<<4);
	LOGMASKED(LOG_IO, "%s: vt1682_210e_io_ab_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

WRITE8_MEMBER(vrt_vt1682_io_device::vt1682_210e_io_ab_w)
{
	LOGMASKED(LOG_IO, "%s: vt1682_210e_io_ab_w writing: %02x (portb: %1x porta: %1x)\n", machine().describe_context(), data, (data & 0xf0) >> 4, data & 0x0f);
	m_porta_out(0, (data & 0x0f), 0x0f);
	m_portb_out(0, ((data & 0xf0)>>4), 0x0f);
}

/*
    Address 0x210f r/w (MAIN CPU)

    0x80 - IOD:3
    0x40 - IOD:2
    0x20 - IOD:1
    0x10 - IOD:0
    0x08 - IOC:3
    0x04 - IOC:2
    0x02 - IOC:1
    0x01 - IOC:0
*/

READ8_MEMBER(vrt_vt1682_io_device::vt1682_210f_io_cd_r)
{
	uint8_t ret = (m_portc_in(0, 0x0f) & 0x0f) | ((m_portd_in(0, 0x0f) & 0x0f)<<4);
	LOGMASKED(LOG_IO, "%s: vt1682_210f_io_cd_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

WRITE8_MEMBER(vrt_vt1682_io_device::vt1682_210f_io_cd_w)
{
	LOGMASKED(LOG_IO, "%s: vt1682_210f_io_cd_w writing: %02x (portd: %1x portc: %1x)\n", machine().describe_context(), data, (data & 0xf0) >> 4, data & 0x0f);
	m_portc_out(0, (data & 0x0f), 0x0f);
	m_portd_out(0, ((data & 0xf0)>>4), 0x0f);
}
