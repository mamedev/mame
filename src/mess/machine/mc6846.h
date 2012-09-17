/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Motorola 6846 timer emulation.

**********************************************************************/

#ifndef MC6846_H
#define MC6846_H

class mc6846_device : public device_t
{
public:
	mc6846_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~mc6846_device() { global_free(m_token); }

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

extern const device_type MC6846;


/* ---------- configuration ------------ */

struct mc6846_interface
{
  /* CPU write to the outside through chip */
  devcb_write8 out_port_func;  /* 8-bit output */
  devcb_write8 out_cp1_func;   /* 1-bit output */
  devcb_write8 out_cp2_func;   /* 1-bit output */

  /* CPU read from the outside through chip */
  devcb_read8 in_port_func; /* 8-bit input */

  /* asynchronous timer output to outside world */
  devcb_write8 out_cto_func; /* 1-bit output */

  /* timer interrupt */
  devcb_write_line irq_func;
};


#define MCFG_MC6846_ADD(_tag, _intrf) \
  MCFG_DEVICE_ADD(_tag, MC6846, 0)	      \
  MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_MC6846_MODIFY(_tag, _intrf) \
  MCFG_DEVICE_MODIFY(_tag)	      \
  MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_MC6846_REMOVE(_tag)		\
  MCFG_DEVICE_REMOVE(_tag)


/* ---------- functions ------------ */
/* interface to CPU via address/data bus*/
extern DECLARE_READ8_DEVICE_HANDLER  ( mc6846_r );
extern DECLARE_WRITE8_DEVICE_HANDLER ( mc6846_w );

/* asynchronous write from outside world into interrupt-generating pins */
extern void mc6846_set_input_cp1 ( device_t *device, int data );
extern void mc6846_set_input_cp2 ( device_t *device, int data );

/* polling from outside world */
extern UINT8  mc6846_get_output_port ( device_t *device );
extern UINT8  mc6846_get_output_cto  ( device_t *device );
extern UINT8  mc6846_get_output_cp2  ( device_t *device );

/* partial access to internal state */
extern UINT16 mc6846_get_preset ( device_t *device ); /* timer interval - 1 in us */

#endif

