/************************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *
 *  Written by Keith Wilkins (mame@esplexo.co.uk)
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


#define DSS_INPUT__GAIN		DISCRETE_INPUT(0)
#define DSS_INPUT__OFFSET	DISCRETE_INPUT(1)
#define DSS_INPUT__INIT		DISCRETE_INPUT(2)


struct dss_adjustment_context
{
	const input_port_config *port;
	INT32		lastpval;
	INT32		pmin;
	double		pscale;
	double		min;
	double		scale;
};

struct dss_input_context
{
	stream_sample_t *ptr;			/* current in ptr for stream */
	double 		gain;				/* node gain */
	double		offset;				/* node offset */
	UINT8		data;				/* data written */
	UINT8		is_stream;
	UINT8		is_buffered;
	UINT32		stream_in_number;
	/* the buffer stream */
	sound_stream *buffer_stream;
};

INLINE discrete_info *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == SOUND);
	assert(sound_get_type(device) == SOUND_DISCRETE);
	return (discrete_info *)device->token;
}

READ8_DEVICE_HANDLER(discrete_sound_r)
{
	discrete_info    *info = get_safe_token(device);
	node_description *node = discrete_find_node(info, offset);

	UINT8 data = 0;

	/* Read the node input value if allowed */
	if (node)
	{
		/* Bring the system up to now */
		stream_update(info->discrete_stream);

		data = (UINT8) node->output[NODE_CHILD_NODE_NUM(offset)];
	}
	else
		fatalerror("discrete_sound_r read from non-existent NODE_%02d\n", offset-NODE_00);

    return data;
}


WRITE8_DEVICE_HANDLER(discrete_sound_w)
{
	discrete_info    *info = get_safe_token(device);
	node_description *node = discrete_find_node(info, offset);

	/* Update the node input value if it's a proper input node */
	if (node)
	{
		struct dss_input_context *context = (struct dss_input_context *)node->context;
		UINT8 new_data    = 0;

		switch (node->module->type)
		{
			case DSS_INPUT_DATA:
			case DSS_INPUT_BUFFER:
				new_data = data;
				break;
			case DSS_INPUT_LOGIC:
			case DSS_INPUT_PULSE:
				new_data = data ? 1 : 0;
				break;
			case DSS_INPUT_NOT:
				new_data = data ? 0 : 1;
				break;
		}

		if (context->data != new_data)
		{
			if (context->is_buffered)
			{
				/* Bring the system up to now */
				stream_update(context->buffer_stream);

				context->data = new_data;
			}
			else
			{
				/* Bring the system up to now */
				stream_update(info->discrete_stream);

				context->data = new_data;

				/* Update the node output here so we don't have to do it each step */
				node->output[0] = new_data * context->gain + context->offset;
			}
		}
	}
	else
	{
		discrete_log(info, "discrete_sound_w write to non-existent NODE_%02d\n", offset-NODE_00);
	}
}

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
#define DSS_ADJUSTMENT__MIN		DISCRETE_INPUT(0)
#define DSS_ADJUSTMENT__MAX		DISCRETE_INPUT(1)
#define DSS_ADJUSTMENT__LOG		DISCRETE_INPUT(2)
#define DSS_ADJUSTMENT__PORT	DISCRETE_INPUT(3)
#define DSS_ADJUSTMENT__PMIN	DISCRETE_INPUT(4)
#define DSS_ADJUSTMENT__PMAX	DISCRETE_INPUT(5)

static DISCRETE_STEP(dss_adjustment)
{
	struct dss_adjustment_context *context = (struct dss_adjustment_context *)node->context;

	INT32  rawportval = input_port_read_direct(context->port);

	/* only recompute if the value changed from last time */
	if (UNEXPECTED(rawportval != context->lastpval))
	{
		double portval   = (double)(rawportval - context->pmin) * context->pscale;
		double scaledval = portval * context->scale + context->min;

		context->lastpval = rawportval;
		if (DSS_ADJUSTMENT__LOG == 0)
			node->output[0] = scaledval;
		else
			node->output[0] = pow(10, scaledval);
	}
}

