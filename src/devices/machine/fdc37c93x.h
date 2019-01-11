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
#include "formats/pc_dsk.h"
#include "formats/naslite_dsk.h"
// parallel port
#include "machine/pc_lpt.h"

// make sure that pckeybrd.cpp 8042kbdc.cpp are present in project

class fdc37c93x_device : public device_t, public device_isa16_card_interface
{
public:
	fdc37c93x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~fdc37c93x_device() {}
	static void static_set_sysopt_pin(device_t &device, int value) { dynamic_cast<fdc37c93x_device &>(device).sysopt_pin = value; }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	template <class Object> devcb_base &set_gp20_reset_callback(Object &&cb) { return m_gp20_reset_callback.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_gp25_gatea20_callback(Object &&cb) { return m_gp25_gatea20_callback.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_irq1_callback(Object &&cb) { return m_irq1_callback.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_irq8_callback(Object &&cb) { return m_irq8_callback.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_irq9_callback(Object &&cb) { return m_irq9_callback.set_callback(std::forward<Object>(cb)); }

	void remap(int space_id, offs_t start, offs_t end) override;

	// to access io ports
	DECLARE_READ8_MEMBER(read_fdc37c93x);
	DECLARE_WRITE8_MEMBER(write_fdc37c93x);
	// for the internal floppy controller
	DECLARE_WRITE_LINE_MEMBER(irq_floppy_w);
	DECLARE_WRITE_LINE_MEMBER(drq_floppy_w);
	// for the internal parallel port
	DECLARE_WRITE_LINE_MEMBER(irq_parallel_w);
	// rtc
	DECLARE_WRITE_LINE_MEMBER(irq_rtc_w);
	// keyboard
	DECLARE_WRITE_LINE_MEMBER(irq_keyboard_w);
	DECLARE_WRITE_LINE_MEMBER(kbdp21_gp25_gatea20_w);
	DECLARE_WRITE_LINE_MEMBER(kbdp20_gp20_reset_w);

	void unmap_fdc(address_map &map);
	void map_lpt(address_map &map);
	void unmap_lpt(address_map &map);
	void map_rtc(address_map &map);
	void unmap_rtc(address_map &map);
	void map_keyboard(address_map &map);
	void unmap_keyboard(address_map &map);

	DECLARE_READ8_MEMBER(disabled_read);
	DECLARE_WRITE8_MEMBER(disabled_write);
	DECLARE_READ8_MEMBER(lpt_read);
	DECLARE_WRITE8_MEMBER(lpt_write);
	DECLARE_READ8_MEMBER(rtc_read);
	DECLARE_WRITE8_MEMBER(rtc_write);
	DECLARE_READ8_MEMBER(at_keybc_r);
	DECLARE_WRITE8_MEMBER(at_keybc_w);
	DECLARE_READ8_MEMBER(keybc_status_r);
	DECLARE_WRITE8_MEMBER(keybc_command_w);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

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
	required_device<pc_fdc_interface> floppy_controller_fdcdev;
	required_device<pc_lpt_device> pc_lpt_lptdev;
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
	void write_serial1_configuration_register(int index, int data) {}
	void write_serial2_configuration_register(int index, int data) {}
	void write_rtc_configuration_register(int index, int data);
	void write_keyboard_configuration_register(int index, int data);
	void write_auxio_configuration_register(int index, int data);
	uint16_t read_global_configuration_register(int index);
	uint16_t read_logical_configuration_register(int index);
	uint16_t read_fdd_configuration_register(int index) { return 0; }
	uint16_t read_ide1_configuration_register(int index) { return 0; }
	uint16_t read_ide2_configuration_register(int index) { return 0; }
	uint16_t read_parallel_configuration_register(int index) { return 0; }
	uint16_t read_serial1_configuration_register(int index) { return 0; }
	uint16_t read_serial2_configuration_register(int index) { return 0; }
	uint16_t read_rtc_configuration_register(int index);
	uint16_t read_keyboard_configuration_register(int index);
	uint16_t read_auxio_configuration_register(int index);
};

DECLARE_DEVICE_TYPE(FDC37C93X, fdc37c93x_device);

#define MCFG_FDC37C93X_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, FDC37C93X, 0)

#define MCFG_FDC37C93X_SYSOPT(_pinvalue) \
	fdc37c93x_device::static_set_sysopt_pin(*device, _pinvalue);

#define MCFG_FDC37C93X_GP20_RESET_CB(_devcb) \
	devcb = &downcast<fdc37c93x_device &>(*device).set_gp20_reset_callback(DEVCB_##_devcb);

#define MCFG_FDC37C93X_GP25_GATEA20_CB(_devcb) \
	devcb = &downcast<fdc37c93x_device &>(*device).set_gp25_gatea20_callback(DEVCB_##_devcb);

#define MCFG_FDC37C93X_IRQ1_CB(_devcb) \
	devcb = &downcast<fdc37c93x_device &>(*device).set_irq1_callback(DEVCB_##_devcb);

#define MCFG_FDC37C93X_IRQ8_CB(_devcb) \
	devcb = &downcast<fdc37c93x_device &>(*device).set_irq8_callback(DEVCB_##_devcb);

#define MCFG_FDC37C93X_IRQ9_CB(_devcb) \
	devcb = &downcast<fdc37c93x_device &>(*device).set_irq9_callback(DEVCB_##_devcb);

#endif // MAME_MACHINE_FDC37C93X_H