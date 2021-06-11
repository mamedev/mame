// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_Z180_HD64X180X_H
#define MAME_CPU_Z180_HD64X180X_H

#pragma once

#include "z180.h"

class hd641180x_device : public z180_device
{
public:
	enum
	{
		HD64X180X_T2FRC = Z180_IOLINES + 1,
		HD64X180X_T2OCR1, HD64X180X_T2OCR2,
		HD64X180X_T2ICR,
		HD64X180X_T2CSR1, HD64X180X_T2CSR2,
		HD64X180X_CCSR,
		HD64X180X_RMCR,
		HD64X180X_DERA,
		HD64X180X_ODRA, HD64X180X_ODRB, HD64X180X_ODRC, HD64X180X_ODRD, HD64X180X_ODRE, HD64X180X_ODRF,
		HD64X180X_DDRA, HD64X180X_DDRB, HD64X180X_DDRC, HD64X180X_DDRD, HD64X180X_DDRE, HD64X180X_DDRF
	};

	// construction/destruction
	hd641180x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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

protected:
	// construction/destruction
	hd641180x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-specific overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// z180_device overrides
	virtual uint8_t z180_internal_port_read(uint8_t port) override;
	virtual void z180_internal_port_write(uint8_t port, uint8_t data) override;

	// internal memory map
	void internal_map(address_map &map);

	// port callbacks
	devcb_read8::array<7> m_port_input_cb;
	devcb_write8::array<6> m_port_output_cb;

	// internal Memory space
	memory_view m_ram_view;

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
};

class hd643180x_device : public hd641180x_device
{
public:
	enum hd64x180x_mp_t : uint8_t
	{
		MP_SINGLE_CHIP = 0,
		MP_EXPANDED_WITHOUT_ROM,
		MP_EXPANDED_WITH_ROM,
		MP_PROM_PROGRAMMING // for HD647180X PROM
	};

	// construction/destruction
	hd643180x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_mp(hd64x180x_mp_t mp) { m_mp = mp; }

protected:
	// construction/destruction
	hd643180x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-specific overrides
	virtual void device_start() override;

private:
	// internal ROM (Mask ROM or PROM)
	optional_region_ptr<uint8_t> m_internal_rom;

	// configurable MP pin
	hd64x180x_mp_t m_mp;
};

class hd647180x_device : public hd643180x_device
{
public:
	// construction/destruction
	hd647180x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

};

// device type declaration
DECLARE_DEVICE_TYPE(HD641180X, hd641180x_device)
DECLARE_DEVICE_TYPE(HD643180X, hd643180x_device)
DECLARE_DEVICE_TYPE(HD647180X, hd647180x_device)

#endif // MAME_CPU_Z180_HD64X180X_H
