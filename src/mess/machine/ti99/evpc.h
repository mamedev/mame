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
#include "ti99defs.h"
#include "peribox.h"
#include "dinvram.h"

extern const device_type TI99_EVPC;

typedef struct _evpc_palette
{
	UINT8		read_index, write_index, mask;
	int 		read;
	int 		state;
	struct { UINT8 red, green, blue; } color[0x100];
	//int dirty;
} evpc_palette;

class snug_enhanced_video_device : public ti_expansion_card_device, public device_nvram_interface
{
public:
	snug_enhanced_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);

	void	crureadz(offs_t offset, UINT8 *value);
	void	cruwrite(offs_t offset, UINT8 data);

protected:
	void device_start(void);
	void device_reset(void);
	void device_stop(void);

	const rom_entry *device_rom_region() const;
	ioport_constructor device_input_ports() const;

	void nvram_default();
	void nvram_read(emu_file &file);
	void nvram_write(emu_file &file);

private:
	UINT8*			m_dsrrom;
	bool			m_RAMEN;
	int 			m_dsr_page;
	UINT8*			m_novram;	/* NOVRAM area */
	evpc_palette	m_palette;
};

#endif
