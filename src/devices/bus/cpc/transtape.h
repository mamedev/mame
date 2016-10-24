// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * transtape.c  --  Hard Micro SA Transtape
 *
 * Spanish hacking device
 *
 */

#ifndef TRANSTAPE_H_
#define TRANSTAPE_H_

#include "emu.h"
#include "cpcexp.h"

class cpc_transtape_device  : public device_t,
						public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_transtape_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual void set_mapping(uint8_t type) override;
	virtual void romen_w(int state) override { m_romen = state; }

	uint8_t input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void output_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void button_red_w(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void button_black_w(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	cpc_expansion_slot_device *m_slot;
	cpu_device* m_cpu;
	address_space* m_space;
	std::unique_ptr<uint8_t[]> m_ram;  // 8kB internal RAM
	bool m_rom_active;
	bool m_romen;
	uint8_t m_output;

	void map_enable();
};

// device type definition
extern const device_type CPC_TRANSTAPE;

#endif /* TRANSTAPE_H_ */
