// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood,Stephane Humbert
/* Kaneko 'Calc' hitbox collision / protection

   It is thought that this is done by the 'CALC1' 'TOYBOX' and 'CALC3' protection chips found on the various boards
   however we have 3 implementations, and they don't quite pair up with the chips at the moment, this might be
   because our implementations are wrong / incomplete, or in some cases the newer chips are backwards compatible.

   galpanica  - CALC1   - type 0
   sandscrp   - CALC1   - type 0 ( only uses Random Number? )
   bonkadv    - TOYBOX  - type 0 ( only uses Random Number, XY Overlap Collision bit and register '0x02' )
   gtmr       - TOYBOX  - type 1 ( only uses Random Number )
   gtmr2      - TOYBOX  - type 1 ( only uses Random Number )
   bloodwar   - TOYBOX  - type 1
   shogwarr   - CALC3   - type 1
   brapboys   - CALC3   - type 2

   note: shogwarr won't work with our brapboys implementation despite them being the same PCB and same MCU, this
         suggests that at least one of the implementations is wrong

   suprnova.c also has a similar device, the implementation hasn't been fully compared

  CALC1 is a 40 pin DIP MCU of unknown type with unknown internal rom

*/


#include "emu.h"
#include "kaneko_hit.h"

const device_type KANEKO_HIT = &device_creator<kaneko_hit_device>;

kaneko_hit_device::kaneko_hit_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, KANEKO_HIT, "Kaneko CALC Hitbox", tag, owner, clock, "kaneko_hit", __FILE__)
{
	m_hittype = -1;
	memset(&m_hit, 0, sizeof m_hit);
	memset(&m_hit3, 0, sizeof m_hit3);

}

void kaneko_hit_device::set_type(device_t &device, int hittype)
{
	kaneko_hit_device &dev = downcast<kaneko_hit_device &>(device);
	dev.m_hittype = hittype;
}


void kaneko_hit_device::device_start()
{
	/* m_hit */
	save_item(NAME(m_hit.x1p));
	save_item(NAME(m_hit.y1p));
	save_item(NAME(m_hit.x1s));
	save_item(NAME(m_hit.y1s));
	save_item(NAME(m_hit.x2p));
	save_item(NAME(m_hit.y2p));
	save_item(NAME(m_hit.x2s));
	save_item(NAME(m_hit.y2s));
	save_item(NAME(m_hit.x12));
	save_item(NAME(m_hit.y12));
	save_item(NAME(m_hit.x21));
	save_item(NAME(m_hit.y21));
	save_item(NAME(m_hit.mult_a));
	save_item(NAME(m_hit.mult_b));

	/* m_hit3 */
	save_item(NAME(m_hit3.x1p));
	save_item(NAME(m_hit3.y1p));
	save_item(NAME(m_hit3.z1p));
	save_item(NAME(m_hit3.x1s));
	save_item(NAME(m_hit3.y1s));
	save_item(NAME(m_hit3.z1s));
	save_item(NAME(m_hit3.x2p));
	save_item(NAME(m_hit3.y2p));
	save_item(NAME(m_hit3.z2p));
	save_item(NAME(m_hit3.x2s));
	save_item(NAME(m_hit3.y2s));
	save_item(NAME(m_hit3.z2s));
	save_item(NAME(m_hit3.x1po));
	save_item(NAME(m_hit3.y1po));
	save_item(NAME(m_hit3.z1po));
	save_item(NAME(m_hit3.x1so));
	save_item(NAME(m_hit3.y1so));
	save_item(NAME(m_hit3.z1so));
	save_item(NAME(m_hit3.x2po));
	save_item(NAME(m_hit3.y2po));
	save_item(NAME(m_hit3.z2po));
	save_item(NAME(m_hit3.x2so));
	save_item(NAME(m_hit3.y2so));
	save_item(NAME(m_hit3.z2so));
	save_item(NAME(m_hit3.x12));
	save_item(NAME(m_hit3.y12));
	save_item(NAME(m_hit3.z12));
	save_item(NAME(m_hit3.x21));
	save_item(NAME(m_hit3.y21));
	save_item(NAME(m_hit3.z21));
	save_item(NAME(m_hit3.x_coll));
	save_item(NAME(m_hit3.y_coll));
	save_item(NAME(m_hit3.z_coll));
	save_item(NAME(m_hit3.x1tox2));
	save_item(NAME(m_hit3.y1toy2));
	save_item(NAME(m_hit3.z1toz2));
	save_item(NAME(m_hit3.mult_a));
	save_item(NAME(m_hit3.mult_b));
	save_item(NAME(m_hit3.flags));
	save_item(NAME(m_hit3.mode));
}

