/***************************************************************************

    validity.c

    Validity checks on internal data structures.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include "validity.h"
#include "emuopts.h"
#include <ctype.h>


//**************************************************************************
//  COMPILE-TIME VALIDATION
//**************************************************************************

// if the following lines error during compile, your PTR64 switch is set incorrectly in the makefile
#ifdef PTR64
UINT8 your_ptr64_flag_is_wrong[(int)(sizeof(void *) - 7)];
#else
UINT8 your_ptr64_flag_is_wrong[(int)(5 - sizeof(void *))];
#endif



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  ioport_string_from_index - return an indexed
//  string from the I/O port system
//-------------------------------------------------

inline const char *validity_checker::ioport_string_from_index(UINT32 index)
{
	return ioport_configurer::string_from_token((const char *)(FPTR)index);
}


//-------------------------------------------------
//  get_defstr_index - return the index of the
//  string assuming it is one of the default
//  strings
//-------------------------------------------------

inline int validity_checker::get_defstr_index(const char *string, bool suppress_error)
{
	// check for strings that should be DEF_STR
	int strindex = m_defstr_map.find(string);
	if (!suppress_error && strindex != 0 && string != ioport_string_from_index(strindex))
		mame_printf_error("Must use DEF_STR( %s )\n", string);
	return strindex;
}


//-------------------------------------------------
//  validate_tag - ensure that the given tag
//  meets the general requirements
//-------------------------------------------------

void validity_checker::validate_tag(const char *tag)
{
	// some common names that are now deprecated
	if (strcmp(tag, "main") == 0 || strcmp(tag, "audio") == 0 || strcmp(tag, "sound") == 0 || strcmp(tag, "left") == 0 || strcmp(tag, "right") == 0)
		mame_printf_error("Invalid generic tag '%s' used\n", tag);

	// scan for invalid characters
	static const char *validchars = "abcdefghijklmnopqrstuvwxyz0123456789_.:^$";
	for (const char *p = tag; *p != 0; p++)
	{
		// only lower-case permitted
		if (*p != tolower((UINT8)*p))
		{
			mame_printf_error("Tag '%s' contains upper-case characters\n", tag);
			break;
		}
		if (*p == ' ')
		{
			mame_printf_error("Tag '%s' contains spaces\n", tag);
			break;
		}
		if (strchr(validchars, *p) == NULL)
		{
			mame_printf_error("Tag '%s' contains invalid character '%c'\n",  tag, *p);
			break;
		}
	}

	// find the start of the final tag
	const char *begin = strrchr(tag, ':');
	if (begin == NULL)
		begin = tag;
	else
		begin += 1;

	// 0-length = bad
	if (*begin == 0)
		mame_printf_error("Found 0-length tag\n");

	// too short/too long = bad
	if (strlen(begin) < MIN_TAG_LENGTH)
		mame_printf_error("Tag '%s' is too short (must be at least %d characters)\n", tag, MIN_TAG_LENGTH);
	if (strlen(begin) > MAX_TAG_LENGTH)
		mame_printf_error("Tag '%s' is too long (must be less than %d characters)\n", tag, MAX_TAG_LENGTH);
}



//**************************************************************************
//  VALIDATION FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  validity_checker - constructor
//-------------------------------------------------

validity_checker::validity_checker(emu_options &options)
	: m_drivlist(options),
		m_errors(0),
		m_warnings(0),
		m_current_driver(NULL),
		m_current_config(NULL),
		m_current_device(NULL),
		m_current_ioport(NULL)
{
	// pre-populate the defstr map with all the default strings
	for (int strnum = 1; strnum < INPUT_STRING_COUNT; strnum++)
	{
		const char *string = ioport_string_from_index(strnum);
		if (string != NULL)
			m_defstr_map.add(string, strnum, false);
	}
}


//-------------------------------------------------
//  check_driver - check a single driver
//-------------------------------------------------

void validity_checker::check_driver(const game_driver &driver)
{
	// simply validate the one driver
	validate_begin();
	validate_one(driver);
	validate_end();
}


//-------------------------------------------------
//  check_shared_source - check all drivers that
//  share the same source file as the given driver
//-------------------------------------------------

void validity_checker::check_shared_source(const game_driver &driver)
{
	// initialize
	validate_begin();

	// then iterate over all drivers and check the ones that share the same source file
	m_drivlist.reset();
	while (m_drivlist.next())
		if (strcmp(driver.source_file, m_drivlist.driver().source_file) == 0)
			validate_one(m_drivlist.driver());

	// cleanup
	validate_end();
}


//-------------------------------------------------
//  check_all - check all drivers
//-------------------------------------------------

void validity_checker::check_all()
{
	// start by checking core stuff
	validate_begin();
	validate_core();
	validate_inlines();

	// then iterate over all drivers and check them
	m_drivlist.reset();
	while (m_drivlist.next())
		validate_one(m_drivlist.driver());

	// cleanup
	validate_end();
}


//-------------------------------------------------
//  validate_begin - prepare for validation by
//  taking over the output callbacks and resetting
//  our internal state
//-------------------------------------------------

void validity_checker::validate_begin()
{
	// take over error and warning outputs
	m_saved_error_output = mame_set_output_channel(OUTPUT_CHANNEL_ERROR, output_delegate(FUNC(validity_checker::error_output), this));
	m_saved_warning_output = mame_set_output_channel(OUTPUT_CHANNEL_WARNING, output_delegate(FUNC(validity_checker::warning_output), this));

	// reset all our maps
	m_names_map.reset();
	m_descriptions_map.reset();
	m_roms_map.reset();
	m_defstr_map.reset();
	m_region_map.reset();

	// reset internal state
	m_errors = 0;
	m_warnings = 0;

	// reset some special case state
	software_list_device::reset_checked_lists();
}


//-------------------------------------------------
//  validate_end - restore output callbacks and
//  clean up
//-------------------------------------------------

void validity_checker::validate_end()
{
	// restore the original output callbacks
	mame_set_output_channel(OUTPUT_CHANNEL_ERROR, m_saved_error_output);
	mame_set_output_channel(OUTPUT_CHANNEL_WARNING, m_saved_warning_output);
}


//-------------------------------------------------
//  validate_drivers - master validity checker
//-------------------------------------------------

void validity_checker::validate_one(const game_driver &driver)
{
	// set the current driver
	m_current_driver = &driver;
	m_current_config = NULL;
	m_current_device = NULL;
	m_current_ioport = NULL;
	m_region_map.reset();

	// reset error/warning state
	int start_errors = m_errors;
	int start_warnings = m_warnings;
	m_error_text.reset();
	m_warning_text.reset();

	// wrap in try/except to catch fatalerrors
	try
	{
		machine_config config(driver, m_drivlist.options());
		m_current_config = &config;
		validate_driver();
		validate_roms();
		validate_inputs();
		validate_display();
		validate_gfx();
		validate_devices();
	}
	catch (emu_fatalerror &err)
	{
		mame_printf_error("Fatal error %s", err.string());
	}
	m_current_config = NULL;

	// if we had warnings or errors, output
	if (m_errors > start_errors || m_warnings > start_warnings)
	{
		astring tempstr;
		output_via_delegate(m_saved_error_output, "Driver %s (file %s): %d errors, %d warnings\n", driver.name, core_filename_extract_base(tempstr, driver.source_file).cstr(), m_errors - start_errors, m_warnings - start_warnings);
		if (m_errors > start_errors)
		{
			m_error_text.replace("\n", "\n   ");
			output_via_delegate(m_saved_error_output, "Errors:\n   %s", m_error_text.cstr());
		}
		if (m_warnings > start_warnings)
		{
			m_warning_text.replace("\n", "\n   ");
			output_via_delegate(m_saved_error_output, "Warnings:\n   %s", m_warning_text.cstr());
		}
		output_via_delegate(m_saved_error_output, "\n");
	}

	// reset the driver/device
	m_current_driver = NULL;
	m_current_config = NULL;
	m_current_device = NULL;
	m_current_ioport = NULL;
}


//-------------------------------------------------
//  validate_core - validate core internal systems
//-------------------------------------------------

void validity_checker::validate_core()
{

	// basic system checks
	UINT8 a = 0xff;
	UINT8 b = a + 1;
	if (b > a) mame_printf_error("UINT8 must be 8 bits\n");

	// check size of core integer types
	if (sizeof(INT8)   != 1) mame_printf_error("INT8 must be 8 bits\n");
	if (sizeof(UINT8)  != 1) mame_printf_error("UINT8 must be 8 bits\n");
	if (sizeof(INT16)  != 2) mame_printf_error("INT16 must be 16 bits\n");
	if (sizeof(UINT16) != 2) mame_printf_error("UINT16 must be 16 bits\n");
	if (sizeof(INT32)  != 4) mame_printf_error("INT32 must be 32 bits\n");
	if (sizeof(UINT32) != 4) mame_printf_error("UINT32 must be 32 bits\n");
	if (sizeof(INT64)  != 8) mame_printf_error("INT64 must be 64 bits\n");
	if (sizeof(UINT64) != 8) mame_printf_error("UINT64 must be 64 bits\n");

	// check pointer size
#ifdef PTR64
	if (sizeof(void *) != 8) mame_printf_error("PTR64 flag enabled, but was compiled for 32-bit target\n");
#else
	if (sizeof(void *) != 4) mame_printf_error("PTR64 flag not enabled, but was compiled for 64-bit target\n");
#endif

	// check endianness definition
	UINT16 lsbtest = 0;
	*(UINT8 *)&lsbtest = 0xff;
#ifdef LSB_FIRST
	if (lsbtest == 0xff00) mame_printf_error("LSB_FIRST specified, but running on a big-endian machine\n");
#else
	if (lsbtest == 0x00ff) mame_printf_error("LSB_FIRST not specified, but running on a little-endian machine\n");
#endif
}


//-------------------------------------------------
//  validate_inlines - validate inline function
//  behaviors
//-------------------------------------------------

void validity_checker::validate_inlines()
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

	// use only non-zero, positive numbers
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
		mame_printf_error("Error testing mul_32x32 (%08X x %08X) = %08X%08X (expected %08X%08X)\n", testi32a, testi32b, (UINT32)(resulti64 >> 32), (UINT32)resulti64, (UINT32)(expectedi64 >> 32), (UINT32)expectedi64);

	resultu64 = mulu_32x32(testu32a, testu32b);
	expectedu64 = (UINT64)testu32a * (UINT64)testu32b;
	if (resultu64 != expectedu64)
		mame_printf_error("Error testing mulu_32x32 (%08X x %08X) = %08X%08X (expected %08X%08X)\n", testu32a, testu32b, (UINT32)(resultu64 >> 32), (UINT32)resultu64, (UINT32)(expectedu64 >> 32), (UINT32)expectedu64);

	resulti32 = mul_32x32_hi(testi32a, testi32b);
	expectedi32 = ((INT64)testi32a * (INT64)testi32b) >> 32;
	if (resulti32 != expectedi32)
		mame_printf_error("Error testing mul_32x32_hi (%08X x %08X) = %08X (expected %08X)\n", testi32a, testi32b, resulti32, expectedi32);

	resultu32 = mulu_32x32_hi(testu32a, testu32b);
	expectedu32 = ((INT64)testu32a * (INT64)testu32b) >> 32;
	if (resultu32 != expectedu32)
		mame_printf_error("Error testing mulu_32x32_hi (%08X x %08X) = %08X (expected %08X)\n", testu32a, testu32b, resultu32, expectedu32);

	resulti32 = mul_32x32_shift(testi32a, testi32b, 7);
	expectedi32 = ((INT64)testi32a * (INT64)testi32b) >> 7;
	if (resulti32 != expectedi32)
		mame_printf_error("Error testing mul_32x32_shift (%08X x %08X) >> 7 = %08X (expected %08X)\n", testi32a, testi32b, resulti32, expectedi32);

	resultu32 = mulu_32x32_shift(testu32a, testu32b, 7);
	expectedu32 = ((INT64)testu32a * (INT64)testu32b) >> 7;
	if (resultu32 != expectedu32)
		mame_printf_error("Error testing mulu_32x32_shift (%08X x %08X) >> 7 = %08X (expected %08X)\n", testu32a, testu32b, resultu32, expectedu32);

	while ((INT64)testi32a * (INT64)0x7fffffff < testi64a)
		testi64a /= 2;
	while ((UINT64)testu32a * (UINT64)bigu32 < testu64a)
		testu64a /= 2;

	resulti32 = div_64x32(testi64a, testi32a);
	expectedi32 = testi64a / (INT64)testi32a;
	if (resulti32 != expectedi32)
		mame_printf_error("Error testing div_64x32 (%08X%08X / %08X) = %08X (expected %08X)\n", (UINT32)(testi64a >> 32), (UINT32)testi64a, testi32a, resulti32, expectedi32);

	resultu32 = divu_64x32(testu64a, testu32a);
	expectedu32 = testu64a / (UINT64)testu32a;
	if (resultu32 != expectedu32)
		mame_printf_error("Error testing divu_64x32 (%08X%08X / %08X) = %08X (expected %08X)\n", (UINT32)(testu64a >> 32), (UINT32)testu64a, testu32a, resultu32, expectedu32);

	resulti32 = div_64x32_rem(testi64a, testi32a, &remainder);
	expectedi32 = testi64a / (INT64)testi32a;
	expremainder = testi64a % (INT64)testi32a;
	if (resulti32 != expectedi32 || remainder != expremainder)
		mame_printf_error("Error testing div_64x32_rem (%08X%08X / %08X) = %08X,%08X (expected %08X,%08X)\n", (UINT32)(testi64a >> 32), (UINT32)testi64a, testi32a, resulti32, remainder, expectedi32, expremainder);

	resultu32 = divu_64x32_rem(testu64a, testu32a, &uremainder);
	expectedu32 = testu64a / (UINT64)testu32a;
	expuremainder = testu64a % (UINT64)testu32a;
	if (resultu32 != expectedu32 || uremainder != expuremainder)
		mame_printf_error("Error testing divu_64x32_rem (%08X%08X / %08X) = %08X,%08X (expected %08X,%08X)\n", (UINT32)(testu64a >> 32), (UINT32)testu64a, testu32a, resultu32, uremainder, expectedu32, expuremainder);

	resulti32 = mod_64x32(testi64a, testi32a);
	expectedi32 = testi64a % (INT64)testi32a;
	if (resulti32 != expectedi32)
		mame_printf_error("Error testing mod_64x32 (%08X%08X / %08X) = %08X (expected %08X)\n", (UINT32)(testi64a >> 32), (UINT32)testi64a, testi32a, resulti32, expectedi32);

	resultu32 = modu_64x32(testu64a, testu32a);
	expectedu32 = testu64a % (UINT64)testu32a;
	if (resultu32 != expectedu32)
		mame_printf_error("Error testing modu_64x32 (%08X%08X / %08X) = %08X (expected %08X)\n", (UINT32)(testu64a >> 32), (UINT32)testu64a, testu32a, resultu32, expectedu32);

	while ((INT64)testi32a * (INT64)0x7fffffff < ((INT32)testi64a << 3))
		testi64a /= 2;
	while ((UINT64)testu32a * (UINT64)0xffffffff < ((UINT32)testu64a << 3))
		testu64a /= 2;

	resulti32 = div_32x32_shift((INT32)testi64a, testi32a, 3);
	expectedi32 = ((INT64)(INT32)testi64a << 3) / (INT64)testi32a;
	if (resulti32 != expectedi32)
		mame_printf_error("Error testing div_32x32_shift (%08X << 3) / %08X = %08X (expected %08X)\n", (INT32)testi64a, testi32a, resulti32, expectedi32);

	resultu32 = divu_32x32_shift((UINT32)testu64a, testu32a, 3);
	expectedu32 = ((UINT64)(UINT32)testu64a << 3) / (UINT64)testu32a;
	if (resultu32 != expectedu32)
		mame_printf_error("Error testing divu_32x32_shift (%08X << 3) / %08X = %08X (expected %08X)\n", (UINT32)testu64a, testu32a, resultu32, expectedu32);

	if (fabs(recip_approx(100.0) - 0.01) > 0.0001)
		mame_printf_error("Error testing recip_approx\n");

	testi32a = (testi32a & 0x0000ffff) | 0x400000;
	if (count_leading_zeros(testi32a) != 9)
		mame_printf_error("Error testing count_leading_zeros\n");
	testi32a = (testi32a | 0xffff0000) & ~0x400000;
	if (count_leading_ones(testi32a) != 9)
		mame_printf_error("Error testing count_leading_ones\n");

	testi32b = testi32a;
	if (compare_exchange32(&testi32a, testi32b, 1000) != testi32b || testi32a != 1000)
		mame_printf_error("Error testing compare_exchange32\n");
#ifdef PTR64
	testi64b = testi64a;
	if (compare_exchange64(&testi64a, testi64b, 1000) != testi64b || testi64a != 1000)
		mame_printf_error("Error testing compare_exchange64\n");
#endif
	if (atomic_exchange32(&testi32a, testi32b) != 1000)
		mame_printf_error("Error testing atomic_exchange32\n");
	if (atomic_add32(&testi32a, 45) != testi32b + 45)
		mame_printf_error("Error testing atomic_add32\n");
	if (atomic_increment32(&testi32a) != testi32b + 46)
		mame_printf_error("Error testing atomic_increment32\n");
	if (atomic_decrement32(&testi32a) != testi32b + 45)
		mame_printf_error("Error testing atomic_decrement32\n");
}


//-------------------------------------------------
//  validate_driver - validate basic driver
//  information
//-------------------------------------------------

void validity_checker::validate_driver()
{
	// check for duplicate names
	astring tempstr;
	if (m_names_map.add(m_current_driver->name, m_current_driver, false) == TMERR_DUPLICATE)
	{
		const game_driver *match = m_names_map.find(m_current_driver->name);
		mame_printf_error("Driver name is a duplicate of %s(%s)\n", core_filename_extract_base(tempstr, match->source_file).cstr(), match->name);
	}

	// check for duplicate descriptions
	if (m_descriptions_map.add(m_current_driver->description, m_current_driver, false) == TMERR_DUPLICATE)
	{
		const game_driver *match = m_descriptions_map.find(m_current_driver->description);
		mame_printf_error("Driver description is a duplicate of %s(%s)\n", core_filename_extract_base(tempstr, match->source_file).cstr(), match->name);
	}

	// determine if we are a clone
	bool is_clone = (strcmp(m_current_driver->parent, "0") != 0);
	int clone_of = m_drivlist.clone(*m_current_driver);
	if (clone_of != -1 && (m_drivlist.driver(clone_of).flags & GAME_IS_BIOS_ROOT))
		is_clone = false;

	// if we have at least 100 drivers, validate the clone
	// (100 is arbitrary, but tries to avoid tiny.mak dependencies)
	if (driver_list::total() > 100 && clone_of == -1 && is_clone)
		mame_printf_error("Driver is a clone of nonexistant driver %s\n", m_current_driver->parent);

	// look for recursive cloning
	if (clone_of != -1 && &m_drivlist.driver(clone_of) == m_current_driver)
		mame_printf_error("Driver is a clone of itself\n");

	// look for clones that are too deep
	if (clone_of != -1 && (clone_of = m_drivlist.non_bios_clone(clone_of)) != -1)
		mame_printf_error("Driver is a clone of a clone\n");

	// make sure the driver name is not too long
	if (!is_clone && strlen(m_current_driver->name) > 8)
		mame_printf_error("Parent driver name must be 8 characters or less\n");
	if (is_clone && strlen(m_current_driver->name) > 16)
		mame_printf_error("Clone driver name must be 16 characters or less\n");

	// make sure the year is only digits, '?' or '+'
	for (const char *s = m_current_driver->year; *s != 0; s++)
		if (!isdigit((UINT8)*s) && *s != '?' && *s != '+')
		{
			mame_printf_error("Driver has an invalid year '%s'\n", m_current_driver->year);
			break;
		}

	// normalize driver->compatible_with
	const char *compatible_with = m_current_driver->compatible_with;
	if (compatible_with != NULL && strcmp(compatible_with, "0") == 0)
		compatible_with = NULL;

	// check for this driver being compatible with a non-existant driver
	if (compatible_with != NULL && m_drivlist.find(m_current_driver->compatible_with) == -1)
		mame_printf_error("Driver is listed as compatible with nonexistant driver %s\n", m_current_driver->compatible_with);

	// check for clone_of and compatible_with being specified at the same time
	if (m_drivlist.clone(*m_current_driver) != -1 && compatible_with != NULL)
		mame_printf_error("Driver cannot be both a clone and listed as compatible with another system\n");

	// find any recursive dependencies on the current driver
	for (int other_drv = m_drivlist.compatible_with(*m_current_driver); other_drv != -1; other_drv = m_drivlist.compatible_with(other_drv))
		if (m_current_driver == &m_drivlist.driver(other_drv))
		{
			mame_printf_error("Driver is recursively compatible with itself\n");
			break;
		}

	// make sure sound-less drivers are flagged
	sound_interface_iterator iter(m_current_config->root_device());
	if ((m_current_driver->flags & GAME_IS_BIOS_ROOT) == 0 && iter.first() == NULL && (m_current_driver->flags & GAME_NO_SOUND) == 0 && (m_current_driver->flags & GAME_NO_SOUND_HW) == 0)
		mame_printf_error("Driver is missing GAME_NO_SOUND flag\n");
}


//-------------------------------------------------
//  validate_roms - validate ROM definitions
//-------------------------------------------------

void validity_checker::validate_roms()
{
	// iterate, starting with the driver's ROMs and continuing with device ROMs
	device_iterator deviter(m_current_config->root_device());
	for (device_t *device = deviter.first(); device != NULL; device = deviter.next())
	{
		// for non-root devices, track the current device
		m_current_device = (device->owner() == NULL) ? NULL : device;

		// scan the ROM entries for this device
		const char *last_region_name = "???";
		const char *last_name = "???";
		UINT32 current_length = 0;
		int items_since_region = 1;
		int last_bios = 0;
		int total_files = 0;
		for (const rom_entry *romp = rom_first_region(*device); romp != NULL && !ROMENTRY_ISEND(romp); romp++)
		{
			// if this is a region, make sure it's valid, and record the length
			if (ROMENTRY_ISREGION(romp))
			{
				// if we haven't seen any items since the last region, print a warning
				if (items_since_region == 0)
					mame_printf_warning("Empty ROM region '%s' (warning)\n", last_region_name);

				// reset our region tracking states
				const char *basetag = ROMREGION_GETTAG(romp);
				items_since_region = (ROMREGION_ISERASE(romp) || ROMREGION_ISDISKDATA(romp)) ? 1 : 0;
				last_region_name = basetag;

				// check for a valid tag
				if (basetag == NULL)
				{
					mame_printf_error("ROM_REGION tag with NULL name\n");
					continue;
				}

				// validate the base tag
				validate_tag(basetag);

				// generate the full tag
				astring fulltag;
				rom_region_name(fulltag, *device, romp);

				// attempt to add it to the map, reporting duplicates as errors
				current_length = ROMREGION_GETLENGTH(romp);
				if (m_region_map.add(fulltag, current_length, false) == TMERR_DUPLICATE)
					mame_printf_error("Multiple ROM_REGIONs with the same tag '%s' defined\n", fulltag.cstr());
			}

			// If this is a system bios, make sure it is using the next available bios number
			else if (ROMENTRY_ISSYSTEM_BIOS(romp))
			{
				int bios_flags = ROM_GETBIOSFLAGS(romp);
				if (bios_flags != last_bios + 1)
					mame_printf_error("Non-sequential bios %s (specified as %d, expected to be %d)\n", ROM_GETNAME(romp), bios_flags, last_bios + 1);
				last_bios = bios_flags;
			}

			// if this is a file, make sure it is properly formatted
			else if (ROMENTRY_ISFILE(romp))
			{
				// track the last filename we found
				last_name = ROM_GETNAME(romp);
				total_files++;

				// make sure it's all lowercase
				for (const char *s = last_name; *s != 0; s++)
					if (tolower((UINT8)*s) != *s)
					{
						mame_printf_error("ROM name '%s' contains upper case characters\n", last_name);
						break;
					}

				// make sure the hash is valid
				hash_collection hashes;
				if (!hashes.from_internal_string(ROM_GETHASHDATA(romp)))
					mame_printf_error("ROM '%s' has an invalid hash string '%s'\n", last_name, ROM_GETHASHDATA(romp));
			}

			// for any non-region ending entries, make sure they don't extend past the end
			if (!ROMENTRY_ISREGIONEND(romp) && current_length > 0)
			{
				items_since_region++;
				if (ROM_GETOFFSET(romp) + ROM_GETLENGTH(romp) > current_length)
					mame_printf_error("ROM '%s' extends past the defined memory region\n", last_name);
			}
		}

		// final check for empty regions
		if (items_since_region == 0)
			mame_printf_warning("Empty ROM region '%s' (warning)\n", last_region_name);


		// reset the current device
		m_current_device = NULL;
	}
}


//-------------------------------------------------
//  validate_display - validate display
//  configurations
//-------------------------------------------------

void validity_checker::validate_display()
{
	// iterate over screen devices looking for paletted screens
	screen_device_iterator iter(m_current_config->root_device());
	bool palette_modes = false;
	for (const screen_device *scrconfig = iter.first(); scrconfig != NULL; scrconfig = iter.next())
		if (scrconfig->format() == BITMAP_FORMAT_IND16)
			palette_modes = true;

	// check for empty palette
	if (palette_modes && m_current_config->m_total_colors == 0)
		mame_printf_error("Driver has zero palette entries but uses a palette-based bitmap format\n");
}


//-------------------------------------------------
//  validate_gfx - validate graphics decoding
//  configuration
//-------------------------------------------------

void validity_checker::validate_gfx()
{
	// bail if no gfx
	if (!m_current_config->m_gfxdecodeinfo)
		return;

	// iterate over graphics decoding entries
	for (int gfxnum = 0; gfxnum < MAX_GFX_ELEMENTS && m_current_config->m_gfxdecodeinfo[gfxnum].gfxlayout != NULL; gfxnum++)
	{
		const gfx_decode_entry &gfx = m_current_config->m_gfxdecodeinfo[gfxnum];
		const gfx_layout &layout = *gfx.gfxlayout;

		// make sure the region exists
		const char *region = gfx.memory_region;
		if (region != NULL)
		{
			// resolve the region
			astring gfxregion;
			m_current_config->root_device().subtag(gfxregion, region);

			// loop over gfx regions
			UINT32 region_length = m_region_map.find(gfxregion);
			if (region_length == 0)
				mame_printf_error("gfx[%d] references non-existent region '%s'\n", gfxnum, region);

			// if we have a valid region, and we're not using auto-sizing, check the decode against the region length
			else if (!IS_FRAC(layout.total))
			{
				// determine which plane is at the largest offset
				int start = 0;
				for (int plane = 0; plane < layout.planes; plane++)
					if (layout.planeoffset[plane] > start)
						start = layout.planeoffset[plane];
				start &= ~(layout.charincrement - 1);

				// determine the total length based on this info
				int len = layout.total * layout.charincrement;

				// do we have enough space in the region to cover the whole decode?
				int avail = region_length - (gfx.start & ~(layout.charincrement / 8 - 1));

				// if not, this is an error
				if ((start + len) / 8 > avail)
					mame_printf_error("gfx[%d] extends past allocated memory of region '%s'\n", gfxnum, region);
			}
		}

		int xscale = (m_current_config->m_gfxdecodeinfo[gfxnum].xscale == 0) ? 1 : m_current_config->m_gfxdecodeinfo[gfxnum].xscale;
		int yscale = (m_current_config->m_gfxdecodeinfo[gfxnum].yscale == 0) ? 1 : m_current_config->m_gfxdecodeinfo[gfxnum].yscale;

		// verify raw decode, which can only be full-region and have no scaling
		if (layout.planeoffset[0] == GFX_RAW)
		{
			if (layout.total != RGN_FRAC(1,1))
				mame_printf_error("gfx[%d] with unsupported layout total\n", gfxnum);
			if (xscale != 1 || yscale != 1)
				mame_printf_error("gfx[%d] with unsupported xscale/yscale\n", gfxnum);
		}

		// verify traditional decode doesn't have too many planes or is not too large
		else
		{
			if (layout.planes > MAX_GFX_PLANES)
				mame_printf_error("gfx[%d] with invalid planes\n", gfxnum);
			if (xscale * layout.width > MAX_ABS_GFX_SIZE || yscale * layout.height > MAX_ABS_GFX_SIZE)
				mame_printf_error("gfx[%d] with invalid xscale/yscale\n", gfxnum);
		}
	}
}


//-------------------------------------------------
//  validate_analog_input_field - validate an
//  analog input field
//-------------------------------------------------

void validity_checker::validate_analog_input_field(ioport_field &field)
{
	// analog ports must have a valid sensitivity
	if (field.sensitivity() == 0)
		mame_printf_error("Analog port with zero sensitivity\n");

	// check that the default falls in the bitmask range
	if (field.defvalue() & ~field.mask())
		mame_printf_error("Analog port with a default value (%X) out of the bitmask range (%X)\n", field.defvalue(), field.mask());

	// tests for positional devices
	if (field.type() == IPT_POSITIONAL || field.type() == IPT_POSITIONAL_V)
	{
		int shift;
		for (shift = 0; shift <= 31 && (~field.mask() & (1 << shift)) != 0; shift++) ;

		// convert the positional max value to be in the bitmask for testing
		INT32 analog_max = field.maxval();
		analog_max = (analog_max - 1) << shift;

		// positional port size must fit in bits used
		if ((field.mask() >> shift) + 1 < field.maxval())
			mame_printf_error("Analog port with a positional port size bigger then the mask size\n");
	}

	// tests for absolute devices
	else if (field.type() > IPT_ANALOG_ABSOLUTE_FIRST && field.type() < IPT_ANALOG_ABSOLUTE_LAST)
	{
		// adjust for signed values
		INT32 default_value = field.defvalue();
		INT32 analog_min = field.minval();
		INT32 analog_max = field.maxval();
		if (analog_min > analog_max)
		{
			analog_min = -analog_min;
			if (default_value > analog_max)
				default_value = -default_value;
		}

		// check that the default falls in the MINMAX range
		if (default_value < analog_min || default_value > analog_max)
			mame_printf_error("Analog port with a default value (%X) out of PORT_MINMAX range (%X-%X)\n", field.defvalue(), field.minval(), field.maxval());

		// check that the MINMAX falls in the bitmask range
		// we use the unadjusted min for testing
		if (field.minval() & ~field.mask() || analog_max & ~field.mask())
			mame_printf_error("Analog port with a PORT_MINMAX (%X-%X) value out of the bitmask range (%X)\n", field.minval(), field.maxval(), field.mask());

		// absolute analog ports do not use PORT_RESET
		if (field.analog_reset())
			mame_printf_error("Absolute analog port using PORT_RESET\n");

		// absolute analog ports do not use PORT_WRAPS
		if (field.analog_wraps())
			mame_printf_error("Absolute analog port using PORT_WRAPS\n");
	}

	// tests for non IPT_POSITIONAL relative devices
	else
	{
		// relative devices do not use PORT_MINMAX
		if (field.minval() != 0 || field.maxval() != field.mask())
			mame_printf_error("Relative port using PORT_MINMAX\n");

		// relative devices do not use a default value
		// the counter is at 0 on power up
		if (field.defvalue() != 0)
			mame_printf_error("Relative port using non-0 default value\n");

		// relative analog ports do not use PORT_WRAPS
		if (field.analog_wraps())
			mame_printf_error("Absolute analog port using PORT_WRAPS\n");
	}
}


//-------------------------------------------------
//  validate_dip_settings - validate a DIP switch
//  setting
//-------------------------------------------------

void validity_checker::validate_dip_settings(ioport_field &field)
{
	const char *demo_sounds = ioport_string_from_index(INPUT_STRING_Demo_Sounds);
	const char *flipscreen = ioport_string_from_index(INPUT_STRING_Flip_Screen);
	UINT8 coin_list[__input_string_coinage_end + 1 - __input_string_coinage_start] = { 0 };
	bool coin_error = false;

	// iterate through the settings
	for (ioport_setting *setting = field.first_setting(); setting != NULL; setting = setting->next())
	{
		// note any coinage strings
		int strindex = get_defstr_index(setting->name());
		if (strindex >= __input_string_coinage_start && strindex <= __input_string_coinage_end)
			coin_list[strindex - __input_string_coinage_start] = 1;

		// make sure demo sounds default to on
		if (field.name() == demo_sounds && strindex == INPUT_STRING_On && field.defvalue() != setting->value())
			mame_printf_error("Demo Sounds must default to On\n");

		// check for bad demo sounds options
		if (field.name() == demo_sounds && (strindex == INPUT_STRING_Yes || strindex == INPUT_STRING_No))
			mame_printf_error("Demo Sounds option must be Off/On, not %s\n", setting->name());

		// check for bad flip screen options
		if (field.name() == flipscreen && (strindex == INPUT_STRING_Yes || strindex == INPUT_STRING_No))
			mame_printf_error("Flip Screen option must be Off/On, not %s\n", setting->name());

		// if we have a neighbor, compare ourselves to him
		if (setting->next() != NULL)
		{
			// check for inverted off/on dispswitch order
			int next_strindex = get_defstr_index(setting->next()->name(), true);
			if (strindex == INPUT_STRING_On && next_strindex == INPUT_STRING_Off)
				mame_printf_error("%s option must have Off/On options in the order: Off, On\n", field.name());

			// check for inverted yes/no dispswitch order
			else if (strindex == INPUT_STRING_Yes && next_strindex == INPUT_STRING_No)
				mame_printf_error("%s option must have Yes/No options in the order: No, Yes\n", field.name());

			// check for inverted upright/cocktail dispswitch order
			else if (strindex == INPUT_STRING_Cocktail && next_strindex == INPUT_STRING_Upright)
				mame_printf_error("%s option must have Upright/Cocktail options in the order: Upright, Cocktail\n", field.name());

			// check for proper coin ordering
			else if (strindex >= __input_string_coinage_start && strindex <= __input_string_coinage_end && next_strindex >= __input_string_coinage_start && next_strindex <= __input_string_coinage_end &&
						strindex >= next_strindex && setting->condition() == setting->next()->condition())
			{
				mame_printf_error("%s option has unsorted coinage %s > %s\n", field.name(), setting->name(), setting->next()->name());
				coin_error = true;
			}
		}
	}

	// if we have a coin error, demonstrate the correct way
	if (coin_error)
	{
		output_via_delegate(m_saved_error_output, "   Note proper coin sort order should be:\n");
		for (int entry = 0; entry < ARRAY_LENGTH(coin_list); entry++)
			if (coin_list[entry])
				output_via_delegate(m_saved_error_output, "      %s\n", ioport_string_from_index(__input_string_coinage_start + entry));
	}
}


//-------------------------------------------------
//  validate_condition - validate a condition
//  stored within an ioport field or setting
//-------------------------------------------------

void validity_checker::validate_condition(ioport_condition &condition, device_t &device, int_map &port_map)
{
	// resolve the tag
	astring porttag;
	device.subtag(porttag, condition.tag());

	// then find a matching port
	if (port_map.find(porttag) == 0)
		mame_printf_error("Condition referencing non-existent ioport tag '%s'\n", condition.tag());
}


//-------------------------------------------------
//  validate_inputs - validate input configuration
//-------------------------------------------------

void validity_checker::validate_inputs()
{
	int_map port_map;

	// iterate over devices
	device_iterator iter(m_current_config->root_device());
	for (device_t *device = iter.first(); device != NULL; device = iter.next())
	{
		// see if this device has ports; if not continue
		if (device->input_ports() == NULL)
			continue;

		// for non-root devices, track the current device
		m_current_device = (device == &m_current_config->root_device()) ? NULL : device;

		// allocate the input ports
		ioport_list portlist;
		astring errorbuf;
		portlist.append(*device, errorbuf);

		// report any errors during construction
		if (errorbuf)
			mame_printf_error("I/O port error during construction:\n%s\n", errorbuf.cstr());

		// do a first pass over ports to add their names and find duplicates
		for (ioport_port *port = portlist.first(); port != NULL; port = port->next())
			if (port_map.add(port->tag(), 1, false) == TMERR_DUPLICATE)
				mame_printf_error("Multiple I/O ports with the same tag '%s' defined\n", port->tag());

		// iterate over ports
		for (ioport_port *port = portlist.first(); port != NULL; port = port->next())
		{
			m_current_ioport = port->tag();

			// iterate through the fields on this port
			for (ioport_field *field = port->first_field(); field != NULL; field = field->next())
			{
				// verify analog inputs
				if (field->is_analog())
					validate_analog_input_field(*field);

				// look for invalid (0) types which should be mapped to IPT_OTHER
				if (field->type() == IPT_INVALID)
					mame_printf_error("Field has an invalid type (0); use IPT_OTHER instead\n");

				// verify dip switches
				if (field->type() == IPT_DIPSWITCH)
				{
					// dip switch fields must have a name
					if (field->name() == NULL)
						mame_printf_error("DIP switch has a NULL name\n");

					// verify the settings list
					validate_dip_settings(*field);
				}

				// verify names
				const char *name = field->specific_name();
				if (name != NULL)
				{
					// check for empty string
					if (name[0] == 0)
						mame_printf_error("Field name is an empty string\n");

					// check for trailing spaces
					if (name[0] != 0 && name[strlen(name) - 1] == ' ')
						mame_printf_error("Field '%s' has trailing spaces\n", name);

					// check for invalid UTF-8
					if (!utf8_is_valid_string(name))
						mame_printf_error("Field '%s' has invalid characters\n", name);

					// look up the string and print an error if default strings are not used
					/*strindex =get_defstr_index(defstr_map, name, driver, &error);*/
				}

				// verify conditions on the field
				if (!field->condition().none())
					validate_condition(field->condition(), *device, port_map);

				// verify conditions on the settings
				for (ioport_setting *setting = field->first_setting(); setting != NULL; setting = setting->next())
					if (!setting->condition().none())
						validate_condition(setting->condition(), *device, port_map);
			}

			// done with this port
			m_current_ioport = NULL;
		}

		// done with this device
		m_current_device = NULL;
	}
}


