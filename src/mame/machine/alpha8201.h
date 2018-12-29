// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    Alpha Denshi ALPHA-8201 family protection emulation

***************************************************************************/

#ifndef MAME_MACHINE_ALPHA8201_H
#define MAME_MACHINE_ALPHA8201_H

#include "cpu/hmcs40/hmcs40.h"

class alpha_8201_device : public device_t
{
public:
	alpha_8201_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	~alpha_8201_device() {}

	// external I/O
	DECLARE_WRITE_LINE_MEMBER(bus_dir_w);
	DECLARE_WRITE_LINE_MEMBER(mcu_start_w);
	DECLARE_READ8_MEMBER(ext_ram_r);
	DECLARE_WRITE8_MEMBER(ext_ram_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
private:
	// devices/pointers
	required_device<hmcs40_cpu_device> m_mcu;

	// internal state
	int m_bus;                  // shared RAM bus direction
	u16 m_mcu_address;          // MCU side RAM address
	u16 m_mcu_d;                // MCU D output data
	u8 m_mcu_r[4];              // MCU R0-R3 output data
	std::unique_ptr<u8[]> m_shared_ram; // 1KB RAM

	void mcu_update_address();
	void mcu_writeram();

	DECLARE_READ8_MEMBER(mcu_data_r);
	DECLARE_WRITE8_MEMBER(mcu_data_w);
	DECLARE_WRITE16_MEMBER(mcu_d_w);
};


DECLARE_DEVICE_TYPE(ALPHA_8201, alpha_8201_device)


#endif // MAME_MACHINE_ALPHA8201_H
