// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*
 * outpout_module.h
 *
 */

#ifndef OUTPUT_MODULE_H_
#define OUTPUT_MODULE_H_

#include "osdepend.h"
#include "modules/osdmodule.h"

//============================================================
//  CONSTANTS
//============================================================

#define OSD_OUTPUT_PROVIDER   "output"

class output_module
{
public:
	output_module() { }

	virtual ~output_module() { }

	virtual void notify(const char *outname, INT32 value) = 0;
};

#endif /* OUTPUT_MODULE_H_ */
