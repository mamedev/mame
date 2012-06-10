/***************************************************************************

Pong (c) 1972 Atari

driver by Couriersud

Notes:

TODO:

============================================================================


***************************************************************************/


#include "emu.h"

#include "machine/rescap.h"
#include "sound/dac.h"

/*
 * H count width to 512
 * Reset at 1C6 = 454
 * V count width to 512, counts on HReset
 * Reset at 105 = 261

 * Clock = 7.159 MHz

 * ==> 15.768 Khz Horz Freq
 * ==> 60.41 Refresh

 * HBlank 0 to 79
 * HSync 32 to 63
 * VBlank 0 to 15
 * VSync 4 to 7

 * Video = (HVID & VVID ) & (NET & PAD1 & PAD2)

 * Net at 256H alternating at 4V
 *
 *
 * http://www.youtube.com/watch?v=pDrRnJOCKZc
 */

#define MASTER_CLOCK 	7139000
#define V_TOTAL 		(0x105+1)
#define	H_TOTAL			(0x1C6+1)   	// 454

#define HBSTART					(H_TOTAL)
#define HBEND					(80)
#define VBSTART					(V_TOTAL)
#define VBEND					(16)

static UINT8 tab7448[16][7] =
{
		{	1, 1, 1, 1, 1, 1, 0 },  /* 00 - not blanked ! */
		{   0, 1, 1, 0, 0, 0, 0 },  /* 01 */
		{   1, 1, 0, 1, 1, 0, 1 },  /* 02 */
		{   1, 1, 1, 1, 0, 0, 1 },  /* 03 */
		{   0, 1, 1, 0, 0, 1, 1 },  /* 04 */
		{   1, 0, 1, 1, 0, 1, 1 },  /* 05 */
		{   0, 0, 1, 1, 1, 1, 1 },  /* 06 */
		{   1, 1, 1, 0, 0, 0, 0 },  /* 07 */
		{   1, 1, 1, 1, 1, 1, 1 },  /* 08 */
		{   1, 1, 1, 0, 0, 1, 1 },  /* 09 */
		{   0, 0, 0, 1, 1, 0, 1 },  /* 10 */
		{   0, 0, 1, 1, 0, 0, 1 },  /* 11 */
		{   0, 1, 0, 0, 0, 1, 1 },  /* 12 */
		{   1, 0, 0, 1, 0, 1, 1 },  /* 13 */
		{   0, 0, 0, 1, 1, 1, 1 },  /* 14 */
		{   0, 0, 0, 0, 0, 0, 0 },  /* 15 */
};

class icMonoFlop
{
public:
	icMonoFlop() { m_Q = 0; m_clocks = 0; m_last = 0; m_cnt = 0;}

	void setPeriod555Std(double R, double C, double dt)
	{
		m_clocks = (1.1 * R * C) / dt;
	}

	inline void process(UINT8 in_trigger)
	{
		if (m_last && !in_trigger)
		{
			m_cnt = m_clocks;
			m_Q = 1;
		}
		m_last = in_trigger;
		if (m_Q)
		{
			m_cnt--;
			if (m_cnt <= 0)
			{
				m_Q = 0;
			}
		}
	}

	inline UINT8 Q() { return m_Q; }

	int m_clocks;

private:
	UINT8 m_Q;
	int m_last;
	int m_cnt;
};


class icRSFF
{
public:
	icRSFF() { m_Q = 0; }

	inline void process(UINT8 in_S, UINT8 in_R)
	{
		if (in_S)
			m_Q = 1;
		else if (in_R)
			m_Q = 0;
	}

	inline UINT8 Q() { return m_Q; }
	inline UINT8 QQ() { return !m_Q; }
private:
	UINT8 m_Q;
};


class ic7474
{
public:
	ic7474() { m_lastclk = 0; m_Q = 0; }

	void process(UINT8 clk, UINT8 in_D, UINT8 clrQ, UINT8 preQ)
	{
		if (!preQ)
			m_Q = 1;
		else if (!clrQ)
			m_Q = 0;
		else if (!m_lastclk && clk)
			m_Q = in_D;
		m_lastclk = clk;
	}

