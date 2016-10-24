// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
// thanks-to:Eric Smith, Brad Oliver, Bernd Wiebelt, Aaron Giles, Andrew Caldwell
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

class avgdvg_device : public device_t
{
public:
	// construction/destruction
	avgdvg_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	static void static_set_vector_tag(device_t &device, const char *tag);

	ioport_value done_r(ioport_field &field, void *param);
	void go_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void go_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void reset_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	/* Tempest and Quantum use this capability */
	void set_flip_x(int flip);
	void set_flip_y(int flip);

	void vg_set_halt_callback(void *ptr, int32_t param);
	void run_state_machine(void *ptr, int32_t param);
protected:
	void apply_flipping(int *x, int *y);
	void vg_set_halt(int dummy);

	void vg_flush();
	void vg_add_point_buf(int x, int y, rgb_t color, int intensity);
	void vg_add_clip (int xmin, int ymin, int xmax, int ymax);

	void register_state();

	uint8_t *avgdvg_vectorram;
	size_t avgdvg_vectorram_size;

	uint8_t *avgdvg_colorram;


	int xmin, xmax, ymin, ymax;
	int xcenter, ycenter;
	emu_timer *vg_run_timer, *vg_halt_timer;

	int flip_x, flip_y;

	int nvect;
	vgvector vectbuf[MAXVECT];


	uint16_t m_pc;
	uint8_t m_sp;
	uint16_t m_dvx;
	uint16_t m_dvy;
	uint8_t m_dvy12;
	uint16_t m_timer;
	uint16_t m_stack[4];
	uint16_t m_data;

	uint8_t m_state_latch;
	uint8_t m_int_latch;
	uint8_t m_scale;
	uint8_t m_bin_scale;
	uint8_t m_intensity;
	uint8_t m_color;
	uint8_t m_enspkl;
	uint8_t m_spkl_shift;
	uint8_t m_map;

	uint16_t m_hst;
	uint16_t m_lst;
	uint16_t m_izblank;

	uint8_t m_op;
	uint8_t m_halt;
	uint8_t m_sync_halt;

	uint16_t m_xdac_xor;
	uint16_t m_ydac_xor;

	int32_t m_xpos;
	int32_t m_ypos;

	int32_t m_clipx_min;
	int32_t m_clipy_min;
	int32_t m_clipx_max;
	int32_t m_clipy_max;


	virtual int handler_0() = 0;
	virtual int handler_1() = 0;
	virtual int handler_2() = 0;
	virtual int handler_3() = 0;
	virtual int handler_4() = 0;
	virtual int handler_5() = 0;
	virtual int handler_6() = 0;
	virtual int handler_7() = 0;
	virtual uint8_t state_addr() = 0;
	virtual void update_databus() = 0;
	virtual void vggo() = 0;
	virtual void vgrst() = 0;

	required_device<vector_device> m_vector;
};

class dvg_device : public avgdvg_device
{
public:
	// construction/destruction
	dvg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void dvg_draw_to(int x, int y, int intensity);

	virtual int handler_0() override;
	virtual int handler_1() override;
	virtual int handler_2() override;
	virtual int handler_3() override;
	virtual int handler_4() override;
	virtual int handler_5() override;
	virtual int handler_6() override;
	virtual int handler_7() override;
	virtual uint8_t state_addr() override;
	virtual void update_databus() override;
	virtual void vggo() override;
	virtual void vgrst() override;

	virtual void device_start() override;
};

// device type definition
extern const device_type DVG;

class avg_device : public avgdvg_device
{
public:
	// construction/destruction
	avg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	avg_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	int avg_common_strobe1();
	int avg_common_strobe2();
	int avg_common_strobe3();

	virtual int handler_0() override;
	virtual int handler_1() override;
	virtual int handler_2() override;
	virtual int handler_3() override;
	virtual int handler_4() override;
	virtual int handler_5() override;
	virtual int handler_6() override;
	virtual int handler_7() override;
	virtual uint8_t state_addr() override;
	virtual void update_databus() override;
	virtual void vggo() override;
	virtual void vgrst() override;

	virtual void device_start() override;
};

// device type definition
extern const device_type AVG;

class avg_tempest_device : public avg_device
{
public:
	// construction/destruction
	avg_tempest_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual int handler_6() override;
	virtual int handler_7() override;
	//virtual void vggo();
};

// device type definition
extern const device_type AVG_TEMPEST;

class avg_mhavoc_device : public avg_device
{
public:
	// construction/destruction
	avg_mhavoc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual int handler_1() override;
	virtual int handler_6() override;
	virtual int handler_7() override;
	virtual void update_databus() override;
	virtual void vgrst() override;
};

// device type definition
extern const device_type AVG_MHAVOC;

class avg_starwars_device : public avg_device
{
public:
	// construction/destruction
	avg_starwars_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual int handler_6() override;
	virtual int handler_7() override;
	virtual void update_databus() override;
};

// device type definition
extern const device_type AVG_STARWARS;

class avg_quantum_device : public avg_device
{
public:
	// construction/destruction
	avg_quantum_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual int handler_0() override;
	virtual int handler_1() override;
	virtual int handler_2() override;
	virtual int handler_3() override;
	virtual int handler_4() override;
	virtual int handler_5() override;
	virtual int handler_6() override;
	virtual int handler_7() override;
	virtual void update_databus() override;
	virtual void vggo() override;
};

// device type definition
extern const device_type AVG_QUANTUM;

class avg_bzone_device : public avg_device
{
public:
	// construction/destruction
	avg_bzone_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual int handler_1() override;
	virtual int handler_6() override;
	virtual int handler_7() override;
};

// device type definition
extern const device_type AVG_BZONE;

class avg_tomcat_device : public avg_device
{
public:
	// construction/destruction
	avg_tomcat_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual int handler_6() override;
	virtual int handler_7() override;
};

// device type definition
extern const device_type AVG_TOMCAT;


#endif
