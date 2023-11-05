// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Fabio Priuli
// thanks-to:Chris Covell
#ifndef MAME_BUS_SEGAAI_SOUNDBOX_H
#define MAME_BUS_SEGAAI_SOUNDBOX_H

#pragma once

#include "segaai_exp.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "sound/ymopm.h"

// ======================> segaai_soundbox_device

class segaai_soundbox_device : public device_t,
						public device_segaai_exp_interface
{
public:
	segaai_soundbox_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual u8 read_lo(offs_t offset) override;
	virtual void write_lo(offs_t offset, u8 data) override;
	virtual u8 read_hi(offs_t offset) override;
	virtual u8 read_io(offs_t offset) override;
	virtual void write_io(offs_t offset, u8 data) override;
	virtual ioport_constructor device_input_ports() const override;

	u8 tmp8255_porta_r();
	u8 tmp8255_portb_r();
	void tmp8255_portb_w(u8 data);
	void tmp8255_portc_w(u8 data);
	void ym2151_irq_w(int state);
	void tmp8253_out0_w(int state);
	void tmp8253_out1_w(int state);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<pit8253_device> m_tmp8253;
	required_device<i8255_device> m_tmp8255;
	required_device<ym2151_device> m_ym2151;
	required_region_ptr<u8> m_rom;
	required_ioport_array<8> m_rows;
	u8 m_ram[0x20000];    // 128KB Expansion RAM
	u8 m_row;
	u8 m_8255_portb;
};

DECLARE_DEVICE_TYPE(SEGAAI_SOUNDBOX, segaai_soundbox_device);

#endif
