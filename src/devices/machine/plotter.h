// license:BSD-3-Clause
// copyright-holders: Fabio Dalla Libera
/*

  Defines plotter abstractions

  * A generic x-y plotter
  * A plotter with the head moving along the x axis and an infinite roll of paper moving along the y axis,

  */

#include "screen.h"
#include "machine/steppers.h"

#ifndef MAME_MACHINE_PLOTTER_H
#define MAME_MACHINE_PLOTTER_H

#pragma once

class generic_plotter_device:public device_t
{

public:

	class Element
	{
	public:
		Element(int w=0, int h=0, uint32_t c=0);
		int width;
		int height;
		uint32_t color;
	};

	class Position
	{
	public:
		Position(int x=0, int y=0);
		int x,y;

		Position operator-(const Position&);
		Position operator+(const Element&);
	};

	generic_plotter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock=0);

	virtual void device_add_mconfig(machine_config &config) override;
	void device_start() override;

	virtual void update_motors(uint8_t xpattern, uint8_t ypattern);

	void set_pencolor(uint32_t);
	void set_penthickness(int);
	void set_headcolor(uint32_t);
	void set_headsize(int);
	void pen_down();
	void pen_up();
	int get_xmotor_pos();
	int get_ymotor_pos();
	virtual Position get_head_pos(){return Position();}
	virtual Position get_paper_pos(){ return Position();}
	virtual void extend_paper(){}

	template <typename F>
	std::enable_if_t<screen_update_rgb32_delegate::supports_callback<F>::value> set_panel_update(F &&callback, const char *name)
		{
			m_panel_update.set(std::forward<F>(callback), name);
		}
	void write_snapshot_to_file();
	virtual void change_paper();

protected:

	generic_plotter_device(const machine_config &mconfig, device_type type,const char *tag, device_t *owner, const Element& view, const Element& paperwin, const Position& paperwinpos, const Position & motorpos, double xscale, double yscale);  

	void device_resolve_objects() override
	{
		m_panel_update.resolve();
	}

	required_device<stepper_device> m_xstepper, m_ystepper;
	bitmap_rgb32 m_paper_bitmap;
	required_device<screen_device> m_screen;
	Element m_view,m_paperwin;
	Position m_paperwinpos, m_paperorigin, m_motorpos;
	double m_xscale;
	double m_yscale;
	bool m_pendown;

private:

	uint32_t screen_update_bitmap(screen_device &screen,bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_head(bitmap_rgb32 &bitmap);

	uint32_t m_headcolor;
	int m_headsize;
	uint32_t m_pencolor;
	int m_penthickness;
	screen_update_rgb32_delegate m_panel_update;
};

class paper_roll_plotter_device:public generic_plotter_device
{
public:

	paper_roll_plotter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock=0);
	virtual void change_paper() override;

protected:

	paper_roll_plotter_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, const Element& view, const Element& paperwin, const Position& paperwinpos, const Position & motorpos, double xscale, double yscale);    
	virtual Position get_head_pos() override;
	virtual Position get_paper_pos() override;
	virtual void extend_paper() override;
};




DECLARE_DEVICE_TYPE(GENERIC_PLOTTER, generic_plotter_device)
DECLARE_DEVICE_TYPE(PAPER_ROLL_PLOTTER, paper_roll_plotter_device)


#endif
