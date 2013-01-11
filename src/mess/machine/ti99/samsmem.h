/****************************************************************************

    TI-99 SuperAMS memory expansion
    See samsmem.c for documentation

    Michael Zapf
    September 2010

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __SAMSMEM__
#define __SAMSMEM__

#include "emu.h"
#include "peribox.h"
#include "ti99defs.h"

extern const device_type TI99_SAMSMEM;

class sams_memory_expansion_device : public ti_expansion_card_device
{
public:
	sams_memory_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);

	void crureadz(offs_t offset, UINT8 *value);
	void cruwrite(offs_t offset, UINT8 data);

protected:
	void device_start(void);
	void device_reset(void);
	const rom_entry *device_rom_region(void) const;

private:
	UINT8*  m_ram;
	int     m_mapper[16];
	bool    m_map_mode;
	bool    m_access_mapper;
};
#endif
