// license:GPL-2.0+
// copyright-holders:Couriersud
#define NETLIST_DEVELOPMENT 1

#include "netlist/nl_dice_compat.h"
#include "netlist/devices/net_lib.h"
#include "netlist/analog/nld_twoterm.h"

//2 555 timers
static Astable555Desc b2_555_desc(OHM(560.0), M_OHM(1.8), U_FARAD(0.1));

static Mono555Desc c9_555_desc(OHM(47.0), U_FARAD(1.0));	// R33, C21

//check these values
//static Paddle1HorizontalDesc pad1_desc(15047.0, 47.0, &c9_555_desc);

static CapacitorDesc c32_desc(U_FARAD(0.1));
static CapacitorDesc c36_desc(N_FARAD(1.0));	//0.001uF = 1nF
static CapacitorDesc c37_desc(P_FARAD(330.0));
static BufferDesc pad_en_buf_desc(DELAY_NS(15.0), DELAY_NS(15.0)); // Prevents 12ns glitch on PAD_EN_n signal from resetting C9. TODO: is this accurate?

#if 0
static VIDEO_DESC( breakout )
    VIDEO_RESISTANCE(1, K_OHM(0.0))
    VIDEO_RESISTANCE(2, K_OHM(3.9))
    VIDEO_RESISTANCE(3, K_OHM(3.9))
    VIDEO_RESISTANCE(4, K_OHM(3.9))
    VIDEO_RESISTANCE(5, K_OHM(3.9))
    VIDEO_RESISTANCE(6, K_OHM(3.9))
    //VIDEO_RESISTANCE(Video::HBLANK_PIN, K_OHM(0.0))
    VIDEO_ORIENTATION(ROTATE_90)
    VIDEO_CONTRAST(4.0)

    // Values guessed based on screenshots. TODO: Make more accurate
    //             X,         Y,   W,        H,    R,    G,    B
    VIDEO_OVERLAY( 15.570e-6, 0.0, 1.911e-6, -1.0, 0.80, 0.15, 0.05 ) // Red Bricks
    VIDEO_OVERLAY( 17.481e-6, 0.0, 1.956e-6, -1.0, 0.95, 0.65, 0.05 ) // Amber Bricks
    VIDEO_OVERLAY( 19.437e-6, 0.0, 1.956e-6, -1.0, 0.05, 0.65, 0.25 ) // Green Bricks
    VIDEO_OVERLAY( 21.393e-6, 0.0, 1.955e-6, -1.0, 0.95, 0.95, 0.20 ) // Yellow Bricks
    VIDEO_OVERLAY( 51.345e-6, 0.0, 1.956e-6, -1.0, 0.05, 0.65, 0.95 ) // Blue Paddle

    // TODO: Different overlays for cocktail cabinet
VIDEO_DESC_END

static AUDIO_DESC( breakout )
    AUDIO_RESISTANCE(1, K_OHM(47.0))
    AUDIO_RESISTANCE(2, K_OHM(47.0))
    AUDIO_RESISTANCE(3, K_OHM(47.0))
    AUDIO_RESISTANCE(4, K_OHM(47.0))
    AUDIO_GAIN(3.0)
VIDEO_DESC_END
#endif

static Mono9602Desc n8_desc(K_OHM(33.0), U_FARAD(100.0), K_OHM(5.6), P_FARAD(0.01)); // No capacitor on 2nd 9602, assume very low internal capacitance
static Mono9602Desc f3_desc(K_OHM(47.0), U_FARAD(1.0), K_OHM(47.0), U_FARAD(1.0));

static Mono9602Desc a7_desc(K_OHM(68.0), U_FARAD(1.0), K_OHM(22.0), U_FARAD(10.0));
static Mono9602Desc a8_desc(K_OHM(27.0), U_FARAD(1.0), K_OHM(27.0), U_FARAD(1.0));

static Dipswitch53137Desc dipswitch1_desc("bonus_credit", "Bonus Credit", 3, "None", "100", "200", "300", "400", "500", "600", "700", "800");
//static DipswitchDesc dipswitch2_desc("cabinet_type", "Cabinet Type", 0, "Normal", "Cocktail");
//static DipswitchDesc dipswitch3_desc("coinage", "Coinage", 0, "1 Coin / 1 Credit", "1 Coin / 2 Credits");
//static DipswitchDesc dipswitch4_desc("ball_count", "Ball Count", 0, "3", "5");

CIRCUIT_LAYOUT( breakout )
  	CHIP("S1", 53137, &dipswitch1_desc)
	CHIP("S2", DIPSWITCH, &dipswitch2_desc)
	CHIP("S3", DIPSWITCH, &dipswitch3_desc)
	CHIP("S4", DIPSWITCH, &dipswitch4_desc)

    SOLVER(Solver, 48000)
    PARAM(Solver.ACCURACY, 1e-7) // works and is sufficient
    //CHIP("CLOCK", CLOCK_14_318_MHZ)
    MAINCLOCK(Y1, 14318000.0)

    ANALOG_INPUT(V5, 5)
#define VCC "V5", Q
#define GND "GND", Q

    //CHIP("Y1", CLOCK_14_318_MHZ)	//Y1

	CHIP_CAPACITOR(C32, &c32_desc)
	CHIP_CAPACITOR(C36, &c36_desc)
 	CHIP_CAPACITOR(C37, &c37_desc)
 	CHIP("PAD_EN_BUF", BUFFER, &pad_en_buf_desc)

	CHIP("A3", 7474)
	CHIP("A4", 7408)
	CHIP("A5", 7400)
	CHIP("A6", 7474)
	CHIP_9602_Mono(A7, &a7_desc)
	CHIP_9602_Mono(A8, &a8_desc)

    CHIP_555_Astable(B2, &b2_555_desc)
	CHIP("B3", 7402)
	CHIP("B4", 9316)
	CHIP("B5", 74193)
	CHIP("B6", 7400)
	CHIP("B7", 9316)
	CHIP("B8", 9316)
	CHIP("B9", 7408)

	CHIP("C2", 7400)
	CHIP("C3", 7400)
	CHIP("C4", 7486)
	CHIP("C5", 7404)
	CHIP("C6", 7486)
	CHIP("C7", 9316)
	CHIP("C8", 9316)
    CHIP_555_Mono(C9, &c9_555_desc)

	CHIP("D2", 7432)
	CHIP("D3", 7474)
	CHIP("D4", 9316)
	CHIP("D5", 7474)
	CHIP("D6", 7408)
	CHIP("D7", 7411)
	CHIP("D8", 7400)
//	CHIP("D9", 4016)	//quad bilateral switch

	CHIP("E1", 7404)
	CHIP("E2", 7486)
	CHIP("E3", 7402)
	CHIP("E4", 7432)
	CHIP("E5", 7408)
	CHIP("E6", 7474)
	CHIP("E7", 7402)
	CHIP("E8", 7474)
	CHIP("E9", 7404)

	CHIP("F1", 9316)
	CHIP("F2", 7411)
	CHIP_9602_Mono(F3, &f3_desc)
	CHIP("F4", 7474)
	CHIP("F5", 7474)
	CHIP("F6", 74193)
	CHIP("F7", 74279)
	CHIP("F8", 7474)
	CHIP("F9", 7404)

	CHIP("H1", 7437)
	CHIP("H2", 7408)
	CHIP("H3", 7427)
	CHIP("H4", 7400)
	CHIP("H5", 9312)
	CHIP("H6", 9310)
	CHIP("H7", 7408)	//sometimes looks like N7 on schematic
	CHIP("H8", 7474)
	CHIP("H9", 7474)

	CHIP("J1", 74175)
	CHIP("J2", 7404)
	CHIP("J3", 7402)
	CHIP("J4", 9312)
	CHIP("J5", 7448)
	CHIP("J6", 9310)
	CHIP("J7", 7420)
	CHIP("J8", 74279)
	CHIP("J9", 7410)

	CHIP("K1", 9316)
	CHIP("K2", 7486)
	CHIP("K3", 7430)
	CHIP("K4", 7408)
	CHIP("K5", 9312)
	CHIP("K6", 9310)
	CHIP("K7", 7486)
	CHIP("K8", 7474)	//TODO: one more than bom?
	CHIP("K9", 74107)

	CHIP("L1", 9316)
	CHIP("L2", 7486)
	CHIP("L3", 82S16)	//RAM
	CHIP("L4", 7411)
	CHIP("L5", 9312)
	CHIP("L6", 9310)
	CHIP("L7", 7486)
	CHIP("L8", 74193)
	CHIP("L9", 7400)	//TODO: 1 more chip than on bom?

	CHIP("M1", 9316)
	CHIP("M2", 7483)
	CHIP("M3", 7486)
	CHIP("M4", 7410)
	CHIP("M5", 9312)
	CHIP("M6", 9310)
	CHIP("M8", 7427)
	CHIP("M9", 7404)

	CHIP("N1", 9316)
	CHIP("N2", 7483)
	CHIP("N3", 7486)
	CHIP("N4", 7411)
	CHIP("N5", 9312)
	CHIP("N6", 9310)
	CHIP("N7", 7408)	//sometimes looks like H7 on schematic
	CHIP_9602_Mono(N8, &n8_desc)
	CHIP("N9", 74192)

	//LM380			//speaker amplifier
	//LM323			//regulator

