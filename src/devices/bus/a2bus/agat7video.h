// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    agat7video.h

    Implementation of Agat-7 onboard video.

*********************************************************************/

#ifndef __A2BUS_AGAT7VIDEO__
#define __A2BUS_AGAT7VIDEO__

#include "emu.h"

#include "a2bus.h"
#include "machine/ram.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_agat7video_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_agat7video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	a2bus_agat7video_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_PALETTE_INIT(agat7);

	ram_device *m_ram_dev;
	uint8_t *m_ram_ptr, *m_aux_ptr, *m_char_ptr;
	int m_char_size;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_cnxx(address_space &space, uint8_t offset) override;
	virtual void write_cnxx(address_space &space, uint8_t offset, uint8_t data) override;

	void text_update_lores(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void text_update_hires(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void graph_update_mono(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void graph_update_lores(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void graph_update_hires(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);

private:
	void do_io(int offset);

	uint32_t m_start_address;
	enum {
		TEXT_LORES = 0,
		TEXT_HIRES,
		GRAPHICS_LORES,
		GRAPHICS_HIRES,
		GRAPHICS_MONO
	} m_video_mode;

	void plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, const uint8_t *textgfx_data, uint32_t textgfx_datalen, int fg, int bg);

public:
	required_device<palette_device> m_palette;
};

// device type definition
extern const device_type A2BUS_AGAT7VIDEO;

#endif /* __A2BUS_AGAT7VIDEO__ */
