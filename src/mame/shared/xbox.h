// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli
#ifndef MAME_SHARED_XBOX_H
#define MAME_SHARED_XBOX_H

#pragma once

#include "xbox_nv2a.h"
#include "xbox_pci.h"
#include "xbox_usb.h"

#include "machine/idectrl.h"
#include "machine/pic8259.h"

/*
 * PIC16LC connected to SMBus
 */

class xbox_pic16lc_device : public device_t, public smbus_interface
{
public:
	xbox_pic16lc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual int execute_command(int command, int rw, int data) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t buffer[0xff];
};

DECLARE_DEVICE_TYPE(XBOX_PIC16LC, xbox_pic16lc_device)

/*
 * CX25871 connected to SMBus
 */

class xbox_cx25871_device : public device_t, public smbus_interface
{
public:
	xbox_cx25871_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual int execute_command(int command, int rw, int data) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
};

DECLARE_DEVICE_TYPE(XBOX_CX25871, xbox_cx25871_device)

/*
 * EEPROM connected to SMBus
 */

class xbox_eeprom_device : public device_t, public smbus_interface
{
public:
	xbox_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual int execute_command(int command, int rw, int data) override;

	std::function<void(void)> hack_eeprom;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
};

DECLARE_DEVICE_TYPE(XBOX_EEPROM, xbox_eeprom_device)

/*
 * Super-io connected to lpc bus used as a rs232 debug port
 */

class xbox_superio_device : public device_t, public lpcbus_device_interface
{
public:
	xbox_superio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void map_extra(address_space *memory_space, address_space *io_space) override;
	virtual uint32_t dma_transfer(int channel, dma_operation operation, dma_size size, uint32_t data) override;
	virtual void set_host(int device_index, lpcbus_host_interface *host) override;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t read_rs232(offs_t offset);
	void write_rs232(offs_t offset, uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	void internal_io_map(address_map &map) ATTR_COLD;

	lpcbus_host_interface *lpchost = nullptr;
	int lpcindex = 0;
	address_space *memspace = nullptr;
	address_space *iospace = nullptr;
	bool configuration_mode;
	int index;
	int selected;
	uint8_t registers[16][256]{}; // 256 registers for up to 16 devices, registers 0-0x2f common to all
};

DECLARE_DEVICE_TYPE(XBOX_SUPERIO, xbox_superio_device)

/*
 * Base
 */

class xbox_base_state : public driver_device
{
public:
	xbox_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		nvidia_nv2a(nullptr),
		debug_irq_active(false),
		debug_irq_number(0),
		m_maincpu(*this, "maincpu"),
		mcpxlpc(*this, ":pci:01.0"),
		ide(*this, ":pci:09.0:ide1"),
		debugc_bios(nullptr) { }

	void xbox_base(machine_config &config);

protected:
	void debug_generate_irq(int irq, bool active);
	virtual void hack_eeprom() {}
	virtual void hack_usb() {}

	void vblank_callback(int state);
	uint32_t screen_update_callback(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override ATTR_COLD;
	void maincpu_interrupt(int state);
	IRQ_CALLBACK_MEMBER(irq_callback);

	nv2a_renderer *nvidia_nv2a;
	bool debug_irq_active;
	int debug_irq_number;
	required_device<cpu_device> m_maincpu;
	required_device<mcpx_isalpc_device> mcpxlpc;
	required_device<bus_master_ide_controller_device> ide;
	static const struct debugger_constants
	{
		uint32_t id;
		uint32_t parameter[8]; // c c c ? ? ? x x
	} debugp[];
	const debugger_constants *debugc_bios;

private:
	void dump_string_command(const std::vector<std::string_view> &params);
	void dump_process_command(const std::vector<std::string_view> &params);
	void dump_list_command(const std::vector<std::string_view> &params);
	void dump_dpc_command(const std::vector<std::string_view> &params);
	void dump_timer_command(const std::vector<std::string_view> &params);
	void curthread_command(const std::vector<std::string_view> &params);
	void threadlist_command(const std::vector<std::string_view> &params);
	void generate_irq_command(const std::vector<std::string_view> &params);
	void nv2a_combiners_command(const std::vector<std::string_view> &params);
	void nv2a_wclipping_command(const std::vector<std::string_view> &params);
	void waitvblank_command(const std::vector<std::string_view> &params);
	void grab_texture_command(const std::vector<std::string_view> &params);
	void grab_vprog_command(const std::vector<std::string_view> &params);
	void vprogdis_command(const std::vector<std::string_view> &params);
	void vdeclaration_command(const std::vector<std::string_view> &params);
	void help_command(const std::vector<std::string_view> &params);
	void xbox_debug_commands(const std::vector<std::string_view> &params);
	int find_bios_index();
	bool find_bios_hash(int bios, uint32_t &crc32);
	void find_debug_params();
};

#endif // MAME_SHARED_XBOX_H
