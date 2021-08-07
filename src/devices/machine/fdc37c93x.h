// license:BSD-3-Clause
// copyright-holders: Samuele Zannoli
/***************************************************************************

fdc37c93x.h

SMSC FDC37C93x Plug and Play Compatible Ultra I/O Controller

***************************************************************************/

#ifndef MAME_MACHINE_FDC37C93X_H
#define MAME_MACHINE_FDC37C93X_H

#pragma once

#include "machine/8042kbdc.h"
// floppy disk controller
#include "machine/upd765.h"
#include "imagedev/floppy.h"
#include "formats/pc_dsk.h"
#include "formats/naslite_dsk.h"
// parallel port
#include "machine/pc_lpt.h"
// serial port
#include "machine/ins8250.h"

// make sure that pckeybrd.cpp 8042kbdc.cpp are present in project

class fdc37c93x_device : public device_t, public device_isa16_card_interface
{
public:
	fdc37c93x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~fdc37c93x_device() {}

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	void set_sysopt_pin(int value) { sysopt_pin = value; }
	auto gp20_reset() { return m_gp20_reset_callback.bind(); }
	auto gp25_gatea20() { return m_gp25_gatea20_callback.bind(); }
	auto irq1() { return m_irq1_callback.bind(); }
	auto irq8() { return m_irq8_callback.bind(); }
	auto irq9() { return m_irq9_callback.bind(); }
	auto txd1() { return m_txd1_callback.bind(); }
	auto ndtr1() { return m_ndtr1_callback.bind(); }
	auto nrts1() { return m_nrts1_callback.bind(); }
	auto txd2() { return m_txd2_callback.bind(); }
	auto ndtr2() { return m_ndtr2_callback.bind(); }
	auto nrts2() { return m_nrts2_callback.bind(); }

	void remap(int space_id, offs_t start, offs_t end) override;

	// for the internal floppy controller
	DECLARE_WRITE_LINE_MEMBER(irq_floppy_w);
	DECLARE_WRITE_LINE_MEMBER(drq_floppy_w);
	// for the internal parallel port
	DECLARE_WRITE_LINE_MEMBER(irq_parallel_w);
	// for the internal uarts
	DECLARE_WRITE_LINE_MEMBER(irq_serial1_w);
	DECLARE_WRITE_LINE_MEMBER(txd_serial1_w);
	DECLARE_WRITE_LINE_MEMBER(dtr_serial1_w);
	DECLARE_WRITE_LINE_MEMBER(rts_serial1_w);
	DECLARE_WRITE_LINE_MEMBER(irq_serial2_w);
	DECLARE_WRITE_LINE_MEMBER(txd_serial2_w);
	DECLARE_WRITE_LINE_MEMBER(dtr_serial2_w);
	DECLARE_WRITE_LINE_MEMBER(rts_serial2_w);
	// chip pins for uarts
	DECLARE_WRITE_LINE_MEMBER(rxd1_w);
	DECLARE_WRITE_LINE_MEMBER(ndcd1_w);
	DECLARE_WRITE_LINE_MEMBER(ndsr1_w);
	DECLARE_WRITE_LINE_MEMBER(nri1_w);
	DECLARE_WRITE_LINE_MEMBER(ncts1_w);
	DECLARE_WRITE_LINE_MEMBER(rxd2_w);
	DECLARE_WRITE_LINE_MEMBER(ndcd2_w);
	DECLARE_WRITE_LINE_MEMBER(ndsr2_w);
	DECLARE_WRITE_LINE_MEMBER(nri2_w);
	DECLARE_WRITE_LINE_MEMBER(ncts2_w);
	// rtc
	DECLARE_WRITE_LINE_MEMBER(irq_rtc_w);
	// keyboard
	DECLARE_WRITE_LINE_MEMBER(irq_keyboard_w);
	DECLARE_WRITE_LINE_MEMBER(irq_mouse_w);
	DECLARE_WRITE_LINE_MEMBER(kbdp21_gp25_gatea20_w);
	DECLARE_WRITE_LINE_MEMBER(kbdp20_gp20_reset_w);

	void unmap_fdc(address_map &map);
	void map_lpt(address_map &map);
	void map_serial1(address_map &map);
	void map_serial2(address_map &map);
	void map_rtc(address_map &map);
	void map_keyboard(address_map &map);
	void unmap_keyboard(address_map &map);

