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

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc9801_86_device

class pc9801_86_device : public device_t
{
public:
	// construction/destruction
	pc9801_86_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(opn_r);
	DECLARE_WRITE8_MEMBER(opn_w);
	DECLARE_READ8_MEMBER(id_r);
	DECLARE_WRITE8_MEMBER(mask_w);
	DECLARE_READ8_MEMBER(pcm_r);
	DECLARE_WRITE8_MEMBER(pcm_w);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	void install_device(offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	int queue_count();
	uint8_t queue_pop();

	required_device<pc9801_slot_device> m_bus;

	uint8_t m_joy_sel, m_mask, m_pcm_mode, m_vol[7], m_pcm_ctrl, m_pcm_mute;
	uint16_t m_head, m_tail, m_count, m_irq_rate;
	bool m_pcmirq, m_fmirq, m_pcm_clk, m_init;
	required_device<ym2608_device>  m_opna;
	required_device<dac_word_interface> m_ldac;
	required_device<dac_word_interface> m_rdac;
	std::vector<uint8_t> m_queue;
	emu_timer *m_dac_timer;

	DECLARE_WRITE_LINE_MEMBER(sound_irq);
	DECLARE_READ8_MEMBER(opn_porta_r);
	DECLARE_WRITE8_MEMBER(opn_portb_w);
};


// device type definition
DECLARE_DEVICE_TYPE(PC9801_86, pc9801_86_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif // MAME_BUS_CBUS_PC9801_86_H
