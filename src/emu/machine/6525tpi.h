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
	devcb_write_line	out_irq_func;

	devcb_read8			in_pa_func;
	devcb_write8		out_pa_func;

	devcb_read8			in_pb_func;
	devcb_write8		out_pb_func;

	devcb_read8			in_pc_func;
	devcb_write8		out_pc_func;

	devcb_write_line	out_ca_func;
	devcb_write_line	out_cb_func;
};


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

class tpi6525_device : public device_t
{
public:
	tpi6525_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tpi6525_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type TPI6525;


#define MCFG_TPI6525_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, TPI6525, 0) \
	MCFG_DEVICE_CONFIG(_intrf)


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

READ8_DEVICE_HANDLER( tpi6525_r );
WRITE8_DEVICE_HANDLER( tpi6525_w );

READ8_DEVICE_HANDLER( tpi6525_porta_r );
WRITE8_DEVICE_HANDLER( tpi6525_porta_w );

READ8_DEVICE_HANDLER( tpi6525_portb_r );
WRITE8_DEVICE_HANDLER( tpi6525_portb_w );

READ8_DEVICE_HANDLER( tpi6525_portc_r );
WRITE8_DEVICE_HANDLER( tpi6525_portc_w );

WRITE_LINE_DEVICE_HANDLER( tpi6525_i0_w );
WRITE_LINE_DEVICE_HANDLER( tpi6525_i1_w );
WRITE_LINE_DEVICE_HANDLER( tpi6525_i2_w );
WRITE_LINE_DEVICE_HANDLER( tpi6525_i3_w );
WRITE_LINE_DEVICE_HANDLER( tpi6525_i4_w );

UINT8 tpi6525_get_ddr_a(device_t *device);
UINT8 tpi6525_get_ddr_b(device_t *device);
UINT8 tpi6525_get_ddr_c(device_t *device);


#endif /* __TPI6525_H__ */
