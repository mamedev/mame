/****************************************************************************

    Nouspikel IDE controller card
    See tn_ide.c for documentation

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __TNIDE__
#define __TNIDE__

#include "emu.h"
#include "ti99defs.h"
#include "machine/rtc65271.h"

extern const device_type TI99_IDE;

class nouspikel_ide_interface_device : public ti_expansion_card_device
{
public:
	nouspikel_ide_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);

	void	crureadz(offs_t offset, UINT8 *value);
	void	cruwrite(offs_t offset, UINT8 value);

	void	do_inta(int state);
	bool	m_ide_irq;
	int 	m_cru_register;

	DECLARE_WRITE_LINE_MEMBER(clock_interrupt_callback);

protected:
	void device_start(void);
	void device_reset(void);
	const rom_entry *device_rom_region() const;
	machine_config_constructor device_mconfig_additions() const;
	ioport_constructor device_input_ports() const;

private:
	rtc65271_device*	m_rtc;
	device_t*			m_ide;

	bool	m_clk_irq;
	bool	m_sram_enable;
	bool	m_sram_enable_dip;
	int		m_cur_page;

	bool	m_tms9995_mode;

	UINT16	m_input_latch;
	UINT16	m_output_latch;

	UINT8	*m_ram;
};

#endif
