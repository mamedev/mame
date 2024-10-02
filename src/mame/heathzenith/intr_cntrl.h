// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Interrupt Controllers

    Interrupt controllers for the Heath H89.

****************************************************************************/

#ifndef MAME_HEATHKIT_INTR_CNTRL_H
#define MAME_HEATHKIT_INTR_CNTRL_H

#pragma once

class heath_intr_socket;

class device_heath_intr_interface : public device_interface
{
public:
	virtual void set_irq_level(u8 level, int state) = 0;

	virtual void set_drq(int state) {}
	virtual void set_irq(int state) {}
	virtual void block_interrupts(u8 data) {}

protected:
	device_heath_intr_interface(const machine_config &mconfig, device_t &device);

	virtual u8 get_instruction() = 0;

	heath_intr_socket *const m_socket;

	friend heath_intr_socket;
};


/**
 * Heath original interrupt controller
 *
 */
class heath_intr_cntrl : public device_t,
						 public device_heath_intr_interface
{
public:
	heath_intr_cntrl(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void set_irq_level(u8 level, int state) override;

protected:
	heath_intr_cntrl(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock = 0);

	virtual u8 get_instruction() override;
	virtual void update_intr_line();

	virtual void device_start() override ATTR_COLD;

	u8 m_intr_lines;
};


/**
 * Base interrupt controller for multiple soft-sectored controllers.
 *
 */
class ss_intr_cntrl : public heath_intr_cntrl
{
public:
	virtual void set_drq(int state) override;
	virtual void set_irq(int state) override;

protected:
	ss_intr_cntrl(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock = 0);

	virtual void device_start() override ATTR_COLD;

	bool m_drq_raised;
	bool m_irq_raised;
};


/**
 * Interrupt controller when the Z37 soft-sectored controller is installed.
 *
 */
class z37_intr_cntrl : public ss_intr_cntrl
{
public:
	z37_intr_cntrl(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void block_interrupts(u8 data) override;

protected:
	virtual u8 get_instruction() override;
	virtual void update_intr_line() override;

	virtual void device_start() override ATTR_COLD;

	bool m_intr_blocked;
};


/**
 * Interrupt controller when the mms soft-sectored controller is installed.
 *
 */
class mms_intr_cntrl : public ss_intr_cntrl
{
public:
	mms_intr_cntrl(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual u8 get_instruction() override;
	virtual void update_intr_line() override;

	virtual void device_start() override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(HEATH_INTR_CNTRL,     heath_intr_cntrl)
DECLARE_DEVICE_TYPE(HEATH_Z37_INTR_CNTRL, z37_intr_cntrl)
DECLARE_DEVICE_TYPE(HEATH_MMS_INTR_CNTRL, mms_intr_cntrl);


class heath_intr_socket : public device_t,
						  public device_single_card_slot_interface<device_heath_intr_interface>
{
public:

	template <typename T>
	heath_intr_socket(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt, bool fixed = false) :
		heath_intr_socket(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
	}

	heath_intr_socket(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	virtual ~heath_intr_socket();

	auto irq_line_cb() { return m_irq_line.bind(); }

	// required operation
	void set_irq_level(u8 level, int state) { if (m_cntrl) { m_cntrl->set_irq_level(level, state); }}

	IRQ_CALLBACK_MEMBER(irq_callback);

	void raise_irq(int state) { m_irq_line(state); }

	void set_irq(int state) { if (m_cntrl) { m_cntrl->set_irq(state); } }
	void set_drq(int state) { if (m_cntrl) { m_cntrl->set_drq(state); } }
	void block_interrupts(u8 data) { if (m_cntrl) { m_cntrl->block_interrupts(data); } }

protected:

	virtual void device_start() override ATTR_COLD;

	devcb_write8                 m_irq_line;

	device_heath_intr_interface *m_cntrl;
};


DECLARE_DEVICE_TYPE(HEATH_INTR_SOCKET, heath_intr_socket)

#endif // MAME_HEATHKIT_H89_INTR_CNTRL_H
