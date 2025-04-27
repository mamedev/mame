// license:BSD-3-Clause
// copyright-holders:K.Wilkins
/************************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *
 *  Written by K.Wilkins (mame@esplexo.co.uk)
 *
 *  (c) K.Wilkins 2000
 *
 ***********************************************************************
 *
 * DSS_ADJUSTMENT        - UI Mapped adjustable input
 * DSS_CONSTANT          - Node based constant - Do we need this ???
 * DSS_INPUT_x           - Input devices
 * DSS_INPUT_STREAM      - Connects external streams to the discrete system
 *
 ************************************************************************/


#define DSS_INPUT__GAIN     DISCRETE_INPUT(0)
#define DSS_INPUT__OFFSET   DISCRETE_INPUT(1)
#define DSS_INPUT__INIT     DISCRETE_INPUT(2)

/************************************************************************
 *
 * DSS_ADJUSTMENT - UI Adjustable constant node to emulate trimmers
 *
 * input[0]    - Enable
 * input[1]    - Minimum value
 * input[2]    - Maximum value
 * input[3]    - Log/Linear 0=Linear !0=Log
 * input[4]    - Input Port number
 * input[5]    -
 * input[6]    -
 *
 ************************************************************************/
#define DSS_ADJUSTMENT__MIN     DISCRETE_INPUT(0)
#define DSS_ADJUSTMENT__MAX     DISCRETE_INPUT(1)
#define DSS_ADJUSTMENT__LOG     DISCRETE_INPUT(2)
#define DSS_ADJUSTMENT__PORT    DISCRETE_INPUT(3)
#define DSS_ADJUSTMENT__PMIN    DISCRETE_INPUT(4)
#define DSS_ADJUSTMENT__PMAX    DISCRETE_INPUT(5)

DISCRETE_STEP(dss_adjustment)
{
	int32_t  rawportval = m_port->read();

	/* only recompute if the value changed from last time */
	if (UNEXPECTED(rawportval != m_lastpval))
	{
		double portval   = (double)(rawportval - m_pmin) * m_pscale;
		double scaledval = portval * m_scale + m_min;

		m_lastpval = rawportval;
		if (DSS_ADJUSTMENT__LOG == 0)
			set_output(0,  scaledval);
		else
			set_output(0,  pow(10, scaledval));
	}
}

DISCRETE_RESET(dss_adjustment)
{
	double min, max;

	m_port = m_device->machine().root_device().ioport(m_device->siblingtag((const char *)this->custom_data()).c_str());
	if (m_port == nullptr)
		fatalerror("DISCRETE_ADJUSTMENT - NODE_%d has invalid tag\n", this->index());

	m_lastpval = 0x7fffffff;
	m_pmin     = DSS_ADJUSTMENT__PMIN;
	m_pscale   = 1.0 / (double)(DSS_ADJUSTMENT__PMAX - DSS_ADJUSTMENT__PMIN);

	/* linear scale */
	if (DSS_ADJUSTMENT__LOG == 0)
	{
		m_min   = DSS_ADJUSTMENT__MIN;
		m_scale = DSS_ADJUSTMENT__MAX - DSS_ADJUSTMENT__MIN;
	}

	/* logarithmic scale */
	else
	{
		/* force minimum and maximum to be > 0 */
		min = (DSS_ADJUSTMENT__MIN > 0) ? DSS_ADJUSTMENT__MIN : 1;
		max = (DSS_ADJUSTMENT__MAX > 0) ? DSS_ADJUSTMENT__MAX : 1;
		m_min   = log10(min);
		m_scale = log10(max) - log10(min);
	}

	this->step();
}


/************************************************************************
 *
 * DSS_CONSTANT - This is a constant.
 *
 * input[0]    - Constant value
 *
 ************************************************************************/
#define DSS_CONSTANT__INIT  DISCRETE_INPUT(0)

DISCRETE_RESET(dss_constant)
{
	set_output(0, DSS_CONSTANT__INIT);
}


/************************************************************************
 *
 * DSS_INPUT_x    - Receives input from discrete_sound_w
 *
 * input[0]    - Gain value
 * input[1]    - Offset value
 * input[2]    - Starting Position
 * input[3]    - Current data value
 *
 ************************************************************************/

DISCRETE_RESET(dss_input_data)
{
	m_gain = DSS_INPUT__GAIN;
	m_offset = DSS_INPUT__OFFSET;

	m_data = DSS_INPUT__INIT;
	set_output(0,  m_data * m_gain + m_offset);
}

void DISCRETE_CLASS_FUNC(dss_input_data, input_write)(int sub_node, uint8_t data )
{
	uint8_t new_data    = 0;

	new_data = data;

	if (m_data != new_data)
	{
		/* Bring the system up to now */
		m_device->update_to_current_time();

		m_data = new_data;

		/* Update the node output here so we don't have to do it each step */
		set_output(0,  m_data * m_gain + m_offset);
	}
}