	// to access io ports
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t disabled_read();
	void disabled_write(uint8_t data);
	uint8_t at_keybc_r(offs_t offset);
	void at_keybc_w(offs_t offset, uint8_t data);
	uint8_t keybc_status_r();
	void keybc_command_w(uint8_t data);

	static void floppy_formats(format_registration &fr);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t dack_r(int line) override;
	virtual void dack_w(int line, uint8_t data) override;
	virtual void eop_w(int state) override;

private:
	// put your private members here
	enum OperatingMode
	{
		Run = 0,
		Configuration = 1
	} mode;
	enum LogicalDevice
	{
		FDC = 0,
		IDE1,
		IDE2,
		Parallel,
		Serial1,
		Serial2,
		RTC,
		Keyboard,
		AuxIO
	};
	int config_key_step;
	int config_index;
	int logical_device;
	int last_dma_line;
	uint8_t global_configuration_registers[0x30];
	uint8_t configuration_registers[9][0x100];
	devcb_write_line m_gp20_reset_callback;
	devcb_write_line m_gp25_gatea20_callback;
	devcb_write_line m_irq1_callback;
	devcb_write_line m_irq8_callback;
	devcb_write_line m_irq9_callback;
	devcb_write_line m_txd1_callback;
	devcb_write_line m_ndtr1_callback;
	devcb_write_line m_nrts1_callback;
	devcb_write_line m_txd2_callback;
	devcb_write_line m_ndtr2_callback;
	devcb_write_line m_nrts2_callback;
	required_device<smc37c78_device> floppy_controller_fdcdev;
	required_device<pc_lpt_device> pc_lpt_lptdev;
	required_device<ns16550_device> pc_serial1_comdev;
	required_device<ns16550_device> pc_serial2_comdev;
	required_device<ds12885_device> ds12885_rtcdev;
	required_device<kbdc8042_device> m_kbdc;
	int sysopt_pin;
	bool enabled_logical[9];
	int dreq_mapping[4];
	void request_irq(int irq, int state);
	void request_dma(int dreq, int state);
	uint16_t get_base_address(int logical, int index);
	void update_dreq_mapping(int dreq, int logical);
	void map_fdc_addresses();
	void unmap_fdc_addresses();
	void map_lpt_addresses();
	void unmap_lpt_addresses();
	void map_serial1_addresses();
	void unmap_serial1_addresses();
	void map_serial2_addresses();
	void unmap_serial2_addresses();
	void map_rtc_addresses();
	void unmap_rtc_addresses();
	void map_keyboard_addresses();
	void unmap_keyboard_addresses();
	void write_global_configuration_register(int index, int data);
	void write_logical_configuration_register(int index, int data);
	void write_fdd_configuration_register(int index, int data);
	void write_ide1_configuration_register(int index, int data) {}
	void write_ide2_configuration_register(int index, int data) {}
	void write_parallel_configuration_register(int index, int data);
	void write_serial1_configuration_register(int index, int data);
	void write_serial2_configuration_register(int index, int data);
	void write_rtc_configuration_register(int index, int data);
	void write_keyboard_configuration_register(int index, int data);
	void write_auxio_configuration_register(int index, int data);
	uint16_t read_global_configuration_register(int index);
	uint16_t read_logical_configuration_register(int index);
	uint16_t read_fdd_configuration_register(int index) { return configuration_registers[logical_device][index]; }
	uint16_t read_ide1_configuration_register(int index) { return configuration_registers[logical_device][index]; }
	uint16_t read_ide2_configuration_register(int index) { return configuration_registers[logical_device][index]; }
	uint16_t read_parallel_configuration_register(int index) { return configuration_registers[logical_device][index]; }
	uint16_t read_serial1_configuration_register(int index) { return configuration_registers[logical_device][index]; }
	uint16_t read_serial2_configuration_register(int index) { return configuration_registers[logical_device][index]; }
	uint16_t read_rtc_configuration_register(int index);
	uint16_t read_keyboard_configuration_register(int index);
	uint16_t read_auxio_configuration_register(int index);
};

DECLARE_DEVICE_TYPE(FDC37C93X, fdc37c93x_device);

#endif // MAME_MACHINE_FDC37C93X_H