	inline UINT8 Q() { return m_Q; }
	inline UINT8 QQ() { return !m_Q; }
private:
	UINT8 m_lastclk;
	UINT8 m_Q;
};

/* 74107 does latch data during high !
 * For modelling purposes, we assume 74107 and 74107A are the same
 * */

class ic74107A
{
public:
	ic74107A() { m_lastclk = 0; m_Q = 0; }

	void process(UINT8 clk, UINT8 in_J, UINT8 in_K, UINT8 clrQ)
	{
		if (!clrQ)
			m_Q = 0;
		else if (m_lastclk && !clk)
		{
			if (!in_J && in_K)
				m_Q = 0;
			else if (in_J && !in_K)
				m_Q = 1;
			else if (in_J && in_K)
				m_Q = !m_Q;
		}
		m_lastclk = clk;
	}

	inline UINT8 Q() { return m_Q; }
	inline UINT8 QQ() { return !m_Q; }
private:
	UINT8 m_lastclk;
	UINT8 m_Q;
};

class ic74107 : public ic74107A
{
};

class ic7493
{
public:
	ic7493() { m_lastclk = 0; m_cnt = 0; }

	void process(UINT8 clk, UINT8 R1, UINT8 R2)
	{
		if (R1 && R2)
			m_cnt = 0;
		else if (m_lastclk && !clk)
		{
			m_cnt = ( m_cnt + 1) & 0x0f;
		}
		m_lastclk = clk;
	}

	inline UINT8 QA() { return (m_cnt >> 0) & 1; }
	inline UINT8 QB() { return (m_cnt >> 1) & 1; }
	inline UINT8 QC() { return (m_cnt >> 2) & 1; }
	inline UINT8 QD() { return (m_cnt >> 3) & 1; }

	inline UINT8 count() { return m_cnt; }
private:
	UINT8 m_lastclk;
	UINT8 m_cnt;
};

class ic7493_9 /* two chained 7493 + flipflop ==> 9 bit cnt */
{
public:
	ic7493_9() { m_lastclk = 0; m_cnt = 0; }

	void process(UINT8 clk, UINT8 R1)
	{
		if (R1)
			m_cnt = 0;
		else if (m_lastclk && !clk)
		{
			m_cnt = ( m_cnt + 1) & 0x1ff;
		}
		m_lastclk = clk;
	}

	inline UINT8 QA() { return (m_cnt >> 0) & 1; }
	inline UINT8 QB() { return (m_cnt >> 1) & 1; }
	inline UINT8 QC() { return (m_cnt >> 2) & 1; }
	inline UINT8 QD() { return (m_cnt >> 3) & 1; }
	inline UINT8 QE() { return (m_cnt >> 4) & 1; }
	inline UINT8 QF() { return (m_cnt >> 5) & 1; }
	inline UINT8 QG() { return (m_cnt >> 6) & 1; }
	inline UINT8 QH() { return (m_cnt >> 7) & 1; }
	inline UINT8 QI() { return (m_cnt >> 8) & 1; }

	inline UINT16 count() { return m_cnt; }

private:
	UINT8 m_lastclk;
	UINT16 m_cnt;
};
// ======================> pong_device

