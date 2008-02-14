/*****************************************************************************

    MB14241 shifter IC emulation

 *****************************************************************************/

#ifndef __MB14241_H__
#define __MB14241_H__

void mb14241_init(int num);

WRITE8_HANDLER( mb14241_0_shift_count_w );
WRITE8_HANDLER( mb14241_0_shift_data_w );
READ8_HANDLER( mb14241_0_shift_result_r );

#endif
