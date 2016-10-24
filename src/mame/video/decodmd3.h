// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *  Data East Pinball DMD Type 3 Display
 */

#ifndef DECODMD3_H_
#define DECODMD3_H_

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "video/mc6845.h"
#include "machine/ram.h"

#define MCFG_DECODMD_TYPE3_ADD(_tag, _region) \
	MCFG_DEVICE_ADD(_tag, DECODMD3, 0) \
	decodmd_type3_device::static_set_gfxregion(*device, _region);

class decodmd_type3_device : public device_t
{
public:
	decodmd_type3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	required_device<cpu_device> m_cpu;
	required_device<mc6845_device> m_mc6845;
	required_device<ram_device> m_ram;
	required_memory_bank m_rambank;
	required_memory_bank m_rombank;

	void data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t busy_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t latch_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void status_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void crtc_address_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void crtc_register_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t crtc_status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dmd_irq(timer_device &timer, void *ptr, int32_t param);
	MC6845_UPDATE_ROW(crtc_update_row);

	static void static_set_gfxregion(device_t &device, const char *tag);

	memory_region* m_rom;

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t m_status;
	uint8_t m_crtc_index;
	uint8_t m_crtc_reg[0x100];
	uint8_t m_latch;
	uint8_t m_ctrl;
	uint8_t m_busy;
	uint8_t m_command;

	const char* m_gfxtag;
};

extern const device_type DECODMD3;


#endif /* DECODMD3_H_ */
