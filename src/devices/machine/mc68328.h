// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

 Motorola 68328 ("DragonBall") System-on-a-Chip implementation

***********************************************************************

                                                             P P P P P P P   P P P P P P P
                                                             E E E E E E E   J J J J J J J
                                                             1 2 3 4 5 6 7   0 1 2 3 4 5 6
                   D   D D D D                               / / / / / / /   / / / / / / /
                   3   4 5 6 7                             ! ! ! ! ! ! ! !   ! ! ! ! ! ! !
                   /   / / / /                       ! !   C C C C C C C C   C C C C C C C
                   P V P P P P     D D G D D D D T T L U V S S S S S S S S G S S S S S S S
                   B C B B B B D D 1 1 N 1 1 1 1 M C W W C A A A A B B B B N C C C C D D D
                   3 C 4 5 6 7 8 9 0 1 D 2 3 4 5 S K E E C 0 1 2 3 0 1 2 3 D 0 1 2 3 0 1 2
                   | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
              +-------------------------------------------------------------------------------+
              |                                                                               |
              |                                                                               |
              |                                                                               |
              |                                                                               |
              |                                                                               |
      D2/PB2--|                                                                               |--PJ7/!CSD3
      D1/PB1--|                                                                               |--VCC
      D0/PB0--|                                                                               |--PD0/!KBD0/!INT0
         TDO--|                                                                               |--PD1/!KBD1/!INT1
         TDI--|                                                                               |--PD2/!KBD2/!INT2
         GND--|                                                                               |--PD3/!KBD3/!INT3
         !OE--|                                                                               |--PD4/!KBD4/!INT4
    !UDS/PC1--|                                                                               |--PD5/!KBD5/!INT5
         !AS--|                                                                               |--PD6/!KBD6/!INT6
          A0--|                                                                               |--PD7/!KBD7/!INT7
        !LDS--|                                                                               |--GND
        R/!W--|                                                                               |--LD0
  !DTACK/PC5--|                                                                               |--LD1
      !RESET--|                                                                               |--LD2
         VCC--|                                                                               |--LD3
     !WE/PC6--|                                                                               |--LFRM
    !JTAGRST--|                                                                               |--LLP
       BBUSW--|                                  MC68328PV                                    |--LCLK
          A1--|                                   TOP VIEW                                    |--LACD
          A2--|                                                                               |--VCC
          A3--|                                                                               |--PK0/SPMTXD0
          A4--|                                                                               |--PK1/SPMRXD0
          A5--|                                                                               |--PK2/SPMCLK0
          A6--|                                                                               |--PK3/SPSEN
         GND--|                                                                               |--PK4/SPSRXD1
          A7--|                                                                               |--PK5/SPSCLK1
          A8--|                                                                               |--PK6/!CE2
          A9--|                                                                               |--PK7/!CE1
         A10--|                                                                               |--GND
         A11--|                                                                               |--PM0/!CTS
         A12--|                                                                               |--PM1/!RTS
         A13--|                                                                               |--PM2/!IRQ6
         A14--|                                                                               |--PM3/!IRQ3
         VCC--|                                                                               |--PM4/!IRQ2
         A15--|                                                                               |--PM5/!IRQ1
     A16/PA0--|                                                                               |--PM6/!PENIRQ
              |                                                                               |
              |   _                                                                           |
              |  (_)                                                                          |
              |\                                                                              |
              | \                                                                             |
              +-------------------------------------------------------------------------------+
                   | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
                   P P P P P G P P P P P P P P V P P P P P P P P G P P P V C G P P P E X P
                   A A A A A N A A F F F F F F C F F G G G G G G N G G C C L N C M L X T L
                   1 2 3 4 5 D 6 7 0 1 2 3 4 5 C 6 7 7 6 5 4 3 2 D 1 0 0 C K D 4 7 L T A L
                   / / / / /   / / / / / / / /   / / / / / / / /   / / /   O   / / G A L V
                   A A A A A   A A A A A A A A   A A R T ! T ! P   R T M       ! U N L   C
                   1 1 1 2 2   2 2 2 2 2 2 2 2   3 3 T I T I T W   X X O       I A D     C
                   7 8 9 0 1   2 3 4 5 6 7 8 9   0 1 C N O N O M   D D C       R R
                                                     O 1 U 2 U O       L       Q T
                                                         T   T         K       7 G
                                                         1   2                   P
                                                                                 I
                                                                                 O

                   Figure 12-1. MC68328 144-Lead Plastic Thin-Quad Flat Pack Pin Assignment

                      Source: MC68328 (DragonBall)(tm) Integrated Processor User's Manual

 *****************************************************************************************************************/

