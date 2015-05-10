// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_CART_MSX_AUDIO_H
#define __MSX_CART_MSX_AUDIO_H

#include "bus/msx_cart/cartridge.h"
#include "sound/8950intf.h"
#include "machine/6850acia.h"
#include "bus/midi/midi.h"


extern const device_type MSX_CART_MSX_AUDIO_NMS1205;
extern const device_type MSX_CART_MSX_AUDIO_HXMU900;
extern const device_type MSX_CART_MSX_AUDIO_FSCA1;


class msx_cart_msx_audio_hxmu900 : public device_t
								, public msx_cart_interface
{
public:
	msx_cart_msx_audio_hxmu900(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual void initialize_cartridge();

	virtual DECLARE_READ8_MEMBER(read_cart);

private:
	required_device<y8950_device> m_y8950;
};


class msx_cart_msx_audio_nms1205 : public device_t
						, public msx_cart_interface
{
public:
	msx_cart_msx_audio_nms1205(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual const rom_entry *device_rom_region() const;

	virtual void initialize_cartridge();

	virtual DECLARE_READ8_MEMBER(read_cart);

	DECLARE_WRITE_LINE_MEMBER(midi_in);
	DECLARE_WRITE_LINE_MEMBER(irq_write);

private:
	required_device<y8950_device> m_y8950;
	required_device<acia6850_device> m_acia6850;
	required_device<midi_port_device> m_mdout;
	required_device<midi_port_device> m_mdthru;
};


class msx_cart_msx_audio_fsca1 : public device_t
						, public msx_cart_interface
{
public:
	msx_cart_msx_audio_fsca1(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;
	virtual const rom_entry *device_rom_region() const;

	virtual void initialize_cartridge();

	virtual DECLARE_READ8_MEMBER(read_cart);
	virtual DECLARE_WRITE8_MEMBER(write_cart);

	DECLARE_WRITE8_MEMBER(write_y8950);
	DECLARE_READ8_MEMBER(read_y8950);

	DECLARE_WRITE8_MEMBER(y8950_io_w);
	DECLARE_READ8_MEMBER(y8950_io_r);

private:
	required_device<y8950_device> m_y8950;
	required_ioport m_io_config;
	required_memory_region m_region_y8950;
	UINT8 m_7ffe;
	UINT8 m_7fff;
};

#endif
