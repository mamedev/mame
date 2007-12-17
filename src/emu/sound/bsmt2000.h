/**********************************************************************************************
 *
 *   BSMT2000 driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#ifndef BSMT2000_H
#define BSMT2000_H

struct BSMT2000interface
{
	int region;						/* memory region where the sample ROM lives */
};

WRITE16_HANDLER( BSMT2000_data_0_w );

extern const struct BSMT2000interface bsmt2000_interface_region_1;
extern const struct BSMT2000interface bsmt2000_interface_region_2;
extern const struct BSMT2000interface bsmt2000_interface_region_3;
extern const struct BSMT2000interface bsmt2000_interface_region_4;

#endif
