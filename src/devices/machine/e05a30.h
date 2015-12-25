// license:BSD-3-Clause
// copyright-holders:Ramiro Polla
/*
 * E05A30 Gate Array (used in the Epson ActionPrinter 2000)
 *
 */

#ifndef __E05A30_H__
#define __E05A30_H__

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_E05A30_PRINTHEAD_CALLBACK(_write) \
	devcb = &e05a30_device::set_printhead_wr_callback(*device, DEVCB_##_write);

#define MCFG_E05A30_PF_STEPPER_CALLBACK(_write) \
	devcb = &e05a30_device::set_pf_stepper_wr_callback(*device, DEVCB_##_write);

#define MCFG_E05A30_CR_STEPPER_CALLBACK(_write) \
	devcb = &e05a30_device::set_cr_stepper_wr_callback(*device, DEVCB_##_write);

#define MCFG_E05A30_READY_CALLBACK(_write) \
	devcb = &e05a30_device::set_ready_wr_callback(*device, DEVCB_##_write);

#define MCFG_E05A30_CENTRONICS_ACK_CALLBACK(_write) \
	devcb = &e05a30_device::set_centronics_ack_wr_callback(*device, DEVCB_##_write);

#define MCFG_E05A30_CENTRONICS_BUSY_CALLBACK(_write) \
	devcb = &e05a30_device::set_centronics_busy_wr_callback(*device, DEVCB_##_write);

#define MCFG_E05A30_CENTRONICS_PERROR_CALLBACK(_write) \
	devcb = &e05a30_device::set_centronics_perror_wr_callback(*device, DEVCB_##_write);

#define MCFG_E05A30_CENTRONICS_FAULT_CALLBACK(_write) \
	devcb = &e05a30_device::set_centronics_fault_wr_callback(*device, DEVCB_##_write);

#define MCFG_E05A30_CENTRONICS_SELECT_CALLBACK(_write) \
	devcb = &e05a30_device::set_centronics_select_wr_callback(*device, DEVCB_##_write);

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class e05a30_device : public device_t
{
public:
	e05a30_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~e05a30_device() {}

	template<class _Object> static devcb_base &set_printhead_wr_callback(device_t &device, _Object object) { return downcast<e05a30_device &>(device).m_write_printhead.set_callback(object); }
	template<class _Object> static devcb_base &set_pf_stepper_wr_callback(device_t &device, _Object object) { return downcast<e05a30_device &>(device).m_write_pf_stepper.set_callback(object); }
	template<class _Object> static devcb_base &set_cr_stepper_wr_callback(device_t &device, _Object object) { return downcast<e05a30_device &>(device).m_write_cr_stepper.set_callback(object); }
	template<class _Object> static devcb_base &set_ready_wr_callback(device_t &device, _Object object) { return downcast<e05a30_device &>(device).m_write_ready.set_callback(object); }
	template<class _Object> static devcb_base &set_centronics_ack_wr_callback(device_t &device, _Object object) { return downcast<e05a30_device &>(device).m_write_centronics_ack.set_callback(object); }
	template<class _Object> static devcb_base &set_centronics_busy_wr_callback(device_t &device, _Object object) { return downcast<e05a30_device &>(device).m_write_centronics_busy.set_callback(object); }
	template<class _Object> static devcb_base &set_centronics_perror_wr_callback(device_t &device, _Object object) { return downcast<e05a30_device &>(device).m_write_centronics_perror.set_callback(object); }
	template<class _Object> static devcb_base &set_centronics_fault_wr_callback(device_t &device, _Object object) { return downcast<e05a30_device &>(device).m_write_centronics_fault.set_callback(object); }
	template<class _Object> static devcb_base &set_centronics_select_wr_callback(device_t &device, _Object object) { return downcast<e05a30_device &>(device).m_write_centronics_select.set_callback(object); }

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

	/* Centronics stuff */
	DECLARE_WRITE_LINE_MEMBER( centronics_input_strobe );
	DECLARE_WRITE_LINE_MEMBER( centronics_input_data0 ) { if (state) m_centronics_data |= 0x01; else m_centronics_data &= ~0x01; }
	DECLARE_WRITE_LINE_MEMBER( centronics_input_data1 ) { if (state) m_centronics_data |= 0x02; else m_centronics_data &= ~0x02; }
	DECLARE_WRITE_LINE_MEMBER( centronics_input_data2 ) { if (state) m_centronics_data |= 0x04; else m_centronics_data &= ~0x04; }
	DECLARE_WRITE_LINE_MEMBER( centronics_input_data3 ) { if (state) m_centronics_data |= 0x08; else m_centronics_data &= ~0x08; }
	DECLARE_WRITE_LINE_MEMBER( centronics_input_data4 ) { if (state) m_centronics_data |= 0x10; else m_centronics_data &= ~0x10; }
	DECLARE_WRITE_LINE_MEMBER( centronics_input_data5 ) { if (state) m_centronics_data |= 0x20; else m_centronics_data &= ~0x20; }
	DECLARE_WRITE_LINE_MEMBER( centronics_input_data6 ) { if (state) m_centronics_data |= 0x40; else m_centronics_data &= ~0x40; }
	DECLARE_WRITE_LINE_MEMBER( centronics_input_data7 ) { if (state) m_centronics_data |= 0x80; else m_centronics_data &= ~0x80; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	/* callbacks */
	devcb_write16 m_write_printhead;
	devcb_write8 m_write_pf_stepper;
	devcb_write8 m_write_cr_stepper;
	devcb_write_line m_write_ready;
	devcb_write_line m_write_centronics_ack;
	devcb_write_line m_write_centronics_busy;
	devcb_write_line m_write_centronics_perror;
	devcb_write_line m_write_centronics_fault;
	devcb_write_line m_write_centronics_select;

	void update_printhead(int pos, UINT8 data);
	void update_pf_stepper(UINT8 data);
	void update_cr_stepper(UINT8 data);

	/* port 0x05 and 0x06 (9-bit) */
	UINT16 m_printhead;
	/* port 0x07 (4-bit) */
	UINT8 m_pf_stepper;
	/* port 0x08 (4-bit) */
	UINT8 m_cr_stepper;

	/* Centronics stuff */
	UINT8 m_centronics_data;
	int m_centronics_busy;
	int m_centronics_nack;
	UINT8 m_centronics_strobe;
	UINT8 m_centronics_data_latch;
	UINT8 m_centronics_data_latched;
};

extern const device_type E05A30;

#endif /* __E05A30_H__ */
