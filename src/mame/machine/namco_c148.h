// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Namco C148 - CPU Bus Manager

***************************************************************************/
#ifndef MAME_MACHINE_NAMCO_C148_H
#define MAME_MACHINE_NAMCO_C148_H

#pragma once




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NAMCO_C148_ADD(_tag, _cputag, _cpumaster) \
	MCFG_DEVICE_ADD(_tag, NAMCO_C148, 0) \
	namco_c148_device::configure_device(*device, _cputag, _cpumaster);

#define MCFG_NAMCO_C148_EXT1_CB(_cb) \
	devcb = &namco_c148_device::set_out_ext1_callback(*device, DEVCB_##_cb);

#define MCFG_NAMCO_C148_EXT2_CB(_cb) \
	devcb = &namco_c148_device::set_out_ext2_callback(*device, DEVCB_##_cb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> namco_c148_device

class namco_c148_device : public device_t
{
public:
	// construction/destruction
	namco_c148_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_map &map);

	static void configure_device(device_t &device, const char *tag, bool is_master)
	{
		namco_c148_device &dev = downcast<namco_c148_device &>(device);
		dev.m_hostcpu_tag = tag;
		dev.m_hostcpu_master = is_master;
	}

	static void link_c148_device(device_t &device, const char *tag)
	{
		namco_c148_device &dev = downcast<namco_c148_device &>(device);

		dev.m_linked_c148_tag = tag;
	}

	template<class _Object> static devcb_base &set_out_ext1_callback(device_t &device, _Object object) { return downcast<namco_c148_device &>(device).m_out_ext1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_ext2_callback(device_t &device, _Object object) { return downcast<namco_c148_device &>(device).m_out_ext2_cb.set_callback(object); }

	devcb_write8 m_out_ext1_cb;
	devcb_write8 m_out_ext2_cb;

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
//  virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_validity_check(validity_checker &valid) const override;
private:
	cpu_device *m_hostcpu;              /**< reference to the host cpu */
	namco_c148_device *m_linked_c148;   /**< reference to linked master/slave c148 */
	const char *m_hostcpu_tag;      /**< host cpu tag name */
	const char *m_linked_c148_tag;  /**< other c148 tag name */
	bool        m_hostcpu_master;   /**< define if host cpu is master */
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



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif // MAME_MACHINE_NAMCO_C148_H
