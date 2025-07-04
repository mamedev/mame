// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, David Carne

// Samsung KS0164 wavetable chip

#ifndef MAME_SOUND_KS0164_H
#define MAME_SOUND_KS0164_H

#pragma once

#include "cpu/ks0164/ks0164.h"
#include "diserial.h"

class ks0164_device : public device_t, public device_sound_interface, public device_memory_interface, public device_serial_interface
{
public:
	ks0164_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 16934400);

	void mpu401_data_w(u8 data);
	void mpu401_ctrl_w(u8 data);
	u8 mpu401_data_r();
	u8 mpu401_status_r();

	void midi_rx(int state) { rx_w(state); }
	auto midi_tx() { return m_midi_tx.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

private:
	enum {
		MPUS_RX_FULL = 0x01,
		MPUS_TX_FULL = 0x02,
		MPUS_RX_CTRL = 0x04,
		MPUS_TX_INT  = 0x40,
		MPUS_RX_INT  = 0x80
	};

	static const u16 sample_dec[0x100];

	devcb_write_line m_midi_tx;

	optional_memory_region m_mem_region;
	required_device<ks0164_cpu_device> m_cpu;
	address_space_config m_mem_config;
	sound_stream *m_stream;
	emu_timer *m_timer;
	memory_access<23, 1, 0, ENDIANNESS_BIG>::cache m_mem_cache;

	u32 m_bank1_base, m_bank2_base;
	u16 m_bank1_select, m_bank2_select;

	u16 m_sregs[0x20][0x20];

	u8 m_mpu_in;
	u8 m_mpu_out;
	u8 m_mpu_status;

	u8 m_midi_in;
	bool m_midi_in_active;

	u8 m_unk60;
	u8 m_voice_select;
	u8 m_irqen_76, m_irqen_77;
	bool m_timer_interrupt;

	util::notifier_subscription m_notif_rom_space;

	void cpu_map(address_map &map) ATTR_COLD;

	u16 vec_r(offs_t offset, u16 mem_mask);
	u16 rom_r(offs_t offset, u16 mem_mask);
	u16 bank1_r(offs_t offset, u16 mem_mask);
	void bank1_w(offs_t offset, u16 data, u16 mem_mask);
	u16 bank2_r(offs_t offset, u16 mem_mask);
	void bank2_w(offs_t offset, u16 data, u16 mem_mask);
	u16 bank1_select_r();
	void bank1_select_w(offs_t, u16 data, u16 mem_mask);
	u16 bank2_select_r();
	void bank2_select_w(offs_t, u16 data, u16 mem_mask);

	u8 irqen_76_r();
	void irqen_76_w(u8 data);
	u8 irqen_77_r();
	void irqen_77_w(u8 data);
	u8 unk60_r();
	void unk60_w(u8 data);
	u8 voice_select_r();
	void voice_select_w(u8 data);
	u16 voice_r(offs_t offset);
	void voice_w(offs_t offset, u16 data, u16 mem_mask);
	void mpuin_set(bool control, u8 data);

	u8 mpu401_r();
	void mpu401_w(u8 data);
	u8 mpu401_istatus_r();
	void mpu401_istatus_w(u8 data);

	u8 midi_r();
	void midi_w(u8 data);
	u8 midi_status_r();
	void midi_status_w(u8 data);
};

DECLARE_DEVICE_TYPE(KS0164, ks0164_device)

#endif
