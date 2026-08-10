// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_HEATHROW_H
#define MAME_APPLE_HEATHROW_H

#pragma once

#include "dbdma.h"
#include "mesh.h"

#include "bus/ata/ataintf.h"
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
	macio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	// interface routines
	auto irq_callback() { return write_irq.bind(); }
	auto pb4_callback() { return write_pb4.bind(); }
	auto pb5_callback() { return write_pb5.bind(); }
	auto cb2_callback() { return write_cb2.bind(); }
	auto pb3_callback() { return read_pb3.bind(); }

	auto codec_r_callback() { return read_codec.bind(); }
	auto codec_w_callback() { return write_codec.bind(); }

	auto iobus_a_r_callback() { return read_iobus_a.bind(); }
	auto iobus_a_w_callback() { return write_iobus_a.bind(); }
	auto iobus_b_r_callback() { return read_iobus_b.bind(); }
	auto iobus_b_w_callback() { return write_iobus_b.bind(); }
	auto iobus_c_r_callback() { return read_iobus_c.bind(); }
	auto iobus_c_w_callback() { return write_iobus_c.bind(); }
	auto iobus_d_r_callback() { return read_iobus_d.bind(); }
	auto iobus_d_w_callback() { return write_iobus_d.bind(); }

	template <typename... T> void set_maincpu_tag(T &&... args) { m_maincpu.set_tag(std::forward<T>(args)...); }

	void cb1_w(int state);
	void cb2_w(int state);
	void scc_irq_w(int state);

	template <int bit> void set_irq_line(int state);

	u32 codec_dma_read(offs_t offset);
	void codec_dma_write(offs_t offset, u32 data);

	void scsi0_irq(int state) { set_irq_line<12>(state); }
	void scsi0_drq(int state);

	void fdc_drq(int state);

protected:
	// device_t implementattion
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void base_map(address_map &map) ATTR_COLD;
	virtual void config_map(address_map &map) override ATTR_COLD;

	void common_init();

	u32 macio_r(offs_t offset);
	void macio_w(offs_t offset, u32 data, u32 mem_mask);

	u32 codec_r(offs_t offset, u32 mem_mask);
	void codec_w(offs_t offset, u32 data, u32 mem_mask);

	u8 fdc_r(offs_t offset);
	void fdc_w(offs_t offset, u8 data);

	u16 scc_r(offs_t offset);
	void scc_w(offs_t offset, u16 data, u16 mem_mask);
	u8 scc_macrisc_r(offs_t offset);
	void scc_macrisc_w(offs_t offset, u8 data);

	u16 mac_via_r(offs_t offset);
	void mac_via_w(offs_t offset, u16 data, u16 mem_mask);

	template <devcb_read32 macio_device::*R> u32 iobus_r(offs_t offset, u32 mem_mask);
	template <devcb_write32 macio_device::*W> void iobus_w(offs_t offset, u32 data, u32 mem_mask);

	devcb_write_line write_irq, write_pb4, write_pb5, write_cb2;
	devcb_read_line read_pb3;

	devcb_read32 read_codec;
	devcb_write32 write_codec;

	devcb_read8 read_fdc_dma;
	devcb_write8 write_fdc_dma;

	devcb_read32 read_iobus_a, read_iobus_b, read_iobus_c, read_iobus_d;
	devcb_write32 write_iobus_a, write_iobus_b, write_iobus_c, write_iobus_d;

	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via1;
	required_device<applefdintf_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<z80scc_device> m_scc;
	required_device<dbdma_device> m_dma_scsi0, m_dma_floppy, m_dma_sccatx, m_dma_sccarx;
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
	grandcentral_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void map(address_map &map) ATTR_COLD;

	// Grand Central has no SCSI controller of its own: it interfaces to two external ones.
	auto scsi0_r_callback() { return read_scsi0.bind(); }
	auto scsi0_w_callback() { return write_scsi0.bind(); }
	auto scsi0_dma_r_callback() { return read_scsi0_dma.bind(); }
	auto scsi0_dma_w_callback() { return write_scsi0_dma.bind(); }

	auto scsi1_r_callback() { return read_scsi1.bind(); }
	auto scsi1_w_callback() { return write_scsi1.bind(); }

	auto iobus_e_r_callback() { return read_iobus_e.bind(); }
	auto iobus_e_w_callback() { return write_iobus_e.bind(); }
	auto iobus_f_r_callback() { return read_iobus_f.bind(); }
	auto iobus_f_w_callback() { return write_iobus_f.bind(); }

	void scsi1_irq(int state) { set_irq_line<13>(state); }
	void scsi1_drq(int state) { m_dma_scsi1->drq_w(state); }
	auto scsi1_dma_r_callback() { return read_scsi1_dma.bind(); }
	auto scsi1_dma_w_callback() { return write_scsi1_dma.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t cache_line_size_r() override { return 0x08; }

private:
	u8 scsi0_r(offs_t offset);
	void scsi0_w(offs_t offset, u8 data);
	u32 scsi0_dma_r(offs_t offset);
	void scsi0_dma_w(offs_t offset, u32 data);

	u8 scsi1_r(offs_t offset);
	void scsi1_w(offs_t offset, u8 data);
	u32 scsi1_dma_r(offs_t offset);
	void scsi1_dma_w(offs_t offset, u32 data);

	template <devcb_read32 grandcentral_device::*R> u32 iobus_r(offs_t offset, u32 mem_mask);
	template <devcb_write32 grandcentral_device::*W> void iobus_w(offs_t offset, u32 data, u32 mem_mask);

	required_device<dbdma_device> m_dma_scsi1;

	devcb_read8 read_scsi0;
	devcb_write8 write_scsi0;
	devcb_read16 read_scsi0_dma;
	devcb_write16 write_scsi0_dma;

	devcb_read8 read_scsi1;
	devcb_write8 write_scsi1;
	devcb_read16 read_scsi1_dma;
	devcb_write16 write_scsi1_dma;

	devcb_read32 read_iobus_e, read_iobus_f;
	devcb_write32 write_iobus_e, write_iobus_f;
};

class ohare_device : public macio_device, public device_nvram_interface
{
public:
	// construction/destruction
	ohare_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);
	ohare_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

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
	required_device<mesh_device> m_mesh;
	required_device_array<ata_interface_device, 2> m_ata;

	void ohare_start();

	u8 mesh_r(offs_t offset);
	void mesh_w(offs_t offset, u8 data);

	template <int Ch> u32 ata_r(offs_t offset, u32 mem_mask);
	template <int Ch> void ata_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Ch> void ata_dmarq(int state);

	u8 nvram_r(offs_t offset);
	void nvram_w(offs_t offset, u8 data);

	u8 m_nvram[0x8000];
	u32 m_ata_config[2];
};

class heathrow_device : public ohare_device
{
public:
	// construction/destruction
	heathrow_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);
	heathrow_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void map(address_map &map) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};

class paddington_device : public heathrow_device
{
public:
	// construction/destruction
	paddington_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

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
