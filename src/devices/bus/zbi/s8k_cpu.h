// license:BSD-3-Clause
// copyright-holders:A. Lenard
/***************************************************************************

    System 8000 CPU-A 1.0 board
    System 8000 CPU-A board
    System 8000 HPCPU board

***************************************************************************/

#ifndef MAME_BUS_ZBI_S8K_CPU_H
#define MAME_BUS_ZBI_S8K_CPU_H

#pragma once

#include "zbi.h"

#include "cpu/z8000/z8000.h"
#include "machine/z8010.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/z80pio.h"
#include "machine/z80scc.h"
#include "machine/z8536.h"

#include "ioport.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class s8k_cpu_base : public device_t, public device_zbi_card_interface
{
public:
	void nmi_switch_w(int state);

	auto ns_cb() { return m_ns_cb.bind(); }
	auto busack_cb() { return m_busack_cb.bind(); }

protected:
	// construction/destruction
	s8k_cpu_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	void base_device_start();
	void base_device_reset();
	void base_device_resolve_objects();

	void addrmap_program(address_map &map) ATTR_COLD { map.unmap_value_low(); }
	void addrmap_data(address_map &map) ATTR_COLD { map.unmap_value_low(); }
	void addrmap_stack(address_map &map) ATTR_COLD { map.unmap_value_low(); }
	virtual void addrmap_io(address_map &map) ATTR_COLD { map.unmap_value_high(); }
	virtual void addrmap_sio(address_map &map) ATTR_COLD { map.unmap_value_high(); }

	z8010_device *select_code_mmu(offs_t offset);
	z8010_device *select_data_mmu(offs_t offset, uint8_t sbr, uint8_t nbr);

	virtual bool translate_addr(int spacenum, bool write, offs_t &offset) = 0;
	uint16_t ram_r(address_space &space, offs_t offset, uint16_t mask);
	void ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mask);

	virtual void card_nvi_w(int state) override
		{ m_maincpu->set_input_line(z8002_device::NVI_LINE, state); }
	virtual void card_vi_w(int state) override
		{ m_maincpu->set_input_line(z8002_device::VI_LINE, state); }
	virtual void card_nmi_w(int state) override
		{ m_maincpu->set_input_line(z8002_device::NMI_LINE, state); }
	virtual void card_busreq_w(int state) override
		{ m_maincpu->set_input_line(z8002_device::BUSREQ_LINE, state); }

	uint16_t nmiack_r();
	uint16_t segtack_r();

	void segt_interrupt(int state);

	void soft_reset_w(uint8_t dummy);

	void install_memory(offs_t lrom_end, offs_t lram_start, offs_t lmem_end);
	virtual void install_memory() = 0;

	uint8_t reg_snvr_r();
	uint8_t reg_trpl_r();
	uint8_t reg_if1l_r();

	//helpers
	void out_ns_cb(int state);
	void out_busack_cb(int state);

	// object finders
	required_device<z8001_device> m_maincpu;
	memory_share_creator<uint16_t> m_local_ram;
	memory_view m_view_code, m_view_data, m_view_stck;
	required_device<z8010_device> m_mmu_code, m_mmu_data, m_mmu_stck;
	required_ioport m_dipsw;

	devcb_write_line m_ns_cb;
	devcb_write_line m_busack_cb;

	// Board registers
	uint8_t m_reg_snvr	= 0; // Segment Violation Register
	uint8_t m_reg_trpl	= 0; // Segment trap memory address low-byte
	uint8_t m_reg_if1l	= 0; // Segment trap instruction low-byte

	bool m_is_seg_os = false;
	bool m_is_seg_user = false;

	uint16_t m_nmi_code = 0;
	int m_normal_mode = 0;
	int m_dma_on = 0;
};


// ======================> zbi_s8k_cpu10_card_device

class zbi_s8k_cpu10_card_device : public s8k_cpu_base
{
public:
	// construction/destruction
	zbi_s8k_cpu10_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	zbi_s8k_cpu10_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void install_memory() override;

	required_device_array<z80sio_device, 4> m_sio;
	required_device_array<z80ctc_device, 3> m_ctc;
	required_device<z80pio_device> m_pio;

	required_ioport m_segjp;

private:
	virtual void addrmap_sio(address_map &map) override ATTR_COLD;

	virtual bool translate_addr(int spacenum, bool write, offs_t &offset) override;

