// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "vt1682_io.h"

#define LOG_IO      (1U << 1)

#define LOG_ALL     (LOG_IO)

#define VERBOSE     (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(VT_VT1682_IO, vrt_vt1682_io_device, "vt1682io", "VRT VT1682 I/O")

vrt_vt1682_io_device::vrt_vt1682_io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VT_VT1682_IO, tag, owner, clock),
	m_porta_out(*this),
	m_portb_out(*this),
	m_portc_out(*this),
	m_portd_out(*this),
	m_porte_out(*this),
	m_portf_out(*this),
	m_porta_in(*this, 0),
	m_portb_in(*this, 0),
	m_portc_in(*this, 0),
	m_portd_in(*this, 0),
	m_porte_in(*this, 0),
	m_portf_in(*this, 0),
	m_xiof_in(*this, 0)
{
}

void vrt_vt1682_io_device::device_start()
{
	m_io_ab_o = 0;
	m_io_cd_o = 0;
	m_io_ef_o = 0;

	save_item(NAME(m_210d_ioconfig));
	save_item(NAME(m_211e_adconfig));
	save_item(NAME(m_214c_ioefconfig));
	save_item(NAME(m_io_ab_o));
	save_item(NAME(m_io_cd_o));
	save_item(NAME(m_io_ef_o));
}

