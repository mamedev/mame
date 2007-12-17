#ifndef __GSTRIKER_H
#define __GSTRIKER_H

/*----------- defined in video/gstriker.c -----------*/

/*** VS920A **********************************************/

#define MAX_VS920A	2

typedef struct
{
	tilemap* tmap;
	UINT16* vram;
	UINT16 pal_base;
	UINT8 gfx_region;
} sVS920A;

extern sVS920A VS920A[MAX_VS920A];

#define VS920A_0_vram	(VS920A[0].vram)
#define VS920A_1_vram	(VS920A[1].vram)

extern WRITE16_HANDLER( VS920A_0_vram_w );
extern WRITE16_HANDLER( VS920A_1_vram_w );



/*** MB60553 **********************************************/

#define MAX_MB60553 2

typedef struct
{
	tilemap* tmap;
	UINT16* vram;
	UINT16 regs[8];
	UINT8 bank[8];
	UINT16 pal_base;
	UINT8 gfx_region;

} tMB60553;

extern tMB60553 MB60553[MAX_MB60553];

#define MB60553_0_vram	(MB60553[0].vram)
#define MB60553_1_vram	(MB60553[1].vram)

extern WRITE16_HANDLER(MB60553_0_regs_w);
extern WRITE16_HANDLER(MB60553_1_regs_w);

extern WRITE16_HANDLER(MB60553_0_vram_w);
extern WRITE16_HANDLER(MB60553_1_vram_w);


/*** CG10103 **********************************************/

#define MAX_CG10103 2

typedef struct
{
	UINT16* vram;
	UINT16 pal_base;
	UINT8 gfx_region;

} tCG10103;

extern tCG10103 CG10103[MAX_CG10103];

#define CG10103_0_vram	(CG10103[0].vram)
#define CG10103_1_vram	(CG10103[1].vram)

#endif
