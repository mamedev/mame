// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    SNUG Enhanced Video Processor Card (evpc)
    See evpc.c for documentation.

    Michael Zapf

    October 2010: Rewritten as device
    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __EVPC__
#define __EVPC__

#include "emu.h"
#include "peribox.h"
#include "video/v9938.h"
#include "sound/sn76496.h"
#include "bus/ti99x/ti99defs.h"

extern const device_type TI99_EVPC;

struct evpc_palette
{
	UINT8       read_index, write_index, mask;
	int         read;
	int         state;
	struct { UINT8 red, green, blue; } color[0x100];
	//int dirty;
};

class snug_enhanced_video_device : public ti_expansion_card_device, public device_nvram_interface
{
public:
	snug_enhanced_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
	DECLARE_SETADDRESS_DBIN_MEMBER(setaddress_dbin) override;

	DECLARE_WRITE_LINE_MEMBER( ready_line );

	DECLARE_READ8Z_MEMBER(crureadz) override;
	DECLARE_WRITE8_MEMBER(cruwrite) override;
	DECLARE_WRITE_LINE_MEMBER( video_interrupt_in );

protected:
	void device_start(void) override;
	void device_reset(void) override;
	void device_stop(void) override;

	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	machine_config_constructor device_mconfig_additions() const override;

	void nvram_default() override;
	void nvram_read(emu_file &file) override;
	void nvram_write(emu_file &file) override;

private:
	int     m_address;
	int     m_dsr_page;
	bool    m_inDsrArea;
	bool    m_novram_accessed;
	bool    m_palette_accessed;
	bool    m_RAMEN;
	bool    m_sound_accessed;
	bool    m_video_accessed;

	UINT8*  m_dsrrom;

	std::unique_ptr<UINT8[]>          m_novram;   // NOVRAM area

	evpc_palette                            m_palette;
	required_device<v9938_device>           m_video;
	required_device<sn76496_base_device>    m_sound;
	evpc_clock_connector*                   m_console_conn;
};

#endif
