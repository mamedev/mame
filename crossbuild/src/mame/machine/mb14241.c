/***************************************************************************

    Midway 8080-based black and white hardware

****************************************************************************/

#include "driver.h"
#include "mb14241.h"


#define MAX_MB14241  1

struct MB14241
{
	UINT16 shift_data;	/* 15 bits only */
	UINT8 shift_count;	/* 3 bits */
};

static struct MB14241 chips[MAX_MB14241];


static void mb14241_shift_count_w(int num, UINT8 data)
{
	chips[num].shift_count = ~data & 0x07;
}

WRITE8_HANDLER( mb14241_0_shift_count_w ) { mb14241_shift_count_w(0, data); }


static void mb14241_shift_data_w(int num, UINT8 data)
{
	chips[num].shift_data = (chips[num].shift_data >> 8) | ((UINT16)data << 7);
}

WRITE8_HANDLER( mb14241_0_shift_data_w ) { mb14241_shift_data_w(0, data); }


static UINT8 mb14241_shift_result_r(int num)
{
	return chips[num].shift_data >> chips[num].shift_count;
}

READ8_HANDLER( mb14241_0_shift_result_r ) { return mb14241_shift_result_r(0); }


void mb14241_init(int num)
{
	state_save_register_item("mb14241", num, chips[num].shift_data);
	state_save_register_item("mb14241", num, chips[num].shift_count);
}
