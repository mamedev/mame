// license:BSD-3-Clause
// copyright-holders:Gordon Jefferyes
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

typedef device_delegate<int (int channel_number)> upd7002_get_analogue_delegate;
#define UPD7002_GET_ANALOGUE(name)  int name(int channel_number)

typedef device_delegate<void (int data)> upd7002_eoc_delegate;
#define UPD7002_EOC(name)   void name(int data)


/***************************************************************************
    MACROS
***************************************************************************/

class upd7002_device : public device_t
{
public:
	upd7002_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~upd7002_device() {}

	static void set_get_analogue_callback(device_t &device, upd7002_get_analogue_delegate callback) { downcast<upd7002_device &>(device).m_get_analogue_cb = callback; }
	static void set_eoc_callback(device_t &device, upd7002_eoc_delegate callback) { downcast<upd7002_device &>(device).m_eoc_cb = callback; }

	DECLARE_READ8_MEMBER(eoc_r);
	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

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

	upd7002_get_analogue_delegate m_get_analogue_cb;
	upd7002_eoc_delegate          m_eoc_cb;

	enum
	{
		TIMER_CONVERSION_COMPLETE
	};
};

extern const device_type UPD7002;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_UPD7002_GET_ANALOGUE_CB(_class, _method) \
	upd7002_device::set_get_analogue_callback(*device, upd7002_get_analogue_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_UPD7002_EOC_CB(_class, _method) \
	upd7002_device::set_eoc_callback(*device, upd7002_eoc_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#endif /* UPD7002_H_ */
