// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 128 MMU socket emulation

**********************************************************************/

#ifndef MAME_BUS_C128_MMU_H
#define MAME_BUS_C128_MMU_H

#pragma once

#include "machine/mos8722.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class c128_mmu_slot_device;


// ======================> device_c128_mmu_interface

class device_c128_mmu_interface : public device_interface
{
public:
	virtual uint8_t read(offs_t offset, uint8_t data) = 0;
	virtual void write(offs_t offset, uint8_t data) = 0;
	virtual offs_t ta_r(offs_t offset, int aec, int *ms0, int *ms1, int *ms2, int *ms3, int *cas0, int *cas1) = 0;
	virtual int fsdir_r() = 0;

	// the MMU's AEC input, which the C128 gates with /DMA
	virtual int aec_r(int state) { return state; }
	virtual void dma_w(int state) { }

protected:
	device_c128_mmu_interface(const machine_config &mconfig, device_t &device);

	c128_mmu_slot_device *const m_slot;
};


// ======================> c128_mmu_slot_device

class c128_mmu_slot_device : public device_t, public device_single_card_slot_interface<device_c128_mmu_interface>
{
public:
	template <typename T>
	c128_mmu_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&opts, const char *dflt)
		: c128_mmu_slot_device(mconfig, tag, owner, clock)
	{
		set_options(std::forward<T>(opts), dflt, false);
	}

	c128_mmu_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto z80en() { return m_write_z80en.bind(); }
	auto fsdir() { return m_write_fsdir.bind(); }
	auto game() { return m_read_game.bind(); }
	auto exrom() { return m_read_exrom.bind(); }
	auto sense40() { return m_read_sense40.bind(); }

	// clips on the 8502 processor port (P0-P2 and P6), and the port override
	auto port() { return m_write_port.bind(); }
	auto drive() { return m_write_drive.bind(); }
	auto caps() { return m_read_caps.bind(); }

	uint8_t read(offs_t offset, uint8_t data) { return m_card->read(offset, data); }
	void write(offs_t offset, uint8_t data) { m_card->write(offset, data); }
	offs_t ta_r(offs_t offset, int aec, int *ms0, int *ms1, int *ms2, int *ms3, int *cas0, int *cas1) { return m_card->ta_r(offset, aec, ms0, ms1, ms2, ms3, cas0, cas1); }
	int fsdir_r() { return m_card->fsdir_r(); }
	int aec_r(int state) { return m_card->aec_r(state); }
	void dma_w(int state) { m_card->dma_w(state); }

	// card interface
	void z80en_w(int state) { m_write_z80en(state); }
	void fsdir_w(int state) { m_write_fsdir(state); }
	int game_r() { return m_read_game(); }
	int exrom_r() { return m_read_exrom(); }
	int sense40_r() { return m_read_sense40(); }
	void port_w(uint8_t data) { m_write_port(data); }
	void drive_w(int state) { m_write_drive(state); }
	int caps_r() { return m_read_caps(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line m_write_z80en;
	devcb_write_line m_write_fsdir;
	devcb_read_line m_read_game;
	devcb_read_line m_read_exrom;
	devcb_read_line m_read_sense40;
	devcb_write8 m_write_port;
	devcb_write_line m_write_drive;
	devcb_read_line m_read_caps;

	device_c128_mmu_interface *m_card;
};


// ======================> c128_mmu_mos8722_device

class c128_mmu_mos8722_device : public device_t, public device_c128_mmu_interface
{
public:
	c128_mmu_mos8722_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_c128_mmu_interface implementation
	virtual uint8_t read(offs_t offset, uint8_t data) override;
	virtual void write(offs_t offset, uint8_t data) override;
	virtual offs_t ta_r(offs_t offset, int aec, int *ms0, int *ms1, int *ms2, int *ms3, int *cas0, int *cas1) override;
	virtual int fsdir_r() override;

protected:
	c128_mmu_mos8722_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD { }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<mos8722_device> m_mmu;
};


DECLARE_DEVICE_TYPE(C128_MMU_SLOT, c128_mmu_slot_device)
DECLARE_DEVICE_TYPE(C128_MMU_MOS8722, c128_mmu_mos8722_device)

void c128_mmu_cards(device_slot_interface &device) ATTR_COLD;

#endif // MAME_BUS_C128_MMU_H
