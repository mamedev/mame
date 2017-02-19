// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp9845_io.h

    I/O bus of HP9845 systems

*********************************************************************/

#pragma once

#ifndef _HP9845_IO_H_
#define _HP9845_IO_H_


#define MCFG_HP9845_IO_SLOT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, HP9845_IO_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(hp9845_io_slot_devices, nullptr, false)

#define HP9845_IO_FIRST_SC  1   // Lowest SC used by I/O cards

#define MCFG_HP9845_IO_SC(_default_sc)              \
	PORT_START("SC") \
	PORT_CONFNAME(0xf , (_default_sc) - HP9845_IO_FIRST_SC , "Select Code") \
	PORT_CONFSETTING(0 , "1")\
	PORT_CONFSETTING(1 , "2")\
	PORT_CONFSETTING(2 , "3")\
	PORT_CONFSETTING(3 , "4")\
	PORT_CONFSETTING(4 , "5")\
	PORT_CONFSETTING(5 , "6")\
	PORT_CONFSETTING(6 , "7")\
	PORT_CONFSETTING(7 , "8")\
	PORT_CONFSETTING(8 , "9")\
	PORT_CONFSETTING(9 , "10")\
	PORT_CONFSETTING(10 , "11")\
	PORT_CONFSETTING(11 , "12")

class hp9845_io_slot_device : public device_t,
							  public device_slot_interface
{
public:
	// construction/destruction
	hp9845_io_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp9845_io_slot_device();

	// device-level overrides
	virtual void device_start() override;
};

class hp9845b_state;

class hp9845_io_card_device : public device_t,
							  public device_slot_card_interface
{
protected:
	// construction/destruction
	hp9845_io_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	virtual ~hp9845_io_card_device();

	// device-level overrides
	virtual void device_reset() override;

	hp9845b_state *m_sys;
	required_ioport m_select_code_port;
	uint8_t m_my_sc;

	// card device handling
	void irq_w(int state);
	void sts_w(int state);
	void flg_w(int state);
	void install_readwrite_handler(read16_delegate rhandler, write16_delegate whandler);
};

// device type definition
extern const device_type HP9845_IO_SLOT;

SLOT_INTERFACE_EXTERN(hp9845_io_slot_devices);

#endif /* _HP9845_IO_H_ */
