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
	void scr_w(uint8_t data); // 0x000

	void grpbasea_w(uint16_t data); // 0x100
	void grpbaseb_w(uint16_t data); // 0x102
	void grpbasec_w(uint16_t data); // 0x104
	void grpbased_w(uint16_t data); // 0x106
	void grpmaska_w(uint16_t data); // 0x108
	void grpmaskb_w(uint16_t data); // 0x10a
	void grpmaskc_w(uint16_t data); // 0x10c
	void grpmaskd_w(uint16_t data); // 0x10e
	template<int ChipSelect> void csa_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0x110, 0x114, 0x118, 0x11c
	template<int ChipSelect> void csa_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0x112, 0x116, 0x11a, 0x11e
	template<int ChipSelect> void csb_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0x120, 0x124, 0x128, 0x12c
	template<int ChipSelect> void csb_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0x122, 0x126, 0x12a, 0x12e
	template<int ChipSelect> void csc_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0x130, 0x134, 0x138, 0x13c
	template<int ChipSelect> void csc_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0x132, 0x136, 0x13a, 0x13e
	template<int ChipSelect> void csd_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0x140, 0x144, 0x148, 0x14c
	template<int ChipSelect> void csd_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0x142, 0x146, 0x14a, 0x14e

	void pllcr_w(uint16_t data); // 0x200
	void pllfsr_w(uint16_t data); // 0x202
	void pctlr_w(uint8_t data); // 0x207
	void ivr_w(uint8_t data); // 0x300
	void icr_w(uint8_t data); // 0x302
	void imr_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0x304
	void imr_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0x306
	void iwr_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0x308
	void iwr_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0x30a
	void isr_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0x30c
	void isr_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0x30e
	void ipr_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0x310
	void ipr_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0x312

	void padir_w(uint8_t data); // 0x400
	void padata_w(uint8_t data); // 0x401
	void pasel_w(uint8_t data); // 0x403
	void pbdir_w(uint8_t data); // 0x408
	void pbdata_w(uint8_t data); // 0x409
	void pbsel_w(uint8_t data); // 0x40b
	void pcdir_w(uint8_t data); // 0x410
	void pcdata_w(uint8_t data); // 0x411
	void pcsel_w(uint8_t data); // 0x413
	void pddir_w(uint8_t data); // 0x418
	void pddata_w(uint8_t data); // 0x419
	void pdpuen_w(uint8_t data); // 0x41a
	void pdpol_w(uint8_t data); // 0x41c
	void pdirqen_w(uint8_t data); // 0x41d
	void pdirqedge_w(uint8_t data); // 0x41f
	void pedir_w(uint8_t data); // 0x420
	void pedata_w(uint8_t data); // 0x421
	void pepuen_w(uint8_t data); // 0x422
	void pesel_w(uint8_t data); // 0x423
	void pfdir_w(uint8_t data); // 0x428
	void pfdata_w(uint8_t data); // 0x429
	void pfpuen_w(uint8_t data); // 0x42a
	void pfsel_w(uint8_t data); // 0x42b
	void pgdir_w(uint8_t data); // 0x430
	void pgdata_w(uint8_t data); // 0x431
	void pgpuen_w(uint8_t data); // 0x432
	void pgsel_w(uint8_t data); // 0x433
	void pjdir_w(uint8_t data); // 0x438
	void pjdata_w(uint8_t data); // 0x439
	void pjsel_w(uint8_t data); // 0x43b
	void pkdir_w(uint8_t data); // 0x440
	void pkdata_w(uint8_t data); // 0x441
	void pkpuen_w(uint8_t data); // 0x442
	void pksel_w(uint8_t data); // 0x443
	void pmdir_w(uint8_t data); // 0x448
	void pmdata_w(uint8_t data); // 0x449
	void pmpuen_w(uint8_t data); // 0x44a
	void pmsel_w(uint8_t data); // 0x44b

	void pwmc_w(uint16_t data); // 0x500
	void pwmp_w(uint16_t data); // 0x502
	void pwmw_w(uint16_t data); // 0x504
	void pwmcnt_w(uint16_t data); // 0x506

	template <int Timer> void tctl_w(uint16_t data); // 0x600, 0x60c
	template <int Timer> void tprer_w(uint16_t data); // 0x602, 0x60e
	template <int Timer> void tcmp_w(uint16_t data); // 0x604, 0x610
	template <int Timer> void tcr_w(uint16_t data); // 0x606, 0x612
	template <int Timer> void tcn_w(uint16_t data); // 0x608, 0x614
	template <int Timer> void tstat_w(uint16_t data); // 0x60a, 0x616
	void wctlr_w(uint16_t data); // 0x618
	void wcmpr_w(uint16_t data); // 0x61a
	void wcn_w(uint16_t data); // 0x61c

	void spisr_w(uint16_t data); // 0x700

	void spimdata_w(uint16_t data); // 0x800
	void spimcont_w(uint16_t data); // 0x802

	void ustcnt_w(uint16_t data); // 0x900
	void ubaud_w(uint16_t data); // 0x902
	void urx_w(uint16_t data); // 0x904
	void utx_w(uint16_t data); // 0x906
	void umisc_w(uint16_t data); // 0x908

	void lssa_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0xa00
	void lssa_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0xa02
	void lvpw_w(uint8_t data); // 0xa05
	void lxmax_w(uint16_t data); // 0xa08
	void lymax_w(uint16_t data); // 0xa0a
	void lcxp_w(uint16_t data); // 0xa18
	void lcyp_w(uint16_t data); // 0xa1a
	void lcwch_w(uint16_t data); // 0xa1c
	void lblkc_w(uint8_t data); // 0xa1f
	void lpicf_w(uint8_t data); // 0xa20
	void lpolcf_w(uint8_t data); // 0xa21
	void lacdrc_w(uint8_t data); // 0xa23
	void lpxcd_w(uint8_t data); // 0xa25
	void lckcon_w(uint8_t data); // 0xa27
	void llbar_w(uint8_t data); // 0xa29
	void lotcr_w(uint8_t data); // 0xa2b
	void lposr_w(uint8_t data); // 0xa2d
	void lfrcm_w(uint8_t data); // 0xa31
	void lgpmr_w(uint8_t data); // 0xa32

	void hmsr_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0xb00
	void hmsr_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0xb02
	void alarm_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0xb04
	void alarm_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0xb06
	void rtcctl_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0xb0c
	void rtcisr_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0xb0e
	void rtcienr_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0xb10
	void stpwtch_w(offs_t offset, uint16_t data, uint16_t mem_mask); // 0xb12

	uint8_t scr_r(); // 0x000

	uint16_t grpbasea_r(); // 0x100
	uint16_t grpbaseb_r(); // 0x102
	uint16_t grpbasec_r(); // 0x104
	uint16_t grpbased_r(); // 0x106
	uint16_t grpmaska_r(); // 0x108
	uint16_t grpmaskb_r(); // 0x10a
	uint16_t grpmaskc_r(); // 0x10c
	uint16_t grpmaskd_r(); // 0x10e

	template<int ChipSelect> uint16_t csa_msw_r(); // 0x110, 0x114, 0x118, 0x11c
	template<int ChipSelect> uint16_t csa_lsw_r(); // 0x112, 0x116, 0x11a, 0x11e
	template<int ChipSelect> uint16_t csb_msw_r(); // 0x120, 0x124, 0x128, 0x12c
	template<int ChipSelect> uint16_t csb_lsw_r(); // 0x122, 0x126, 0x12a, 0x12e
	template<int ChipSelect> uint16_t csc_msw_r(); // 0x130, 0x134, 0x138, 0x13c
	template<int ChipSelect> uint16_t csc_lsw_r(); // 0x132, 0x136, 0x13a, 0x13e
	template<int ChipSelect> uint16_t csd_msw_r(); // 0x140, 0x144, 0x148, 0x14c
	template<int ChipSelect> uint16_t csd_lsw_r(); // 0x142, 0x146, 0x14a, 0x14e

	uint16_t pllcr_r(); // 0x200
	uint16_t pllfsr_r(); // 0x202
	uint8_t pctlr_r(); // 0x207

	uint8_t ivr_r(); // 0x300
	uint16_t icr_r(); // 0x302
	uint16_t imr_msw_r(); // 0x304
	uint16_t imr_lsw_r(); // 0x306
	uint16_t iwr_msw_r(); // 0x308
	uint16_t iwr_lsw_r(); // 0x30a
	uint16_t isr_msw_r(); // 0x30c
	uint16_t isr_lsw_r(); // 0x30e
	uint16_t ipr_msw_r(); // 0x310
	uint16_t ipr_lsw_r(); // 0x312

	uint8_t padir_r(); // 0x400
	uint8_t padata_r(); // 0x401
	uint8_t pasel_r(); // 0x403
	uint8_t pbdir_r(); // 0x408
	uint8_t pbdata_r(); // 0x409
	uint8_t pbsel_r(); // 0x40b
	uint8_t pcdir_r(); // 0x410
	uint8_t pcdata_r(); // 0x411
	uint8_t pcsel_r(); // 0x413
	uint8_t pddir_r(); // 0x418
	uint8_t pddata_r(); // 0x419
	uint8_t pdpuen_r(); // 0x41a
	uint8_t pdpol_r(); // 0x41c
	uint8_t pdirqen_r(); // 0x41d
	uint8_t pdirqedge_r(); // 0x41f
	uint8_t pedir_r(); // 0x420
	uint8_t pedata_r(); // 0x421
	uint8_t pepuen_r(); // 0x422
	uint8_t pesel_r(); // 0x423
	uint8_t pfdir_r(); // 0x428
	uint8_t pfdata_r(); // 0x429
	uint8_t pfpuen_r(); // 0x42a
	uint8_t pfsel_r(); // 0x42b
	uint8_t pgdir_r(); // 0x430
	uint8_t pgdata_r(); // 0x431
	uint8_t pgpuen_r(); // 0x432
	uint8_t pgsel_r(); // 0x433
	uint8_t pjdir_r(); // 0x438
	uint8_t pjdata_r(); // 0x439
	uint8_t pjsel_r(); // 0x43b
	uint8_t pkdir_r(); // 0x440
	uint8_t pkdata_r(); // 0x441
	uint8_t pkpuen_r(); // 0x442
	uint8_t pksel_r(); // 0x443
	uint8_t pmdata_r(); // 0x449
	uint8_t pmdir_r(); // 0x448
	uint8_t pmpuen_r(); // 0x44a
	uint8_t pmsel_r(); // 0x44b

	uint16_t pwmc_r(); // 0x500
	uint16_t pwmp_r(); // 0x502
	uint16_t pwmw_r(); // 0x504
	uint16_t pwmcnt_r(); // 0x506

	template <int Timer> uint16_t tctl_r(); // 0x600, 0x60c
	template <int Timer> uint16_t tprer_r(); // 0x602, 0x60e
	template <int Timer> uint16_t tcmp_r(); // 0x604, 0x610
	template <int Timer> uint16_t tcr_r(); // 0x606, 0x612
	template <int Timer> uint16_t tcn_r(); // 0x608, 0x614
	template <int Timer> uint16_t tstat_r(); // 0x60a, 0x616
	uint16_t wctlr_r(); // 0x618
	uint16_t wcmpr_r(); // 0x61a
	uint16_t wcn_r(); // 0x61c

	uint16_t spisr_r(); // 0x700

	uint16_t spimdata_r(); // 0x800
	uint16_t spimcont_r(); // 0x802

	uint16_t ustcnt_r(); // 0x900
	uint16_t ubaud_r(); // 0x902
	uint16_t urx_r(); // 0x904
	uint16_t utx_r(); // 0x906
	uint16_t umisc_r(); // 0x908

	uint16_t lssa_msw_r(); // 0xa00
	uint16_t lssa_lsw_r(); // 0xa02
	uint8_t lvpw_r(); // 0xa05
	uint16_t lxmax_r(); // 0xa08
	uint16_t lymax_r(); // 0xa0a
	uint16_t lcxp_r(); // 0xa18
	uint16_t lcyp_r(); // 0xa1a
	uint16_t lcwch_r(); // 0xa1c
	uint8_t lblkc_r(); // 0xa1f
	uint8_t lpicf_r(); // 0xa20
	uint8_t lpolcf_r(); // 0xa21
	uint8_t lacdrc_r(); // 0xa23
	uint8_t lpxcd_r(); // 0xa25
	uint8_t lckcon_r(); // 0xa27
	uint8_t llbar_r(); // 0xa29
	uint8_t lotcr_r(); // 0xa2b
	uint8_t lposr_r(); // 0xa2d
	uint8_t lfrcm_r(); // 0xa31
	uint16_t lgpmr_r(); // 0xa32

	uint16_t hmsr_msw_r(); // 0xb00
	uint16_t hmsr_lsw_r(); // 0xb02
	uint16_t alarm_msw_r(); // 0xb04
	uint16_t alarm_lsw_r(); // 0xb06
	uint16_t rtcctl_r(); // 0xb0c
	uint16_t rtcisr_r(); // 0xb0e
	uint16_t rtcienr_r(); // 0xb10
	uint16_t stpwtch_r(); // 0xb12

	// $(FF)FFF000
	uint8_t   m_scr;        // System Control Register

	// $(FF)FFF100
	uint16_t  m_grpbasea;   // Chip Select Group A Base Register
	uint16_t  m_grpbaseb;   // Chip Select Group B Base Register
	uint16_t  m_grpbasec;   // Chip Select Group C Base Register
	uint16_t  m_grpbased;   // Chip Select Group D Base Register
	uint16_t  m_grpmaska;   // Chip Select Group A Mask Register
	uint16_t  m_grpmaskb;   // Chip Select Group B Mask Register
	uint16_t  m_grpmaskc;   // Chip Select Group C Mask Register
	uint16_t  m_grpmaskd;   // Chip Select Group D Mask Register
	uint32_t  m_csa[4];     // Group A Chip Select Registers
	uint32_t  m_csb[4];     // Group B Chip Select Registers
	uint32_t  m_csc[4];     // Group C Chip Select Registers
	uint32_t  m_csd[4];     // Group D Chip Select Registers

	// $(FF)FFF200
	uint16_t  m_pllcr;      // PLL Control Register
	uint16_t  m_pllfsr;     // PLL Frequency Select Register
	uint8_t   m_pctlr;      // Power Control Register

	// $(FF)FFF300
	uint8_t   m_ivr;        // Interrupt Vector Register
	uint16_t  m_icr;        // Interrupt Control Register
	uint32_t  m_imr;        // Interrupt Mask Register
	uint32_t  m_iwr;        // Interrupt Wakeup Enable Register
	uint32_t  m_isr;        // Interrupt Status Register
	uint32_t  m_ipr;        // Interrupt Pending Register

	// $(FF)FFF400
	uint8_t   m_padir;      // Port A Direction Register
	uint8_t   m_padata;     // Port A Data Register
	uint8_t   m_pasel;      // Port A Select Register

	uint8_t   m_pbdir;      // Port B Direction Register
	uint8_t   m_pbdata;     // Port B Data Register
	uint8_t   m_pbsel;      // Port B Select Register

	uint8_t   m_pcdir;      // Port C Direction Register
	uint8_t   m_pcdata;     // Port C Data Register
	uint8_t   m_pcsel;      // Port C Select Register

	uint8_t   m_pddir;      // Port D Direction Register
	uint8_t   m_pddata;     // Port D Data Register
	uint8_t   m_pdpuen;     // Port D Pullup Enable Register
	uint8_t   m_pdpol;      // Port D Polarity Register
	uint8_t   m_pdirqen;    // Port D IRQ Enable Register
	uint8_t   m_pddataedge; // Port D Data Edge Level
	uint8_t   m_pdirqedge;  // Port D IRQ Edge Register

	uint8_t   m_pedir;      // Port E Direction Register
	uint8_t   m_pedata;     // Port E Data Register
	uint8_t   m_pepuen;     // Port E Pullup Enable Register
	uint8_t   m_pesel;      // Port E Select Register

	uint8_t   m_pfdir;      // Port F Direction Register
	uint8_t   m_pfdata;     // Port F Data Register
	uint8_t   m_pfpuen;     // Port F Pullup Enable Register
	uint8_t   m_pfsel;      // Port F Select Register

	uint8_t   m_pgdir;      // Port G Direction Register
	uint8_t   m_pgdata;     // Port G Data Register
	uint8_t   m_pgpuen;     // Port G Pullup Enable Register
	uint8_t   m_pgsel;      // Port G Select Register

	uint8_t   m_pjdir;      // Port J Direction Register
	uint8_t   m_pjdata;     // Port J Data Register
	uint8_t   m_pjsel;      // Port J Select Register
	uint8_t   m_pkdir;      // Port K Direction Register
	uint8_t   m_pkdata;     // Port K Data Register
	uint8_t   m_pkpuen;     // Port K Pullup Enable Register
	uint8_t   m_pksel;      // Port K Select Register

	uint8_t   m_pmdir;      // Port M Direction Register
	uint8_t   m_pmdata;     // Port M Data Register
	uint8_t   m_pmpuen;     // Port M Pullup Enable Register
	uint8_t   m_pmsel;      // Port M Select Register

	// $(FF)FFF500
	uint16_t  m_pwmc;       // PWM Control Register
	uint16_t  m_pwmp;       // PWM Period Register
	uint16_t  m_pwmw;       // PWM Width Register
	uint16_t  m_pwmcnt;     // PWN Counter

	// $(FF)FFF600
	uint16_t  m_tctl[2];    // Timer Control Register
	uint16_t  m_tprer[2];   // Timer Prescaler Register
	uint16_t  m_tcmp[2];    // Timer Compare Register
	uint16_t  m_tcr[2];     // Timer Capture Register
	uint16_t  m_tcn[2];     // Timer Counter
	uint16_t  m_tstat[2];   // Timer Status
	uint16_t  m_wctlr;      // Watchdog Control Register
	uint16_t  m_wcmpr;      // Watchdog Compare Register
	uint16_t  m_wcn;        // Watchdog Counter
	uint16_t  m_tclear[2];  // Timer Clearable Status

	// $(FF)FFF700
	uint16_t  m_spisr;      // SPIS Register

	// $(FF)FFF800
	uint16_t  m_spimdata;   // SPIM Data Register
	uint16_t  m_spimcont;   // SPIM Control/Status Register

	// $(FF)FFF900
	uint16_t  m_ustcnt;     // UART Status/Control Register
	uint16_t  m_ubaud;      // UART Baud Control Register
	uint16_t  m_urx;        // UART RX Register
	uint16_t  m_utx;        // UART TX Register
	uint16_t  m_umisc;      // UART Misc Register

	// $(FF)FFFA00
	uint32_t  m_lssa;       // Screen Starting Address Register
	uint8_t   m_lvpw;       // Virtual Page Width Register
	uint16_t  m_lxmax;      // Screen Width Register
	uint16_t  m_lymax;      // Screen Height Register
	uint16_t  m_lcxp;       // Cursor X Position
	uint16_t  m_lcyp;       // Cursor Y Position
	uint16_t  m_lcwch;      // Cursor Width & Height Register
	uint8_t   m_lblkc;      // Blink Control Register
	uint8_t   m_lpicf;      // Panel Interface Config Register
	uint8_t   m_lpolcf;     // Polarity Config Register
	uint8_t   m_lacdrc;     // ACD (M) Rate Control Register
	uint8_t   m_lpxcd;      // Pixel Clock Divider Register
	uint8_t   m_lckcon;     // Clocking Control Register
	uint8_t   m_llbar;      // Last Buffer Address Register
	uint8_t   m_lotcr;      // Octet Terminal Count Register
	uint8_t   m_lposr;      // Panning Offset Register
	uint8_t   m_lfrcm;      // Frame Rate Control Modulation Register
	uint16_t  m_lgpmr;      // Gray Palette Mapping Register

	// $(FF)FFFB00
	uint32_t  m_hmsr;       // RTC Hours Minutes Seconds Register
	uint32_t  m_alarm;      // RTC Alarm Register
	uint16_t  m_rtcctl;     // RTC Control Register
	uint16_t  m_rtcisr;     // RTC Interrupt Status Register
	uint16_t  m_rtcienr;    // RTC Interrupt Enable Register
	uint16_t  m_stpwtch;    // Stopwatch Minutes

	void internal_map(address_map &map);

	// internal state
	void set_interrupt_line(uint32_t line, uint32_t active);
	void poll_port_d_interrupts();
	void cpu_space_map(address_map &map);
	uint8_t irq_callback(offs_t offset);

	template<int Timer> uint32_t get_timer_frequency();
	template<int Timer> void maybe_start_timer(uint32_t new_enable);
	template<int Timer> void timer_compare_event();

	void register_state_save();

	TIMER_CALLBACK_MEMBER(timer1_hit);
	TIMER_CALLBACK_MEMBER(timer2_hit);
	TIMER_CALLBACK_MEMBER(pwm_transition);
	TIMER_CALLBACK_MEMBER(rtc_tick);

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

	devcb_write_line m_out_pwm_cb;    /* 1-bit output */

	devcb_write16 m_out_spim_cb;      /* 16-bit output */
	devcb_read16  m_in_spim_cb;       /* 16-bit input */

	devcb_write_line m_spim_xch_trigger_cb;    /* SPIM exchange trigger */ /*todo: not really a write line, fix*/
};


DECLARE_DEVICE_TYPE(MC68328, mc68328_device)

#endif // MAME_MACHINE_MC68328_H
