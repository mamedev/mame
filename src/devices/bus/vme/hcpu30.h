// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef MAME_BUS_VME_HCPU30_H
#define MAME_BUS_VME_HCPU30_H

#pragma once

#include "vme.h"

#include "bus/centronics/ctronics.h"
#include "bus/nscsi/hd.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68020.h"
#include "cpu/m68000/m68030.h"
#include "imagedev/floppy.h"
#include "machine/clock.h"
#include "machine/msm6242.h"
#include "machine/scnxx562.h"
#include "machine/terminal.h"
#include "machine/upd765.h"
#include "machine/wd33c9x.h"

DECLARE_DEVICE_TYPE(VME_HCPU30, vme_hcpu30_card_device)

class vme_hcpu30_card_device : public device_t, public device_vme_card_interface
{
public:
	vme_hcpu30_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_hcpu30_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(bus_error_off);

private:
	required_device<m68000_musashi_device> m_maincpu;
	required_device<duscc68562_device> m_dusccterm;
	required_device<wd33c93a_device> m_scsi;
	required_device<upd765_family_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<rtc62421_device> m_rtc;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<input_buffer_device> m_cent_status_in;
	required_device<m68000_musashi_device> m_oscpu;
	required_shared_ptr<uint32_t> m_mailbox;
	required_shared_ptr<uint32_t> m_p_ram;
	required_region_ptr<uint32_t> m_sysrom;
	required_ioport m_dips;

	void dusirq_callback(int state);
	void scsiirq_callback(int state);
	void scsidrq_callback(int state);
	void fdcirq_callback(int state);
	void fdcdrq_callback(int state);

	// Pointer to System ROMs needed by bootvect_r and masking RAM buffer for post reset accesses
	memory_passthrough_handler m_rom_shadow_tap;
	uint16_t    m_irq_state;
	uint16_t    m_irq_mask;
	uint8_t     m_rtc_reg[16];
	bool        m_rtc_hack;
	int         m_fdcdrq_hack;
	bool        m_bus_error;
	emu_timer  *m_bus_error_timer;

	uint32_t irq_state_r(offs_t offset);
	void irq_mask_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t rtc_r(offs_t offset);
	void rtc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t dma_r(offs_t offset);
	void dma_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t trap_r(offs_t offset, uint32_t mem_mask);
	void trap_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void set_bus_error(uint32_t address, bool write, uint32_t mem_mask);
	void update_030_irq(int irq, line_state state);

	void hcpu30_mem(address_map &map) ATTR_COLD;
	void hcpu30_os_mem(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;
	void oscpu_space_map(address_map &map) ATTR_COLD;
};

#endif // MAME_BUS_VME_HCPU30_H
