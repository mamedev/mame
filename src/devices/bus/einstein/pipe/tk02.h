// license: GPL-2.0+
// copyright-holders: Dirk Best, Phill Harvey-Smith
/***************************************************************************

    TK02 80 Column Monochrome Unit

***************************************************************************/

#ifndef MAME_BUS_EINSTEIN_TK02_H
#define MAME_BUS_EINSTEIN_TK02_H

#pragma once

#include "pipe.h"
#include "video/mc6845.h"
#include "emupal.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tk02_device

class tk02_device : public device_t, public device_tatung_pipe_interface
{
public:
	// construction/destruction
	tk02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void map(address_map &map);

	DECLARE_WRITE_LINE_MEMBER(de_w);

	DECLARE_READ8_MEMBER(ram_r);
	DECLARE_WRITE8_MEMBER(ram_w);
	DECLARE_READ8_MEMBER(status_r);

	MC6845_UPDATE_ROW(crtc_update_row);

	required_device<tatung_pipe_device> m_pipe;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_memory_region m_gfx;
	required_ioport_array<4> m_links;

	std::unique_ptr<uint8_t[]> m_ram;
	int m_de;
};

// device type definition
DECLARE_DEVICE_TYPE(TK02_80COL, tk02_device)

#endif // MAME_BUS_EINSTEIN_TK02_H
