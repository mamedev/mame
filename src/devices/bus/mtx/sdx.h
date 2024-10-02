// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    MTX SDX Controller

**********************************************************************/

#ifndef MAME_BUS_MTX_EXP_SDX_H
#define MAME_BUS_MTX_EXP_SDX_H

#include "exp.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class mtx_sdx_device :
	public device_t,
	public device_mtx_exp_interface
{
public:
	static void floppy_formats(format_registration &fr);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	uint8_t sdx_status_r();
	void sdx_control_w(uint8_t data);
	void motor_w(int state);

protected:
	// construction/destruction
	mtx_sdx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	required_memory_region m_sdx_rom;
	required_device<mb8877_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_ioport_array<2> m_dsw;
	uint8_t m_control;
};

class mtx_sdxbas_device : public mtx_sdx_device
{
public:
	// construction/destruction
	mtx_sdxbas_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

class mtx_sdxcpm_device : public mtx_sdx_device
{
public:
	// construction/destruction
	mtx_sdxcpm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t mtx_80col_r(offs_t offset);
	void mtx_80col_w(offs_t offset, uint8_t data);
	MC6845_UPDATE_ROW(crtc_update_row);

	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<hd6845s_device> m_crtc;
	required_memory_region m_char_rom;
	uint8_t m_80col_char_ram[0x800];
	uint8_t m_80col_attr_ram[0x800];
	uint8_t m_80col_ascii, m_80col_attr;
	uint16_t m_80col_addr;
};



// device type definition
DECLARE_DEVICE_TYPE(MTX_SDXBAS, mtx_sdxbas_device)
DECLARE_DEVICE_TYPE(MTX_SDXCPM, mtx_sdxcpm_device)


#endif // MAME_BUS_MTX_EXP_SDX_H
