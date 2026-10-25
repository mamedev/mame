// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Acorn FileStore E01/E01S network hard disk emulation

**********************************************************************/

#ifndef MAME_BUS_ECONET_E01_H
#define MAME_BUS_ECONET_E01_H

#pragma once

#include "econet.h"
#include "bus/centronics/ctronics.h"
#include "cpu/m6502/r65c02.h"
#include "imagedev/floppy.h"
#include "machine/6522via.h"
#include "machine/74123.h"
#include "machine/output_latch.h"
#include "machine/mc146818.h"
#include "machine/mc6854.h"
#include "machine/nscsi_cb.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"

class econet_e01_device : public device_t, public device_econet_interface
{
public:
	// construction/destruction
	econet_e01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	econet_e01_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_econet_interface overrides
	virtual void econet_data(int state) override;
	virtual void econet_clk(int state) override;
	virtual void econet_valid(int state) override;

private:
	void rtc_irq_w(int state);
	void adlc_irq_w(int state);
	void econet_data_w(int state);
	void adlc_rts_w(int state);
	void clk_det_w(int state);
	void via_irq_w(int state);
	void clk_en_w(int state);
	void fdc_irq_w(int state);
	void fdc_drq_w(int state);
	void fdc_hld_w(int state);
	void scsi_bsy_w(int state);
	void scsi_req_w(int state);
	void scsi_io_w(int state);

	TIMER_CALLBACK_MEMBER(clk_tick);
	void update_econet_clk();

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t ram_select_r();
	void floppy_w(uint8_t data);
	void update_floppy();
	uint8_t rtc_data_r();
	void rtc_data_w(uint8_t data);
	uint8_t network_irq_disable_r();
	void network_irq_disable_w(uint8_t data);
	uint8_t network_irq_enable_r();
	void network_irq_enable_w(uint8_t data);
	uint8_t hdc_data_r();
	void hdc_data_w(uint8_t data);
	uint8_t hdc_status_r();
	void hdc_select_w(uint8_t data);
	void hdc_irq_enable_w(uint8_t data);

	static void floppy_formats_afs(format_registration &fr);

	required_device<r65c102_device> m_maincpu;
	required_device<wd2793_device> m_fdc;
	required_device<mc6854_device> m_adlc;
	required_device<ttl74123_device> m_clk_det;
	required_device<mc146818_device> m_rtc;
	required_device<ram_device> m_ram;
	required_device<nscsi_callback_device> m_scsi;
	required_device_array<floppy_connector, 2> m_floppy;
	required_memory_region m_rom;
	required_device<centronics_device> m_centronics;
	required_ioport m_flap;
	required_ioport m_links;

	output_finder<> m_led;

	inline void update_interrupts();
	inline void network_irq_enable(int enabled);
	inline void hdc_irq_enable(int enabled);

	void e01_mem(address_map &map) ATTR_COLD;

	// interrupt state
	bool m_adlc_ie;
	bool m_hdc_ie;
	bool m_rtc_irq;
	bool m_via_irq;
	bool m_hdc_irq;
	bool m_fdc_irq;
	uint8_t m_scsi_data;
	bool m_fdc_drq;
	bool m_adlc_irq;
	bool m_clk_present;
	bool m_data_valid;
	bool m_fdc_hld;
	uint8_t m_floppy_sel;
	bool m_floppy_side;
	bool m_clk_en;
	bool m_clk_tc;
	bool m_rtc_ce;
	bool m_ram_en;

	// timers
	emu_timer *m_clk_timer;
};


// ======================> econet_e01s_device

class econet_e01s_device :  public econet_e01_device
{
public:
	// construction/destruction
	econet_e01s_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(ECONET_E01,  econet_e01_device)
DECLARE_DEVICE_TYPE(ECONET_E01S, econet_e01s_device)



#endif // MAME_BUS_ECONET_E01_H
