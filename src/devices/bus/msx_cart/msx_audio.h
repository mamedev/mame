// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_MSX_AUDIO_H
#define MAME_BUS_MSX_CART_MSX_AUDIO_H

#pragma once

#include "bus/msx_cart/cartridge.h"
#include "sound/8950intf.h"
#include "machine/6850acia.h"
#include "bus/midi/midi.h"


DECLARE_DEVICE_TYPE(MSX_CART_MSX_AUDIO_HXMU900, msx_cart_msx_audio_hxmu900_device)
DECLARE_DEVICE_TYPE(MSX_CART_MSX_AUDIO_NMS1205, msx_cart_msx_audio_nms1205_device)
DECLARE_DEVICE_TYPE(MSX_CART_MSX_AUDIO_FSCA1,   msx_cart_msx_audio_fsca1_device)


class msx_cart_msx_audio_hxmu900_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_msx_audio_hxmu900_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void initialize_cartridge() override;

	virtual uint8_t read_cart(offs_t offset) override;

protected:
	virtual void device_start() override;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	required_device<y8950_device> m_y8950;
};


class msx_cart_msx_audio_nms1205_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_msx_audio_nms1205_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void initialize_cartridge() override;

	virtual uint8_t read_cart(offs_t offset) override;

protected:
	virtual void device_start() override;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	DECLARE_WRITE_LINE_MEMBER(midi_in);
	DECLARE_WRITE_LINE_MEMBER(irq_write);

	required_device<y8950_device> m_y8950;
	required_device<acia6850_device> m_acia6850;
	required_device<midi_port_device> m_mdout;
	required_device<midi_port_device> m_mdthru;
};


class msx_cart_msx_audio_fsca1_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_msx_audio_fsca1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void initialize_cartridge() override;

	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	void write_y8950(offs_t offset, uint8_t data);
	uint8_t read_y8950(offs_t offset);

	void y8950_io_w(uint8_t data);
	uint8_t y8950_io_r();

	required_device<y8950_device> m_y8950;
	required_ioport m_io_config;
	required_memory_region m_region_y8950;
	uint8_t m_7ffe;
	uint8_t m_7fff;
};

#endif // MAME_BUS_MSX_CART_MSX_AUDIO_H