void kaneko_hit_device::device_reset()
{
}


READ16_MEMBER(kaneko_hit_device::kaneko_hit_r)
{
	switch (m_hittype)
	{
		case 0: return kaneko_hit_type0_r(space,offset,mem_mask);
		case 1: return kaneko_hit_type1_r(space,offset,mem_mask);
		case 2: return kaneko_hit_type2_r(space,offset,mem_mask);

		default:
			fatalerror("kaneko_hit_r called, but m_hittype not set\n");
	}
}

WRITE16_MEMBER(kaneko_hit_device::kaneko_hit_w)
{
	switch (m_hittype)
	{
		case 0: kaneko_hit_type0_w(space,offset,data, mem_mask); break;
		case 1: kaneko_hit_type1_w(space,offset,data, mem_mask); break;
		case 2: kaneko_hit_type2_w(space,offset,data, mem_mask); break;

		default:
			fatalerror("kaneko_hit_r called, but m_hittype not set\n");
	}
}

/*********************************************************************
 TYPE 0
 galpanica
 sandscrp
 bonkadv
*********************************************************************/

READ16_MEMBER(kaneko_hit_device::kaneko_hit_type0_r)
{
	calc1_hit_t &hit = m_hit;
	UINT16 data = 0;

	switch (offset)
	{
		case 0x00/2: // watchdog
			machine().watchdog_reset();
			return 0;

		case 0x02/2: // unknown (yet!), used by *MANY* games !!!
			//popmessage("unknown collision reg");
			break;

		case 0x04/2: // similar to the hit detection from SuperNova, but much simpler

			// X Absolute Collision
			if      (hit.x1p >  hit.x2p)    data |= 0x0200;
			else if (hit.x1p == hit.x2p)    data |= 0x0400;
			else if (hit.x1p <  hit.x2p)    data |= 0x0800;

			// Y Absolute Collision
			if      (hit.y1p >  hit.y2p)    data |= 0x2000;
			else if (hit.y1p == hit.y2p)    data |= 0x4000;
			else if (hit.y1p <  hit.y2p)    data |= 0x8000;

			// XY Overlap Collision
			hit.x12 = (hit.x1p) - (hit.x2p + hit.x2s);
			hit.y12 = (hit.y1p) - (hit.y2p + hit.y2s);
			hit.x21 = (hit.x1p + hit.x1s) - (hit.x2p);
			hit.y21 = (hit.y1p + hit.y1s) - (hit.y2p);

			if ((hit.x12 < 0) && (hit.y12 < 0) &&
				(hit.x21 >= 0) && (hit.y21 >= 0))
					data |= 0x0001;

			return data;

		case 0x10/2:
			return (((UINT32)hit.mult_a * (UINT32)hit.mult_b) >> 16);
		case 0x12/2:
			return (((UINT32)hit.mult_a * (UINT32)hit.mult_b) & 0xffff);

		case 0x14/2:
			return (machine().rand() & 0xffff);

		default:
			logerror("CPU #0 PC %06x: warning - read unmapped calc address %06x\n",space.device().safe_pc(),offset<<1);
	}

	return 0;
}

WRITE16_MEMBER(kaneko_hit_device::kaneko_hit_type0_w)
{
	calc1_hit_t &hit = m_hit;
	data &= mem_mask;

	switch (offset)
	{
		// p is position, s is size
		case 0x00/2: hit.x1p    = data; break;
		case 0x02/2: hit.x1s    = data; break;
		case 0x04/2: hit.y1p    = data; break;
		case 0x06/2: hit.y1s    = data; break;
		case 0x08/2: hit.x2p    = data; break;
		case 0x0a/2: hit.x2s    = data; break;
		case 0x0c/2: hit.y2p    = data; break;
		case 0x0e/2: hit.y2s    = data; break;
		case 0x10/2: hit.mult_a = data; break;
		case 0x12/2: hit.mult_b = data; break;

		default:
			logerror("CPU #0 PC %06x: warning - write unmapped hit address %06x\n",space.device().safe_pc(),offset<<1);
	}
}


