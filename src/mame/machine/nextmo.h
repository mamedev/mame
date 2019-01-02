// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_NEXTMO_H
#define MAME_MACHINE_NEXTMO_H

#pragma once


class nextmo_device : public device_t
{
public:
	nextmo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_wr_callback() { return irq_cb.bind(); }
	auto drq_wr_callback() { return drq_cb.bind(); }

	void map(address_map &map);

	DECLARE_READ8_MEMBER(r4_r);
	DECLARE_WRITE8_MEMBER(r4_w);
	DECLARE_READ8_MEMBER(r5_r);
	DECLARE_WRITE8_MEMBER(r5_w);
	DECLARE_READ8_MEMBER(r6_r);
	DECLARE_WRITE8_MEMBER(r6_w);
	DECLARE_READ8_MEMBER(r7_r);
	DECLARE_WRITE8_MEMBER(r7_w);
	DECLARE_READ8_MEMBER(r8_r);
	DECLARE_WRITE8_MEMBER(r8_w);
	DECLARE_READ8_MEMBER(r9_r);
	DECLARE_WRITE8_MEMBER(r9_w);
	DECLARE_READ8_MEMBER(ra_r);
	DECLARE_WRITE8_MEMBER(ra_w);
	DECLARE_READ8_MEMBER(rb_r);
	DECLARE_WRITE8_MEMBER(rb_w);
	DECLARE_READ8_MEMBER(r10_r);
	DECLARE_WRITE8_MEMBER(r10_w);

	uint8_t dma_r();
	void dma_w(uint8_t data);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

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

#endif // MAME_MACHINE_NEXTMO_H
