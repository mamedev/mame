// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Template for skeleton device

***************************************************************************/

#pragma once

#ifndef __NAMCO_C148DEV_H__
#define __NAMCO_C148DEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NAMCO_C148_ADD(_tag, _cputag, _cpumaster) \
	MCFG_DEVICE_ADD(_tag, NAMCO_C148, 0) \
	namco_c148_device::configure_device(*device, _cputag, _cpumaster);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> namco_c148_device

class namco_c148_device : public device_t
{
public:
	// construction/destruction
	namco_c148_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_ADDRESS_MAP(map, 16);

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
private:
	cpu_device *m_hostcpu;      		/**< reference to the host cpu */
	namco_c148_device *m_linked_c148;	/**< reference to linked master/slave c148 */
	const char *m_hostcpu_tag;		/**< host cpu tag name */
	const char *m_linked_c148_tag;	/**< other c148 tag name */
	bool		m_hostcpu_master;	/**< define if host cpu is master */
	struct{
		uint8_t cpu;
		uint8_t ex;
		uint8_t sci;
		uint8_t pos;
		uint8_t vblank;
	}m_irqlevel;

	uint8_t m_posirq_line;
	void flush_irq_acks();
};


// device type definition
extern const device_type NAMCO_C148;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
