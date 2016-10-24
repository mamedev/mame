// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * Data East Pinball DMD Type 1 display
 */

#ifndef DECODMD1_H_
#define DECODMD1_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"

#define MCFG_DECODMD_TYPE1_ADD(_tag, _region) \
	MCFG_DEVICE_ADD(_tag, DECODMD1, 0) \
	decodmd_type1_device::static_set_gfxregion(*device, _region);

#define B_CLR 0x01
#define B_SET 0x02
#define B_CLK 0x04

class decodmd_type1_device : public device_t
{
public:
	decodmd_type1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	required_device<cpu_device> m_cpu;
	required_memory_bank m_rombank1;
	required_memory_bank m_rombank2;
	required_device<ram_device> m_ram;
	memory_region* m_rom;

	uint32_t screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect );

	uint8_t latch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t busy_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ctrl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void status_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dmd_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dmd_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dmd_nmi(timer_device &timer, void *ptr, int32_t param);

	static void static_set_gfxregion(device_t &device, const char *tag);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t m_latch;
	uint8_t m_status;
	uint8_t m_ctrl;
	uint8_t m_busy;
	uint8_t m_command;
	uint8_t m_bank;
	uint8_t m_rowclock;
	uint8_t m_rowdata;
	uint32_t m_rowselect;
	uint8_t m_blank;
	uint32_t m_pxdata1;
	uint32_t m_pxdata2;
	uint32_t m_pxdata1_latched;
	uint32_t m_pxdata2_latched;
	bool m_frameswap;
	uint32_t m_pixels[0x200];
	uint8_t m_busy_lines;
	uint32_t m_prevrow;
	const char* m_gfxtag;

	void output_data();
	void set_busy(uint8_t input, uint8_t val);
};

extern const device_type DECODMD1;


#endif /* DECODMD1_H_ */
