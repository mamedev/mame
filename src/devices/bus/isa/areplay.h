// license:BSD-3-Clause
// copyright-holders:Bavarese
/***************************************************************************

	Action Replay for DOS (ISA card 1994; DATEL UK)

***************************************************************************/

#ifndef MAME_BUS_ISA_AREPLAY_H
#define MAME_BUS_ISA_AREPLAY_H

#pragma once

#include "isa.h"
#include "machine/timer.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************
// One shot timer (555 timer with unknown interval)
// TODO: determine exact value
#define ONE_SHOT_TIMER_DELAY_MS 100

#define SIZE_ROM_WINDOW 0x2000
#define SIZE_RAM_BANK 0x1000


// This is the RAM bank chosen when the AR starts up (after Reset):
#define INITIAL_RAM_BANK_NO 1

// Assume the scratch is at bank 0 in RAM (0 never selected by firmware, which only accesses 1 - 1F)
#define SCRATCH_RAM_BANK_NO 0


// Port 1:
#define WRITE_ENABLE_BIT  0x01
#define FREEZE_BUTTON_BIT 0x02
#define ACTION_REPLAY_IRQ 0x08
#define SLOMO_SWITCH_BIT  0x40
#define LED_ENABLE_BIT    0x80

// Port 2:
#define PORT2_IRQ_ENABLE 0x80

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class isa8_areplay_device : 
        public device_t,
	public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_areplay_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(ar_read); // one read register
	DECLARE_WRITE8_MEMBER(ar_write); // two write registers.

	TIMER_DEVICE_CALLBACK_MEMBER(heartbeat_timer);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	uint8_t m_board_ram[ (0x1F + 1) * SIZE_RAM_BANK]; // maximum scratch RAM just to be safe (32 pages) 

        // 16 x 8 = 128K flash ROM. Extra bank (+1) was added to map write attempts (when flashing) 
	uint8_t m_banked_flash[ (16 + 1) * SIZE_ROM_WINDOW];

	uint8_t m_post_mortem_ram[ SIZE_RAM_BANK];
	uint8_t m_tsr_ram[ SIZE_RAM_BANK];

	uint8_t m_write_port1;

	int m_current_rom_start;
	int m_current_rom_page;
	int m_current_RAM_PAGE;

	int m_current_irq_selected;

	bool m_timer_fired;
	emu_timer   *one_shot_timer;

	bool m_is_heartbeat_present;

	void raise_processor_interrupt(int ref, bool state);
	void force_irq_to(bool flag);
};

// device type definition
DECLARE_DEVICE_TYPE(ISA8_AREPLAY, isa8_areplay_device)

#endif // MAME_BUS_ISA_AREPLAY_H
