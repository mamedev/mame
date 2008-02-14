#ifndef NAMCO50_H
#define NAMCO50_H

#define CPUTAG_50XX "50XX"
#define CPUTAG_50XX_2 "50XX_2"

ADDRESS_MAP_EXTERN( namco_50xx_map_program );
ADDRESS_MAP_EXTERN( namco_50xx_map_data );
ADDRESS_MAP_EXTERN( namco_50xx_map_io );

ADDRESS_MAP_EXTERN( namco_50xx_2_map_program );
ADDRESS_MAP_EXTERN( namco_50xx_2_map_data );
ADDRESS_MAP_EXTERN( namco_50xx_2_map_io );

UINT8 namco_50xx_read(void);
void namco_50xx_read_request(void);
void namco_50xx_write(UINT8 data);

UINT8 namco_50xx_2_read(void);
void namco_50xx_2_read_request(void);
void namco_50xx_2_write(UINT8 data);

#endif	/* NAMCO54_H */
