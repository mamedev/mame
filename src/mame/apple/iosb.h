// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_IOSB_H
#define MAME_APPLE_IOSB_H

#pragma once

#include "macrtc.h"

#include "cpu/m68000/m68040.h"
#include "machine/6522via.h"
#include "machine/applefdintf.h"
#include "machine/ncr53c90.h"
#include "machine/swim2.h"
#include "sound/asc.h"
#include "speaker.h"

// ======================> iosb_base

class iosb_base :  public device_t
{
public:
	// construction/destruction
	iosb_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// interface routines
	auto write_adb_st() { return m_adb_st.bind(); } // ADB state
	auto write_cb1() { return m_cb1.bind(); }   // ADB clock
	auto write_cb2() { return m_cb2.bind(); }   // ADB data
	auto write_dfac_clock() { return m_dfac_clock_w.bind(); }
	auto write_dfac_data() { return m_dfac_data_w.bind(); }
	auto write_dfac_latch() { return m_dfac_latch_w.bind(); }

	auto read_pa1()  { return m_pa1.bind(); }   // ID bits
	auto read_pa2()  { return m_pa2.bind(); }
	auto read_pa4()  { return m_pa4.bind(); }
	auto read_pa6()  { return m_pa6.bind(); }

	virtual void map(address_map &map) ATTR_COLD;

	template <typename... T> void set_maincpu_tag(T &&... args) { m_maincpu.set_tag(std::forward<T>(args)...); }
	template <typename... T> void set_scsi_tag(T &&... args) { m_ncr.set_tag(std::forward<T>(args)...); }

	void pb3_w(int state) { m_adb_interrupt = state; }
	void cb1_w(int state);  // ADB clock
	void cb2_w(int state);  // ADB data
	void scsi_irq_w(int state);
	void scc_irq_w(int state);

	template <u8 mask>
	void via2_irq_w(int state);

	void via_sync();

	void scsi_drq_w(int state);
	u8 turboscsi_r(offs_t offset);
	void turboscsi_w(offs_t offset, u8 data);
	u32 turboscsi_dma_r(offs_t offset, u32 mem_mask);
	void turboscsi_dma_w(offs_t offset, u32 data, u32 mem_mask);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t via_in_b();
	virtual void via_out_b(uint8_t data);

	virtual u16 iosb_regs_r(offs_t offset);
	virtual void iosb_regs_w(offs_t offset, u16 data, u16 mem_mask);

	devcb_write8 m_adb_st;
	devcb_write_line m_cb1, m_cb2, m_dfac_clock_w, m_dfac_data_w, m_dfac_latch_w;
	devcb_read_line m_pa1, m_pa2, m_pa4, m_pa6;

	required_device<m68000_musashi_device> m_maincpu;
	required_device<ncr53c96_device> m_ncr;
	required_device<via6522_device> m_via1, m_via2;
	required_device<asc_device> m_asc;
	required_device<applefdintf_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;

	u16 m_iosb_regs[0x20];

	u8 m_nubus_irqs;

private:
	emu_timer *m_6015_timer;
	int m_via_interrupt, m_via2_interrupt, m_scc_interrupt, m_last_taken_interrupt;
	floppy_image_device *m_cur_floppy = nullptr;
	int m_hdsel;
	int m_adb_interrupt;
	int m_via2_ca1_hack;

	s32 m_drq, m_scsi_irq, m_asc_irq;
	u32 m_scsi_read_cycles, m_scsi_write_cycles, m_scsi_dma_read_cycles, m_scsi_dma_write_cycles;
	u32 m_scsi_dma_result;
	bool m_scsi_second_half;

	u16 mac_via_r(offs_t offset);
	void mac_via_w(offs_t offset, u16 data, u16 mem_mask);
	u16 mac_via2_r(offs_t offset);
	void mac_via2_w(offs_t offset, u16 data, u16 mem_mask);

	uint8_t via_in_a();
	uint8_t via2_in_a();
	void via_out_a(uint8_t data);
	void field_interrupts();
	void via_out_cb1(int state);
	void via_out_cb2(int state);
	void via2_out_b(uint8_t data);
	void via1_irq(int state);
	void via2_irq(int state);
	void asc_irq(int state);
	TIMER_CALLBACK_MEMBER(mac_6015_tick);

	void phases_w(uint8_t phases);
	void devsel_w(uint8_t devsel);
	u16 swim_r(offs_t offset, u16 mem_mask);
	void swim_w(offs_t offset, u16 data, u16 mem_mask);
};

/// \brief Device class for Apple IOSB I/O controller ASIC.
///
/// IOSB includes 2 VIAs, TurboSCSI logic, a SWIM 2 floppy controller,
/// and more.
class iosb_device : public iosb_base
{
public:
	// construction/destruction
	iosb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t via_in_b() override;
	virtual void via_out_b(uint8_t data) override;

	required_device<rtc3430042_device> m_rtc;

private:
};

class primetime_device : public iosb_base
{
public:
	// construction/destruction
	primetime_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
	primetime_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto pb4_callback() { return write_pb4.bind(); }
	auto pb5_callback() { return write_pb5.bind(); }
	auto pb3_callback() { return read_pb3.bind(); }

protected:
	virtual uint8_t via_in_b() override;
	virtual void via_out_b(uint8_t data) override;

	devcb_write_line write_pb4, write_pb5;
	devcb_read_line read_pb3;

private:
};

class primetimeii_device : public primetime_device
{
public:
	// construction/destruction
	primetimeii_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void map(address_map &map) override ATTR_COLD;

	void ata_irq_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	u16 ata_regs_r(offs_t offset);

	s32 m_ata_irq;
	u16 m_primetimeii_regs[0x20];
};

// device type definition
DECLARE_DEVICE_TYPE(IOSB, iosb_device)
DECLARE_DEVICE_TYPE(PRIMETIME, primetime_device)
DECLARE_DEVICE_TYPE(PRIMETIMEII, primetimeii_device)

#endif // MAME_APPLE_IOSB_H
