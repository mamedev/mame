/*----------- defined in machine/cchip.c -----------*/

MACHINE_RESET( cchip1 );
READ16_HANDLER( cchip1_ctrl_r );
READ16_HANDLER( cchip1_ram_r );
WRITE16_HANDLER( cchip1_ctrl_w );
WRITE16_HANDLER( cchip1_bank_w );
WRITE16_HANDLER( cchip1_ram_w );

