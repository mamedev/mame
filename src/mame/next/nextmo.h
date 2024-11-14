// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_NEXT_NEXTMO_H
#define MAME_NEXT_NEXTMO_H

#pragma once


class nextmo_device : public device_t
{
public:
	nextmo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_wr_callback() { return irq_cb.bind(); }
	auto drq_wr_callback() { return drq_cb.bind(); }

	void map(address_map &map) ATTR_COLD;

	uint8_t r4_r();
	void r4_w(uint8_t data);
	uint8_t r5_r();
	void r5_w(uint8_t data);
	uint8_t r6_r();
	void r6_w(uint8_t data);
	uint8_t r7_r();
	void r7_w(uint8_t data);
	uint8_t r8_r();
	void r8_w(uint8_t data);
	uint8_t r9_r();
	void r9_w(uint8_t data);
	uint8_t ra_r();
	void ra_w(uint8_t data);
	uint8_t rb_r();
	void rb_w(uint8_t data);
	uint8_t r10_r(offs_t offset);
	void r10_w(offs_t offset, uint8_t data);

	uint8_t dma_r();
	void dma_w(uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t sector[0x510];
	uint8_t r4, r5, r6, r7;
	devcb_write_line irq_cb, drq_cb;
	int sector_pos;

	void check_dma_end();
	void check_ecc();
	void compute_ecc();
};

DECLARE_DEVICE_TYPE(NEXTMO, nextmo_device)

#endif // MAME_NEXT_NEXTMO_H
