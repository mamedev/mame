/***************************************************************************

  includes/psx.h

***************************************************************************/

#if !defined( PSX_H )

#include "cpu/psx/dma.h"

#define PSX_RC_STOP ( 0x01 )
#define PSX_RC_RESET ( 0x04 ) /* guess */
#define PSX_RC_COUNTTARGET ( 0x08 )
#define PSX_RC_IRQTARGET ( 0x10 )
#define PSX_RC_IRQOVERFLOW ( 0x20 )
#define PSX_RC_REPEAT ( 0x40 )
#define PSX_RC_CLC ( 0x100 )
#define PSX_RC_DIV ( 0x200 )

#define PSX_IRQ_ROOTCOUNTER3	0x0001
#define PSX_IRQ_CDROM			0x0004
#define PSX_IRQ_DMA				0x0008
#define PSX_IRQ_ROOTCOUNTER0	0x0010
#define PSX_IRQ_ROOTCOUNTER1	0x0020
#define PSX_IRQ_ROOTCOUNTER2	0x0040
#define PSX_IRQ_SIO0			0x0080
#define PSX_IRQ_SIO1			0x0100
#define PSX_IRQ_SPU				0x0200
#define PSX_IRQ_EXTCD			0x0400
#define PSX_IRQ_MASK			(PSX_IRQ_ROOTCOUNTER3 | PSX_IRQ_CDROM | PSX_IRQ_DMA | PSX_IRQ_ROOTCOUNTER2 | PSX_IRQ_ROOTCOUNTER1 | PSX_IRQ_ROOTCOUNTER0 | PSX_IRQ_SIO0 | PSX_IRQ_SIO1 | PSX_IRQ_SPU | PSX_IRQ_EXTCD)

#define PSX_SIO_OUT_DATA ( 1 )	/* COMMAND */
#define PSX_SIO_OUT_DTR ( 2 )	/* ATT */
#define PSX_SIO_OUT_RTS ( 4 )
#define PSX_SIO_OUT_CLOCK ( 8 )	/* CLOCK */
#define PSX_SIO_IN_DATA ( 1 )	/* DATA */
#define PSX_SIO_IN_DSR ( 2 )	/* ACK */
#define PSX_SIO_IN_CTS ( 4 )

typedef void ( *psx_sio_handler )( running_machine &, int );

#define	DCTSIZE ( 8 )
#define	DCTSIZE2 ( DCTSIZE * DCTSIZE )

#define SIO_BUF_SIZE ( 8 )

#define SIO_STATUS_TX_RDY ( 1 << 0 )
#define SIO_STATUS_RX_RDY ( 1 << 1 )
#define SIO_STATUS_TX_EMPTY ( 1 << 2 )
#define SIO_STATUS_OVERRUN ( 1 << 4 )
#define SIO_STATUS_DSR ( 1 << 7 )
#define SIO_STATUS_IRQ ( 1 << 9 )

#define SIO_CONTROL_TX_ENA ( 1 << 0 )
#define SIO_CONTROL_IACK ( 1 << 4 )
#define SIO_CONTROL_RESET ( 1 << 6 )
#define SIO_CONTROL_TX_IENA ( 1 << 10 )
#define SIO_CONTROL_RX_IENA ( 1 << 11 )
#define SIO_CONTROL_DSR_IENA ( 1 << 12 )
#define SIO_CONTROL_DTR ( 1 << 13 )

#define MDEC_COS_PRECALC_BITS ( 21 )

typedef struct _psx_root psx_root;
struct _psx_root
{
	emu_timer *timer;
	UINT16 n_count;
	UINT16 n_mode;
	UINT16 n_target;
	UINT64 n_start;
};

