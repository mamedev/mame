// license:BSD-3-Clause
// copyright-holders:K.Wilkins,Couriersud,Derrick Renaud,Frank Palazzolo
#ifndef MAME_SOUND_DISC_CLS_H
#define MAME_SOUND_DISC_CLS_H

#pragma once

#include "wavwrite.h"

/***********************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *
 *  Written by K.Wilkins (mame@esplexo.co.uk)
 *
 *  (c) K.Wilkins 2000
 *
 *  Coding started in November 2000
 *
 *  Additions/bugfix February 2003 - Derrick Renaud, F.Palazzolo, K.Wilkins
 *  Discrete parallel tasks 2009 - Couriersud
 *  Discrete classes 2010        - Couriersud
 *
 ***********************************************************************/

#define DISCRETE_CLASS_NAME(_name) discrete_ ## _name ## _node

#define DISCRETE_CLASS_INPUT(_name, _num)   inline double _name() { return *(m_input[_num]); }

#define DISCRETE_CLASS_CONSTRUCTOR(_name, _base)                        \
	public:                                                             \
		DISCRETE_CLASS_NAME(_name)()                                    \
			: DISCRETE_CLASS_NAME(_base)() { }

#define DISCRETE_CLASS_DESTRUCTOR(_name)                                \
	public:                                                             \
		virtual ~ DISCRETE_CLASS_NAME(_name)() { }

#define  DISCRETE_CLASS_STEP_RESET(_name, _maxout, _priv)               \
class DISCRETE_CLASS_NAME(_name): public discrete_base_node, public discrete_step_interface         \
{                                                                       \
	DISCRETE_CLASS_CONSTRUCTOR(_name, base)                             \
	DISCRETE_CLASS_DESTRUCTOR(_name)                                    \
public:                                                                 \
	virtual void step() override;                                       \
	virtual void reset() override;                                      \
	virtual int max_output() override { return _maxout; }               \
private:                                                                \
	_priv                                                               \
}

#define DISCRETE_CLASS_STEP(_name, _maxout, _priv)                      \
class DISCRETE_CLASS_NAME(_name): public discrete_base_node, public discrete_step_interface             \
{                                                                       \
	DISCRETE_CLASS_CONSTRUCTOR(_name, base)                             \
	DISCRETE_CLASS_DESTRUCTOR(_name)                                    \
public:                                                                 \
	virtual void step() override;                                       \
	virtual void reset() override  { this->step(); }                    \
	virtual int max_output() override { return _maxout; }               \
private:                                                                \
	_priv                                                               \
}

#define DISCRETE_CLASS_RESET(_name, _maxout)                            \
class DISCRETE_CLASS_NAME(_name): public discrete_base_node             \
{                                                                       \
	DISCRETE_CLASS_CONSTRUCTOR(_name, base)                             \
	DISCRETE_CLASS_DESTRUCTOR(_name)                                    \
public:                                                                 \
	virtual void reset() override;                                      \
	virtual int max_output() override { return _maxout; }               \
}

#define DISCRETE_CLASS(_name, _maxout, _priv)                      \
class DISCRETE_CLASS_NAME(_name): public discrete_base_node, public discrete_step_interface             \
{                                                                       \
	DISCRETE_CLASS_DESTRUCTOR(_name)                                    \
	DISCRETE_CLASS_CONSTRUCTOR(_name, base)                             \
public:                                                                 \
	virtual void step() override;                                       \
	virtual void reset() override;                                      \
	virtual void start() override;                                      \
	virtual void stop() override;                                       \
	virtual int max_output() override { return _maxout; }               \
private:                                                                \
	_priv                                                               \
}

class DISCRETE_CLASS_NAME(special): public discrete_base_node
{
	DISCRETE_CLASS_CONSTRUCTOR(special, base)
	DISCRETE_CLASS_DESTRUCTOR(special)
public:
	virtual int max_output() override { return 0; }
};

class DISCRETE_CLASS_NAME(unimplemented): public discrete_base_node
{
	DISCRETE_CLASS_CONSTRUCTOR(unimplemented, base)
	DISCRETE_CLASS_DESTRUCTOR(unimplemented)
public:
	virtual int max_output() override { return 0; }
};

/*************************************
 *
 *  disc_sys.inc
 *
 *************************************/

class DISCRETE_CLASS_NAME(dso_output):  public discrete_base_node,
										public discrete_sound_output_interface,
										public discrete_step_interface
{
	DISCRETE_CLASS_CONSTRUCTOR(dso_output, base)
	DISCRETE_CLASS_DESTRUCTOR(dso_output)
public:
	virtual void step() override {
		/* Add gain to the output and put into the buffers */
		/* Clipping will be handled by the main sound system */
		double val = DISCRETE_INPUT(0) * DISCRETE_INPUT(1);
		m_stream->put(m_stream_output, m_outview_sample++, val * (1.0 / 32768.0));
	}
	virtual int max_output() override { return 0; }
	virtual void set_output_ptr(sound_stream &stream, int output) override { m_stream = &stream; m_stream_output = output; m_outview_sample = 0; }
private:
	sound_stream *m_stream  = nullptr;
	int m_stream_output     = 0;
	u32 m_outview_sample    = 0U;
};

