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


class mc68328_base_device : public m68000_device
{
public:
	typedef device_delegate<void (double, int, int, u8, u8)> lcd_info_changed_delegate;

	template <int Line> auto out_port_a() { return m_out_port_a_cb[Line].bind(); }
	template <int Line> auto out_port_b() { return m_out_port_b_cb[Line].bind(); }
	template <int Line> auto out_port_c() { return m_out_port_c_cb[Line].bind(); }
	template <int Line> auto out_port_d() { return m_out_port_d_cb[Line].bind(); }
	template <int Line> auto out_port_e() { return m_out_port_e_cb[Line].bind(); }
	template <int Line> auto out_port_f() { return m_out_port_f_cb[Line].bind(); }
	template <int Line> auto out_port_g() { return m_out_port_g_cb[Line].bind(); }
	template <int Line> auto in_port_a() { return m_in_port_a_cb[Line].bind(); }
	template <int Line> auto in_port_b() { return m_in_port_b_cb[Line].bind(); }
	template <int Line> auto in_port_c() { return m_in_port_c_cb[Line].bind(); }
	template <int Line> auto in_port_d() { return m_in_port_d_cb[Line].bind(); }
	template <int Line> auto in_port_e() { return m_in_port_e_cb[Line].bind(); }
	template <int Line> auto in_port_f() { return m_in_port_f_cb[Line].bind(); }
	template <int Line> auto in_port_g() { return m_in_port_g_cb[Line].bind(); }

	void port_d_in_w(int state, int bit);

	auto out_spim() { return m_out_spim_cb.bind(); }
	auto in_spim() { return m_in_spim_cb.bind(); }

	auto out_pwm() { return m_out_pwm_cb.bind(); }

	auto out_flm() { return m_out_flm_cb.bind(); }
	auto out_llp() { return m_out_llp_cb.bind(); }
	auto out_lsclk() { return m_out_lsclk_cb.bind(); }
	auto out_ld() { return m_out_ld_cb.bind(); }

	template <typename... T>
	void set_lcd_info_changed(T &&... args)
	{
		m_lcd_info_changed_cb.set(std::forward<T>(args)...);
	}

	void irq5_w(int state);

protected:
	mc68328_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	enum : u8
	{
		SCR_WDTH8               = 0x01,
		SCR_DMAP                = 0x04,
		SCR_SO                  = 0x08,
		SCR_BETEN               = 0x10,
		SCR_PRV                 = 0x20,
		SCR_WPV                 = 0x40,
		SCR_BETO                = 0x80,

		PCTLR_WIDTH             = 0x1f,
		PCTLR_STOP              = 0x40,
		PCTLR_PC_EN             = 0x80,

		BLKC_BD                 = 0x7f,
		BLKC_BKEN               = 0x80,

		LPOLCF_PIXPOL           = 0x01,
		LPOLCF_LPPOL_BIT        = 1,
		LPOLCF_FLMPOL_BIT       = 2,
		LPOLCF_LCKPOL           = 0x08,

		LACDRC_MASK             = 0x0f,

		LPXCD_MASK              = 0x3f,

		LBAR_MASK               = 0x7f,

		LPOSR_POS               = 0x07,
		LPOSR_BOS               = 0x08,

		LFRCM_YMOD              = 0x0f,
		LFRCM_XMOD              = 0xf0,
	};

	enum : u16
	{
		GRPBASE_VALID           = 0x0001,
		GRPBASE_BASE_ADDR       = 0xfff0,

		GRPMASK_BASE_MASK       = 0xfff0,

		PLLCR_DISPLL            = 0x0008,
		PLLCR_CLKEN             = 0x0010,
		PLLCR_SYSCLK_SEL_DIV2   = 0x0000,
		PLLCR_SYSCLK_SEL_DIV4   = 0x0100,
		PLLCR_SYSCLK_SEL_DIV8   = 0x0200,
		PLLCR_SYSCLK_SEL_DIV16  = 0x0300,
		PLLCR_SYSCLK_SEL_DIV1_0 = 0x0400,
		PLLCR_SYSCLK_SEL_DIV1_1 = 0x0500,
		PLLCR_SYSCLK_SEL_DIV1_2 = 0x0600,
		PLLCR_SYSCLK_SEL_DIV1_3 = 0x0700,
		PLLCR_SYSCLK_SEL        = 0x0700,
		PLLCR_SYSCLK_SHIFT      = 8,
		PLLCR_PIXCLK_SEL_DIV2   = 0x0000,
		PLLCR_PIXCLK_SEL_DIV4   = 0x0800,
		PLLCR_PIXCLK_SEL_DIV8   = 0x1000,
		PLLCR_PIXCLK_SEL_DIV16  = 0x1800,
		PLLCR_PIXCLK_SEL_DIV1_0 = 0x2000,
		PLLCR_PIXCLK_SEL_DIV1_1 = 0x2800,
		PLLCR_PIXCLK_SEL_DIV1_2 = 0x3000,
		PLLCR_PIXCLK_SEL_DIV1_3 = 0x3800,
		PLLCR_PIXCLK_SEL        = 0x3800,
		PLLCR_PIXCLK_SHIFT      = 11,

		PLLFSR_PCNT             = 0x00ff,
		PLLFSR_QCNT             = 0x0f00,
		PLLFSR_PROT             = 0x4000,
		PLLFSR_CLK32            = 0x8000,

		ICR_ET6                 = 0x0100,
		ICR_ET3                 = 0x0200,
		ICR_ET2                 = 0x0400,
		ICR_ET1                 = 0x0800,
		ICR_POL6                = 0x1000,
		ICR_POL3                = 0x2000,
		ICR_POL2                = 0x4000,
		ICR_POL1                = 0x8000,

		PWMC_CLKSEL             = 0x0007,
		PWMC_EN                 = 0x0010,
		PWMC_POL                = 0x0040,
		PWMC_PIN                = 0x0080,
		PWMC_LOAD               = 0x0100,
		PWMC_IRQ_EN             = 0x4000,
		PWMC_IRQ                = 0x8000,