class pong_device : public device_t,
					  public device_execute_interface
{
public:
	// construction/destruction
	pong_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	bitmap_rgb32 *m_bitmap;
	int paddle0;
	int paddle1;
	UINT8 m_rst;

	dac_device *m_dac; /* just to have a sound device */

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	virtual void execute_run();

    int m_icount;

private:

    void step_one_clock();

    /* some gates */

    inline UINT8 g_not(UINT8 in)	{return in ^ 1; }
    inline UINT8 g_nand7(UINT8 in1, UINT8 in2, UINT8 in3, UINT8 in4, UINT8 in5, UINT8 in6, UINT8 in7) 	{ return (in1 && in2 && in3 && in4 && in5 && in6 && in7) ? 0 : 1; }
    inline UINT8 g_nand5(UINT8 in1, UINT8 in2, UINT8 in3, UINT8 in4, UINT8 in5) 	{ return (in1 && in2 && in3 && in4 && in5) ? 0 : 1; }
    inline UINT8 g_nand3(UINT8 in1, UINT8 in2, UINT8 in3) 	{ return (in1 && in2 && in3) ? 0 : 1; }
    inline UINT8 g_nand2(UINT8 in1, UINT8 in2) 	{ return (in1 && in2) ? 0 : 1; }
    inline UINT8 g_nor4(UINT8 in1, UINT8 in2, UINT8 in3, UINT8 in4) 	{ return (in1 || in2 || in3 || in4) ? 0 : 1; }
    inline UINT8 g_nor3(UINT8 in1, UINT8 in2, UINT8 in3) 	{ return (in1 || in2 || in3) ? 0 : 1; }
    inline UINT8 g_nor2(UINT8 in1, UINT8 in2) 				{ return (in1 || in2) ? 0 : 1; }

    inline UINT8 s_4H()			{ return ic_f8.QC(); }
    inline UINT8 s_8H()			{ return ic_f8.QD(); }
    inline UINT8 s_16H()		{ return ic_f8.QE(); }
    inline UINT8 s_32H()		{ return ic_f8.QF(); }
    inline UINT8 s_64H()		{ return ic_f8.QG(); }
    inline UINT8 s_128H()		{ return ic_f8.QH(); }
    inline UINT8 s_256H()		{ return ic_f8.QI(); }
    inline UINT8 sQ_256H()		{ return !s_256H(); }

    inline UINT8 s_4V()			{ return ic_e8.QC(); }
    inline UINT8 s_8V()			{ return ic_e8.QD(); }
    inline UINT8 s_16V()		{ return ic_e8.QE(); }
    inline UINT8 s_32V()		{ return ic_e8.QF(); }
    inline UINT8 s_64V()		{ return ic_e8.QG(); }
    inline UINT8 s_128V()		{ return ic_e8.QH(); }
    inline UINT8 sQ_256V()		{ return ic_e8.QI() ^ 1; }

    inline UINT8 s_hreset()		{ return ic_e7b.QQ(); }
    inline UINT8 s_vreset()		{ return ic_e7a.QQ(); }

    inline UINT8 sQ_vblank()	{ return ic_ff_vb.QQ(); }
    inline UINT8 s_vblank()		{ return ic_ff_vb.Q(); }
    inline UINT8 sQ_vsync()		{ return g_nand3(s_vblank(), s_4V(), g_not(s_8V())); }

    inline UINT8 s_hblank()		{ return (ic_ff_hb.Q()); }
    inline UINT8 sQ_hsync()		{ return g_nand2(s_hblank(),s_32H()); }

    /* paddle triggers monoflop duration which triggers a counter up to 16 on hsync */
    inline UINT8 sQ_vpad2()		{ return !(ic_e8.count() >= paddle1 && ic_e8.count() < paddle1 + 16); }
    inline UINT8 sQ_vpad1()		{ return !(ic_e8.count() >= paddle0 && ic_e8.count() < paddle0 + 16); }

    inline UINT8 sQ_hvid()		{ return m_hvid; }
    inline UINT8 sQ_vvid()		{ return m_vvid; }
    inline UINT8 s_vvid()		{ return !m_vvid; }
    inline UINT8 s_pad2()		{ return g_nor3(g_nand2(s_128H(), m_ff_h3a), sQ_vpad2(), sQ_256H()); }
    inline UINT8 s_pad1()		{ return g_nor3(g_nand2(s_128H(), m_ff_h3a), sQ_vpad1(), s_256H()); }
    inline UINT8 sQ_attract()	{ return g_nor2(s_StopG(), sQ_run()); }
    inline UINT8 s_attract()	{ return !sQ_attract(); }
    inline UINT8 s_net()		{ return g_nor3(g_nand2(m_jk_256, s_256H()),s_vblank(),s_4V()); }
    inline UINT8 s_video()		{ return g_not(g_nor4(g_nor2(sQ_hvid(), sQ_vvid()), s_pad1(), s_pad2(), s_net())); }
    inline UINT8 s_hit_sound_en() {return ic_c2a.QQ(); }
    inline UINT8 s_hit_sound()	{ return g_nand2(s_hit_sound_en(), s_vpos16()); }
    inline UINT8 s_score_sound(){ return g_nand2(s_SC(),s_vpos32()); }
    inline UINT8 s_topbothitsound() { return g_nand2(ic_f3_topbot.Q(), s_32V()); }

    inline UINT8 s_sound()	{ return g_nand2(sQ_attract(), g_nand3(s_topbothitsound(), s_hit_sound(), s_score_sound())); }

    inline UINT8 sQ_hit()		{ return !s_hit(); }
    inline UINT8 s_hit()		{ return g_nand2(sQ_hit1(), sQ_hit2()); }
    inline UINT8 sQ_hit1()		{ return g_nand2(s_pad1(), g_nor2(sQ_hvid(), sQ_vvid())); }
    inline UINT8 sQ_hit2()		{ return g_nand2(s_pad2(), g_nor2(sQ_hvid(), sQ_vvid())); }

    inline UINT8 sQ_Miss()		{ return g_nand2(s_hblank(), g_not(sQ_hvid())); }
    inline UINT8 s_Missed()		{ return g_nand2(g_not(sQ_Miss()), sQ_attract()); }


    inline UINT8 scoreDigit()
    {
    	if (!s_C5_EN())
    		return 15;
    	switch ((ic_f8.count() >> 5) & 3)  // 32H, 64H
    	{
    	case 0:
    		return (ic_c7_score1.count() >= 10) ? 1 : 15;
    	case 1:
    		return (ic_c7_score1.count() % 10);
    	case 2:
    		return (ic_d7_score2.count() >= 10) ? 1 : 15;
    	case 3:
    		return (ic_d7_score2.count() % 10);
    	}
    	return 0;
    }
    inline UINT8 s_scoreFE() { return g_nor3(g_not(s_16H()), s_4H(), s_8H()); }
    inline UINT8 s_scoreBC() { return g_nor2(g_not(s_16H()), g_nand2(s_4H(), s_8H())); }
    inline UINT8 s_scoreA() { return g_nor3(g_not(s_16H()), s_8V(), s_4V()); }
    inline UINT8 s_scoreGD() { return g_not(g_nand3(s_16H(), s_8V(), s_4V())); }

    inline UINT8 s_C5_EN()
    {
    	UINT8 tmp1;
    	tmp1 = g_nor2(g_nor3(s_256H(),s_64H(),g_not(s_128H())), g_not(g_nand3(s_256H(),s_64H(),g_not(s_128H()))) );
    	return g_nor4(tmp1, s_64V(), s_128V(), g_not(s_32V()));
    }
    inline UINT8 s_score()
    {
    	return g_nand7(
    			g_nand3(g_not(s_16V()), tab7448[scoreDigit()][5], s_scoreFE()),
    			g_nand3(       s_16V(), tab7448[scoreDigit()][4], s_scoreFE()),
    			g_nand3(g_not(s_16V()), tab7448[scoreDigit()][1], s_scoreBC()),
    			g_nand3(s_16V(), tab7448[scoreDigit()][2], s_scoreBC()),
    			g_nand3(g_not(s_16V()), tab7448[scoreDigit()][0], s_scoreA()),
    			g_nand3(g_not(s_16V()), tab7448[scoreDigit()][6], s_scoreGD()),
    			g_nand3(s_16V(), tab7448[scoreDigit()][3], s_scoreGD())
    			);
    }
    inline UINT8 s_Aa() { return g_nand2(s_Ba(),g_nand2(s_move(), ic_h3b.Q())); }
    inline UINT8 s_Ba() { return g_nand2(s_move(), ic_h3b.QQ()); }
    inline UINT8 s_Serve() { return ic_b5b_serve.QQ(); }
    inline UINT8 sQ_Serve() { return ic_b5b_serve.Q(); }
    inline UINT8 s_SC() { return ic_g4_sc.Q(); }       /* monoflop with NE555 determines score sound */
    inline UINT8 s_SRST() { return m_rst; }
    inline UINT8 sQ_SRST() { return !s_SRST(); }
    inline UINT8 sQ_run() { return 0; }
    inline UINT8 s_rstspeed() { return g_nand2(sQ_SRST(), sQ_Miss()); }
    inline UINT8 s_StopG() { return g_nand2(sQ_StopG1(), sQ_StopG2()); }
    inline UINT8 sQ_StopG1() { return (ic_c7_score1.count() != 15); } // FIXME later
    inline UINT8 sQ_StopG2() { return (ic_d7_score2.count() != 15); } // FIXME later
    inline UINT8 s_L() { return ic_h3b.Q(); }
    inline UINT8 s_R() { return ic_h3b.QQ(); }

    inline UINT8 s_vpos256() { return (m_vball_cnt & 0xf0) == 0xf0; }
    inline UINT8 s_vpos16() { return (m_vball_cnt & 0x10) == 0x10; }
    inline UINT8 s_vpos32() { return (m_vball_cnt & 0x20) == 0x20; }



    void step_hball_cnt()
    {
    	if (!s_hblank())
    	{
        	m_hball_cnt++;
        	if (m_hball_cnt > 0x1ff)
        		m_hball_cnt = 0x88 | (s_Ba() << 1) | s_Aa();
    	}
    	if (!g_nand2(s_Serve(),sQ_attract()))
    	{
    		m_hball_cnt = 0;
    	}
    	m_hvid = !((m_hball_cnt & 0x1fc) == 0x1fc);
    }

    void step_vball_cnt()
    {
    	UINT8 add;
    	/* get paddle ofsets */
    	if (m_last_hit && !s_hit())
    	{
    		m_p_latch = (((s_256H() ? ic_e8.count() - paddle1 : ic_e8.count() - paddle0) >> 1) & 0x07) ^ 0x07;
    	}
		m_last_hit = s_hit();

    	/* now step */
    	if (sQ_vblank())
    	{
        	if (ic_h2a_2.Q())
        	{
        		add = (m_p_latch ^ 0x07);
        	}
        	else
        	{
        		add = m_p_latch;
        	}

        	add += ( 0x06 + ((add >> 2) ^ 1));

    		if (m_last_hsync && !sQ_hsync())
    		{
    			m_vball_cnt++;
            	//printf("add %d\n", add);
    		}
    		m_last_hsync = sQ_hsync();
        	if (m_vball_cnt > 0xff)
        		m_vball_cnt = add;	/* A6 .. D6 */
    	}
    	m_vvid = !((m_vball_cnt & 0xfc) == 0xfc);

    }

    inline UINT8 s_move() { return g_nand2(ic_h2b.Q(), ic_h2a.Q()); }

    ic7474	ic_c2a;
    ic7493	ic_f1;
    ic74107 ic_h2a;
    ic74107 ic_h2b;
    ic7474	ic_h3b;

    /* horizontal counter */
    ic7493_9 ic_f8;	/* f8, f9, f6b */
    ic7474	ic_e7b;

    /* vertical counter */
    ic7493_9 ic_e8;	/* e8, e9, d9b */
    ic7474	ic_e7a;

    /* vblank flip flop */
    icRSFF	ic_ff_vb;	/* f5b, f5c */

    /* hblank flip flop */
    icRSFF	ic_ff_hb;	/* h5b, h5c */

    /* serve monoflog */
    icMonoFlop ic_f4_serve;
    ic7474 ic_b5b_serve;

    ic74107 ic_h2a_2;	/* two marked at position h2a */

    UINT8	m_jk_256;
    UINT8	m_ff_h3a;
    ic7493 	ic_c7_score1;		/* FIXME: realized by 74LS90 and 74LS107, both falling edge, using a 93 for now ... */
    ic7493 	ic_d7_score2;		/* FIXME: realized by 74LS90 and 74LS107, both falling edge, using a 93 for now ... */

    ic74107	ic_f3_topbot;
    icMonoFlop ic_g4_sc;

    UINT16  m_hball_cnt;	/* 2x 9316 and 74107 FF */
    UINT8	m_hvid;

    UINT16  m_vball_cnt;	/* 2x 9316  */

    UINT8	m_last_hsync;
    UINT8	m_vvid;
    UINT8	m_p_latch;
    UINT8	m_last_hit;
};


