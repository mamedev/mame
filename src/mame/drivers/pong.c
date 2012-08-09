/***************************************************************************

Pong (c) 1972 Atari

driver by Couriersud

Notes:

TODO:

============================================================================


***************************************************************************/


#include "emu.h"

#include "machine/rescap.h"
#include "machine/netlist.h"
#include "machine/net_lib.h"
#include "sound/dac.h"
#include "astring.h"

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

#define MASTER_CLOCK	7139000
#define V_TOTAL 		(0x105+1)
#define	H_TOTAL			(0x1C6+1)   	// 454

#define HBSTART					(H_TOTAL)
#define HBEND					(80)
#define VBSTART					(V_TOTAL)
#define VBEND					(16)

#define HRES_MULT					(2)

enum input_changed_enum
{
	IC_PADDLE1,
	IC_PADDLE2,
	IC_COIN,
	IC_SWITCH,
	IC_VR1,
	IC_VR2,
	IC_GATEDELAY
};

static NETLIST_START(pong_schematics)
	NETDEV_CONST(high, 1)
	NETDEV_CONST(low, 0)
	NETDEV_INPUT(clk)
	NETDEV_INPUT(SRST)

	TTL_7404_INVERT(hitQ, hit)
	TTL_7400_NAND(hit, hit1Q, hit2Q)

	TTL_7402_NOR(attractQ, StopG, runQ)
	TTL_7404_INVERT(attract, attractQ)

	TTL_7420_NAND(ic_h6a, hvidQ, hvidQ, hvidQ, hvidQ)
	NET_ALIAS(hvid, ic_h6a.Q)

	TTL_7400_NAND(ic_e6c, hvid, hblank)
	NET_ALIAS(MissQ, ic_e6c.Q)

	TTL_7404_INVERT(ic_d1e, MissQ)
	TTL_7400_NAND(ic_e1a, ic_d1e.Q, attractQ)
	NET_ALIAS(Missed, ic_e1a.Q)

	// runQ is basically the output of a RS flipflop
	// This is realized with discrete components in the real thing
	NETDEV_RSFF(runQ_ff, SRST, StopG)
	NET_ALIAS(runQ, runQ_ff.QQ)

	TTL_7400_NAND(rstspeed, SRSTQ, MissQ)
	TTL_7400_NAND(StopG, StopG1Q, StopG2Q)
	NET_ALIAS(L, ic_h3b.Q)
	NET_ALIAS(R, ic_h3b.QQ)

	TTL_7400_NAND(hit1Q, pad1, ic_g1b.Q)
	TTL_7400_NAND(hit2Q, pad2, ic_g1b.Q)

	TTL_7404_INVERT(SRSTQ, SRST)

	TTL_7400_NAND(ic_g3c, 128H, ic_h3a.QQ)
	TTL_7427_NOR(ic_g2c, ic_g3c.Q, 256H, vpad1Q)
	NET_ALIAS(pad1, ic_g2c.Q)
	TTL_7427_NOR(ic_g2a, ic_g3c.Q, 256HQ, vpad2Q)
	NET_ALIAS(pad2, ic_g2a.Q)

	/* horizontal counter */
	TTL_7493(ic_f8, clk, ic_e7b.QQ, ic_e7b.QQ)	/* f8, f9, f6b */
	TTL_7493(ic_f9, ic_f8.QD, ic_e7b.QQ, ic_e7b.QQ)	/* f8, f9, f6b */
	TTL_74107(ic_f6b, ic_f9.QD, high, high, ic_e7b.Q)
	TTL_7430_NAND(ic_f7, ic_f8.QB, ic_f8.QC, ic_f9.QC, ic_f9.QD, ic_f6b.Q, high, high, high)
	TTL_7474(ic_e7b, clk, ic_f7, high, high)

	NET_ALIAS(hreset, ic_e7b.QQ)
	NET_ALIAS(hresetQ, ic_e7b.Q)
	NET_ALIAS(  4H, ic_f8.QC)
	NET_ALIAS(  8H, ic_f8.QD)
	NET_ALIAS( 16H, ic_f9.QA)
	NET_ALIAS( 32H, ic_f9.QB)
	NET_ALIAS( 64H, ic_f9.QC)
	NET_ALIAS(128H, ic_f9.QD)
	NET_ALIAS(256H, ic_f6b.Q)
	NET_ALIAS(256HQ, ic_f6b.QQ)

	/* vertical counter */
	TTL_7493(ic_e8, hreset, ic_e7a.QQ, ic_e7a.QQ)	/* e8, e9, d9b */
	TTL_7493(ic_e9, ic_e8.QD, ic_e7a.QQ, ic_e7a.QQ)	/* e8, e9, d9b */
	TTL_74107(ic_d9b, ic_e9.QD, high, high, ic_e7a.Q)
	TTL_7474(ic_e7a, hreset, e7a_data, high, high)
	TTL_7410_NAND(e7a_data, ic_e8.QA, ic_e8.QC, ic_d9b.Q)

	NET_ALIAS(vreset, ic_e7a.QQ)
	NET_ALIAS(  4V, ic_e8.QC)
	NET_ALIAS(  8V, ic_e8.QD)
	NET_ALIAS( 16V, ic_e9.QA)
	NET_ALIAS( 32V, ic_e9.QB)
	NET_ALIAS( 64V, ic_e9.QC)
	NET_ALIAS(128V, ic_e9.QD)
	NET_ALIAS(256V,  ic_d9b.Q)
	NET_ALIAS(256VQ, ic_d9b.QQ)


	/* hblank flip flop */

	TTL_7400_NAND(ic_g5b, 16H, 64H)

	/* the time critical one */
	TTL_7400_NAND(ic_h5c, ic_h5b.Q, hresetQ)
	TTL_7400_NAND(ic_h5b, ic_h5c.Q, ic_g5b.Q)

	NET_ALIAS(hblank,  ic_h5c.Q)
	NET_ALIAS(hblankQ,  ic_h5b.Q)
	TTL_7400_NAND(hsyncQ, hblank, 32H)

	/* vblank flip flop */
	TTL_7402_NOR(ic_f5c, ic_f5d.Q, vreset)
	TTL_7402_NOR(ic_f5d, ic_f5c.Q, 16V)

	NET_ALIAS(vblank,  ic_f5d.Q)
	NET_ALIAS(vblankQ, ic_f5c.Q)

	TTL_7400_NAND(ic_h5a, 8V, 8V)
	TTL_7410_NAND(ic_g5a, vblank, 4V, ic_h5a.Q)
	NET_ALIAS(vsyncQ, ic_g5a.Q)

	/* move logic */

	TTL_7400_NAND(ic_e1d, hit_sound, ic_e1c.Q)
	TTL_7400_NAND(ic_e1c, ic_f1.QC, ic_f1.QD)
	TTL_7493(ic_f1, ic_e1d.Q, rstspeed, rstspeed)

	TTL_7402_NOR(ic_g1d, ic_f1.QC, ic_f1.QD)
	TTL_7400_NAND(ic_h1a, ic_g1d.Q, ic_g1d.Q)
	TTL_7400_NAND(ic_h1d, ic_e1c.Q, ic_h1a.Q)

	TTL_7400_NAND(ic_h1c, ic_h1d.Q, vreset)
	TTL_7400_NAND(ic_h1b, ic_h1a.Q, vreset)
	TTL_7402_NOR(ic_g1c, 256HQ, vreset)

	TTL_74107(ic_h2a, ic_g1c.Q, ic_h2b.Q, low, ic_h1b.Q)
	TTL_74107(ic_h2b, ic_g1c.Q, high, move, ic_h1c.Q)
	TTL_7400_NAND(ic_h4a, ic_h2b.Q, ic_h2a.Q)
	NET_ALIAS(move, ic_h4a.Q)

	TTL_7400_NAND(ic_c1d, SC, attract)
	TTL_7404_INVERT(ic_d1a, ic_c1d.Q)
	TTL_7474(ic_h3b, ic_d1a.Q, ic_h3b.QQ, hit1Q, hit2Q)

	TTL_7400_NAND(ic_h4d, ic_h3b.Q, move)
	TTL_7400_NAND(ic_h4b, ic_h3b.QQ, move)
	TTL_7400_NAND(ic_h4c, ic_h4d.Q, ic_h4b.Q)
	NET_ALIAS(Aa, ic_h4c.Q)
	NET_ALIAS(Ba, ic_h4b.Q)

	/* hvid circuit */

	TTL_7400_NAND(hball_resetQ, Serve, attractQ)

	TTL_9316(ic_g7, clk, high, hblankQ, hball_resetQ, ic_g5c.Q, Aa, Ba, low, high)
	TTL_9316(ic_h7, clk, ic_g7.RC, high, hball_resetQ, ic_g5c.Q, low, low, low, high)
	TTL_74107(ic_g6b, ic_h7.RC, high, high, hball_resetQ)
	TTL_7410_NAND(ic_g5c, ic_g6b.Q, ic_h7.RC, ic_g7.RC)
	TTL_7420_NAND(ic_h6b, ic_g6b.Q, ic_h7.RC, ic_g7.QC, ic_g7.QD)
	NET_ALIAS(hvidQ, ic_h6b.Q)

	/* vvid circuit */

	TTL_9316(ic_b3, hsyncQ, high, vblankQ, high, ic_b2b.Q, a6, b6, c6, d6)
	TTL_9316(ic_a3, hsyncQ, ic_b3.RC, high, high, ic_b2b.Q, low, low, low, low)
	TTL_7400_NAND(ic_b2b, ic_a3.RC, ic_b3.RC)
	TTL_7410_NAND(ic_e2b, ic_a3.RC, ic_b3.QC, ic_b3.QD)
	NET_ALIAS(vvidQ, ic_e2b.Q)
	TTL_7404_INVERT(vvid, vvidQ)	/* D2D */
	NET_ALIAS(vpos256, ic_a3.RC)
	NET_ALIAS(vpos32, ic_a3.QB)
	NET_ALIAS(vpos16, ic_a3.QA)

	/* vball ctrl circuit */

	TTL_7450_ANDORINVERT(ic_a6a, b1, 256HQ, b2, 256H)
	TTL_7450_ANDORINVERT(ic_a6b, c1, 256HQ, c2, 256H)
	TTL_7450_ANDORINVERT(ic_b6b, d1, 256HQ, d2, 256H)

	TTL_7474(ic_a5b, hit, ic_a6a, attractQ, high)
	TTL_7474(ic_a5a, hit, ic_a6b, attractQ, high)
	TTL_7474(ic_b5a, hit, ic_b6b, attractQ, high)
	TTL_74107(ic_h2x, vblank, vvid, vvid, hitQ)	/* two marked at position h2a ==> this h2x */

	TTL_7486_XOR(ic_a4c, ic_a5b.Q, ic_h2x.Q)
	TTL_7486_XOR(ic_a4b, ic_a5a.Q, ic_h2x.Q)

	TTL_7450_ANDORINVERT(ic_b6a, ic_b5a.Q, ic_h2x.Q, ic_b5a.QQ, ic_h2x.QQ)

	TTL_7404_INVERT(ic_c4a, ic_b6a)

	TTL_7483(ic_b4, ic_a4c, ic_a4b, ic_b6a, low, ic_c4a, high, high, low, low)
	NET_ALIAS(a6, ic_b4.SA)
	NET_ALIAS(b6, ic_b4.SB)
	NET_ALIAS(c6, ic_b4.SC)
	NET_ALIAS(d6, ic_b4.SD)

	/* serve monoflop */
	TTL_7404_INVERT(f4_trig, rstspeed)
	NE555N_MSTABLE(ic_f4_serve, f4_trig)
	NETDEV_PARAM(ic_f4_serve.R, RES_K(330))
	NETDEV_PARAM(ic_f4_serve.C, CAP_U(4.7))

	TTL_7427_NOR(ic_e5a, ic_f4_serve.Q, StopG, runQ)
	TTL_7474(ic_b5b_serve, pad1, ic_e5a, ic_e5a, high)

	NET_ALIAS(Serve, ic_b5b_serve.QQ)
	NET_ALIAS(ServeQ, ic_b5b_serve.Q)

	/* score logic */

	TTL_7474(ic_h3a, 4H, 128H, high, attractQ)

	/* sound logic */
	TTL_7474(ic_c2a, vpos256, high, hitQ, high)
	TTL_74107(ic_f3_topbot, vblank, vvid, vvidQ, ServeQ)
	NE555N_MSTABLE(ic_g4_sc, MissQ)
	NET_ALIAS(SC, ic_g4_sc.Q)       /* monoflop with NE555 determines score sound */
	NETDEV_PARAM(ic_g4_sc.R, RES_K(220))
	NETDEV_PARAM(ic_g4_sc.C, CAP_U(1))

	NET_ALIAS(hit_sound_en, ic_c2a.QQ)
	TTL_7400_NAND(hit_sound, hit_sound_en, vpos16)
	TTL_7400_NAND(score_sound, SC, vpos32)
	TTL_7400_NAND(topbothitsound, ic_f3_topbot.Q, vpos32)

	TTL_7410_NAND(ic_c4b, topbothitsound, hit_sound, score_sound)
	TTL_7400_NAND(ic_c1b, ic_c4b.Q, attractQ)
	NET_ALIAS(sound, ic_c1b.Q)


	/* paddle1 logic 1  */

	NE555N_MSTABLE(ic_b9, 256VQ)
	NETDEV_PARAM(ic_b9.R, RES_K(90))
	NETDEV_PARAM(ic_b9.C, CAP_U(.1))
	NETDEV_PARAM(ic_b9.VL, 0.7)
	TTL_7404_INVERT(ic_c9b, ic_b9.Q)
	TTL_7400_NAND(ic_b7b, ic_a7b.Q, hsyncQ)
	TTL_7493(ic_b8, ic_b7b.Q, ic_b9.Q, ic_b9.Q)
	TTL_7400_NAND(ic_b7a, ic_c9b.Q, ic_a7b.Q)
	TTL_7420_NAND(ic_a7b, ic_b8.QA, ic_b8.QB, ic_b8.QC, ic_b8.QD)
	NET_ALIAS(vpad1Q, ic_b7a.Q)

	NET_ALIAS(b1, ic_b8.QB)
	NET_ALIAS(c1, ic_b8.QC)
	NET_ALIAS(d1, ic_b8.QD)

	/* paddle1 logic 2 */

	NE555N_MSTABLE(ic_a9, 256VQ)
	NETDEV_PARAM(ic_a9.R, RES_K(90))
	NETDEV_PARAM(ic_a9.C, CAP_U(.1))
	NETDEV_PARAM(ic_a9.VL, 0.7)
	TTL_7404_INVERT(ic_c9a, ic_a9.Q)
	TTL_7400_NAND(ic_b7c, ic_a7a.Q, hsyncQ)
	TTL_7493(ic_a8, ic_b7c.Q, ic_a9.Q, ic_a9.Q)
	TTL_7400_NAND(ic_b7d, ic_c9a.Q, ic_a7a.Q)
	TTL_7420_NAND(ic_a7a, ic_a8.QA, ic_a8.QB, ic_a8.QC, ic_a8.QD)
	NET_ALIAS(vpad2Q, ic_b7d.Q)

	NET_ALIAS(b2, ic_a8.QB)
	NET_ALIAS(c2, ic_a8.QC)
	NET_ALIAS(d2, ic_a8.QD)

	/* C5-EN Logic */

	TTL_7404_INVERT(ic_e3a, 128H)
	TTL_7427_NOR( ic_e3b, 256H, 64H, ic_e3a.Q)
	TTL_7410_NAND(ic_e2c, 256H, 64H, ic_e3a.Q)
	TTL_7404_INVERT(ic_e3c, ic_e2c.Q)
	TTL_7402_NOR(ic_d2c, ic_e3c.Q, ic_e3b.Q)
	TTL_7404_INVERT(ic_g1a, 32V)
	TTL_7425_NOR(ic_f2a, ic_g1a.Q, 64V, 128V, ic_d2c.Q)
	NET_ALIAS(c5-en, ic_f2a.Q)

	/* Score logic ... */

	TTL_7402_NOR(ic_f5b, L, Missed)
	TTL_7490(ic_c7, ic_f5b, SRST, SRST, low, low)
	TTL_74107(ic_c8a, ic_c7.QD, high, high, SRSTQ)
	NETDEV_SWITCH2(sw1a, high, ic_c7.QC)
	NETDEV_PARAM(sw1a.POS, 0)
	TTL_7410_NAND(ic_d8a, ic_c7.QA, sw1a.Q, ic_c8a.Q)		/* would be nand2 for 11 instead of 15 points, need a switch dev! */

	NET_ALIAS(StopG1Q, ic_d8a.Q)
	NET_ALIAS(score1_1, ic_c7.QA)
	NET_ALIAS(score1_2, ic_c7.QB)
	NET_ALIAS(score1_4, ic_c7.QC)
	NET_ALIAS(score1_8, ic_c7.QD)
	NET_ALIAS(score1_10, ic_c8a.Q)
	NET_ALIAS(score1_10Q, ic_c8a.QQ)

	TTL_7402_NOR(ic_f5a, R, Missed)
	TTL_7490(ic_d7, ic_f5a, SRST, SRST, low, low)
	TTL_74107(ic_c8b, ic_d7.QD, high, high, SRSTQ)
	NETDEV_SWITCH2(sw1b, high, ic_d7.QC)
	NETDEV_PARAM(sw1b.POS, 0)
	TTL_7410_NAND(ic_d8b, ic_d7.QA, sw1b.Q, ic_c8b.Q)		/* would be nand2 for 11 instead of 15 points, need a switch dev! */

	NET_ALIAS(StopG2Q, ic_d8b.Q)
	NET_ALIAS(score2_1, ic_d7.QA)
	NET_ALIAS(score2_2, ic_d7.QB)
	NET_ALIAS(score2_4, ic_d7.QC)
	NET_ALIAS(score2_8, ic_d7.QD)
	NET_ALIAS(score2_10, ic_c8b.Q)
	NET_ALIAS(score2_10Q, ic_c8b.QQ)

	/* Score display */

	TTL_74153(ic_d6a, score1_10Q, score1_4, score2_10Q, score2_4, 32H, 64H, low)
	TTL_74153(ic_d6b, score1_10Q, score1_8, score2_10Q, score2_8, 32H, 64H, low)

	TTL_74153(ic_c6a, high, score1_1, high, score2_1, 32H, 64H, low)
	TTL_74153(ic_c6b, score1_10Q, score1_2, score2_10Q, score2_2, 32H, 64H, low)

	TTL_7448(ic_c5, ic_c6a.AY, ic_c6b.AY, ic_d6a.AY, ic_d6b.AY, high, c5-en, high)

	TTL_7404_INVERT(ic_e4b, 16H)
	TTL_7427_NOR(ic_e5c, ic_e4b.Q, 8H, 4H)
	NET_ALIAS(scoreFE, ic_e5c.Q)

	TTL_7400_NAND(ic_c3d, 4H, 8H)
	TTL_7402_NOR(ic_d2b, ic_e4b.Q, ic_c3d.Q)
	NET_ALIAS(scoreBC, ic_d2b.Q)

	TTL_7427_NOR(ic_e5b, ic_e4b.Q, 8V, 4V)
	NET_ALIAS(scoreA, ic_e5b.Q)

	TTL_7410_NAND(ic_e2a, 16H, 8V, 4V)
	TTL_7404_INVERT(ic_e4a, ic_e2a.Q)
	NET_ALIAS(scoreGD, ic_e4a.Q)

	TTL_7404_INVERT(ic_e4c, 16V)

	TTL_7410_NAND(ic_d4a, ic_e4c.Q, ic_c5.f, scoreFE)
	TTL_7410_NAND(ic_d5c,      16V, ic_c5.e, scoreFE)
	TTL_7410_NAND(ic_c4c, ic_e4c.Q, ic_c5.b, scoreBC)
	TTL_7410_NAND(ic_d5a,      16V, ic_c5.c, scoreBC)
	TTL_7410_NAND(ic_d4c, ic_e4c.Q, ic_c5.a, scoreA)
	TTL_7410_NAND(ic_d4b, ic_e4c.Q, ic_c5.g, scoreGD)
	TTL_7410_NAND(ic_d5b,      16V, ic_c5.d, scoreGD)

	TTL_7430_NAND(ic_d3, ic_d4a, ic_d5c, ic_c4c, ic_d5a, ic_d4c, ic_d4b, ic_d5b, high)
	NET_ALIAS(score, ic_d3.Q)		//FIXME

	/* net */
	TTL_74107(ic_f3b, clk, 256H, 256HQ, high)
	TTL_7400_NAND(ic_g3b, ic_f3b.QQ, 256H)
	TTL_7427_NOR(ic_g2b, ic_g3b.Q, vblank, 4V)
	NET_ALIAS(net, ic_g2b.Q)

	/* video */
	TTL_7402_NOR(ic_g1b, hvidQ, vvidQ)
	TTL_7425_NOR(ic_f2b, ic_g1b.Q, pad1, pad2, net)
	TTL_7404_INVERT(ic_e4e, ic_f2b.Q)
	NET_ALIAS(video, ic_e4e.Q)

