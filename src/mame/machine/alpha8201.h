// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    Alpha Denshi ALPHA-8201 family protection emulation

***************************************************************************/

#ifndef _ALPHA8201_H_
#define _ALPHA8201_H_

#include "emu.h"

class alpha_8201_device : public device_t
{
public:
	alpha_8201_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~alpha_8201_device() {}

	DECLARE_READ8_MEMBER(mcu_data_r);
	DECLARE_WRITE8_MEMBER(mcu_data_w);
	DECLARE_WRITE16_MEMBER(mcu_d_w);

	// external I/O
	DECLARE_WRITE_LINE_MEMBER(bus_dir_w);
	DECLARE_WRITE_LINE_MEMBER(mcu_start_w);
	DECLARE_READ8_MEMBER(ext_ram_r);
	DECLARE_WRITE8_MEMBER(ext_ram_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	// devices/pointers
	required_device<cpu_device> m_mcu;

	// internal state
	int m_bus;                      // shared RAM bus direction
	UINT16 m_mcu_address;           // MCU side RAM address
	UINT16 m_mcu_d;                 // MCU D output data
	UINT8 m_mcu_r[4];               // MCU R0-R3 output data
	std::unique_ptr<UINT8[]> m_shared_ram;            // 1KB RAM

	void mcu_update_address();
	void mcu_writeram();
};


extern const device_type ALPHA_8201;


#endif /* _ALPHA8201_H_ */
