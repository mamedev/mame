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
#define MODULE(name)    extern void name##_get_info(const imgtool_class *imgclass, uint32_t state, union imgtoolinfo *info);
#include "modules.cpp"
#undef MODULE

/* step 2: define the modules[] array */
#define MODULE(name)    name##_get_info,
static void (*const modules[])(const imgtool_class *imgclass, uint32_t state, union imgtoolinfo *info) =
{
#include "modules.cpp"
};

/* step 3: declare imgtool_create_canonical_library() */
imgtoolerr_t imgtool_create_canonical_library(bool omit_untested, std::unique_ptr<imgtool::library> &library)
{
	/* list of modules that we drop */
	static const char *const irrelevant_modules[] =
	{
		"coco_os9_rsdos"
	};

	library.reset(new imgtool::library());
	if (!library)
		return IMGTOOLERR_OUTOFMEMORY;

	// create all modules
	for (auto &module : modules)
		library->add(module);

	// remove irrelevant modules
	for (auto &module : irrelevant_modules)
		library->unlink(module);

	// if we are omitting untested, go through and block out the functionality in question
	if (omit_untested)
	{
		for (auto &module : library->modules())
		{
			if (module->writing_untested)
			{
				module->write_sector = nullptr;
			}
			if (module->creation_untested)
			{
				module->create = nullptr;
				module->createimage_optguide = nullptr;
				module->createimage_optspec.clear();
			}
		}
	}

	return IMGTOOLERR_SUCCESS;
}


#else /* MODULES_RECURSIVE */

MODULE(amiga_floppy)
MODULE(concept)
MODULE(mac_mfs)
MODULE(mac_hfs)
MODULE(hd)
MODULE(rsdos)
MODULE(dgndos)
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
MODULE(hp9845_tape)
MODULE(hp85_tape)
MODULE(rt11)

#endif /* MODULES_RECURSIVE */
