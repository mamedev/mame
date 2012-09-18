/*----------- defined in machine/cchip.c -----------*/

MACHINE_RESET( cchip1 );
DECLARE_READ16_HANDLER( cchip1_ctrl_r );
DECLARE_READ16_HANDLER( cchip1_ram_r );
DECLARE_WRITE16_HANDLER( cchip1_ctrl_w );
DECLARE_WRITE16_HANDLER( cchip1_bank_w );
DECLARE_WRITE16_HANDLER( cchip1_ram_w );