NETLIST_END

static NETLIST_START(pong)

	//NETLIST_INCLUDE(pong_schematics)
	NETLIST_MEMREGION("maincpu")
	NETDEV_CALLBACK(sound_cb, sound)
	NETDEV_CALLBACK(video_cb, video)
	NETDEV_CALLBACK(score_cb, score)

NETLIST_END

static NETLIST_START(pong_fast)

	NETLIST_INCLUDE(pong_schematics)
	/* the signal above is delayed on pong due to counter at gate delays.
     * This is approximated by the following circuit ...
     */
	NET_REMOVE_DEV(ic_h5b)
	NETDEV_DELAY_RISE(ic_g5b_D, clk, ic_g5b.Q)
	TTL_7400_NAND(ic_h5b, ic_h5c.Q, ic_g5b_D.Q)

	NETDEV_CALLBACK(sound_cb, sound)
	NETDEV_CALLBACK(video_cb, video)
	NETDEV_CALLBACK(score_cb, score)

NETLIST_END

class pong_state : public driver_device
{
public:
	pong_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_dac(*this, "dac"),				/* just to have a sound device */
		  m_srst(*this, "maincpu", "SRST"),
		  m_sw1a(*this, "maincpu", "sw1a.POS"),
		  m_sw1b(*this, "maincpu", "sw1b.POS"),
		  m_p_V0(*this, "maincpu", "ic_a9.VT"),
		  m_p_V1(*this, "maincpu", "ic_b9.VT"),
		  m_p_R0(*this, "maincpu", "ic_a9.R"),
		  m_p_R1(*this, "maincpu", "ic_b9.R")
	{
	}

