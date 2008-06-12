/*----------- defined in machine/chaknpop.c -----------*/

extern UINT8 *chaknpop_ram;
DRIVER_INIT( chaknpop );
MACHINE_RESET( chaknpop );
READ8_HANDLER( chaknpop_mcu_portA_r );
READ8_HANDLER( chaknpop_mcu_portB_r );
READ8_HANDLER( chaknpop_mcu_portC_r );
WRITE8_HANDLER( chaknpop_mcu_portA_w );
WRITE8_HANDLER( chaknpop_mcu_portB_w );
WRITE8_HANDLER( chaknpop_mcu_portC_w );


/*----------- defined in video/chaknpop.c -----------*/

extern UINT8 *chaknpop_txram;
extern UINT8 *chaknpop_sprram;
extern size_t chaknpop_sprram_size;
extern UINT8 *chaknpop_attrram;

PALETTE_INIT( chaknpop );
VIDEO_START( chaknpop );
VIDEO_UPDATE( chaknpop );

READ8_HANDLER( chaknpop_gfxmode_r );
WRITE8_HANDLER( chaknpop_gfxmode_w );

WRITE8_HANDLER( chaknpop_txram_w );

WRITE8_HANDLER( chaknpop_attrram_w );