class pong_state : public driver_device
{
public:
	pong_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{  }

	// devices
	required_device<pong_device> m_maincpu;

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:

	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
};

// device type definition
const device_type PONG = &device_creator<pong_device>;

void pong_state::machine_start()
{
//	save_item(NAME(m_sound_nmi_enabled));
//	save_item(NAME(m_nmi_enable));
	dac_device *t = machine().device<dac_device>("dac");
	downcast<pong_device *>(m_maincpu.target())->m_dac = t;
}

void pong_state::machine_reset()
{
//	m_sound_nmi_enabled = FALSE;
//	m_nmi_enable = 0;
}


void pong_state::video_start()
{
	//FIXME: createtemporary bitmap
}


UINT32 pong_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	//draw_bullets(bitmap, cliprect);
	//draw_sprites(bitmap, cliprect);
	downcast<pong_device *>(m_maincpu.target())->paddle0 = ioport("PADDLE0")->read_safe(0);
	downcast<pong_device *>(m_maincpu.target())->paddle1 = ioport("PADDLE1")->read_safe(0);
	downcast<pong_device *>(m_maincpu.target())->m_rst = ioport("IN0")->read_safe(0) & 1;

	copybitmap(bitmap, *(downcast<pong_device *>(m_maincpu.target())->m_bitmap), 0, 0, 0, 0, cliprect);
	return 0;
}

