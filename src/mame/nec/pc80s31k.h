// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

NEC PC-80S31K

***************************************************************************/

#ifndef MAME_NEC_PC80S31K_H
#define MAME_NEC_PC80S31K_H

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

	void host_map(address_map &map) ATTR_COLD;

protected:
	pc80s31_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void fdc_io(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(tc_zero_tick);

	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	u8 m_irq_vector = 0;

	required_device<z80_device> m_fdc_cpu;
private:
	required_memory_region m_fdc_rom;
	required_device<i8255_device> m_ppi_host;
	required_device<i8255_device> m_ppi_fdc;
	required_device_array<generic_latch_8_device, 6> m_latch;

	void fdc_map(address_map &map) ATTR_COLD;

	template <unsigned N> u8 latch_r();
	template <unsigned N> void latch_w(u8 data);

	u8 terminal_count_r(address_space &space);
	void motor_control_w(u8 data);

	emu_timer *m_tc_zero_timer = nullptr;

	IRQ_CALLBACK_MEMBER(irq_cb);
};

class pc80s31k_device : public pc80s31_device
{
public:
	pc80s31k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	pc80s31k_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void fdc_io(address_map &map) override ATTR_COLD;

private:
	void drive_mode_w(u8 data);
};

class pc88va2_fd_if_device : public pc80s31k_device
{
public:
	pc88va2_fd_if_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_wr_callback() { return m_write_irq.bind(); }
	auto drq_wr_callback() { return m_write_drq.bind(); }

	void host_io(address_map &map) ATTR_COLD;

	void tc_w(int state) { m_fdc->tc_w(state); }
	u8 dack_r() { return m_fdc->dma_r(); }
	void dack_w(u8 data) { m_fdc->dma_w(data); }

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	devcb_write_line    m_write_irq;
	devcb_write_line    m_write_drq;

	emu_timer *m_motor_start_timer[2]{};
	emu_timer *m_fdc_timer = nullptr;

	template <unsigned DriveN> TIMER_CALLBACK_MEMBER(motor_start_timer_cb);
	TIMER_CALLBACK_MEMBER(fdc_timer_cb);

	void fdc_update_ready(floppy_image_device *, int);

	void host_mode_w(u8 data);
	void host_drive_rate_w(u8 data);
	void host_motor_control_w(u8 data);
	void host_fdc_control_w(u8 data);
	u8 host_ready_r();

	u8 m_fdc_ctrl_2 = 0;
	u8 m_fdc_mode = 0;
	bool m_xtmask = false;
	bool m_dmae = false;
};

// device type definition
DECLARE_DEVICE_TYPE(PC80S31,       pc80s31_device)
DECLARE_DEVICE_TYPE(PC80S31K,      pc80s31k_device)
DECLARE_DEVICE_TYPE(PC88VA2_FD_IF, pc88va2_fd_if_device)

#endif // MAME_NEC_PC80S31K_H
