// license:BSD-3-Clause
// copyright-holders: Joakim Larsson Edström
#ifndef MAME_BUS_ISA_EIS_HGB107X_H
#define MAME_BUS_ISA_EIS_HGB107X_H

#pragma once

#include "isa.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class isa8_epc_mda_device : public device_t, public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_epc_mda_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t io_read(offs_t offset);
	void io_write(offs_t offset, uint8_t data);
	uint8_t status_r();
	void mode_control_w(uint8_t data);
	void hsync_changed(int state);
	void vsync_changed(int state);

	/* Monitor */
	DECLARE_INPUT_CHANGED_MEMBER(monitor_changed);

protected:
	isa8_epc_mda_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	inline int get_xres();
	inline int get_yres();

	enum {
		VM_COLS80 = 0x01,
		VM_GRAPH  = 0x02,
		VM_HOR640 = 0x04,
		VM_MONO   = 0x08,
		VM_VER400 = 0x10
	};

	enum {
		MR1_COLS80 = 0x01,
		MR1_GRAPH = 0x02,
		MR1_VIDEO = 0x08,
		MR1_HOR640 = 0x10,
		MR1_BLINK = 0x20
	};

	enum {
		MR2_COLEMU = 0x04,
		MR2_CHRSET = 0x40,
		MR2_VER400 = 0x80
	};

	enum {
		ATTR_BLINK = 0x80,
		ATTR_BACKG = 0x70,
		ATTR_INTEN = 0x08,
		ATTR_FOREG = 0x07,
		ATTR_ULINE = 0x01,
	};

	required_device<mc6845_device> m_crtc;
	MC6845_UPDATE_ROW( crtc_update_row );

	std::unique_ptr<uint8_t[]>   m_soft_chr_gen;
	required_ioport m_s1;
	uint8_t m_color_mode;
	uint8_t m_mode_control2;
	required_device<screen_device> m_screen;
	required_ioport m_io_monitor;
	required_region_ptr<uint8_t> m_chargen;

	uint8_t m_vmode;
	rgb_t (*m_pal)[4];
	rgb_t m_3111_pal[4];
	rgb_t m_371x_pal[4];
	bool m_installed;
	hd6845s_device *m_hd6845s;
public:
	int m_framecnt;

	uint8_t   m_mode_control;

	int     m_update_row_type;
	uint8_t   *m_chr_gen;
	uint8_t   m_vsync;
	uint8_t   m_hsync;
	std::vector<uint8_t> m_videoram;
	uint8_t   m_pixel;
};

// device type definition
DECLARE_DEVICE_TYPE(ISA8_EPC_MDA, isa8_epc_mda_device)

#endif // MAME_BUS_ISA_EIS_HGB107X_H