//    CHIP("CREDIT_LIGHT1", LAMP)
//    CHIP("CREDIT_LIGHT2", LAMP)
//    CHIP("SERVE_LIGHT", LAMP)

    CHIP("PAD1", PADDLE1_HORIZONTAL_INPUT, &pad1_desc)
    PADDLE_CONNECTION("PAD1", "C9")

    CHIP_LATCH(LATCH)
    CHIP("COIN1", COIN_INPUT)

    //CHIP("COIN2", COIN_INPUT)

    CHIP("START", START_INPUT)

    CHIP("SERVE", BUTTONS1_INPUT)

    //TODO: coin2 and start 2

    VIDEO(breakout)
    AUDIO(breakout)

#ifdef DEBUG
	CHIP("LOG1", VCD_LOG, &vcd_log_desc)
#endif

   //HSYNC and VSYNC
   #define H1_d 		"L1", 14
   #define H2_d 		"L1", 13
   #define H4_d 		"L1", 12
   #define H8_d 		"L1", 11
   #define H8_n 	"J2", 2
   #define H16_d  	"K1", 14
   #define H16_n 	"J2", 6
   #define H32_d 	 	"K1", 13
   #define H32_n 	"J2", 4
   #define H64_d 	 	"K1", 12
   #define H128_d  	"K1", 11

   #define V1_d 	 	"M1", 14
   #define V2_d 	 	"M1", 13
   #define V4_d 	 	"M1", 12
   #define V8_d 	 	"M1", 11
   #define V16_d 	 	"N1", 14
   #define V16_n 	"J2", 10
   #define V32_d 	 	"N1", 13
   #define V64_d 	 	"N1", 12
   #define V64I  	"H7", 11
   #define V64_n 	"M9", 10
   #define V128_d  	"N1", 11

   #define H1		"L2", 8
   #define H2		"L2", 11
   #define H4		"L2", 3
   #define H8		"L2", 6
   #define H16		"K2", 8
   #define H32		"K2", 11
   #define H64		"K2", 3
   #define H128		"K2", 6

   //#define V1
   #define V2		"M3", 3
   #define V4		"M3", 6
   #define V8		"M3", 11
   #define V16		"N3", 8
   #define V32		"N3", 3
   #define V64		"N3", 6
   #define V128		"N3", 11

   #define HSYNC  	"J1", 2
   #define HSYNC_n  	"J1", 3
   #define VSYNC  	"J1", 7
   #define VSYNC_n  	"J1", 6
   #define PSYNC  	"J1", 11
   #define PSYNC_n  	"J1", 10
   #define BSYNC  	"J1", 15
   #define BSYNC_n  	"J1", 14

   #define BALL 	"D7", 6
   #define BALL_DISPLAY "A4", 6
   #define PLAYFIELD 	"H4", 3
   #define SCORE 	"D3", 5
   #define VERT_TRIG_n 	"H1", 8

   #define CLOCK 	"H1", 11
   #define SCLOCK 	"K1", 15
   #define CKBH 	"F1", 13

   #define PAD_n 	"K3", 8
   #define PAD_EN_n 	"C2", 8

   //#define P VCC
   #define P "V5", Q

   #define COIN 		"L9", 6
   #define COIN1_n 		"F8", 5
   #define COIN2_n 		"H9", 5
   #define CREDIT_1_OR_2 	"L9", 3
   #define CREDIT_1_OR_2_n 	"F9", 8
   #define CREDIT2 		"F9", 6
   #define CREDIT2_n 		"M8", 8
   #define CR_START1 		"E8", 5
   #define CR_START1_n 		"E8", 6	//Schematic shows E8.6 as positive CR_START1, but this can't be right?
   #define CR_START2 		"E8", 9
   #define CR_START2_n 		"E8", 8
   #define CSW1 		"F9", 12
   #define CSW2 		"F9", 2

   #define P2_CONDITIONAL	"H1", 3
   #define P2_CONDITIONAL_dash 	"H7", 8
   #define PLAYER_2 		"B4", 14
   #define PLAYER_2_n 		"M9", 8

   #define START_GAME 		"D8", 6
   #define START_GAME1_n 	"M9", 4
   #define START_GAME_n 	"M9", 6

   #define BG1_n	"K8", 9
   #define BG1 		"K8", 8
   #define BG2_n	"K8", 5
   #define BG2		"K8", 6

   #define FREE_GAME_TONE	"N7", 3
   #define BONUS_COIN		"L9", 11

   //#define Q        "E9", 6
   #define LAT_Q	"E9", 6
   #define Q_n 		"LATCH", 3
   #define SBD_n 	"D2", 11

   #define PLAY_CP 	"D2", 8
   #define PLGM2_n 	"F7", 7
   #define VB_HIT_n 	"A5", 6

   #define SERVE_n 	"SERVE", 1
   #define SERVE_WAIT 	"A3", 9
   #define SERVE_WAIT_n "A3", 8

   #define BRICK_DISPLAY 	"E3", 1
   #define BRICK_HIT 		"E6", 5
   #define BRICK_HIT_n 		"E6", 6

   //#define EGL 		"A4", 3
   #define EGL		"C37" , 2
   #define EGL_n 	"C5", 2

   #define RAM_PLAYER1 	"E7", 4
   #define A1 		"H6", 14
   #define B1 		"H6", 13
   #define C1 		"H6", 12
   #define D1 		"H6", 11
   #define E1 		"J6", 14
   #define F1 		"J6", 13
   #define G1 		"J6", 12
   #define H01 		"J6", 11
   #define I1 		"K6", 14
   #define J1 		"K6", 13
   #define K1 		"K6", 12
   #define L1 		"K6", 11
   #define A2 		"N6", 14
   #define B2 		"N6", 13
   #define C2 		"N6", 12
   #define D2 		"N6", 11
   #define E2 		"M6", 14
   #define F2 		"M6", 13
   #define G2 		"M6", 12
   #define H02 		"M6", 11	//TODO: better name for these signals
   #define I2 		"L6", 14
   #define J2 		"L6", 13
   #define K2 		"L6", 12
   #define L2 		"L6", 11

   #define CX0 		"C6", 11
   #define CX1 		"C6", 6
   #define X0 		"C5", 10
   #define X1 		"B6", 3
   #define X2 		"C6", 3
   #define Y0 		"B6", 11
   #define Y1 		"B6", 6
   #define Y2 		"A6", 6
   #define DN 		"C4", 3
   #define PC 		"D4", 12
   #define PD 		"D4", 11
   #define SU_n 	"D5", 8
   #define V_SLOW 	"C5", 8

   #define PLNR 	"E3", 4
   #define SCI_n 	"H4", 6
   #define SFL_n 	"E9", 12	//score flash
   #define TOP_n 	"E9", 2

   #define BP_HIT_n 	"A5", 8
   #define BTB_HIT_n 	"C3", 3

   #define SET_BRICKS 	"D3", 9
   #define SET_BRICKS_n "D3", 8

   #define BALL_A 	"B4", 13
   #define BALL_B 	"B4", 12
   #define BALL_C 	"B4", 11

   #define FPD1 	"F3", 10
   #define FPD1_n 	"F3", 9
   #define FPD2 	"F3", 6
   #define FPD2_n 	"F3", 7

   #define COUNT 	"N7", 11
   #define COUNT_1 	"N7", 8
   #define COUNT_2 	"N7", 6

   #define ATTRACT 	"E6", 8
   #define ATTRACT_n 	"E6", 9

   #define BRICK_SOUND 	"B8", 14
   #define P_HIT_SOUND 	"B7", 12
   #define VB_HIT_SOUND "B7", 11

   #define LH_SIDE 	"J3", 13
   #define RH_SIDE 	"H2", 3
   #define TOP_BOUND 	"K4", 6

