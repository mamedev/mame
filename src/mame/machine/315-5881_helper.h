
// use the internal key list, or external files?
#define USE_315_5881_HELPER 1

INT64 get_315_5881_key(running_machine &machine);

#if USE_315_5881_HELPER

#define _315_5881_KEYFILE(name,hash) \
	/* nothing */

#define _315_5881_UNUSED_OR_UNKNOWN \
	/* nothing */

#define _315_5881_UNUSED \
	/* nothing */


#else

#define _315_5881_KEYFILE(name,hash) \
	ROM_REGION( 4, "315_5881key", 0 ) \
	ROM_LOAD( name, 0, 4, hash )

#define _315_5881_UNUSED_OR_UNKNOWN \
	ROM_REGION( 4, "315_5881key", ROMREGION_ERASE00 )

#define _315_5881_UNUSED \
	ROM_REGION( 4, "315_5881key", ROMREGION_ERASE00 )



#endif