/***************************************************************************

  includes/psx.h

***************************************************************************/

#if !defined( PSX_H )

/*----------- defined in video/psx.c -----------*/

PALETTE_INIT( psx );
VIDEO_START( psx_type1 );
VIDEO_START( psx_type2 );
VIDEO_UPDATE( psx );
INTERRUPT_GEN( psx_vblank );
extern void psx_gpu_reset( void );
extern void psx_gpu_read( UINT32 *p_ram, INT32 n_size );
extern void psx_gpu_write( UINT32 *p_ram, INT32 n_size );
READ32_HANDLER( psx_gpu_r );
WRITE32_HANDLER( psx_gpu_w );
extern void psx_lightgun_set( int, int );

/*----------- defined in machine/psx.c -----------*/

extern UINT32 *g_p_n_psxram;
extern size_t g_n_psxramsize;
typedef void ( *psx_dma_read_handler )( UINT32, INT32 );
typedef void ( *psx_dma_write_handler )( UINT32, INT32 );
WRITE32_HANDLER( psx_com_delay_w );
READ32_HANDLER( psx_com_delay_r );
WRITE32_HANDLER( psx_irq_w );
READ32_HANDLER( psx_irq_r );
extern void psx_irq_set( UINT32 );
extern void psx_dma_install_read_handler( int, psx_dma_read_handler );
extern void psx_dma_install_write_handler( int, psx_dma_read_handler );
WRITE32_HANDLER( psx_dma_w );
READ32_HANDLER( psx_dma_r );
WRITE32_HANDLER( psx_counter_w );
READ32_HANDLER( psx_counter_r );
WRITE32_HANDLER( psx_sio_w );
READ32_HANDLER( psx_sio_r );
typedef void ( *psx_sio_handler )( int );
extern void psx_sio_install_handler( int, psx_sio_handler );
extern void psx_sio_input( int, int, int );

#define PSX_SIO_OUT_DATA ( 1 )	/* COMMAND */
#define PSX_SIO_OUT_DTR ( 2 )	/* ATT */
#define PSX_SIO_OUT_RTS ( 4 )
#define PSX_SIO_OUT_CLOCK ( 8 )	/* CLOCK */
#define PSX_SIO_IN_DATA ( 1 )	/* DATA */
#define PSX_SIO_IN_DSR ( 2 )	/* ACK */
#define PSX_SIO_IN_CTS ( 4 )

WRITE32_HANDLER( psx_mdec_w );
READ32_HANDLER( psx_mdec_r );
extern void psx_machine_init( void );
extern void psx_driver_init( void );

#define PSX_H ( 1 )
#endif
