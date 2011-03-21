/***************************************************************************

  namcond1.h

  Common functions & declarations for the Namco ND-1 driver

***************************************************************************/

/*----------- defined in machine/namcond1.c -----------*/

extern UINT8 namcond1_h8_irq5_enabled;
extern UINT16 *namcond1_shared_ram;

extern READ16_HANDLER( namcond1_shared_ram_r );
extern READ16_HANDLER( namcond1_cuskey_r );
extern WRITE16_HANDLER( namcond1_shared_ram_w );
extern WRITE16_HANDLER( namcond1_cuskey_w );

MACHINE_START( namcond1 );
MACHINE_RESET( namcond1 );

