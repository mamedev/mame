/************************************************************************************

    Bellfruit dotmatrix driver, (under heavy construction !!!)

*************************************************************************************/
#ifndef BFM_DM01
#define BFM_DM01

struct bfmdm01_interface
{
	void (*busy_func)(running_machine &machine, int state);
};
ADDRESS_MAP_EXTERN( bfm_dm01_memmap,8 );

void BFM_dm01_config(running_machine &machine, const bfmdm01_interface *intf);

INTERRUPT_GEN( bfm_dm01_vbl );

void BFM_dm01_reset(running_machine &machine);

void BFM_dm01_writedata(running_machine &machine,UINT8 data);

int BFM_dm01_busy(void);

#endif