#ifndef MAME_MACHINE_MC68328_H
#define MAME_MACHINE_MC68328_H

#include "cpu/m68000/m68000.h"


class mc68328_device : public m68000_device
{
public:
	mc68328_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_port_a() { return m_out_port_a_cb.bind(); }
	auto out_port_b() { return m_out_port_b_cb.bind(); }
	auto out_port_c() { return m_out_port_c_cb.bind(); }
	auto out_port_d() { return m_out_port_d_cb.bind(); }
	auto out_port_e() { return m_out_port_e_cb.bind(); }
	auto out_port_f() { return m_out_port_f_cb.bind(); }
	auto out_port_g() { return m_out_port_g_cb.bind(); }
	auto out_port_j() { return m_out_port_j_cb.bind(); }
	auto out_port_k() { return m_out_port_k_cb.bind(); }
	auto out_port_m() { return m_out_port_m_cb.bind(); }
	auto in_port_a() { return m_in_port_a_cb.bind(); }
	auto in_port_b() { return m_in_port_b_cb.bind(); }
	auto in_port_c() { return m_in_port_c_cb.bind(); }
	auto in_port_d() { return m_in_port_d_cb.bind(); }
	auto in_port_e() { return m_in_port_e_cb.bind(); }
	auto in_port_f() { return m_in_port_f_cb.bind(); }
	auto in_port_g() { return m_in_port_g_cb.bind(); }
	auto in_port_j() { return m_in_port_j_cb.bind(); }
	auto in_port_k() { return m_in_port_k_cb.bind(); }
	auto in_port_m() { return m_in_port_m_cb.bind(); }
	auto out_pwm() { return m_out_pwm_cb.bind(); }
	auto out_spim() { return m_out_spim_cb.bind(); }
	auto in_spim() { return m_in_spim_cb.bind(); }
	auto spim_xch_trigger() { return m_spim_xch_trigger_cb.bind(); }

