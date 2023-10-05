// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
#ifndef MAME_BUS_ISA_SBLASTER_LLE_H
#define MAME_BUS_ISA_SBLASTER_LLE_H

#pragma once

#include "isa.h"
#include "bus/pc_joy/pc_joy.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/dac.h"
#include "sound/ymopl.h"
#include "sound/spkrdev.h"
#include "speaker.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class isa8_sblaster_2_0_lle_device :
	public device_t,
	public device_isa8_card_interface
{
public:
	isa8_sblaster_2_0_lle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t ym3812_16_r(offs_t offset);
	void    ym3812_16_w(offs_t offset, uint8_t data);

	uint8_t dsp_reset_r(offs_t offset) { return 0; }
	uint8_t dsp_data_r(offs_t offset);
	uint8_t dsp_wbuf_status_r(offs_t offset);
	uint8_t dsp_rbuf_status_r(offs_t offset);

	void    dsp_reset_w(offs_t offset, uint8_t data);
	void    dsp_data_w(offs_t offset, uint8_t data) {}
	void    dsp_cmd_w(offs_t offset, uint8_t data);
	void    dsp_rbuf_status_w(offs_t offset, uint8_t data) {}

	void    map_dsp_program(address_map &map);
	void    map_dsp_io(address_map &map);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual uint8_t dack_r(int line) override;
	virtual void dack_w(int line, uint8_t data) override;

	uint8_t dsp_latch_r(offs_t offset);
	void    dsp_latch_w(offs_t offset, uint8_t data);

private:
	uint8_t dsp_port2_r();
	uint8_t dsp_port3_r();

	void    dsp_port2_w(uint8_t data);
	void    dsp_port3_w(uint8_t data);

	void    raise_irq();
	void    lower_irq();
	void    raise_dma();
	void    lower_dma();

	bool m_irq_in_flag;
	bool m_dav_pc;
	bool m_dav_dsp;
	bool m_dma_en;

	bool m_irequest;
	bool m_drequest;

	bool m_irq_raised;
	bool m_dma_raised;

	uint8_t m_host_to_dsp_latch;
	uint8_t m_dsp_to_host_latch;

	required_device<i80c51_device> m_dsp;
	required_device<pc_joy_device> m_joy;
	required_device<ym3812_device> m_ym3812;
	required_device<mc1408_device> m_dac;
	required_device<speaker_device> m_speaker;
};

DECLARE_DEVICE_TYPE(ISA8_SOUND_BLASTER_2_0_LLE, isa8_sblaster_2_0_lle_device)

#endif
