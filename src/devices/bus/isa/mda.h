// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic
#pragma once

#ifndef __ISA_MDA_H__
#define __ISA_MDA_H__

#include "emu.h"
#include "isa.h"
#include "video/mc6845.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_mda_device

class isa8_mda_device : public device_t,
	public device_isa8_card_interface
{
public:
	friend class isa8_hercules_device;

	// construction/destruction
	isa8_mda_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	isa8_mda_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	void hsync_changed(int state);
	void vsync_changed(int state);
	virtual uint8_t io_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual void io_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual uint8_t status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual void mode_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void pc_cpu_line(int state);

	virtual MC6845_UPDATE_ROW( crtc_update_row );
	MC6845_UPDATE_ROW( mda_text_inten_update_row );
	MC6845_UPDATE_ROW( mda_text_blink_update_row );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
public:
	int m_framecnt;

	uint8_t   m_mode_control;

	int     m_update_row_type;
	uint8_t   *m_chr_gen;
	uint8_t   m_vsync;
	uint8_t   m_hsync;
	std::vector<uint8_t> m_videoram;
	uint8_t   m_pixel;
	required_device<palette_device> m_palette;
};


// device type definition
extern const device_type ISA8_MDA;

// ======================> isa8_hercules_device

class isa8_hercules_device :
		public isa8_mda_device
{
public:
	// construction/destruction
	isa8_hercules_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual uint8_t io_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void io_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void mode_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual MC6845_UPDATE_ROW( crtc_update_row ) override;
	MC6845_UPDATE_ROW( hercules_gfx_update_row );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

public:
	uint8_t m_configuration_switch; //hercules
};


// device type definition
extern const device_type ISA8_HERCULES;

// ======================> isa8_ec1840_0002_device

class isa8_ec1840_0002_device :
		public isa8_mda_device
{
public:
	// construction/destruction
	isa8_ec1840_0002_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual void mode_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual MC6845_UPDATE_ROW( crtc_update_row ) override;
	MC6845_UPDATE_ROW( mda_lowres_text_inten_update_row );
	MC6845_UPDATE_ROW( mda_lowres_text_blink_update_row );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

public:
	std::unique_ptr<uint8_t[]>   m_soft_chr_gen;

};

// device type definition
extern const device_type ISA8_EC1840_0002;

#endif  /* __ISA_MDA_H__ */