		TCTL_TEN                = 0x0001,
		TCTL_TEN_ENABLE         = 0x0001,
		TCTL_TEN_BIT            = 0,
		TCTL_CLKSOURCE          = 0x000e,
		TCTL_CLKSOURCE_STOP     = 0x0000,
		TCTL_CLKSOURCE_SYSCLK   = 0x0002,
		TCTL_CLKSOURCE_SYSCLK16 = 0x0004,
		TCTL_CLKSOURCE_TIN      = 0x0006,
		TCTL_CLKSOURCE_32KHZ4   = 0x0008,
		TCTL_CLKSOURCE_32KHZ5   = 0x000a,
		TCTL_CLKSOURCE_32KHZ6   = 0x000c,
		TCTL_CLKSOURCE_32KHZ7   = 0x000e,
		TCTL_IRQEN              = 0x0010,
		TCTL_IRQEN_ENABLE       = 0x0010,
		TCTL_OM                 = 0x0020,
		TCTL_OM_ACTIVELOW       = 0x0000,
		TCTL_OM_TOGGLE          = 0x0020,
		TCTL_CAPTURE            = 0x00c0,
		TCTL_CAPTURE_NOINT      = 0x0000,
		TCTL_CAPTURE_RISING     = 0x0040,
		TCTL_CAPTURE_FALLING    = 0x0080,
		TCTL_CAPTURE_BOTH       = 0x00c0,
		TCTL_FRR                = 0x0100,
		TCTL_FRR_RESTART        = 0x0000,
		TCTL_FRR_FREERUN        = 0x0100,

		TSTAT_COMP              = 0x0001,
		TSTAT_CAPT              = 0x0002,

		SPIS_SPISEN             = 0x0100,
		SPIS_POL                = 0x0200,
		SPIS_PHA                = 0x0400,
		SPIS_OVRWR              = 0x0800,
		SPIS_DATA_RDY           = 0x1000,
		SPIS_ENPOL              = 0x2000,
		SPIS_IRQEN              = 0x4000,
		SPIS_SPIS_IRQ           = 0x8000,

		SPIM_BIT_COUNT          = 0x000f,
		SPIM_POL_BIT            = 4,
		SPIM_PHA_BIT            = 5,
		SPIM_IRQEN_BIT          = 6,
		SPIM_SPIMIRQ_BIT        = 7,
		SPIM_XCH_BIT            = 8,
		SPIM_SPMEN_BIT          = 9,
		SPIM_RATE_MASK          = 0xe000,
		SPIM_RATE_SHIFT         = 13,

		USTCNT_TX_AVAIL_EN      = 0x0001,
		USTCNT_TX_HALF_EN       = 0x0002,
		USTCNT_TX_EMPTY_EN      = 0x0004,
		USTCNT_RX_RDY_EN        = 0x0008,
		USTCNT_RX_HALF_EN       = 0x0010,
		USTCNT_RX_FULL_EN       = 0x0020,
		USTCNT_CTS_DELTA_EN     = 0x0040,
		USTCNT_GPIO_DELTA_EN    = 0x0080,
		USTCNT_8_7              = 0x0100,
		USTCNT_STOP_BITS        = 0x0200,
		USTCNT_ODD_EVEN         = 0x0400,
		USTCNT_PARITY_EN        = 0x0800,
		USTCNT_RX_CLK_CONT      = 0x1000,
		USTCNT_TX_EN            = 0x2000,
		USTCNT_RX_EN            = 0x4000,
		USTCNT_UART_EN          = 0x8000,

		UBAUD_PRESCALER         = 0x00ff,
		UBAUD_DIVIDE            = 0x0700,
		UBAUD_DIVIDE_1          = 0x0000,
		UBAUD_DIVIDE_2          = 0x0100,
		UBAUD_DIVIDE_4          = 0x0200,
		UBAUD_DIVIDE_8          = 0x0300,
		UBAUD_DIVIDE_16         = 0x0400,
		UBAUD_DIVIDE_32         = 0x0500,
		UBAUD_DIVIDE_64         = 0x0600,
		UBAUD_DIVIDE_128        = 0x0700,
		UBAUD_BAUD_SRC          = 0x0800,
		UBAUD_GPIO_SRC          = 0x1000,
		UBAUD_GPIO_DIR          = 0x2000,
		UBAUD_GPIO              = 0x4000,
		UBAUD_GPIO_DELTA        = 0x8000,

		URX_PARITY_ERROR        = 0x0100,
		URX_BREAK               = 0x0200,
		URX_FRAME_ERROR         = 0x0400,
		URX_OVRUN               = 0x0800,
		URX_DATA_READY          = 0x2000,
		URX_FIFO_HALF           = 0x4000,
		URX_FIFO_FULL           = 0x8000,

		UTX_CTS_DELTA           = 0x0100,
		UTX_CTS_STATUS          = 0x0200,
		UTX_IGNORE_CTS          = 0x0800,
		UTX_SEND_BREAK          = 0x1000,
		UTX_TX_AVAIL            = 0x2000,
		UTX_FIFO_HALF           = 0x4000,
		UTX_FIFO_EMPTY          = 0x8000,

		UMISC_IRDA_LOOP         = 0x0010,
		UMISC_IRDA_ENABLE       = 0x0020,
		UMISC_RTS               = 0x0040,
		UMISC_RTS_CONT          = 0x0080,
		UMISC_LOOP              = 0x1000,
		UMISC_FORCE_PERR        = 0x2000,
		UMISC_CLK_SRC           = 0x4000,

		CXP_MASK                = 0x03ff,
		CXP_CC                  = 0xc000,
		CXP_CC_XLU              = 0x0000,
		CXP_CC_BLACK            = 0x4000,
		CXP_CC_INVERSE          = 0x8000,
		CXP_CC_INVALID          = 0xc000,

		CYP_MASK                = 0x01ff,

		CWCH_CH                 = 0x001f,
		CWCH_CW                 = 0x1f00,

		LXMAX_MASK              = 0x03ff,

		LYMAX_MASK              = 0x03ff,

		RTCCTL_38_4_BIT         = 5,
		RTCCTL_ENABLE_BIT       = 7,
		RTCCTL_MASK             = 0x00a0,

