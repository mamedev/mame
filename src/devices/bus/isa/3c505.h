// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_ISA_3C505_H
#define MAME_BUS_ISA_3C505_H

#pragma once

#include "cpu/i86/i186.h"
#include "machine/i82586.h"
#include "machine/ram.h"

#include "bus/isa/isa.h"

class isa16_3c505_device
	: public device_t
	, public device_isa16_card_interface
{
public:
	isa16_3c505_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void map_main(address_map &map) ATTR_COLD;
	void map_io(address_map &map) ATTR_COLD;
	void map_isa(address_map &map) ATTR_COLD;

	enum acr_mask : u8
	{
		ACR_ASF1 = 0x01, // adapter status flag 1
		ACR_ASF2 = 0x02, // adapter status flag 2
		ACR_ASF3 = 0x04, // adapter status flag 3
		ACR_LED1 = 0x08, // led control 1
		ACR_LED2 = 0x10, // led control 2
		ACR_R586 = 0x20, // reset 82586
		ACR_FLSH = 0x40, // flush data register
		ACR_LPBK = 0x80, // loopback control

		ACR_ASF  = 0x07,
	};
	enum asr_mask : u8
	{
		ASR_HSF1 = 0x01, // host status flag 1
		ASR_HSF2 = 0x02, // host status flag 2
		ASR_SWTC = 0x04, // external switch
		ASR_8_16 = 0x08, // 8/16 bit
		ASR_DIR  = 0x10, // direction flag
		ASR_HCRF = 0x20, // host command register full
		ASR_ACRE = 0x40, // adapter command register empty
		ASR_ARDY = 0x80, // data register ready

		ASR_HSF  = 0x03,
	};
	enum hcr_mask : u8
	{
		HCR_HSF1 = 0x01, // host status flag 1
		HCR_HSF2 = 0x02, // host status flag 2
		HCR_CMDE = 0x04, // command register interrupt enable
		HCR_TCEN = 0x08, // terminal count interrupt enable
		HCR_DIR  = 0x10, // direction flag
		HCR_DMAE = 0x20, // dma enable
		HCR_FLSH = 0x40, // flush data register
		HCR_ATTN = 0x80, // attention

		HCR_HSF  = 0x03,
	};
	enum hsr_mask : u8
	{
		HSR_ASF1 = 0x01, // adapter status flag 1
		HSR_ASF2 = 0x02, // adapter status flag 2
		HSR_ASF3 = 0x04, // adapter status flag 3
		HSR_DONE = 0x08, // dma done
		HSR_DIR  = 0x10, // direction
		HSR_ACRF = 0x20, // adapter command register full
		HSR_HCRE = 0x40, // host command register empty
		HSR_HRDY = 0x80, // data register ready

		HSR_ASF  = 0x07,
	};
	enum hdr_mask : u8
	{
		HDR_BRST = 0x01, // dma burst
	};

	// adapter register helpers
	u8 acmd_r();
	u8 acr_r() { return m_acr; }
	u8 asr_r() { return m_asr; }
	u16 adata_r();
	void acmd_w(u8 data);
	void acr_w(u8 data);
	void adata_w(u16 data);

	// host register helpers
	u8 hcmd_r();
	u8 hsr_r() { return m_hsr; }
	u16 hdata_r(offs_t offset, u16 mem_mask = 0xffff);
	u8 hcr_r() { return m_hcr; }
	void hcmd_w(u8 data);
	void hdr_w(u8 data) { m_hdr = data & HDR_BRST; }
	void hdata_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	void hcr_w(u8 data);

	void update_cpu_drq(int state);
	void update_cpu_irq(int state);
	void update_isa_drq(int state);
	void update_isa_irq(int state);
	void update_rdy(u8 const acr, u8 const hcr);

	// dma helpers
	virtual u8 dack_r(int line) override { return hdata_r(0, 0x00ff); }
	virtual void dack_w(int line, u8 data) override { hdata_w(0, data, 0x00ff); }
	virtual void eop_w(int state) override;
	virtual u16 dack16_r(int line) override { return hdata_r(0); }
	virtual void dack16_w(int line, u16 data) override { hdata_w(0, data); }

private:
	required_device<i80186_cpu_device> m_cpu;
	required_device<i82586_device> m_net;
	required_device<ram_device> m_ram;

	output_finder<2> m_led;

	required_ioport m_iobase;
	required_ioport m_irqdrq;
	required_ioport m_romopts;
	required_ioport m_test;

	u8 m_acmdr; // adapter command register
	u8 m_acr;   // adapter control register
	u8 m_asr;   // adapter status register

	u8 m_hcmdr; // host command register
	u8 m_hcr;   // host control register
	u8 m_hsr;   // host status register
	u8 m_hdr;   // host aux dma register

	static constexpr unsigned FIFO_SIZE = 20;
	util::fifo<u8, FIFO_SIZE> m_data;

	bool m_installed;
	unsigned m_isa_irq;
	unsigned m_isa_drq;

	bool m_cpu_drq_asserted;
	bool m_cpu_irq_asserted;
	bool m_isa_drq_asserted;
	bool m_isa_irq_asserted;
};

// device type definition
DECLARE_DEVICE_TYPE(ISA16_3C505, isa16_3c505_device)

#endif // MAME_BUS_ISA_3C505_H