	// devices
	required_device<netlist_mame_device> m_maincpu;
	required_device<dac_device> m_dac; /* just to have a sound device */

	// sub devices
	netlist_mame_device::required_output m_srst;
	netlist_mame_device::required_param m_sw1a;
	netlist_mame_device::required_param m_sw1b;
	netlist_mame_device::required_param m_p_V0;
	netlist_mame_device::required_param m_p_V1;
	netlist_mame_device::required_param m_p_R0;
	netlist_mame_device::required_param m_p_R1;

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed);

	void sound_cb(net_sig_t newval)
	{
		m_dac->write_unsigned8(128*(!newval));
	}

	void video_cb(net_sig_t newval)
	{
		update_vid();
		m_vid = (m_vid & 2) | (UINT8)  newval;
	}

	void score_cb(net_sig_t newval)
	{
		update_vid();
		m_vid = (m_vid & 1) | ((UINT8) newval<<1);
	}

protected:

	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();

private:

	void update_vid()
	{
		UINT64 clocks = m_maincpu->total_cycles(); // m_maincpu->attotime_to_cycles(m_maincpu->local_time());
		int new_x = clocks % ( m_maincpu->SubCycles() * H_TOTAL);
		int new_y = (clocks / (m_maincpu->SubCycles() * H_TOTAL)) % V_TOTAL;
		if (m_vid > 0)
		{
			rgb_t col = MAKE_RGB(255,255,255);
			if (new_y > m_last_y)
				new_x =  m_maincpu->SubCycles() * H_TOTAL-1;
			int delta = (new_x - m_last_x);
			if (delta <  m_maincpu->SubCycles())
				col = MAKE_RGB(255 * delta /( m_maincpu->SubCycles()),255 * delta /( m_maincpu->SubCycles()),255 * delta /( m_maincpu->SubCycles()));
			for (int i = m_last_x / ( m_maincpu->SubCycles() / HRES_MULT); i < new_x / ( m_maincpu->SubCycles()/HRES_MULT); i++)
				m_bitmap->pix(m_last_y, i) = col;
		}
		m_last_x = new_x;
		m_last_y = new_y;
	}

	UINT8 m_vid;
	int m_last_x;
	int m_last_y;
	bitmap_rgb32 *m_bitmap;

};