	DECLARE_WRITE_LINE_MEMBER(set_penirq_line);
	void set_port_d_lines(uint8_t state, int bit);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	struct mc68328_regs_t
	{
		// $(FF)FFF000
		uint8_t   scr;        // System Control Register
		uint8_t   unused0[255];

		// $(FF)FFF100
		uint16_t  grpbasea;   // Chip Select Group A Base Register
		uint16_t  grpbaseb;   // Chip Select Group B Base Register
		uint16_t  grpbasec;   // Chip Select Group C Base Register
		uint16_t  grpbased;   // Chip Select Group D Base Register
		uint16_t  grpmaska;   // Chip Select Group A Mask Register
		uint16_t  grpmaskb;   // Chip Select Group B Mask Register
		uint16_t  grpmaskc;   // Chip Select Group C Mask Register
		uint16_t  grpmaskd;   // Chip Select Group D Mask Register
		uint32_t  csa0;       // Group A Chip Select 0 Register
		uint32_t  csa1;       // Group A Chip Select 1 Register
		uint32_t  csa2;       // Group A Chip Select 2 Register
		uint32_t  csa3;       // Group A Chip Select 3 Register
		uint32_t  csb0;       // Group B Chip Select 0 Register
		uint32_t  csb1;       // Group B Chip Select 1 Register
		uint32_t  csb2;       // Group B Chip Select 2 Register
		uint32_t  csb3;       // Group B Chip Select 3 Register
		uint32_t  csc0;       // Group C Chip Select 0 Register
		uint32_t  csc1;       // Group C Chip Select 1 Register
		uint32_t  csc2;       // Group C Chip Select 2 Register
		uint32_t  csc3;       // Group C Chip Select 3 Register
		uint32_t  csd0;       // Group D Chip Select 0 Register
		uint32_t  csd1;       // Group D Chip Select 1 Register
		uint32_t  csd2;       // Group D Chip Select 2 Register
		uint32_t  csd3;       // Group D Chip Select 3 Register
		uint8_t   unused1[176];

		// $(FF)FFF200
		uint16_t  pllcr;      // PLL Control Register
		uint16_t  pllfsr;     // PLL Frequency Select Register
		uint8_t   pad2[3];
		uint8_t   pctlr;      // Power Control Register
		uint8_t   unused3[248];

		// $(FF)FFF300
		uint8_t   ivr;        // Interrupt Vector Register
		uint8_t   unused4[1];
		uint16_t  icr;        // Interrupt Control Register
		uint32_t  imr;        // Interrupt Mask Register
		uint32_t  iwr;        // Interrupt Wakeup Enable Register
		uint32_t  isr;        // Interrupt Status Register
		uint32_t  ipr;        // Interrupt Pending Register
		uint8_t   unused5[236];

		// $(FF)FFF400
		uint8_t   padir;      // Port A Direction Register
		uint8_t   padata;     // Port A Data Register
		uint8_t   unused6[1];
		uint8_t   pasel;      // Port A Select Register
		uint8_t   unused7[4];

		uint8_t   pbdir;      // Port B Direction Register
		uint8_t   pbdata;     // Port B Data Register
		uint8_t   unused8[1];
		uint8_t   pbsel;      // Port B Select Register
		uint8_t   unused9[4];

		uint8_t   pcdir;      // Port C Direction Register
		uint8_t   pcdata;     // Port C Data Register
		uint8_t   unused10[1];
		uint8_t   pcsel;      // Port C Select Register
		uint8_t   unused11[4];

		uint8_t   pddir;      // Port D Direction Register
		uint8_t   pddata;     // Port D Data Register
		uint8_t   pdpuen;     // Port D Pullup Enable Register
		uint8_t   unused12[1];
		uint8_t   pdpol;      // Port D Polarity Register
		uint8_t   pdirqen;    // Port D IRQ Enable Register
		uint8_t   pddataedge; // Port D Data Edge Level
		uint8_t   pdirqedge;  // Port D IRQ Edge Register

		uint8_t   pedir;      // Port E Direction Register
		uint8_t   pedata;     // Port E Data Register
		uint8_t   pepuen;     // Port E Pullup Enable Register
		uint8_t   pesel;      // Port E Select Register
		uint8_t   unused14[4];

		uint8_t   pfdir;      // Port F Direction Register
		uint8_t   pfdata;     // Port F Data Register
		uint8_t   pfpuen;     // Port F Pullup Enable Register
		uint8_t   pfsel;      // Port F Select Register
		uint8_t   unused15[4];

		uint8_t   pgdir;      // Port G Direction Register
		uint8_t   pgdata;     // Port G Data Register
		uint8_t   pgpuen;     // Port G Pullup Enable Register
		uint8_t   pgsel;      // Port G Select Register
		uint8_t   unused16[4];

		uint8_t   pjdir;      // Port J Direction Register
		uint8_t   pjdata;     // Port J Data Register
		uint8_t   unused17[1];
		uint8_t   pjsel;      // Port J Select Register
		uint8_t   unused18[4];
		uint8_t   pkdir;      // Port K Direction Register
		uint8_t   pkdata;     // Port K Data Register
		uint8_t   pkpuen;     // Port K Pullup Enable Register
		uint8_t   pksel;      // Port K Select Register
		uint8_t   unused19[4];

		uint8_t   pmdir;      // Port M Direction Register
		uint8_t   pmdata;     // Port M Data Register
		uint8_t   pmpuen;     // Port M Pullup Enable Register
		uint8_t   pmsel;      // Port M Select Register
		uint8_t   unused20[180];

		// $(FF)FFF500
		uint16_t  pwmc;       // PWM Control Register
		uint16_t  pwmp;       // PWM Period Register
		uint16_t  pwmw;       // PWM Width Register
		uint16_t  pwmcnt;     // PWN Counter
		uint8_t   unused21[248];

		// $(FF)FFF600
		uint16_t  tctl[2];    // Timer Control Register
		uint16_t  tprer[2];   // Timer Prescaler Register
		uint16_t  tcmp[2];    // Timer Compare Register
		uint16_t  tcr[2];     // Timer Capture Register
		uint16_t  tcn[2];     // Timer Counter
		uint16_t  tstat[2];   // Timer Status
		uint16_t  wctlr;      // Watchdog Control Register
		uint16_t  wcmpr;      // Watchdog Compare Register
		uint16_t  wcn;        // Watchdog Counter
		uint8_t   tclear[2];  // Timer Clearable Status
		uint8_t   unused22[224];

		// $(FF)FFF700
		uint16_t  spisr;      // SPIS Register
		uint8_t   unused23[254];

		// $(FF)FFF800
		uint16_t  spimdata;   // SPIM Data Register
		uint16_t  spimcont;   // SPIM Control/Status Register
		uint8_t   unused24[252];

		// $(FF)FFF900
		uint16_t  ustcnt;     // UART Status/Control Register
		uint16_t  ubaud;      // UART Baud Control Register
		uint16_t  urx;        // UART RX Register
		uint16_t  utx;        // UART TX Register
		uint16_t  umisc;      // UART Misc Register
		uint8_t   unused25[246];

		// $(FF)FFFA00
		uint32_t  lssa;       // Screen Starting Address Register
		uint8_t   unused26[1];
		uint8_t   lvpw;       // Virtual Page Width Register
		uint8_t   unused27[2];
		uint16_t  lxmax;      // Screen Width Register
		uint16_t  lymax;      // Screen Height Register
		uint8_t   unused28[12];
		uint16_t  lcxp;       // Cursor X Position
		uint16_t  lcyp;       // Cursor Y Position
		uint16_t  lcwch;      // Cursor Width & Height Register
		uint8_t   unused29[1];
		uint8_t   lblkc;      // Blink Control Register
		uint8_t   lpicf;      // Panel Interface Config Register
		uint8_t   lpolcf;     // Polarity Config Register
		uint8_t   unused30[1];
		uint8_t   lacdrc;     // ACD (M) Rate Control Register
		uint8_t   unused31[1];
		uint8_t   lpxcd;      // Pixel Clock Divider Register
		uint8_t   unused32[1];
		uint8_t   lckcon;     // Clocking Control Register
		uint8_t   unused33[1];
		uint8_t   llbar;      // Last Buffer Address Register
		uint8_t   unused34[1];
		uint8_t   lotcr;      // Octet Terminal Count Register
		uint8_t   unused35[1];
		uint8_t   lposr;      // Panning Offset Register
		uint8_t   unused36[3];
		uint8_t   lfrcm;      // Frame Rate Control Modulation Register
		uint16_t  lgpmr;      // Gray Palette Mapping Register
		uint8_t   unused37[204];

		// $(FF)FFFB00
		uint32_t  hmsr;       // RTC Hours Minutes Seconds Register
		uint32_t  alarm;      // RTC Alarm Register
		uint8_t   unused38[4];
		uint16_t  rtcctl;     // RTC Control Register
		uint16_t  rtcisr;     // RTC Interrupt Status Register
		uint16_t  rtcienr;    // RTC Interrupt Enable Register
		uint16_t  stpwtch;    // Stopwatch Minutes
		uint8_t   unused42[1260];
	};

