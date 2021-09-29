// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

NEC PC-80S31K

***************************************************************************/

#ifndef MAME_MACHINE_PC80S31K_H
#define MAME_MACHINE_PC80S31K_H

#pragma once

#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "machine/upd765.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class pc80s31_device : public device_t
{
public:
	// construction/destruction
	pc80s31_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void host_map(address_map &map);

protected:
	pc80s31_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void fdc_io(address_map &map);

	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	u8 m_irq_vector;

private:
	required_device<z80_device> m_fdc_cpu;
	required_memory_region m_fdc_rom;
	required_device<i8255_device> m_ppi_host;
	required_device<i8255_device> m_ppi_fdc;
	required_device_array<generic_latch_8_device, 6> m_latch;

	void fdc_map(address_map &map);

	template <unsigned N> u8 latch_r();
	template <unsigned N> void latch_w(u8 data);

	u8 terminal_count_r(address_space &space);
	void motor_control_w(u8 data);

	emu_timer *m_tc_zero_timer;

	IRQ_CALLBACK_MEMBER(irq_cb);
};

class pc80s31k_device : public pc80s31_device
{
public:
	pc80s31k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void fdc_io(address_map &map) override;

private:
	void drive_mode_w(u8 data);
};

// device type definition
DECLARE_DEVICE_TYPE(PC80S31, pc80s31_device)
DECLARE_DEVICE_TYPE(PC80S31K, pc80s31k_device)

#endif // MAME_MACHINE_PC80S31K_H
