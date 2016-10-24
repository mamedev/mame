// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *  Data East Pinball DMD Type 2 Display
 */

#ifndef DECODMD_H_
#define DECODMD_H_

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/mc6845.h"
#include "machine/ram.h"

#define MCFG_DECODMD_TYPE2_ADD(_tag, _region) \
	MCFG_DEVICE_ADD(_tag, DECODMD2, 0) \
	decodmd_type2_device::static_set_gfxregion(*device, _region);

#define START_ADDRESS       (((m_crtc_reg[0x0c]<<8) & 0x3f00) | (m_crtc_reg[0x0d] & 0xff))

class decodmd_type2_device : public device_t
{
public:
	decodmd_type2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	required_device<cpu_device> m_cpu;
	required_device<mc6845_device> m_mc6845;
	required_memory_bank m_rombank1;
	required_memory_bank m_rombank2;
	required_memory_bank m_rambank;
	required_device<ram_device> m_ram;
	memory_region* m_rom;

	void bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void crtc_address_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void crtc_register_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t crtc_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t latch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t busy_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ctrl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void status_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dmd_firq(timer_device &timer, void *ptr, int32_t param);
	MC6845_UPDATE_ROW(crtc_update_row);

	static void static_set_gfxregion(device_t &device, const char *tag);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t m_crtc_index;
	uint8_t m_crtc_reg[0x100];
	uint8_t m_latch;
	uint8_t m_status;
	uint8_t m_ctrl;
	uint8_t m_busy;
	uint8_t m_command;
	const char* m_gfxtag;
};

extern const device_type DECODMD2;

#endif /* DECODMD_H_ */
