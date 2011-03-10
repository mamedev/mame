#ifndef __GSTRIKER_H
#define __GSTRIKER_H

/*** VS920A **********************************************/

#define MAX_VS920A 2

typedef struct
{
	tilemap_t* tmap;
	UINT16* vram;
	UINT16 pal_base;
	UINT8 gfx_region;

} sVS920A;

/*** MB60553 **********************************************/

#define MAX_MB60553 2

typedef struct
{
	tilemap_t* tmap;
	UINT16* vram;
	UINT16 regs[8];
	UINT8 bank[8];
	UINT16 pal_base;
	UINT8 gfx_region;

} tMB60553;

/*** CG10103 **********************************************/

#define MAX_CG10103 2

typedef struct
{
	UINT16* vram;
	UINT16 pal_base;
	UINT8 gfx_region;
	UINT8 transpen;

} tCG10103;

class gstriker_state : public driver_device
{
public:
	gstriker_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 dmmy_8f_ret;
	int pending_command;
	UINT16 *work_ram;
	int gametype;
	UINT16 mcu_data;
	UINT16 prot_reg[2];
	UINT16 *lineram;
	sVS920A VS920A[MAX_VS920A];
	tMB60553 MB60553[MAX_MB60553];
	tCG10103 CG10103[MAX_CG10103];
	sVS920A* VS920A_cur_chip;
	tMB60553 *MB60553_cur_chip;
	tCG10103* CG10103_cur_chip;
};


/*----------- defined in video/gstriker.c -----------*/

WRITE16_HANDLER( VS920A_0_vram_w );
WRITE16_HANDLER( VS920A_1_vram_w );
WRITE16_HANDLER( MB60553_0_regs_w );
WRITE16_HANDLER( MB60553_1_regs_w );
WRITE16_HANDLER( MB60553_0_vram_w );
WRITE16_HANDLER( MB60553_1_vram_w );

SCREEN_UPDATE( gstriker );
VIDEO_START( gstriker );
VIDEO_START( twrldc94 );
VIDEO_START( vgoalsoc );
#endif
