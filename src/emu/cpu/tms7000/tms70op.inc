/*****************************************************************************
 *
 *   tms70op.inc (Op code functions)
 *   Portable TMS7000 emulator (Texas Instruments 7000)
 *
 *   Copyright tim lindner, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     tlindner@macmess.org
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/

//SJE: Changed all references to ICount to icount (to match MAME requirements)
//TJL: (From Gilles Fetis) JPZ in TI documentation was wrong:
//     if ((pSR & SR_N) == 0 && (pSR & SR_Z) != 0)
//     should be:
//     if ((pSR & SR_N) == 0)

#include "emu.h"

void tms7000_device::illegal()
{
	/* This is a guess */
	m_icount -= 4;
}

void tms7000_device::adc_b2a()
{
	UINT16  t;

	t = RDA + RDB + GET_C;
	WRA(t);

	CLR_NZC;
	SET_C8(t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 5;
}

void tms7000_device::adc_r2a()
{
	UINT16  t;
	UINT8   v;

	IMMBYTE(v);

	t = RM(v) + RDA + GET_C;
	WRA(t);

	CLR_NZC;
	SET_C8(t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::adc_r2b()
{
	UINT16  t;
	UINT8   v;

	IMMBYTE(v);

	t = RM(v) + RDB + GET_C;
	WRB(t);

	CLR_NZC;
	SET_C8(t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::adc_r2r()
{
	UINT16  t;
	UINT8   i,j;

	IMMBYTE(i);
	IMMBYTE(j);

	t = RM(i)+RM(j) + GET_C;
	WM(j,t);

	CLR_NZC;
	SET_C8(t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 10;
}

void tms7000_device::adc_i2a()
{
	UINT16  t;
	UINT8   v;

	IMMBYTE(v);

	t = v + RDA + GET_C;
	WRA(t);

	CLR_NZC;
	SET_C8(t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 7;
}

void tms7000_device::adc_i2b()
{
	UINT16  t;
	UINT8   v;

	IMMBYTE(v);

	t = v + RDB + GET_C;
	WRB(t);

	CLR_NZC;
	SET_C8(t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 7;
}

void tms7000_device::adc_i2r()
{
	UINT16  t;
	UINT8   i,j;

	IMMBYTE(i);
	IMMBYTE(j);

	t = i+RM(j) + GET_C;
	WM(j,t);

	CLR_NZC;
	SET_C8(t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 9;
}

void tms7000_device::add_b2a()
{
	UINT16  t;

	t = RDA + RDB;
	WRA(t);

	CLR_NZC;
	SET_C8(t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 5;
}

void tms7000_device::add_r2a()
{
	UINT16  t;
	UINT8   v;

	IMMBYTE(v);

	t = RM(v) + RDA;
	WRA(t);

	CLR_NZC;
	SET_C8(t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::add_r2b()
{
	UINT16  t;
	UINT8   v;

	IMMBYTE(v);

	t = RM(v) + RDB;
	WRB(t);

	CLR_NZC;
	SET_C8(t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::add_r2r()
{
	UINT16  t;
	UINT8   i,j;

	IMMBYTE(i);
	IMMBYTE(j);

	t = RM(i)+RM(j);
	WM(j,t);

	CLR_NZC;
	SET_C8(t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 10;
}

void tms7000_device::add_i2a()
{
	UINT16  t;
	UINT8   v;

	IMMBYTE(v);

	t = v + RDA;
	WRA(t);

	CLR_NZC;
	SET_C8(t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 7;
}

void tms7000_device::add_i2b()
{
	UINT16  t;
	UINT8   v;

	IMMBYTE(v);

	t = v + RDB;
	WRB(t);

	CLR_NZC;
	SET_C8(t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 7;
}

void tms7000_device::add_i2r()
{
	UINT16  t;
	UINT8   i,j;

	IMMBYTE(i);
	IMMBYTE(j);

	t = i+RM(j);
	WM(j,t);

	CLR_NZC;
	SET_C8(t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 9;
}

void tms7000_device::and_b2a()
{
	UINT8   t;

	t = RDA & RDB;
	WRA(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 5;
}

void tms7000_device::and_r2a()
{
	UINT8   t;
	UINT8   v;

	IMMBYTE(v);

	t = RM(v) & RDA;
	WRA(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::and_r2b()
{
	UINT8   t;
	UINT8   v;

	IMMBYTE(v);

	t = RM(v) & RDB;
	WRB(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::and_r2r()
{
	UINT8   t;
	UINT8   i,j;

	IMMBYTE(i);
	IMMBYTE(j);

	t = RM(i) & RM(j);
	WM(j,t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 10;
}

void tms7000_device::and_i2a()
{
	UINT8   t;
	UINT8   v;

	IMMBYTE(v);

	t = v & RDA;
	WRA(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 7;
}

void tms7000_device::and_i2b()
{
	UINT8   t;
	UINT8   v;

	IMMBYTE(v);

	t = v & RDB;
	WRB(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 7;
}

void tms7000_device::and_i2r()
{
	UINT8   t;
	UINT8   i,j;

	IMMBYTE(i);
	IMMBYTE(j);

	t = i & RM(j);
	WM(j,t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 9;
}

void tms7000_device::andp_a2p()
{
	UINT8   t;
	UINT8   v;

	IMMBYTE(v);
	t = RDA & RM( 0x0100 + v);
	WM( 0x0100+v, t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 10;
}

void tms7000_device::andp_b2p()
{
	UINT8   t;
	UINT8   v;

	IMMBYTE(v);
	t = RDB & RM( 0x0100 + v);
	WM( 0x0100+v, t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 9;
}


void tms7000_device::movp_i2p()
{
	UINT8   i,v;

	IMMBYTE(i);
	IMMBYTE(v);
	WM( 0x0100+v, i);

	CLR_NZC;
	SET_N8(i);
	SET_Z8(i);

	m_icount -= 11;
}

void tms7000_device::andp_i2p()
{
	UINT8   t;
	UINT8   i,v;

	IMMBYTE(i);
	IMMBYTE(v);
	t = i & RM( 0x0100 + v);
	WM( 0x0100+v, t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 11;
}

void tms7000_device::br_dir()
{
	PAIR p;

	IMMWORD( p );
	pPC = p.d;
	m_icount -= 10;
}

void tms7000_device::br_ind()
{
	UINT8   v;

	IMMBYTE( v );
	PC.w.l = RRF16(v);

	m_icount -= 9;
}

void tms7000_device::br_inx()
{
	PAIR p;

	IMMWORD( p );
	pPC = p.w.l + RDB;
	m_icount -= 12;
}

void tms7000_device::btjo_b2a()
{
	UINT8   t;

	t = RDB & RDA;

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE( j );
		pPC += j;
		m_icount -= 9;
	}
	else
	{
		pPC++;
		m_icount -= 7;
	}
}

void tms7000_device::btjo_r2a()
{
	UINT8   t,r;

	IMMBYTE( r );
	t = RM( r ) & RDA;

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE( j );
		pPC += j;
		m_icount -= 9;
	}
	else
	{
		pPC++;
		m_icount -= 7;
	}
}

void tms7000_device::btjo_r2b()
{
	UINT8   t,r;

	IMMBYTE(r);
	t = RM(r) & RDB;

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE(j);
		pPC += j;
		m_icount -= 12;
	}
	else
	{
		pPC++;
		m_icount -= 10;
	}
}

void tms7000_device::btjo_r2r()
{
	UINT8   t,r,s;

	IMMBYTE(r);
	IMMBYTE(s);
	t = RM(r) & RM(s);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE(j);
		pPC += j;
		m_icount -= 14;
	}
	else
	{
		pPC++;
		m_icount -= 12;
	}
}

void tms7000_device::btjo_i2a()
{
	UINT8   t,r;

	IMMBYTE(r);
	t = r & RDA;

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE(j);
		pPC += j;
		m_icount -= 11;
	}
	else
	{
		pPC++;
		m_icount -= 9;
	}
}

void tms7000_device::btjo_i2b()
{
	UINT8   t,i;

	IMMBYTE(i);
	t = i & RDB;

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE(j);
		pPC += j;
		m_icount -= 11;
	}
	else
	{
		pPC++;
		m_icount -= 9;
	}
}

void tms7000_device::btjo_i2r()
{
	UINT8   t,i,r;

	IMMBYTE(i);
	IMMBYTE(r);
	t = i & RM(r);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE(j);
		pPC += j;
		m_icount -= 13;
	}
	else
	{
		pPC++;
		m_icount -= 11;
	}
}

void tms7000_device::btjop_ap()
{
	UINT8   t,p;

	IMMBYTE(p);

	t = RM(0x100+p) & RDA;

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE(j);
		pPC += j;
		m_icount -= 13;
	}
	else
	{
		pPC++;
		m_icount -= 11;
	}
}

void tms7000_device::btjop_bp()
{
	UINT8   t,p;

	IMMBYTE(p);

	t = RM(0x100+p) & RDB;

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE(j);
		pPC += j;
		m_icount -= 12;
	}
	else
	{
		pPC++;
		m_icount -= 10;
	}
}

void tms7000_device::btjop_ip()
{
	UINT8   t,p,i;

	IMMBYTE(i);
	IMMBYTE(p);

	t = RM(0x100+p) & i;

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE(j);
		pPC += j;
		m_icount -= 14;
	}
	else
	{
		pPC++;
		m_icount -= 12;
	}
}

void tms7000_device::btjz_b2a()
{
	UINT8   t;

	t = RDB & ~RDA;

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE( j );
		pPC += j;
		m_icount -= 9;
	}
	else
	{
		pPC++;
		m_icount -= 7;
	}
}

void tms7000_device::btjz_r2a()
{
	UINT8   t,r;

	IMMBYTE( r );
	t = RM( r ) & ~RDA;

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE( j );
		pPC += j;
		m_icount -= 9;
	}
	else
	{
		pPC++;
		m_icount -= 7;
	}
}

void tms7000_device::btjz_r2b()
{
	UINT8   t,r;

	IMMBYTE(r);
	t = RM(r) & ~RDB;

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE(j);
		pPC += j;
		m_icount -= 12;
	}
	else
	{
		pPC++;
		m_icount -= 10;
	}
}

void tms7000_device::btjz_r2r()
{
	UINT8   t,r,s;

	IMMBYTE(r);
	IMMBYTE(s);
	t = RM(r) & ~RM(s);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE(j);
		pPC += j;
		m_icount -= 14;
	}
	else
	{
		pPC++;
		m_icount -= 12;
	}
}

void tms7000_device::btjz_i2a()
{
	UINT8   t,r;

	IMMBYTE(r);
	t = r & ~RDA;

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE(j);
		pPC += j;
		m_icount -= 11;
	}
	else
	{
		pPC++;
		m_icount -= 9;
	}
}

void tms7000_device::btjz_i2b()
{
	UINT8   t,i;

	IMMBYTE(i);
	t = i & ~RDB;

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE(j);
		pPC += j;
		m_icount -= 11;
	}
	else
	{
		pPC++;
		m_icount -= 9;
	}
}

void tms7000_device::btjz_i2r()
{
	UINT8   t,i,r;

	IMMBYTE(i);
	IMMBYTE(r);
	t = i & ~RM(r);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE(j);
		pPC += j;
		m_icount -= 13;
	}
	else
	{
		pPC++;
		m_icount -= 11;
	}
}

void tms7000_device::btjzp_ap()
{
	UINT8   t,p;

	IMMBYTE(p);

	t = RDA & ~RM(0x100+p);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE(j);
		pPC += j;
		m_icount -= 13;
	}
	else
	{
		pPC++;
		m_icount -= 11;
	}
}

void tms7000_device::btjzp_bp()
{
	UINT8   t,p;

	IMMBYTE(p);

	t = RDB & ~RM(0x100+p);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE(j);
		pPC += j;
		m_icount -= 12;
	}
	else
	{
		pPC++;
		m_icount -= 10;
	}
}

void tms7000_device::btjzp_ip()
{
	UINT8   t,p,i;

	IMMBYTE(i);
	IMMBYTE(p);

	t = i & ~RM(0x100+p);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if(t != 0)
	{
		INT8    j;

		SIMMBYTE(j);
		pPC += j;
		m_icount -= 14;
	}
	else
	{
		pPC++;
		m_icount -= 12;
	}
}

void tms7000_device::call_dir()
{
	PAIR    tPC;

	IMMWORD( tPC );
	PUSHWORD( PC );
	pPC = tPC.d;

	m_icount -= 14;
}

void tms7000_device::call_ind()
{
	UINT8   v;

	IMMBYTE( v );
	PUSHWORD( PC );
	PC.w.l = RRF16(v);

	m_icount -= 13;
}

void tms7000_device::call_inx()
{
	PAIR    tPC;

	IMMWORD( tPC );
	PUSHWORD( PC );
	pPC = tPC.w.l + RDB;
	m_icount -= 16;
}

void tms7000_device::clr_a()
{
	WRA(0);
	m_icount -= 5;
}

void tms7000_device::clr_b()
{
	WRB(0);
	m_icount -= 5;
}

void tms7000_device::clr_r()
{
	UINT8   r;

	IMMBYTE(r);
	WM(r,0);
	m_icount -= 7;
}

void tms7000_device::clrc()
{
	UINT8   a;

	a = RDA;

	CLR_NZC;
	SET_N8(a);
	SET_Z8(a);

	m_icount -= 6;
}

void tms7000_device::cmp_ba()
{
	UINT16 t;

	t = RDA - RDB;

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if( t==0 )
		SETC;
	else
		SET_C8( ~t );

	m_icount -= 5;
}

void tms7000_device::cmp_ra()
{
	UINT16  t;
	UINT8   r;

	IMMBYTE(r);
	t = RDA - RM(r);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if( t==0 )
		SETC;
	else
		SET_C8( ~t );

	m_icount -= 8;
}

void tms7000_device::cmp_rb()
{
	UINT16  t;
	UINT8   r;

	IMMBYTE(r);
	t = RDB - RM(r);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if( t==0 )
		SETC;
	else
		SET_C8( ~t );

	m_icount -= 8;
}

void tms7000_device::cmp_rr()
{
	UINT16  t;
	UINT8   r,s;

	IMMBYTE(r);
	IMMBYTE(s);
	t = RM(s) - RM(r);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if( t==0 )
		SETC;
	else
		SET_C8( ~t );

	m_icount -= 10;
}

void tms7000_device::cmp_ia()
{
	UINT16  t;
	UINT8   i;

	IMMBYTE(i);
	t = RDA - i;

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if( t==0 )
		SETC;
	else
		SET_C8( ~t );

	m_icount -= 7;
}

void tms7000_device::cmp_ib()
{
	UINT16  t;
	UINT8   i;

	IMMBYTE(i);
	t = RDB - i;

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if( t==0 )
		SETC;
	else
		SET_C8( ~t );

	m_icount -= 7;
}

void tms7000_device::cmp_ir()
{
	UINT16  t;
	UINT8   i,r;

	IMMBYTE(i);
	IMMBYTE(r);
	t = RM(r) - i;

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if( t==0 )
		SETC;
	else
		SET_C8( ~t );

	m_icount -= 9;
}

void tms7000_device::cmpa_dir()
{
	UINT16  t;
	PAIR    i;

	IMMWORD( i );
	t = RDA - RM(i.w.l);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if( t==0 )
		SETC;
	else
		SET_C8( ~t );

	m_icount -= 12;
}

void tms7000_device::cmpa_ind()
{
	UINT16  t;
	PAIR    p;
	INT8    i;

	IMMBYTE(i);
	p.w.l = RRF16(i);
	t = RDA - RM(p.w.l);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if( t==0 )
		SETC;
	else
		SET_C8( ~t );

	m_icount -= 11;
}

void tms7000_device::cmpa_inx()
{
	UINT16  t;
	PAIR    i;

	IMMWORD( i );
	t = RDA - RM(i.w.l + RDB);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if( t==0 )
		SETC;
	else
		SET_C8( ~t );

	m_icount -= 14;
}

void tms7000_device::dac_b2a()
{
	WRA(bcd_add(RDA, RDB, pSR & SR_C));

	m_icount -= 7;
}

void tms7000_device::dac_r2a()
{
	UINT8 r;
	IMMBYTE(r);

	WRA(bcd_add(RDA, RM(r), pSR & SR_C));

	m_icount -= 10;
}

void tms7000_device::dac_r2b()
{
	UINT8 r;
	IMMBYTE(r);

	WRB(bcd_add(RDB, RM(r), pSR & SR_C));

	m_icount -= 10;
}

void tms7000_device::dac_r2r()
{
	UINT8 s, r;
	IMMBYTE(s);
	IMMBYTE(r);

	WM(r, bcd_add(RM(s), RM(r), pSR & SR_C));

	m_icount -= 12;
}

void tms7000_device::dac_i2a()
{
	UINT8 i;
	IMMBYTE(i);

	WRA(bcd_add(i, RDA, pSR & SR_C));

	m_icount -= 9;
}

void tms7000_device::dac_i2b()
{
	UINT8 i;
	IMMBYTE(i);

	WRB(bcd_add(i, RDB, pSR & SR_C));

	m_icount -= 9;
}

void tms7000_device::dac_i2r()
{
	UINT8 i, r;
	IMMBYTE(i);
	IMMBYTE(r);

	WM(r, bcd_add(i, RM(r), pSR & SR_C));

	m_icount -= 11;
}

void tms7000_device::dec_a()
{
	UINT16 t;

	t = RDA - 1;

	WRA( t );

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);
	SET_C8(~t);

	m_icount -= 5;
}

void tms7000_device::dec_b()
{
	UINT16 t;

	t = RDB - 1;

	WRB( t );

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);
	SET_C8(~t);

	m_icount -= 5;
}

void tms7000_device::dec_r()
{
	UINT16  t;
	UINT8   r;

	IMMBYTE(r);

	t = RM(r) - 1;

	WM( r, t );

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);
	SET_C8(~t);

	m_icount -= 7;
}

void tms7000_device::decd_a()
{
	PAIR    t;

	t.w.h = 0;
	t.w.l = RRF16(0);
	t.d -= 1;
	WRF16(0,t);

	CLR_NZC;
	SET_N8(t.b.h);
	SET_Z8(t.b.h);

	SET_C16(~(t.d));

	m_icount -= 9;
}

void tms7000_device::decd_b()
{
	PAIR    t;

	t.w.h = 0;
	t.w.l = RRF16(1);
	t.d -= 1;
	WRF16(1,t);

	CLR_NZC;
	SET_N8(t.b.h);
	SET_Z8(t.b.h);

	SET_C16(~(t.d));

	m_icount -= 9;
}

void tms7000_device::decd_r()
{
	UINT8   r;
	PAIR    t;

	IMMBYTE(r);
	t.w.h = 0;
	t.w.l = RRF16(r);
	t.d -= 1;
	WRF16(r,t);

	CLR_NZC;
	SET_N8(t.b.h);
	SET_Z8(t.b.h);

	SET_C16(~(t.d));

	m_icount -= 11;
}

void tms7000_device::dint()
{
	CLR_NZCI;
	m_icount -= 5;
}

void tms7000_device::djnz_a()
{
	UINT16 t;

	t = RDA - 1;

	WRA( t );

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if( t != 0 )
	{
		INT8    s;

		SIMMBYTE(s);
		pPC += s;
		m_icount -= 7;
	}
	else
	{
		pPC++;
		m_icount -= 2;
	}
}

void tms7000_device::djnz_b()
{
	UINT16 t;

	t = RDB - 1;

	WRB( t );

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if( t != 0 )
	{
		INT8    s;

		SIMMBYTE(s);
		pPC += s;
		m_icount -= 7;
	}
	else
	{
		pPC++;
		m_icount -= 2;
	}
}

void tms7000_device::djnz_r()
{
	UINT16  t;
	UINT8   r;

	IMMBYTE(r);

	t = RM(r) - 1;

	WM(r,t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	if( t != 0 )
	{
		INT8    s;

		SIMMBYTE(s);
		pPC += s;
		m_icount -= 9;
	}
	else
	{
		pPC++;
		m_icount -= 3;
	}
}

void tms7000_device::dsb_b2a()
{
	WRA(bcd_sub(RDA, RDB, pSR & SR_C));

	m_icount -= 7;
}

void tms7000_device::dsb_r2a()
{
	UINT8 r;
	IMMBYTE(r);

	WRA(bcd_sub(RDA, RM(r), pSR & SR_C));

	m_icount -= 10;
}

void tms7000_device::dsb_r2b()
{
	UINT8 r;
	IMMBYTE(r);

	WRB(bcd_sub(RDB, RM(r), pSR & SR_C));

	m_icount -= 10;
}

void tms7000_device::dsb_r2r()
{
	UINT8 s, r;
	IMMBYTE(s);
	IMMBYTE(r);

	WM(r, bcd_sub(RM(s), RM(r), pSR & SR_C));

	m_icount -= 12;
}

void tms7000_device::dsb_i2a()
{
	UINT8 i;
	IMMBYTE(i);

	WRA(bcd_sub(RDA, i, pSR & SR_C));

	m_icount -= 9;
}

void tms7000_device::dsb_i2b()
{
	UINT8 i;
	IMMBYTE(i);

	WRB(bcd_sub(RDB, i, pSR & SR_C));

	m_icount -= 9;
}

void tms7000_device::dsb_i2r()
{
	UINT8 i, r;
	IMMBYTE(i);
	IMMBYTE(r);

	WM(r, bcd_sub(RM(r), i, pSR & SR_C));

	m_icount -= 11;
}

void tms7000_device::eint()
{
	pSR |= (SR_N|SR_Z|SR_C|SR_I);
	m_icount -= 5;
	tms7000_check_IRQ_lines();
}

void tms7000_device::idle()
{
	m_idle_state = 1;
	m_icount -= 6;
}

void tms7000_device::inc_a()
{
	UINT16  t;

	t = RDA + 1;

	WRA( t );

	CLR_NZC;
	SET_C8(t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 5;
}

void tms7000_device::inc_b()
{
	UINT16  t;

	t = RDB + 1;

	WRB( t );

	CLR_NZC;
	SET_C8(t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 5;
}

void tms7000_device::inc_r()
{
	UINT16  t;
	UINT8   r;

	IMMBYTE(r);

	t = RM(r) + 1;

	WM( r, t );

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);
	SET_C8(t);

	m_icount -= 7;
}

void tms7000_device::inv_a()
{
	UINT16 t;

	t = ~(RDA);
	WRA(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 5;
}

void tms7000_device::inv_b()
{
	UINT16 t;

	t = ~(RDB);
	WRB(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 5;
}

void tms7000_device::inv_r()
{
	UINT16  t;
	UINT8   r;

	IMMBYTE(r);

	t = ~(RM(r));

	WM( r, t );

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 7;
}

void tms7000_device::jc()
{
	if( pSR & SR_C )
	{
		INT8 s;

		SIMMBYTE( s );
		pPC += s;
		m_icount -= 7;
	}
	else
	{
		pPC++;
		m_icount -= 5;
	}
}

void tms7000_device::jeq()
{
	if( pSR & SR_Z )
	{
		INT8 s;

		SIMMBYTE( s );
		pPC += s;
		m_icount -= 7;
	}
	else
	{
		pPC++;
		m_icount -= 5;
	}
}

void tms7000_device::jl()
{
	if( pSR & SR_C )
	{
		pPC++;
		m_icount -= 5;
	}
	else
	{
		INT8 s;

		SIMMBYTE( s );
		pPC += s;
		m_icount -= 7;
	}
}

void tms7000_device::jmp()
{
	INT8 s;

	SIMMBYTE( s );
	pPC += s;
	m_icount -= 7;
}

void tms7000_device::j_jn()
{
	if( pSR & SR_N )
	{
		INT8 s;

		SIMMBYTE( s );
		pPC += s;
		m_icount -= 7;
	}
	else
	{
		pPC++;
		m_icount -= 5;
	}

}

void tms7000_device::jne()
{
	if( pSR & SR_Z )
	{
		pPC++;
		m_icount -= 5;
	}
	else
	{
		INT8 s;

		SIMMBYTE( s );
		pPC += s;
		m_icount -= 7;
	}
}

void tms7000_device::jp()
{
	if( pSR & (SR_Z|SR_N) )
	{
		pPC++;
		m_icount -= 5;
	}
	else
	{
		INT8 s;

		SIMMBYTE( s );
		pPC += s;
		m_icount -= 7;
	}
}

void tms7000_device::jpz()
{
	if ((pSR & SR_N) == 0)
	{
		INT8 s;

		SIMMBYTE( s );
		pPC += s;
		m_icount -= 7;
	}
	else
	{
		pPC++;
		m_icount -= 5;
	}
}

void tms7000_device::lda_dir()
{
	UINT16  t;
	PAIR    i;

	IMMWORD( i );
	t = RM(i.w.l);
	WRA(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 11;
}

void tms7000_device::lda_ind()
{
	UINT16  t;
	PAIR    p;
	INT8    i;

	IMMBYTE(i);
	p.w.l=RRF16(i);
	t = RM(p.w.l);
	WRA(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 10;
}

void tms7000_device::lda_inx()
{
	UINT16  t;
	PAIR    i;

	IMMWORD( i );
	t = RM(i.w.l + RDB);
	WRA(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 13;
}

void tms7000_device::ldsp()
{
	pSP = RDB;
	m_icount -= 5;
}

void tms7000_device::mov_a2b()
{
	UINT16  t;

	t = RDA;
	WRB(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 6;
}

void tms7000_device::mov_b2a()
{
	UINT16  t;

	t = RDB;
	WRA(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 5;
}


void tms7000_device::mov_a2r()
{
	UINT8   r;
	UINT16  t;

	IMMBYTE(r);

	t = RDA;
	WM(r,t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::mov_b2r()
{
	UINT8   r;
	UINT16  t;

	IMMBYTE(r);

	t = RDB;
	WM(r,t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 7;
}

void tms7000_device::mov_r2a()
{
	UINT8   r;
	UINT16  t;

	IMMBYTE(r);
	t = RM(r);
	WRA(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::mov_r2b()
{
	UINT8   r;
	UINT16  t;

	IMMBYTE(r);
	t = RM(r);
	WRB(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::mov_r2r()
{
	UINT8   r,s;
	UINT16  t;

	IMMBYTE(r);
	IMMBYTE(s);
	t = RM(r);
	WM(s,t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 10;
}

void tms7000_device::mov_i2a()
{
	UINT16  t;

	IMMBYTE(t);
	WRA(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 7;
}

void tms7000_device::mov_i2b()
{
	UINT16  t;

	IMMBYTE(t);
	WRB(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 7;
}

void tms7000_device::mov_i2r()
{
	UINT16  t;
	UINT8   r;

	IMMBYTE(t);
	IMMBYTE(r);
	WM(r,t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 9;
}

void tms7000_device::movd_imm()
{
	PAIR    t;
	UINT8   r;

	IMMWORD(t);
	IMMBYTE(r);
	WRF16(r,t);

	CLR_NZC;
	SET_N8(t.b.h);
	SET_Z8(t.b.h);

	m_icount -= 15;

}

void tms7000_device::movd_r()
{
	PAIR    t;
	UINT8   r,s;

	IMMBYTE(r);
	IMMBYTE(s);
	t.w.l = RRF16(r);
	WRF16(s,t);

	CLR_NZC;
	SET_N8(t.b.h);
	SET_Z8(t.b.h);

	m_icount -= 14;

}

void tms7000_device::movd_inx()
{
	PAIR    t;
	UINT8   r;

	IMMWORD(t);
	t.w.l += RDB;
	IMMBYTE(r);
	WRF16(r,t);

	CLR_NZC;
	SET_N8(t.b.h);
	SET_Z8(t.b.h);

	m_icount -= 17;
}

void tms7000_device::movp_a2p()
{
	UINT8   p;
	UINT16  t;

	IMMBYTE(p);
	t=RDA;
	WM( 0x0100+p,t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 10;
}

void tms7000_device::movp_b2p()
{
	UINT8   p;
	UINT16  t;

	IMMBYTE(p);
	t=RDB;
	WM( 0x0100+p,t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 10;
}

#if 0
/* this appears to be unused */
void tms7000_device::movp_r2p()
{
	UINT8   p,r;
	UINT16  t;

	IMMBYTE(r);
	IMMBYTE(p);
	t=RM(r);
	WM( 0x0100+p,t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 11;
}
#endif

void tms7000_device::movp_p2a()
{
	UINT8   p;
	UINT16  t;

	IMMBYTE(p);
	t=RM(0x0100+p);
	WRA(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 9;
}

void tms7000_device::movp_p2b()
{
	UINT8   p;
	UINT16  t;

	IMMBYTE(p);
	t=RM(0x0100+p);
	WRB(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::mpy_ba()
{
	PAIR t;

	t.w.l = RDA * RDB;

	WRF16(0x01,t);

	CLR_NZC;
	SET_N8(t.b.h);
	SET_Z8(t.b.h);

	m_icount -= 43;

}

void tms7000_device::mpy_ra()
{
	PAIR    t;
	UINT8   r;

	IMMBYTE(r);

	t.w.l = RDA * RM(r);

	WRF16(0x01,t);

	CLR_NZC;
	SET_N8(t.b.h);
	SET_Z8(t.b.h);

	m_icount -= 46;

}

void tms7000_device::mpy_rb()
{
	PAIR    t;
	UINT8   r;

	IMMBYTE(r);

	t.w.l = RDB * RM(r);

	WRF16(0x01,t);

	CLR_NZC;
	SET_N8(t.b.h);
	SET_Z8(t.b.h);

	m_icount -= 46;

}

void tms7000_device::mpy_rr()
{
	PAIR    t;
	UINT8   r,s;

	IMMBYTE(r);
	IMMBYTE(s);

	t.w.l = RM(s) * RM(r);

	WRF16(0x01,t);

	CLR_NZC;
	SET_N8(t.b.h);
	SET_Z8(t.b.h);

	m_icount -= 48;

}

void tms7000_device::mpy_ia()
{
	PAIR    t;
	UINT8   i;

	IMMBYTE(i);

	t.w.l = RDA * i;

	WRF16(0x01,t);

	CLR_NZC;
	SET_N8(t.b.h);
	SET_Z8(t.b.h);

	m_icount -= 45;

}

void tms7000_device::mpy_ib()
{
	PAIR    t;
	UINT8   i;

	IMMBYTE(i);

	t.w.l = RDB * i;

	WRF16(0x01,t);

	CLR_NZC;
	SET_N8(t.b.h);
	SET_Z8(t.b.h);

	m_icount -= 45;

}

void tms7000_device::mpy_ir()
{
	PAIR    t;
	UINT8   i,r;

	IMMBYTE(i);
	IMMBYTE(r);

	t.w.l = RM(r) * i;

	WRF16(0x01,t);

	CLR_NZC;
	SET_N8(t.b.h);
	SET_Z8(t.b.h);

	m_icount -= 47;

}

void tms7000_device::nop()
{
	m_icount -= 4;
}

void tms7000_device::or_b2a()
{
	UINT8   t;

	t = RDA | RDB;
	WRA(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 5;
}

void tms7000_device::or_r2a()
{
	UINT8   t;
	UINT8   v;

	IMMBYTE(v);

	t = RM(v) | RDA;
	WRA(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::or_r2b()
{
	UINT8   t;
	UINT8   v;

	IMMBYTE(v);

	t = RM(v) | RDB;
	WRB(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::or_r2r()
{
	UINT8   t;
	UINT8   i,j;

	IMMBYTE(i);
	IMMBYTE(j);

	t = RM(i) | RM(j);
	WM(j,t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 10;
}

void tms7000_device::or_i2a()
{
	UINT8   t;
	UINT8   v;

	IMMBYTE(v);

	t = v | RDA;
	WRA(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 7;
}

void tms7000_device::or_i2b()
{
	UINT8   t;
	UINT8   v;

	IMMBYTE(v);

	t = v | RDB;
	WRB(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 7;
}

void tms7000_device::or_i2r()
{
	UINT8   t;
	UINT8   i,j;

	IMMBYTE(i);
	IMMBYTE(j);

	t = i | RM(j);
	WM(j,t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 9;
}

void tms7000_device::orp_a2p()
{
	UINT8   t;
	UINT8   v;

	IMMBYTE(v);
	t = RDA | RM( 0x0100 + v);
	WM( 0x0100+v, t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 10;
}

void tms7000_device::orp_b2p()
{
	UINT8   t;
	UINT8   v;

	IMMBYTE(v);
	t = RDB | RM( 0x0100 + v);
	WM( 0x0100+v, t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 9;
}

void tms7000_device::orp_i2p()
{
	UINT8   t;
	UINT8   i,v;

	IMMBYTE(i);
	IMMBYTE(v);
	t = i | RM( 0x0100 + v);
	WM( 0x0100+v, t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 11;
}

void tms7000_device::pop_a()
{
	UINT16  t;

	PULLBYTE(t);
	WRA(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 6;
}

void tms7000_device::pop_b()
{
	UINT16  t;

	PULLBYTE(t);
	WRB(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 6;
}

void tms7000_device::pop_r()
{
	UINT16  t;
	UINT8   r;

	IMMBYTE(r);
	PULLBYTE(t);
	WM(r,t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::pop_st()
{
	UINT16  t;

	PULLBYTE(t);
	pSR = t;

	m_icount -= 6;
}

void tms7000_device::push_a()
{
	UINT16  t;

	t = RDA;
	PUSHBYTE(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 6;
}

void tms7000_device::push_b()
{
	UINT16  t;

	t = RDB;
	PUSHBYTE(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 6;
}

void tms7000_device::push_r()
{
	UINT16  t;
	INT8    r;

	IMMBYTE(r);
	t = RM(r);
	PUSHBYTE(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::push_st()
{
	UINT16  t;
	t = pSR;
	PUSHBYTE(t);

	m_icount -= 6;
}

void tms7000_device::reti()
{
	PULLWORD( PC );
	PULLBYTE( pSR );

	m_icount -= 9;
	tms7000_check_IRQ_lines();
}

void tms7000_device::rets()
{
	PULLWORD( PC );
	m_icount -= 7;
}

void tms7000_device::rl_a()
{
	UINT16  t;

	t = RDA << 1;

	CLR_NZC;
	SET_C8(t);

	if( pSR & SR_C )
		t |= 0x01;

	SET_N8(t);
	SET_Z8(t);
	WRA(t);

	m_icount -= 5;
}

void tms7000_device::rl_b()
{
	UINT16  t;

	t = RDB << 1;

	CLR_NZC;
	SET_C8(t);

	if( pSR & SR_C )
		t |= 0x01;

	SET_N8(t);
	SET_Z8(t);
	WRB(t);

	m_icount -= 5;
}

void tms7000_device::rl_r()
{
	UINT16  t;
	UINT8   r;

	IMMBYTE(r);
	t = RM(r) << 1;

	CLR_NZC;
	SET_C8(t);

	if( pSR & SR_C )
		t |= 0x01;

	SET_N8(t);
	SET_Z8(t);
	WM(r,t);

	m_icount -= 7;
}

void tms7000_device::rlc_a()
{
	UINT16  t;
	int     old_carry;

	old_carry = (pSR & SR_C);

	t = RDA << 1;

	CLR_NZC;
	SET_C8(t);

	if( old_carry )
		t |= 0x01;

	SET_N8(t);
	SET_Z8(t);
	WRA(t);

	m_icount -= 5;
}

void tms7000_device::rlc_b()
{
	UINT16  t;
	int     old_carry;

	old_carry = (pSR & SR_C);

	t = RDB << 1;

	CLR_NZC;
	SET_C8(t);

	if( old_carry )
		t |= 0x01;

	SET_N8(t);
	SET_Z8(t);
	WRB(t);

	m_icount -= 5;
}

void tms7000_device::rlc_r()
{
	UINT16  t;
	UINT8   r;
	int     old_carry;

	old_carry = (pSR & SR_C);

	IMMBYTE(r);
	t = RM(r) << 1;

	CLR_NZC;
	SET_C8(t);

	if( old_carry )
		t |= 0x01;

	SET_N8(t);
	SET_Z8(t);
	WM(r,t);

	m_icount -= 7;
}

void tms7000_device::rr_a()
{
	UINT16  t;
	int     old_bit0;

	t = RDA;

	old_bit0 = t & 0x0001;
	t = t >> 1;

	CLR_NZC;

	if( old_bit0 )
	{
		SETC;
		t |= 0x80;
	}

	SET_N8(t);
	SET_Z8(t);

	WRA(t);

	m_icount -= 5;
}

void tms7000_device::rr_b()
{
	UINT16  t;
	int     old_bit0;

	t = RDB;

	old_bit0 = t & 0x0001;
	t = t >> 1;

	CLR_NZC;

	if( old_bit0 )
	{
		SETC;
		t |= 0x80;
	}

	SET_N8(t);
	SET_Z8(t);

	WRB(t);

	m_icount -= 5;
}

void tms7000_device::rr_r()
{
	UINT16  t;
	UINT8   r;

	int     old_bit0;

	IMMBYTE(r);
	t = RM(r);

	old_bit0 = t & 0x0001;
	t = t >> 1;

	CLR_NZC;

	if( old_bit0 )
	{
		SETC;
		t |= 0x80;
	}

	SET_N8(t);
	SET_Z8(t);

	WM(r,t);

	m_icount -= 7;
}

void tms7000_device::rrc_a()
{
	UINT16  t;
	int     old_bit0;

	t = RDA;

	old_bit0 = t & 0x0001;
	/* Place carry bit in 9th position */
	t |= ((pSR & SR_C) << 1);
	t = t >> 1;

	CLR_NZC;

	if( old_bit0 )
		SETC;
	SET_N8(t);
	SET_Z8(t);

	WRA(t);

	m_icount -= 5;
}

void tms7000_device::rrc_b()
{
	UINT16  t;
	int     old_bit0;

	t = RDB;

	old_bit0 = t & 0x0001;
	/* Place carry bit in 9th position */
	t |= ((pSR & SR_C) << 1);
	t = t >> 1;

	CLR_NZC;

	if( old_bit0 )
		SETC;
	SET_N8(t);
	SET_Z8(t);

	WRB(t);

	m_icount -= 5;
}

void tms7000_device::rrc_r()
{
	UINT16  t;
	UINT8   r;
	int     old_bit0;

	IMMBYTE(r);
	t = RM(r);

	old_bit0 = t & 0x0001;
	/* Place carry bit in 9th position */
	t |= ((pSR & SR_C) << 1);
	t = t >> 1;

	CLR_NZC;

	if( old_bit0 )
		SETC;
	SET_N8(t);
	SET_Z8(t);

	WM(r,t);

	m_icount -= 7;
}

void tms7000_device::sbb_ba()
{
	UINT16  t;

	t = RDA - RDB - ((pSR & SR_C) ? 0 : 1);
	WRA(t);

	CLR_NZC;
	SET_C8(~t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 5;
}

void tms7000_device::sbb_ra()
{
	UINT16  t;
	UINT8   r;

	IMMBYTE(r);
	t = RDA - RM(r) - ((pSR & SR_C) ? 0 : 1);
	WRA(t);

	CLR_NZC;
	SET_C8(~t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::sbb_rb()
{
	UINT16  t;
	UINT8   r;

	IMMBYTE(r);
	t = RDB - RM(r) - ((pSR & SR_C) ? 0 : 1);
	WRB(t);

	CLR_NZC;
	SET_C8(~t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::sbb_rr()
{
	UINT16  t;
	UINT8   r,s;

	IMMBYTE(s);
	IMMBYTE(r);
	t = RM(r) - RM(s) - ((pSR & SR_C) ? 0 : 1);
	WM(r,t);

	CLR_NZC;
	SET_C8(~t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 10;
}

void tms7000_device::sbb_ia()
{
	UINT16  t;
	UINT8   i;

	IMMBYTE(i);
	t = RDA - i - ((pSR & SR_C) ? 0 : 1);
	WRA(t);

	CLR_NZC;
	SET_C8(~t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 7;
}

void tms7000_device::sbb_ib()
{
	UINT16  t;
	UINT8   i;

	IMMBYTE(i);
	t = RDB - i - ((pSR & SR_C) ? 0 : 1);
	WRB(t);

	CLR_NZC;
	SET_C8(~t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 7;
}

void tms7000_device::sbb_ir()
{
	UINT16  t;
	UINT8   r,i;

	IMMBYTE(i);
	IMMBYTE(r);
	t = RM(r) - i - ((pSR & SR_C) ? 0 : 1);
	WM(r,t);

	CLR_NZC;
	SET_C8(~t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 9;
}

void tms7000_device::setc()
{
	CLR_NZC;
	pSR |= (SR_C|SR_Z);

	m_icount -= 5;
}

void tms7000_device::sta_dir()
{
	UINT16  t;
	PAIR    i;

	t = RDA;
	IMMWORD( i );

	WM(i.w.l,t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 11;
}

void tms7000_device::sta_ind()
{
	UINT16  t;
	PAIR    p;
	INT8    r;

	IMMBYTE(r);
	p.w.l = RRF16(r);
	t = RDA;
	WM(p.w.l,t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 10;
}

void tms7000_device::sta_inx()
{
	UINT16  t;
	PAIR    i;

	IMMWORD( i );
	t = RDA;
	WM(i.w.l+RDB,t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 13;
}

void tms7000_device::stsp()
{
	WRB(pSP);

	m_icount -= 6;
}

void tms7000_device::sub_ba()
{
	UINT16  t;

	t = RDA - RDB;
	WRA(t);

	CLR_NZC;
	SET_C8(~t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 5;
}

void tms7000_device::sub_ra()
{
	UINT16  t;
	UINT8   r;

	IMMBYTE(r);
	t = RDA - RM(r);
	WRA(t);

	CLR_NZC;
	SET_C8(~t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::sub_rb()
{
	UINT16  t;
	UINT8   r;

	IMMBYTE(r);
	t = RDB - RM(r);
	WRB(t);

	CLR_NZC;
	SET_C8(~t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::sub_rr()
{
	UINT16  t;
	UINT8   r,s;

	IMMBYTE(s);
	IMMBYTE(r);
	t = RM(r) - RM(s);
	WM(r,t);

	CLR_NZC;
	SET_C8(~t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 10;
}

void tms7000_device::sub_ia()
{
	UINT16  t;
	UINT8   i;

	IMMBYTE(i);
	t = RDA - i;
	WRA(t);

	CLR_NZC;
	SET_C8(~t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 7;
}

void tms7000_device::sub_ib()
{
	UINT16  t;
	UINT8   i;

	IMMBYTE(i);
	t = RDB - i;
	WRB(t);

	CLR_NZC;
	SET_C8(~t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 7;
}

void tms7000_device::sub_ir()
{
	UINT16  t;
	UINT8   r,i;

	IMMBYTE(i);
	IMMBYTE(r);
	t = RM(r) - i;
	WM(r,t);

	CLR_NZC;
	SET_C8(~t);
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 9;
}

void tms7000_device::trap_0()
{
	PUSHWORD( PC );
	pPC = RM16(0xfffe);
	m_icount -= 14;
}

void tms7000_device::trap_1()
{
	PUSHWORD( PC );
	pPC = RM16(0xfffc);
	m_icount -= 14;
}

void tms7000_device::trap_2()
{
	PUSHWORD( PC );
	pPC = RM16(0xfffa);
	m_icount -= 14;
}

void tms7000_device::trap_3()
{
	PUSHWORD( PC );
	pPC = RM16(0xfff8);
	m_icount -= 14;
}

void tms7000_device::trap_4()
{
	PUSHWORD( PC );
	pPC = RM16(0xfff6);
	m_icount -= 14;
}

void tms7000_device::trap_5()
{
	PUSHWORD( PC );
	pPC = RM16(0xfff4);
	m_icount -= 14;
}

void tms7000_device::trap_6()
{
	PUSHWORD( PC );
	pPC = RM16(0xfff2);
	m_icount -= 14;
}

void tms7000_device::trap_7()
{
	PUSHWORD( PC );
	pPC = RM16(0xfff0);
	m_icount -= 14;
}

void tms7000_device::trap_8()
{
	PUSHWORD( PC );
	pPC = RM16(0xffee);
	m_icount -= 14;
}

void tms7000_device::trap_9()
{
	PUSHWORD( PC );
	pPC = RM16(0xffec);
	m_icount -= 14;
}

void tms7000_device::trap_10()
{
	PUSHWORD( PC );
	pPC = RM16(0xffea);
	m_icount -= 14;
}

void tms7000_device::trap_11()
{
	PUSHWORD( PC );
	pPC = RM16(0xffe8);
	m_icount -= 14;
}

void tms7000_device::trap_12()
{
	PUSHWORD( PC );
	pPC = RM16(0xffe6);
	m_icount -= 14;
}

void tms7000_device::trap_13()
{
	PUSHWORD( PC );
	pPC = RM16(0xffe4);
	m_icount -= 14;
}

void tms7000_device::trap_14()
{
	PUSHWORD( PC );
	pPC = RM16(0xffe2);
	m_icount -= 14;
}

void tms7000_device::trap_15()
{
	PUSHWORD( PC );
	pPC = RM16(0xffe0);
	m_icount -= 14;
}

void tms7000_device::trap_16()
{
	PUSHWORD( PC );
	pPC = RM16(0xffde);
	m_icount -= 14;
}

void tms7000_device::trap_17()
{
	PUSHWORD( PC );
	pPC = RM16(0xffdc);
	m_icount -= 14;
}

void tms7000_device::trap_18()
{
	PUSHWORD( PC );
	pPC = RM16(0xffda);
	m_icount -= 14;
}

void tms7000_device::trap_19()
{
	PUSHWORD( PC );
	pPC = RM16(0xffd8);
	m_icount -= 14;
}

void tms7000_device::trap_20()
{
	PUSHWORD( PC );
	pPC = RM16(0xffd6);
	m_icount -= 14;
}

void tms7000_device::trap_21()
{
	PUSHWORD( PC );
	pPC = RM16(0xffd4);
	m_icount -= 14;
}

void tms7000_device::trap_22()
{
	PUSHWORD( PC );
	pPC = RM16(0xffd2);
	m_icount -= 14;
}

void tms7000_device::trap_23()
{
	PUSHWORD( PC );
	pPC = RM16(0xffd0);
	m_icount -= 14;
}

void tms7000_device::swap_a()
{
	UINT8   a,b;
	UINT16  t;

	a = b = RDA;

	a <<= 4;
	b >>= 4;
	t = a+b;

	WRA(t);

	CLR_NZC;

	pSR|=((t&0x0001)<<7);
	SET_N8(t);
	SET_Z8(t);

	m_icount -=8;
}

void tms7000_device::swap_b()
{
	UINT8   a,b;
	UINT16  t;

	a = b = RDB;

	a <<= 4;
	b >>= 4;
	t = a+b;

	WRB(t);

	CLR_NZC;

	pSR|=((t&0x0001)<<7);
	SET_N8(t);
	SET_Z8(t);

	m_icount -=8;
}

void tms7000_device::swap_r()
{
	UINT8   a,b,r;
	UINT16  t;

	IMMBYTE(r);
	a = b = RM(r);

	a <<= 4;
	b >>= 4;
	t = a+b;

	WM(r,t);

	CLR_NZC;

	pSR|=((t&0x0001)<<7);
	SET_N8(t);
	SET_Z8(t);

	m_icount -=8;
}

void tms7000_device::swap_r_exl()
{
	UINT8   a,b,r;
	UINT16  t;

	IMMBYTE(r);

	if (r == 0)
	{   /* opcode D7 00 (LVDP) mostly equivalent to MOVP P46,A??? (timings must
        be different, possibly the microcode polls the state of the VDP RDY
        line prior to doing the transfer) */
		t=RM(0x012e);
		WRA(t);

		CLR_NZC;
		SET_N8(t);
		SET_Z8(t);

		m_icount -= 9;  /* ?????? */
	}
	else
	{   /* stright swap Rn instruction */
		a = b = RM(r);

		a <<= 4;
		b >>= 4;
		t = a+b;

		WM(r,t);

		CLR_NZC;

		pSR|=((t&0x0001)<<7);
		SET_N8(t);
		SET_Z8(t);

		m_icount -=8;
	}
}

void tms7000_device::tstb()
{
	UINT16  t;

	t=RDB;

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 6;
}

void tms7000_device::xchb_a()
{
	UINT16  t,u;

	t = RDB;
	u = RDA;

	WRA(t);
	WRB(u);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 6;
}

void tms7000_device::xchb_b()
{
	UINT16  t;
/*  UINT16  u;  */

	t = RDB;
/*  u = RDB;    */

/*  WRB(t);     */
/*  WRB(u);     */

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 6;
}

void tms7000_device::xchb_r()
{
	UINT16  t,u;
	UINT8   r;

	IMMBYTE(r);

	t = RDB;
	u = RM(r);

	WRA(t);
	WRB(u);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::xor_b2a()
{
	UINT8   t;

	t = RDA ^ RDB;
	WRA(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 5;
}

void tms7000_device::xor_r2a()
{
	UINT8   t;
	UINT8   v;

	IMMBYTE(v);

	t = RM(v) ^ RDA;
	WRA(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::xor_r2b()
{
	UINT8   t;
	UINT8   v;

	IMMBYTE(v);

	t = RM(v) ^ RDB;
	WRB(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 8;
}

void tms7000_device::xor_r2r()
{
	UINT8   t;
	UINT8   i,j;

	IMMBYTE(i);
	IMMBYTE(j);

	t = RM(i) ^ RM(j);
	WM(j,t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 10;
}

void tms7000_device::xor_i2a()
{
	UINT8   t;
	UINT8   v;

	IMMBYTE(v);

	t = v ^ RDA;
	WRA(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 7;
}

void tms7000_device::xor_i2b()
{
	UINT8   t;
	UINT8   v;

	IMMBYTE(v);

	t = v ^ RDB;
	WRB(t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 7;
}

void tms7000_device::xor_i2r()
{
	UINT8   t;
	UINT8   i,j;

	IMMBYTE(i);
	IMMBYTE(j);

	t = i ^ RM(j);
	WM(j,t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 9;
}

void tms7000_device::xorp_a2p()
{
	UINT8   t;
	UINT8   v;

	IMMBYTE(v);
	t = RDA ^ RM( 0x0100 + v);
	WM( 0x0100+v, t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 10;
}

void tms7000_device::xorp_b2p()
{
	UINT8   t;
	UINT8   v;

	IMMBYTE(v);
	t = RDB ^ RM( 0x0100 + v);
	WM( 0x0100+v, t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 9;
}

void tms7000_device::xorp_i2p()
{
	UINT8   t;
	UINT8   i,v;

	IMMBYTE(i);
	IMMBYTE(v);
	t = i ^ RM( 0x0100 + v);
	WM( 0x0100+v, t);

	CLR_NZC;
	SET_N8(t);
	SET_Z8(t);

	m_icount -= 11;
}