void pong_state::machine_start()
{

	m_bitmap = auto_bitmap_rgb32_alloc(machine(),H_TOTAL * HRES_MULT,V_TOTAL);

	m_maincpu->setup().register_callback("sound_cb", net_output_delegate(&pong_state::sound_cb, "pong_state::sound_cb", this));
	m_maincpu->setup().register_callback("video_cb", net_output_delegate(&pong_state::video_cb, "pong_state::video_cb", this));
	m_maincpu->setup().register_callback("score_cb", net_output_delegate(&pong_state::score_cb, "pong_state::score_cb", this));
}

void pong_state::machine_reset()
{
//  m_sound_nmi_enabled = FALSE;
//  m_nmi_enable = 0;
}


void pong_state::video_start()
{
	//FIXME: createtemporary bitmap
}


UINT32 pong_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	copybitmap(bitmap, *m_bitmap, 0, 0, 0, 0, cliprect);
	m_bitmap->fill(MAKE_RGB(0,0,0));
	return 0;
}


INPUT_CHANGED_MEMBER(pong_state::input_changed)
{

	static const double NE555_R = RES_K(5);
	static const double PRE_R = RES_R(470);

	double pad;
	int numpad = (FPTR) (param);

	switch (numpad)
	{
	case IC_PADDLE1:
	case IC_PADDLE2:
	{
		// http://ion.chem.usu.edu/~sbialkow/Classes/564/Thevenin/Thevenin.html

		double fac = (double) newval / (double) 256;
		double R1 = fac * RES_K(10);
		double R3 = (1.0 - fac) * RES_K(10);
		double vA = 5.0 * R3 / (R3 + R1);
		double vB = 5.0 * 2 * NE555_R / (2 * NE555_R + NE555_R);
		double Req = RES_2_PARALLEL(R1, R3) + RES_2_PARALLEL(NE555_R, 2.0 * NE555_R);
		double pad = vA + (vB - vA)*PRE_R / (Req + PRE_R);
		switch (numpad)
		{
		case IC_PADDLE1:	m_p_V0->setTo(pad); break;
		case IC_PADDLE2:	m_p_V1->setTo(pad); break;
		}
		break;
	}
	case IC_SWITCH:
		m_sw1a->setTo(newval ? 1 : 0);
		m_sw1b->setTo(newval ? 1 : 0);
		break;
	case IC_COIN:
		m_srst->setTo(newval & 1);
		break;
	case IC_VR1:
	case IC_VR2:
		pad = (double) newval / (double) 100 * RES_K(50) + RES_K(56);
		switch (numpad)
		{
		case IC_VR1:	m_p_R0->setTo(pad); break;
		case IC_VR2:	m_p_R1->setTo(pad); break;
		}
		break;
	case IC_GATEDELAY:
		m_maincpu->netlist().set_gatedelay(newval);
		break;
	}


}


