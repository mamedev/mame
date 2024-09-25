// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_HEATHROW_H
#define MAME_APPLE_HEATHROW_H

#pragma once

#include "dbdma.h"

#include "machine/pci.h"
#include "machine/6522via.h"
#include "machine/applefdintf.h"
#include "machine/swim3.h"
#include "machine/z80scc.h"
#include "speaker.h"

class macio_device :  public pci_device
{
public:
	// construction/destruction
	macio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// interface routines
	auto irq_callback() { return write_irq.bind(); }
	auto pb4_callback() { return write_pb4.bind(); }
	auto pb5_callback() { return write_pb5.bind(); }
	auto cb2_callback() { return write_cb2.bind(); }
	auto pb3_callback() { return read_pb3.bind(); }

	auto codec_r_callback() { return read_codec.bind(); }
	auto codec_w_callback() { return write_codec.bind(); }

	template <typename... T> void set_maincpu_tag(T &&... args) { m_maincpu.set_tag(std::forward<T>(args)...); }

	void cb1_w(int state);
	void cb2_w(int state);
	void scc_irq_w(int state);

	template <int bit> void set_irq_line(int state);

	u32 codec_dma_read(u32 offset);
	void codec_dma_write(u32 offset, u32 data);

	u32 codec_r(offs_t offset);
	void codec_w(offs_t offset, u32 data);

protected:
	// device-level overrides
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void base_map(address_map &map) ATTR_COLD;
	virtual void config_map(address_map &map) override ATTR_COLD;

	void common_init();

	uint16_t swim_r(offs_t offset, u16 mem_mask);
	void swim_w(offs_t offset, u16 data, u16 mem_mask);

	u32 macio_r(offs_t offset);
	void macio_w(offs_t offset, u32 data, u32 mem_mask);

	u8 fdc_r(offs_t offset);
	void fdc_w(offs_t offset, u8 data);

	u16 scc_r(offs_t offset);
	void scc_w(offs_t offset, u16 data);
	u8 scc_macrisc_r(offs_t offset);
	void scc_macrisc_w(offs_t offset, u8 data);

	u16 mac_via_r(offs_t offset);
	void mac_via_w(offs_t offset, u16 data, u16 mem_mask);

	devcb_write_line write_irq, write_pb4, write_pb5, write_cb2;
	devcb_read_line read_pb3;

	devcb_read32 read_codec;
	devcb_write32 write_codec;

	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via1;
	required_device<applefdintf_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<z80scc_device> m_scc;
	required_device<dbdma_device> m_dma_scsi, m_dma_floppy, m_dma_sccatx, m_dma_sccarx;
	required_device<dbdma_device> m_dma_sccbtx, m_dma_sccbrx, m_dma_audio_in, m_dma_audio_out;

private:
	floppy_image_device *m_cur_floppy = nullptr;
	int m_hdsel;

	u8 via_in_a();
	u8 via_in_b();
	void via_out_a(u8 data);
	void via_out_b(u8 data);
	void via_sync();
	void field_interrupts();
	void via_out_cb2(int state);

	void phases_w(u8 phases);
	void devsel_w(u8 devsel);

	u32 m_toggle;

	// Interrupts
	void recalc_irqs();
	u32 m_InterruptEvents, m_InterruptMask, m_InterruptLevels;
	u32 m_InterruptEvents2, m_InterruptMask2, m_InterruptLevels2;
};

class grandcentral_device : public macio_device
{
public:
	// construction/destruction
	grandcentral_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void map(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<dbdma_device> m_dma_scsi1;
};

class ohare_device : public macio_device, public device_nvram_interface
{
public:
	// construction/destruction
	ohare_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	ohare_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void map(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	required_device<dbdma_device> m_dma_ata0, m_dma_ata1;

	u8 nvram_r(offs_t offset);
	void nvram_w(offs_t offset, u8 data);

	u8 m_nvram[0x8000];
};

class heathrow_device : public ohare_device
{
public:
	// construction/destruction
	heathrow_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	heathrow_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void map(address_map &map) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};

class paddington_device : public heathrow_device
{
public:
	// construction/destruction
	paddington_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(GRAND_CENTRAL, grandcentral_device)
DECLARE_DEVICE_TYPE(HEATHROW, heathrow_device)
DECLARE_DEVICE_TYPE(PADDINGTON, paddington_device)
DECLARE_DEVICE_TYPE(OHARE, ohare_device)

#endif // MAME_APPLE_HEATHROW_H
