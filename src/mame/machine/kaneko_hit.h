// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/* Kaneko Hit protection */


struct calc1_hit_t
{
	UINT16 x1p, y1p, x1s, y1s;
	UINT16 x2p, y2p, x2s, y2s;

	INT16 x12, y12, x21, y21;

	UINT16 mult_a, mult_b;
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

	UINT16 mult_a, mult_b;

	UINT16 flags;
	UINT16 mode;
};



class kaneko_hit_device : public device_t
{
public:
	kaneko_hit_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	static void set_type(device_t &device, int hittype);

	int m_hittype;

	DECLARE_READ16_MEMBER(kaneko_hit_r);
	DECLARE_WRITE16_MEMBER(kaneko_hit_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	calc1_hit_t m_hit;
	calc3_hit_t m_hit3;

	DECLARE_READ16_MEMBER(kaneko_hit_type0_r);
	DECLARE_WRITE16_MEMBER(kaneko_hit_type0_w);

	DECLARE_READ16_MEMBER(kaneko_hit_type1_r);
	DECLARE_WRITE16_MEMBER(kaneko_hit_type1_w);
	INT16 calc_compute_x(calc1_hit_t &hit);
	INT16 calc_compute_y(calc1_hit_t &hit);

	DECLARE_WRITE16_MEMBER(kaneko_hit_type2_w);
	DECLARE_READ16_MEMBER(kaneko_hit_type2_r);
	int type2_calc_compute(int x1, int w1, int x2, int w2);
	void type2_calc_org(int mode, int x0, int s0,  int* x1, int* s1);
	void type2_recalc_collisions(calc3_hit_t &hit3);
};


extern const device_type KANEKO_HIT;
