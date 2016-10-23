// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood,Stephane Humbert
/* Kaneko Hit protection */

#include "machine/watchdog.h"


struct calc1_hit_t
{
	uint16_t x1p, y1p, x1s, y1s;
	uint16_t x2p, y2p, x2s, y2s;

	int16_t x12, y12, x21, y21;

	uint16_t mult_a, mult_b;
};

struct calc3_hit_t
{
	int x1p, y1p, z1p, x1s, y1s, z1s;
	int x2p, y2p, z2p, x2s, y2s, z2s;

	int x1po, y1po, z1po, x1so, y1so, z1so;
	int x2po, y2po, z2po, x2so, y2so, z2so;

	int x12, y12, z12, x21, y21, z21;

	int x_coll, y_coll, z_coll;

	int x1tox2, y1toy2, z1toz2;

	uint16_t mult_a, mult_b;

	uint16_t flags;
	uint16_t mode;
};



class kaneko_hit_device : public device_t
{
public:
	kaneko_hit_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void set_type(device_t &device, int hittype);

	int m_hittype;

	uint16_t kaneko_hit_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void kaneko_hit_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<watchdog_timer_device> m_watchdog;

	calc1_hit_t m_hit;
	calc3_hit_t m_hit3;

	uint16_t kaneko_hit_type0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void kaneko_hit_type0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t kaneko_hit_type1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void kaneko_hit_type1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	int16_t calc_compute_x(calc1_hit_t &hit);
	int16_t calc_compute_y(calc1_hit_t &hit);

	void kaneko_hit_type2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t kaneko_hit_type2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	int type2_calc_compute(int x1, int w1, int x2, int w2);
	void type2_calc_org(int mode, int x0, int s0,  int* x1, int* s1);
	void type2_recalc_collisions(calc3_hit_t &hit3);
};


extern const device_type KANEKO_HIT;