DISCRETE_RESET(dss_input_logic)
{
	m_gain = DSS_INPUT__GAIN;
	m_offset = DSS_INPUT__OFFSET;

	m_data = (DSS_INPUT__INIT == 0) ? 0 : 1;
	set_output(0,  m_data * m_gain + m_offset);
}

void DISCRETE_CLASS_FUNC(dss_input_logic, input_write)(int sub_node, uint8_t data )
{
	uint8_t new_data    = 0;

	new_data =  data ? 1 : 0;

	if (m_data != new_data)
	{
		/* Bring the system up to now */
		m_device->update_to_current_time();

		m_data = new_data;

		/* Update the node output here so we don't have to do it each step */
		set_output(0,  m_data * m_gain + m_offset);
	}
}

DISCRETE_RESET(dss_input_not)
{
	m_gain = DSS_INPUT__GAIN;
	m_offset = DSS_INPUT__OFFSET;

	m_data = (DSS_INPUT__INIT == 0) ? 1 : 0;
	set_output(0,  m_data * m_gain + m_offset);
}

void DISCRETE_CLASS_FUNC(dss_input_not, input_write)(int sub_node, uint8_t data )
{
	uint8_t new_data    = 0;

	new_data = data ? 0 : 1;

	if (m_data != new_data)
	{
		/* Bring the system up to now */
		m_device->update_to_current_time();

		m_data = new_data;

		/* Update the node output here so we don't have to do it each step */
		set_output(0,  m_data * m_gain + m_offset);
	}
}

DISCRETE_STEP(dss_input_pulse)
{
	/* Set a valid output */
	set_output(0,  m_data);
	/* Reset the input to default for the next cycle */
	/* node order is now important */
	m_data = DSS_INPUT__INIT;
}

DISCRETE_RESET(dss_input_pulse)
{
	m_data = (DSS_INPUT__INIT == 0) ? 0 : 1;
	set_output(0,  m_data);
}

void DISCRETE_CLASS_FUNC(dss_input_pulse, input_write)(int sub_node, uint8_t data )
{
	uint8_t new_data    = 0;

	new_data =  data ? 1 : 0;

	if (m_data != new_data)
	{
		/* Bring the system up to now */
		m_device->update_to_current_time();
		m_data = new_data;
	}
}

/************************************************************************
 *
 * DSS_INPUT_STREAM    - Receives input from a routed stream
 *
 * input[0]    - Input stream number
 * input[1]    - Gain value
 * input[2]    - Offset value
 *
 ************************************************************************/
#define DSS_INPUT_STREAM__STREAM    DISCRETE_INPUT(0)
#define DSS_INPUT_STREAM__GAIN      DISCRETE_INPUT(1)
#define DSS_INPUT_STREAM__OFFSET    DISCRETE_INPUT(2)

void discrete_dss_input_stream_node::stream_generate(sound_stream &stream)
{
	stream.fill(0, m_data * (1.0 / 32768.0));
}
DISCRETE_STEP(dss_input_stream)
{
	/* the context pointer is set to point to the current input stream data in discrete_stream_update */
	if (EXPECTED(m_buffer_stream))
	{
		set_output(0,  m_buffer_stream->get(m_stream_in_number, m_inview_sample) * 32768.0 * m_gain + m_offset);
		m_inview_sample++;
	}
	else
		set_output(0,  0);
}

DISCRETE_RESET(dss_input_stream)
{
	m_data = 0;
}

void DISCRETE_CLASS_FUNC(dss_input_stream, input_write)(int sub_node, uint8_t data )
{
	uint8_t new_data    = 0;

	new_data =  data;

	if (m_data != new_data)
	{
		if (m_is_buffered)
		{
			/* Bring the system up to now */
			m_buffer_stream->update();

			m_data = new_data;
		}
		else
		{
			/* Bring the system up to now */
			m_device->update_to_current_time();

			m_data = new_data;

			/* Update the node output here so we don't have to do it each step */
			set_output(0,  new_data * m_gain + m_offset);
		}
	}
}

DISCRETE_START(dss_input_stream)
{
	discrete_base_node::start();

	/* Stream out number is set during start */
	m_stream_in_number = DSS_INPUT_STREAM__STREAM;
	m_gain = DSS_INPUT_STREAM__GAIN;
	m_offset = DSS_INPUT_STREAM__OFFSET;

	m_is_buffered = is_buffered();
	m_buffer_stream = nullptr;
}

void DISCRETE_CLASS_NAME(dss_input_stream)::stream_start(void)
{
	if (m_is_buffered)
	{
		/* stream_buffered input only supported for sound devices */
		discrete_sound_device *snd_device = downcast<discrete_sound_device *>(m_device);
		//assert(DSS_INPUT_STREAM__STREAM < snd_device->m_input_stream_list.count());

		m_buffer_stream = m_device->machine().sound().stream_alloc(*snd_device, 0, 1, this->sample_rate(), stream_update_delegate(&discrete_dss_input_stream_node::stream_generate,this), STREAM_DEFAULT_FLAGS);

		// WTF?
		//		snd_device->get_stream()->set_input(m_stream_in_number, m_buffer_stream);
	}
}