DISCRETE_CLASS(dso_csvlog, 0,
	FILE *m_csv_file = nullptr;
	int64_t m_sample_num = 0;
	char m_name[32];
);

DISCRETE_CLASS(dso_wavlog, 0,
	util::wav_file_ptr m_wavfile;
	char m_name[32];
);

/*************************************
 *
 *  disc_inp.inc
 *
 *************************************/

class DISCRETE_CLASS_NAME(dss_adjustment): public discrete_base_node, public discrete_step_interface
{
	DISCRETE_CLASS_CONSTRUCTOR(dss_adjustment, base)
	DISCRETE_CLASS_DESTRUCTOR(dss_adjustment)
public:
	virtual void step() override;
	virtual void reset() override;
private:
	ioport_port *m_port = 0;
	int32_t                 m_lastpval = 0;
	int32_t                 m_pmin = 0;
	double                  m_pscale = 0.0;
	double                  m_min = 0.0;
	double                  m_scale = 0.0;
};

DISCRETE_CLASS_RESET(dss_constant, 1);

class DISCRETE_CLASS_NAME(dss_input_data): public discrete_base_node, public discrete_input_interface
{
	DISCRETE_CLASS_DESTRUCTOR(dss_input_data)
	DISCRETE_CLASS_CONSTRUCTOR(dss_input_data, base)
public:
	virtual void reset() override;
	virtual void input_write(int sub_node, uint8_t data ) override;
private:
	double      m_gain = 0.0;       /* node gain */
	double      m_offset = 0.0;     /* node offset */
	uint8_t     m_data = 0;         /* data written */
};

class DISCRETE_CLASS_NAME(dss_input_logic): public discrete_base_node, public discrete_input_interface
{
	DISCRETE_CLASS_CONSTRUCTOR(dss_input_logic, base)
	DISCRETE_CLASS_DESTRUCTOR(dss_input_logic)
public:
	virtual void reset() override;
	virtual void input_write(int sub_node, uint8_t data ) override;
private:
	double      m_gain = 0.0;       /* node gain */
	double      m_offset = 0.0;     /* node offset */
	uint8_t     m_data = 0;         /* data written */
};

class DISCRETE_CLASS_NAME(dss_input_not): public discrete_base_node, public discrete_input_interface
{
	DISCRETE_CLASS_CONSTRUCTOR(dss_input_not, base)
	DISCRETE_CLASS_DESTRUCTOR(dss_input_not)
public:
	virtual void reset() override;
	virtual void input_write(int sub_node, uint8_t data ) override;
private:
	double      m_gain = 0.0;       /* node gain */
	double      m_offset = 0.0;     /* node offset */
	uint8_t     m_data = 0;         /* data written */
};

class DISCRETE_CLASS_NAME(dss_input_pulse): public discrete_base_node, public discrete_input_interface, public discrete_step_interface
{
	DISCRETE_CLASS_CONSTRUCTOR(dss_input_pulse, base)
	DISCRETE_CLASS_DESTRUCTOR(dss_input_pulse)
public:
	virtual void step() override;
	virtual void reset() override;
	virtual void input_write(int sub_node, uint8_t data ) override;
private:
	//double      m_gain = 0.0;       /* node gain */
	//double      m_offset = 0.0;     /* node offset */
	uint8_t     m_data = 0;         /* data written */
};

class DISCRETE_CLASS_NAME(dss_input_stream): public discrete_base_node, public discrete_input_interface, public discrete_step_interface
{
	DISCRETE_CLASS_CONSTRUCTOR(dss_input_stream, base)
	DISCRETE_CLASS_DESTRUCTOR(dss_input_stream)
public:
	virtual void step() override;
	virtual void reset() override;
	virtual void start() override;
	virtual void input_write(int sub_node, uint8_t data ) override;
	virtual bool is_buffered() { return false; }

	/* This is called by discrete_sound_device */
	void stream_start();

//protected:
	uint32_t              m_stream_in_number = 0;
	uint32_t              m_inview_sample = 0;
private:
	void stream_generate(sound_stream &stream);

	double      m_gain = 0.0;       /* node gain */
	double      m_offset = 0.0;     /* node offset */
	uint8_t     m_data = 0;         /* data written */
	uint8_t     m_is_buffered = 0;
	/* the buffer stream */
	sound_stream        *m_buffer_stream = nullptr;
};

class DISCRETE_CLASS_NAME(dss_input_buffer): public DISCRETE_CLASS_NAME(dss_input_stream)
{
	DISCRETE_CLASS_CONSTRUCTOR(dss_input_buffer, dss_input_stream)
	DISCRETE_CLASS_DESTRUCTOR(dss_input_buffer)
public:
	virtual bool is_buffered() override { return true; }
};

#include "disc_wav.h"
#include "disc_mth.h"
#include "disc_flt.h"
#include "disc_dev.h"

#endif // MAME_SOUND_DISC_CLS_H
