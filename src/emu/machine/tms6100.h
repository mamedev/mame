// license:BSD-3-Clause
// copyright-holders:Couriersud
#pragma once

#ifndef __TMS6100_H__
#define __TMS6100_H__

/* TMS 6100 memory controller */

class tms6100_device : public device_t
{
public:
	tms6100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms6100_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	DECLARE_WRITE_LINE_MEMBER( tms6100_m0_w );
	DECLARE_WRITE_LINE_MEMBER( tms6100_m1_w );
	DECLARE_WRITE_LINE_MEMBER( tms6100_romclock_w );
	DECLARE_WRITE8_MEMBER( tms6100_addr_w );

	DECLARE_READ_LINE_MEMBER( tms6100_data_r );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	required_region_ptr<UINT8> m_rom;
	UINT32 m_address;
	UINT32 m_address_latch;
	UINT8  m_loadptr;
	UINT8  m_m0;
	UINT8  m_m1;
	UINT8  m_addr_bits;
	UINT8  m_tms_clock;
	UINT8  m_data;
	UINT8  m_state;
	//UINT8  m_variant;

};

extern const device_type TMS6100;

class m58819_device : public tms6100_device
{
public:
	m58819_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type M58819;


#endif /* __TMS6100_H__ */