pong_device::pong_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PONG, "PONG", tag, owner, clock),
	  device_execute_interface(mconfig, *this)
{
}


void pong_device::device_config_complete()
{

}
void pong_device::device_start()
{

	double dt = clocks_to_attotime(1).as_double();
	ic_f4_serve.m_clocks = MASTER_CLOCK;  // about a second ..., but FIXME
	ic_g4_sc.setPeriod555Std(RES_K(220), CAP_U(1), dt);

	m_p_latch = 5;

	m_bitmap = auto_bitmap_rgb32_alloc(machine(),H_TOTAL,V_TOTAL);

	// set our instruction counter
	m_icountptr = &m_icount;

	save_item(NAME(m_p_latch));

}
void pong_device::device_reset()
{

}
void pong_device::device_post_load()
{

}
void pong_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{

}

void pong_device::execute_run()
{
	//bool check_debugger = ((device_t::machine().debug_flags & DEBUG_FLAG_ENABLED) != 0);

	//printf("Burning %d cycles\n", m_icount);
	do
	{
		// debugging
		//m_ppc = m_pc;	// copy PC to previous PC
		//if (check_debugger)
		//	debugger_instruction_hook(this, 0); //m_pc);

		// instruction fetch
		//UINT16 op = opcode_read();

		step_one_clock();

		/* TODO: score and video are mixed by resistor network */
		if (s_video())
		{
			//printf("video %d %d\n", m_vcount, m_hcount);
			m_bitmap->pix(ic_e8.count(), ic_f8.count()) = MAKE_RGB(255,255,255);
		}
		else if (s_score())
			m_bitmap->pix(ic_e8.count(), ic_f8.count()) = MAKE_RGB(255,255,255);

		m_icount--;
	} while (m_icount > 0);
}