		RTCINT_STOPWATCH        = 0x0001,
		RTCINT_MINUTE           = 0x0002,
		RTCINT_ALARM            = 0x0004,
		RTCINT_DAY              = 0x0008,
		RTCINT_SECOND           = 0x0010,

		RTCSTPWTCH_MASK         = 0x003f,
	};

	enum : u32
	{
		CSAB_WAIT               = 0x00000007,
		CSAB_RO                 = 0x00000008,
		CSAB_MASK               = 0x0000ff00,
		CSAB_BSW                = 0x00010000,
		CSAB_COMPARE            = 0xff000000,

		CSCD_WAIT               = 0x00000007,
		CSCD_RO                 = 0x00000008,
		CSCD_MASK               = 0x0000fff0,
		CSCD_BSW                = 0x00010000,
		CSCD_COMPARE            = 0xfff00000,

		INT_SPIM                = 0,
		INT_SPIM_MASK           = (1 << INT_SPIM),
		INT_TIMER2              = 1,
		INT_TIMER2_MASK         = (1 << INT_TIMER2),
		INT_UART                = 2,
		INT_UART_MASK           = (1 << INT_UART),
		INT_WDT                 = 3,
		INT_WDT_MASK            = (1 << INT_WDT),
		INT_RTC                 = 4,
		INT_RTC_MASK            = (1 << INT_RTC),
		INT_KB                  = 6,
		INT_KB_MASK             = (1 << INT_KB),
		INT_PWM                 = 7,
		INT_PWM_MASK            = (1 << INT_PWM),
		INT_INT0                = 8,
		INT_INT0_MASK           = (1 << INT_INT0),
		INT_INT1                = 9,
		INT_INT1_MASK           = (1 << INT_INT1),
		INT_INT2                = 10,
		INT_INT2_MASK           = (1 << INT_INT2),
		INT_INT3                = 11,
		INT_INT3_MASK           = (1 << INT_INT3),
		INT_INT4                = 12,
		INT_INT4_MASK           = (1 << INT_INT4),
		INT_INT5                = 13,
		INT_INT5_MASK           = (1 << INT_INT5),
		INT_INT6                = 14,
		INT_INT6_MASK           = (1 << INT_INT6),
		INT_INT7                = 15,
		INT_INT7_MASK           = (1 << INT_INT7),
		INT_KBDINTS             = 8,
		INT_KBDINTS_MASK        = (1 << INT_KBDINTS),
		INT_IRQ1                = 16,
		INT_IRQ1_MASK           = (1 << INT_IRQ1),
		INT_IRQ2                = 17,
		INT_IRQ2_MASK           = (1 << INT_IRQ2),
		INT_IRQ3                = 18,
		INT_IRQ3_MASK           = (1 << INT_IRQ3),
		INT_IRQ6                = 19,
		INT_IRQ6_MASK           = (1 << INT_IRQ6),
		INT_IRQ5                = 20,
		INT_IRQ5_MASK           = (1 << INT_IRQ5),
		INT_SPIS                = 21,
		INT_SPIS_MASK           = (1 << INT_SPIS),
		INT_TIMER1              = 22,
		INT_TIMER1_MASK         = (1 << INT_TIMER1),
		INT_IRQ7                = 23,
		INT_IRQ7_MASK           = (1 << INT_IRQ7),

		RTCHMSR_SECONDS         = 0x0000003f,
		RTCHMSR_SECONDS_SHIFT   = 0,
		RTCHMSR_MINUTES         = 0x003f0000,
		RTCHMSR_MINUTES_SHIFT   = 16,
		RTCHMSR_HOURS           = 0x1f000000,
		RTCHMSR_HOURS_SHIFT     = 24,
	};

	virtual void scr_w(u8 data);

	void grpbasea_w(u16 data);
	void grpbaseb_w(u16 data);
	void grpbasec_w(u16 data);
	void grpbased_w(u16 data);

	void pllcr_w(u16 data);
	void pllfsr_w(u16 data);
	void pctlr_w(u8 data);
	void ivr_w(u8 data);
	void icr_w(u8 data);
	void imr_msw_w(offs_t offset, u16 data, u16 mem_mask);
	void imr_lsw_w(offs_t offset, u16 data, u16 mem_mask);
	void isr_msw_w(offs_t offset, u16 data, u16 mem_mask);
	void isr_lsw_w(offs_t offset, u16 data, u16 mem_mask);
	void ipr_msw_w(offs_t offset, u16 data, u16 mem_mask);
	void ipr_lsw_w(offs_t offset, u16 data, u16 mem_mask);

	void padir_w(u8 data);
	void padata_w(u8 data);
	void pbdir_w(u8 data);
	void pbdata_w(u8 data);
	void pbsel_w(u8 data);
	void pcdir_w(u8 data);
	void pcdata_w(u8 data);
	void pcsel_w(u8 data);
	void pddir_w(u8 data);
	void pddata_w(u8 data);
	void pdpuen_w(u8 data);
	void pdpol_w(u8 data);
	void pdirqen_w(u8 data);
	void pdirqedge_w(u8 data);
	void pedir_w(u8 data);
	void pedata_w(u8 data);
	void pepuen_w(u8 data);
	void pesel_w(u8 data);
	void pfdir_w(u8 data);
	void pfdata_w(u8 data);
	void pfpuen_w(u8 data);
	void pfsel_w(u8 data);
	void pgdir_w(u8 data);
	void pgdata_w(u8 data);
	void pgpuen_w(u8 data);
	void pgsel_w(u8 data);

	virtual void pwmc_w(u16 data) = 0;

	template <int Timer> void tctl_w(u16 data);
	template <int Timer> void tprer_w(u16 data);
	template <int Timer> void tcmp_w(u16 data);
	template <int Timer> void tcr_w(u16 data);
	template <int Timer> void tcn_w(u16 data);
	template <int Timer> void tstat_w(u16 data);

	void spimdata_w(u16 data);
	void spimcont_w(u16 data);

	void ustcnt_w(u16 data);
	void ubaud_w(u16 data);
	void urx_w(u16 data);
	void utx_w(u16 data);
	void umisc_w(u16 data);

