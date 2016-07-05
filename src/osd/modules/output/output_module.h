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
	output_module(): m_machine(nullptr) { }

	virtual ~output_module() { }

	virtual void notify(const char *outname, INT32 value) = 0;

	void set_machine(running_machine *machine) { m_machine = machine;  };
	running_machine &machine() const { return *m_machine; }
private:
	running_machine *m_machine;
};

#endif /* OUTPUT_MODULE_H_ */
