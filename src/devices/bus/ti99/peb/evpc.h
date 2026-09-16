// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    SNUG Enhanced Video Processor Card (evpc)
    See evpc.c for documentation.

    Michael Zapf

    October 2010: Rewritten as device
    February 2012: Rewritten as class

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_EVPC_H
#define MAME_BUS_TI99_PEB_EVPC_H

#pragma once

#include "peribox.h"
#include "machine/timer.h"
#include "video/v9938.h"
#include "sound/sn76496.h"
#include "bus/ti99/colorbus/colorbus.h"
#include "bus/ti99/internal/evpcconn.h"

namespace bus::ti99::peb {

class snug_enhanced_video_device : public device_t, public device_ti99_peribox_card_interface, public device_nvram_interface
{
public:
	snug_enhanced_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
	void setaddress_dbin(offs_t offset, int state) override;

	void crureadz(offs_t offset, uint8_t *value) override;
	void cruwrite(offs_t offset, uint8_t data) override;

	TIMER_DEVICE_CALLBACK_MEMBER( hblank_interrupt );

protected:
	struct evpc_palette
	{
		uint8_t       read_index, write_index, mask;
		int         read;
		int         state;
		struct { uint8_t red, green, blue; } color[0x100];
		//int dirty;
	};

	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void device_stop() override;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	void ready_line(int state);

	void video_interrupt_in(int state);

	int     m_address;
	int     m_dsr_page;
	bool    m_inDsrArea;
	bool    m_novram_accessed;
	bool    m_palette_accessed;
	bool    m_RAMEN;
	bool    m_sound_accessed;
	bool    m_video_accessed;

	int     m_intlevel;

	uint8_t*  m_dsrrom;

	std::unique_ptr<uint8_t[]>          m_novram;   // NOVRAM area

	evpc_palette                            m_palette;
	required_device<v9938_device>           m_video;
	required_device<sn76496_base_device>    m_sound;
	required_device<bus::ti99::colorbus::v9938_colorbus_device>   m_colorbus;
	optional_device<bus::ti99::internal::evpc_clock_connector>   m_console_conn;
};

} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_EVPC, bus::ti99::peb, snug_enhanced_video_device)

#endif // MAME_BUS_TI99_PEB_EVPC_H
