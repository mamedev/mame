// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_NINTENDO_VT1682_IO_H
#define MAME_NINTENDO_VT1682_IO_H

#pragma once

DECLARE_DEVICE_TYPE(VT_VT1682_IO, vrt_vt1682_io_device)

class vrt_vt1682_io_device : public device_t
{
public:
	// construction/destruction
	vrt_vt1682_io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto porta_out() { return m_porta_out.bind(); }
	auto portb_out() { return m_portb_out.bind(); }
	auto portc_out() { return m_portc_out.bind(); }
	auto portd_out() { return m_portd_out.bind(); }
	auto porte_out() { return m_porte_out.bind(); }
	auto portf_out() { return m_portf_out.bind(); }
	auto porta_in() { return m_porta_in.bind(); }
	auto portb_in() { return m_portb_in.bind(); }
	auto portc_in() { return m_portc_in.bind(); }
	auto portd_in() { return m_portd_in.bind(); }
	auto porte_in() { return m_porte_in.bind(); }
	auto portf_in() { return m_portf_in.bind(); }
	template <int N> auto xiof_in() { return m_xiof_in[N].bind(); }

	uint8_t vt1682_210e_io_ab_r();
	uint8_t vt1682_210f_io_cd_r();
	uint8_t vt1682_214d_io_ef_r();

	void vt1682_210e_io_ab_w(uint8_t data);
	void vt1682_210f_io_cd_w(uint8_t data);
	void vt1682_214d_io_ef_w(uint8_t data);

	uint8_t vt1682_210d_ioconfig_r();
	void vt1682_210d_ioconfig_w(uint8_t data);
	uint8_t vt1682_211e_adc_data_r();
	void vt1682_211e_adconfig_w(uint8_t data);
	uint8_t vt1682_214c_ioefconfig_r();
	void vt1682_214c_ioefconfig_w(uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_io_ab_o;
	uint8_t m_io_cd_o;
	uint8_t m_io_ef_o;

	uint8_t m_210d_ioconfig;
	uint8_t m_211e_adconfig;
	uint8_t m_214c_ioefconfig;

	// actually 4-bit ports
	devcb_write8 m_porta_out;
	devcb_write8 m_portb_out;
	devcb_write8 m_portc_out;
	devcb_write8 m_portd_out;
	devcb_write8 m_porte_out;
	devcb_write8 m_portf_out;
	devcb_read8 m_porta_in;
	devcb_read8 m_portb_in;
	devcb_read8 m_portc_in;
	devcb_read8 m_portd_in;
	devcb_read8 m_porte_in;
	devcb_read8 m_portf_in;
	devcb_read8::array<4> m_xiof_in;
};

#endif // MAME_NINTENDO_VT1682_IO_H
