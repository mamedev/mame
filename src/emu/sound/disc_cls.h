#pragma once

#ifndef __DISC_CLS_H__
#define __DISC_CLS_H__

/***********************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *
 *  Written by Keith Wilkins (mame@esplexo.co.uk)
 *
 *  (c) K.Wilkins 2000
 *
 *  Coding started in November 2000
 *
 *  Additions/bugfix February 2003 - D.Renaud, F.Palazzolo, K.Wilkins
 *  Discrete parallel tasks 2009 - Couriersud
 *
 ***********************************************************************/

#define DISCRETE_CLASS_NAME(_name) discrete_ ## _name ## _node

#if 0
#define DISCRETE_CLASS_CONSTRUCTOR(_name, _base, _ctxsize)				\
		DISCRETE_CLASS_NAME(_name)(discrete_device * pdev, const discrete_sound_block *block) \
		 : DISCRETE_CLASS_NAME(_base)(pdev, block) { m_context_size = _ctxsize; }
#else
#define DISCRETE_CLASS_CONSTRUCTOR(_name, _base)				\
		DISCRETE_CLASS_NAME(_name)() \
		 : DISCRETE_CLASS_NAME(_base)() { }
#endif
#if 1
#define DISCRETE_CLASS_DESTRUCTOR(_name)								\
	public:																\
		virtual ~ DISCRETE_CLASS_NAME(_name)(void) { }
#else
#define DISCRETE_CLASS_DESTRUCTOR(_name)
#endif

#define  DISCRETE_CLASS_STEP_RESETA(_name, _maxout, _priv)				\
class DISCRETE_CLASS_NAME(_name): public discrete_base_node, public discrete_step_interface			\
{																		\
public:																	\
	DISCRETE_CLASS_CONSTRUCTOR(_name, base)								\
	void step(void);													\
	void reset(void);													\
	int max_output(void) { return _maxout; }							\
	DISCRETE_CLASS_DESTRUCTOR(_name)									\
private:																\
	_priv																\
}

#define DISCRETE_CLASS_STEPA(_name, _maxout, _priv)	 				\
class DISCRETE_CLASS_NAME(_name): public discrete_base_node, public discrete_step_interface				\
{																		\
public:																	\
	DISCRETE_CLASS_CONSTRUCTOR(_name, base)								\
	void step(void);													\
	void reset(void)			{ this->step(); }						\
	int max_output(void) { return _maxout; }							\
	DISCRETE_CLASS_DESTRUCTOR(_name)									\
private:																\
	_priv																\
}

#define  DISCRETE_CLASS_RESET(_name, _maxout) 				\
class DISCRETE_CLASS_NAME(_name): public discrete_base_node				\
{																		\
public:																	\
	DISCRETE_CLASS_CONSTRUCTOR(_name, base)								\
	void reset(void);													\
	int max_output(void) { return _maxout; }							\
	DISCRETE_CLASS_DESTRUCTOR(_name)									\
}

#define  DISCRETE_CLASSA(_name, _maxout, _priv) 						\
class DISCRETE_CLASS_NAME(_name): public discrete_base_node, public discrete_step_interface				\
{																		\
public:																	\
	DISCRETE_CLASS_CONSTRUCTOR(_name, base)								\
	void step(void);													\
	void reset(void);													\
	void start(void);													\
	void stop(void);													\
	int max_output(void) { return _maxout; }							\
	DISCRETE_CLASS_DESTRUCTOR(_name)									\
private:																\
	_priv																\
}

class DISCRETE_CLASS_NAME(special): public discrete_base_node
{
public:
	DISCRETE_CLASS_CONSTRUCTOR(special, base)
	int max_output(void) { return 0; }
	DISCRETE_CLASS_DESTRUCTOR(special)
};

class DISCRETE_CLASS_NAME(unimplemented): public discrete_base_node
{
public:
	DISCRETE_CLASS_CONSTRUCTOR(unimplemented, base)
	int max_output(void) { return 0; }
	DISCRETE_CLASS_DESTRUCTOR(unimplemented)
};

struct dst_size_context
{
//	int size;
};


/*************************************
 *
 *  disc_sys.c
 *
 *************************************/

class DISCRETE_CLASS_NAME(dso_output): public discrete_base_node, public discrete_output_interface, public discrete_step_interface
{
public:
	DISCRETE_CLASS_CONSTRUCTOR(dso_output, base)
	void step(void) {
		/* Add gain to the output and put into the buffers */
		/* Clipping will be handled by the main sound system */
		double val = DISCRETE_INPUT(0) * DISCRETE_INPUT(1);
		*m_ptr++ = val;
	}
	int max_output(void) { return 0; }
	void set_output(stream_sample_t *ptr) { m_ptr = ptr; }
	DISCRETE_CLASS_DESTRUCTOR(dso_output)
private:
	stream_sample_t		*m_ptr;
};

