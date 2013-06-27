/*****************************************************************************
 *
 * includes/tandy1t.h
 *
 ****************************************************************************/

#ifndef TANDY1T_H_
#define TANDY1T_H_

#include "includes/pc.h"

class tandy_pc_state : public pc_state
{
public:
	tandy_pc_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc_state(mconfig, type, tag)
		, m_romcs0(*this, "romcs0")
		, m_romcs1(*this, "romcs1")
		, m_biosbank(*this, "biosbank")
	{ }

	DECLARE_WRITE8_MEMBER ( pc_t1t_p37x_w );
	DECLARE_READ8_MEMBER ( pc_t1t_p37x_r );

	DECLARE_WRITE8_MEMBER ( tandy1000_pio_w );
	DECLARE_READ8_MEMBER(tandy1000_pio_r);
	DECLARE_READ8_MEMBER( tandy1000_bank_r );
	DECLARE_WRITE8_MEMBER( tandy1000_bank_w );

	int tandy1000_read_eeprom();
	void tandy1000_write_eeprom(UINT8 data);
	void tandy1000_set_bios_bank();

	DECLARE_DRIVER_INIT(t1000hx);
	DECLARE_DRIVER_INIT(t1000sl);

	DECLARE_MACHINE_RESET(tandy1000rl);

	struct
	{
		UINT8 low, high;
	} m_eeprom_ee[0x40]; /* only 0 to 4 used in hx, addressing seems to allow this */

protected:
	int m_eeprom_state;
	int m_eeprom_clock;
	UINT8 m_eeprom_oper;
	UINT16 m_eeprom_data;

	UINT8 m_tandy_data[8];

	UINT8 m_tandy_bios_bank;    /* I/O port FFEAh */
	UINT8 m_tandy_ppi_portb, m_tandy_ppi_portc;

	// Memory regions for the machines that support rom banking
	optional_memory_region m_romcs0;
	optional_memory_region m_romcs1;
	optional_memory_bank m_biosbank;
};

extern NVRAM_HANDLER( tandy1000 );
INPUT_PORTS_EXTERN( t1000_keyboard );


#endif /* TANDY1T_H_ */
