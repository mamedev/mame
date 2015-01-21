/* naomicrypt.h */

// use internal M1 key tables, or external files (0 = external files)
#define USE_NAOMICRYPT 0

UINT32 get_naomi_key(running_machine &machine);

#define _NAOMI_M1_KEYFILE(name,hash) \
	ROM_REGION( 4, "rom_key", 0 ) \
	ROM_LOAD( name, 0, 4, hash )


#define _NAOMI_M1_KEYFILE_UNUSED \
	ROM_REGION( 4, "rom_key", ROMREGION_ERASE00 ) \

