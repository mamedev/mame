// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, MetalliC
#ifndef __MD_STM95_H
#define __MD_STM95_H

#include "md_slot.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

/* ST M95320 32Kbit serial EEPROM implementation */
// TO DO: STM95 should be made a separate EEPROM device and this should be merged with md_eeprom.c!

#define M95320_SIZE 0x1000

enum STMSTATE
{
	IDLE = 0,
	CMD_WRSR,
	CMD_RDSR,
	M95320_CMD_READ,
	CMD_WRITE,
	READING,
	WRITING
};

class stm95_eeprom_device
{
public:
	stm95_eeprom_device(running_machine &machine, UINT8 *eeprom);
	running_machine &machine() const { return m_machine; }

	UINT8   *eeprom_data;
	void    set_cs_line(int);
	void    set_halt_line(int state) {}; // not implemented
	void    set_si_line(int);
	void    set_sck_line(int state);
	int     get_so_line(void);

protected:
	int     latch;
	int     reset_line;
	int     sck_line;
	int     WEL;

	STMSTATE    stm_state;
	int     stream_pos;
	int     stream_data;
	int     eeprom_addr;

	running_machine& m_machine;
};


// ======================> md_eeprom_stm95_device

class md_eeprom_stm95_device : public device_t,
						public device_md_cart_interface
{
public:
	// construction/destruction
	md_eeprom_stm95_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	md_eeprom_stm95_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read) override;
	virtual DECLARE_READ16_MEMBER(read_a13) override;
	virtual DECLARE_WRITE16_MEMBER(write_a13) override;

private:
	UINT8 m_bank[3];
	int m_rdcnt;

	std::unique_ptr<stm95_eeprom_device> m_stm95;
};


// device type definition
extern const device_type MD_EEPROM_STM95;

#endif