DISCRETE_CLASSA(dso_csvlog, 0,
	FILE *m_csv_file;
	INT64 m_sample_num;
	char  m_name[32];
);

DISCRETE_CLASSA(dso_wavlog, 0,
	wav_file *m_wavfile;
	char      m_name[32];
);

/*************************************
 *
 *  disc_inp.c
 *
 *************************************/

class DISCRETE_CLASS_NAME(dss_adjustment): public discrete_base_node, public discrete_step_interface
{
public:
	DISCRETE_CLASS_CONSTRUCTOR(dss_adjustment, base)
	void step(void);
	void reset(void);
private:
	const input_port_config *m_port;
	INT32					m_lastpval;
	INT32					m_pmin;
	double					m_pscale;
	double					m_min;
	double					m_scale;
	DISCRETE_CLASS_DESTRUCTOR(dss_adjustment)
};

DISCRETE_CLASS_RESET(dss_constant, 1);

class DISCRETE_CLASS_NAME(dss_input_data): public discrete_base_node, public discrete_input_interface
{
public:
	DISCRETE_CLASS_CONSTRUCTOR(dss_input_data, base)
	void reset(void);
	void input_write(int sub_node, UINT8 data );
	DISCRETE_CLASS_DESTRUCTOR(dss_input_data)
private:
	double		m_gain;				/* node gain */
	double		m_offset;			/* node offset */
	UINT8		m_data;				/* data written */
};

class DISCRETE_CLASS_NAME(dss_input_logic): public discrete_base_node, public discrete_input_interface
{
public:
	DISCRETE_CLASS_CONSTRUCTOR(dss_input_logic, base)
	void reset(void);
	void input_write(int sub_node, UINT8 data );
	DISCRETE_CLASS_DESTRUCTOR(dss_input_logic)
private:
	double		m_gain;				/* node gain */
	double		m_offset;			/* node offset */
	UINT8		m_data;				/* data written */
};

class DISCRETE_CLASS_NAME(dss_input_not): public discrete_base_node, public discrete_input_interface
{
public:
	DISCRETE_CLASS_CONSTRUCTOR(dss_input_not, base)
	void reset(void);
	void input_write(int sub_node, UINT8 data );
	DISCRETE_CLASS_DESTRUCTOR(dss_input_not)
private:
	double		m_gain;				/* node gain */
	double		m_offset;			/* node offset */
	UINT8		m_data;				/* data written */
};

class DISCRETE_CLASS_NAME(dss_input_pulse): public discrete_base_node, public discrete_input_interface, public discrete_step_interface
{
public:
	DISCRETE_CLASS_CONSTRUCTOR(dss_input_pulse, base)
	void step(void);
	void reset(void);
	void input_write(int sub_node, UINT8 data );
	DISCRETE_CLASS_DESTRUCTOR(dss_input_pulse)
private:
	double		m_gain;				/* node gain */
	double		m_offset;			/* node offset */
	UINT8		m_data;				/* data written */
};

class DISCRETE_CLASS_NAME(dss_input_stream): public discrete_base_node, public discrete_input_interface, public discrete_step_interface
{
public:
	DISCRETE_CLASS_CONSTRUCTOR(dss_input_stream, base)
	void step(void);
	void reset(void);
	void start(void);
	void input_write(int sub_node, UINT8 data );
	virtual bool is_buffered(void) { return false; }
//protected:
	UINT32				m_stream_in_number;
	stream_sample_t		*m_ptr;			/* current in ptr for stream */
	DISCRETE_CLASS_DESTRUCTOR(dss_input_stream)
private:
	static STREAM_UPDATE( static_stream_generate );
	void stream_generate(stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	double		m_gain;				/* node gain */
	double		m_offset;			/* node offset */
	UINT8		m_data;				/* data written */
	UINT8				m_is_buffered;
	/* the buffer stream */
	sound_stream 		*m_buffer_stream;
};

class DISCRETE_CLASS_NAME(dss_input_buffer): public DISCRETE_CLASS_NAME(dss_input_stream)
{
public:
	DISCRETE_CLASS_CONSTRUCTOR(dss_input_buffer, dss_input_stream)
	bool is_buffered(void) { return true; }
	DISCRETE_CLASS_DESTRUCTOR(dss_input_buffer)
};

#include "disc_wav.h"
#include "disc_mth.h"
#include "disc_flt.h"
#include "disc_dev.h"

#endif /* __DISCRETE_H__ */
