/***************************************************************************

    E05A03 Gate Array (used in the Epson LX-800)

    license: MAME, GPL-2.0+
    copyright-holders: Dirk Best

***************************************************************************/

#ifndef __E05A03_H__
#define __E05A03_H__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct e05a03_interface
{
	devcb_read8 m_in_data_cb;

	devcb_write_line m_out_nlq_lp_cb;
	devcb_write_line m_out_pe_lp_cb;
	devcb_write_line m_out_pe_cb;
	devcb_write_line m_out_reso_cb;
};


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

class e05a03_device : public device_t,
								public e05a03_interface
{
public:
	e05a03_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~e05a03_device() {}

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

	WRITE_LINE_MEMBER( home_w ); /* home position signal */
	WRITE_LINE_MEMBER( fire_w ); /* printhead solenoids trigger */
	WRITE_LINE_MEMBER( strobe_w );
	READ_LINE_MEMBER( busy_r );
	WRITE_LINE_MEMBER( resi_w ); /* reset input */
	WRITE_LINE_MEMBER( init_w ); /* centronics init */

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state

	/* 24-bit shift register, port 0x00, 0x01 and 0x02 */
	UINT32 m_shift;

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
	UINT16 m_printhead;

	/* port 0x06 (4-bit) */
	UINT8 m_pf_motor;

	/* port 0x07 (4-bit) */
	UINT8 m_cr_motor;

	/* callbacks */
	devcb_resolved_write_line m_out_nlq_lp_func; /* pin 2, nlq lamp output */
	devcb_resolved_write_line m_out_pe_lp_func;  /* pin 3, paper empty lamp output */
	devcb_resolved_write_line m_out_reso_func;   /* pin 25, reset output */
	devcb_resolved_write_line m_out_pe_func;     /* pin 35, centronics pe output */
	devcb_resolved_read8 m_in_data_func;         /* pin 47-54, centronics data input */
};

extern const device_type E05A03;


#define MCFG_E05A03_ADD(_tag, _intf) \
	MCFG_DEVICE_ADD(_tag, E05A03, 0) \
	MCFG_DEVICE_CONFIG(_intf)


#endif /* __E05A03_H__ */