static INPUT_PORTS_START( pong )
	PORT_START( "PADDLE0" )	/* fake input port for player 1 paddle */
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(2) PORT_KEYDELTA(100) PORT_CENTERDELTA(0)	PORT_CHANGED_MEMBER(DEVICE_SELF, pong_state, input_changed,IC_PADDLE1)

	PORT_START( "PADDLE1" )	/* fake input port for player 2 paddle */
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(2) PORT_KEYDELTA(100) PORT_CENTERDELTA(0) PORT_PLAYER(2) PORT_CHANGED_MEMBER(DEVICE_SELF, pong_state, input_changed, IC_PADDLE2)

	PORT_START("IN0") /* fake as well */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )		PORT_CHANGED_MEMBER(DEVICE_SELF, pong_state, input_changed, IC_COIN)
	PORT_DIPNAME( 0x06, 0x00, "Game Won" )			PORT_DIPLOCATION("SW1A:1,SW1B:1") PORT_CHANGED_MEMBER(DEVICE_SELF, pong_state, input_changed, IC_SWITCH)
	PORT_DIPSETTING(    0x00, "11" )
	PORT_DIPSETTING(    0x06, "15" )

	PORT_START("VR1")
	PORT_ADJUSTER( 63, "VR1 - 50k, Paddle 1 adjustment" )	PORT_CHANGED_MEMBER(DEVICE_SELF, pong_state, input_changed, IC_VR1)
	PORT_START("VR2")
	PORT_ADJUSTER( 63, "VR2 - 50k, Paddle 2 adjustment" )	PORT_CHANGED_MEMBER(DEVICE_SELF, pong_state, input_changed, IC_VR2)
	PORT_START("GATESPEED")
	PORT_ADJUSTER( 100, "Logic Gate Delay" ) PORT_MINMAX(10, 200)	PORT_CHANGED_MEMBER(DEVICE_SELF, pong_state, input_changed, IC_GATEDELAY)

