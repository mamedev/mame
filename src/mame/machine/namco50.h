#ifndef NAMCO50_H
#define NAMCO50_H

#define CPUTAG_50XX "50XX"
#define CPUTAG_50XX_2 "50XX_2"

ADDRESS_MAP_EXTERN( namco_50xx_map_program, 8 );
ADDRESS_MAP_EXTERN( namco_50xx_map_data, 8 );
ADDRESS_MAP_EXTERN( namco_50xx_map_io, 8 );

ADDRESS_MAP_EXTERN( namco_50xx_2_map_program, 8 );
ADDRESS_MAP_EXTERN( namco_50xx_2_map_data, 8 );
ADDRESS_MAP_EXTERN( namco_50xx_2_map_io, 8 );

UINT8 namco_50xx_read(running_machine *machine);
void namco_50xx_read_request(running_machine *machine);
void namco_50xx_write(running_machine *machine, UINT8 data);

UINT8 namco_50xx_2_read(running_machine *machine);
void namco_50xx_2_read_request(running_machine *machine);
void namco_50xx_2_write(running_machine *machine, UINT8 data);

#endif	/* NAMCO54_H */
