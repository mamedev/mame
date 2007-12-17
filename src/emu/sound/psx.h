/***************************************************************************

    PSX SPU

    preliminary version by smf.

***************************************************************************/

#if !defined( PSX_SPU_H )

WRITE32_HANDLER( psx_spu_w );
READ32_HANDLER( psx_spu_r );
WRITE32_HANDLER( psx_spu_delay_w );
READ32_HANDLER( psx_spu_delay_r );

typedef void ( *spu_handler )( UINT32, INT32 );

struct PSXSPUinterface
{
	UINT32 **p_psxram;
	void (*irq_set)(UINT32);
	void (*spu_install_read_handler)(int,spu_handler);
	void (*spu_install_write_handler)(int,spu_handler);
};

#define PSX_SPU_H ( 1 )
#endif
