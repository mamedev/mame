// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef NEXTMO_H
#define NEXTMO_H

#define MCFG_NEXTMO_IRQ_CALLBACK(_write) \
	devcb = &nextmo_device::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_NEXTMO_DRQ_CALLBACK(_write) \
	devcb = &nextmo_device::set_drq_wr_callback(*device, DEVCB_##_write);

class nextmo_device : public device_t
{
public:
	nextmo_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<nextmo_device &>(device).irq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_drq_wr_callback(device_t &device, _Object object) { return downcast<nextmo_device &>(device).drq_cb.set_callback(object); }

	DECLARE_ADDRESS_MAP(map, 32);

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

	UINT8 dma_r();
	void dma_w(UINT8 data);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	UINT8 sector[0x510];
	UINT8 r4, r5, r6, r7;
	devcb_write_line irq_cb, drq_cb;
	int sector_pos;

	void check_dma_end();
	void check_ecc();
	void compute_ecc();
};

extern const device_type NEXTMO;

#endif
