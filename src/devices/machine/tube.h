// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Tube ULA emulation

**********************************************************************/

#ifndef MAME_MACHINE_TUBE_H
#define MAME_MACHINE_TUBE_H

#pragma once



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TUBE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TUBE, 0);

#define MCFG_TUBE_HIRQ_HANDLER(_devcb) \
	devcb = &downcast<tube_device &>(*device).set_hirq_handler(DEVCB_##_devcb);

#define MCFG_TUBE_PNMI_HANDLER(_devcb) \
	devcb = &downcast<tube_device &>(*device).set_pnmi_handler(DEVCB_##_devcb);

#define MCFG_TUBE_PIRQ_HANDLER(_devcb) \
	devcb = &downcast<tube_device &>(*device).set_pirq_handler(DEVCB_##_devcb);

#define MCFG_TUBE_DRQ_HANDLER(_devcb) \
	devcb = &downcast<tube_device &>(*device).set_drq_handler(DEVCB_##_devcb);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> tube_device

class tube_device : public device_t
{
public:
	// construction/destruction
	tube_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	template <class Object> devcb_base &set_hirq_handler(Object &&cb) { return m_hirq_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_pnmi_handler(Object &&cb) { return m_pnmi_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_pirq_handler(Object &&cb) { return m_pirq_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_drq_handler(Object &&cb) { return m_drq_handler.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER(host_r);
	DECLARE_WRITE8_MEMBER(host_w);
	DECLARE_READ8_MEMBER(parasite_r);
	DECLARE_WRITE8_MEMBER(parasite_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t m_ph1[24];
	uint8_t m_ph2;
	uint8_t m_ph3[2];
	uint8_t m_ph4;
	uint8_t m_hp1;
	uint8_t m_hp2;
	uint8_t m_hp3[2];
	uint8_t m_hp4;
	uint8_t m_hstat[4];
	uint8_t m_pstat[4];
	uint8_t m_r1stat;
	int m_ph1pos;
	int m_ph3pos;
	int m_hp3pos;

	void update_interrupts();

	devcb_write_line m_hirq_handler;
	devcb_write_line m_pnmi_handler;
	devcb_write_line m_pirq_handler;
	devcb_write_line m_drq_handler;
};


// device type definition
DECLARE_DEVICE_TYPE(TUBE, tube_device)

#endif // MAME_MACHINE_TUBE_H
