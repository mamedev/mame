// license:BSD-3-Clause
// copyright-holders:Brian H. Gilbert
#ifndef MAME_BUS_NES_DRIP_H
#define MAME_BUS_NES_DRIP_H

#pragma once

#include "nxrom.h"


// ======================> nes_drip_sound_device

// two FIFO driven sample playback channels, one sample per output clock (the CPU clock)
class nes_drip_sound_device : public device_t, public device_sound_interface
{
public:
	nes_drip_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u8 status_r(unsigned channel);
	void write(offs_t offset, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;

private:
	sound_stream *m_stream;
	u8 m_fifo[2 * 256];
	u8 m_head[2];       // index of the sample being played
	u16 m_count[2];     // samples in the FIFO, the one being played included
	u16 m_period[2];    // CPU cycles per sample, 12 bits
	u16 m_timer[2];
	u8 m_volume[2];
};


// ======================> nes_dripgame_device

class nes_dripgame_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_dripgame_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;
	virtual u8 nt_r(offs_t offset) override;

	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<nes_drip_sound_device> m_snd;
	required_ioport m_dipsw;

	u8 m_ext_attr[2][0x400];    // write-only attribute RAM, one page for each nametable
	u8 m_irq_low;
	u16 m_irq_count;
	bool m_irq_enable;
	u8 m_mirror;
	bool m_ext_attr_enable;
	bool m_sram_write;
	u16 m_last_nt;              // address of the last nametable byte fetched, for the attribute fetch that follows
};


// device type definition
DECLARE_DEVICE_TYPE(NES_DRIP_SOUND, nes_drip_sound_device)
DECLARE_DEVICE_TYPE(NES_DRIPGAME,   nes_dripgame_device)

#endif // MAME_BUS_NES_DRIP_H
