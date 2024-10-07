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
#include "bus/scsi/scsi.h"
#include "cpu/m6502/m65c02.h"
#include "imagedev/floppy.h"
#include "machine/6522via.h"
#include "machine/buffer.h"
#include "machine/output_latch.h"
#include "machine/mc146818.h"
#include "machine/mc6854.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"

class econet_e01_device : public device_t,
	public device_econet_interface
{
public:
	// construction/destruction
	econet_e01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	enum
	{
		TYPE_E01 = 0,
		TYPE_E01S
	};

	econet_e01_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int variant);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_econet_interface overrides
	virtual void econet_data(int state) override;
	virtual void econet_clk(int state) override;

private:
	void rtc_irq_w(int state);
	void adlc_irq_w(int state);
	void econet_data_w(int state);
	void via_irq_w(int state);
	void clk_en_w(int state);
	void fdc_irq_w(int state);
	void fdc_drq_w(int state);
	void scsi_bsy_w(int state);
	void scsi_req_w(int state);

	TIMER_CALLBACK_MEMBER(clk_tick);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t ram_select_r();
	void floppy_w(uint8_t data);
	uint8_t network_irq_disable_r();
	void network_irq_disable_w(uint8_t data);
	uint8_t network_irq_enable_r();
	void network_irq_enable_w(uint8_t data);
	uint8_t hdc_data_r();
	void hdc_data_w(uint8_t data);
	void hdc_select_w(uint8_t data);
	void hdc_irq_enable_w(uint8_t data);

	static void floppy_formats_afs(format_registration &fr);

	required_device<m65c02_device> m_maincpu;
	required_device<wd2793_device> m_fdc;
	required_device<mc6854_device> m_adlc;
	required_device<mc146818_device> m_rtc;
	required_device<ram_device> m_ram;
	required_device<scsi_port_device> m_scsibus;
	required_device<output_latch_device> m_scsi_data_out;
	required_device<input_buffer_device> m_scsi_data_in;
	required_device<input_buffer_device> m_scsi_ctrl_in;
	required_device_array<floppy_connector, 2> m_floppy;
	required_memory_region m_rom;
	required_device<centronics_device> m_centronics;

	output_finder<> m_led;

	inline void update_interrupts();
	inline void network_irq_enable(int enabled);
	inline void hdc_irq_enable(int enabled);

	void e01_mem(address_map &map) ATTR_COLD;

	// interrupt state
	int m_adlc_ie;
	int m_hdc_ie;
	int m_rtc_irq;
	int m_via_irq;
	int m_hdc_irq;
	int m_fdc_irq;
	bool m_fdc_drq;
	int m_adlc_irq;
	int m_clk_en;
	bool m_ram_en;

	const int m_variant;

	// timers
	emu_timer *m_clk_timer;
};


// ======================> econet_e01s_device

class econet_e01s_device :  public econet_e01_device
{
public:
	// construction/destruction
	econet_e01s_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(ECONET_E01,  econet_e01_device)
DECLARE_DEVICE_TYPE(ECONET_E01S, econet_e01s_device)



#endif // MAME_BUS_ECONET_E01_H
