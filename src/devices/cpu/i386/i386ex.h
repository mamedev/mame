// license:BSD-3-Clause
// copyright-holders: Felipe Sanches
#ifndef MAME_CPU_I386_I386EX_H
#define MAME_CPU_I386_I386EX_H
#include "i386.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ins8250.h"

class i386ex_device : public i386_device
{
public:
	// construction/destruction
	i386ex_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t get_slave_ack(offs_t offset);
	void set_pin_configuration(offs_t offset, uint16_t data);
	void set_timer_configuration(offs_t offset, uint16_t data);
	void set_serial_io_configuration(offs_t offset, uint16_t data);
	void set_dma_configuration(offs_t offset, uint16_t data);
	uint16_t get_interrupt_configuration(offs_t offset);
	void set_interrupt_configuration(offs_t offset, uint16_t data);
	void chip_select_unit_w(offs_t offset, uint16_t data);
	void set_clock_prescaler(offs_t offset, uint16_t data);

protected:
	void io_map(address_map&);

	// device-level overrides
	virtual u8 mem_pr8(offs_t address) override { return macache16.read_byte(address); };
	virtual u16 mem_pr16(offs_t address) override { return macache16.read_word(address); };
	virtual u32 mem_pr32(offs_t address) override { return macache16.read_dword(address); };
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_reset() override;
	address_space_config m_386ex_io_config;

	device_memory_interface::space_config_vector memory_space_config() const override
	{
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_IO,      &m_386ex_io_config)
		};
	}

private:
	uint16_t m_INTCFG;
	uint16_t m_CS_address[8];
	uint16_t m_CS_mask[8];
	required_device<ns16450_device> m_uart0;
	required_device<ns16450_device> m_uart1;
	required_device<pit8254_device> m_pit;
	required_device<pic8259_device> m_pic_master;
	required_device<pic8259_device> m_pic_slave;
};

DECLARE_DEVICE_TYPE(I386EX, i386ex_device)

#endif // MAME_CPU_I386_I386EX_H