/*********************************************************************
 TYPE 1
 shogwarr
 bloorwar
*********************************************************************/

READ16_MEMBER(kaneko_hit_device::kaneko_hit_type1_r)
{
	calc1_hit_t &hit = m_hit;
	UINT16 data = 0;
	INT16 x_coll, y_coll;


	x_coll = calc_compute_x(hit);
	y_coll = calc_compute_y(hit);

	switch (offset)
	{
		case 0x00/2: // X distance
			return x_coll;

		case 0x02/2: // Y distance
			return y_coll;

		case 0x04/2: // similar to the hit detection from SuperNova, but much simpler

			// 4th nibble: Y Absolute Collision -> possible values = 9,8,4,3,2
			if      (hit.y1p >  hit.y2p)    data |= 0x2000;
			else if (hit.y1p == hit.y2p)    data |= 0x4000;
			else if (hit.y1p <  hit.y2p)    data |= 0x8000;
			if (y_coll<0) data |= 0x1000;

			// 3rd nibble: X Absolute Collision -> possible values = 9,8,4,3,2
			if      (hit.x1p >  hit.x2p)    data |= 0x0200;
			else if (hit.x1p == hit.x2p)    data |= 0x0400;
			else if (hit.x1p <  hit.x2p)    data |= 0x0800;
			if (x_coll<0) data |= 0x0100;

			// 2nd nibble: always set to 4
			data |= 0x0040;

			// 1st nibble: XY Overlap Collision -> possible values = 0,2,4,f
			if (x_coll>=0) data |= 0x0004;
			if (y_coll>=0) data |= 0x0002;
			if ((x_coll>=0)&&(y_coll>=0)) data |= 0x000F;

			return data;

		case 0x14/2:
			return (machine().rand() & 0xffff);

		case 0x20/2: return hit.x1p;
		case 0x22/2: return hit.x1s;
		case 0x24/2: return hit.y1p;
		case 0x26/2: return hit.y1s;

		case 0x2c/2: return hit.x2p;
		case 0x2e/2: return hit.x2s;
		case 0x30/2: return hit.y2p;
		case 0x32/2: return hit.y2s;

		default:
			logerror("CPU #0 PC %06x: warning - read unmapped calc address %06x\n",space.device().safe_pc(),offset<<1);
	}

	return 0;
}

WRITE16_MEMBER(kaneko_hit_device::kaneko_hit_type1_w)
{
	calc1_hit_t &hit = m_hit;
	data &= mem_mask;

	switch (offset)
	{
		// p is position, s is size
		case 0x20/2: hit.x1p = data; break;
		case 0x22/2: hit.x1s = data; break;
		case 0x24/2: hit.y1p = data; break;
		case 0x26/2: hit.y1s = data; break;

		case 0x2c/2: hit.x2p = data; break;
		case 0x2e/2: hit.x2s = data; break;
		case 0x30/2: hit.y2p = data; break;
		case 0x32/2: hit.y2s = data; break;

		// this register is set to zero before any computation,
		// but it has no effect on inputs or result registers
		case 0x38/2: break;

		default:
			logerror("CPU #0 PC %06x: warning - write unmapped hit address %06x\n",space.device().safe_pc(),offset<<1);
	}
}

/*
 collision detection: absolute "distance", negative if no overlap
         [one inside other] | [ normal overlap ] | [   no overlap   ]
  rect1   <-------------->  |  <----------->     |  <--->
  rect2       <----->       |     <----------->  |             <--->
  result      <---------->  |     <-------->     |       <---->
*/

INT16 kaneko_hit_device::calc_compute_x(calc1_hit_t &hit)
{
	INT16 x_coll;

	// X distance
	if ((hit.x2p >= hit.x1p) && (hit.x2p < (hit.x1p + hit.x1s)))        // x2p inside x1
		x_coll = (hit.x1s - (hit.x2p - hit.x1p));
	else if ((hit.x1p >= hit.x2p) && (hit.x1p < (hit.x2p + hit.x2s)))   // x1p inside x2
		x_coll = (hit.x2s - (hit.x1p - hit.x2p));
	else                                                                // normal/no overlap
		x_coll = ((hit.x1s + hit.x2s)/2) - abs((hit.x1p + hit.x1s/2) - (hit.x2p + hit.x2s/2));

	return x_coll;
}

