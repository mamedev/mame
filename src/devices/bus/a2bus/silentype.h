// license:BSD-3-Clause
// copyright-holders: Golden Child
/*********************************************************************

    a2silentype.h

    Implementation of the Apple II Silentype Printer

*********************************************************************/

#ifndef MAME_BUS_A2BUS_SILENTYPE_H
#define MAME_BUS_A2BUS_SILENTYPE_H

#pragma once

#include "a2bus.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_silentype_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	a2bus_silentype_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_silentype_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override;

	virtual void device_add_mconfig(machine_config &config) override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_c800(uint16_t offset) override;
	virtual void write_c800(uint16_t offset, uint8_t data) override;


	void update_printhead(uint8_t data);
	void update_pf_stepper(uint8_t data);
	void update_cr_stepper(uint8_t data);

	uint8_t *m_rom;
	uint8_t m_ram[256];

	bitmap_rgb32 m_bitmap;

	int m_xpos=250;
	int m_ypos=0;
	uint16_t m_shift_reg,m_parallel_reg;
	int m_romenable=0;  // start off disabled

	required_device<screen_device> m_screen;

	int right_offset = 0;
	int left_offset = 3;

	double headtemp[7] = {0.0}; // initialize to zero - nan bugs
//  int heattime=3500;   // time in usec to hit max temp  (smaller numbers mean faster)  5 levels

// equal 4 levels
//  int heattime=4500;   // time in usec to hit max temp  (smaller numbers mean faster)
// int heattime=2000;   // time in usec to hit max temp  (smaller numbers mean faster)  5 levels saturated in the middle
//      int heattime=4000;   // time in usec to hit max temp  (smaller numbers mean faster)  5 levels
				int heattime=3000;   // time in usec to hit max temp  (smaller numbers mean faster)  5 levels  25 percent   68/255 was darkest at 4000
	int decaytime=1000;  // time in usec to cool off

//  int hstepper;
//  int vstepper;
	int hstepperlast = 0;
	int vstepperlast = 0;
//  int headbits;
	int lastheadbits = 0;
	int xdirection;
	int newpageflag;

	int page_count=0;

	double last_update_time;

 private:
	uint32_t screen_update_silentype(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual const tiny_rom_entry *device_rom_region() const override;

	const int dpi=60;
	const int PAPER_WIDTH = 8.5 * dpi;  // 8.5 inches wide at 60 dpi
	const int PAPER_HEIGHT = 11 * dpi;   // 11  inches high at 60 dpi
	const int PAPER_SCREEN_HEIGHT = 384; // match the height of the apple II driver

	uint32_t BITS(uint32_t x, u8 m, u8 n) {return (((x)>>(n))&(((uint32_t)1<<((m)-(n)+1))-1));}

	int wrap(int x,int mod){if (x<0) return (x+((-1*(x/mod))+1)*mod)%mod; else return x % mod;}
	// mod=4 for x = -25,5 do if (x<0) then print(x, (x+((-1*(math.floor(x/mod)))+1)*mod)%mod) else print(x, x % mod) end end
	int getbit(int a, int bit) {return (a & bit)!=0;}
// this setbit works on a mask 1<<bit
//  int setbit(int& a, int bit, int value) {a=(a&~bit)|(bit*((value!=0)?1:0)); return a;}

// this setbit works on a bitnumber
//int setbit(int& a, int bit, int value) {a=(a&~(1<<bit))|((1<<bit)*((value!=0)?1:0)); return a;}
int setbit(int& a, int bit, int value) {a=(a&~(1<<bit))|((((value!=0)?(1<<bit):0))); return a;}
	void write_snapshot_to_file();

	void darken_pixel(double headtemp, u32& pixel);
};


// device type definition
DECLARE_DEVICE_TYPE(A2BUS_SILENTYPE, a2bus_silentype_device)
#endif // MAME_BUS_A2BUS_A2SILENTYPE_H
