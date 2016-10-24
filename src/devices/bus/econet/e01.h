// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Acorn FileStore E01/E01S network hard disk emulation

**********************************************************************/

#pragma once

#ifndef __E01__
#define __E01__

#include "econet.h"
#include "bus/centronics/ctronics.h"
#include "bus/scsi/scsi.h"
#include "cpu/m6502/m65c02.h"
#include "machine/6522via.h"
#include "machine/buffer.h"
#include "machine/latch.h"
#include "machine/mc146818.h"
#include "machine/mc6854.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "formats/afs_dsk.h"

class e01_device : public device_t,
	public device_econet_interface
{
public:
	// construction/destruction
	e01_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	e01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	enum
	{
		TYPE_E01 = 0,
		TYPE_E01S
	};

	DECLARE_FLOPPY_FORMATS(floppy_formats_afs);

	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ram_select_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void floppy_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t network_irq_disable_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void network_irq_disable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t network_irq_enable_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void network_irq_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t hdc_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hdc_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hdc_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hdc_irq_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rtc_address_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void rtc_address_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rtc_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void rtc_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rtc_irq_w(int state);
	void adlc_irq_w(int state);
	void econet_data_w(int state);
	void via_irq_w(int state);
	void clk_en_w(int state);
	void fdc_irq_w(int state);
	void fdc_drq_w(int state);
	void scsi_bsy_w(int state);
	void scsi_req_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	// device_econet_interface overrides
	virtual void econet_data(int state) override;
	virtual void econet_clk(int state) override;

	required_device<m65c02_device> m_maincpu;
	required_device<wd2793_t> m_fdc;
	required_device<mc6854_device> m_adlc;
	required_device<mc146818_device> m_rtc;
	required_device<ram_device> m_ram;
	required_device<SCSI_PORT_DEVICE> m_scsibus;
	required_device<output_latch_device> m_scsi_data_out;
	required_device<input_buffer_device> m_scsi_data_in;
	required_device<input_buffer_device> m_scsi_ctrl_in;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_memory_region m_rom;
	required_device<centronics_device> m_centronics;

	inline void update_interrupts();
	inline void network_irq_enable(int enabled);
	inline void hdc_irq_enable(int enabled);

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

	int m_variant;

	// timers
	emu_timer *m_clk_timer;
};


// ======================> e01s_device

class e01s_device :  public e01_device
{
public:
	// construction/destruction
	e01s_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
extern const device_type E01;
extern const device_type E01S;



#endif
