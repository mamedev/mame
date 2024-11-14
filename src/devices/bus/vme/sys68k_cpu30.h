// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom

#ifndef MAME_BUS_VME_SYS68K_CPU30_H
#define MAME_BUS_VME_SYS68K_CPU30_H

#pragma once

#include "bus/vme/vme.h"
#include "cpu/m68000/m68030.h"
#include "machine/ram.h" // For variants that only differs in amount of RAM
#include "machine/scnxx562.h"
#include "machine/68230pit.h"
#include "machine/msm6242.h"
#include "machine/fga002.h"

class sys68k_cpu30_device_base
	: public device_t
	, public device_vme_card_interface
{
public:
	sys68k_cpu30_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock, uint8_t board_id);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void sys68k_cpu30(machine_config &config, XTAL clock, char const *const ram_default, char const *const ram_options);

	void fdc_w(offs_t offset, uint8_t data);
	uint8_t fdc_r(offs_t offset);
	void scsi_w(offs_t offset, uint8_t data);
	uint8_t scsi_r(offs_t offset);
	uint8_t slot1_status_r();

	/* Interrupt  support */
	void cpu_space_map(address_map &map) ATTR_COLD;
	void fga_irq_callback(int state);
	uint8_t fga_irq_state = 0;
	//  int fga_irq_vector = 0;
	int fga_irq_level = 0;

	/* Rotary switch PIT input */
	uint8_t rotary_rd();
	uint8_t flop_dmac_r();
	void flop_dmac_w(uint8_t data);
	uint8_t pit1c_r();
	void pit1c_w(uint8_t data);
	uint8_t pit2a_r();
	void pit2a_w(uint8_t data);
	uint8_t board_mem_id_rd();
	uint8_t pit2c_r();
	void pit2c_w(uint8_t data);

	/* VME bus accesses */
	//uint16_t vme_a24_r();
	//void vme_a24_w(uint16_t data);
	//uint16_t vme_a16_r();
	//void vme_a16_w(uint16_t data);

	void cpu30_mem(address_map &map) ATTR_COLD;

	required_device<m68000_musashi_device> m_maincpu;
	required_device<ram_device> m_ram;

	required_device<duscc68562_device> m_dusccterm;

	required_device<pit68230_device> m_pit1;
	required_device<pit68230_device> m_pit2;

	required_device<fga002_device> m_fga002;

	required_device<rtc72423_device> m_rtc;

	required_memory_region m_boot;
	memory_passthrough_handler m_boot_mph;

	// Helper functions
	void update_irq_to_maincpu();

	uint8_t const m_board_id;
};

class vme_sys68k_cpu30_card_device : public sys68k_cpu30_device_base
{
public:
	vme_sys68k_cpu30_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};
class vme_sys68k_cpu30x_card_device : public sys68k_cpu30_device_base
{
public:
	vme_sys68k_cpu30x_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};
class vme_sys68k_cpu30xa_card_device : public sys68k_cpu30_device_base
{
public:
	vme_sys68k_cpu30xa_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};
class vme_sys68k_cpu30za_card_device : public sys68k_cpu30_device_base
{
public:
	vme_sys68k_cpu30za_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};
class vme_sys68k_cpu30be_card_device : public sys68k_cpu30_device_base
{
public:
	vme_sys68k_cpu30be_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};
class vme_sys68k_cpu30lite_card_device : public sys68k_cpu30_device_base
{
public:
	vme_sys68k_cpu30lite_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};
class vme_sys68k_cpu33_card_device : public sys68k_cpu30_device_base
{
public:
	vme_sys68k_cpu33_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(VME_SYS68K_CPU30,        vme_sys68k_cpu30_card_device)
DECLARE_DEVICE_TYPE(VME_SYS68K_CPU30X,       vme_sys68k_cpu30x_card_device)
DECLARE_DEVICE_TYPE(VME_SYS68K_CPU30XA,      vme_sys68k_cpu30xa_card_device)
DECLARE_DEVICE_TYPE(VME_SYS68K_CPU30ZA,      vme_sys68k_cpu30za_card_device)
DECLARE_DEVICE_TYPE(VME_SYS68K_CPU30BE,      vme_sys68k_cpu30be_card_device)
DECLARE_DEVICE_TYPE(VME_SYS68K_CPU30LITE,    vme_sys68k_cpu30lite_card_device)
DECLARE_DEVICE_TYPE(VME_SYS68K_CPU33,        vme_sys68k_cpu33_card_device)

#endif // MAME_BUS_VME_SYS68K_CPU30_H
