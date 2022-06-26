// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************
 
    TexElec Snark Barker New Wave MCA
    OPL3-based Sound Blaster 2.0 clone.

***************************************************************************/

#ifndef MAME_BUS_MCA_SNARK_BARKER_H
#define MAME_BUS_MCA_SNARK_BARKER_H

#pragma once

#include "mca.h"
#include "bus/pc_joy/pc_joy.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/dac.h"
#include "sound/ymopl.h"
#include "sound/spkrdev.h"
#include "speaker.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mca16_snark_barker_device

class mca16_snark_barker_device :
		public device_t,
		public device_mca16_card_interface
{
public:
	// construction/destruction
	mca16_snark_barker_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void unmap() override;
    virtual void remap() override;

	virtual uint8_t io8_r(offs_t offset) override;
	virtual void io8_w(offs_t offset, uint8_t data) override;

	virtual void pos_w(offs_t offset, uint8_t data) override;

    void update_pos(uint8_t data);

	uint8_t ym3812_16_r(offs_t offset);
	void ym3812_16_w(offs_t offset, uint8_t data);

	uint8_t dsp_reset_r(offs_t offset);
	uint8_t dsp_data_r(offs_t offset);
	uint8_t dsp_wbuf_status_r(offs_t offset);
	uint8_t dsp_rbuf_status_r(offs_t offset);

	void dsp_reset_w(offs_t offset, uint8_t data);
	void dsp_data_w(offs_t offset, uint8_t data);
	void dsp_cmd_w(offs_t offset, uint8_t data);
	void dsp_rbuf_status_w(offs_t offset, uint8_t data);

	void map_dsp_program(address_map &map);
	void map_dsp_io(address_map &map);

protected:
	mca16_snark_barker_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual bool map_has_changed() override;

	virtual uint8_t dack_r(int line) override;
	virtual void dack_w(int line, uint8_t data) override;

	uint8_t dsp_latch_r(offs_t offset);
	void dsp_latch_w(offs_t offset, uint8_t data);

	required_device<i80c51_device> m_dsp;
	required_device<pc_joy_device> m_joy;
    required_device<ym3812_device> m_ym3812;
	required_device<mc1408_device> m_dac;
	required_device<speaker_device> m_speaker;

private:
	uint8_t dsp_port0_r();
	uint8_t dsp_port2_r();
	uint8_t dsp_port3_r();
	void dsp_port0_w(uint8_t data);
	void dsp_port1_w(uint8_t data);
	void dsp_port2_w(uint8_t data);
	void dsp_port3_w(uint8_t data);

	void raise_irq();
	void lower_irq();
	void raise_dma();
	void lower_dma();

	uint8_t get_pos_irq();
	uint8_t get_pos_dma();

	bool 		m_is_mapped;

	uint8_t		m_cur_dma_line;
	uint8_t		m_cur_irq_line;

	uint16_t 	m_setup_io_port;

	bool m_irq_in_flag;
	bool m_dav_pc;
	bool m_dav_dsp;
	bool m_dma_en;

	bool m_irequest;
	bool m_drequest;

	uint8_t m_host_to_dsp_latch;
	uint8_t m_dsp_to_host_latch;

	bool m_irq_raised;
	bool m_dma_raised;
};

// device type definition
DECLARE_DEVICE_TYPE(MCA16_SNARK_BARKER, mca16_snark_barker_device)

#endif