/*----------- defined in machine/mexico86.c -----------*/

extern UINT8 *mexico86_protection_ram;

WRITE8_HANDLER( mexico86_f008_w );
INTERRUPT_GEN( kikikai_interrupt );
INTERRUPT_GEN( mexico86_m68705_interrupt );
READ8_HANDLER( mexico86_68705_portA_r );
WRITE8_HANDLER( mexico86_68705_portA_w );
WRITE8_HANDLER( mexico86_68705_ddrA_w );
READ8_HANDLER( mexico86_68705_portB_r );
WRITE8_HANDLER( mexico86_68705_portB_w );
WRITE8_HANDLER( mexico86_68705_ddrB_w );


/*----------- defined in video/mexico86.c -----------*/

extern UINT8 *mexico86_videoram,*mexico86_objectram;
extern size_t mexico86_objectram_size;

WRITE8_HANDLER( mexico86_bankswitch_w );
VIDEO_UPDATE( mexico86 );
VIDEO_UPDATE( kikikai );
