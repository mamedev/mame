// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    NEC PC-9801-86

***************************************************************************/

#ifndef MAME_BUS_CBUS_PC9801_86_H
#define MAME_BUS_CBUS_PC9801_86_H

#pragma once

#include "bus/cbus/pc9801_cbus.h"
#include "sound/dac.h"
#include "sound/ymopn.h"
#include "pc9801_snd.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc9801_86_device

class pc9801_86_device : public pc9801_snd_device
{
public:
	// construction/destruction
	pc9801_86_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	pc9801_86_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void sound_irq(int state);

protected:
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
	virtual u16 read_io_base() override;

	TIMER_CALLBACK_MEMBER(dac_tick);

	required_device<pc9801_slot_device> m_bus;
	required_device<ym2608_device>  m_opna;

	void opna_map(address_map &map) ATTR_COLD;

	u8 opna_r(offs_t offset);
	void opna_w(offs_t offset, u8 data);
	virtual u8 id_r();
	void mask_w(u8 data);
	u8 pcm_r(offs_t offset);
	void pcm_w(offs_t offset, u8 data);

	u8 m_mask;

private:
	int queue_count();
	u8 queue_pop();

	u8 m_pcm_mode, m_vol[7], m_pcm_ctrl, m_pcm_mute;
	uint16_t m_head, m_tail, m_count, m_irq_rate;
	bool m_pcmirq, m_fmirq, m_pcm_clk, m_init;
	required_device<dac_word_interface> m_ldac;
	required_device<dac_word_interface> m_rdac;
	std::vector<u8> m_queue;
	emu_timer *m_dac_timer;
};

class pc9801_speakboard_device : public pc9801_86_device
{
public:
	// construction/destruction
	pc9801_speakboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	u8 opna_slave_r(offs_t offset);
	void opna_slave_w(offs_t offset, u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_device<ym2608_device>  m_opna_slave;
};

class otomichan_kai_device : public pc9801_86_device
{
public:
	// construction/destruction
	otomichan_kai_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	u8 opn2c_r(offs_t offset);
	void opn2c_w(offs_t offset, u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_device<ym3438_device>  m_opn2c;

	virtual u8 id_r() override;
};

// device type definition
DECLARE_DEVICE_TYPE(PC9801_86, pc9801_86_device)
DECLARE_DEVICE_TYPE(PC9801_SPEAKBOARD, pc9801_speakboard_device)
DECLARE_DEVICE_TYPE(OTOMICHAN_KAI, otomichan_kai_device)


#endif // MAME_BUS_CBUS_PC9801_86_H
