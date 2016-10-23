// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef __STEREO_FX__
#define __STEREO_FX__

#include "emu.h"
#include "isa.h"
#include "bus/pc_joy/pc_joy.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/3812intf.h"

//*********************************************************************
//   TYPE DEFINITIONS
//*********************************************************************

// ====================> stereo_fx_device

class stereo_fx_device : public device_t,
						public device_isa8_card_interface
{
public:
	// construction/destruction
	stereo_fx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	required_device<pc_joy_device> m_joy;
	required_device<cpu_device> m_cpu;

	// mcu ports
	uint8_t dev_dsp_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dev_dsp_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t p1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t p3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dev_host_irq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void raise_drq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port20_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port00_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// host ports
	uint8_t dsp_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dsp_cmd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dsp_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dsp_wbuf_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dsp_rbuf_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t invalid_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void invalid_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	uint8_t dack_r(int line) override;
	void dack_w(int line, uint8_t data) override;
private:
	// internal state
	bool m_data_in;
	uint8_t m_in_byte;
	bool m_data_out;
	uint8_t m_out_byte;

	uint8_t m_port20;
	uint8_t m_port00;
	emu_timer *m_timer;
	uint8_t m_t0;
	uint8_t m_t1;
};

// device type definition

extern const device_type ISA8_STEREO_FX;

#endif
