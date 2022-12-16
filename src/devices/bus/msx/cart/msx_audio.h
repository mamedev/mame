// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_MSX_AUDIO_H
#define MAME_BUS_MSX_CART_MSX_AUDIO_H

#pragma once

#include "bus/msx/slot/cartridge.h"
#include "sound/ymopl.h"
#include "machine/6850acia.h"
#include "bus/midi/midi.h"


DECLARE_DEVICE_TYPE(MSX_CART_MSX_AUDIO_HXMU900, msx_cart_msx_audio_hxmu900_device)
DECLARE_DEVICE_TYPE(MSX_CART_MSX_AUDIO_NMS1205, msx_cart_msx_audio_nms1205_device)
DECLARE_DEVICE_TYPE(MSX_CART_MSX_AUDIO_FSCA1,   msx_cart_msx_audio_fsca1_device)


class msx_cart_msx_audio_hxmu900_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_msx_audio_hxmu900_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

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
	msx_cart_msx_audio_nms1205_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

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
	msx_cart_msx_audio_fsca1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	void write_y8950(offs_t offset, u8 data);
	u8 read_y8950(offs_t offset);
	void y8950_io_w(u8 data);
	u8 y8950_io_r();
	void bank_w(u8 data);
	void write_7fff(u8 data);

	required_device<y8950_device> m_y8950;
	required_ioport m_io_config;
	required_memory_region m_region_y8950;
	memory_bank_array_creator<2> m_rombank;
	memory_view m_view0;
	memory_view m_view1;
	u8 m_7fff;
};

#endif // MAME_BUS_MSX_CART_MSX_AUDIO_H
