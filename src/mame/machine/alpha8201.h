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
	alpha_8201_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~alpha_8201_device() {}

	uint8_t mcu_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_d_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// external I/O
	void bus_dir_w(int state);
	void mcu_start_w(int state);
	uint8_t ext_ram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ext_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

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
	uint16_t m_mcu_address;           // MCU side RAM address
	uint16_t m_mcu_d;                 // MCU D output data
	uint8_t m_mcu_r[4];               // MCU R0-R3 output data
	std::unique_ptr<uint8_t[]> m_shared_ram;            // 1KB RAM

	void mcu_update_address();
	void mcu_writeram();
};


extern const device_type ALPHA_8201;


#endif /* _ALPHA8201_H_ */
