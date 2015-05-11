// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    modules.c

    List of Imgtool modules

***************************************************************************/

#include "imgtool.h"
#include "modules.h"

#ifndef MODULES_RECURSIVE
#define MODULES_RECURSIVE

/* step 1: declare all external references */
#define MODULE(name)    extern void name##_get_info(const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info);
#include "modules.c"
#undef MODULE

/* step 2: define the modules[] array */
#define MODULE(name)    name##_get_info,
static void (*const modules[])(const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info) =
{
#include "modules.c"
};

/* step 3: declare imgtool_create_cannonical_library() */
imgtoolerr_t imgtool_create_cannonical_library(int omit_untested, imgtool_library **library)
{
	imgtoolerr_t err;
	size_t i;
	imgtool_library *lib;
	imgtool_module *module;

	/* list of modules that we drop */
	static const char *const irrelevant_modules[] =
	{
		"coco_os9_rsdos"
	};

	lib = imgtool_library_create();
	if (!lib)
	{
		err = IMGTOOLERR_OUTOFMEMORY;
		goto error;
	}

	/* create all modules */
	for (i = 0; i < ARRAY_LENGTH(modules); i++)
		imgtool_library_add(lib, modules[i]);

	/* remove irrelevant modules */
	for (i = 0; i < ARRAY_LENGTH(irrelevant_modules); i++)
	{
		imgtool_library_unlink(lib, irrelevant_modules[i]);
	}

	/* if we are omitting untested, go through and block out the functionality in question */
	if (omit_untested)
	{
		module = NULL;
		while((module = imgtool_library_iterate(lib, module)) != NULL)
		{
			if (module->writing_untested)
			{
				module->write_sector = NULL;
			}
			if (module->creation_untested)
			{
				module->create = NULL;
				module->createimage_optguide = NULL;
				module->createimage_optspec = NULL;
			}
		}
	}

	*library = lib;
	return IMGTOOLERR_SUCCESS;

error:
	if (lib)
		imgtool_library_close(lib);
	return err;

}


#else /* MODULES_RECURSIVE */

MODULE(amiga_floppy)
MODULE(concept)
MODULE(mac_mfs)
MODULE(mac_hfs)
MODULE(hd)
MODULE(rsdos)
MODULE(vzdos)
MODULE(os9)
MODULE(ti99_old)
MODULE(ti99_v9t9)
MODULE(ti99_pc99fm)
MODULE(ti99_pc99mfm)
MODULE(ti99_ti99hd)
MODULE(ti990)
MODULE(pc_floppy)
MODULE(pc_chd)
MODULE(prodos_525)
MODULE(prodos_35)
MODULE(thom_fd_basic)
MODULE(thom_qd_basic)
MODULE(thom_sap_basic)
MODULE(cybiko)
MODULE(cybikoxt)
MODULE(psion)
MODULE(bml3)
MODULE(hp48)

#endif /* MODULES_RECURSIVE */
