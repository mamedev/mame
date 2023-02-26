// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_HEATHROW_H
#define MAME_APPLE_HEATHROW_H

#pragma once

#include "machine/pci.h"
#include "machine/6522via.h"
#include "machine/applefdintf.h"
#include "machine/swim3.h"
#include "machine/z80scc.h"
#include "speaker.h"
#include "sound/awacs.h"

// ======================> heathrow_device

class heathrow_device :  public pci_device
{
public:
	// construction/destruction
	heathrow_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	heathrow_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// interface routines
	auto pb4_callback() { return write_pb4.bind(); }
	auto pb5_callback() { return write_pb5.bind(); }
	auto cb2_callback() { return write_cb2.bind(); }
	auto pb3_callback() { return read_pb3.bind(); }

	void map(address_map &map);

	template <typename... T> void set_maincpu_tag(T &&... args) { m_maincpu.set_tag(std::forward<T>(args)...); }

	DECLARE_WRITE_LINE_MEMBER(cb1_w);
	DECLARE_WRITE_LINE_MEMBER(cb2_w);
	DECLARE_WRITE_LINE_MEMBER(scc_irq_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void config_map(address_map &map) override;

	void common_init();

private:
	emu_timer *m_6015_timer;
	devcb_write_line write_pb4, write_pb5, write_cb2;
	devcb_read_line read_pb3;

	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via1;
	required_device<applefdintf_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<awacs_device> m_awacs;
	required_device<z80scc_device> m_scc;

	floppy_image_device *m_cur_floppy = nullptr;
	int m_hdsel;

	u8 m_nvram[0x20000/4];

	u16 mac_via_r(offs_t offset);
	void mac_via_w(offs_t offset, u16 data, u16 mem_mask);

	u8 via_in_a();
	u8 via_in_b();
	void via_out_a(u8 data);
	void via_out_b(u8 data);
	void via_sync();
	void field_interrupts();
	DECLARE_WRITE_LINE_MEMBER(via_out_cb2);
	DECLARE_WRITE_LINE_MEMBER(via1_irq);
	DECLARE_WRITE_LINE_MEMBER(via2_irq);
	DECLARE_WRITE_LINE_MEMBER(asc_irq);
	TIMER_CALLBACK_MEMBER(mac_6015_tick);

	void phases_w(u8 phases);
	void devsel_w(u8 devsel);
	uint16_t swim_r(offs_t offset, u16 mem_mask);
	void swim_w(offs_t offset, u16 data, u16 mem_mask);

	u32 macio_r(offs_t offset);
	void macio_w(offs_t offset, u32 data, u32 mem_mask);

	u8 fdc_r(offs_t offset);
	void fdc_w(offs_t offset, u8 data);

	u8 nvram_r(offs_t offset);
	void nvram_w(offs_t offset, u8 data);

	u16 scc_r(offs_t offset);
	void scc_w(offs_t offset, u16 data);
	u8 scc_macrisc_r(offs_t offset);
	void scc_macrisc_w(offs_t offset, u8 data);

	u32 m_toggle;
	u32 unk_r(offs_t offset);

	// DMA
	u32 sound_dma_output(offs_t offset);
	void sound_dma_input(offs_t offset, u32 value);
};

class paddington_device : public heathrow_device
{
public:
	// construction/destruction
	paddington_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
};

class grandcentral_device : public heathrow_device
{
public:
	// construction/destruction
	grandcentral_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
};

class ohare_device : public heathrow_device
{
public:
	// construction/destruction
	ohare_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
};

// device type definition
DECLARE_DEVICE_TYPE(GRAND_CENTRAL, grandcentral_device)
DECLARE_DEVICE_TYPE(HEATHROW, heathrow_device)
DECLARE_DEVICE_TYPE(PADDINGTON, paddington_device)
DECLARE_DEVICE_TYPE(OHARE, ohare_device)

#endif // MAME_APPLE_HEATHROW_H