static DISCRETE_RESET(dss_adjustment)
{
	struct dss_adjustment_context *context = (struct dss_adjustment_context *)node->context;

	double min, max;

	if (node->custom)
	{
		context->port = input_port_by_tag(node->info->device->machine->portconfig, (const char *)node->custom);
		if (context->port == NULL)
			fatalerror("DISCRETE_ADJUSTMENT_TAG - NODE_%d has invalid tag", NODE_BLOCKINDEX(node));
	}
	else
		context->port = input_port_by_index(node->info->device->machine->portconfig, DSS_ADJUSTMENT__PORT);

	context->lastpval = 0x7fffffff;
	context->pmin     = DSS_ADJUSTMENT__PMIN;
	context->pscale   = 1.0 / (double)(DSS_ADJUSTMENT__PMAX - DSS_ADJUSTMENT__PMIN);

	/* linear scale */
	if (DSS_ADJUSTMENT__LOG == 0)
	{
		context->min   = DSS_ADJUSTMENT__MIN;
		context->scale = DSS_ADJUSTMENT__MAX - DSS_ADJUSTMENT__MIN;
	}

	/* logarithmic scale */
	else
	{
		/* force minimum and maximum to be > 0 */
		min = (DSS_ADJUSTMENT__MIN > 0) ? DSS_ADJUSTMENT__MIN : 1;
		max = (DSS_ADJUSTMENT__MAX > 0) ? DSS_ADJUSTMENT__MAX : 1;
		context->min   = log10(min);
		context->scale = log10(max) - log10(min);
	}

	DISCRETE_STEP_CALL(dss_adjustment);
}


/************************************************************************
 *
 * DSS_CONSTANT - This is a constant.
 *
 * input[0]    - Constant value
 *
 ************************************************************************/
#define DSS_CONSTANT__INIT	DISCRETE_INPUT(0)

static DISCRETE_RESET(dss_constant)
{
	node->output[0]= DSS_CONSTANT__INIT;
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
static DISCRETE_RESET(dss_input)
{
	struct dss_input_context *context = (struct dss_input_context *)node->context;

	context->is_buffered = FALSE;
	context->is_stream = FALSE;
	context->gain = DSS_INPUT__GAIN;
	context->offset = DSS_INPUT__OFFSET;

	switch (node->module->type)
	{
		case DSS_INPUT_DATA:
			context->data = DSS_INPUT__INIT;
			break;
		case DSS_INPUT_LOGIC:
		case DSS_INPUT_PULSE:
			context->data = (DSS_INPUT__INIT == 0) ? 0 : 1;
			break;
		case DSS_INPUT_NOT:
			context->data = (DSS_INPUT__INIT == 0) ? 1 : 0;
			break;
	}
	node->output[0] = context->data * context->gain + context->offset;
}

static DISCRETE_STEP(dss_input_pulse)
{
	struct dss_input_context *context = (struct dss_input_context *)node->context;

	/* Set a valid output */
	node->output[0] = context->data;
	/* Reset the input to default for the next cycle */
	/* node order is now important */
	context->data = DSS_INPUT__INIT;
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
#define DSS_INPUT_STREAM__STREAM	DISCRETE_INPUT(0)
#define DSS_INPUT_STREAM__GAIN		DISCRETE_INPUT(1)
#define DSS_INPUT_STREAM__OFFSET	DISCRETE_INPUT(2)

static DISCRETE_STEP(dss_input_stream)
{
	/* the context pointer is set to point to the current input stream data in discrete_stream_update */
	struct dss_input_context *context = (struct dss_input_context *)node->context;

	if (EXPECTED(context->ptr))
	{
		node->output[0] = (*context->ptr) * context->gain + context->offset;
		context->ptr++;
	}
	else
		node->output[0] = 0;
}

static DISCRETE_RESET(dss_input_stream)
{
	struct dss_input_context *context = (struct dss_input_context *)node->context;

	context->ptr = NULL;
	context->data = 0;
}

static DISCRETE_START(dss_input_stream)
{
	struct dss_input_context *context = (struct dss_input_context *)node->context;

	assert(DSS_INPUT_STREAM__STREAM < linked_list_count(node->info->input_list));

	context->is_stream = TRUE;
	/* Stream out number is set during start */
	context->stream_in_number = DSS_INPUT_STREAM__STREAM;
	context->gain = DSS_INPUT_STREAM__GAIN;
	context->offset = DSS_INPUT_STREAM__OFFSET;
	context->ptr = NULL;
	//context->data = 0;

	if (node->block->type == DSS_INPUT_BUFFER)
	{
		context->is_buffered = TRUE;
		context->buffer_stream = stream_create(node->info->device, 0, 1, node->info->sample_rate, (void *) node, buffer_stream_update);

		stream_set_input(node->info->discrete_stream, context->stream_in_number,
			context->buffer_stream, 0, 1.0);
	}
	else
	{
		context->is_buffered = FALSE;
		context->buffer_stream = NULL;
	}
}
