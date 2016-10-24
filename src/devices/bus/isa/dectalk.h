// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef ISA_DECTALK_H_
#define ISA_DECTALK_H_

#include "emu.h"
#include "isa.h"
#include "sound/dac.h"
#include "cpu/i86/i186.h"
#include "cpu/tms32010/tms32010.h"

class dectalk_isa_device : public device_t,
						public device_isa8_card_interface
{
public:
	dectalk_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void status_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t cmd_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t host_irq_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint8_t dma_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dma_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dac_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_dma_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_dma_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void output_ctl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	int bio_line_r();
	void irq_line_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void clock_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint16_t m_cmd, m_stat, m_data, m_dsp_dma, m_ctl;
	uint8_t m_dma, m_vol, m_bio;

	required_device<i80186_cpu_device> m_cpu;
	required_device<dac_12bit_r2r_device> m_dac;
	required_device<cpu_device> m_dsp;
};

extern const device_type ISA8_DECTALK;

#endif /* ISA_DECTALK_H_ */
