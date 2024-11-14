// license:BSD-3-Clause
// copyright-holders:Barry Rodewald

#ifndef MAME_VIDEO_PC_VGA_ATI_H
#define MAME_VIDEO_PC_VGA_ATI_H

#pragma once

#include "machine/eepromser.h"
#include "video/ati_mach8.h"
#include "video/pc_vga.h"

#include "screen.h"


class ati_vga_device : public svga_device
{
public:
	// construction/destruction
	ati_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;

	// VGA registers
	uint8_t ati_port_ext_r(offs_t offset);
	void ati_port_ext_w(offs_t offset, uint8_t data);

	virtual uint16_t offset() override;

	mach8_device* get_8514() { return m_8514; }
protected:
	ati_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void ati_define_video_mode();
	void set_dot_clock();
	struct
	{
		uint8_t ext_reg[64];
		uint8_t ext_reg_select;
		uint8_t vga_chip_id;
	} ati;

private:
	mach8_device* m_8514;
};

// device type definition
DECLARE_DEVICE_TYPE(ATI_VGA, ati_vga_device)

#endif // MAME_VIDEO_PC_VGA_ATI_H
