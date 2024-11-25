// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
// thanks-to:Eric Smith, Brad Oliver, Bernd Wiebelt, Aaron Giles, Andrew Caldwell
#ifndef MAME_VIDEO_AVGDVG_H
#define MAME_VIDEO_AVGDVG_H

#pragma once

#include "video/vector.h"


class avgdvg_device_base : public device_t
{
public:
	template <typename T> void set_vector(T &&tag)
	{
		m_vector.set_tag(std::forward<T>(tag));
	}
	template <typename T> void set_memory(T &&tag, int no, offs_t base)
	{
		m_memspace.set_tag(std::forward<T>(tag), no);
		m_membase = base;
	}

	int done_r();
	void go_w(u8 data = 0);
	void reset_w(u8 data = 0);

	void go_word_w(u16 data = 0);
	void reset_word_w(u16 data = 0);

	// Tempest and Quantum use this capability
	void set_flip_x(bool flip) { m_flip_x = flip; }
	void set_flip_y(bool flip) { m_flip_y = flip; }

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

	avgdvg_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

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

	u8 OP0() const { return BIT(m_op, 0); }
	u8 OP1() const { return BIT(m_op, 1); }
	u8 OP2() const { return BIT(m_op, 2); }
	u8 OP3() const { return BIT(m_op, 3); }

	u8 ST3() const { return BIT(m_state_latch, 3); }

	void apply_flipping(int &x, int &y) const;
	void vg_set_halt(int dummy);

	void vg_flush();
	void vg_add_point_buf(int x, int y, rgb_t color, int intensity);
	void vg_add_clip(int xmin, int ymin, int xmax, int ymax);

	required_device<vector_device> m_vector;
	required_address_space m_memspace;
	offs_t m_membase;

	int m_xmin, m_ymin;
	int m_xcenter, m_ycenter;

	int m_nvect;
	vgvector m_vectbuf[MAXVECT];

	u16 m_pc;
	u8 m_sp;
	u16 m_dvx;
	u16 m_dvy;
	u16 m_stack[4];
	u16 m_data;

	u8 m_state_latch;
	u8 m_scale;
	u8 m_intensity;

	u8 m_op;
	u8 m_halt;
	u8 m_sync_halt;

	s32 m_xpos;
	s32 m_ypos;

private:
	TIMER_CALLBACK_MEMBER(vg_set_halt_callback);
	TIMER_CALLBACK_MEMBER(run_state_machine);

	required_region_ptr<u8> m_prom;
	emu_timer *m_vg_run_timer, *m_vg_halt_timer;

	bool m_flip_x, m_flip_y;
};


class dvg_device : public avgdvg_device_base
{
public:
	dvg_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;

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

private:
	void dvg_draw_to(int x, int y, int intensity);
};


class avg_device : public avgdvg_device_base
{
public:
	avg_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	avg_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

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

	int avg_common_strobe1();
	int avg_common_strobe2();
	int avg_common_strobe3();

	int m_xmax = 0, m_ymax = 0;

	u8 m_dvy12 = 0;
	u16 m_timer = 0;

	u8 m_int_latch = 0;
	u8 m_bin_scale = 0;
	u8 m_color = 0;

	u16 m_xdac_xor = 0;
	u16 m_ydac_xor = 0;
};


class avg_tempest_device : public avg_device
{
public:
	avg_tempest_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual int handler_6() override;
	virtual int handler_7() override;
	//virtual void vggo();

private:
	required_shared_ptr<u8> m_colorram;
};


class avg_mhavoc_device : public avg_device
{
public:
	avg_mhavoc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual int handler_1() override;
	virtual int handler_6() override;
	virtual int handler_7() override;
	virtual void update_databus() override;
	virtual void vgrst() override;

private:
	required_shared_ptr<u8> m_colorram;
	required_region_ptr<u8> m_bank_region;

	u8 m_enspkl = 0;
	u8 m_spkl_shift = 0;
	u8 m_map = 0;

	u16 m_lst = 0;
};


class avg_starwars_device : public avg_device
{
public:
	avg_starwars_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual int handler_6() override;
	virtual int handler_7() override;
	virtual void update_databus() override;
};


class avg_quantum_device : public avg_device
{
public:
	avg_quantum_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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

private:
	required_shared_ptr<u16> m_colorram;
};


class avg_bzone_device : public avg_device
{
public:
	avg_bzone_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual int handler_1() override;
	virtual int handler_6() override;
	virtual int handler_7() override;

private:
	u16 m_hst = 0;
	u16 m_lst = 0;
	u16 m_izblank = 0;

	s32 m_clipx_min = 0;
	s32 m_clipy_min = 0;
	s32 m_clipx_max = 0;
	s32 m_clipy_max = 0;
};


// device type declarations
DECLARE_DEVICE_TYPE(DVG,          dvg_device)
DECLARE_DEVICE_TYPE(AVG,          avg_device)
DECLARE_DEVICE_TYPE(AVG_TEMPEST,  avg_tempest_device)
DECLARE_DEVICE_TYPE(AVG_MHAVOC,   avg_mhavoc_device)
DECLARE_DEVICE_TYPE(AVG_STARWARS, avg_starwars_device)
DECLARE_DEVICE_TYPE(AVG_QUANTUM,  avg_quantum_device)
DECLARE_DEVICE_TYPE(AVG_BZONE,    avg_bzone_device)

#endif // MAME_VIDEO_AVGDVG_H