	void lssa_msw_w(offs_t offset, u16 data, u16 mem_mask);
	void lssa_lsw_w(offs_t offset, u16 data, u16 mem_mask);
	void lvpw_w(u8 data);
	void lxmax_w(u16 data);
	void lymax_w(u16 data);
	void lcxp_w(u16 data);
	void lcyp_w(u16 data);
	void lcwch_w(u16 data);
	void lblkc_w(u8 data);
	virtual void lpicf_w(u8 data) = 0;
	void lpolcf_w(u8 data);
	void lacdrc_w(u8 data);
	void lpxcd_w(u8 data);
	virtual void lckcon_w(u8 data) = 0;
	void lposr_w(u8 data);
	void lfrcm_w(u8 data);

	void hmsr_msw_w(offs_t offset, u16 data, u16 mem_mask);
	void hmsr_lsw_w(offs_t offset, u16 data, u16 mem_mask);
	void alarm_msw_w(offs_t offset, u16 data, u16 mem_mask);
	void alarm_lsw_w(offs_t offset, u16 data, u16 mem_mask);
	virtual void rtcctl_w(offs_t offset, u16 data, u16 mem_mask);
	void rtcisr_w(offs_t offset, u16 data, u16 mem_mask);
	void rtcienr_w(offs_t offset, u16 data, u16 mem_mask);
	void stpwtch_w(offs_t offset, u16 data, u16 mem_mask);

	u8  scr_r();

	u16 grpbasea_r();
	u16 grpbaseb_r();
	u16 grpbasec_r();
	u16 grpbased_r();

	u16 pllcr_r();
	u16 pllfsr_r();
	u8  pctlr_r();

	u8  ivr_r();
	u16 icr_r();
	u16 imr_msw_r();
	u16 imr_lsw_r();
	u16 isr_msw_r();
	u16 isr_lsw_r();
	u16 ipr_msw_r();
	u16 ipr_lsw_r();

	u8  padir_r();
	u8  padata_r();
	u8  pbdir_r();
	u8  pbdata_r();
	u8  pbsel_r();
	u8  pcdir_r();
	u8  pcdata_r();
	u8  pcsel_r();
	u8  pddir_r();
	u8  pddata_r();
	u8  pdpuen_r();
	u8  pdpol_r();
	u8  pdirqen_r();
	u8  pdirqedge_r();
	u8  pedir_r();
	u8  pedata_r();
	u8  pepuen_r();
	u8  pesel_r();
	u8  pfdir_r();
	u8  pfdata_r();
	u8  pfpuen_r();
	u8  pfsel_r();
	u8  pgdir_r();
	u8  pgdata_r();
	u8  pgpuen_r();
	u8  pgsel_r();

	virtual u16 pwmc_r() = 0;

	template <int Timer> u16 tctl_r();
	template <int Timer> u16 tprer_r();
	template <int Timer> u16 tcmp_r();
	template <int Timer> u16 tcr_r();
	template <int Timer> u16 tcn_r();
	template <int Timer> u16 tstat_r();

	u16 spimdata_r();
	u16 spimcont_r();

	u16 ustcnt_r();
	u16 ubaud_r();
	u16 urx_r();
	u16 utx_r();
	u16 umisc_r();

	u16 lssa_msw_r();
	u16 lssa_lsw_r();
	u8  lvpw_r();
	u16 lxmax_r();
	u16 lymax_r();
	u16 lcxp_r();
	u16 lcyp_r();
	u16 lcwch_r();
	u8  lblkc_r();
	u8  lpicf_r();
	u8  lpolcf_r();
	u8  lacdrc_r();
	u8  lpxcd_r();
	u8  lckcon_r();
	u8  lposr_r();
	u8  lfrcm_r();

	u16 hmsr_msw_r();
	u16 hmsr_lsw_r();
	u16 alarm_msw_r();
	u16 alarm_lsw_r();
	u16 rtcctl_r();
	u16 rtcisr_r();
	u16 rtcienr_r();
	u16 stpwtch_r();

	// $(FF)FFF000
	u8   m_scr;         // System Control Register

	// $(FF)FFF100
	u16  m_grpbasea;    // Chip Select Group A Base Register
	u16  m_grpbaseb;    // Chip Select Group B Base Register
	u16  m_grpbasec;    // Chip Select Group C Base Register
	u16  m_grpbased;    // Chip Select Group D Base Register

	// $(FF)FFF200
	u16  m_pllcr;       // PLL Control Register
	u16  m_pllfsr;      // PLL Frequency Select Register
	u8   m_pctlr;       // Power Control Register

	// $(FF)FFF300
	u8   m_ivr;         // Interrupt Vector Register
	u16  m_icr;         // Interrupt Control Register
	u32  m_imr;         // Interrupt Mask Register
	u32  m_gisr;        // (global) Interrupt Status Register
	u32  m_ipr;         // Interrupt Pending Register

	// $(FF)FFF400
	u8   m_padir;       // Port A Direction Register
	u8   m_padata;      // Port A Data Register
	u8   m_pasel;       // Port A Select Register

	u8   m_pbdir;       // Port B Direction Register
	u8   m_pbdata;      // Port B Data Register
	u8   m_pbsel;       // Port B Select Register

	u8   m_pcdir;       // Port C Direction Register
	u8   m_pcdata;      // Port C Data Register
	u8   m_pcsel;       // Port C Select Register

	u8   m_pddir;       // Port D Direction Register
	u8   m_pddata;      // Port D Data Register
	u8   m_pdpuen;      // Port D Pullup Enable Register
	u8   m_pdpol;       // Port D Polarity Register
	u8   m_pdirqen;     // Port D IRQ Enable Register
	u8   m_pdirqedge;   // Port D IRQ Edge Register
	u8   m_pdindata;    // Port D direct input data (not memory-mapped)

	u8   m_pedir;       // Port E Direction Register
	u8   m_pedata;      // Port E Data Register
	u8   m_pepuen;      // Port E Pullup Enable Register
	u8   m_pesel;       // Port E Select Register

