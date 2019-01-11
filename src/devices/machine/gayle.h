// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    GAYLE

    Gate array used in the Amiga 600 and Amiga 1200 computers.

***************************************************************************/

#ifndef MAME_MACHINE_GAYLE_H
#define MAME_MACHINE_GAYLE_H

#pragma once



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_GAYLE_ADD(_tag, _clock, _id) \
	MCFG_DEVICE_ADD(_tag, GAYLE, _clock) \
	downcast<gayle_device &>(*device).set_id(_id);

#define MCFG_GAYLE_INT2_HANDLER(_devcb) \
	devcb = &downcast<gayle_device &>(*device).set_int2_handler(DEVCB_##_devcb);

#define MCFG_GAYLE_CS0_READ_HANDLER(_devcb) \
	devcb = &downcast<gayle_device &>(*device).set_cs0_read_handler(DEVCB_##_devcb);

#define MCFG_GAYLE_CS0_WRITE_HANDLER(_devcb) \
	devcb = &downcast<gayle_device &>(*device).set_cs0_write_handler(DEVCB_##_devcb);

#define MCFG_GAYLE_CS1_READ_HANDLER(_devcb) \
	devcb = &downcast<gayle_device &>(*device).set_cs1_read_handler(DEVCB_##_devcb);

#define MCFG_GAYLE_CS1_WRITE_HANDLER(_devcb) \
	devcb = &downcast<gayle_device &>(*device).set_cs1_write_handler(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gayle_device

class gayle_device : public device_t
{
public:
	// construction/destruction
	gayle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	template <class Object> devcb_base &set_int2_handler(Object &&cb) { return m_int2_w.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_cs0_read_handler(Object &&cb) { return m_cs0_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_cs0_write_handler(Object &&cb) { return m_cs0_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_cs1_read_handler(Object &&cb) { return m_cs1_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_cs1_write_handler(Object &&cb) { return m_cs1_write.set_callback(std::forward<Object>(cb)); }

	// interface
	DECLARE_WRITE_LINE_MEMBER( ide_interrupt_w );

	DECLARE_READ16_MEMBER( gayle_r );
	DECLARE_WRITE16_MEMBER( gayle_w );
	DECLARE_READ16_MEMBER( gayle_id_r );
	DECLARE_WRITE16_MEMBER( gayle_id_w );

	// inline configuration
	void set_id(uint8_t id) { m_gayle_id = id; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum
	{
		GAYLE_CS = 0,   // interrupt status
		GAYLE_IRQ,      // interrupt change
		GAYLE_INTEN,    // interrupt enable register
		GAYLE_CFG       // config register
	};

	devcb_write_line m_int2_w;

	devcb_read16 m_cs0_read;
	devcb_write16 m_cs0_write;
	devcb_read16 m_cs1_read;
	devcb_write16 m_cs1_write;

	uint8_t m_gayle_id;
	int m_gayle_id_count;
	uint8_t m_gayle_reg[4];
};

// device type definition
DECLARE_DEVICE_TYPE(GAYLE, gayle_device)

#endif // MAME_MACHINE_GAYLE_H
