// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Interrupt Controllers

    Interrupt controllers for the Heath H89.

****************************************************************************/

#ifndef MAME_HEATHKIT_INTR_CNTRL_H
#define MAME_HEATHKIT_INTR_CNTRL_H

#pragma once

/**
 * Heath H89 interrupt controller
 *
 */
class heath_intr_cntrl : public device_t
{
public:
	heath_intr_cntrl(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void raise_irq(uint8_t level);
	virtual void lower_irq(uint8_t level);

	IRQ_CALLBACK_MEMBER(irq_callback);

	auto irq_line_cb() { return m_irq_line.bind(); }

protected:
	heath_intr_cntrl(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual uint8_t get_instruction();
	virtual void update_intr_line();

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_resolve_objects() override;

	devcb_write8 m_irq_line;

	uint8_t m_intr_lines;
};

/**
 * Interrupt controller when the Z37 soft-sectored controller is installed.
 *
 */
class z37_intr_cntrl : public heath_intr_cntrl
{
public:
	z37_intr_cntrl(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void set_drq(uint8_t data);
	virtual void set_intrq(uint8_t data);
	virtual void block_interrupts(uint8_t data);

protected:

	virtual uint8_t get_instruction() override;
	virtual void update_intr_line() override;

	virtual void device_start() override;
	virtual void device_reset() override;

private:
	bool m_interrupts_blocked;
	bool m_drq_raised;
	bool m_fd_irq_raised;
};

DECLARE_DEVICE_TYPE(HEATH_INTR_CNTRL, heath_intr_cntrl)
DECLARE_DEVICE_TYPE(HEATH_Z37_INTR_CNTRL, z37_intr_cntrl)

#endif // MAME_HEATHKIT_H89_INTR_CNTRL_H