	uint16_t reg_scr_r(offs_t offset, uint16_t mask);
	void reg_scr_w(uint16_t data);
	uint8_t reg_sbr_r();
	void reg_sbr_w(uint8_t data);
	uint8_t reg_nbr_r();
	void reg_nbr_w(uint8_t data);

	uint8_t comms_r(offs_t offset);
	void comms_w(offs_t offset, uint8_t data);

	uint8_t mmu_cmd_r(offs_t offset);
	void mmu_cmd_w(offs_t offset, uint8_t data);

	uint8_t pa_data_r();

	void centronics_busy_w(uint8_t data);
	void centronics_select_w(uint8_t data);
	void centronics_fault_w(uint8_t data);
	void centronics_ack_w(uint8_t data);

	// Board registers
	uint8_t m_reg_scr	= 0; // System Configuration Register
	uint8_t m_reg_sbr	= 0; // System Break Register
	uint8_t m_reg_nbr	= 0; // Normal Break Register

	int m_centronics_busy = 0;
	int m_centronics_select = 0;
	int m_centronics_fault = 0;
	int m_centronics_ack = 0;
};

// ======================> zbi_s8k_cpu_card_device

class zbi_s8k_cpu_card_device : public zbi_s8k_cpu10_card_device
{
public:
	// construction/destruction
	zbi_s8k_cpu_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t implementation
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void install_memory() override;
};

// ======================> zbi_s8k_hpcpu_card_device
/*
 The differences in the hardware (from a programming point of view)
 between the CPU-A and HPCPU boards are listed below.

 - HPCPU has no system break register for the MMUs.
 - HPCPU has no on-board printer port.
 - HPCPU uses an 8530 SCC for the console port and tty0
   (instead of an SIO). Tty2-ttyn are mapped off-board.
 - HPCPU uses an 8536 CIO for the counter/timer functions
   instead of Z-80 CTCs.
 - The System Configuration Register on the HPCPU is a full
   16 bits wide. The SCR on CPU-A is only 8 bits wide, but
   may be read or written as a word register (ignoring the
   high byte). All bit definitions in the low byte of the HPCPU
   SCR correspond directly to the definitions for the CPU-A bits.
   There are additional bits in the high byte of the HPCPU SCR
   such as another boot device bit and a cache memory enable/disable
   bit.
 - The 'serial number' that is passed in R7 from the CPU PROMs
   has always been zero for CPU-A, but will have the low bit (bit #0)
   set for the HPCPU and the next bit (bit #1) set if cache memory
   is present and passes the power-up diagnostics.
 - Single-step on the HPCPU is done with a hardware register and
   NMI instead of a CTC as on CPU-A.
*/

class zbi_s8k_hpcpu_card_device : public s8k_cpu_base
{
public:
	// construction/destruction
	zbi_s8k_hpcpu_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void install_memory() override;

	memory_share_creator<uint16_t> m_cache;
	required_device<scc8530_device> m_scc;
	required_device<z8536_device> m_cio;

private:
	virtual void addrmap_sio(address_map &map) override ATTR_COLD;

	virtual bool translate_addr(int spacenum, bool write, offs_t &offset) override;

	uint16_t reg_scr_r();
	void reg_scr_w(uint16_t data);
	uint8_t reg_ubr_r();
	void reg_ubr_w(uint8_t data);

	uint8_t spec_io_r(offs_t offset);
	void spec_io_w(offs_t offset, uint8_t data);

	// CIO port offsets are in reverse order
	uint8_t cio_r(offs_t offset) { return m_cio->read(~(offset >> 1)); }
	void cio_w(offs_t offset, uint8_t data) { m_cio->write(~(offset >> 1), data); }

	// Board registers
	uint16_t m_reg_scr	= 0; // System Configuration Register
	uint8_t m_reg_ubr	= 0; // User Break Register
};

// device type definition
DECLARE_DEVICE_TYPE(ZBI_S8K_CPU10,	zbi_s8k_cpu10_card_device)
DECLARE_DEVICE_TYPE(ZBI_S8K_CPU,	zbi_s8k_cpu_card_device)
DECLARE_DEVICE_TYPE(ZBI_S8K_HPCPU,	zbi_s8k_hpcpu_card_device)

#endif // MAME_BUS_ZBI_S8K_CPU_H
