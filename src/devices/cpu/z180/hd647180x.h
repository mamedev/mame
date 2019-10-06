// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_Z180_HD647180X_H
#define MAME_CPU_Z180_HD647180X_H

#pragma once

#include "z180.h"

class hd64x180x_base_device : public z180_device
{
public:
	enum
	{
		HD647180X_T2FRC = Z180_IOLINES + 1,
		HD647180X_T2OCR1, HD647180X_T2OCR2,
		HD647180X_T2ICR,
		HD647180X_T2CSR1, HD647180X_T2CSR2,
		HD647180X_CCSR,
		HD647180X_RMCR,
		HD647180X_DERA,
		HD647180X_ODRA, HD647180X_ODRB, HD647180X_ODRC, HD647180X_ODRD, HD647180X_ODRE, HD647180X_ODRF,
		HD647180X_DDRA, HD647180X_DDRB, HD647180X_DDRC, HD647180X_DDRD, HD647180X_DDRE, HD647180X_DDRF
	};

	auto in_pa_callback() { return m_port_input_cb[0].bind(); }
	auto in_pb_callback() { return m_port_input_cb[1].bind(); }
	auto in_pc_callback() { return m_port_input_cb[2].bind(); }
	auto in_pd_callback() { return m_port_input_cb[3].bind(); }
	auto in_pe_callback() { return m_port_input_cb[4].bind(); }
	auto in_pf_callback() { return m_port_input_cb[5].bind(); }
	auto in_pg_callback() { return m_port_input_cb[6].bind(); }
	auto out_pa_callback() { return m_port_output_cb[0].bind(); }
	auto out_pb_callback() { return m_port_output_cb[1].bind(); }
	auto out_pc_callback() { return m_port_output_cb[2].bind(); }
	auto out_pd_callback() { return m_port_output_cb[3].bind(); }
	auto out_pe_callback() { return m_port_output_cb[4].bind(); }
	auto out_pf_callback() { return m_port_output_cb[5].bind(); }

	void set_mp(bool mp1, bool mp0)
	{
		const uint8_t old_mp = m_mp;
		m_mp = mp1 ? (1 << 1) : 0 | mp0 ? (1 << 0) : 0;
		if (old_mp != 3 && m_mp == 3)
			set_disable(true);
		else if (old_mp == 3 && m_mp != 3)
			set_disable(false);
	}

	void prom_w(offs_t offset, u8 data)
	{
		if (mode_promwrite())
			m_data->write_byte(offset & 0x3fff, data);
	}

protected:
	// construction/destruction
	hd64x180x_base_device(const machine_config &mconfig, device_type &type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal_map);

	// device-specific overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// z180_device overrides
	virtual uint8_t z180_read_memory(offs_t addr) override;
	virtual void z180_write_memory(offs_t addr, uint8_t data) override;
	virtual uint8_t z180_read_port(offs_t port) override;
	virtual void z180_write_port(offs_t port, uint8_t data) override;
	virtual uint8_t z180_internal_port_read(uint8_t port) override;
	virtual void z180_internal_port_write(uint8_t port, uint8_t data) override;

	// port callbacks
	devcb_read8 m_port_input_cb[7];
	devcb_write8 m_port_output_cb[6];

	// internal RAM space
	address_space_config m_data_config;
	address_space *m_data;

	// internal registers
	PAIR16 m_t2frc;
	PAIR16 m_t2ocr[2];
	PAIR16 m_t2icr;
	uint8_t m_t2csr[2];
	uint8_t m_ccsr;
	uint8_t m_rmcr;
	uint8_t m_dera;
	uint8_t m_odr[6];
	uint8_t m_ddr[6];

	uint8_t m_mp;
	bool mp0() const { return BIT(m_mp, 0); }
	bool mp1() const { return BIT(m_mp, 1); }
	bool mode_single() const              { return !mp1() && !mp0(); }
	bool mode_expanded_nointrom() const   { return !mp1() &&  mp0(); }
	bool mode_expanded_withintrom() const { return  mp1() && !mp0(); }
	bool mode_promwrite() const           { return  mp1() &&  mp0(); }
};

class hd641180x_device : public hd64x180x_base_device
{
public:
	// construction/destruction
	hd641180x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	// internal memory maps
	void romless_map(address_map &map);
};

class hd643180x_device : public hd64x180x_base_device
{
public:
	// construction/destruction
	hd643180x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	// internal memory maps
	void maskrom_map(address_map &map);
};

class hd647180x_device : public hd64x180x_base_device
{
public:
	// construction/destruction
	hd647180x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	// internal memory maps
	void prom_map(address_map &map);
};

// device type declaration
DECLARE_DEVICE_TYPE(HD641180X, hd641180x_device)
DECLARE_DEVICE_TYPE(HD643180X, hd643180x_device)
DECLARE_DEVICE_TYPE(HD647180X, hd647180x_device)

#endif // MAME_CPU_Z180_HD647180X_H
