// license:BSD-3-Clause
// copyright-holders:K.Wilkins,Derrick Renaud
/************************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *
 *  Written by K.Wilkins (mame@esplexo.co.uk)
 *
 *  (c) K.Wilkins 2000
 *  (c) Derrick Renaud 2003-2004
 *
 ************************************************************************
 *
 * DSO_OUTPUT            - Output node
 * DSO_TASK              - Task node
 *
 * Task and list routines
 *
 ************************************************************************/




/*************************************
 *
 *  Task node (main task execution)
 *
 *************************************/

DISCRETE_START( dso_csvlog )
{
	int log_num, node_num;

	log_num = m_device->same_module_index(*this);
	m_sample_num = 0;

	sprintf(m_name, "discrete_%s_%d.csv", m_device->tag(), log_num);
	m_csv_file = fopen(m_name, "w");
	/* Output some header info */
	fprintf(m_csv_file, "\"MAME Discrete System Node Log\"\n");
	fprintf(m_csv_file, "\"Log Version\", 1.0\n");
	fprintf(m_csv_file, "\"Sample Rate\", %d\n", this->sample_rate());
	fprintf(m_csv_file, "\n");
	fprintf(m_csv_file, "\"Sample\"");
	for (node_num = 0; node_num < this->active_inputs(); node_num++)
	{
		fprintf(m_csv_file, ", \"NODE_%2d\"", NODE_INDEX(this->input_node(node_num)));
	}
	fprintf(m_csv_file, "\n");
}

DISCRETE_STOP( dso_csvlog )
{
	/* close any csv files */
	if (m_csv_file)
		fclose(m_csv_file);
}

DISCRETE_STEP( dso_csvlog )
{
	int nodenum;

	/* Dump any csv logs */
	fprintf(m_csv_file, "%s", string_format("%I64d", ++m_sample_num).c_str());
	for (nodenum = 0; nodenum < this->active_inputs(); nodenum++)
	{
		fprintf(m_csv_file, ", %f", *this->m_input[nodenum]);
	}
	fprintf(m_csv_file, "\n");
}

DISCRETE_RESET( dso_csvlog )
{
	this->step();
}


DISCRETE_START( dso_wavlog )
{
	int log_num;

	log_num = m_device->same_module_index(*this);
	sprintf(m_name, "discrete_%s_%d.wav", m_device->tag(), log_num);
	m_wavfile = wav_open(m_name, sample_rate(), active_inputs()/2);
}

DISCRETE_STOP( dso_wavlog )
{
	/* close any wave files */
	if (m_wavfile)
		wav_close(m_wavfile);
}

DISCRETE_STEP( dso_wavlog )
{
	double val;
	INT16 wave_data_l, wave_data_r;

	/* Dump any wave logs */
	/* get nodes to be logged and apply gain, then clip to 16 bit */
	val = DISCRETE_INPUT(0) * DISCRETE_INPUT(1);
	val = (val < -32768) ? -32768 : (val > 32767) ? 32767 : val;
	wave_data_l = (INT16)val;
	if (this->active_inputs() == 2)
	{
		/* DISCRETE_WAVLOG1 */
		wav_add_data_16(m_wavfile, &wave_data_l, 1);
	}
	else
	{
		/* DISCRETE_WAVLOG2 */
		val = DISCRETE_INPUT(2) * DISCRETE_INPUT(3);
		val = (val < -32768) ? -32768 : (val > 32767) ? 32767 : val;
		wave_data_r = (INT16)val;

		wav_add_data_16lr(m_wavfile, &wave_data_l, &wave_data_r, 1);
	}
}

DISCRETE_RESET( dso_wavlog )
{
	this->step();
}
