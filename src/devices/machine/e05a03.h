// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    E05A03 Gate Array (used in the Epson LX-800)

***************************************************************************/

#ifndef __E05A03_H__
#define __E05A03_H__

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_E05A03_NLQ_LP_CALLBACK(_write) \
	devcb = &e05a03_device::set_nlq_lp_wr_callback(*device, DEVCB_##_write);

#define MCFG_E05A03_PE_LP_CALLBACK(_write) \
	devcb = &e05a03_device::set_pe_lp_wr_callback(*device, DEVCB_##_write);

#define MCFG_E05A03_RESO_CALLBACK(_write) \
	devcb = &e05a03_device::set_reso_wr_callback(*device, DEVCB_##_write);

#define MCFG_E05A03_PE_CALLBACK(_write) \
	devcb = &e05a03_device::set_pe_wr_callback(*device, DEVCB_##_write);

#define MCFG_E05A03_DATA_CALLBACK(_read) \
	devcb = &e05a03_device::set_data_rd_callback(*device, DEVCB_##_read);


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class e05a03_device : public device_t
{
public:
	e05a03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~e05a03_device() {}

	template<class _Object> static devcb_base &set_nlq_lp_wr_callback(device_t &device, _Object object) { return downcast<e05a03_device &>(device).m_write_nlq_lp.set_callback(object); }
	template<class _Object> static devcb_base &set_pe_lp_wr_callback(device_t &device, _Object object) { return downcast<e05a03_device &>(device).m_write_pe_lp.set_callback(object); }
	template<class _Object> static devcb_base &set_reso_wr_callback(device_t &device, _Object object) { return downcast<e05a03_device &>(device).m_write_reso.set_callback(object); }
	template<class _Object> static devcb_base &set_pe_wr_callback(device_t &device, _Object object) { return downcast<e05a03_device &>(device).m_write_pe.set_callback(object); }
	template<class _Object> static devcb_base &set_data_rd_callback(device_t &device, _Object object) { return downcast<e05a03_device &>(device).m_read_data.set_callback(object); }

	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void home_w(int state); /* home position signal */
	void fire_w(int state); /* printhead solenoids trigger */
	void strobe_w(int state);
	int busy_r();
	void resi_w(int state); /* reset input */
	void init_w(int state); /* centronics init */

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	/* callbacks */
	devcb_write_line m_write_nlq_lp; /* pin 2, nlq lamp output */
	devcb_write_line m_write_pe_lp;  /* pin 3, paper empty lamp output */
	devcb_write_line m_write_reso;   /* pin 25, reset output */
	devcb_write_line m_write_pe;     /* pin 35, centronics pe output */
	devcb_read8 m_read_data;         /* pin 47-54, centronics data input */

	/* 24-bit shift register, port 0x00, 0x01 and 0x02 */
	uint32_t m_shift;

	/* port 0x03 */
	int m_busy_leading;
	int m_busy_software;
	int m_nlqlp;
	int m_cndlp;

#if 0
	int m_pe;
	int m_pelp;
#endif

	/* port 0x04 and 0x05 (9-bit) */
	uint16_t m_printhead;

	/* port 0x06 (4-bit) */
	uint8_t m_pf_motor;

	/* port 0x07 (4-bit) */
	uint8_t m_cr_motor;
};

extern const device_type E05A03;


#endif /* __E05A03_H__ */