INT16 kaneko_hit_device::calc_compute_y(calc1_hit_t &hit)
{
	INT16 y_coll;

	// Y distance
	if ((hit.y2p >= hit.y1p) && (hit.y2p < (hit.y1p + hit.y1s)))        // y2p inside y1
		y_coll = (hit.y1s - (hit.y2p - hit.y1p));
	else if ((hit.y1p >= hit.y2p) && (hit.y1p < (hit.y2p + hit.y2s)))   // y1p inside y2
		y_coll = (hit.y2s - (hit.y1p - hit.y2p));
	else                                                                // normal/no overlap
		y_coll = ((hit.y1s + hit.y2s)/2) - abs((hit.y1p + hit.y1s/2) - (hit.y2p + hit.y2s/2));

	return y_coll;
}


/*********************************************************************
 TYPE 2
 brapboys
*********************************************************************/


WRITE16_MEMBER(kaneko_hit_device::kaneko_hit_type2_w)
{
	calc3_hit_t &hit3 = m_hit3;
	data &= mem_mask;

	int idx=offset*4;
	switch (idx)
	{
		// p is position, s is size
		case 0x00:
		case 0x28:

					hit3.x1po = data; break;

		case 0x04:
		case 0x2c:
					hit3.x1so = data; break;

		case 0x08:
		case 0x30:
					hit3.y1po = data; break;

		case 0x0c:
		case 0x34:
					hit3.y1so = data; break;

		case 0x10:
		case 0x58:
					hit3.x2po = data; break;

		case 0x14:
		case 0x5c:
					hit3.x2so = data; break;

		case 0x18:
		case 0x60:
					hit3.y2po = data; break;

		case 0x1c:
		case 0x64:
					hit3.y2so = data; break;

		case 0x38:
		case 0x50:
					hit3.z1po = data; break;
		case 0x3c:
		case 0x54:
					hit3.z1so = data; break;

		case 0x20:
		case 0x68:
					hit3.z2po = data; break;

		case 0x24:
		case 0x6c:
					hit3.z2so = data; break;

		case 0x70:
					hit3.mode=data;break;

		default:
			logerror("CPU #0 PC %06x: warning - write unmapped hit address %06x [ %06x] = %06x\n",space.device().safe_pc(),offset<<1, idx, data);
	}

	type2_recalc_collisions(hit3);
}


READ16_MEMBER(kaneko_hit_device::kaneko_hit_type2_r)
{
	calc3_hit_t &hit3 = m_hit3;
	int idx=offset*4;

	switch (idx)
	{
		case 0x00: // X distance
		case 0x10:
			return hit3.x_coll;

		case 0x04: // Y distance
		case 0x14:
			return hit3.y_coll;

		case 0x18: // Z distance
			return hit3.z_coll;

		case 0x08:
		case 0x1c:

			return hit3.flags;

		case 0x28:
			return (machine().rand() & 0xffff);

		case 0x40: return hit3.x1po;
		case 0x44: return hit3.x1so;
		case 0x48: return hit3.y1po;
		case 0x4c: return hit3.y1so;
		case 0x50: return hit3.z1po;
		case 0x54: return hit3.z1so;

		case 0x58: return hit3.x2po;
		case 0x5c: return hit3.x2so;
		case 0x60: return hit3.y2po;
		case 0x64: return hit3.y2so;
		case 0x68: return hit3.z2po;
		case 0x6c: return hit3.z2so;

		case 0x80: return hit3.x1tox2;
		case 0x84: return hit3.y1toy2;
		case 0x88: return hit3.z1toz2;

		default:
			logerror("CPU #0 PC %06x: warning - read unmapped calc address %06x [ %06x]\n",space.device().safe_pc(),offset<<1, idx);
	}

	return 0;
}

//calculate simple intersection of two segments

int kaneko_hit_device::type2_calc_compute(int x1, int w1, int x2, int w2)
{
	int dist;

	if(x2>=x1 && x2+w2<=(x1+w1))
	{
		//x2 inside x1
		dist=w2;
	}
	else
	{
		if(x1>=x2 && x1+w1<=(x2+w2))
		{
			//x1 inside x2
			dist=w1;
		}
		else
		{
			if(x2<x1)
			{
				//swap
				int tmp=x1;
				x1=x2;
				x2=tmp;
				tmp=w1;
				w1=w2;
				w2=tmp;
			}
			dist=x1+w1-x2;
		}
	}
	return dist;
}

