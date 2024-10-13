// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_VME_IP4_H
#define MAME_BUS_VME_IP4_H

#pragma once

#include "cpu/mips/mips1.h"

#include "machine/ds1315.h"
#include "machine/mc68681.h"
#include "machine/pit8253.h"
#include "machine/wd33c9x.h"
#include "sound/saa1099.h"

#include "bus/rs232/rs232.h"
#include "bus/vme/vme.h"

class sgi_ip4_device
	: public device_t
	, public device_vme_card_interface
{
public:
	sgi_ip4_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_config_complete() override;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void map(address_map &map) ATTR_COLD;

	template <unsigned N> void lio_irq(int state) { lio_irq(N, state); }
	void lio_irq(unsigned number, int state);
	template <unsigned N> void vme_irq(int state) { vme_irq(N, state); }
	void vme_irq(unsigned number, int state);
	void scsi_drq(int state);

	u16 cpucfg_r() { return m_cpucfg; }
	void cpucfg_w(u16 data);

	void parity_r(offs_t offset, u32 &data, u32 mem_mask);
	void parity_w(offs_t offset, u32 &data, u32 mem_mask);

	void mailbox_w(offs_t offset, u8 data);

private:
	required_device<mips1_device_base> m_cpu;
	required_device<ds1315_device> m_rtc;
	required_device<pit8254_device> m_pit;
	required_device<wd33c9x_base_device> m_scsi;
	required_device_array<scn2681_device, 3> m_duart;
	required_device_array<rs232_port_device, 4> m_serial;
	required_device<saa1099_device> m_saa;

	memory_share_creator<u8> m_nvram;
	required_shared_ptr<u32> m_ram;

	output_finder<5> m_leds;

	// machine registers
	u16 m_cpucfg;
	u16 m_dmalo;
	u16 m_dmahi;
	u8 m_lio_isr;
	u8 m_vme_isr;
	u8 m_vme_imr;
	u8 m_parerr;
	u32 m_erradr;

	// other machine state
	std::unique_ptr<u8[]> m_parity;
	memory_passthrough_handler m_parity_mph;
	u32 m_parity_bad;
	bool m_lio_irq;
	bool m_vme_irq;
};

DECLARE_DEVICE_TYPE(SGI_IP4, sgi_ip4_device)

#endif // MAME_BUS_VME_IP4_H
