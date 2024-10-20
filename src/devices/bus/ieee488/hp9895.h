// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp9895.h

    HP9895 floppy disk drive

*********************************************************************/

#ifndef MAME_BUS_IEEE488_HP9895_H
#define MAME_BUS_IEEE488_HP9895_H

#pragma once

#include "ieee488.h"
#include "cpu/z80/z80.h"
#include "machine/phi.h"
#include "imagedev/floppy.h"
#include "machine/fdc_pll.h"

class hp9895_device : public device_t,
					  public device_ieee488_interface
{
public:
	// construction/destruction
	hp9895_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_ieee488_interface overrides
	virtual void ieee488_eoi(int state) override;
	virtual void ieee488_dav(int state) override;
	virtual void ieee488_nrfd(int state) override;
	virtual void ieee488_ndac(int state) override;
	virtual void ieee488_ifc(int state) override;
	virtual void ieee488_srq(int state) override;
	virtual void ieee488_atn(int state) override;
	virtual void ieee488_ren(int state) override;

private:
	// PHI write CBs
	void phi_eoi_w(int state);
	void phi_dav_w(int state);
	void phi_nrfd_w(int state);
	void phi_ndac_w(int state);
	void phi_ifc_w(int state);
	void phi_srq_w(int state);
	void phi_atn_w(int state);
	void phi_ren_w(int state);

	// PHI DIO r/w CBs
	uint8_t phi_dio_r();
	void phi_dio_w(uint8_t data);

	// PHI IRQ/Z80 NMI
	void phi_int_w(int state);

	// Z80 IRQ
	void z80_m1_w(uint8_t data);

	// Floppy interface
	void data_w(uint8_t data);
	void clock_w(uint8_t data);
	void reset_w(uint8_t data);
	void leds_w(uint8_t data);
	void cntl_w(uint8_t data);
	void drv_w(uint8_t data);
	void xv_w(uint8_t data);
	uint8_t data_r();
	uint8_t clock_r();
	uint8_t drivstat_r();
	uint8_t switches_r();
	uint8_t switches2_r();

	// PHI register read
	uint8_t phi_reg_r(offs_t offset);

	// Floppy drive interface
	void floppy_ready_cb(floppy_image_device *floppy , int state);

	TIMER_CALLBACK_MEMBER(timeout_timer_tick);
	TIMER_CALLBACK_MEMBER(byte_timer_tick);
	TIMER_CALLBACK_MEMBER(half_bit_timer_tick);

	uint8_t get_switches2(void) const;
	attotime get_half_bit_cell_period(void) const;
	floppy_image_device *get_write_device(void) const;
	void preset_crc(void);
	void update_crc(bool bit);
	bool shift_sr(uint8_t& sr , bool input_bit);
	void get_next_transition(const attotime& from_when , attotime& edge);
	void read_bit(bool crc_upd);
	void write_bit(bool data_bit , bool clock_bit);

	void z80_io_map(address_map &map) ATTR_COLD;
	void z80_program_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_cpu;
	required_device<phi_device> m_phi;
	required_device_array<floppy_connector, 2> m_drives;
	required_ioport m_switches;

	bool m_cpu_irq;
	floppy_image_device *m_current_drive;
	unsigned m_current_drive_idx;
	bool m_dskchg[ 2 ];
	uint16_t m_crc; // U77
	bool m_crcerr_syn;
	bool m_overrun;
	bool m_accdata;
	bool m_timeout;
	uint8_t m_cntl_reg; // U31
	uint8_t m_clock_sr; // U22 & U4
	uint8_t m_clock_reg;    // U23 & U5
	uint8_t m_data_sr;  // U24 & U6
	uint8_t m_wr_context;
	bool m_had_transition;
	bool m_lckup;
	bool m_amdt;
	uint8_t m_sync_cnt; // U28 & U73
	bool m_hiden;
	bool m_mgnena;

	// Timers
	emu_timer *m_timeout_timer;
	emu_timer *m_byte_timer;
	emu_timer *m_half_bit_timer;

	// PLL
	fdc_pll_t m_pll;
};

// device type definition
DECLARE_DEVICE_TYPE(HP9895, hp9895_device)

#endif // MAME_BUS_IEEE488_HP9895_H