//calc segment coordinates

void kaneko_hit_device::type2_calc_org(int mode, int x0, int s0,  int* x1, int* s1)
{
	switch(mode)
	{
		case 0: *x1=x0; *s1=s0; break;
		case 1: *x1=x0-s0/2; *s1=s0; break;
		case 2: *x1=x0-s0; *s1=s0; break;
		case 3: *x1=x0-s0; *s1=2*s0; break;
	}
	//x1 is the left most coord, s1 = width
}

void kaneko_hit_device::type2_recalc_collisions(calc3_hit_t &hit3)
{
	//calculate positions and sizes

	int mode=hit3.mode;

	hit3.flags=0;

	type2_calc_org((mode>>0)&3, hit3.x1po, hit3.x1so, &hit3.x1p, &hit3.x1s);
	type2_calc_org((mode>>2)&3, hit3.y1po, hit3.y1so, &hit3.y1p, &hit3.y1s);
	type2_calc_org((mode>>4)&3, hit3.z1po, hit3.z1so, &hit3.z1p, &hit3.z1s);

	type2_calc_org((mode>>8)&3, hit3.x2po, hit3.x2so, &hit3.x2p, &hit3.x2s);
	type2_calc_org((mode>>10)&3, hit3.y2po, hit3.y2so, &hit3.y2p, &hit3.y2s);
	type2_calc_org((mode>>12)&3, hit3.z2po, hit3.z2so, &hit3.z2p, &hit3.z2s);


	hit3.x1tox2=abs(hit3.x2po-hit3.x1po);
	hit3.y1toy2=abs(hit3.y2po-hit3.y1po);
	hit3.z1toz2=abs(hit3.z2po-hit3.z1po);


	hit3.x_coll = type2_calc_compute(hit3.x1p, hit3.x1s, hit3.x2p, hit3.x2s);
	hit3.y_coll = type2_calc_compute(hit3.y1p, hit3.y1s, hit3.y2p, hit3.y2s);
	hit3.z_coll = type2_calc_compute(hit3.z1p, hit3.z1s, hit3.z2p, hit3.z2s);


	// 4th nibble: Y Absolute Collision -> possible values = 9,8,4,3,2
	if      (hit3.y1p >  hit3.y2p)  hit3.flags |= 0x2000;
	else if (hit3.y1p == hit3.y2p)  hit3.flags |= 0x4000;
	else if (hit3.y1p <  hit3.y2p)  hit3.flags |= 0x8000;
	if (hit3.y_coll<0) hit3.flags |= 0x1000;

	// 3rd nibble: X Absolute Collision -> possible values = 9,8,4,3,2
	if      (hit3.x1p >  hit3.x2p)  hit3.flags |= 0x0200;
	else if (hit3.x1p == hit3.x2p)  hit3.flags |= 0x0400;
	else if (hit3.x1p <  hit3.x2p)  hit3.flags |= 0x0800;
	if (hit3.x_coll<0) hit3.flags |= 0x0100;

	// 2nd nibble: Z Absolute Collision -> possible values = 9,8,4,3,2
	if      (hit3.z1p >  hit3.z2p)  hit3.flags |= 0x0020;
	else if (hit3.z1p == hit3.z2p)  hit3.flags |= 0x0040;
	else if (hit3.z1p <  hit3.z2p)  hit3.flags |= 0x0080;
	if (hit3.z_coll<0) hit3.flags |= 0x0010;

	// 1st nibble: XYZ Overlap Collision
	if ((hit3.x_coll>=0)&&(hit3.y_coll>=0)&&(hit3.z_coll>=0)) hit3.flags |= 0x0008;
	if ((hit3.x_coll>=0)&&(hit3.z_coll>=0)) hit3.flags |= 0x0004;
	if ((hit3.y_coll>=0)&&(hit3.z_coll>=0)) hit3.flags |= 0x0002;
	if ((hit3.x_coll>=0)&&(hit3.y_coll>=0)) hit3.flags |= 0x0001;
}