typedef struct _psx_sio psx_sio;
struct _psx_sio
{
	UINT32 n_status;
	UINT32 n_mode;
	UINT32 n_control;
	UINT32 n_baud;
	UINT32 n_tx;
	UINT32 n_rx;
	UINT32 n_tx_prev;
	UINT32 n_rx_prev;
	UINT32 n_tx_data;
	UINT32 n_rx_data;
	UINT32 n_tx_shift;
	UINT32 n_rx_shift;
	UINT32 n_tx_bits;
	UINT32 n_rx_bits;

	emu_timer *timer;
	psx_sio_handler fn_handler;
};

typedef struct _psx_mdec psx_mdec;
struct _psx_mdec
{
	UINT32 n_decoded;
	UINT32 n_offset;
	UINT16 p_n_output[ 24 * 16 ];

	INT32 p_n_quantize_y[ DCTSIZE2 ];
	INT32 p_n_quantize_uv[ DCTSIZE2 ];
	INT32 p_n_cos[ DCTSIZE2 ];
	INT32 p_n_cos_precalc[ DCTSIZE2 * DCTSIZE2 ];

	UINT32 n_0_command;
	UINT32 n_0_address;
	UINT32 n_0_size;
	UINT32 n_1_command;
	UINT32 n_1_status;

	UINT16 p_n_clamp8[ 256 * 3 ];
	UINT16 p_n_r5[ 256 * 3 ];
	UINT16 p_n_g5[ 256 * 3 ];
	UINT16 p_n_b5[ 256 * 3 ];

	INT32 p_n_unpacked[ DCTSIZE2 * 6 * 2 ];
};

typedef struct _psx_machine psx_machine;
struct _psx_machine
{
	running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }

	running_machine *m_machine;
	UINT32 *p_n_psxram;
	size_t n_psxramsize;

	UINT32 n_com_delay;
	UINT32 n_irqdata;
	UINT32 n_irqmask;

	psx_root root[3];

	psx_sio sio[2];

	psx_mdec mdec;
};

typedef struct _psx_gpu psx_gpu;

class psx_state : public driver_device
{
public:
	psx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	psx_machine *m_p_psx;
	psx_gpu *m_p_psxgpu;

	UINT32 *m_p_n_psxram;
	size_t m_n_psxramsize;
};


/*----------- defined in video/psx.c -----------*/

PALETTE_INIT( psx );
VIDEO_START( psx_type1 );
VIDEO_START( psx_type2 );
SCREEN_UPDATE( psx );
INTERRUPT_GEN( psx_vblank );
extern void psx_gpu_reset( running_machine &machine );
extern void psx_gpu_read( running_machine &, UINT32 *p_ram, INT32 n_size );
extern void psx_gpu_write( running_machine &, UINT32 *p_ram, INT32 n_size );
READ32_HANDLER( psx_gpu_r );
WRITE32_HANDLER( psx_gpu_w );
extern void psx_lightgun_set( running_machine &, int, int );

/*----------- defined in machine/psx.c -----------*/

WRITE32_HANDLER( psx_com_delay_w );
READ32_HANDLER( psx_com_delay_r );
WRITE32_HANDLER( psx_irq_w );
READ32_HANDLER( psx_irq_r );
extern void psx_irq_set( running_machine &, UINT32 );
extern void psx_dma_install_read_handler( running_machine &, int, psx_dma_read_handler );
extern void psx_dma_install_write_handler( running_machine &, int, psx_dma_read_handler );
WRITE32_HANDLER( psx_dma_w );
READ32_HANDLER( psx_dma_r );
WRITE32_HANDLER( psx_counter_w );
READ32_HANDLER( psx_counter_r );
WRITE32_HANDLER( psx_sio_w );
READ32_HANDLER( psx_sio_r );
extern void psx_sio_install_handler( running_machine &, int, psx_sio_handler );
extern void psx_sio_input( running_machine &, int, int, int );

WRITE32_HANDLER( psx_mdec_w );
READ32_HANDLER( psx_mdec_r );
extern void psx_machine_init( running_machine &machine );
extern void psx_driver_init( running_machine &machine );

#define PSX_H ( 1 )
#endif