	void internal_map(address_map &map);

	// internal state
	void set_interrupt_line(uint32_t line, uint32_t active);
	void poll_port_d_interrupts();
	void cpu_space_map(address_map &map);
	uint8_t irq_callback(offs_t offset);
	uint32_t get_timer_frequency(uint32_t index);
	void maybe_start_timer(uint32_t index, uint32_t new_enable);
	void timer_compare_event(uint32_t index);

	void register_state_save();

	TIMER_CALLBACK_MEMBER(timer1_hit);
	TIMER_CALLBACK_MEMBER(timer2_hit);
	TIMER_CALLBACK_MEMBER(pwm_transition);
	TIMER_CALLBACK_MEMBER(rtc_tick);

	void internal_write(offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t internal_read(offs_t offset, uint16_t mem_mask = 0xffff);

	mc68328_regs_t m_regs;

	emu_timer *m_gptimer[2];
	emu_timer *m_rtc;
	emu_timer *m_pwm;

	devcb_write8  m_out_port_a_cb;    /* 8-bit output */
	devcb_write8  m_out_port_b_cb;    /* 8-bit output */
	devcb_write8  m_out_port_c_cb;    /* 8-bit output */
	devcb_write8  m_out_port_d_cb;    /* 8-bit output */
	devcb_write8  m_out_port_e_cb;    /* 8-bit output */
	devcb_write8  m_out_port_f_cb;    /* 8-bit output */
	devcb_write8  m_out_port_g_cb;    /* 8-bit output */
	devcb_write8  m_out_port_j_cb;    /* 8-bit output */
	devcb_write8  m_out_port_k_cb;    /* 8-bit output */
	devcb_write8  m_out_port_m_cb;    /* 8-bit output */

	devcb_read8   m_in_port_a_cb;     /* 8-bit input */
	devcb_read8   m_in_port_b_cb;     /* 8-bit input */
	devcb_read8   m_in_port_c_cb;     /* 8-bit input */
	devcb_read8   m_in_port_d_cb;     /* 8-bit input */
	devcb_read8   m_in_port_e_cb;     /* 8-bit input */
	devcb_read8   m_in_port_f_cb;     /* 8-bit input */
	devcb_read8   m_in_port_g_cb;     /* 8-bit input */
	devcb_read8   m_in_port_j_cb;     /* 8-bit input */
	devcb_read8   m_in_port_k_cb;     /* 8-bit input */
	devcb_read8   m_in_port_m_cb;     /* 8-bit input */

	devcb_write8  m_out_pwm_cb;       /* 1-bit output */

	devcb_write16 m_out_spim_cb;      /* 16-bit output */
	devcb_read16  m_in_spim_cb;       /* 16-bit input */

	devcb_write_line m_spim_xch_trigger_cb;    /* SPIM exchange trigger */ /*todo: not really a write line, fix*/
};


DECLARE_DEVICE_TYPE(MC68328, mc68328_device)

#endif // MAME_MACHINE_MC68328_H
