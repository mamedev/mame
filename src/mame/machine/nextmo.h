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
	nextmo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<nextmo_device &>(device).irq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_drq_wr_callback(device_t &device, _Object object) { return downcast<nextmo_device &>(device).drq_cb.set_callback(object); }

	DECLARE_ADDRESS_MAP(map, 32);

	uint8_t r4_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void r4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t r5_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void r5_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t r6_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void r6_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t r7_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void r7_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t r8_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void r8_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t r9_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void r9_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ra_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ra_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void rb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t r10_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void r10_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

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

extern const device_type NEXTMO;

#endif
