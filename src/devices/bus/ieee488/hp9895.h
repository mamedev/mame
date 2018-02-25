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
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

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
	DECLARE_WRITE_LINE_MEMBER(phi_eoi_w);
	DECLARE_WRITE_LINE_MEMBER(phi_dav_w);
	DECLARE_WRITE_LINE_MEMBER(phi_nrfd_w);
	DECLARE_WRITE_LINE_MEMBER(phi_ndac_w);
	DECLARE_WRITE_LINE_MEMBER(phi_ifc_w);
	DECLARE_WRITE_LINE_MEMBER(phi_srq_w);
	DECLARE_WRITE_LINE_MEMBER(phi_atn_w);
	DECLARE_WRITE_LINE_MEMBER(phi_ren_w);

	// PHI DIO r/w CBs
	DECLARE_READ8_MEMBER(phi_dio_r);
	DECLARE_WRITE8_MEMBER(phi_dio_w);

	// PHI IRQ/Z80 NMI
	DECLARE_WRITE_LINE_MEMBER(phi_int_w);

	// Z80 IRQ
	DECLARE_WRITE8_MEMBER(z80_m1_w);

	// Floppy interface
	DECLARE_WRITE8_MEMBER(data_w);
	DECLARE_WRITE8_MEMBER(clock_w);
	DECLARE_WRITE8_MEMBER(reset_w);
	DECLARE_WRITE8_MEMBER(leds_w);
	DECLARE_WRITE8_MEMBER(cntl_w);
	DECLARE_WRITE8_MEMBER(drv_w);
	DECLARE_WRITE8_MEMBER(xv_w);
	DECLARE_READ8_MEMBER(data_r);
	DECLARE_READ8_MEMBER(clock_r);
	DECLARE_READ8_MEMBER(drivstat_r);
	DECLARE_READ8_MEMBER(switches_r);
	DECLARE_READ8_MEMBER(switches2_r);

	// PHI register read
	DECLARE_READ8_MEMBER(phi_reg_r);

	// Floppy drive interface
	void floppy_ready_cb(floppy_image_device *floppy , int state);

	void z80_io_map(address_map &map);
	void z80_program_map(address_map &map);

	required_device<z80_device> m_cpu;
	required_device<phi_device> m_phi;
	required_device<floppy_connector> m_drives[ 2 ];
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

	uint8_t get_switches2(void) const;
	attotime get_half_bit_cell_period(void) const;
	floppy_image_device *get_write_device(void) const;
	void preset_crc(void);
	void update_crc(bool bit);
	bool shift_sr(uint8_t& sr , bool input_bit);
	void get_next_transition(const attotime& from_when , attotime& edge);
	void read_bit(bool crc_upd);
	void write_bit(bool data_bit , bool clock_bit);
};

// device type definition
DECLARE_DEVICE_TYPE(HP9895, hp9895_device)

#endif // MAME_BUS_IEEE488_HP9895_H
