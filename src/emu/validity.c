/***************************************************************************

    validity.c

    Validity checks on internal data structures.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "hash.h"
#include "validity.h"

#include <ctype.h>


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define REPORT_TIMES				(0)



/***************************************************************************
    COMPILE-TIME VALIDATION
***************************************************************************/

/* if the following lines error during compile, your PTR64 switch is set incorrectly in the makefile */
#ifdef PTR64
UINT8 your_ptr64_flag_is_wrong[(int)(sizeof(void *) - 7)];
#else
UINT8 your_ptr64_flag_is_wrong[(int)(5 - sizeof(void *))];
#endif



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef tagmap_t<const game_driver *> game_driver_map;

typedef tagmap_t<FPTR> int_map;

class region_entry
{
public:
	region_entry()
		: length(0) { }

	astring tag;
	UINT32 length;
};


class region_array
{
public:
	region_entry entries[256];
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    input_port_string_from_index - return an
    indexed string from the input port system
-------------------------------------------------*/

INLINE const char *input_port_string_from_index(UINT32 index)
{
	input_port_token token;
	token.i = index;
	return input_port_string_from_token(token);
}


/*-------------------------------------------------
    validate_tag - ensure that the given tag
    meets the general requirements
-------------------------------------------------*/

bool validate_tag(const game_driver *driver, const char *object, const char *tag)
{
	const char *validchars = "abcdefghijklmnopqrstuvwxyz0123456789_.:";
	const char *begin = strrchr(tag, ':');
	const char *p;
	bool error = false;

	/* some common names that are now deprecated */
	if (strcmp(tag, "main") == 0 ||
		strcmp(tag, "audio") == 0 ||
		strcmp(tag, "sound") == 0 ||
		strcmp(tag, "left") == 0 ||
		strcmp(tag, "right") == 0)
	{
		mame_printf_error("%s: %s has invalid generic tag '%s'\n", driver->source_file, driver->name, tag);
		error = true;
	}

	for (p = tag; *p != 0; p++)
	{
		if (*p != tolower((UINT8)*p))
		{
			mame_printf_error("%s: %s has %s with tag '%s' containing upper-case characters\n", driver->source_file, driver->name, object, tag);
			error = true;
			break;
		}
		if (*p == ' ')
		{
			mame_printf_error("%s: %s has %s with tag '%s' containing spaces\n", driver->source_file, driver->name, object, tag);
			error = true;
			break;
		}
		if (strchr(validchars, *p) == NULL)
		{
			mame_printf_error("%s: %s has %s with tag '%s' containing invalid character '%c'\n", driver->source_file, driver->name, object, tag, *p);
			error = true;
			break;
		}
	}

	if (begin == NULL)
		begin = tag;
	else
		begin += 1;

	if (strlen(begin) == 0)
	{
		mame_printf_error("%s: %s has %s with 0-length tag\n", driver->source_file, driver->name, object);
		error = true;
	}
	if (strlen(begin) < MIN_TAG_LENGTH)
	{
		mame_printf_error("%s: %s has %s with tag '%s' < %d characters\n", driver->source_file, driver->name, object, tag, MIN_TAG_LENGTH);
		error = true;
	}
	if (strlen(begin) > MAX_TAG_LENGTH)
	{
		mame_printf_error("%s: %s has %s with tag '%s' > %d characters\n", driver->source_file, driver->name, object, tag, MAX_TAG_LENGTH);
		error = true;
	}

	return !error;
}



/***************************************************************************
    VALIDATION FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    validate_inlines - validate inline function
    behaviors
-------------------------------------------------*/

static bool validate_inlines(void)
{
#undef rand
	volatile UINT64 testu64a = rand() ^ (rand() << 15) ^ ((UINT64)rand() << 30) ^ ((UINT64)rand() << 45);
	volatile INT64 testi64a = rand() ^ (rand() << 15) ^ ((INT64)rand() << 30) ^ ((INT64)rand() << 45);
#ifdef PTR64
	volatile INT64 testi64b = rand() ^ (rand() << 15) ^ ((INT64)rand() << 30) ^ ((INT64)rand() << 45);
#endif
	volatile UINT32 testu32a = rand() ^ (rand() << 15);
	volatile UINT32 testu32b = rand() ^ (rand() << 15);
	volatile INT32 testi32a = rand() ^ (rand() << 15);
	volatile INT32 testi32b = rand() ^ (rand() << 15);
	INT32 resulti32, expectedi32;
	UINT32 resultu32, expectedu32;
	INT64 resulti64, expectedi64;
	UINT64 resultu64, expectedu64;
	INT32 remainder, expremainder;
	UINT32 uremainder, expuremainder, bigu32 = 0xffffffff;
	bool error = false;

	/* use only non-zero, positive numbers */
	if (testu64a == 0) testu64a++;
	if (testi64a == 0) testi64a++;
	else if (testi64a < 0) testi64a = -testi64a;
#ifdef PTR64
	if (testi64b == 0) testi64b++;
	else if (testi64b < 0) testi64b = -testi64b;
#endif
	if (testu32a == 0) testu32a++;
	if (testu32b == 0) testu32b++;
	if (testi32a == 0) testi32a++;
	else if (testi32a < 0) testi32a = -testi32a;
	if (testi32b == 0) testi32b++;
	else if (testi32b < 0) testi32b = -testi32b;

	resulti64 = mul_32x32(testi32a, testi32b);
	expectedi64 = (INT64)testi32a * (INT64)testi32b;
	if (resulti64 != expectedi64)
		{ mame_printf_error("Error testing mul_32x32 (%08X x %08X) = %08X%08X (expected %08X%08X)\n", testi32a, testi32b, (UINT32)(resulti64 >> 32), (UINT32)resulti64, (UINT32)(expectedi64 >> 32), (UINT32)expectedi64); error = true; }

	resultu64 = mulu_32x32(testu32a, testu32b);
	expectedu64 = (UINT64)testu32a * (UINT64)testu32b;
	if (resultu64 != expectedu64)
		{ mame_printf_error("Error testing mulu_32x32 (%08X x %08X) = %08X%08X (expected %08X%08X)\n", testu32a, testu32b, (UINT32)(resultu64 >> 32), (UINT32)resultu64, (UINT32)(expectedu64 >> 32), (UINT32)expectedu64); error = true; }

	resulti32 = mul_32x32_hi(testi32a, testi32b);
	expectedi32 = ((INT64)testi32a * (INT64)testi32b) >> 32;
	if (resulti32 != expectedi32)
		{ mame_printf_error("Error testing mul_32x32_hi (%08X x %08X) = %08X (expected %08X)\n", testi32a, testi32b, resulti32, expectedi32); error = true; }

	resultu32 = mulu_32x32_hi(testu32a, testu32b);
	expectedu32 = ((INT64)testu32a * (INT64)testu32b) >> 32;
	if (resultu32 != expectedu32)
		{ mame_printf_error("Error testing mulu_32x32_hi (%08X x %08X) = %08X (expected %08X)\n", testu32a, testu32b, resultu32, expectedu32); error = true; }

	resulti32 = mul_32x32_shift(testi32a, testi32b, 7);
	expectedi32 = ((INT64)testi32a * (INT64)testi32b) >> 7;
	if (resulti32 != expectedi32)
		{ mame_printf_error("Error testing mul_32x32_shift (%08X x %08X) >> 7 = %08X (expected %08X)\n", testi32a, testi32b, resulti32, expectedi32); error = true; }

	resultu32 = mulu_32x32_shift(testu32a, testu32b, 7);
	expectedu32 = ((INT64)testu32a * (INT64)testu32b) >> 7;
	if (resultu32 != expectedu32)
		{ mame_printf_error("Error testing mulu_32x32_shift (%08X x %08X) >> 7 = %08X (expected %08X)\n", testu32a, testu32b, resultu32, expectedu32); error = true; }

	while ((INT64)testi32a * (INT64)0x7fffffff < testi64a)
		testi64a /= 2;
	while ((UINT64)testu32a * (UINT64)bigu32 < testu64a)
		testu64a /= 2;

	resulti32 = div_64x32(testi64a, testi32a);
	expectedi32 = testi64a / (INT64)testi32a;
	if (resulti32 != expectedi32)
		{ mame_printf_error("Error testing div_64x32 (%08X%08X / %08X) = %08X (expected %08X)\n", (UINT32)(testi64a >> 32), (UINT32)testi64a, testi32a, resulti32, expectedi32); error = true; }

	resultu32 = divu_64x32(testu64a, testu32a);
	expectedu32 = testu64a / (UINT64)testu32a;
	if (resultu32 != expectedu32)
		{ mame_printf_error("Error testing divu_64x32 (%08X%08X / %08X) = %08X (expected %08X)\n", (UINT32)(testu64a >> 32), (UINT32)testu64a, testu32a, resultu32, expectedu32); error = true; }

	resulti32 = div_64x32_rem(testi64a, testi32a, &remainder);
	expectedi32 = testi64a / (INT64)testi32a;
	expremainder = testi64a % (INT64)testi32a;
	if (resulti32 != expectedi32 || remainder != expremainder)
		{ mame_printf_error("Error testing div_64x32_rem (%08X%08X / %08X) = %08X,%08X (expected %08X,%08X)\n", (UINT32)(testi64a >> 32), (UINT32)testi64a, testi32a, resulti32, remainder, expectedi32, expremainder); error = true; }

	resultu32 = divu_64x32_rem(testu64a, testu32a, &uremainder);
	expectedu32 = testu64a / (UINT64)testu32a;
	expuremainder = testu64a % (UINT64)testu32a;
	if (resultu32 != expectedu32 || uremainder != expuremainder)
		{ mame_printf_error("Error testing divu_64x32_rem (%08X%08X / %08X) = %08X,%08X (expected %08X,%08X)\n", (UINT32)(testu64a >> 32), (UINT32)testu64a, testu32a, resultu32, uremainder, expectedu32, expuremainder); error = true; }

	resulti32 = mod_64x32(testi64a, testi32a);
	expectedi32 = testi64a % (INT64)testi32a;
	if (resulti32 != expectedi32)
		{ mame_printf_error("Error testing mod_64x32 (%08X%08X / %08X) = %08X (expected %08X)\n", (UINT32)(testi64a >> 32), (UINT32)testi64a, testi32a, resulti32, expectedi32); error = true; }

	resultu32 = modu_64x32(testu64a, testu32a);
	expectedu32 = testu64a % (UINT64)testu32a;
	if (resultu32 != expectedu32)
		{ mame_printf_error("Error testing modu_64x32 (%08X%08X / %08X) = %08X (expected %08X)\n", (UINT32)(testu64a >> 32), (UINT32)testu64a, testu32a, resultu32, expectedu32); error = true; }

	while ((INT64)testi32a * (INT64)0x7fffffff < ((INT32)testi64a << 3))
		testi64a /= 2;
	while ((UINT64)testu32a * (UINT64)0xffffffff < ((UINT32)testu64a << 3))
		testu64a /= 2;

	resulti32 = div_32x32_shift((INT32)testi64a, testi32a, 3);
	expectedi32 = ((INT64)(INT32)testi64a << 3) / (INT64)testi32a;
	if (resulti32 != expectedi32)
		{ mame_printf_error("Error testing div_32x32_shift (%08X << 3) / %08X = %08X (expected %08X)\n", (INT32)testi64a, testi32a, resulti32, expectedi32); error = true; }

	resultu32 = divu_32x32_shift((UINT32)testu64a, testu32a, 3);
	expectedu32 = ((UINT64)(UINT32)testu64a << 3) / (UINT64)testu32a;
	if (resultu32 != expectedu32)
		{ mame_printf_error("Error testing divu_32x32_shift (%08X << 3) / %08X = %08X (expected %08X)\n", (UINT32)testu64a, testu32a, resultu32, expectedu32); error = true; }

	if (fabs(recip_approx(100.0) - 0.01) > 0.0001)
		{ mame_printf_error("Error testing recip_approx\n"); error = true; }

	testi32a = (testi32a & 0x0000ffff) | 0x400000;
	if (count_leading_zeros(testi32a) != 9)
		{ mame_printf_error("Error testing count_leading_zeros\n"); error = true; }
	testi32a = (testi32a | 0xffff0000) & ~0x400000;
	if (count_leading_ones(testi32a) != 9)
		{ mame_printf_error("Error testing count_leading_ones\n"); error = true; }

	testi32b = testi32a;
	if (compare_exchange32(&testi32a, testi32b, 1000) != testi32b || testi32a != 1000)
		{ mame_printf_error("Error testing compare_exchange32\n"); error = true; }
#ifdef PTR64
	testi64b = testi64a;
	if (compare_exchange64(&testi64a, testi64b, 1000) != testi64b || testi64a != 1000)
		{ mame_printf_error("Error testing compare_exchange64\n"); error = true; }
#endif
	if (atomic_exchange32(&testi32a, testi32b) != 1000)
		{ mame_printf_error("Error testing atomic_exchange32\n"); error = true; }
	if (atomic_add32(&testi32a, 45) != testi32b + 45)
		{ mame_printf_error("Error testing atomic_add32\n"); error = true; }
	if (atomic_increment32(&testi32a) != testi32b + 46)
		{ mame_printf_error("Error testing atomic_increment32\n"); error = true; }
	if (atomic_decrement32(&testi32a) != testi32b + 45)
		{ mame_printf_error("Error testing atomic_decrement32\n"); error = true; }

	return error;
}


/*-------------------------------------------------
    validate_driver - validate basic driver
    information
-------------------------------------------------*/

static bool validate_driver(int drivnum, const machine_config *config, game_driver_map &names, game_driver_map &descriptions)
{
	const game_driver *driver = drivers[drivnum];
	const game_driver *clone_of;
	const char *compatible_with;
	const game_driver *other_drv;
	bool error = FALSE, is_clone;
	const char *s;

	enum { NAME_LEN_PARENT = 8, NAME_LEN_CLONE = 16 };

	/* check for duplicate names */
	if (names.add(driver->name, driver, FALSE) == TMERR_DUPLICATE)
	{
		const game_driver *match = names.find(driver->name);
		mame_printf_error("%s: %s is a duplicate name (%s, %s)\n", driver->source_file, driver->name, match->source_file, match->name);
		error = true;
	}

	/* check for duplicate descriptions */
	if (descriptions.add(driver->description, driver, FALSE) == TMERR_DUPLICATE)
	{
		const game_driver *match = descriptions.find(driver->description);
		mame_printf_error("%s: %s is a duplicate description (%s, %s)\n", driver->source_file, driver->description, match->source_file, match->description);
		error = true;
	}

	/* determine the clone */
	is_clone = (strcmp(driver->parent, "0") != 0);
	clone_of = driver_get_clone(driver);
	if (clone_of && (clone_of->flags & GAME_IS_BIOS_ROOT))
		is_clone = false;

	/* if we have at least 100 drivers, validate the clone */
	/* (100 is arbitrary, but tries to avoid tiny.mak dependencies) */
	if (driver_list_get_count(drivers) > 100 && !clone_of && is_clone)
	{
		mame_printf_error("%s: %s is a non-existant clone\n", driver->source_file, driver->parent);
		error = true;
	}

	/* look for recursive cloning */
	if (clone_of == driver)
	{
		mame_printf_error("%s: %s is set as a clone of itself\n", driver->source_file, driver->name);
		error = true;
	}

	/* look for clones that are too deep */
	if (clone_of != NULL && (clone_of = driver_get_clone(clone_of)) != NULL && (clone_of->flags & GAME_IS_BIOS_ROOT) == 0)
	{
		mame_printf_error("%s: %s is a clone of a clone\n", driver->source_file, driver->name);
		error = true;
	}

	/* make sure the driver name is 8 chars or less */
	if ((is_clone && strlen(driver->name) > NAME_LEN_CLONE) || ((!is_clone) && strlen(driver->name) > NAME_LEN_PARENT))
	{
		mame_printf_error("%s: %s %s driver name must be %d characters or less\n", driver->source_file, driver->name,
						  is_clone ? "clone" : "parent", is_clone ? NAME_LEN_CLONE : NAME_LEN_PARENT);
		error = true;
	}

	/* make sure the year is only digits, '?' or '+' */
	for (s = driver->year; *s; s++)
		if (!isdigit((UINT8)*s) && *s != '?' && *s != '+')
		{
			mame_printf_error("%s: %s has an invalid year '%s'\n", driver->source_file, driver->name, driver->year);
			error = true;
			break;
		}

	/* normalize driver->compatible_with */
	compatible_with = driver->compatible_with;
	if ((compatible_with != NULL) && !strcmp(compatible_with, "0"))
		compatible_with = NULL;

	/* check for this driver being compatible with a non-existant driver */
	if ((compatible_with != NULL) && (driver_get_name(driver->compatible_with) == NULL))
	{
		mame_printf_error("%s: is compatible with %s, which is not in drivers[]\n", driver->name, driver->compatible_with);
		error = true;
	}

	/* check for clone_of and compatible_with being specified at the same time */
	if ((driver_get_clone(driver) != NULL) && (compatible_with != NULL))
	{
		mame_printf_error("%s: both compatible_with and clone_of are specified\n", driver->name);
		error = true;
	}

	/* find any recursive dependencies on the current driver */
	for (other_drv = driver_get_compatible(driver); other_drv != NULL; other_drv = driver_get_compatible(other_drv))
	{
		if (driver == other_drv)
		{
			mame_printf_error("%s: recursive compatibility\n", driver->name);
			error = true;
			break;
		}
	}

	/* make sure sound-less drivers are flagged */
	const device_config_sound_interface *sound;
	if ((driver->flags & GAME_IS_BIOS_ROOT) == 0 && !config->m_devicelist.first(sound) && (driver->flags & GAME_NO_SOUND) == 0 && (driver->flags & GAME_NO_SOUND_HW) == 0)
	{
		mame_printf_error("%s: %s missing GAME_NO_SOUND flag\n", driver->source_file, driver->name);
		error = true;
	}

	return error;
}


/*-------------------------------------------------
    validate_roms - validate ROM definitions
-------------------------------------------------*/

static bool validate_roms(int drivnum, const machine_config *config, region_array *rgninfo, game_driver_map &roms)
{
	const game_driver *driver = drivers[drivnum];
	int bios_flags = 0, last_bios = 0;
	const char *last_rgnname = "???";
	const char *last_name = "???";
	region_entry *currgn = NULL;
	int items_since_region = 1;
	bool error = false;

	/* check for duplicate ROM entries */
/*
    if (driver->rom != NULL && (driver->flags & GAME_NO_STANDALONE) == 0)
    {
        char romaddr[20];
        sprintf(romaddr, "%p", driver->rom);
        if (roms.add(romaddr, driver, FALSE) == TMERR_DUPLICATE)
        {
            const game_driver *match = roms.find(romaddr);
            mame_printf_error("%s: %s uses the same ROM set as (%s, %s)\n", driver->source_file, driver->description, match->source_file, match->name);
            error = true;
        }
    }
*/
	/* iterate, starting with the driver's ROMs and continuing with device ROMs */
	for (const rom_source *source = rom_first_source(driver, config); source != NULL; source = rom_next_source(driver, config, source))
	{
		/* scan the ROM entries */
		for (const rom_entry *romp = rom_first_region(driver, source); !ROMENTRY_ISEND(romp); romp++)
		{
			/* if this is a region, make sure it's valid, and record the length */
			if (ROMENTRY_ISREGION(romp))
			{
				const char *regiontag = ROMREGION_GETTAG(romp);

				/* if we haven't seen any items since the last region, print a warning */
				if (items_since_region == 0)
					mame_printf_warning("%s: %s has empty ROM region '%s' (warning)\n", driver->source_file, driver->name, last_rgnname);
				items_since_region = (ROMREGION_ISERASE(romp) || ROMREGION_ISDISKDATA(romp)) ? 1 : 0;
				currgn = NULL;
				last_rgnname = regiontag;

				/* check for a valid tag */
				if (regiontag == NULL)
				{
					mame_printf_error("%s: %s has NULL ROM_REGION tag\n", driver->source_file, driver->name);
					error = true;
				}

				/* load by name entries must be 8 characters or less */
				else if (ROMREGION_ISLOADBYNAME(romp) && strlen(regiontag) > 8)
				{
					mame_printf_error("%s: %s has load-by-name region '%s' with name >8 characters\n", driver->source_file, driver->name, regiontag);
					error = true;
				}

				/* find any empty entry, checking for duplicates */
				else
				{
					astring fulltag;

					/* iterate over all regions found so far */
					rom_region_name(fulltag, driver, source, romp);
					for (int rgnnum = 0; rgnnum < ARRAY_LENGTH(rgninfo->entries); rgnnum++)
					{
						/* stop when we hit an empty */
						if (!rgninfo->entries[rgnnum].tag)
						{
							currgn = &rgninfo->entries[rgnnum];
							currgn->tag = fulltag;
							currgn->length = ROMREGION_GETLENGTH(romp);
							break;
						}

						/* fail if we hit a duplicate */
						if (fulltag == rgninfo->entries[rgnnum].tag)
						{
							mame_printf_error("%s: %s has duplicate ROM_REGION tag '%s'\n", driver->source_file, driver->name, fulltag.cstr());
							error = true;
							break;
						}
					}
				}

				/* validate the region tag */
				if (!validate_tag(driver, "region", regiontag))
					error = true;
			}

			/* If this is a system bios, make sure it is using the next available bios number */
			else if (ROMENTRY_ISSYSTEM_BIOS(romp))
			{
				bios_flags = ROM_GETBIOSFLAGS(romp);
				if (last_bios+1 != bios_flags)
				{
					const char *name = ROM_GETNAME(romp);
					mame_printf_error("%s: %s has non-sequential bios %s\n", driver->source_file, driver->name, name);
					error = true;
				}
				last_bios = bios_flags;
			}

			/* if this is a file, make sure it is properly formatted */
			else if (ROMENTRY_ISFILE(romp))
			{
				const char *hash;
				const char *s;

				items_since_region++;

				/* track the last filename we found */
				last_name = ROM_GETNAME(romp);

				/* make sure it's all lowercase */
				for (s = last_name; *s; s++)
					if (tolower((UINT8)*s) != *s)
					{
						mame_printf_error("%s: %s has upper case ROM name %s\n", driver->source_file, driver->name, last_name);
						error = true;
						break;
					}

				/* make sure the hash is valid */
				hash = ROM_GETHASHDATA(romp);
				if (!hash_verify_string(hash))
				{
					mame_printf_error("%s: rom '%s' has an invalid hash string '%s'\n", driver->name, last_name, hash);
					error = true;
				}
			}

			// count copies/fills as valid items
			else if (ROMENTRY_ISCOPY(romp) || ROMENTRY_ISFILL(romp))
				items_since_region++;

			/* for any non-region ending entries, make sure they don't extend past the end */
			if (!ROMENTRY_ISREGIONEND(romp) && currgn != NULL)
			{
				items_since_region++;

				if (ROM_GETOFFSET(romp) + ROM_GETLENGTH(romp) > currgn->length)
				{
					mame_printf_error("%s: %s has ROM %s extending past the defined memory region\n", driver->source_file, driver->name, last_name);
					error = true;
				}
			}
		}

		/* final check for empty regions */
		if (items_since_region == 0)
			mame_printf_warning("%s: %s has empty ROM region (warning)\n", driver->source_file, driver->name);
	}

	return error;
}


/*-------------------------------------------------
    validate_display - validate display
    configurations
-------------------------------------------------*/

static bool validate_display(int drivnum, const machine_config *config)
{
	const game_driver *driver = drivers[drivnum];
	bool palette_modes = false;
	bool error = false;

	for (const screen_device_config *scrconfig = screen_first(*config); scrconfig != NULL; scrconfig = screen_next(scrconfig))
		if (scrconfig->format() == BITMAP_FORMAT_INDEXED16)
			palette_modes = true;

	/* check for empty palette */
	if (palette_modes && config->m_total_colors == 0)
	{
		mame_printf_error("%s: %s has zero palette entries\n", driver->source_file, driver->name);
		error = true;
	}

	return error;
}


/*-------------------------------------------------
    validate_gfx - validate graphics decoding
    configuration
-------------------------------------------------*/

static bool validate_gfx(int drivnum, const machine_config *config, region_array *rgninfo)
{
	const game_driver *driver = drivers[drivnum];
	bool error = false;
	int gfxnum;

	/* bail if no gfx */
	if (!config->m_gfxdecodeinfo)
		return false;

	/* iterate over graphics decoding entries */
	for (gfxnum = 0; gfxnum < MAX_GFX_ELEMENTS && config->m_gfxdecodeinfo[gfxnum].gfxlayout != NULL; gfxnum++)
	{
		const gfx_decode_entry *gfx = &config->m_gfxdecodeinfo[gfxnum];
		const char *region = gfx->memory_region;
		int xscale = (config->m_gfxdecodeinfo[gfxnum].xscale == 0) ? 1 : config->m_gfxdecodeinfo[gfxnum].xscale;
		int yscale = (config->m_gfxdecodeinfo[gfxnum].yscale == 0) ? 1 : config->m_gfxdecodeinfo[gfxnum].yscale;
		const gfx_layout *gl = gfx->gfxlayout;
		int israw = (gl->planeoffset[0] == GFX_RAW);
		int planes = gl->planes;
		UINT16 width = gl->width;
		UINT16 height = gl->height;
		UINT32 total = gl->total;

		/* make sure the region exists */
		if (region != NULL)
		{
			int rgnnum;

			/* loop over gfx regions */
			for (rgnnum = 0; rgnnum < ARRAY_LENGTH(rgninfo->entries); rgnnum++)
			{
				/* stop if we hit an empty */
				if (!rgninfo->entries[rgnnum].tag)
				{
					mame_printf_error("%s: %s has gfx[%d] referencing non-existent region '%s'\n", driver->source_file, driver->name, gfxnum, region);
					error = true;
					break;
				}

				/* if we hit a match, check against the length */
				if (rgninfo->entries[rgnnum].tag == region)
				{
					/* if we have a valid region, and we're not using auto-sizing, check the decode against the region length */
					if (!IS_FRAC(total))
					{
						int len, avail, plane, start;
						UINT32 charincrement = gl->charincrement;
						const UINT32 *poffset = gl->planeoffset;

						/* determine which plane is the largest */
						start = 0;
						for (plane = 0; plane < planes; plane++)
							if (poffset[plane] > start)
								start = poffset[plane];
						start &= ~(charincrement - 1);

						/* determine the total length based on this info */
						len = total * charincrement;

						/* do we have enough space in the region to cover the whole decode? */
						avail = rgninfo->entries[rgnnum].length - (gfx->start & ~(charincrement/8-1));

						/* if not, this is an error */
						if ((start + len) / 8 > avail)
						{
							mame_printf_error("%s: %s has gfx[%d] extending past allocated memory of region '%s'\n", driver->source_file, driver->name, gfxnum, region);
							error = true;
						}
					}
					break;
				}
			}
		}

		if (israw)
		{
			if (total != RGN_FRAC(1,1))
			{
				mame_printf_error("%s: %s has gfx[%d] with unsupported layout total\n", driver->source_file, driver->name, gfxnum);
				error = true;
			}

			if (xscale != 1 || yscale != 1)
			{
				mame_printf_error("%s: %s has gfx[%d] with unsupported xscale/yscale\n", driver->source_file, driver->name, gfxnum);
				error = true;
			}
		}
		else
		{
			if (planes > MAX_GFX_PLANES)
			{
				mame_printf_error("%s: %s has gfx[%d] with invalid planes\n", driver->source_file, driver->name, gfxnum);
				error = true;
			}

			if (xscale * width > MAX_ABS_GFX_SIZE || yscale * height > MAX_ABS_GFX_SIZE)
			{
				mame_printf_error("%s: %s has gfx[%d] with invalid xscale/yscale\n", driver->source_file, driver->name, gfxnum);
				error = true;
			}
		}
	}

	return error;
}


/*-------------------------------------------------
    get_defstr_index - return the index of the
    string assuming it is one of the default
    strings
-------------------------------------------------*/

static int get_defstr_index(int_map &defstr_map, const char *name, const game_driver *driver, bool *error)
{
	/* check for strings that should be DEF_STR */
	int strindex = defstr_map.find(name);
	if (strindex != 0 && name != input_port_string_from_index(strindex) && error != NULL)
	{
		mame_printf_error("%s: %s must use DEF_STR( %s )\n", driver->source_file, driver->name, name);
		*error = true;
	}

	return strindex;
}


/*-------------------------------------------------
    validate_analog_input_field - validate an
    analog input field
-------------------------------------------------*/

static void validate_analog_input_field(const input_field_config *field, const game_driver *driver, bool *error)
{
	INT32 analog_max = field->max;
	INT32 analog_min = field->min;
	int shift;

	if (field->type == IPT_POSITIONAL || field->type == IPT_POSITIONAL_V)
	{
		for (shift = 0; (shift <= 31) && (~field->mask & (1 << shift)); shift++) ;
		/* convert the positional max value to be in the bitmask for testing */
		analog_max = (analog_max - 1) << shift;

		/* positional port size must fit in bits used */
		if (((field->mask >> shift) + 1) < field->max)
		{
			mame_printf_error("%s: %s has an analog port with a positional port size bigger then the mask size\n", driver->source_file, driver->name);
			*error = true;
		}
	}
	else
	{
		/* only positional controls use PORT_WRAPS */
		if (field->flags & ANALOG_FLAG_WRAPS)
		{
			mame_printf_error("%s: %s only positional analog ports use PORT_WRAPS\n", driver->source_file, driver->name);
			*error = true;
		}
	}

	/* analog ports must have a valid sensitivity */
	if (field->sensitivity == 0)
	{
		mame_printf_error("%s: %s has an analog port with zero sensitivity\n", driver->source_file, driver->name);
		*error = true;
	}

	/* check that the default falls in the bitmask range */
	if (field->defvalue & ~field->mask)
	{
		mame_printf_error("%s: %s has an analog port with a default value out of the bitmask range\n", driver->source_file, driver->name);
		*error = true;
	}

	/* tests for absolute devices */
	if (field->type >= __ipt_analog_absolute_start && field->type <= __ipt_analog_absolute_end)
	{
		INT32 default_value = field->defvalue;

		/* adjust for signed values */
		if (analog_min > analog_max)
		{
			analog_min = -analog_min;
			if (default_value > analog_max)
				default_value = -default_value;
		}

		/* check that the default falls in the MINMAX range */
		if (default_value < analog_min || default_value > analog_max)
		{
			mame_printf_error("%s: %s has an analog port with a default value out PORT_MINMAX range\n", driver->source_file, driver->name);
			*error = true;
		}

		/* check that the MINMAX falls in the bitmask range */
		/* we use the unadjusted min for testing */
		if (field->min & ~field->mask || analog_max & ~field->mask)
		{
			mame_printf_error("%s: %s has an analog port with a PORT_MINMAX value out of the bitmask range\n", driver->source_file, driver->name);
			*error = true;
		}

		/* absolute analog ports do not use PORT_RESET */
		if (field->flags & ANALOG_FLAG_RESET)
		{
			mame_printf_error("%s: %s - absolute analog ports do not use PORT_RESET\n", driver->source_file, driver->name);
			*error = true;
		}
	}

	/* tests for relative devices */
	else
	{
		/* tests for non IPT_POSITIONAL relative devices */
		if (field->type != IPT_POSITIONAL && field->type != IPT_POSITIONAL_V)
		{
			/* relative devices do not use PORT_MINMAX */
			if (field->min != 0 || field->max != field->mask)
			{
				mame_printf_error("%s: %s - relative ports do not use PORT_MINMAX\n", driver->source_file, driver->name);
				*error = true;
			}

			/* relative devices do not use a default value */
			/* the counter is at 0 on power up */
			if (field->defvalue != 0)
			{
				mame_printf_error("%s: %s - relative ports do not use a default value other then 0\n", driver->source_file, driver->name);
				*error = true;
			}
		}
	}
}


/*-------------------------------------------------
    validate_dip_settings - validate a DIP switch
    setting
-------------------------------------------------*/

static void validate_dip_settings(const input_field_config *field, const game_driver *driver, int_map &defstr_map, bool *error)
{
	const char *demo_sounds = input_port_string_from_index(INPUT_STRING_Demo_Sounds);
	const char *flipscreen = input_port_string_from_index(INPUT_STRING_Flip_Screen);
	UINT8 coin_list[INPUT_STRING_1C_9C + 1 - INPUT_STRING_9C_1C] = { 0 };
	const input_setting_config *setting;
	int coin_error = FALSE;

	/* iterate through the settings */
	for (setting = field->settinglist; setting != NULL; setting = setting->next)
	{
		int strindex = get_defstr_index(defstr_map, setting->name, driver, error);

		/* note any coinage strings */
		if (strindex >= INPUT_STRING_9C_1C && strindex <= INPUT_STRING_1C_9C)
			coin_list[strindex - INPUT_STRING_9C_1C] = 1;

		/* make sure demo sounds default to on */
		if (field->name == demo_sounds && strindex == INPUT_STRING_On && field->defvalue != setting->value)
		{
			mame_printf_error("%s: %s Demo Sounds must default to On\n", driver->source_file, driver->name);
			*error = true;
		}

		/* check for bad demo sounds options */
		if (field->name == demo_sounds && (strindex == INPUT_STRING_Yes || strindex == INPUT_STRING_No))
		{
			mame_printf_error("%s: %s has wrong Demo Sounds option %s (must be Off/On)\n", driver->source_file, driver->name, setting->name);
			*error = true;
		}

		/* check for bad flip screen options */
		if (field->name == flipscreen && (strindex == INPUT_STRING_Yes || strindex == INPUT_STRING_No))
		{
			mame_printf_error("%s: %s has wrong Flip Screen option %s (must be Off/On)\n", driver->source_file, driver->name, setting->name);
			*error = true;
		}

		/* if we have a neighbor, compare ourselves to him */
		if (setting->next != NULL)
		{
			int next_strindex = get_defstr_index(defstr_map, setting->next->name, driver, error);

			/* check for inverted off/on dispswitch order */
			if (strindex == INPUT_STRING_On && next_strindex == INPUT_STRING_Off)
			{
				mame_printf_error("%s: %s has inverted Off/On dipswitch order\n", driver->source_file, driver->name);
				*error = true;
			}

			/* check for inverted yes/no dispswitch order */
			else if (strindex == INPUT_STRING_Yes && next_strindex == INPUT_STRING_No)
			{
				mame_printf_error("%s: %s has inverted No/Yes dipswitch order\n", driver->source_file, driver->name);
				*error = true;
			}

			/* check for inverted upright/cocktail dispswitch order */
			else if (strindex == INPUT_STRING_Cocktail && next_strindex == INPUT_STRING_Upright)
			{
				mame_printf_error("%s: %s has inverted Upright/Cocktail dipswitch order\n", driver->source_file, driver->name);
				*error = true;
			}

			/* check for proper coin ordering */
			else if (strindex >= INPUT_STRING_9C_1C && strindex <= INPUT_STRING_1C_9C && next_strindex >= INPUT_STRING_9C_1C && next_strindex <= INPUT_STRING_1C_9C &&
					 strindex >= next_strindex && memcmp(&setting->condition, &setting->next->condition, sizeof(setting->condition)) == 0)
			{
				mame_printf_error("%s: %s has unsorted coinage %s > %s\n", driver->source_file, driver->name, setting->name, setting->next->name);
				coin_error = *error = true;
			}
		}
	}

	/* if we have a coin error, demonstrate the correct way */
	if (coin_error)
	{
		int entry;

		mame_printf_error("%s: %s proper coin sort order should be:\n", driver->source_file, driver->name);
		for (entry = 0; entry < ARRAY_LENGTH(coin_list); entry++)
			if (coin_list[entry])
				mame_printf_error("%s\n", input_port_string_from_index(INPUT_STRING_9C_1C + entry));
	}
}


/*-------------------------------------------------
    validate_inputs - validate input configuration
-------------------------------------------------*/

static bool validate_inputs(int drivnum, const machine_config *config, int_map &defstr_map, ioport_list &portlist)
{
	const input_port_config *scanport;
	const input_port_config *port;
	const input_field_config *field;
	const game_driver *driver = drivers[drivnum];
	int empty_string_found = FALSE;
	char errorbuf[1024];
	bool error = false;

	/* skip if no ports */
	if (driver->ipt == NULL)
		return FALSE;

	/* allocate the input ports */
	input_port_list_init(portlist, driver->ipt, errorbuf, sizeof(errorbuf), FALSE);
	if (errorbuf[0] != 0)
	{
		mame_printf_error("%s: %s has input port errors:\n%s\n", driver->source_file, driver->name, errorbuf);
		error = true;
	}

	/* check for duplicate tags */
	for (port = portlist.first(); port != NULL; port = port->next())
		if (port->tag != NULL)
			for (scanport = port->next(); scanport != NULL; scanport = scanport->next())
				if (scanport->tag != NULL && strcmp(port->tag, scanport->tag) == 0)
				{
					mame_printf_error("%s: %s has a duplicate input port tag '%s'\n", driver->source_file, driver->name, port->tag);
					error = true;
				}

	/* iterate over the results */
	for (port = portlist.first(); port != NULL; port = port->next())
		for (field = port->fieldlist; field != NULL; field = field->next)
		{
			const input_setting_config *setting;
			//int strindex = 0;

			/* verify analog inputs */
			if (input_type_is_analog(field->type))
				validate_analog_input_field(field, driver, &error);

			/* verify dip switches */
			if (field->type == IPT_DIPSWITCH)
			{
				/* dip switch fields must have a name */
				if (field->name == NULL)
				{
					mame_printf_error("%s: %s has a DIP switch name or setting with no name\n", driver->source_file, driver->name);
					error = true;
				}

				/* verify the settings list */
				validate_dip_settings(field, driver, defstr_map, &error);
			}

			/* look for invalid (0) types which should be mapped to IPT_OTHER */
			if (field->type == IPT_INVALID)
			{
				mame_printf_error("%s: %s has an input port with an invalid type (0); use IPT_OTHER instead\n", driver->source_file, driver->name);
				error = true;
			}

			/* verify names */
			if (field->name != NULL)
			{
				/* check for empty string */
				if (field->name[0] == 0 && !empty_string_found)
				{
					mame_printf_error("%s: %s has an input with an empty string\n", driver->source_file, driver->name);
					empty_string_found = error = true;
				}

				/* check for trailing spaces */
				if (field->name[0] != 0 && field->name[strlen(field->name) - 1] == ' ')
				{
					mame_printf_error("%s: %s input '%s' has trailing spaces\n", driver->source_file, driver->name, field->name);
					error = true;
				}

				/* check for invalid UTF-8 */
				if (!utf8_is_valid_string(field->name))
				{
					mame_printf_error("%s: %s input '%s' has invalid characters\n", driver->source_file, driver->name, field->name);
					error = true;
				}

				/* look up the string and print an error if default strings are not used */
				/*strindex = */get_defstr_index(defstr_map, field->name, driver, &error);
			}

			/* verify conditions on the field */
			if (field->condition.tag != NULL)
			{
				/* find a matching port */
				for (scanport = portlist.first(); scanport != NULL; scanport = scanport->next())
					if (scanport->tag != NULL && strcmp(field->condition.tag, scanport->tag) == 0)
						break;

				/* if none, error */
				if (scanport == NULL)
				{
					mame_printf_error("%s: %s has a condition referencing non-existent input port tag '%s'\n", driver->source_file, driver->name, field->condition.tag);
					error = true;
				}
			}

			/* verify conditions on the settings */
			for (setting = field->settinglist; setting != NULL; setting = setting->next)
				if (setting->condition.tag != NULL)
				{
					/* find a matching port */
					for (scanport = portlist.first(); scanport != NULL; scanport = scanport->next())
						if (scanport->tag != NULL && strcmp(setting->condition.tag, scanport->tag) == 0)
							break;

					/* if none, error */
					if (scanport == NULL)
					{
						mame_printf_error("%s: %s has a condition referencing non-existent input port tag '%s'\n", driver->source_file, driver->name, setting->condition.tag);
						error = true;
					}
				}
		}

	error = error || validate_natural_keyboard_statics();

	/* free the config */
	return error;
}


/*-------------------------------------------------
    validate_devices - run per-device validity
    checks
-------------------------------------------------*/

static bool validate_devices(int drivnum, const machine_config *config, const ioport_list &portlist, region_array *rgninfo)
{
	bool error = false;
	const game_driver *driver = drivers[drivnum];

	for (const device_config *devconfig = config->m_devicelist.first(); devconfig != NULL; devconfig = devconfig->next())
	{
		/* validate the device tag */
		if (!validate_tag(driver, devconfig->name(), devconfig->tag()))
			error = true;

		/* look for duplicates */
		for (const device_config *scanconfig = config->m_devicelist.first(); scanconfig != devconfig; scanconfig = scanconfig->next())
			if (strcmp(scanconfig->tag(), devconfig->tag()) == 0)
			{
				mame_printf_warning("%s: %s has multiple devices with the tag '%s'\n", driver->source_file, driver->name, devconfig->tag());
				break;
			}

		/* check for device-specific validity check */
		if (devconfig->validity_check(*driver))
			error = true;

	}
	return error;
}


/*-------------------------------------------------
    mame_validitychecks - master validity checker
-------------------------------------------------*/

bool mame_validitychecks(const game_driver *curdriver)
{
	osd_ticks_t prep = 0;
	osd_ticks_t expansion = 0;
	osd_ticks_t driver_checks = 0;
	osd_ticks_t rom_checks = 0;
	osd_ticks_t gfx_checks = 0;
	osd_ticks_t display_checks = 0;
	osd_ticks_t input_checks = 0;
	osd_ticks_t device_checks = 0;

	int drivnum, strnum;
	bool error = false;
	UINT16 lsbtest;
	UINT8 a, b;

	game_driver_map names;
	game_driver_map descriptions;
	game_driver_map roms;
	int_map defstr;

	/* basic system checks */
	a = 0xff;
	b = a + 1;
	if (b > a)	{ mame_printf_error("UINT8 must be 8 bits\n"); error = true; }

	if (sizeof(INT8)   != 1)	{ mame_printf_error("INT8 must be 8 bits\n"); error = true; }
	if (sizeof(UINT8)  != 1)	{ mame_printf_error("UINT8 must be 8 bits\n"); error = true; }
	if (sizeof(INT16)  != 2)	{ mame_printf_error("INT16 must be 16 bits\n"); error = true; }
	if (sizeof(UINT16) != 2)	{ mame_printf_error("UINT16 must be 16 bits\n"); error = true; }
	if (sizeof(INT32)  != 4)	{ mame_printf_error("INT32 must be 32 bits\n"); error = true; }
	if (sizeof(UINT32) != 4)	{ mame_printf_error("UINT32 must be 32 bits\n"); error = true; }
	if (sizeof(INT64)  != 8)	{ mame_printf_error("INT64 must be 64 bits\n"); error = true; }
	if (sizeof(UINT64) != 8)	{ mame_printf_error("UINT64 must be 64 bits\n"); error = true; }
#ifdef PTR64
	if (sizeof(void *) != 8)	{ mame_printf_error("PTR64 flag enabled, but was compiled for 32-bit target\n"); error = true; }
#else
	if (sizeof(void *) != 4)	{ mame_printf_error("PTR64 flag not enabled, but was compiled for 64-bit target\n"); error = true; }
#endif
	lsbtest = 0;
	*(UINT8 *)&lsbtest = 0xff;
#ifdef LSB_FIRST
	if (lsbtest == 0xff00)		{ mame_printf_error("LSB_FIRST specified, but running on a big-endian machine\n"); error = true; }
#else
	if (lsbtest == 0x00ff)		{ mame_printf_error("LSB_FIRST not specified, but running on a little-endian machine\n"); error = true; }
#endif

	/* validate inline function behavior */
	error = validate_inlines() || error;

	get_profile_ticks();

	/* pre-populate the defstr tagmap with all the default strings */
	prep -= get_profile_ticks();
	for (strnum = 1; strnum < INPUT_STRING_COUNT; strnum++)
	{
		const char *string = input_port_string_from_index(strnum);
		if (string != NULL)
			defstr.add(string, strnum, FALSE);
	}
	prep += get_profile_ticks();

	/* iterate over all drivers */
	for (drivnum = 0; drivers[drivnum]; drivnum++)
	{
		const game_driver *driver = drivers[drivnum];
		machine_config *config = NULL;
		ioport_list portlist;
		region_array rgninfo;

		/* non-debug builds only care about games in the same driver */
		if (curdriver != NULL && strcmp(curdriver->source_file, driver->source_file) != 0)
			continue;

		try
		{
			/* expand the machine driver */
			expansion -= get_profile_ticks();
			config = global_alloc(machine_config(driver->machine_config));
			expansion += get_profile_ticks();

			/* validate the driver entry */
			driver_checks -= get_profile_ticks();
			error = validate_driver(drivnum, config, names, descriptions) || error;
			driver_checks += get_profile_ticks();

			/* validate the ROM information */
			rom_checks -= get_profile_ticks();
			error = validate_roms(drivnum, config, &rgninfo, roms) || error;
			rom_checks += get_profile_ticks();

			/* validate input ports */
			input_checks -= get_profile_ticks();
			error = validate_inputs(drivnum, config, defstr, portlist) || error;
			input_checks += get_profile_ticks();

			/* validate the display */
			display_checks -= get_profile_ticks();
			error = validate_display(drivnum, config) || error;
			display_checks += get_profile_ticks();

			/* validate the graphics decoding */
			gfx_checks -= get_profile_ticks();
			error = validate_gfx(drivnum, config, &rgninfo) || error;
			gfx_checks += get_profile_ticks();

			/* validate devices */
			device_checks -= get_profile_ticks();
			error = validate_devices(drivnum, config, portlist, &rgninfo) || error;
			device_checks += get_profile_ticks();
		}
		catch (emu_fatalerror &err)
		{
			global_free(config);
			throw emu_fatalerror("Validating %s (%s): %s", driver->name, driver->source_file, err.string());
		}

		global_free(config);
	}

#if (REPORT_TIMES)
	mame_printf_info("Prep:      %8dm\n", (int)(prep / 1000000));
	mame_printf_info("Expansion: %8dm\n", (int)(expansion / 1000000));
	mame_printf_info("Driver:    %8dm\n", (int)(driver_checks / 1000000));
	mame_printf_info("ROM:       %8dm\n", (int)(rom_checks / 1000000));
	mame_printf_info("CPU:       %8dm\n", (int)(cpu_checks / 1000000));
	mame_printf_info("Display:   %8dm\n", (int)(display_checks / 1000000));
	mame_printf_info("Graphics:  %8dm\n", (int)(gfx_checks / 1000000));
	mame_printf_info("Input:     %8dm\n", (int)(input_checks / 1000000));
#endif

	return error;
}
