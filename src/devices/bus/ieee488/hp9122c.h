// license:BSD-3-Clause
// copyright-holders: Sven Schnelle
/*********************************************************************

    hp9122c.h

    HP9122C floppy/hard disk drive

*********************************************************************/

#ifndef MAME_BUS_IEEE488_HP9122C_H
#define MAME_BUS_IEEE488_HP9122C_H

#pragma once

#include "ieee488.h"
#include "cpu/m6809/m6809.h"
#include "machine/i8291a.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"
#include "machine/fdc_pll.h"

class hp9122c_device : public device_t,
					  public device_ieee488_interface
{
public:
	// construction/destruction
	hp9122c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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


	/* Control register bits:
	 * 0 - Head select
	 * 1 - unused
	 * 2 - FDC Clock selector - 0 - 1Mhz, 1 - 2Mhz
	 * 3 - IntSel1
	 * 4 - DS1#
	 * 5 - DS0#
	 * 6 - LED
	 * 7 - IntSel0
	 *
	 * Intsel:
	 *  * - FIRQ       IRQ
	 *  0 - FDCIRQ     FDCOrg
	 *  1 - HPIBINT    HPIBDReq
	 *  2 - GND        IndexInt
	 *  3 - GND        GND
	 */

	constexpr static int REG_CNTL_HEADSEL = 1 << 0;
	constexpr static int REG_CNTL_CLOCK_SEL = 1 << 2;
	constexpr static int REG_CNTL_INTSEL1 = 1 << 3;
	constexpr static int REG_CNTL_DS1 = 1 << 4;
	constexpr static int REG_CNTL_DS0 = 1 << 5;
	constexpr static int REG_CNTL_LED =  1 << 6;
	constexpr static int REG_CNTL_INTSEL0 = 1 << 7;

	/* Status register bits:
	 *  0 - Addr0
	 *  1 - Addr1
	 *  2 - Addr2
	 *  3 - Dual(H)/Single(L)
	 *  4 - T(L)
	 *  5 - Diskchg#
	 *  6 - Addr3
	 *  7 - density indicatior: H - DD, L - HD
	 */

	constexpr static int REG_STATUS_DUAL = 1 << 3;
	constexpr static int REG_STATUS_TEST = 1 << 4;
	constexpr static int REG_STATUS_DISKCHG = 1 << 5;
	constexpr static int REG_STATUS_LOW_DENSITY = 1 << 7;

	void i8291a_eoi_w(int state);
	void i8291a_dav_w(int state);
	void i8291a_nrfd_w(int state);
	void i8291a_ndac_w(int state);
	void i8291a_ifc_w(int state);
	void i8291a_srq_w(int state);
	void i8291a_atn_w(int state);
	void i8291a_ren_w(int state);

	uint8_t i8291a_dio_r();
	void i8291a_dio_w(uint8_t data);

	void i8291a_int_w(int state);
	void i8291a_dreq_w(int state);

	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);

	void cmd_w(uint8_t data);
	uint8_t status_r();
	void clridx_w(uint8_t data);

	void cpu_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_cpu;
	required_device<i8291a_device> m_i8291a;
	required_device<mb8876_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;

	required_ioport m_hpib_addr;
	required_ioport m_testmode;

	output_finder<3> m_leds;

	void index_pulse_cb(floppy_image_device *floppy, int state);
	int m_intsel;
	void update_intsel(void);

	bool m_fdc_irq;
	bool m_i8291a_irq;
	bool m_fdc_drq;
	bool m_i8291a_drq;

	bool m_cpuirq;
	bool m_cpufirq;

	bool m_index_int;

	bool m_ds0;
	bool m_ds1;

	TIMER_CALLBACK_MEMBER(motor_timeout);
	emu_timer *m_motor_timer;
};

// device type definition
DECLARE_DEVICE_TYPE(HP9122C, hp9122c_device)

#endif // MAME_BUS_IEEE488_HP9122C_H
