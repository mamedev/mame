// license:BSD-3-Clause
// copyright-holders:Carl

#ifndef MAME_BUS_ISA_SB16_H
#define MAME_BUS_ISA_SB16_H

#pragma once

#include "isa.h"
#include "bus/pc_joy/pc_joy.h"
#include "machine/input_merger.h"
#include "sound/ct1741.h"
#include "sound/ct1745.h"
#include "sound/dac.h"
#include "sound/ymopl.h"

//*********************************************************************
//   TYPE DEFINITIONS
//*********************************************************************

// ====================> sb16_device

class sb16_lle_device : public device_t, public device_isa16_card_interface
{
public:
	// construction/destruction
	sb16_lle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	uint8_t dack_r(int line) override;
	void dack_w(int line, uint8_t data) override;
	uint16_t dack16_r(int line) override;
	void dack16_w(int line, uint16_t data) override;

	virtual void remap(int space_id, offs_t start, offs_t end) override;
private:
	uint8_t mpu401_r(offs_t offset);
	void mpu401_w(offs_t offset, uint8_t data);

	void host_io(address_map &map) ATTR_COLD;

	required_device<ymf262_device> m_opl3;
	required_device<ct1741_dsp_device> m_dsp;
	required_device<ct1745_mixer_device> m_mixer;
	required_device<dac_16bit_r2r_device> m_ldac;
	required_device<dac_16bit_r2r_device> m_rdac;
	required_device<input_merger_device> m_irqs;
	required_device<pc_joy_device> m_joy;

	// internal state
	uint8_t m_mpu_byte;
	bool m_irq8, m_irq16, m_irq_midi;
	uint8_t m_irq_sel, m_dma_sel;
};

// device type definition

DECLARE_DEVICE_TYPE(ISA16_SB16, sb16_lle_device)

#endif // MAME_BUS_ISA_SB16_H
