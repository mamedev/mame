#ifndef __MultiPCM_H__
#define __MultiPCM_H__

struct MultiPCM_interface
{
	int region;
};


WRITE8_HANDLER( MultiPCM_reg_0_w );
READ8_HANDLER( MultiPCM_reg_0_r);
WRITE8_HANDLER( MultiPCM_reg_1_w );
READ8_HANDLER( MultiPCM_reg_1_r);

void multipcm_set_bank(int which, UINT32 leftoffs, UINT32 rightoffs);

#endif
