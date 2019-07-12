// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
// thanks-to:Eric Smith, Brad Oliver, Bernd Wiebelt, Aaron Giles, Andrew Caldwell
#ifndef MAME_VIDEO_AVGDVG_H
#define MAME_VIDEO_AVGDVG_H

#pragma once

#include "video/vector.h"


// ======================> avgdvg_device

class avgdvg_device : public device_t
{
public:
	template <typename T> void set_vector_tag(T &&tag) { m_vector.set_tag(std::forward<T>(tag)); }

	DECLARE_CUSTOM_INPUT_MEMBER(done_r);
	void go_w(u8 data = 0);
	void reset_w(u8 data = 0);

	void go_word_w(u16 data = 0);
	void reset_word_w(u16 data = 0);

	/* Tempest and Quantum use this capability */
	void set_flip_x(bool flip) { m_flip_x = flip; }
	void set_flip_y(bool flip) { m_flip_y = flip; }

	TIMER_CALLBACK_MEMBER(vg_set_halt_callback);
	TIMER_CALLBACK_MEMBER(run_state_machine);
protected:
	static constexpr unsigned MAXVECT = 10000;

	struct vgvector
	{
		int x; int y;
		rgb_t color;
		int intensity;
		int arg1; int arg2;
		int status;
	};

	// construction/destruction
	avgdvg_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void apply_flipping(int *x, int *y);
	void vg_set_halt(int dummy);

	void vg_flush();
	void vg_add_point_buf(int x, int y, rgb_t color, int intensity);
	void vg_add_clip(int xmin, int ymin, int xmax, int ymax);

	void register_state();

	required_shared_ptr<u8> m_vectorram;
	optional_shared_ptr<u8> m_colorram;
	required_region_ptr<u8> m_prom;

	int m_xmin, m_xmax, m_ymin, m_ymax;
	int m_xcenter, m_ycenter;
	emu_timer *m_vg_run_timer, *m_vg_halt_timer;

	bool m_flip_x, m_flip_y;

	int m_nvect;
	vgvector m_vectbuf[MAXVECT];

	u16 m_pc;
	u8 m_sp;
	u16 m_dvx;
	u16 m_dvy;
	u8 m_dvy12;
	u16 m_timer;
	u16 m_stack[4];
	u16 m_data;

	u8 m_state_latch;
	u8 m_int_latch;
	u8 m_scale;
	u8 m_bin_scale;
	u8 m_intensity;
	u8 m_color;
	u8 m_enspkl;
	u8 m_spkl_shift;
	u8 m_map;

	u16 m_hst;
	u16 m_lst;
	u16 m_izblank;

	u8 m_op;
	u8 m_halt;
	u8 m_sync_halt;

	u16 m_xdac_xor;
	u16 m_ydac_xor;

	s32 m_xpos;
	s32 m_ypos;

	s32 m_clipx_min;
	s32 m_clipy_min;
	s32 m_clipx_max;
	s32 m_clipy_max;

	virtual int handler_0() = 0;
	virtual int handler_1() = 0;
	virtual int handler_2() = 0;
	virtual int handler_3() = 0;
	virtual int handler_4() = 0;
	virtual int handler_5() = 0;
	virtual int handler_6() = 0;
	virtual int handler_7() = 0;
	virtual u8 state_addr() = 0;
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

protected:
	virtual int handler_0() override;
	virtual int handler_1() override;
	virtual int handler_2() override;
	virtual int handler_3() override;
	virtual int handler_4() override;
	virtual int handler_5() override;
	virtual int handler_6() override;
	virtual int handler_7() override;
	virtual u8 state_addr() override;
	virtual void update_databus() override;
	virtual void vggo() override;
	virtual void vgrst() override;

	virtual void device_start() override;
};

// device type definition
DECLARE_DEVICE_TYPE(DVG, dvg_device)

class avg_device : public avgdvg_device
{
public:
	// construction/destruction
	avg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	int avg_common_strobe1();
	int avg_common_strobe2();
	int avg_common_strobe3();

protected:
	avg_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual int handler_0() override;
	virtual int handler_1() override;
	virtual int handler_2() override;
	virtual int handler_3() override;
	virtual int handler_4() override;
	virtual int handler_5() override;
	virtual int handler_6() override;
	virtual int handler_7() override;
	virtual u8 state_addr() override;
	virtual void update_databus() override;
	virtual void vggo() override;
	virtual void vgrst() override;

	virtual void device_start() override;
};

// device type definition
DECLARE_DEVICE_TYPE(AVG, avg_device)

class avg_tempest_device : public avg_device
{
public:
	// construction/destruction
	avg_tempest_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual int handler_6() override;
	virtual int handler_7() override;
	//virtual void vggo();
};

// device type definition
DECLARE_DEVICE_TYPE(AVG_TEMPEST, avg_tempest_device)

class avg_mhavoc_device : public avg_device
{
public:
	// construction/destruction
	avg_mhavoc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual int handler_1() override;
	virtual int handler_6() override;
	virtual int handler_7() override;
	virtual void update_databus() override;
	virtual void vgrst() override;

private:
	required_region_ptr<u8> m_bank_region;
};

// device type definition
DECLARE_DEVICE_TYPE(AVG_MHAVOC, avg_mhavoc_device)

class avg_starwars_device : public avg_device
{
public:
	// construction/destruction
	avg_starwars_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual int handler_6() override;
	virtual int handler_7() override;
	virtual void update_databus() override;
};

// device type definition
DECLARE_DEVICE_TYPE(AVG_STARWARS, avg_starwars_device)

class avg_quantum_device : public avg_device
{
public:
	// construction/destruction
	avg_quantum_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
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
DECLARE_DEVICE_TYPE(AVG_QUANTUM, avg_quantum_device)

class avg_bzone_device : public avg_device
{
public:
	// construction/destruction
	avg_bzone_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual int handler_1() override;
	virtual int handler_6() override;
	virtual int handler_7() override;
};

// device type definition
DECLARE_DEVICE_TYPE(AVG_BZONE, avg_bzone_device)

class avg_tomcat_device : public avg_device
{
public:
	// construction/destruction
	avg_tomcat_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual int handler_6() override;
	virtual int handler_7() override;
};

// device type definition
DECLARE_DEVICE_TYPE(AVG_TOMCAT, avg_tomcat_device)


#endif // MAME_VIDEO_AVGDVG_H