INPUT_PORTS_END

static MACHINE_CONFIG_START( pong, pong_state )

	/* basic machine hardware */
	MCFG_NETLIST_ADD("maincpu", MASTER_CLOCK, pong, 10)

    /* video hardware */
    MCFG_SCREEN_ADD("screen", RASTER)
    MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK * HRES_MULT, H_TOTAL * HRES_MULT, HBEND * HRES_MULT, HBSTART * HRES_MULT, V_TOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(pong_state, screen_update)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 48000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pongf, pong )

	/* basic machine hardware */
	MCFG_NETLIST_REPLACE("maincpu", MASTER_CLOCK, pong_fast, 2)

MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pong ) /* dummy to satisfy game entry*/
	ROM_REGION( 0x10000, "maincpu", 0 )	/* enough for netlist */
	ROM_LOAD( "pong.netlist", 0x00000, 10306, CRC(bb92b267) SHA1(0dd6b3209ac1335a97cfe159502d24556f531007) )
ROM_END

ROM_START( pongf ) /* dummy to satisfy game entry*/
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END


GAME( 1972, pong,  0, pong, pong, pong_state,  0, ROT0, "Atari", "Pong (Rev E)", 0 )
GAME( 1972, pongf,  0, pongf, pong, pong_state,  0, ROT0, "Atari", "Pong (Rev E), no subcycles", 0 )
