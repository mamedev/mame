// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef _TC0110PCR_H_
#define _TC0110PCR_H_


class tc0110pcr_device : public device_t
{
public:
	tc0110pcr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~tc0110pcr_device() {}

	uint16_t word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff); /* color index goes up in step of 2 */
	void step1_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);   /* color index goes up in step of 1 */
	void step1_rbswap_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);    /* swaps red and blue components */
	void step1_4bpg_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);  /* only 4 bits per color gun */

	void restore_colors();

	static void static_set_palette_tag(device_t &device, const char *tag);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	std::unique_ptr<uint16_t[]>     m_ram;
	int          m_type;
	int          m_addr;
	required_device<palette_device> m_palette;
};

extern const device_type TC0110PCR;

#define MCFG_TC0110PCR_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TC0110PCR, 0)
#define MCFG_TC0110PCR_PALETTE(_palette_tag) \
	tc0110pcr_device::static_set_palette_tag(*device, "^" _palette_tag);

#endif
