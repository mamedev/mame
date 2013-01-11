/****************************************************************************

    TI-99 P-Code Card emulation.
    See p_code.c for documentation

    Michael Zapf
    July 2009
    Revised July 2010
    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __PCODE__
#define __PCODE__

#include "emu.h"
#include "peribox.h"
#include "grom.h"

extern const device_type TI99_P_CODE;

class ti_pcode_card_device : public ti_expansion_card_device
{
public:
	ti_pcode_card_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);
	void crureadz(offs_t offset, UINT8 *value);
	void cruwrite(offs_t offset, UINT8 data);

	DECLARE_WRITE_LINE_MEMBER( ready_line );
	DECLARE_INPUT_CHANGED_MEMBER( switch_changed );

protected:
	void device_start();
	void device_reset();
	void device_config_complete();
	const rom_entry *device_rom_region() const;
	machine_config_constructor device_mconfig_additions() const;
	ioport_constructor device_input_ports() const;

private:
	ti99_grom_device*   m_grom[8];
	UINT8*              m_rom;
	int                 m_bank_select;
	bool                m_switch;
};

#endif
