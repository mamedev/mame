// license:BSD-3-Clause
// copyright-holders:Eric Smith, Brad Oliver, Bernd Wiebelt, Aaron Giles, Andrew Caldwell
#ifndef __AVGDVG__
#define __AVGDVG__

#define MAXVECT      (10000)

#include "video/vector.h"

#define MCFG_AVGDVG_VECTOR(_tag) \
	avgdvg_device::static_set_vector_tag(*device, "^" _tag);

struct vgvector
{
	int x; int y;
	rgb_t color;
	int intensity;
	int arg1; int arg2;
	int status;
};

// ======================> avgdvg_device

class avgdvg_device : public device_t,
                      public device_execute_interface
{
public:
	// construction/destruction
	avgdvg_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	static void static_set_vector_tag(device_t &device, const char *tag);

	DECLARE_CUSTOM_INPUT_MEMBER(done_r);
	DECLARE_WRITE8_MEMBER(go_w);
	DECLARE_WRITE8_MEMBER(reset_w);

	DECLARE_WRITE16_MEMBER(go_word_w);
	DECLARE_WRITE16_MEMBER(reset_word_w);

	/* Tempest and Quantum use this capability */
	void set_flip_x(int flip);
	void set_flip_y(int flip);

protected:
	void apply_flipping(int *x, int *y);
	void vg_set_halt(int dummy);

	void vg_flush();
	void vg_add_point_buf(int x, int y, rgb_t color, int intensity);
	void vg_add_clip (int xmin, int ymin, int xmax, int ymax);

	void register_state();

    virtual void execute_run();
    int m_icount;
    
	UINT8 *avgdvg_vectorram;
	size_t avgdvg_vectorram_size;

    UINT8 *avgdvg_colorram;

	int m_xmin, m_xmax, m_ymin, m_ymax;
	int m_xcenter, m_ycenter;
	int m_flipx, m_flipy;

	int m_nvect;
	vgvector vectbuf[MAXVECT];

	UINT16 m_pc;
	UINT8 m_sp;
	UINT16 m_dvx;
	UINT16 m_dvy;
	UINT8 m_dvy12;
	UINT16 m_timer;
	UINT16 m_stack[4];
	UINT16 m_data;

	UINT8 m_state_latch;
	UINT8 m_int_latch;
	UINT8 m_scale;
	UINT8 m_bin_scale;
	UINT8 m_intensity;
	UINT8 m_color;
	UINT8 m_enspkl;
	UINT8 m_spkl_shift;
	UINT8 m_map;

	UINT16 m_hst;
	UINT16 m_lst;
	UINT16 m_izblank;

	UINT8 m_op;
	UINT8 m_halt;
	UINT8 m_sync_halt;

	UINT16 m_xdac_xor;
	UINT16 m_ydac_xor;

	INT32 m_xpos;
	INT32 m_ypos;

	INT32 m_clipx_min;
	INT32 m_clipy_min;
	INT32 m_clipx_max;
	INT32 m_clipy_max;


	virtual int handler_0() = 0;
	virtual int handler_1() = 0;
	virtual int handler_2() = 0;
	virtual int handler_3() = 0;
	virtual int handler_4() = 0;
	virtual int handler_5() = 0;
	virtual int handler_6() = 0;
	virtual int handler_7() = 0;
	virtual UINT8 state_addr() = 0;
	virtual void update_databus() = 0;
	virtual void vggo() = 0;
	virtual void vgrst() = 0;

	required_device<vector_device> m_vector;
};

class dvg_device : public avgdvg_device
{
public:
	// construction/destruction
	dvg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void dvg_draw_to(int x, int y, int intensity);

	virtual int handler_0();
	virtual int handler_1();
	virtual int handler_2();
	virtual int handler_3();
	virtual int handler_4();
	virtual int handler_5();
	virtual int handler_6();
	virtual int handler_7();
	virtual UINT8 state_addr();
	virtual void update_databus();
	virtual void vggo();
	virtual void vgrst();

	virtual void device_start();
};

// device type definition
extern const device_type DVG;

class avg_device : public avgdvg_device
{
public:
	// construction/destruction
	avg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	avg_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	int avg_common_strobe1();
	int avg_common_strobe2();
	int avg_common_strobe3();

	virtual int handler_0();
	virtual int handler_1();
	virtual int handler_2();
	virtual int handler_3();
	virtual int handler_4();
	virtual int handler_5();
	virtual int handler_6();
	virtual int handler_7();
	virtual UINT8 state_addr();
	virtual void update_databus();
	virtual void vggo();
	virtual void vgrst();

	virtual void device_start();
	void avg_start_common();
};

// device type definition
extern const device_type AVG;

class avg_tempest_device : public avg_device
{
public:
	// construction/destruction
	avg_tempest_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual int handler_6();
	virtual int handler_7();
	//virtual void vggo();
};

// device type definition
extern const device_type AVG_TEMPEST;

class avg_mhavoc_device : public avg_device
{
public:
	// construction/destruction
	avg_mhavoc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual int handler_1();
	virtual int handler_6();
	virtual int handler_7();
	virtual void update_databus();
	virtual void vgrst();
};

// device type definition
extern const device_type AVG_MHAVOC;

class avg_starwars_device : public avg_device
{
public:
	// construction/destruction
	avg_starwars_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual int handler_6();
	virtual int handler_7();
	virtual void update_databus();
};

// device type definition
extern const device_type AVG_STARWARS;

class avg_quantum_device : public avg_device
{
public:
	// construction/destruction
	avg_quantum_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual int handler_0();
	virtual int handler_1();
	virtual int handler_2();
	virtual int handler_3();
	virtual int handler_4();
	virtual int handler_5();
	virtual int handler_6();
	virtual int handler_7();
	virtual void update_databus();
	virtual void vggo();
};

// device type definition
extern const device_type AVG_QUANTUM;

class avg_bzone_device : public avg_device
{
public:
	// construction/destruction
	avg_bzone_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual int handler_1();
	virtual int handler_6();
	virtual int handler_7();
};

// device type definition
extern const device_type AVG_BZONE;

class avg_tomcat_device : public avg_device
{
public:
	// construction/destruction
	avg_tomcat_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual int handler_6();
	virtual int handler_7();
};

// device type definition
extern const device_type AVG_TOMCAT;


#endif
