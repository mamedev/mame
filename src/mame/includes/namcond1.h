/***************************************************************************

  namcond1.h

  Common functions & declarations for the Namco ND-1 driver

***************************************************************************/

class namcond1_state : public driver_device
{
public:
	namcond1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_h8_irq5_enabled;
	UINT16 *m_shared_ram;
	int m_p8;
	DECLARE_WRITE16_MEMBER(sharedram_sub_w);
	DECLARE_READ16_MEMBER(sharedram_sub_r);
	DECLARE_READ8_MEMBER(mcu_p7_read);
	DECLARE_READ8_MEMBER(mcu_pa_read);
	DECLARE_WRITE8_MEMBER(mcu_pa_write);
	DECLARE_READ16_MEMBER(namcond1_shared_ram_r);
	DECLARE_READ16_MEMBER(namcond1_cuskey_r);
	DECLARE_WRITE16_MEMBER(namcond1_shared_ram_w);
	DECLARE_WRITE16_MEMBER(namcond1_cuskey_w);
};


/*----------- defined in machine/namcond1.c -----------*/


MACHINE_START( namcond1 );
MACHINE_RESET( namcond1 );