void vrt_vt1682_io_device::device_reset()
{
	m_210d_ioconfig = 0;
	m_211e_adconfig = 0;
	m_214c_ioefconfig = 0;
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

uint8_t vrt_vt1682_io_device::vt1682_210d_ioconfig_r()
{
	uint8_t ret = m_210d_ioconfig;
	LOGMASKED(LOG_IO, "%s: vt1682_210d_ioconfig_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vrt_vt1682_io_device::vt1682_210d_ioconfig_w(uint8_t data)
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

uint8_t vrt_vt1682_io_device::vt1682_210e_io_ab_r()
{
	uint8_t ret;
	if (BIT(m_210d_ioconfig, 0))
		ret = m_io_ab_o & 0x0f;
	else
		ret = m_porta_in(0, 0x0f) & 0x0f;
	if (BIT(m_210d_ioconfig, 2))
		ret |= m_io_ab_o & 0xf0;
	else
		ret |= m_portb_in(0, 0x0f) << 4;

	return ret;
}

void vrt_vt1682_io_device::vt1682_210e_io_ab_w(uint8_t data)
{
	if (((m_io_ab_o ^ data) & 0x0f) != 0)
	{
		LOGMASKED(LOG_IO, "%s: IOA_O = %1x\n", machine().describe_context(), data & 0x0f);
		m_porta_out(0, data & 0x0f, 0x0f);
	}
	if (((m_io_ab_o ^ data) & 0xf0) != 0)
	{
		LOGMASKED(LOG_IO, "%s: IOB_O = %1x\n", machine().describe_context(), data >> 4);
		m_portb_out(0, data >> 4, 0x0f);
	}

	m_io_ab_o = data;
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

uint8_t vrt_vt1682_io_device::vt1682_210f_io_cd_r()
{
	uint8_t ret;
	if (BIT(m_210d_ioconfig, 4))
		ret = m_io_cd_o & 0x0f;
	else
		ret = m_portc_in(0, 0x0f) & 0x0f;
	if (BIT(m_210d_ioconfig, 6))
		ret |= m_io_cd_o & 0xf0;
	else
		ret |= m_portd_in(0, 0x0f) << 4;

	return ret;
}

void vrt_vt1682_io_device::vt1682_210f_io_cd_w(uint8_t data)
{
	if (((m_io_cd_o ^ data) & 0x0f) != 0)
	{
		LOGMASKED(LOG_IO, "%s: IOC_O = %1x\n", machine().describe_context(), data & 0x0f);
		m_portc_out(0, data & 0x0f, 0x0f);
	}
	if (((m_io_cd_o ^ data) & 0xf0) != 0)
	{
		LOGMASKED(LOG_IO, "%s: IOD_O = %1x\n", machine().describe_context(), data >> 4);
		m_portd_out(0, data >> 4, 0x0f);
	}

	m_io_cd_o = data;
}

/*
    Address 0x211e WRITE (MAIN CPU)

    0x80 - ADCEN
    0x40 - ADCS1
    0x20 - ADCS0
    0x10 - (unused)
    0x08 - IOFOEN3
    0x04 - IOFOEN2
    0x02 - IOFOEN1
    0x01 - IOFOEN0

    Address 0x211e READ (MAIN CPU)

    0x80 - ADC DATA:7
    0x40 - ADC DATA:6
    0x20 - ADC DATA:5
    0x10 - ADC DATA:4
    0x08 - ADC DATA:3
    0x04 - ADC DATA:2
    0x02 - ADC DATA:1
    0x01 - ADC DATA:0
*/

uint8_t vrt_vt1682_io_device::vt1682_211e_adc_data_r()
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_IO, "%s: vt1682_211e_adc_data_r (ADCSEL = %d)\n", machine().describe_context(), (m_211e_adconfig & 0x60) >> 5);
	return m_xiof_in[(m_211e_adconfig & 0x60) >> 5]();
}

void vrt_vt1682_io_device::vt1682_211e_adconfig_w(uint8_t data)
{
	LOGMASKED(LOG_IO, "%s: vt1682_211e_adconfig_w writing: %02x ( ADCEN:%d | ADCSEL:%d | IOF_OE:%1x )\n", machine().describe_context(), data,
		(data & 0x80) ? 1 : 0,
		(data & 0x60) >> 5,
		(data & 0x0f));
	if (((data ^ m_211e_adconfig) & 0x0f) != 0)
		m_portf_out(0, (m_io_ef_o >> 4 | ~data) & 0x0f, 0x0f);
	m_211e_adconfig = data;
}

/*
    Address 0x214c r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - Keychange Enable
    0x10 - Keychange Enable
    0x08 - IOFEN
    0x04 - (unused)
    0x02 - (unused)
    0x01 - IOEOEN
*/

uint8_t vrt_vt1682_io_device::vt1682_214c_ioefconfig_r()
{
	return m_214c_ioefconfig;
}

void vrt_vt1682_io_device::vt1682_214c_ioefconfig_w(uint8_t data)
{
	LOGMASKED(LOG_IO, "%s: vt1682_214c_ioefconfig_w writing: %02x ( IOF_ENB:%1x | IOE_OE:%1x )\n", machine().describe_context(), data,
		(data & 0x08) ? 1 : 0,
		(data & 0x01) ? 1 : 0);
	m_214c_ioefconfig = data;
}

/*
    Address 0x214d r/w (MAIN CPU)

    0x80 - IOF:3
    0x40 - IOF:2
    0x20 - IOF:1
    0x10 - IOF:0
    0x08 - IOE:3
    0x04 - IOE:2
    0x02 - IOE:1
    0x01 - IOE:0
*/

uint8_t vrt_vt1682_io_device::vt1682_214d_io_ef_r()
{
	uint8_t ioe_imask = (BIT(m_214c_ioefconfig, 0) ? 0 : 0x07) | (BIT(m_211e_adconfig, 4) ? 0 : 0x08);
	uint8_t ret = m_io_ef_o & ((~ioe_imask & 0x0f) | ((m_211e_adconfig & 0x0f) << 4));
	if (ioe_imask != 0)
		ret |= m_porte_in(0, ioe_imask) & ioe_imask;
	if ((m_211e_adconfig & 0x0f) != 0x0f)
		ret |= (m_portf_in(0, ~m_211e_adconfig & 0x0f) & ~m_211e_adconfig & 0x0f) << 4;
	return ret;
}

void vrt_vt1682_io_device::vt1682_214d_io_ef_w(uint8_t data)
{
	uint8_t ioe_omask = (BIT(m_214c_ioefconfig, 0) ? 0x07 : 0) | (BIT(m_211e_adconfig, 4) ? 0x08 : 0);
	if (((m_io_ef_o ^ data) & ioe_omask) != 0)
	{
		LOGMASKED(LOG_IO, "%s: IOE_O = %1x & %1x\n", machine().describe_context(), data & ioe_omask, ioe_omask);
		m_porte_out(0, data & ioe_omask, ioe_omask);
	}
	if ((((m_io_ef_o ^ data) >> 4) & (m_211e_adconfig & 0x0f)) != 0)
	{
		LOGMASKED(LOG_IO, "%s: IOF_O = %1x & %1x\n", machine().describe_context(), (data >> 4) & m_211e_adconfig & 0x0f, m_211e_adconfig & 0x0f);
		m_portf_out(0, (data >> 4 | ~m_211e_adconfig) & 0x0f, 0x0f);
	}

	m_io_ef_o = data;
}
