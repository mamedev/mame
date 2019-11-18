// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Namco C148 - CPU Bus Manager

***************************************************************************/
#ifndef MAME_MACHINE_NAMCO_C148_H
#define MAME_MACHINE_NAMCO_C148_H

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

	void map(address_map &map);

	template <typename T> void set_hostcpu(T &&tag, bool is_master)
	{
		m_hostcpu.set_tag(std::forward<T>(tag));
		m_hostcpu_master = is_master;
	}

	template <typename T> void link_c148_device(T &&tag) { m_linked_c148.set_tag(std::forward<T>(tag)); }

	auto out_ext1_callback() { return m_out_ext1_cb.bind(); }
	auto out_ext2_callback() { return m_out_ext2_cb.bind(); }

	DECLARE_READ8_MEMBER( vblank_irq_level_r );
	DECLARE_WRITE8_MEMBER( vblank_irq_level_w );
	DECLARE_READ16_MEMBER( vblank_irq_ack_r );
	DECLARE_WRITE16_MEMBER( vblank_irq_ack_w );

	DECLARE_READ8_MEMBER( pos_irq_level_r );
	DECLARE_WRITE8_MEMBER( pos_irq_level_w );
	DECLARE_READ16_MEMBER( pos_irq_ack_r );
	DECLARE_WRITE16_MEMBER( pos_irq_ack_w );

	DECLARE_READ8_MEMBER( cpu_irq_level_r );
	DECLARE_WRITE8_MEMBER( cpu_irq_level_w );
	DECLARE_READ16_MEMBER( cpu_irq_ack_r );
	DECLARE_WRITE16_MEMBER( cpu_irq_ack_w );

	DECLARE_READ8_MEMBER( ex_irq_level_r );
	DECLARE_WRITE8_MEMBER( ex_irq_level_w );
	DECLARE_READ16_MEMBER( ex_irq_ack_r );
	DECLARE_WRITE16_MEMBER( ex_irq_ack_w );

	DECLARE_READ8_MEMBER( sci_irq_level_r );
	DECLARE_WRITE8_MEMBER( sci_irq_level_w );
	DECLARE_READ16_MEMBER( sci_irq_ack_r );
	DECLARE_WRITE16_MEMBER( sci_irq_ack_w );

	DECLARE_READ8_MEMBER( ext_posirq_line_r );
	DECLARE_WRITE8_MEMBER( ext_posirq_line_w );
	DECLARE_WRITE16_MEMBER( cpu_irq_assert_w );

	DECLARE_READ8_MEMBER( bus_ctrl_r );
	DECLARE_WRITE8_MEMBER( bus_ctrl_w );

	DECLARE_READ8_MEMBER( ext_r );
	DECLARE_WRITE8_MEMBER( ext1_w );
	DECLARE_WRITE8_MEMBER( ext2_w );
	void vblank_irq_trigger();
	void pos_irq_trigger();
	void ex_irq_trigger();
	void sci_irq_trigger();
	uint8_t get_posirq_line();

protected:
	void cpu_irq_trigger();
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;

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


#endif // MAME_MACHINE_NAMCO_C148_H