	u8   m_pfdir;       // Port F Direction Register
	u8   m_pfdata;      // Port F Data Register
	u8   m_pfpuen;      // Port F Pullup Enable Register
	u8   m_pfsel;       // Port F Select Register

	u8   m_pgdir;       // Port G Direction Register
	u8   m_pgdata;      // Port G Data Register
	u8   m_pgpuen;      // Port G Pullup Enable Register
	u8   m_pgsel;       // Port G Select Register

	// $(FF)FFF500
	u16  m_pwmc;        // PWM Control Register
	bool m_pwmo;

	// $(FF)FFF600
	struct timer_regs
	{
		u16  tctl;      // Timer Control Register
		u16  tprer;     // Timer Prescaler Register
		u16  tcmp;      // Timer Compare Register
		u16  tcr;       // Timer Capture Register
		u16  tcn;       // Timer Counter
		u16  tstat;     // Timer Status
		u16  tclear;    // Timer Clearable Status
	};

	// $(FF)FFF800
	u16  m_spimdata;    // SPIM Data Register
	u16  m_spimcont;    // SPIM Control/Status Register
	bool m_spmtxd;      // SPIM Shift-register output (TODO: multiplex onto Port K)
	bool m_spmrxd;      // SPIM Shift-register input  (TODO: multiplex onto Port K)
	bool m_spmclk;      // SPIM Shift-register clock  (TODO: multiplex onto Port K)
	u16  m_spim_bit_read_idx; // Starting bit index for SPI transfers

	// $(FF)FFF900
	u16  m_ustcnt;      // UART Status/Control Register
	u16  m_ubaud;       // UART Baud Control Register
	u16  m_urx;         // UART RX Register
	u16  m_utx;         // UART TX Register
	u16  m_umisc;       // UART Misc Register

	// $(FF)FFFA00
	u32  m_lssa;        // Screen Starting Address Register
	u32  m_lssa_end;    // Screen Starting Address Register, buffer end address (not memory-mapped)
	u8   m_lvpw;        // Virtual Page Width Register
	u16  m_lxmax;       // Screen Width Register
	u16  m_lymax;       // Screen Height Register
	u16  m_lcxp;        // Cursor X Position
	u16  m_lcyp;        // Cursor Y Position
	u16  m_lcwch;       // Cursor Width & Height Register
	u8   m_lblkc;       // Blink Control Register
	u8   m_lpicf;       // Panel Interface Config Register
	u8   m_lpolcf;      // Polarity Config Register
	u8   m_lacdrc;      // ACD (M) Rate Control Register
	u8   m_lpxcd;       // Pixel Clock Divider Register
	u8   m_lckcon;      // Clocking Control Register
	u8   m_lposr;       // Panning Offset Register
	u8   m_lfrcm;       // Frame Rate Control Modulation Register
	bool m_lcd_update_pending;

	// $(FF)FFFB00
	u32  m_hmsr;        // RTC Hours Minutes Seconds Register
	u32  m_alarm;       // RTC Alarm Register
	u16  m_rtcctl;      // RTC Control Register
	u16  m_rtcisr;      // RTC Interrupt Status Register
	u16  m_rtcienr;     // RTC Interrupt Enable Register
	u16  m_stpwtch;     // Stopwatch Minutes

	void base_internal_map(u32 addr_bits, address_map &map);

	// internal state
	u8 irq_callback(offs_t offset);
	virtual void register_state_save();

	void update_ipr_state(u32 changed_mask);
	void update_imr_state(u32 changed_mask);
	void set_interrupt_line(u32 line, u32 active);
	virtual u32 get_irq_mask_for_level(int level) = 0;
	virtual int get_irq_level_for_mask(u32 level) = 0;

	virtual emu_timer *get_timer(int timer) = 0;
	virtual timer_regs &get_timer_regs(int timer) = 0;
	virtual u32 get_timer_int(int timer) = 0;
	template<int Timer> u32 get_timer_frequency();
	template<int Timer> void update_gptimer_state();
	template<int Timer> TIMER_CALLBACK_MEMBER(timer_tick);

	virtual void lcd_update_info() = 0;
	virtual u16 lcd_get_lxmax_mask() = 0;
	virtual int lcd_get_width() = 0;
	virtual u32 lcd_get_line_word_count() = 0;
	virtual attotime lcd_get_line_rate() = 0;
	virtual u8 lcd_get_panel_bit_size() = 0;
	virtual attotime get_pixclk_rate() = 0;

	TIMER_CALLBACK_MEMBER(refclk_tick);
	virtual TIMER_CALLBACK_MEMBER(pwm_tick) = 0;
	TIMER_CALLBACK_MEMBER(rtc_tick);
	TIMER_CALLBACK_MEMBER(spim_tick);
	TIMER_CALLBACK_MEMBER(lcd_scan_tick);
	void fill_lcd_dma_buffer();

	virtual bool rtc_int_is_active() = 0;
	virtual void rtc_advance_seconds();
	virtual u16 rtc_get_int_mask() = 0;
	virtual bool rtc_get_alarm_match() = 0;

	emu_timer *m_refclk;
	emu_timer *m_pwm;
	emu_timer *m_rtc;
	emu_timer *m_spim;

	emu_timer *m_lcd_scan;
	bool m_lcd_first_line;
	u32  m_lcd_sysmem_ptr;
	u32  m_lcd_line_bit;
	u32  m_lcd_line_word;
	bool m_lsclk;
	std::unique_ptr<u16[]> m_lcd_line_buffer;

	devcb_write_line::array<8> m_out_port_a_cb;
	devcb_write_line::array<8> m_out_port_b_cb;
	devcb_write_line::array<8> m_out_port_c_cb;
	devcb_write_line::array<8> m_out_port_d_cb;
	devcb_write_line::array<8> m_out_port_e_cb;
	devcb_write_line::array<8> m_out_port_f_cb;
	devcb_write_line::array<8> m_out_port_g_cb;

