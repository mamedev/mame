#ifndef NAMCO54_H
#define NAMCO54_H

#include "sound/discrete.h"

#define CPUTAG_54XX "54XX"

ADDRESS_MAP_EXTERN( namco_54xx_map_program );
ADDRESS_MAP_EXTERN( namco_54xx_map_data );
ADDRESS_MAP_EXTERN( namco_54xx_map_io );

void namco_54xx_write(UINT8 data);

/* discrete nodes */
#define NAMCO_54XX_0_DATA		NODE_01
#define NAMCO_54XX_1_DATA		NODE_02
#define NAMCO_54XX_2_DATA		NODE_03
#define NAMCO_52XX_P_DATA		NODE_04

#endif	/* NAMCO54_H */
