// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/* Sega 315-5641 / D77591 / 9442CA010 */

// this is the PICO sound chip, we are not sure if it's the same as a 7759 or not, it requires FIFO logic
// which the 7759 does _not_ have but it is possible that is handled somewhere else on the PICO hardawre.

#include "upd7759.h"


class sega_315_5641_pcm_device : public upd7759_device
{
public:
	sega_315_5641_pcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t get_fifo_space();
	void advance_state() override;
	virtual void port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	uint8_t       m_fifo_data[0x40];
	uint8_t       m_fifo_read;    // last read offset (will read in m_fifo_read+1)
	uint8_t       m_fifo_write;   // write offset

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;


};

extern const device_type SEGA_315_5641_PCM;
