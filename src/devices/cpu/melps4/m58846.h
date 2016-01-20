// license:BSD-3-Clause
// copyright-holders:hap
/*

  Mitsubishi M58846 MCU

*/

#ifndef _M58846_H_
#define _M58846_H_

#include "melps4.h"

// note: for pinout and more info, see melps4.h


class m58846_device : public melps4_cpu_device
{
public:
	m58846_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual void execute_one() override;

	// device_disasm_interface overrides
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	// timers
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void write_v(UINT8 data) override;

	emu_timer *m_timer;
	void reset_timer();
};



extern const device_type M58846;


#endif /* _M58846_H_ */