//-------------------------------------------------
//  validate_devices - run per-device validity
//  checks
//-------------------------------------------------

void validity_checker::validate_devices()
{
	int_map device_map;

	// iterate over devices
	device_iterator iter(m_current_config->root_device());
	for (const device_t *device = iter.first(); device != NULL; device = iter.next())
	{
		// for non-root devices, track the current device
		m_current_device = (device == &m_current_config->root_device()) ? NULL : device;

		// validate the device tag
		validate_tag(device->basetag());

		// look for duplicates
		if (device_map.add(device->tag(), 0, false) == TMERR_DUPLICATE)
			mame_printf_error("Multiple devices with the same tag '%s' defined\n", device->tag());

		// if we have a ROM region, we must have a shortname
		if (device->rom_region() != NULL && strcmp(device->shortname(), "") == 0)
			mame_printf_error("Device has ROM definition but does not have short name defined\n");

		// check for device-specific validity check
		device->validity_check(*this);

		// done with this device
		m_current_device = NULL;
	}

	// if device is slot cart device, we must have a shortname
	int_map slot_device_map;
	slot_interface_iterator slotiter(m_current_config->root_device());
	for (const device_slot_interface *slot = slotiter.first(); slot != NULL; slot = slotiter.next())
	{
		const slot_interface* intf = slot->get_slot_interfaces();
		for (int i = 0; intf && intf[i].name != NULL; i++)
		{
			astring temptag("_");
			temptag.cat(intf[i].name);
			device_t *dev = const_cast<machine_config &>(*m_current_config).device_add(&m_current_config->root_device(), temptag.cstr(), intf[i].devtype, 0);

			// notify this device and all its subdevices that they are now configured
			device_iterator subiter(*dev);
			for (device_t *device = subiter.first(); device != NULL; device = subiter.next())
				if (!device->configured())
					device->config_complete();

			if (strcmp(dev->shortname(), "") == 0) {
				if (slot_device_map.add(dev->name(), 0, false) != TMERR_DUPLICATE)
					mame_printf_error("Device '%s' is slot cart device but does not have short name defined\n",dev->name());
			}

			const_cast<machine_config &>(*m_current_config).device_remove(&m_current_config->root_device(), temptag.cstr());
			global_free(dev);
		}
	}

}


