// license:BSD-3-Clause
// copyright-holders:smf
#ifndef PC9801_SASI_H_
#define PC9801_SASI_H_

#include "scsihd.h"

class pc9801_sasi_device  : public scsihd_device
{
public:
	// construction/destruction
	pc9801_sasi_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void ExecCommand();
};

// device type definition
extern const device_type PC9801_SASI;

#endif /* PC9801_SASI_H_ */
