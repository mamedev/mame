// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    NEC PC-9801-86

***************************************************************************/

#ifndef MAME_BUS_PC98_CBUS_PC9801_86_H
#define MAME_BUS_PC98_CBUS_PC9801_86_H

#pragma once

#include "slot.h"

#include "bus/msx/ctrl/ctrl.h"
#include "machine/input_merger.h"
#include "sound/dac.h"
#include "sound/ymopn.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc9801_86_device

class pc9801_86_device : public device_t
					   , public device_pc98_cbus_slot_interface
{
public:
	// construction/destruction
	pc9801_86_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::MICROPHONE; }
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

protected:
	pc9801_86_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	u8 pcm_control_r();
	void pcm_control_w(u8 data);

	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void opna_reset_routes_config(machine_config &config);
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	void pc9801_86_config(machine_config &config);
	u16 read_io_base();

	TIMER_CALLBACK_MEMBER(dac_tick);

	required_device<ym2608_device>  m_opna;
	required_device<input_merger_device> m_irqs;

	void opna_map(address_map &map) ATTR_COLD;

	virtual u8 id_r();
	void mask_w(u8 data);

	u8 m_mask;
	virtual void io_map(address_map &map) ATTR_COLD;
	virtual void remap(int space_id, offs_t start, offs_t end) override;

private:
	int queue_count();
	u8 queue_pop();

	u8 m_pcm_mode, m_vol[7], m_pcm_ctrl, m_pcm_mute;
	u16 m_head, m_tail, m_count, m_irq_rate;
	bool m_pcmirq, m_pcm_clk, m_init;
	required_device<dac_16bit_r2r_twos_complement_device> m_ldac;
	required_device<dac_16bit_r2r_twos_complement_device> m_rdac;
	std::vector<u8> m_queue;
	optional_memory_region m_bios;
	required_device<msx_general_purpose_port_device> m_joy;

	emu_timer *m_dac_timer;

	void dac_transfer();

	u8 m_joy_sel;
};

class otomichan_kai_device : public pc9801_86_device
{
public:
	// construction/destruction
	otomichan_kai_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type imperfect_features() { return feature::SOUND; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void io_map(address_map &map) override ATTR_COLD;
private:
	required_device<ym3438_device>  m_opn2c;

	virtual u8 id_r() override;
};

// device type definition
DECLARE_DEVICE_TYPE(PC9801_86, pc9801_86_device)
DECLARE_DEVICE_TYPE(OTOMICHAN_KAI, otomichan_kai_device)


#endif // MAME_BUS_PC98_CBUS_PC9801_86_H
