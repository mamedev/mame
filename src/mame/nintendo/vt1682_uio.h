// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_NINTENDO_VT1682_UIO_H
#define MAME_NINTENDO_VT1682_UIO_H

#pragma once

DECLARE_DEVICE_TYPE(VT_VT1682_UIO, vrt_vt1682_uio_device)

class vrt_vt1682_uio_device : public device_t
{
public:
	// construction/destruction
	vrt_vt1682_uio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto porta_out() { return m_porta_out.bind(); }
	auto porta_in() { return m_porta_in.bind(); }
	auto portb_out() { return m_portb_out.bind(); }
	auto portb_in() { return m_portb_in.bind(); }

	uint8_t inteact_2129_uio_a_data_r();
	void inteact_2129_uio_a_data_w(uint8_t data);
	uint8_t inteact_212a_uio_a_direction_r();
	void inteact_212a_uio_a_direction_w(uint8_t data);
	uint8_t inteact_212b_uio_a_attribute_r();
	void inteact_212b_uio_a_attribute_w(uint8_t data);

	uint8_t inteact_2149_uio_b_data_r();
	void inteact_2149_uio_b_data_w(uint8_t data);
	uint8_t inteact_214a_uio_b_direction_r();
	void inteact_214a_uio_b_direction_w(uint8_t data);
	uint8_t inteact_214b_uio_b_attribute_r();
	void inteact_214b_uio_b_attribute_w(uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_write8 m_porta_out;
	devcb_read8 m_porta_in;
	devcb_write8 m_portb_out;
	devcb_read8 m_portb_in;

	uint8_t m_2129_uio_a_data;
	uint8_t m_212a_uio_a_direction;
	uint8_t m_212b_uio_a_attribute;

	uint8_t m_2149_uio_b_data;
	uint8_t m_214a_uio_b_direction;
	uint8_t m_214b_uio_b_attribute;

};

#endif // MAME_NINTENDO_VT1682_UIO_H
