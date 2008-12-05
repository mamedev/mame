/*----------- defined in machine/cchip.c -----------*/

extern UINT16 *cchip2_ram;
READ16_HANDLER ( cchip2_word_r );
WRITE16_HANDLER( cchip2_word_w );

MACHINE_RESET( cchip1 );
READ16_HANDLER( cchip1_ctrl_r );
READ16_HANDLER( cchip1_ram_r );
WRITE16_HANDLER( cchip1_ctrl_w );
WRITE16_HANDLER( cchip1_bank_w );
WRITE16_HANDLER( cchip1_ram_w );


/*----------- defined in machine/bonzeadv.c -----------*/

READ16_HANDLER( bonzeadv_cchip_ctrl_r );
READ16_HANDLER( bonzeadv_cchip_ram_r );
WRITE16_HANDLER( bonzeadv_cchip_ctrl_w );
WRITE16_HANDLER( bonzeadv_cchip_bank_w );
WRITE16_HANDLER( bonzeadv_cchip_ram_w );


/*----------- defined in machine/opwolf.c -----------*/

void opwolf_cchip_init(running_machine *machine);
READ16_HANDLER( opwolf_cchip_status_r );
READ16_HANDLER( opwolf_cchip_data_r );
WRITE16_HANDLER( opwolf_cchip_status_w );
WRITE16_HANDLER( opwolf_cchip_data_w );
WRITE16_HANDLER( opwolf_cchip_bank_w );


/*----------- defined in machine/rainbow.c -----------*/

void rainbow_cchip_init(running_machine *machine, int version);
READ16_HANDLER( rainbow_cchip_ctrl_r );
READ16_HANDLER( rainbow_cchip_ram_r );
WRITE16_HANDLER( rainbow_cchip_ctrl_w );
WRITE16_HANDLER( rainbow_cchip_bank_w );
WRITE16_HANDLER( rainbow_cchip_ram_w );


/*----------- defined in machine/volfied.c -----------*/

void volfied_cchip_init(running_machine *machine);
READ16_HANDLER( volfied_cchip_ctrl_r );
READ16_HANDLER( volfied_cchip_ram_r );
WRITE16_HANDLER( volfied_cchip_ctrl_w );
WRITE16_HANDLER( volfied_cchip_bank_w );
WRITE16_HANDLER( volfied_cchip_ram_w );
