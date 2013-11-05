/*****************************************************************************
 *
 * machine/tpi6525.h
 *
 * mos tri port interface 6525
 * mos triple interface adapter 6523
 *
 * peter.trauner@jk.uni-linz.ac.at
 *
 * used in commodore b series
 * used in commodore c1551 floppy disk drive
 *
 * tia6523 is a tpi6525 without control register!?
 *
 * tia6523
 *   only some lines of port b and c are in the pinout!
 *
 * connector to floppy c1551 (delivered with c1551 as c16 expansion)
 *   port a for data read/write
 *   port b
 *   0 status 0
 *   1 status 1
 *   port c
 *   6 dav output edge data on port a available
 *   7 ack input edge ready for next datum
 *
 ****************************************************************************/

#ifndef __TPI6525_H__
#define __TPI6525_H__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct tpi6525_interface
{
	devcb_write_line    m_out_irq_cb;

	devcb_read8         m_in_pa_cb;
	devcb_write8        m_out_pa_cb;

	devcb_read8         m_in_pb_cb;
	devcb_write8        m_out_pb_cb;

	devcb_read8         m_in_pc_cb;
	devcb_write8        m_out_pc_cb;

	devcb_write_line    m_out_ca_cb;
	devcb_write_line    m_out_cb_cb;
};


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

class tpi6525_device : public device_t,
								public tpi6525_interface
{
public:
	tpi6525_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tpi6525_device() {}

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( i0_w );
	DECLARE_WRITE_LINE_MEMBER( i1_w );
	DECLARE_WRITE_LINE_MEMBER( i2_w );
	DECLARE_WRITE_LINE_MEMBER( i3_w );
	DECLARE_WRITE_LINE_MEMBER( i4_w );

	DECLARE_READ8_MEMBER( pa_r );
	DECLARE_READ8_MEMBER( pb_r );
	DECLARE_READ8_MEMBER( pc_r );
	DECLARE_WRITE8_MEMBER( pa_w );
	DECLARE_WRITE8_MEMBER( pb_w );
	DECLARE_WRITE8_MEMBER( pc_w );

	UINT8 get_ddr_a();
	UINT8 get_ddr_b();
	UINT8 get_ddr_c();

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	devcb_resolved_write_line   m_out_irq_func;
	devcb_resolved_read8        m_in_pa_func;
	devcb_resolved_write8       m_out_pa_func;
	devcb_resolved_read8        m_in_pb_func;
	devcb_resolved_write8       m_out_pb_func;
	devcb_resolved_read8        m_in_pc_func;
	devcb_resolved_write8       m_out_pc_func;
	devcb_resolved_write_line   m_out_ca_func;
	devcb_resolved_write_line   m_out_cb_func;

	UINT8 m_port_a, m_ddr_a, m_in_a;
	UINT8 m_port_b, m_ddr_b, m_in_b;
	UINT8 m_port_c, m_ddr_c, m_in_c;

	UINT8 m_ca_level, m_cb_level, m_interrupt_level;

	UINT8 m_cr;
	UINT8 m_air;

	UINT8 m_irq_level[5];

	void set_interrupt();
	void clear_interrupt();
};

extern const device_type TPI6525;


#define MCFG_TPI6525_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, TPI6525, 0) \
	MCFG_DEVICE_CONFIG(_intrf)



#endif /* __TPI6525_H__ */
