// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Namco C148 - CPU Bus Manager

***************************************************************************/
#ifndef MAME_NAMCO_NAMCO_C148_H
#define MAME_NAMCO_NAMCO_C148_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> namco_c148_device

class namco_c148_device : public device_t
{
public:
	// construction/destruction
	template <typename T>
	namco_c148_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&hostcpu, bool is_master)
		: namco_c148_device(mconfig, tag, owner, clock)
	{
		set_hostcpu(std::forward<T>(hostcpu), is_master);
	}
	namco_c148_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_map &map) ATTR_COLD;

	template <typename T> void set_hostcpu(T &&tag, bool is_master)
	{
		m_hostcpu.set_tag(std::forward<T>(tag));
		m_hostcpu_master = is_master;
	}

	template <typename T> void link_c148_device(T &&tag) { m_linked_c148.set_tag(std::forward<T>(tag)); }

	auto out_ext1_callback() { return m_out_ext1_cb.bind(); }
	auto out_ext2_callback() { return m_out_ext2_cb.bind(); }

	uint8_t vblank_irq_level_r();
	void vblank_irq_level_w(uint8_t data);
	uint16_t vblank_irq_ack_r();
	void vblank_irq_ack_w(uint16_t data);

	uint8_t pos_irq_level_r();
	void pos_irq_level_w(uint8_t data);
	uint16_t pos_irq_ack_r();
	void pos_irq_ack_w(uint16_t data);

	uint8_t cpu_irq_level_r();
	void cpu_irq_level_w(uint8_t data);
	uint16_t cpu_irq_ack_r();
	void cpu_irq_ack_w(uint16_t data);

	uint8_t ex_irq_level_r();
	void ex_irq_level_w(uint8_t data);
	uint16_t ex_irq_ack_r();
	void ex_irq_ack_w(uint16_t data);

	uint8_t sci_irq_level_r();
	void sci_irq_level_w(uint8_t data);
	uint16_t sci_irq_ack_r();
	void sci_irq_ack_w(uint16_t data);

	uint8_t ext_posirq_line_r();
	void ext_posirq_line_w(uint8_t data);
	void cpu_irq_assert_w(uint16_t data);

	uint8_t bus_ctrl_r();
	void bus_ctrl_w(uint8_t data);

	uint8_t ext_r();
	void ext1_w(uint8_t data);
	void ext2_w(uint8_t data);
	void vblank_irq_trigger();
	void pos_irq_trigger();
	void ex_irq_trigger();
	void sci_irq_trigger();
	uint8_t get_posirq_line();

protected:
	void cpu_irq_trigger();
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_write8 m_out_ext1_cb;
	devcb_write8 m_out_ext2_cb;

	required_device<cpu_device> m_hostcpu;              // reference to the host cpu
	optional_device<namco_c148_device> m_linked_c148;   // reference to linked master/slave c148
	bool        m_hostcpu_master;                       // define if host cpu is master

	struct{
		uint8_t cpu;
		uint8_t ex;
		uint8_t sci;
		uint8_t pos;
		uint8_t vblank;
	}m_irqlevel;

	uint8_t m_posirq_line;
	uint8_t m_bus_reg;
	void flush_irq_acks();
};


// device type definition
DECLARE_DEVICE_TYPE(NAMCO_C148, namco_c148_device)


#endif // MAME_NAMCO_NAMCO_C148_H
