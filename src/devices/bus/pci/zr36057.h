// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_PCI_ZR36057_H
#define MAME_BUS_PCI_ZR36057_H

#pragma once

#include "pci_slot.h"
#include "video/saa7110.h"
#include "video/zr36060.h"

class zr36057_device : public pci_card_device
{
public:
	zr36057_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::CAPTURE; }

protected:
	zr36057_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

//  virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	required_device<zr36060_device> m_guest;
	required_device<saa7110a_device> m_decoder;
	void asr_map(address_map &map) ATTR_COLD;

	void software_reset();
	u32 postoffice_r(offs_t offset);
	void postoffice_w(offs_t offset, u32 data, u32 mem_mask);

	// Video Front End
	struct {
		u32 horizontal_config;
		u32 vertical_config;
		int hspol, vspol;
		u16 hstart, hend, vstart, vend;
	} m_vfe;

	u8 m_jpeg_guest_id, m_jpeg_guest_reg;

	bool m_softreset;
	u8 m_gpio_ddr, m_pci_waitstate_control;

	struct {
		u8 time[8];
	} m_guestbus;

	struct {
		bool dir; /**< true: Write, false: Read */
		bool time_out;
		bool pending;
		u8 guest_id;
		u8 guest_reg;
	} m_po; /**< PostOffice */

	int m_decoder_sdao_state;
};

DECLARE_DEVICE_TYPE(ZR36057_PCI, zr36057_device)

#endif // MAME_BUS_PCI_ZR36057_H
