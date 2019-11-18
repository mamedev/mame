// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    NEC PC-9801-86

***************************************************************************/

#ifndef MAME_BUS_CBUS_PC9801_86_H
#define MAME_BUS_CBUS_PC9801_86_H

#pragma once

#include "bus/cbus/pc9801_cbus.h"
#include "sound/2608intf.h"
#include "sound/dac.h"
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

	DECLARE_READ8_MEMBER(opna_r);
	DECLARE_WRITE8_MEMBER(opna_w);
	DECLARE_READ8_MEMBER(id_r);
	DECLARE_WRITE8_MEMBER(mask_w);
	DECLARE_READ8_MEMBER(pcm_r);
	DECLARE_WRITE8_MEMBER(pcm_w);

	DECLARE_WRITE_LINE_MEMBER(sound_irq);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	void pc9801_86_config(machine_config &config);

	required_device<pc9801_slot_device> m_bus;
	required_device<ym2608_device>  m_opna;

private:
	int queue_count();
	uint8_t queue_pop();


	uint8_t m_mask, m_pcm_mode, m_vol[7], m_pcm_ctrl, m_pcm_mute;
	uint16_t m_head, m_tail, m_count, m_irq_rate;
	bool m_pcmirq, m_fmirq, m_pcm_clk, m_init;
	required_device<dac_word_interface> m_ldac;
	required_device<dac_word_interface> m_rdac;
	std::vector<uint8_t> m_queue;
	emu_timer *m_dac_timer;
};

class pc9801_speakboard_device : public pc9801_86_device
{
public:
	// construction/destruction
	pc9801_speakboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(opna_slave_r);
	DECLARE_WRITE8_MEMBER(opna_slave_w);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	required_device<ym2608_device>  m_opna_slave;
};


// device type definition
DECLARE_DEVICE_TYPE(PC9801_86, pc9801_86_device)
DECLARE_DEVICE_TYPE(PC9801_SPEAKBOARD, pc9801_speakboard_device)





#endif // MAME_BUS_CBUS_PC9801_86_H
