/************************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *
 *  Written by Keith Wilkins (mame@esplexo.co.uk)
 *
 *  (c) K.Wilkins 2000
 *  (c) D.Renaud 2003-2004
 *
 ************************************************************************
 *
 * DSO_OUTPUT            - Output node
 * DSO_TASK              - Task node
 *
 * Task and list routines
 * 
 ************************************************************************/

struct dso_csvlog_context
{
	FILE *csv_file;
	INT64 sample_num;
	char name[32];
};

struct dso_wavelog_context
{
	wav_file *wav_file;
	char name[32];
};

static DISCRETE_STEP( dso_task )
{
	discrete_task_context *ctx =  (discrete_task_context *) node->context;
	int i;
	
	for (i = 0; i < ctx->numbuffered; i++)
		*(ctx->ptr[i]++) = DISCRETE_INPUT(i);
}

static DISCRETE_RESET( dso_task )
{
	/* nothing to do - just avoid being stepped */
}

static DISCRETE_STEP( dso_output )
{
	stream_sample_t **output = (stream_sample_t **) &node->context;
	double val;

	/* Add gain to the output and put into the buffers */
	/* Clipping will be handled by the main sound system */
	val = DISCRETE_INPUT(0) * DISCRETE_INPUT(1);
	**output = val;
	(*output)++;
}

static DISCRETE_RESET( dso_output )
{
	/* nothing to do - just avoid being stepped */
}

static DISCRETE_START( dso_csvlog )
{
	struct dso_csvlog_context *context = (struct dso_csvlog_context *) node->context;
	int log_num, node_num;

	log_num = node_module_index(node);
	context->sample_num = 0;

	sprintf(context->name, "discrete%s_%d.csv", node->info->device->tag, log_num);
	context->csv_file = fopen(context->name, "w");
	/* Output some header info */
	fprintf(context->csv_file, "\"MAME Discrete System Node Log\"\n");
	fprintf(context->csv_file, "\"Log Version\", 1.0\n");
	fprintf(context->csv_file, "\"Sample Rate\", %d\n", node->info->sample_rate);
	fprintf(context->csv_file, "\n");
	fprintf(context->csv_file, "\"Sample\"");
	for (node_num = 0; node_num < node->active_inputs; node_num++)
	{
		fprintf(context->csv_file, ", \"NODE_%2d\"", NODE_INDEX(node->block->input_node[node_num]));
	}
	fprintf(context->csv_file, "\n");
}

static DISCRETE_STOP( dso_csvlog )
{
	struct dso_csvlog_context *context = (struct dso_csvlog_context *) node->context;
	
	/* close any csv files */
	if (context->csv_file)
		fclose(context->csv_file);
}

static DISCRETE_STEP( dso_csvlog )
{
	struct dso_csvlog_context *context = (struct dso_csvlog_context *) node->context;
	int nodenum;

	/* Dump any csv logs */
	fprintf(context->csv_file, "%" I64FMT "d", ++context->sample_num);
	for (nodenum = 0; nodenum < node->active_inputs; nodenum++)
	{
		fprintf(context->csv_file, ", %f", *node->input[nodenum]);
	}
	fprintf(context->csv_file, "\n");
}

static DISCRETE_START( dso_wavelog )
{
	struct dso_wavelog_context *context = (struct dso_wavelog_context *) node->context;
	int log_num;

	log_num = node_module_index(node);
	sprintf(context->name, "discrete%s_%d.wav", node->info->device->tag, log_num);
	context->wav_file = wav_open(context->name, node->info->sample_rate, node->active_inputs/2);
}

static DISCRETE_STOP( dso_wavelog )
{
	struct dso_wavelog_context *context = (struct dso_wavelog_context *) node->context;

	/* close any wave files */
	if (context->wav_file)
		wav_close(context->wav_file);	
}

static DISCRETE_STEP( dso_wavelog )
{
	struct dso_wavelog_context *context = (struct dso_wavelog_context *) node->context;
	double val;
	INT16 wave_data_l, wave_data_r;

	/* Dump any wave logs */
	/* get nodes to be logged and apply gain, then clip to 16 bit */
	val = DISCRETE_INPUT(0) * DISCRETE_INPUT(1);
	val = (val < -32768) ? -32768 : (val > 32767) ? 32767 : val;
	wave_data_l = (INT16)val;
	if (node->active_inputs == 2)
	{
		/* DISCRETE_WAVELOG1 */
		wav_add_data_16(context->wav_file, &wave_data_l, 1);
	}
	else
	{
		/* DISCRETE_WAVELOG2 */
		val = DISCRETE_INPUT(2) * DISCRETE_INPUT(3);
		val = (val < -32768) ? -32768 : (val > 32767) ? 32767 : val;
		wave_data_r = (INT16)val;

		wav_add_data_16lr(context->wav_file, &wave_data_l, &wave_data_r, 1);
	}
}
