// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_BUS_SPECTRUM_DMA_SLOT_H
#define MAME_BUS_SPECTRUM_DMA_SLOT_H

#include "emu.h"
#include "osdcomm.h"
#pragma once

#include "cpu/z80/z80.h"
#include "machine/z80dma.h"

class device_dma_card_interface;

class dma_slot_device : public device_t
	, public device_single_card_slot_interface<device_dma_card_interface>
{
	friend class device_dma_card_interface;

public:
	dma_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T>
	dma_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&opts, const char *dflt)
		: dma_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	virtual ~dma_slot_device();

	template <typename T> void set_io_space(T &&tag, int spacenum) { m_io.set_tag(std::forward<T>(tag), spacenum); }

	auto out_busreq_callback() { return m_out_busreq_cb.bind(); }
	auto out_int_callback() { return m_out_int_cb.bind(); }
	auto in_mreq_callback() { return m_in_mreq_cb.bind(); }
	auto out_mreq_callback() { return m_out_mreq_cb.bind(); }
	auto in_iorq_callback() { return m_in_iorq_cb.bind(); }
	auto out_iorq_callback() { return m_out_iorq_cb.bind(); }

	void busreq_w(int state);
	void int_w(int state);
	void mreq_w(int state);
	u8 mreq_r(offs_t offset);
	void iorq_w(int state);
	u8 iorq_r(offs_t offset);

	void bai_w(int state);

	address_space &io() const { return *m_io; }

protected:
	virtual void device_start() override ATTR_COLD;

	required_address_space m_io;

	devcb_write_line   m_out_busreq_cb;
	devcb_write_line   m_out_int_cb;
	devcb_read8        m_in_mreq_cb;
	devcb_write8       m_out_mreq_cb;
	devcb_read8        m_in_iorq_cb;
	devcb_write8       m_out_iorq_cb;

	device_dma_card_interface *m_card;
};

class device_dma_card_interface : public device_interface
{
public:
	virtual ~device_dma_card_interface();

	virtual void bai_w(int state) {};

protected:
	device_dma_card_interface(const machine_config &mconfig, device_t &device);

	dma_slot_device *m_slot;
};

class dma_device : public device_t, public device_dma_card_interface
{
public:
	virtual void bai_w(int state) override;

protected:
	dma_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_validity_check(validity_checker &valid) const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	required_device<z80dma_device> m_dma;
	u8 m_dma_port;
};

DECLARE_DEVICE_TYPE(DMA_SLOT, dma_slot_device)

void default_dma_slot_devices(device_slot_interface &device);

#endif // MAME_BUS_SPECTRUM_DMA_SLOT_H
