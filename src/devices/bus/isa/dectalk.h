// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_BUS_ISA_DECTALK_H
#define MAME_BUS_ISA_DECTALK_H

#pragma once

#include "isa.h"
#include "sound/dac.h"
#include "cpu/i86/i186.h"
#include "cpu/tms32010/tms32010.h"

class dectalk_isa_device : public device_t,
						public device_isa8_card_interface
{
public:
	dectalk_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	DECLARE_READ_LINE_MEMBER(bio_line_r);
	DECLARE_WRITE_LINE_MEMBER(clock_w);

	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8_MEMBER(read);

	DECLARE_WRITE16_MEMBER(status_w);
	DECLARE_READ16_MEMBER(cmd_r);
	DECLARE_WRITE16_MEMBER(data_w);
	DECLARE_READ16_MEMBER(data_r);
	DECLARE_READ16_MEMBER(host_irq_r);
	DECLARE_READ8_MEMBER(dma_r);
	DECLARE_WRITE8_MEMBER(dma_w);
	DECLARE_WRITE16_MEMBER(dac_w);
	DECLARE_READ16_MEMBER(dsp_dma_r);
	DECLARE_WRITE16_MEMBER(dsp_dma_w);
	DECLARE_WRITE16_MEMBER(output_ctl_w);
	DECLARE_WRITE16_MEMBER(irq_line_w);

	void dectalk_cpu_io(address_map &map);
	void dectalk_cpu_map(address_map &map);
	void dectalk_dsp_io(address_map &map);
	void dectalk_dsp_map(address_map &map);

	uint16_t m_cmd, m_stat, m_data, m_dsp_dma, m_ctl;
	uint8_t m_dma, m_vol, m_bio;

	required_device<i80186_cpu_device> m_cpu;
	required_device<dac_12bit_r2r_device> m_dac;
	required_device<tms32015_device> m_dsp;
};

DECLARE_DEVICE_TYPE(ISA8_DECTALK, dectalk_isa_device)

#endif // MAME_BUS_ISA_DECTALK_H