	devcb_read_line::array<8>  m_in_port_a_cb;
	devcb_read_line::array<8>  m_in_port_b_cb;
	devcb_read_line::array<8>  m_in_port_c_cb;
	devcb_read_line::array<8>  m_in_port_d_cb;
	devcb_read_line::array<8>  m_in_port_e_cb;
	devcb_read_line::array<8>  m_in_port_f_cb;
	devcb_read_line::array<8>  m_in_port_g_cb;

	devcb_write_line m_out_pwm_cb;

	devcb_write_line m_out_spim_cb;
	devcb_read_line  m_in_spim_cb;

	devcb_write_line m_out_flm_cb;
	devcb_write_line m_out_llp_cb;
	devcb_write_line m_out_lsclk_cb;
	devcb_write8     m_out_ld_cb;
	lcd_info_changed_delegate m_lcd_info_changed_cb;

	static const u32 VCO_DIVISORS[8];
};

class mc68328_device : public mc68328_base_device
{
public:
	mc68328_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <int Line> auto out_port_j() { return m_out_port_j_cb[Line].bind(); }
	template <int Line> auto out_port_k() { return m_out_port_k_cb[Line].bind(); }
	template <int Line> auto out_port_m() { return m_out_port_m_cb[Line].bind(); }
	template <int Line> auto in_port_j() { return m_in_port_j_cb[Line].bind(); }
	template <int Line> auto in_port_k() { return m_in_port_k_cb[Line].bind(); }
	template <int Line> auto in_port_m() { return m_in_port_m_cb[Line].bind(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum : u8
	{
		LPICF_GS                = 0x01,
		LPICF_GS_BIT            = 0,
		LPICF_PBSIZ             = 0x06,
		LPICF_PBSIZ_SHIFT       = 1,
		LPICF_PBSIZ_1           = 0x00,
		LPICF_PBSIZ_2           = 0x02,
		LPICF_PBSIZ_4           = 0x04,

		LCKCON_PCDS_BIT         = 0,
		LCKCON_DWIDTH           = 0x02,
		LCKCON_DWIDTH_BIT       = 1,
		LCKCON_WS               = 0x30,
		LCKCON_WS_SHIFT         = 4,
		LCKCON_WS_1             = 0x00,
		LCKCON_WS_2             = 0x10,
		LCKCON_WS_3             = 0x20,
		LCKCON_WS_4             = 0x30,
		LCKCON_DMA16            = 0x40,
		LCKCON_DMA16_BIT        = 6,
		LCKCON_LCDON_BIT        = 7,
	};

	enum : u16
	{
		PWMC_CLKSEL             = 0x0007,
		PWMC_PWMEN              = 0x0010,
		PWMC_POL                = 0x0040,
		PWMC_PIN                = 0x0080,
		PWMC_LOAD               = 0x0100,
		PWMC_IRQEN              = 0x4000,
		PWMC_PWMIRQ             = 0x8000,

		LGPMR_PAL2              = 0x0007,
		LGPMR_PAL3              = 0x0070,
		LGPMR_PAL0              = 0x0700,
		LGPMR_PAL1              = 0x7000,

		WCTLR_WDRST             = 0x0008,
		WCTLR_LOCK              = 0x0004,
		WCTLR_FI                = 0x0002,
		WCTLR_WDEN              = 0x0001,

		RTCIENR_MASK            = 0x001f,
	};

	void grpmaska_w(u16 data);
	void grpmaskb_w(u16 data);
	void grpmaskc_w(u16 data);
	void grpmaskd_w(u16 data);
	template<int ChipSelect> void csa_msw_w(offs_t offset, u16 data, u16 mem_mask);
	template<int ChipSelect> void csa_lsw_w(offs_t offset, u16 data, u16 mem_mask);
	template<int ChipSelect> void csb_msw_w(offs_t offset, u16 data, u16 mem_mask);
	template<int ChipSelect> void csb_lsw_w(offs_t offset, u16 data, u16 mem_mask);
	template<int ChipSelect> void csc_msw_w(offs_t offset, u16 data, u16 mem_mask);
	template<int ChipSelect> void csc_lsw_w(offs_t offset, u16 data, u16 mem_mask);
	template<int ChipSelect> void csd_msw_w(offs_t offset, u16 data, u16 mem_mask);
	template<int ChipSelect> void csd_lsw_w(offs_t offset, u16 data, u16 mem_mask);

	void iwr_msw_w(offs_t offset, u16 data, u16 mem_mask);
	void iwr_lsw_w(offs_t offset, u16 data, u16 mem_mask);

	void pasel_w(u8 data);
	void pjdir_w(u8 data);
	void pjdata_w(u8 data);
	void pjsel_w(u8 data);
	void pkdir_w(u8 data);
	void pkdata_w(u8 data);
	void pkpuen_w(u8 data);
	void pksel_w(u8 data);
	void pmdir_w(u8 data);
	void pmdata_w(u8 data);
	void pmpuen_w(u8 data);
	void pmsel_w(u8 data);

	virtual void pwmc_w(u16 data) override;
	void pwmp_w(u16 data);
	void pwmw_w(u16 data);
	void pwmcnt_w(u16 data);

	void wctlr_w(u16 data);
	void wcmpr_w(u16 data);
	void wcn_w(u16 data);

	void spisr_w(u16 data);

	virtual void lpicf_w(u8 data) override;
	virtual void lckcon_w(u8 data) override;
	void llbar_w(u8 data);
	void lotcr_w(u8 data);
	void lgpmr_w(u8 data);

	u16  grpmaska_r();
	u16  grpmaskb_r();
	u16  grpmaskc_r();
	u16  grpmaskd_r();
	template<int ChipSelect> u16 csa_msw_r();
	template<int ChipSelect> u16 csa_lsw_r();
	template<int ChipSelect> u16 csb_msw_r();
	template<int ChipSelect> u16 csb_lsw_r();
	template<int ChipSelect> u16 csc_msw_r();
	template<int ChipSelect> u16 csc_lsw_r();
	template<int ChipSelect> u16 csd_msw_r();
	template<int ChipSelect> u16 csd_lsw_r();

	u16 iwr_msw_r();
	u16 iwr_lsw_r();

	u8  pasel_r();
	u8  pjdir_r();
	u8  pjdata_r();
	u8  pjsel_r();
	u8  pkdir_r();
	u8  pkdata_r();
	u8  pkpuen_r();
	u8  pksel_r();
	u8  pmdata_r();
	u8  pmdir_r();
	u8  pmpuen_r();
	u8  pmsel_r();

	virtual u16 pwmc_r() override;
	u16 pwmp_r();
	u16 pwmw_r();
	u16 pwmcnt_r();

	u16 wctlr_r();
	u16 wcmpr_r();
	u16 wcn_r();

	u16 spisr_r();

	u8  llbar_r();
	u8  lotcr_r();
	u16 lgpmr_r();

	// $(FF)FFF108
	u16 m_grpmaska;     // Chip Select Group A Mask Register
	u16 m_grpmaskb;     // Chip Select Group B Mask Register
	u16 m_grpmaskc;     // Chip Select Group C Mask Register
	u16 m_grpmaskd;     // Chip Select Group D Mask Register
	u32 m_csa[4];       // Group A Chip Select Registers
	u32 m_csb[4];       // Group B Chip Select Registers
	u32 m_csc[4];       // Group C Chip Select Registers
	u32 m_csd[4];       // Group D Chip Select Registers

	// $(FF)FFF300
	u32 m_iwr;          // Interrupt Wakeup Enable Register

	// $(FF)FFF438
	u8  m_pjdir;        // Port J Direction Register
	u8  m_pjdata;       // Port J Data Register
	u8  m_pjsel;        // Port J Select Register

	u8  m_pkdir;        // Port K Direction Register
	u8  m_pkdata;       // Port K Data Register
	u8  m_pkpuen;       // Port K Pullup Enable Register
	u8  m_pksel;        // Port K Select Register

	u8  m_pmdir;        // Port M Direction Register
	u8  m_pmdata;       // Port M Data Register
	u8  m_pmpuen;       // Port M Pullup Enable Register
	u8  m_pmsel;        // Port M Select Register

	// $(FF)FFF502
	u16 m_pwmp;         // PWM Period Register
	u16 m_pwmw;         // PWM Width Register
	u16 m_pwmcnt;       // PWM Counter

	// $(FF)FFF600
	timer_regs m_timer_regs[2];
	u16 m_wctlr;        // Watchdog Control Register
	u16 m_wcmpr;        // Watchdog Compare Register
	u16 m_wcn;          // Watchdog Counter

	// $(FF)FFF700
	u16 m_spisr;        // SPIS Register

	// $(FF)FFFA00
	u8  m_llbar;        // Last Buffer Address Register
	u8  m_lotcr;        // Octet Terminal Count Register
	u16 m_lgpmr;        // Gray Palette Mapping Register

	void internal_map(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;

	// internal state
	virtual void register_state_save() override;

	virtual u32 get_irq_mask_for_level(int level) override;
	virtual int get_irq_level_for_mask(u32 level) override;

	virtual TIMER_CALLBACK_MEMBER(pwm_tick) override;

	virtual emu_timer *get_timer(int timer) override;
	virtual timer_regs &get_timer_regs(int timer) override;
	virtual u32 get_timer_int(int timer) override;

	virtual void lcd_update_info() override;
	virtual u16 lcd_get_lxmax_mask() override;
	virtual int lcd_get_width() override;
	virtual u32 lcd_get_line_word_count() override;
	virtual attotime lcd_get_line_rate() override;
	virtual u8 lcd_get_panel_bit_size() override;
	virtual attotime get_pixclk_rate() override;

	virtual bool rtc_int_is_active() override;
	virtual u16 rtc_get_int_mask() override;
	virtual bool rtc_get_alarm_match() override;

	emu_timer   *m_gptimer[2];

	devcb_write_line::array<8> m_out_port_j_cb;
	devcb_write_line::array<8> m_out_port_k_cb;
	devcb_write_line::array<8> m_out_port_m_cb;

	devcb_read_line::array<8>  m_in_port_j_cb;
	devcb_read_line::array<8>  m_in_port_k_cb;
	devcb_read_line::array<8>  m_in_port_m_cb;
};

class mc68ez328_device : public mc68328_base_device
{
public:
	mc68ez328_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum : u8
	{
		LPICF_GS                = 0x03,
		LPICF_GS_2COL           = 0x00,
		LPICF_GS_4COL           = 0x01,
		LPICF_GS_16COL          = 0x02,
		LPICF_PBSIZ             = 0x0c,
		LPICF_PBSIZ_SHIFT       = 2,
		LPICF_PBSIZ_1           = 0x00,
		LPICF_PBSIZ_2           = 0x04,
		LPICF_PBSIZ_4           = 0x08,

		LCKCON_DWS              = 0x0f,
		LCKCON_DWS_SHIFT        = 0,
		LCKCON_DWIDTH           = 0x40,
		LCKCON_DWIDTH_BIT       = 6,
		LCKCON_LCDON_BIT        = 7,
	};

	enum : u16
	{
		CSA_MASK                = 0x81ff,
		CSB_MASK                = 0xf9ff,
		CSC_MASK                = 0xf9ff,
		CSD_MASK                = 0xffff,
		EMUCS_MASK              = 0x0070,
		CS_EN_BIT               = 0,
		CS_SIZ_MASK             = 0x000e,
		CS_SIZ_SHIFT            = 1,
		CS_WS_MASK              = 0x0070,
		CS_WS_SHIFT             = 4,
		CS_BSW_BIT              = 7,
		CS_FLASH_BIT            = 8,
		CS_DRAM_BIT             = 9,
		CS_COMB_BIT             = 10,
		CS_UPSIZ_MASK           = 0x1800,
		CS_UPSIZ_SHIFT          = 11,
		CS_ROP_BIT              = 13,
		CS_SOP_BIT              = 14,
		CS_RO_BIT               = 15,

		PWMC_CLKSEL             = 0x0003,
		PWMC_REPEAT             = 0x000c,
		PWMC_REPEAT_SHIFT       = 2,
		PWMC_EN                 = 0x0010,
		PWMC_FIFO_AV            = 0x0020,
		PWMC_IRQ_EN             = 0x0040,
		PWMC_IRQ                = 0x0080,
		PWMC_IRQ_BIT            = 7,
		PWMC_PRESCALE           = 0x7f00,
		PWMC_PRESCALE_SHIFT     = 8,
		PWMC_CLK_SRC            = 0x8000,
		PWMC_RECALC_MASK        = PWMC_CLK_SRC | PWMC_PRESCALE | PWMC_CLKSEL | PWMC_EN,

		WATCHDOG_MASK           = 0x0083,
		WATCHDOG_EN_BIT         = 0,
		WATCHDOG_ISEL_BIT       = 1,
		WATCHDOG_INTF           = 0x0080,
		WATCHDOG_INTF_BIT       = 7,
		WATCHDOG_CNT_MASK       = 0x0300,
		WATCHDOG_CNT_SHIFT      = 8,

		RTCINT_HOUR             = 0x0020,
		RTCINT_SAM0             = 0x0100,
		RTCINT_SAM1             = 0x0200,
		RTCINT_SAM2             = 0x0400,
		RTCINT_SAM3             = 0x0800,
		RTCINT_SAM4             = 0x1000,
		RTCINT_SAM5             = 0x2000,
		RTCINT_SAM6             = 0x4000,
		RTCINT_SAM7             = 0x8000,
		RTCINT_RTCIRQ_MASK      = 0x00ff,

		RTC_DAYS_MASK           = 0x01ff,
	};

	enum : u32
	{
		INT_MSAM                = 22,
		INT_MSAM_MASK           = (1 << INT_MSAM),
		INT_MEMIQ               = 23,
		INT_MEMIQ_MASK          = (1 << INT_MEMIQ),
	};

	virtual void scr_w(u8 data) override;

	void csa_w(offs_t offset, u16 data, u16 mem_mask);
	void csb_w(offs_t offset, u16 data, u16 mem_mask);
	void csc_w(offs_t offset, u16 data, u16 mem_mask);
	void csd_w(offs_t offset, u16 data, u16 mem_mask);
	void emucs_w(offs_t offset, u16 data, u16 mem_mask);

	virtual void pwmc_w(u16 data) override;
	void pwms_w(offs_t offset, u16 data, u16 mem_mask);
	void pwmp_w(u8 data);
	void pwmcnt_w(u8 data);

	virtual void lpicf_w(u8 data) override;
	virtual void lckcon_w(u8 data) override;
	void lrra_w(u8 data);
	void pwmr_w(offs_t offset, u16 data, u16 mem_mask);

	void watchdog_w(offs_t offset, u16 data, u16 mem_mask);
	virtual void rtcctl_w(offs_t offset, u16 data, u16 mem_mask) override;
	void dayr_w(offs_t offset, u16 data, u16 mem_mask);
	void dayalarm_w(offs_t offset, u16 data, u16 mem_mask);

	u8 revision_r(offs_t offset);

	u16 csa_r();
	u16 csb_r();
	u16 csc_r();
	u16 csd_r();
	u16 emucs_r();

	virtual u16 pwmc_r() override;
	u16 pwms_r();
	u8  pwmp_r();
	u8  pwmcnt_r();

	u8  lrra_r();
	u16 pwmr_r();

	u16 watchdog_r();
	u16 dayr_r();
	u16 dayalarm_r();

	// $(FF)FFF100
	u16 m_csa;          // Group A Chip-Select Register
	u16 m_csb;          // Group B Chip-Select Register
	u16 m_csc;          // Group C Chip-Select Register
	u16 m_csd;          // Group D Chip-Select Register
	u16 m_emucs;        // Emulation Chip-Select Register

	// $(FF)FFF502
	u8  m_pwmp;         // PWM Period Register
	u8  m_pwmcnt;       // PWM Counter
	u8  m_pwmfifo[5];   // PWM FIFO
	u8  m_pwmfifo_wr;   // PWM FIFO Write Index
	u8  m_pwmfifo_rd;   // PWM FIFO Read Index
	u8  m_pwmfifo_cnt;  // PWM FIFO Count
	u8  m_pwm_rep_cnt;  // PWM Repeat Count

	// $(FF)FFF600
	timer_regs m_timer_regs;

	// $(FF)FFFA00
	u8   m_lrra;        // LCD Refresh Rate Adjustment Register
	u16  m_pwmr;        // PWM Contrast Control Register

	// $(FF)FFFB00
	u16  m_watchdog;    // Watchdog Timer Register
	u16  m_dayr;        // RTC Day Count Register
	u16  m_dayalarm;    // RTC Day Alarm Register
	u8   m_sam_cnt;     // RTC Sample Timer Counter (internal, not readable)

	void internal_map(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;

	// internal state
	virtual void register_state_save() override;

	virtual u32 get_irq_mask_for_level(int level) override;
	virtual int get_irq_level_for_mask(u32 level) override;

	void update_pwm_period(bool high_cycle);
	void pwm_fifo_push(u8 data);
	void pwm_fifo_pop();
	virtual TIMER_CALLBACK_MEMBER(pwm_tick) override;

	virtual emu_timer *get_timer(int timer) override;
	virtual timer_regs &get_timer_regs(int timer) override;
	virtual u32 get_timer_int(int timer) override;

	virtual void lcd_update_info() override;
	virtual u16 lcd_get_lxmax_mask() override;
	virtual int lcd_get_width() override;
	virtual u32 lcd_get_line_word_count() override;
	virtual attotime lcd_get_line_rate() override;
	virtual u8 lcd_get_panel_bit_size() override;
	virtual attotime get_pixclk_rate() override;

	TIMER_CALLBACK_MEMBER(sample_timer_tick);
	virtual bool rtc_int_is_active() override;
	virtual void rtc_advance_seconds() override;
	virtual u16 rtc_get_int_mask() override;
	virtual bool rtc_get_alarm_match() override;

	emu_timer *m_gptimer;
	emu_timer *m_rtc_sample_timer;
};

DECLARE_DEVICE_TYPE(MC68328, mc68328_device)
DECLARE_DEVICE_TYPE(MC68EZ328, mc68ez328_device)

#endif // MAME_MACHINE_MC68328_H
