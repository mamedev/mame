/*********************************************************************

    bitbngr.h

    TRS style "bitbanger" serial port

*********************************************************************/

#ifndef __BITBNGR_H__
#define __BITBNGR_H__


enum
{
	BITBANGER_PRINTER           = 0,
	BITBANGER_MODEM,
	BITBANGER_MODE_MAX,

	BITBANGER_150               = 0,
	BITBANGER_300,
	BITBANGER_600,
	BITBANGER_1200,
	BITBANGER_2400,
	BITBANGER_4800,
	BITBANGER_9600,
	BITBANGER_14400,
	BITBANGER_19200,
	BITBANGER_28800,
	BITBANGER_38400,
	BITBANGER_57600,
	BITBANGER_115200,
	BITBANGER_BAUD_MAX,

	BITBANGER_NEG40PERCENT  = 0,
	BITBANGER_NEG35PERCENT,
	BITBANGER_NEG30PERCENT,
	BITBANGER_NEG25PERCENT,
	BITBANGER_NEG20PERCENT,
	BITBANGER_NEG15PERCENT,
	BITBANGER_NEG10PERCENT,
	BITBANGER_NEG5PERCENT,
	BITBANGER_0PERCENT,
	BITBANGER_POS5PERCENT,
	BITBANGER_POS10PERCENT,
	BITBANGER_POS15PERCENT,
	BITBANGER_POS20PERCENT,
	BITBANGER_POS25PERCENT,
	BITBANGER_POS30PERCENT,
	BITBANGER_POS35PERCENT,
	BITBANGER_POS40PERCENT,
	BITBANGER_TUNE_MAX
};

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MCFG_BITBANGER_INPUT_CB(_devcb) \
	devcb = &bitbanger_device::set_input_callback(*device, DEVCB2_##_devcb);

#define MCFG_BITBANGER_DEFAULT_MODE(_mode) \
	bitbanger_device::set_default_mode(*device, _mode);
	
#define MCFG_BITBANGER_DEFAULT_BAUD(_baud) \
	bitbanger_device::set_default_baud(*device, _baud);
	
#define MCFG_BITBANGER_DEFAULT_TUNE(_tune) \
	bitbanger_device::set_default_tune(*device, _tune);

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


class bitbanger_device :    public device_t,
							public device_image_interface
{
public:
	// construction/destruction
	bitbanger_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb2_base &set_input_callback(device_t &device, _Object object) { return downcast<bitbanger_device &>(device).m_input_cb.set_callback(object); }
	static void set_default_mode(device_t &device, int default_mode) { downcast<bitbanger_device &>(device).m_mode = default_mode; }
	static void set_default_baud(device_t &device, int default_baud) { downcast<bitbanger_device &>(device).m_baud = default_baud; }
	static void set_default_tune(device_t &device, int default_tune) { downcast<bitbanger_device &>(device).m_tune = default_tune; }
	
	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();
	virtual bool call_create(int format_type, option_resolution *format_options);

	// image device
	virtual iodevice_t image_type() const { return IO_SERIAL; }
	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const { return 1; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *file_extensions() const { return ""; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	// outputs data to a bitbanger port
	void output(int value);

	// reads the current input value
	int input(void) const { return m_current_input; }

	// ui functions
	const char *mode_string(void);
	const char *baud_string(void);
	const char *tune_string(void);
	bool inc_mode(bool test);
	bool dec_mode(bool test);
	bool inc_tune(bool test);
	bool dec_tune(bool test);
	bool inc_baud(bool test);
	bool dec_baud(bool test);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void device_config_complete();

private:
	// constants
	static const device_timer_id TIMER_OUTPUT = 0;
	static const device_timer_id TIMER_INPUT = 1;

	// methods
	void native_output(UINT8 data);
	UINT32 native_input(void *buffer, UINT32 length);
	void bytes_to_bits_81N(void);
	void timer_input(void);
	void timer_output(void);
	float tune_value(void);
	UINT32 baud_value(void);
	void set_input_line(UINT8 line);

	// variables
	emu_timer *                 m_output_timer;
	emu_timer *                 m_input_timer;
	
	devcb2_write_line    	    m_input_cb; /* callback to driver */
	int                         m_output_value;
	int                         m_build_count;
	int                         m_build_byte;
	attotime                    m_idle_delay;
	attotime                    m_current_baud;
	UINT32                      m_input_buffer_size;
	UINT32                      m_input_buffer_cursor;
	int                         m_mode; /* emulating a printer or modem */
	int                         m_baud; /* output bits per second */
	int                         m_tune; /* fine tune adjustment to the baud */
	UINT8                       m_current_input;
	UINT8                       m_input_buffer[1000];
};

// device type definition
extern const device_type BITBANGER;

// device iterator
typedef device_type_iterator<&device_creator<bitbanger_device>, bitbanger_device> bitbanger_device_iterator;

#endif /* __BITBNGR_H__ */