//-------------------------------------------------
//  build_output_prefix - create a prefix
//  indicating the current source file, driver,
//  and device
//-------------------------------------------------

void validity_checker::build_output_prefix(astring &string)
{
	// start empty
	string.reset();

	// if we have a current device, indicate that
	if (m_current_device != NULL)
		string.cat(m_current_device->name()).cat(" device '").cat(m_current_device->tag()).cat("': ");

	// if we have a current port, indicate that as well
	if (m_current_ioport != NULL)
		string.cat("ioport '").cat(m_current_ioport).cat("': ");
}


//-------------------------------------------------
//  error_output - error message output override
//-------------------------------------------------

void validity_checker::error_output(const char *format, va_list argptr)
{
	// count the error
	m_errors++;

	// output the source(driver) device 'tag'
	astring output;
	build_output_prefix(output);

	// generate the string
	output.catvprintf(format, argptr);
	m_error_text.cat(output);
}


//-------------------------------------------------
//  warning_output - warning message output
//  override
//-------------------------------------------------

void validity_checker::warning_output(const char *format, va_list argptr)
{
	// count the error
	m_warnings++;

	// output the source(driver) device 'tag'
	astring output;
	build_output_prefix(output);

	// generate the string and output to the original target
	output.catvprintf(format, argptr);
	m_warning_text.cat(output);
}


//-------------------------------------------------
//  output_via_delegate - helper to output a
//  message via a varargs string, so the argptr
//  can be forwarded onto the given delegate
//-------------------------------------------------

void validity_checker::output_via_delegate(output_delegate &delegate, const char *format, ...)
{
	va_list argptr;

	// call through to the delegate with the proper parameters
	va_start(argptr, format);
	delegate(format, argptr);
	va_end(argptr);
}
