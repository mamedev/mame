// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMD RAMLink emulation

**********************************************************************/

#ifndef MAME_BUS_C64_RAMLINK_H
#define MAME_BUS_C64_RAMLINK_H

#pragma once

#include "exp.h"

#include "bus/cbmiec/cmdpar.h"
#include "machine/i8255.h"
#include "machine/msm6242.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_ramlink_cartridge_device

class c64_ramlink_cartridge_device : public device_t,
									 public device_c64_expansion_card_interface,
									 public device_cmd_parallel_interface,
									 public device_nvram_interface
{
public:
	// construction/destruction
	c64_ramlink_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER( switch_changed );
	DECLARE_INPUT_CHANGED_MEMBER( reset_changed );
	DECLARE_INPUT_CHANGED_MEMBER( swap_changed );

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_nvram_interface implementation
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	// device_c64_expansion_card_interface implementation
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual int c64_game_r(offs_t offset, int sphi2, int ba, int rw) override;
	virtual int c64_exrom_r(offs_t offset, int sphi2, int ba, int rw) override;
	virtual void c64_ba_w(int state) override;
	virtual void c64_phi2_w(int state) override;
	virtual void c64_irq_w(int state) override;
	virtual void c64_nmi_w(int state) override;

private:
	required_device<i8255_device> m_ppi;
	required_device<rtc72421_device> m_rtc;
	required_device<c64_expansion_slot_device> m_ramport;
	required_device<c64_expansion_slot_device> m_exp;
	required_region_ptr<uint8_t> m_rom;
	required_ioport m_config;
	required_ioport m_switches;
	output_finder<5> m_leds;

	memory_share_creator<uint8_t> m_ram;
	memory_share_creator<uint8_t> m_card;

	bool m_enabled;
	bool m_direct;
	uint8_t m_ramcard;

	bool m_on;
	bool m_dos;
	bool m_trap;
	uint8_t m_io1_mode;
	uint16_t m_ram_page;
	uint32_t m_card_addr;
	uint8_t m_rom_bank;
	uint8_t m_ppi_pb;
	bool m_mapped;
	uint8_t m_swap;

	bool m_c128;
	bool m_boot;
	uint8_t m_mmu_cr;
	uint8_t m_mmu_pcr[4];

	uint8_t m_irq[2];
	uint8_t m_nmi[2];
	uint8_t m_dma[2];
	uint8_t m_reset[3];

	template <unsigned N> void irq_w(int state);
	template <unsigned N> void nmi_w(int state);
	template <unsigned N> void dma_w(int state);
	template <unsigned N> void reset_w(int state);
	uint8_t dma_cd_r(offs_t offset);
	void dma_cd_w(offs_t offset, uint8_t data);

	uint8_t ppi_pc_r();
	void ppi_pb_w(uint8_t data);

	bool active() const { return m_enabled && m_on; }
	bool c128_mode() const { return m_enabled && m_c128; }
	bool c128_kernal(offs_t offset) const;
	bool kernal_mapped(offs_t offset, int sphi2, int ba, int rw, int game, int exrom) const;
	void update_leds();
	void mmu_snoop(offs_t offset, uint8_t data);
	bool card_installed() const;
	offs_t card_offset(offs_t offset) const;
	bool io1_internal() const;
	bool io2_decoded(offs_t offset, int rw) const;
	bool cable_access(offs_t offset, int io2) const;
	void ramport_strobes(offs_t offset, int rw, int io1, int io2, int &rp_io1, int &rp_io2) const;
	void exp_strobes(offs_t offset, int io1, int io2, int &exp_io1, int &exp_io2) const;
	void switch_off();
};


// device type definition
DECLARE_DEVICE_TYPE(C64_RAMLINK, c64_ramlink_cartridge_device)


#endif // MAME_BUS_C64_RAMLINK_H
