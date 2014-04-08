/*****************************************************************************
 *
 * machine/upd7002.h
 *
 * uPD7002 Analogue to Digital Converter
 *
 * Driver by Gordon Jefferyes <mess_bbc@gjeffery.dircon.co.uk>
 *
 ****************************************************************************/

#ifndef UPD7002_H_
#define UPD7002_H_

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef int (*upd7002_get_analogue_func)(device_t *device, int channel_number);
#define UPD7002_GET_ANALOGUE(name)  int name(device_t *device, int channel_number )

typedef void (*upd7002_eoc_func)(device_t *device, int data);
#define UPD7002_EOC(name)   void name(device_t *device, int data )


struct upd7002_interface
{
	upd7002_get_analogue_func get_analogue_func;
	upd7002_eoc_func          eoc_func;
};

/***************************************************************************
    MACROS
***************************************************************************/

class upd7002_device : public device_t,
								public upd7002_interface
{
public:
	upd7002_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~upd7002_device() {}

	DECLARE_READ8_MEMBER ( eoc_r );
	DECLARE_READ8_MEMBER ( read );
	DECLARE_WRITE8_MEMBER ( write );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	// internal state

	/* Status Register
	    D0 and D1 define the currently selected input channel
	    D2 flag output
	    D3 0 = 8 bit mode   1 = 12 bit mode
	    D4 2nd MSB of conversion
	    D5     MSB of conversion
	    D6 0 = busy, 1 = not busy    (~busy)
	    D7 0 = conversion completed, 1 = conversion not completed  (~EOC)
	*/
	int m_status;

	/* High data byte
	    This byte contains the 8 most significant bits of the analogue to digital conversion. */
	int m_data1;

	/* Low data byte
	    In 12 bit mode: Bits 7 to 4 define the four low order bits of the conversion.
	    In  8 bit mode. All bits 7 to 4 are inaccurate.
	    Bits 3 to 0 are always set to low. */
	int m_data0;


	/* temporary store of the next A to D conversion */
	int m_digitalvalue;

	/* this counter is used to check a full end of conversion has been reached
	if the uPD7002 is half way through one conversion and a new conversion is requested
	the counter at the end of the first conversion will not match and not be processed
	only then at the end of the second conversion will the conversion complete function run */
	int m_conversion_counter;

	enum
	{
		TIMER_CONVERSION_COMPLETE
	};
};

extern const device_type UPD7002;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_UPD7002_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, UPD7002, 0) \
	MCFG_DEVICE_CONFIG(_intrf)


#endif /* UPD7002_H_ */
