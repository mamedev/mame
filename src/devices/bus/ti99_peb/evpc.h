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
	snug_enhanced_video_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;

	DECLARE_READ8Z_MEMBER(crureadz) override;
	DECLARE_WRITE8_MEMBER(cruwrite) override;

protected:
	virtual void device_start(void) override;
	virtual void device_reset(void) override;
	virtual void device_stop(void) override;

	virtual const rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	void nvram_default() override;
	void nvram_read(emu_file &file) override;
	void nvram_write(emu_file &file) override;

private:
	UINT8*          m_dsrrom;
	bool            m_RAMEN;
	int             m_dsr_page;
	std::unique_ptr<UINT8[]>          m_novram;   /* NOVRAM area */
	evpc_palette    m_palette;
};

#endif
