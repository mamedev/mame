/*********************************************************************

    mc146818.h

    Implementation of the MC146818 chip

    Real time clock chip with batteru buffered ram
    Used in IBM PC/AT, several PC clones, Amstrad NC200

    Peter Trauner (peter.trauner@jk.uni-linz.ac.at)

*********************************************************************/

#ifndef MC146818_H
#define MC146818_H

typedef enum
{
	MC146818_STANDARD,
	MC146818_IGNORE_CENTURY, // century is NOT set, for systems having other usage of this byte
	MC146818_ENHANCED
} MC146818_TYPE;



/* initialize mc146818 emulation, call only once at beginning */
void mc146818_init(running_machine *machine, MC146818_TYPE type);

/* loads data from standard nvram file */
void mc146818_load(running_machine *machine);

/* loads data from file stream */
void mc146818_load_stream(mame_file *file);

/* saves data into standard nvram file */
void mc146818_save(running_machine *machine);

/* saves data into file stream */
void mc146818_save_stream(mame_file *file);

NVRAM_HANDLER( mc146818 );

READ8_HANDLER(mc146818_port_r);
WRITE8_HANDLER(mc146818_port_w);

READ16_HANDLER(mc146818_port16le_r);
WRITE16_HANDLER(mc146818_port16le_w);

READ32_HANDLER(mc146818_port32le_r);
WRITE32_HANDLER(mc146818_port32le_w);

READ64_HANDLER(mc146818_port64be_r);
WRITE64_HANDLER(mc146818_port64be_w);

#endif /* MC146818_H */
