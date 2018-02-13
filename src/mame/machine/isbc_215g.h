// license:BSD-3-Clause
// copyright-holders:Carl

#ifndef MAME_MACHINE_ISBC_215G_H
#define MAME_MACHINE_ISBC_215G_H

#pragma once

#include "cpu/i8089/i8089.h"
#include "bus/isbx/isbx.h"
#include "imagedev/harddriv.h"

class isbc_215g_device : public device_t
{
public:
	isbc_215g_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ16_MEMBER(io_r);
	DECLARE_WRITE16_MEMBER(io_w);
	DECLARE_READ16_MEMBER(mem_r);
	DECLARE_WRITE16_MEMBER(mem_w);

	static void static_set_wakeup_addr(device_t &device, uint32_t wakeup) { downcast<isbc_215g_device &>(device).m_wakeup = wakeup; }
	static void static_set_maincpu_tag(device_t &device, const char *maincpu_tag) { downcast<isbc_215g_device &>(device).m_maincpu_tag = maincpu_tag; }
	template<class _Object> static devcb_base &static_set_irq_callback(device_t &device, _Object object) { return downcast<isbc_215g_device &>(device).m_out_irq_func.set_callback(object); }

	void isbc_215g_io(address_map &map);
	void isbc_215g_mem(address_map &map);
protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	const tiny_rom_entry *device_rom_region() const override;

private:
	void find_sector();
	uint16_t read_sector();
	bool write_sector(uint16_t data);

	required_device<i8089_device> m_dmac;
	required_device<harddisk_image_device> m_hdd0;
	required_device<harddisk_image_device> m_hdd1;
	required_device<isbx_slot_device> m_sbx1;
	required_device<isbx_slot_device> m_sbx2;

	devcb_write_line m_out_irq_func;

	int m_reset;
	uint16_t m_wakeup, m_secoffset, m_sector[512];
	const char *m_maincpu_tag;
	address_space *m_maincpu_mem;
	uint32_t m_lba[2];
	uint16_t m_cyl[2];
	uint8_t m_idcompare[4], m_drive, m_head, m_index;
	int8_t m_format_bytes;
	bool m_idfound, m_stepdir, m_wrgate, m_rdgate, m_amsrch;

	bool m_isbx_irq[4], m_fdctc, m_step, m_format;

	const struct hard_disk_info* m_geom[2];

	DECLARE_WRITE_LINE_MEMBER(isbx_irq_00_w);
	DECLARE_WRITE_LINE_MEMBER(isbx_irq_01_w);
	DECLARE_WRITE_LINE_MEMBER(isbx_irq_10_w);
	DECLARE_WRITE_LINE_MEMBER(isbx_irq_11_w);
};

#define MCFG_ISBC_215_ADD(_tag, _wakeup, _maincpu_tag) \
	MCFG_DEVICE_ADD(_tag, ISBC_215G, 0) \
	isbc_215g_device::static_set_wakeup_addr(*device, _wakeup); \
	isbc_215g_device::static_set_maincpu_tag(*device, _maincpu_tag);

#define MCFG_ISBC_215_IRQ(_irq_line) \
	devcb = &isbc_215g_device::static_set_irq_callback(*device, DEVCB_##_irq_line);

DECLARE_DEVICE_TYPE(ISBC_215G, isbc_215g_device)

#endif // MAME_MACHINE_ISBC_215G_H
