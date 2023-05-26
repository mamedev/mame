// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Interrupt Controllers

    Interrupt controllers for the Heath H89.

****************************************************************************/

#ifndef MAME_HEATHKIT_INTR_CNTRL_H
#define MAME_HEATHKIT_INTR_CNTRL_H

#pragma once

class heath_intr_cntrl : public device_t
{
public:
	heath_intr_cntrl(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void raise_irq(uint8_t level);

	// interface routines
	auto intr_inst_callback() { return m_intr_inst.bind(); }

protected:
	heath_intr_cntrl(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_instr(uint8_t data);

	virtual void device_start() override;

	virtual void device_resolve_objects() override;

	devcb_write8 m_intr_inst;

};

class z37_intr_cntrl : public heath_intr_cntrl
{
public:
	z37_intr_cntrl(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void raise_irq(uint8_t level) override;

	virtual void set_drq(uint8_t data);
	virtual void set_intrq(uint8_t data);

	virtual void block_interrupts(uint8_t block);

protected:

	virtual void device_start() override;

private:
	bool interrupts_blocked;

};

DECLARE_DEVICE_TYPE(HEATH_INTR_CNTRL, heath_intr_cntrl)
DECLARE_DEVICE_TYPE(HEATH_Z37_INTR_CNTRL, z37_intr_cntrl)


#endif // MAME_HEATHKIT_H89_INTR_CNTRL_H