//   CONNECTION(CREDIT_1_OR_2, "CREDIT_LIGHT1", 1)
//   CONNECTION(CREDIT2, "CREDIT_LIGHT2", 1)
//   CONNECTION(SERVE_WAIT_n, "SERVE_LIGHT", 1)

   //Audio
   CONNECTION("M9", 2, "F6", 5)
   CONNECTION("M9", 2, "F7", 15)
   CONNECTION("F6", 13, "F7", 14)
   CONNECTION(START_GAME_n, "F6", 11)
   CONNECTION(P, "F6", 15)
   CONNECTION(P, "F6", 1)
   CONNECTION(P, "F6", 10)
   CONNECTION(P, "F6", 9)
   CONNECTION(GND, "F6", 14)
   CONNECTION("F7", 13, "J9", 2)
   CONNECTION(VSYNC, "J9", 1)
   CONNECTION("A7", 9, "J9", 13)
   CONNECTION("J9", 12, "F6", 4)
   CONNECTION("J9", 12, "A7", 11)
   CONNECTION(GND, "A7", 12)
   CONNECTION(ATTRACT_n, "A7", 13)
   CONNECTION("J9", 12, "A8", 11)
   CONNECTION(GND, "A8", 12)
   CONNECTION(ATTRACT_n, "A8", 13)
   CONNECTION(VB_HIT_n, "A7", 5)
   CONNECTION(GND, "A7", 4)
   CONNECTION(ATTRACT_n, "A7", 3)
   CONNECTION(BP_HIT_n, "A8", 5)
   CONNECTION(GND, "A8", 4)
   CONNECTION(ATTRACT_n, "A8", 3)
   CONNECTION("A8", 6, "B9", 13)
   CONNECTION(P_HIT_SOUND, "B9", 12)
   CONNECTION("A8", 10, "B9", 10)
   CONNECTION(BRICK_SOUND, "B9", 9)
   CONNECTION("A7", 6, "B9", 4)
   CONNECTION(VB_HIT_SOUND, "B9", 5)

   CONNECTION(GND, "S1", 1)
   CONNECTION(P, "S1", 2)
   CONNECTION(GND, "S1", 4)
   CONNECTION(P, "S1", 3)
   CONNECTION(GND, "S1", 12)
   CONNECTION(P, "S1", 11)
   CONNECTION(GND, "S1", 9)
   CONNECTION(P, "S1", 10)

   //Free Game Selector
   CONNECTION(I1, "K7", 2)
   CONNECTION("S1", 15, "K7", 1)
   CONNECTION(J1, "K7", 12)
   CONNECTION("S1", 14, "K7", 13)
   CONNECTION(K1, "K7", 5)
   CONNECTION("S1", 6, "K7", 4)
   CONNECTION(L1, "K7", 9)
   CONNECTION("S1", 7, "K7", 10)

   CONNECTION(I2, "L7", 2)
   CONNECTION("S1", 15, "L7", 1)
   CONNECTION(J2, "L7", 12)
   CONNECTION("S1", 14, "L7", 13)
   CONNECTION(K2, "L7", 5)
   CONNECTION("S1", 6, "L7", 4)
   CONNECTION(L2, "L7", 9)
   CONNECTION("S1", 7, "L7", 10)


   CONNECTION("K7", 3, "J7", 13)
   CONNECTION("K7", 11, "J7", 12)
   CONNECTION("K7", 6, "J7", 10)
   CONNECTION("K7", 8, "J7", 9)

   CONNECTION("L7", 3, "J7", 1)
   CONNECTION("L7", 11, "J7", 2)
   CONNECTION("L7", 6, "J7", 4)
   CONNECTION("L7", 8, "J7", 5)

   CONNECTION(START_GAME1_n, "J8", 12)
   CONNECTION(BG1_n, "J8", 11)
   CONNECTION("J7", 8, "J8", 10)

   CONNECTION(START_GAME1_n, "J8", 2)
   CONNECTION(BG2_n, "J8", 3)
   CONNECTION("J7", 6, "J8", 1)

   CONNECTION("J8", 9, "K8", 12)
   CONNECTION(EGL, "K8", 11)
   CONNECTION(P, "K8", 13)
   CONNECTION(LAT_Q, "K8", 10)

   CONNECTION("J8", 4, "K8", 2)
   CONNECTION(EGL, "K8", 3)
   CONNECTION(P, "K8", 1)
   CONNECTION(LAT_Q, "K8", 4)

   CONNECTION(P, "K9", 8)
   CONNECTION("J8", 9, "K9", 9)
   CONNECTION(GND, "K9", 11)
   CONNECTION(HSYNC_n, "K9", 10)

   CONNECTION(P, "K9", 1)
   CONNECTION("J8", 4, "K9", 12)
   CONNECTION(GND, "K9", 4)
   CONNECTION(HSYNC_n, "K9", 13)

   CONNECTION("K9", 6, "L9", 12)
   CONNECTION("K9", 2, "L9", 13)

   CONNECTION(P, "N8", 5)
   CONNECTION(BONUS_COIN, "N8", 4)
   CONNECTION(ATTRACT_n, "N8", 3)

   CONNECTION(V4_d, "N7", 2)
   CONNECTION("N8", 6, "N7", 1)
   //
   CONNECTION(GND, "M2", 13)
   CONNECTION(V1_d, "M2", 10)
   CONNECTION(V2_d, "M2", 8)
   CONNECTION(V4_d, "M2", 3)
   CONNECTION(V8_d, "M2", 1)
   CONNECTION(GND, "M2", 11)
   CONNECTION(P2_CONDITIONAL, "M2", 7)
   CONNECTION(GND, "M2", 4)
   CONNECTION(GND, "M2", 16)

   CONNECTION("M2", 14, "N2", 13)
   CONNECTION(V16_d, "N2", 10)
   CONNECTION(V32_d, "N2", 8)
   CONNECTION(V64_d, "N2", 3)
   CONNECTION(V128_d, "N2", 1)
   CONNECTION(GND, "N2", 11)
   CONNECTION(P2_CONDITIONAL, "N2", 7)
   CONNECTION(GND, "N2", 4)
   CONNECTION(GND, "N2", 16)

   CONNECTION("M2", 6, "M3", 2)
   CONNECTION(P2_CONDITIONAL, "M3", 1)
   CONNECTION("M2", 2, "M3", 5)
   CONNECTION(P2_CONDITIONAL, "M3", 4)
   CONNECTION("M2", 15, "M3", 12)
   CONNECTION(P2_CONDITIONAL, "M3", 13)

   CONNECTION(P2_CONDITIONAL, "N3", 10)
   CONNECTION("N2", 9, "N3", 9)
   CONNECTION(P2_CONDITIONAL, "N3", 1)
   CONNECTION("N2", 6, "N3", 2)
   CONNECTION(P2_CONDITIONAL, "N3", 4)
   CONNECTION("N2", 2, "N3", 5)
   CONNECTION(P2_CONDITIONAL, "N3", 13)
   CONNECTION("N2", 15, "N3", 12)

   CONNECTION(H1_d, "L2", 9)
   CONNECTION(P2_CONDITIONAL, "L2", 10)
   CONNECTION(H2_d, "L2", 12)
   CONNECTION(P2_CONDITIONAL, "L2", 13)
   CONNECTION(H4_d, "L2", 2)
   CONNECTION(P2_CONDITIONAL, "L2", 1)
   CONNECTION(H8_d, "L2", 5)
   CONNECTION(P2_CONDITIONAL, "L2", 4)

   CONNECTION(P2_CONDITIONAL, "K2", 10)
   CONNECTION(H16_d, "K2", 9)
   CONNECTION(P2_CONDITIONAL, "K2", 13)
   CONNECTION(H32_d, "K2", 12)
   CONNECTION(P2_CONDITIONAL, "K2", 1)
   CONNECTION(H64_d, "K2", 2)
   CONNECTION(P2_CONDITIONAL, "K2", 4)
   CONNECTION(H128_d, "K2", 5)


   CONNECTION(V64, "M9", 11)
   CONNECTION(H16, "J2", 5)
   CONNECTION(H32, "J2", 3)
   CONNECTION(V16, "J2", 11)
   CONNECTION(H8, "J2", 1)

   CONNECTION(CLOCK, "J1", 9)
   CONNECTION(SCLOCK, "J1", 4)
   CONNECTION("N4", 6, "J1", 5)
   CONNECTION(PAD_n, "J1", 12)
   CONNECTION(BALL_DISPLAY, "J1", 13)
   CONNECTION(P, "J1", 1)

   CONNECTION(P, "K1", 1)
   CONNECTION(P, "K1", 3)
   CONNECTION(P, "K1", 4)
   CONNECTION(P, "K1", 5)
   CONNECTION(P, "K1", 6)
   CONNECTION(P, "K1", 9)
   CONNECTION(P, "K1", 10)
   CONNECTION(CLOCK, "K1", 2)
   CONNECTION("L1", 15, "K1", 7)

   CONNECTION(P, "L1", 1)
   CONNECTION(P, "L1", 3)
   CONNECTION(GND, "L1", 4)
   CONNECTION(P, "L1", 5)
   CONNECTION(GND, "L1", 6)
   CONNECTION(VERT_TRIG_n, "L1", 9)
   CONNECTION(P, "L1", 10)
   CONNECTION(CLOCK, "L1", 2)
   CONNECTION(P, "L1", 7)

   CONNECTION(P, "N1", 1)
   CONNECTION(P, "N1", 10)
   CONNECTION(P, "N1", 3)
   CONNECTION(P, "N1", 4)
   CONNECTION(P, "N1", 5)
   CONNECTION(P, "N1", 6)
   CONNECTION(P, "N1", 9)
   CONNECTION(CLOCK, "N1", 2)
   CONNECTION("H2", 6, "N1", 7)

   CONNECTION("M1", 15, "H2", 5)
   CONNECTION("L1", 15, "H2", 4)

   CONNECTION(V128_d, "N4", 5)
   CONNECTION(V64_d, "N4", 3)
   CONNECTION(V32_d, "N4", 4)
   CONNECTION("N4", 6, "H1", 10)
   CONNECTION(VSYNC_n, "H1", 9)

   CONNECTION(P, "M1", 1)
   CONNECTION(GND, "M1", 3)
   CONNECTION(GND, "M1", 4)
   CONNECTION(P, "M1", 5)
   CONNECTION(GND, "M1", 6)
   CONNECTION(VERT_TRIG_n, "M1", 9)
   CONNECTION(CLOCK, "M1", 2)
   CONNECTION("L1", 15, "M1", 7)
   CONNECTION("K1", 15, "M1", 10)


   //9312 circuit
   CONNECTION(PLAYER_2, "M9", 9)
   CONNECTION(BALL_A, "C5", 5)
   CONNECTION(BALL_A, "C4", 13)
   CONNECTION(BALL_B, "C4", 12)
   CONNECTION(BALL_A, "A4", 13)
   CONNECTION(BALL_B, "A4", 12)
   CONNECTION(BALL_C, "C4", 10)
   CONNECTION("A4", 11, "C4", 9)

   CONNECTION(A2, "N5", 1)
   CONNECTION(E2, "N5", 2)
   CONNECTION(I2, "N5", 3)
   CONNECTION("C5", 6, "N5", 4)
   CONNECTION(A1, "N5", 5)
   CONNECTION(E1, "N5", 6)
   CONNECTION(I1, "N5", 7)
   CONNECTION(PLAYER_2_n, "N5", 9)
   CONNECTION(H32_n, "N5", 10)
   CONNECTION(V16, "N5", 11)
   CONNECTION(V64, "N5", 12)
   CONNECTION(V128, "N5", 13)

   CONNECTION(B2, "M5", 1)
   CONNECTION(F2, "M5", 2)
   CONNECTION(J2, "M5", 3)
   CONNECTION("C4", 11, "M5", 4)
   CONNECTION(B1, "M5", 5)
   CONNECTION(F1, "M5", 6)
   CONNECTION(J1, "M5", 7)
   CONNECTION(PLAYER_2, "M5", 9)
   CONNECTION(H32_n, "M5", 10)
   CONNECTION(V16, "M5", 11)
   CONNECTION(V64, "M5", 12)
   CONNECTION(V128, "M5", 13)

   CONNECTION(C2, "L5", 1)
   CONNECTION(G2, "L5", 2)
   CONNECTION(K2, "L5", 3)
   CONNECTION("C4", 8, "L5", 4)
   CONNECTION(C1, "L5", 5)
   CONNECTION(G1, "L5", 6)
   CONNECTION(K1, "L5", 7)
   CONNECTION(GND, "L5", 9)
   CONNECTION(H32_n, "L5", 10)
   CONNECTION(V16, "L5", 11)
   CONNECTION(V64, "L5", 12)
   CONNECTION(V128, "L5", 13)

   CONNECTION(D2, "K5", 1)
   CONNECTION(H02, "K5", 2)
   CONNECTION(L2, "K5", 3)
   CONNECTION(GND, "K5", 4)
   CONNECTION(D1, "K5", 5)
   CONNECTION(H01, "K5", 6)
   CONNECTION(L1, "K5", 7)
   CONNECTION(GND, "K5", 9)
   CONNECTION(H32_n, "K5", 10)
   CONNECTION(V16, "K5", 11)
   CONNECTION(V64, "K5", 12)
   CONNECTION(V128, "K5", 13)

   CONNECTION(P, "J5", 4)
   CONNECTION(P, "J5", 3)
   CONNECTION("N5", 15, "J5", 7)
   CONNECTION("M5", 15, "J5", 1)
   CONNECTION("L5", 15, "J5", 2)
   CONNECTION("K5", 15, "J5", 6)
   CONNECTION(H32, "J5", 5)

   CONNECTION("J5", 13, "H5", 1)
   CONNECTION(GND, "H5", 2)
   CONNECTION(GND, "H5", 3)
   CONNECTION("J5", 14, "H5", 4)
   CONNECTION(GND, "H5", 5)
   CONNECTION(GND, "H5", 6)
   CONNECTION("J5", 10, "H5", 7)
   CONNECTION(GND, "H5", 9)

   CONNECTION(V4, "K4", 12)
   CONNECTION(V8, "K4", 13)

   CONNECTION("K4", 11, "H5", 10)
   CONNECTION(H2, "H5", 11)
   CONNECTION(H4, "H5", 12)
   CONNECTION(H8, "H5", 13)

   CONNECTION(H2, "L4", 3)
   CONNECTION(H4, "L4", 5)
   CONNECTION(H8, "L4", 4)

   CONNECTION("J5", 12 , "J4", 1)
   CONNECTION("J5", 11, "J4", 2)
   CONNECTION(GND, "J4", 3)
   CONNECTION(GND, "J4", 4)
   CONNECTION("J5", 15, "J4", 5)
   CONNECTION("J5", 9, "J4", 6)
   CONNECTION(GND, "J4", 7)
   CONNECTION(GND, "J4", 9)
   CONNECTION("L4", 6, "J4", 10)
   CONNECTION(H8, "J4", 11)
   CONNECTION(V4, "J4", 12)
   CONNECTION(V8, "J4", 13)

   CONNECTION("H5", 14, "H4", 13)
   CONNECTION("J4", 14, "H4", 12)

   //PADDLES
   CONNECTION(ATTRACT_n, "B2", 4)

   CONNECTION("B2", 3, "E9", 13)
   CONNECTION(PLAYER_2_n, "M3", 9)
   CONNECTION(V128, "M3", 10)
   CONNECTION(H64, "J3", 8)
   CONNECTION(H128, "J3", 9)
   CONNECTION(V32, "E3", 5)
   CONNECTION(V16_n, "E3", 6)

   CONNECTION(SFL_n, "M8", 1)
   CONNECTION("M3", 8, "M8", 2)
   CONNECTION(PLNR, "M8", 13)
   CONNECTION("J3", 10, "E9", 1)
   CONNECTION(V64, "E2", 5)
   CONNECTION(V32, "E2", 4)
   CONNECTION(PLNR, "E2", 10)
   CONNECTION(H16, "E2", 9)

   CONNECTION("M8", 12, "M8", 3)
   CONNECTION(TOP_n, "M8", 4)
   CONNECTION(TOP_n, "M8", 5)
   CONNECTION("H4", 11, "F2", 11)
   CONNECTION("E2", 6, "F2", 10)
   CONNECTION("E2", 8, "F2", 9)

   CONNECTION("M8", 6, "H4", 5)
   CONNECTION("F2", 8, "H4", 4)

   CONNECTION(PAD_EN_n, "PAD_EN_BUF", 1)

   CONNECTION("PAD_EN_BUF", 2, "C9", 4)
   CONNECTION("PAD_EN_BUF", 2, "C9", 2)
   CONNECTION(BTB_HIT_n, "C5", 3)
   CONNECTION(P, "F5", 10)
   CONNECTION(P, "F5", 12)
   CONNECTION("C5", 4, "F5", 11)
   CONNECTION(SERVE_WAIT_n, "F5", 13)
   CONNECTION(H64, "E5", 13)
   CONNECTION("F5", 9, "E5", 12)
   CONNECTION(H128, "E5", 10)
   CONNECTION("F5", 8, "E5", 9)
   CONNECTION("E5", 11, "E4", 12)
   CONNECTION("E5", 8, "E4", 13)
   CONNECTION("E4", 11, "D4", 2)
   CONNECTION(P, "D4", 3)
   CONNECTION(P, "D4", 4)
   CONNECTION(P, "D4", 5)
   CONNECTION(P, "D4", 6)
   CONNECTION(P, "D4", 9)
   CONNECTION(P, "D4", 10)
   CONNECTION("C3", 11, "D4", 7)
   CONNECTION(VSYNC_n, "D4", 1)

   CONNECTION("D4", 15, "E4", 10)
   CONNECTION("H7", 6, "E4", 9)
   CONNECTION("C9", 3, "H7", 5)
   CONNECTION(PAD_EN_n, "H7", 4)
   CONNECTION("E4", 8, "C3", 12)
   CONNECTION(ATTRACT_n, "C3", 13)
   CONNECTION(H8, "J3", 2)
   CONNECTION(H32, "J3", 3)

   CONNECTION("C3", 11, "K3", 12)
   CONNECTION(H128, "K3", 5)
   CONNECTION(H64, "K3", 6)
   CONNECTION(H16, "K3", 11)
   CONNECTION(H4, "K3", 4)
   CONNECTION("J3", 1, "K3", 1)
   CONNECTION(P, "K3", 3)
   CONNECTION(P, "K3", 2)

   CONNECTION(V16_d, "D7", 1)
   CONNECTION(V64_d, "D7", 13)
   CONNECTION(V128_d, "D7", 2)
   CONNECTION("D7", 12, "H1", 4)
   CONNECTION(V8_d, "H1", 5)
   CONNECTION("H1", 6, "C2", 4)
   CONNECTION("H1", 6, "C2", 5)
   CONNECTION(V32_d, "J2", 9)
   CONNECTION("J2", 8, "C2", 10)
   CONNECTION("C2", 6, "C2", 9)


   //SCORE
   CONNECTION(SCI_n, "D3", 4)
   CONNECTION(GND, "D3", 2)
   CONNECTION(GND, "D3", 3)
   CONNECTION(GND, "D3", 1)

   //PLAYER2_CONDITIONAL
   CONNECTION(PLAYER_2, "H7", 10)
   CONNECTION(GND, "S2", 1)
   CONNECTION(P, "S2", 2)
   CONNECTION("S2", 3, "H7", 9)

   //A-L 1 and 2
   CONNECTION(SET_BRICKS_n, "B3", 2)
   CONNECTION(H2, "B3", 3)
   CONNECTION("B3", 1, "E7", 6)
   CONNECTION(PLAYER_2, "E7", 5)
   CONNECTION(P, "N6", 9)
   CONNECTION(P, "M6", 9)
   CONNECTION(P, "L6", 9)
   CONNECTION(P, "H6", 9)
   CONNECTION(P, "J6", 9)
   CONNECTION(P, "K6", 9)

   CONNECTION(P, "N6", 10)
   CONNECTION(PLAYER_2, "N6", 7)
   CONNECTION(COUNT_2, "N6", 2)
   CONNECTION(START_GAME_n, "N6", 1)

   CONNECTION("N6", 15, "M6", 10)
   CONNECTION(PLAYER_2, "M6", 7)
   CONNECTION(COUNT_2, "M6", 2)
   CONNECTION(START_GAME_n, "M6", 1)

   CONNECTION("M6", 15, "L6", 10)
   CONNECTION(PLAYER_2, "L6", 7)
   CONNECTION(COUNT_2, "L6", 2)
   CONNECTION(START_GAME_n, "L6", 1)

   CONNECTION(P, "H6", 10)
   CONNECTION(RAM_PLAYER1, "H6", 7)
   CONNECTION(COUNT_1, "H6", 2)
   CONNECTION(START_GAME_n, "H6", 1)

   CONNECTION("H6", 15, "J6", 10)
   CONNECTION(RAM_PLAYER1, "J6", 7)
   CONNECTION(COUNT_1, "J6", 2)
   CONNECTION(START_GAME_n, "J6", 1)

   CONNECTION("J6", 15, "K6", 10)
   CONNECTION(RAM_PLAYER1, "K6", 7)
   CONNECTION(COUNT_1, "K6", 2)
   CONNECTION(START_GAME_n, "K6", 1)


   //CX0 and CX1
   CONNECTION(BRICK_HIT, "H2", 9)
   CONNECTION(H16_n, "H2", 10)
   CONNECTION(P, "D5", 10)
   CONNECTION(P, "D5", 12)
   CONNECTION("H2", 8, "D5", 11)
   CONNECTION(SERVE_WAIT_n, "D5", 13)
   CONNECTION(X0, "C6", 13)
   CONNECTION("D5", 9, "C6", 12)
   CONNECTION("D5", 9, "D6", 12)
   CONNECTION(DN, "D6", 13)
   CONNECTION("D6", 11, "C6", 4)
   CONNECTION(X1, "C6", 5)

   //COUNT1 and COUNT2
   CONNECTION(P, "N8", 11)
   CONNECTION(BRICK_HIT, "N8", 12)
   CONNECTION(ATTRACT_n, "N8", 13)
   CONNECTION("N8", 9, "N9", 11)
   CONNECTION(P, "N9", 15)
   CONNECTION(P, "N9", 5)

   CONNECTION(COUNT, "N9", 4)
   CONNECTION(START_GAME, "N9", 14)
   CONNECTION(H8_n, "N9", 1)
   CONNECTION(H16_n, "N9", 10)
   CONNECTION(GND, "N9", 9)

   CONNECTION("N9", 13, "N7", 13)
   CONNECTION(SCLOCK, "N7", 12)

   CONNECTION(PLAYER_2, "N7", 5)
   CONNECTION(COUNT, "N7", 4)

   CONNECTION(COUNT, "M9", 1)
   CONNECTION(COUNT, "N7", 10)
   CONNECTION(RAM_PLAYER1, "N7", 9)


   //Ball Logic
   CONNECTION(P, "C7", 1)
   CONNECTION(P, "C7", 10)
   CONNECTION(P, "C7", 7)
   CONNECTION(CX0, "C7", 3)
   CONNECTION(CX1, "C7", 4)
   CONNECTION(X2, "C7", 5)
   CONNECTION(GND, "C7", 6)
   CONNECTION("D8", 8, "C7", 9)
   CONNECTION("C7", 15, "C8", 7)
   CONNECTION("C7", 11, "C8", 10)
   CONNECTION("C7", 12, "D7", 10)
   CONNECTION("C7", 13, "D7", 11)
   CONNECTION(CLOCK, "C7", 2)

   CONNECTION(P, "C8", 1)
   CONNECTION(P, "C8", 3)
   CONNECTION(P, "C8", 4)
   CONNECTION(P, "C8", 5)
   CONNECTION(P, "C8", 6)
   CONNECTION(P, "C8", 9)
   CONNECTION(CLOCK, "C8", 2)
   CONNECTION("C8", 15, "B7", 7)
   CONNECTION("C7", 15, "B7", 10)

   CONNECTION(P, "B7", 1)
   CONNECTION(Y0, "B7", 3)
   CONNECTION(Y1, "B7", 4)
   CONNECTION(Y2, "B7", 5)
   CONNECTION(GND, "B7", 6)
   CONNECTION("D8", 8, "B7", 9)
   CONNECTION(CLOCK, "B7", 2)
   CONNECTION("B7", 15, "B8", 7)

   CONNECTION(VB_HIT_SOUND, "D7", 9)

   CONNECTION(P, "B8", 1)
   CONNECTION(P, "B8", 3)
   CONNECTION(P, "B8", 4)
   CONNECTION(P, "B8", 5)
   CONNECTION(P, "B8", 6)
   CONNECTION(P, "B8", 9)
   CONNECTION("C8", 15, "B8", 10)
   CONNECTION(CLOCK, "B8", 2)

   CONNECTION("B8", 15, "D8", 10)
   CONNECTION("B7", 15, "D8", 9)

   CONNECTION("D7", 8, "D7", 5)
   CONNECTION(P_HIT_SOUND, "D7", 4)
   CONNECTION("B8", 15, "D7", 3)

   //Clock Generator
   CONNECTION(CKBH, "H1", 12)
   CONNECTION("F1", 14, "H1", 13)
   CONNECTION("F1", 15, "E1", 5)

   CONNECTION(P, "F1", 1)
   CONNECTION(P, "F1", 7)
   CONNECTION(P, "F1", 10)
   CONNECTION(GND, "F1", 3)
   CONNECTION(P, "F1", 4)
   CONNECTION(GND, "F1", 5)
   CONNECTION(GND, "F1", 6)
   CONNECTION("E1", 6, "F1", 9)
   CONNECTION("Y1", 1, "F1", 2)

   // RH and LH Sides
   CONNECTION(V128, "N4", 1)
   CONNECTION(V64, "N4", 2)
   CONNECTION(V16, "N4", 13)
   CONNECTION("N4", 12, "N4", 11)
   CONNECTION(V8, "N4", 10)
   CONNECTION(V4, "N4", 9)
   CONNECTION("N4", 8, "H2", 2)
   CONNECTION(V32, "H2", 1)
   CONNECTION("N4", 8, "J2", 13)
   CONNECTION("J2", 12, "J3", 11)
   CONNECTION(V32, "J3", 12)

   // Top Bound
   CONNECTION(H128, "H3", 4)
   CONNECTION(H64, "H3", 5)
   CONNECTION(H32, "H3", 3)
   CONNECTION("H3", 6, "L4", 9)
   CONNECTION(H16, "L4", 10)
   CONNECTION(H8, "L4", 11)
   CONNECTION("L4", 8, "K4", 5)
   CONNECTION(VSYNC_n, "K4", 4)

   //Cabinet type

   // Coin Circuit
   CONNECTION("COIN1", 1, "F9", 13)

   CONNECTION(CSW1, "F9", 11)
   //CONNECTION("F9", 10, "F9", 13) //TODO: causes lots of bouncing, commented out since this trace is not implemented in gotcha

   CONNECTION(V64, "H7", 12)
   CONNECTION(V64, "H7", 13)

   CONNECTION(CSW1, "F8", 10)
   CONNECTION("F9", 10, "F8", 12)
   CONNECTION(V64I, "F8", 11)
   CONNECTION(P, "F8", 13)
   CONNECTION(P, "F8", 1)
   CONNECTION(V64I, "F8", 3)
   CONNECTION("F8", 9, "F8", 2)
   CONNECTION(CSW1, "F8", 4)

   CONNECTION("F8", 6, "H8", 12)
   CONNECTION(P, "H8", 10)
   CONNECTION(V16_d, "H8", 11)
   CONNECTION(P, "H8", 13)
   CONNECTION("H8", 9, "J8", 15)
   CONNECTION("H8", 9, "J9", 9)
   CONNECTION(V16_d, "J8", 14)
   CONNECTION("J8", 13, "J9", 10)
   CONNECTION(P, "S3", 1)
   CONNECTION(V4_d, "S3", 2)
   CONNECTION("S3", 3, "J9", 11)

   CONNECTION("J9", 8, "L9", 5)
   CONNECTION("J9", 6, "L9", 4)

   //COIN2 circuit
   //CONNECTION("COIN2", 1, "F9", 1)
   CONNECTION(GND, "F9", 1)		//TODO: coin2 not implemented

   CONNECTION(CSW2, "F9", 3)
   CONNECTION(CSW2, "H9", 10)
   CONNECTION("F9", 4, "H9", 12)
   CONNECTION(V64_n, "H9", 11)
   CONNECTION(V64_n, "H9", 3)
   CONNECTION(P, "H9", 13)
   CONNECTION("H9", 9, "H9", 2)
   CONNECTION(CSW2, "H9", 4)
   CONNECTION(P, "H9", 1)

   CONNECTION(P, "H8", 4)
   CONNECTION("H9", 6, "H8", 2)
   CONNECTION(V16_d, "H8", 3)
   CONNECTION(P, "H8", 1)
   CONNECTION("H8", 5, "J8", 6)
   CONNECTION(V16_d, "J8", 5)
   CONNECTION(P, "J9", 3)
   CONNECTION("H8", 5, "J9", 5)
   CONNECTION("J8", 7, "J9", 4)

   CONNECTION(P2_CONDITIONAL_dash, "E9", 9)
   CONNECTION("E9", 8, "H1", 1)
   CONNECTION("E9", 8, "H1", 2)

   //Start2 Input
   CONNECTION("START", 2, "E9", 11)
   CONNECTION("E9", 10, "E8", 12)
   CONNECTION(P, "E8", 10)
   CONNECTION(V64I, "E8", 11)

   CONNECTION(V128_d, "F7", 2)
   CONNECTION(V128_d, "F7", 3)
   CONNECTION(CREDIT2_n, "F7", 1)
   CONNECTION(ATTRACT_n, "E7", 12)
   CONNECTION("F7", 4, "E7", 11)
   CONNECTION("E7", 13, "E8", 13)

   //Start1 Input
   CONNECTION("START", 1, "E9", 3)
   CONNECTION("E9", 4, "E8", 2)
   CONNECTION(P, "E8", 4)
   CONNECTION(V64_d, "E8", 3)


   CONNECTION(CREDIT_1_OR_2_n, "E7", 2)
   CONNECTION(ATTRACT_n, "E7", 3)
   CONNECTION("E7", 1, "E8", 1)

   CONNECTION(CR_START1_n, "D8", 4) // Schematic shows CR_START1 ?
   CONNECTION(CR_START2_n, "D8", 5)


   CONNECTION(START_GAME, "M9", 3)
   CONNECTION(START_GAME, "M9", 5)

   CONNECTION(V32, "D6", 4)
   CONNECTION(ATTRACT, "D6", 5)
   CONNECTION(P, "E6", 10)
   CONNECTION(START_GAME, "E6", 12)
   CONNECTION("D6", 6, "E6", 11)
   CONNECTION("D6", 3, "E6", 13)

   //TODO: hows this whole latch stuff work? what about Q_n going to COIN1_n and COIN2_n
   CONNECTION(CREDIT_1_OR_2_n, "D8", 13)
   CONNECTION(EGL, "D8", 12)
   CONNECTION("LATCH", 1, "D8", 11)		//set
   CONNECTION("LATCH", 2, COIN1_n)		//reset
   //CONNECTION("LATCH", 3, COIN2_n)		//set	//TODO: coin2 here
   CONNECTION( "LATCH", 3, "E9", 5)		//output


   CONNECTION(LAT_Q, "D6", 1)
   CONNECTION(EGL_n, "D6", 2)

   //Serve
   CONNECTION(H64, "J3", 6)
   CONNECTION(H32, "J3", 5)
   CONNECTION("J3", 4, "L4", 13)
   CONNECTION(H128, "L4", 2)
   CONNECTION(H16, "L4", 1)

   CONNECTION(BALL_DISPLAY, "H2", 13)
   CONNECTION(H128, "H2", 12)
   CONNECTION("H2", 11, "C3", 9)
   CONNECTION(HSYNC, "C3", 10)
   CONNECTION("C3", 8, "B3", 5)
   CONNECTION(H8_d, "B3", 6)
   CONNECTION("B3", 4, "C3", 4)
   CONNECTION(H4, "C3", 5)
   CONNECTION("C3", 6, "A4", 9)
   CONNECTION(START_GAME1_n, "A4", 10)

   CONNECTION(SERVE_WAIT_n, "D2", 13)

   CONNECTION(SERVE_n, "D2", 12)
   CONNECTION(SERVE_n, "A3", 4)
   CONNECTION(P, "A3", 2)
   CONNECTION(ATTRACT, "A3", 3)
   CONNECTION(SERVE_WAIT, "A3", 1)

   CONNECTION(BALL, "E1", 13)
   CONNECTION("E1", 12, "B3", 8)
   CONNECTION("A3", 6, "B3", 9)
   CONNECTION("B3", 10, "B3", 11)
   CONNECTION(SERVE_WAIT_n, "B3", 12)
   CONNECTION("B3", 13, "A3", 12)
   CONNECTION("A4", 8, "A3", 10)
   CONNECTION("L4", 12, "A3", 11)
   CONNECTION(P, "A3", 13)

   //Set Bricks
   CONNECTION(P, "D3", 10)
   CONNECTION(P, "D3", 12)
   CONNECTION(START_GAME, "D3", 11)
   CONNECTION(SERVE_n, "D3", 13)

   //Playfield
   CONNECTION(LH_SIDE, "H3", 1)
   CONNECTION(TOP_BOUND, "H3", 13)
   CONNECTION(RH_SIDE, "H3", 2)
   CONNECTION("H3", 12, "H4", 2)
   CONNECTION("E1", 2, "C36", 1)
   CONNECTION("C36", 2, "H4", 1)

   CONNECTION(BALL_DISPLAY, "A5", 10)
   CONNECTION(PSYNC, "A5", 9)
   CONNECTION(BSYNC, "C3", 2)
   CONNECTION(TOP_BOUND, "C3", 1)

   CONNECTION(PC, "C4", 4)
   CONNECTION(PD, "C4", 5)
   CONNECTION(BP_HIT_n, "C5", 13)
   CONNECTION(PD, "A5", 1)
   CONNECTION("C5", 12, "A5", 2)
   CONNECTION(BSYNC, "A5", 5)
   CONNECTION(VSYNC, "A5", 4)

   CONNECTION("C5", 12, "A5", 13)
   CONNECTION("A5", 3, "A5", 12)

   CONNECTION(BRICK_HIT, "D5", 3)
   CONNECTION("E5", 3, "D5", 1)
   CONNECTION("D5", 6, "D5", 2)
   CONNECTION(BP_HIT_n, "D5", 4)

   CONNECTION(P, "A6", 10)
   CONNECTION("C4", 6, "A6", 12)
   CONNECTION(BP_HIT_n, "A6", 11)
   CONNECTION(P, "A6", 13)

   CONNECTION("A5", 3, "A6", 4)
   CONNECTION(V16_d, "A6", 2)
   CONNECTION(VB_HIT_n, "A6", 3)
   CONNECTION("A5", 11, "A6", 1)

   CONNECTION(P2_CONDITIONAL, "C6", 1)
   CONNECTION("D5", 5, "C6", 2)
   CONNECTION("D5", 6, "C4", 2)
   CONNECTION(P2_CONDITIONAL, "C4", 1)

   CONNECTION(V_SLOW, "B6", 12)
   CONNECTION("A6", 8, "B6", 13)
   CONNECTION(V_SLOW, "C6", 10)
   CONNECTION("A6", 5, "C6", 9)

   CONNECTION(Y0, "B6", 4)
   CONNECTION("C6", 8, "B6", 5)

   CONNECTION(X2, "D6", 10)
   CONNECTION("B6", 8, "D6", 9)
   CONNECTION("B5", 7, "B6", 9)
   CONNECTION("B5", 6, "B6", 10)
   CONNECTION(X0, "B6", 1)
   CONNECTION(X2, "B6", 2)

   CONNECTION("B5", 6, "C5", 11)
   CONNECTION("B5", 7, "C5", 9)

   CONNECTION(SU_n, "B5", 11)
   CONNECTION(P, "B5", 15)
   CONNECTION(P, "B5", 1)
   CONNECTION(P, "B5", 10)
   CONNECTION(P, "B5", 9)
   CONNECTION(P, "B5", 4)
   CONNECTION("D6", 8, "B5", 5)
   CONNECTION(SERVE_WAIT, "B5", 14)

   CONNECTION(BTB_HIT_n, "E5", 1)
   CONNECTION(SBD_n, "E5", 2)

   CONNECTION(BP_HIT_n, "E5", 4)
   CONNECTION(BTB_HIT_n, "E5", 5)
   CONNECTION("E5", 6, "F7", 11)
   CONNECTION("E5", 6, "F7", 12)
   CONNECTION(BRICK_HIT_n, "F7", 10)
   CONNECTION("F7", 9, "C2", 2)
   CONNECTION(BALL_DISPLAY, "C2", 1)
   CONNECTION("L3", 6, "E3", 11)
   CONNECTION("C2", 3, "E3", 12)

   CONNECTION(SET_BRICKS_n, "E6", 4)
   CONNECTION("E3", 13, "E6", 2)
   CONNECTION(CKBH, "E6", 3)
   CONNECTION("E3", 13, "D2", 2)
   CONNECTION(SET_BRICKS, "D2", 1)
   CONNECTION("D2", 3, "E6", 1)

   CONNECTION(BRICK_DISPLAY, "E1", 1)
   CONNECTION(H1, "K4", 9)
   CONNECTION(H2, "K4", 10)
   CONNECTION("K4", 8, "E3", 2)
   CONNECTION("L3", 6, "E3", 3)

   CONNECTION(ATTRACT_n, "C2", 13)
   CONNECTION(SET_BRICKS_n, "C2", 12)
   CONNECTION("C2", 11, "H3", 10)
   CONNECTION(FPD1, "H3", 9)
   CONNECTION(FPD2, "H3", 11)
   CONNECTION("H3", 8, "E1", 3)
   CONNECTION("E1", 4, "C32", 1)
   CONNECTION("C32", 2, "L3", 13)
   CONNECTION(H4, "L3", 2)
   CONNECTION(H8, "L3", 1)
   CONNECTION(H16, "L3", 15)
   CONNECTION(V32, "L3", 14)
   CONNECTION(V64, "L3", 7)
   CONNECTION(V128, "L3", 9)
   CONNECTION(V16, "L3", 10)
   CONNECTION(RAM_PLAYER1, "L3", 11)
   CONNECTION(H32, "L3", 3)
   CONNECTION(H128, "L3", 4)
   CONNECTION("H4", 8, "L3", 5)

   CONNECTION(V2, "M4", 5)
   CONNECTION(V4, "M4", 4)
   CONNECTION(V8, "M4", 3)
   CONNECTION("M4", 6, "H4", 9)
   CONNECTION(VSYNC_n, "K4", 2)
   CONNECTION(H64, "K4", 1)
   CONNECTION("K4", 3, "H4", 10)
   CONNECTION(FPD1_n, "F2", 13)
   CONNECTION(BRICK_HIT_n, "F2", 2)
   CONNECTION(FPD2_n, "F2", 1)

   CONNECTION("F2", 12, "L3", 12)

   //FPD circuits
   CONNECTION(K2, "M4", 2)
   CONNECTION(G2, "M4", 13)
   CONNECTION(D2, "M4", 1)
   CONNECTION(K1, "M4", 9)
   CONNECTION(G1, "M4", 10)
   CONNECTION(D1, "M4", 11)
   CONNECTION(BP_HIT_n, "E4", 2)
   CONNECTION("M4", 12, "E4", 1)
   CONNECTION(BP_HIT_n, "E4", 4)
   CONNECTION("M4", 8, "E4", 5)

   CONNECTION(P, "F4", 4)
   CONNECTION(P, "F4", 2)
   CONNECTION("E4", 3, "F4", 3)
   CONNECTION(START_GAME1_n, "F4", 1)

   CONNECTION(P, "F4", 10)
   CONNECTION(P, "F4", 12)
   CONNECTION("E4", 6, "F4", 11)
   CONNECTION(START_GAME1_n, "F4", 13)

   CONNECTION("F4", 6, "F3", 5)
   CONNECTION(GND, "F3", 4)
   CONNECTION("F4", 8, "F3", 11)
   CONNECTION(GND, "F3", 12)

   CONNECTION(P, "F3", 3)
   CONNECTION(P, "F3", 13)


   //CREDIT_COUNTER
   CONNECTION(BONUS_COIN, "E7", 8)
   CONNECTION(COIN, "E7", 9)
   CONNECTION(CR_START1_n, "H7", 2)
   CONNECTION(V8, "D8", 1)
   CONNECTION(CR_START2, "D8", 2)
   CONNECTION("D8", 3, "H7", 1)

   CONNECTION("L8", 12, "L8", 11) // not on schematic, on rollover load 16, keeps you from losing all credits
   CONNECTION(P, "L8", 15)
   CONNECTION(P, "L8", 1)
   CONNECTION(P, "L8", 10)
   CONNECTION(P, "L8", 9)
   CONNECTION("E7", 10, "L8", 5)
   CONNECTION("H7", 3, "L8", 4)

   CONNECTION(LAT_Q, "L9", 10)
   CONNECTION("L8", 13, "L9", 9)
   CONNECTION("L9", 8, "L8", 14)
   CONNECTION("L8", 7, "M8", 9)
   CONNECTION("L8", 6, "M8", 10)
   CONNECTION("L8", 2, "M8", 11)
   CONNECTION("L8", 3, "M9", 13)

   CONNECTION(CREDIT2_n, "F9", 5)
   CONNECTION(CREDIT2_n, "L9", 2)
   CONNECTION("M9", 12, "L9", 1)
   CONNECTION(CREDIT_1_OR_2, "F9", 9)

   //PLGM2_n
   CONNECTION(CR_START1_n, "F7", 6)
   CONNECTION(CR_START2_n, "F7", 5)

   //PLAY_CP
   CONNECTION(PLGM2_n, "F2", 5)
   CONNECTION(PLAYER_2, "F2", 4)
   CONNECTION(H1, "F2", 3)

   CONNECTION(P, "F5", 4)
   CONNECTION(P, "F5", 2)
   CONNECTION(SERVE_WAIT, "F5", 3)
   CONNECTION(H128, "F5", 1)
   CONNECTION("F2", 6, "D2", 9)
   CONNECTION("F5", 5, "D2", 10)

   //EGL
   CONNECTION(P, "B4", 10)
   CONNECTION(P, "B4", 7)
   CONNECTION(P, "B4", 9)
   CONNECTION(PLAY_CP, "B4", 2)

   CONNECTION(EGL, "C5", 1)



   CONNECTION(START_GAME1_n, "B4", 1)
   CONNECTION(BALL_A, "A4", 2)
   CONNECTION(BALL_B, "S4", 1)
   CONNECTION(BALL_C, "S4", 2)
   CONNECTION("S4", 3, "A4", 1)
   CONNECTION("A4", 3, "C37", 1)

   CONNECTION(SERVE_WAIT_n, "A4", 5)
   CONNECTION(BALL, "A4", 4)

   // Ball Circuit


   // Video Summing
   CONNECTION(V16_d, "D2", 4)
   CONNECTION(V8_d, "D2", 5)
   CONNECTION("D2", 6, "E3", 8)
   CONNECTION(VSYNC_n, "E3", 9)
   CONNECTION(HSYNC_n, "E2", 12)
   CONNECTION("E3", 10, "E2", 13)
   CONNECTION(PSYNC, "B9", 1)
   CONNECTION(VSYNC_n, "B9", 2)

   //CONNECTION("VIDEO", 1, "E2", 11)
   CONNECTION("VIDEO", 2, PLAYFIELD)
   CONNECTION("VIDEO", 3, BSYNC)
   CONNECTION("VIDEO", 4, SCORE)
   CONNECTION("VIDEO", 5, "B9", 3)
   //CONNECTION("VIDEO", 6, P)

   CONNECTION("VIDEO", Video::HBLANK_PIN, HSYNC)
   CONNECTION("VIDEO", Video::VBLANK_PIN, "E3", 10)


   // Audio Summing
   CONNECTION("AUDIO", 1, "B9", 11)
   CONNECTION("AUDIO", 2, "B9", 8)
   CONNECTION("AUDIO", 3, FREE_GAME_TONE)
   CONNECTION("AUDIO", 4, "B9", 6)


#ifdef DEBUG
    // RAM access
    /*CONNECTION("LOG1", 3, H4)		//A
    CONNECTION("LOG1", 4, H8)		//B
    CONNECTION("LOG1", 5, H16)	//C
    CONNECTION("LOG1", 6, V32)		//D
    CONNECTION("LOG1", 7, V64)		//E
    CONNECTION("LOG1", 8, V128)	//F
    CONNECTION("LOG1", 9, V16)	//G
    CONNECTION("LOG1", 10, RAM_PLAYER1)		//H
    CONNECTION("LOG1", 11, H32)		//I
    CONNECTION("LOG1", 12, H128)		//J
    CONNECTION("LOG1", 13, "H4", 8)	//K
    CONNECTION("LOG1", 14, "E1", 4)	//L
    CONNECTION("LOG1", 15, "F2", 12)	//M
    CONNECTION("LOG1", 16, "L3", 6)*/	//N
#endif

CIRCUIT_LAYOUT_END

