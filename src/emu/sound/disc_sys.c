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
	wav_file *wavfile;
	char name[32];
};


/*************************************
 *
 *  Task node (main task execution)
 *
 *************************************/

static void task_check(discrete_task *task, const discrete_task *dest_task)
{
	int inputnum;
	const linked_list_entry *node_entry;
	const linked_list_entry *step_entry;

	/* Determine, which nodes in the task are referenced by nodes in dest_task
     * and add them to the list of nodes to be buffered for further processing
     */
	for (node_entry = task->list; node_entry != NULL; node_entry = node_entry->next)
	{
		const node_description *task_node = (node_description *) node_entry->ptr;

		for (step_entry = dest_task->list; step_entry != NULL; step_entry = step_entry->next)
		{
			node_description *dest_node = (node_description *) step_entry->ptr;

			/* loop over all active inputs */
			for (inputnum = 0; inputnum < dest_node->active_inputs; inputnum++)
			{
				int inputnode = dest_node->block->input_node[inputnum];
				if IS_VALUE_A_NODE(inputnode)
				{
					if (NODE_DEFAULT_NODE(task_node->block->node) == NODE_DEFAULT_NODE(inputnode))
					{
						discrete_source_node *source;
						int i, found = -1;

						for (i = 0; i < task->numbuffered; i++)
							if (task->nodes[i]->block->node == inputnode)
							{
								found = i;
								break;
							}

						if (found<0)
						{
							if (task->numbuffered >= DISCRETE_MAX_TASK_OUTPUTS)
								fatalerror("dso_task_start - Number of maximum buffered nodes exceeded");

							task->node_buf[task->numbuffered] = auto_alloc_array(task_node->info->device->machine, double, 2048);
							task->source[task->numbuffered] = (double *) dest_node->input[inputnum];
							task->nodes[task->numbuffered] = discrete_find_node(task_node->info, inputnode);
							i = task->numbuffered;
							task->numbuffered++;
						}
						discrete_log(task_node->info, "dso_task_start - buffering %d(%d) in task %p group %d referenced by %d group %d", NODE_INDEX(inputnode), NODE_CHILD_NODE_NUM(inputnode), task, task->task_group, NODE_BLOCKINDEX(dest_node), dest_task->task_group);

						/* register into source list */
						source = auto_alloc(dest_node->info->device->machine, discrete_source_node);
						linked_list_add(dest_node->info,
								(linked_list_entry **) (FPTR) &dest_task->source_list,
								source);
						source->task = task;
						source->output_node = i;

						/* point the input to a buffered location */
						dest_node->input[inputnum] = &source->buffer;

					}
				}
			}
		}
	}
}

static DISCRETE_START( dso_task_start )
{
	discrete_task *task =  (discrete_task *) node->context;
	const linked_list_entry *task_entry;

	task->task_group = (int) DISCRETE_INPUT(0);

	if (task->task_group < 0 || task->task_group >= DISCRETE_MAX_TASK_GROUPS)
		fatalerror("discrete_dso_task: illegal task_group %d", task->task_group);

	for (task_entry = node->info->task_list; task_entry != NULL; task_entry = task_entry->next)
	{
		discrete_task *dest_task = (discrete_task *) task_entry->ptr;

		if (task->task_group > dest_task->task_group)
			task_check(dest_task, task);
	}

}

static DISCRETE_STEP( dso_task_end )
{
	discrete_task *task =  (discrete_task *) node->context;
	int i;

	for (i = 0; i < task->numbuffered; i++)
		*(task->ptr[i]++) = *task->source[i];
}

static DISCRETE_STEP( dso_task_start )
{
	const discrete_task *task =  (discrete_task *) node->context;
	const linked_list_entry *entry;

	/* update source node buffer */
	for (entry = task->source_list; entry != 0; entry = entry->next)
	{
		discrete_source_node *sn = (discrete_source_node *) entry->ptr;
		sn->buffer = *sn->ptr++;
	}
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

	sprintf(context->name, "discrete_%s_%d.csv", node->info->device->tag, log_num);
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
	sprintf(context->name, "discrete_%s_%d.wav", node->info->device->tag, log_num);
	context->wavfile = wav_open(context->name, node->info->sample_rate, node->active_inputs/2);
}

static DISCRETE_STOP( dso_wavelog )
{
	struct dso_wavelog_context *context = (struct dso_wavelog_context *) node->context;

	/* close any wave files */
	if (context->wavfile)
		wav_close(context->wavfile);
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
		wav_add_data_16(context->wavfile, &wave_data_l, 1);
	}
	else
	{
		/* DISCRETE_WAVELOG2 */
		val = DISCRETE_INPUT(2) * DISCRETE_INPUT(3);
		val = (val < -32768) ? -32768 : (val > 32767) ? 32767 : val;
		wave_data_r = (INT16)val;

		wav_add_data_16lr(context->wavfile, &wave_data_l, &wave_data_r, 1);
	}
}
