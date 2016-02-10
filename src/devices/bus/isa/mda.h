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
	isa8_mda_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	isa8_mda_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const rom_entry *device_rom_region() const override;

	DECLARE_WRITE_LINE_MEMBER(hsync_changed);
	DECLARE_WRITE_LINE_MEMBER(vsync_changed);
	virtual DECLARE_READ8_MEMBER(io_read);
	virtual DECLARE_WRITE8_MEMBER(io_write);
	virtual DECLARE_READ8_MEMBER(status_r);
	virtual DECLARE_WRITE8_MEMBER(mode_control_w);

	WRITE_LINE_MEMBER(pc_cpu_line);

	virtual MC6845_UPDATE_ROW( crtc_update_row );
	MC6845_UPDATE_ROW( mda_text_inten_update_row );
	MC6845_UPDATE_ROW( mda_text_blink_update_row );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
public:
	int m_framecnt;

	UINT8   m_mode_control;

	int     m_update_row_type;
	UINT8   *m_chr_gen;
	UINT8   m_vsync;
	UINT8   m_hsync;
	dynamic_buffer m_videoram;
	UINT8   m_pixel;
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
	isa8_hercules_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const rom_entry *device_rom_region() const override;

	virtual DECLARE_READ8_MEMBER(io_read) override;
	virtual DECLARE_WRITE8_MEMBER(io_write) override;
	virtual DECLARE_READ8_MEMBER(status_r) override;
	virtual DECLARE_WRITE8_MEMBER(mode_control_w) override;

	virtual MC6845_UPDATE_ROW( crtc_update_row ) override;
	MC6845_UPDATE_ROW( hercules_gfx_update_row );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

public:
	UINT8 m_configuration_switch; //hercules
};


// device type definition
extern const device_type ISA8_HERCULES;

// ======================> isa8_ec1840_0002_device

class isa8_ec1840_0002_device :
		public isa8_mda_device
{
public:
	// construction/destruction
	isa8_ec1840_0002_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual DECLARE_WRITE8_MEMBER(mode_control_w) override;

	virtual MC6845_UPDATE_ROW( crtc_update_row ) override;
	MC6845_UPDATE_ROW( mda_lowres_text_inten_update_row );
	MC6845_UPDATE_ROW( mda_lowres_text_blink_update_row );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

public:
	std::unique_ptr<UINT8[]>   m_soft_chr_gen;

};

// device type definition
extern const device_type ISA8_EC1840_0002;

#endif  /* __ISA_MDA_H__ */
