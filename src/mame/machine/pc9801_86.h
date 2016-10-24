// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    NEC PC-9801-86

***************************************************************************/


#pragma once

#ifndef __PC9801_86DEV_H__
#define __PC9801_86DEV_H__

#include "machine/pic8259.h"
#include "sound/2608intf.h"
#include "sound/dac.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc9801_86_device

class pc9801_86_device : public device_t
{
public:
	// construction/destruction
	pc9801_86_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	uint8_t opn_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void opn_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t opn_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void opn_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t id_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pcm_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pcm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_irq(int state);
	virtual const tiny_rom_entry *device_rom_region() const override;
protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	void install_device(offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	int queue_count();
	uint8_t queue_pop();

	uint8_t m_joy_sel, m_mask, m_pcm_mode, m_vol[7], m_pcm_ctrl, m_pcm_mute;
	uint16_t m_head, m_tail, m_count, m_irq_rate;
	bool m_pcmirq, m_fmirq;
	required_device<ym2608_device>  m_opna;
	required_device<dac_word_interface> m_ldac;
	required_device<dac_word_interface> m_rdac;
	std::vector<uint8_t> m_queue;
	emu_timer *m_dac_timer;
};


// device type definition
extern const device_type PC9801_86;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
