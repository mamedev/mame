// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
#ifndef MAME_AUDIO_T5182_H
#define MAME_AUDIO_T5182_H

#pragma once

#include "sound/ymopm.h"
#include "cpu/z80/z80.h"

class t5182_device : public device_t

{
public:
	t5182_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	enum
	{
		VECTOR_INIT,
		YM2151_ASSERT,
		YM2151_CLEAR,
		YM2151_ACK,
		CPU_ASSERT,
		CPU_CLEAR
	};

	enum
	{
		SETIRQ_CB
	};

	void sound_irq_w(uint8_t data);
	uint8_t sharedram_semaphore_snd_r();
	void sharedram_semaphore_main_acquire_w(uint8_t data);
	void sharedram_semaphore_main_release_w(uint8_t data);
	uint8_t sharedram_r(offs_t offset);
	void sharedram_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(ym2151_irq_handler);

	void t5182_io(address_map &map);
	void t5182_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	// internal state
	required_device<cpu_device> m_ourcpu;
	required_shared_ptr<uint8_t> m_sharedram;
	int m_irqstate;
	int m_semaphore_main;
	int m_semaphore_snd;
	emu_timer *m_setirq_cb;
	TIMER_CALLBACK_MEMBER( setirq_callback );

	void sharedram_semaphore_snd_acquire_w(uint8_t data);
	void sharedram_semaphore_snd_release_w(uint8_t data);
	uint8_t sharedram_semaphore_main_r();
	void ym2151_irq_ack_w(uint8_t data);
	void cpu_irq_ack_w(uint8_t data);
};

DECLARE_DEVICE_TYPE(T5182, t5182_device)

#endif // MAME_AUDIO_T5182_H
