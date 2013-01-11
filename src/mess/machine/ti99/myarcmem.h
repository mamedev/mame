/****************************************************************************

    TI-99 Myarc memory expansion
    See myarcmem.c for documentation

    Michael Zapf, September 2010
    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __MYARCMEM__
#define __MYARCMEM__

#include "emu.h"
#include "peribox.h"
#include "ti99defs.h"

extern const device_type TI99_MYARCMEM;

class myarc_memory_expansion_device : public ti_expansion_card_device
{
public:
	myarc_memory_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);

	void crureadz(offs_t offset, UINT8 *value);
	void cruwrite(offs_t offset, UINT8 data);

protected:
	void device_start(void);
	void device_reset(void);
	const rom_entry *device_rom_region(void) const;
	ioport_constructor device_input_ports() const;

private:
	int     get_base(int offset);
	UINT8*  m_ram;
	UINT8*  m_dsrrom;
	int     m_bank;
	int     m_size;
};

#endif