void pong_device::step_one_clock()
{
	UINT8 last_4H = s_4H();

	/* falling edge */
	m_jk_256 = sQ_256H();

	ic_f8.process(0,ic_e7b.QQ());
	ic_e7b.process(0, g_nand5(ic_f8.QB(),ic_f8.QC(), ic_f8.QG(), ic_f8.QH(), ic_f8.QI()),1,1);

	ic_e8.process(s_hreset(),ic_e7a.QQ());
	ic_e7a.process(s_hreset(), g_nand3(ic_e8.QA(),ic_e8.QC(), ic_e8.QI()),1,1);

	ic_f8.process(1,ic_e7b.QQ());
	ic_e7b.process(1, g_nand5(ic_f8.QB(),ic_f8.QC(), ic_f8.QG(), ic_f8.QH(), ic_f8.QI()),1,1);

	ic_e8.process(s_hreset(),ic_e7a.QQ());
	ic_e7a.process(s_hreset(), g_nand3(ic_e8.QA(),ic_e8.QC(), ic_e8.QI()),1,1);

	ic_ff_vb.process(s_vreset(), s_16V());
	ic_ff_hb.process(s_hreset(), g_not(g_nand2(s_16H(),s_64H())));

	//if (s_hblank() && ic_e8.count()==0)
	//	printf("hblank %d\n", ic_f8.count());

	{
		/* move logic */
		UINT8 clk = g_nor2(sQ_256H(), s_vreset());
		UINT8 tmp1 = g_nand2(ic_f1.QC(), ic_f1.QD());
		UINT8 tmp2 = g_not(g_nor2(ic_f1.QC(), ic_f1.QD()));

		ic_h2b.process(clk, 1, s_move(), g_nand2(s_vreset(), g_nand2(tmp1, tmp2)));
		ic_h2a.process(clk, ic_h2b.Q(), 0, g_nand2(s_vreset(), tmp2));

		ic_h3b.process(g_not(g_nand2(s_SC(), s_attract())), ic_h3b.QQ(), sQ_hit1(), sQ_hit2());

	}

	/* sound logic */
	ic_c2a.process(s_vpos256(), 1, sQ_hit(), 1);
	ic_f3_topbot.process(s_vblank(),s_vvid(),sQ_vvid(),sQ_Serve());
	ic_g4_sc.process(sQ_Miss());

	step_hball_cnt();
	step_vball_cnt();

	/* flipflop - vertical add */
	ic_h2a_2.process(s_vblank(),!sQ_vvid(),!sQ_vvid(),sQ_hit());

	ic_f1.process(g_nand2(s_hit_sound(), g_nand2(ic_f1.QC(), ic_f1.QD())),s_rstspeed(), s_rstspeed());

	if (ic_e8.count() == 16 && ic_f8.count()==0)
	{
		m_bitmap->fill(MAKE_RGB(0,0,0));
	}

	if (!sQ_attract())
		m_ff_h3a = 0;
	else
	{
		if (!last_4H && s_4H()) // raising edge for 7474
			m_ff_h3a = !s_128H();
	}

	ic_f4_serve.process(g_not(s_rstspeed()));
	UINT8 tmpx = g_nor3(ic_f4_serve.Q(), s_StopG(), sQ_run());
	ic_b5b_serve.process(s_pad1(), tmpx, tmpx, 1);

	/* Score logic ... */
	ic_c7_score1.process(g_nor2(s_L(), s_Missed()), s_SRST(), s_SRST());
	ic_d7_score2.process(g_nor2(s_R(), s_Missed()), s_SRST(), s_SRST());

	dac_w(m_dac, 0, 255*s_sound());
#if 0
	{
		static UINT8 lst_sound = 0;
		UINT8 ns = s_sound();
		if (ns != lst_sound)
			printf("sound toggle %d %d\n", ns,  s_SC()); // ic_f3_topbot.Q());
		lst_sound=ns;
	}

#endif
}



static INPUT_PORTS_START( pong )
	PORT_START( "PADDLE0" )	/* fake input port for player 1 paddle */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15)

	PORT_START( "PADDLE1" )	/* fake input port for player 2 paddle */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("IN0") /* fake as well */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_DIPNAME( 0x02, 0x00, "Game Won" )			PORT_DIPLOCATION("SW1B:4")
	PORT_DIPSETTING(    0x00, "11" )
	PORT_DIPSETTING(    0x02, "15" )

INPUT_PORTS_END

static MACHINE_CONFIG_START( pong, pong_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PONG, MASTER_CLOCK)

    /* video hardware */
    MCFG_SCREEN_ADD("screen", RASTER)
    MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK, H_TOTAL, HBEND, HBSTART, V_TOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(pong_state, screen_update)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("dac", DAC, 0)

	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	//MCFG_SPEAKER_STANDARD_MONO("mono")

	//MCFG_SOUND_ADD("aysnd", AY8910, 1789750)
	//MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pong ) /* dummy to satisfy game entry*/
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END


GAME( 1972, pong,  0, pong, pong,  0, ROT0, "Atari", "Pong (Rev E)", GAME_IMPERFECT_COLORS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
