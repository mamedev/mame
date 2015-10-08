// license:BSD-3-Clause
// copyright-holders:Lukasz Markowski
#ifndef CXHUMAX_H_
#define CXHUMAX_H_

#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "machine/intelfsh.h"
#include "machine/i2cmem.h"
#include "machine/terminal.h"

#define MAX_CX_TIMERS   16

struct cx_timer_t
{
	UINT32 value;
	UINT32 limit;
	UINT32 mode;
	UINT32 timebase;
	emu_timer *timer;
};

struct cx_timer_regs_t
{
	cx_timer_t timer[MAX_CX_TIMERS];
	UINT32 timer_irq;
};

#define TERMINAL_TAG "terminal"

class cxhumax_state : public driver_device
{
public:
	cxhumax_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_flash(*this, "flash"),
		m_ram(*this, "ram"),
		m_terminal(*this, TERMINAL_TAG),
		m_i2cmem(*this, "eeprom")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<intel_28f320j3d_device> m_flash;
	required_shared_ptr<UINT32> m_ram;
	required_device<generic_terminal_device> m_terminal;

	DECLARE_WRITE32_MEMBER ( flash_w );
	DECLARE_READ32_MEMBER ( flash_r );

	DECLARE_WRITE32_MEMBER ( cx_hsx_w );
	DECLARE_READ32_MEMBER ( cx_hsx_r );

	DECLARE_WRITE32_MEMBER ( cx_romdescr_w );
	DECLARE_READ32_MEMBER ( cx_romdescr_r );
	DECLARE_WRITE32_MEMBER ( cx_isaromdescr_w );
	DECLARE_READ32_MEMBER ( cx_isaromdescr_r );
	DECLARE_WRITE32_MEMBER ( cx_isadescr_w );
	DECLARE_READ32_MEMBER ( cx_isadescr_r );
	DECLARE_WRITE32_MEMBER ( cx_rommap_w );
	DECLARE_READ32_MEMBER ( cx_rommap_r );
	DECLARE_WRITE32_MEMBER ( cx_rommode_w );
	DECLARE_READ32_MEMBER ( cx_rommode_r );
	DECLARE_WRITE32_MEMBER ( cx_xoemask_w );
	DECLARE_READ32_MEMBER ( cx_xoemask_r );
	DECLARE_WRITE32_MEMBER ( cx_pci_w );
	DECLARE_READ32_MEMBER ( cx_pci_r );
	DECLARE_WRITE32_MEMBER ( cx_extdesc_w );
	DECLARE_READ32_MEMBER ( cx_extdesc_r );

	DECLARE_WRITE32_MEMBER ( cx_remap_w );
	DECLARE_WRITE32_MEMBER ( cx_scratch_w );
	DECLARE_READ32_MEMBER ( cx_scratch_r );

	DECLARE_WRITE32_MEMBER ( cx_timers_w );
	DECLARE_READ32_MEMBER ( cx_timers_r );

	DECLARE_WRITE32_MEMBER ( cx_uart2_w );
	DECLARE_READ32_MEMBER ( cx_uart2_r );

	DECLARE_WRITE32_MEMBER ( cx_pll_w );
	DECLARE_READ32_MEMBER ( cx_pll_r );
	DECLARE_WRITE32_MEMBER ( cx_clkdiv_w );
	DECLARE_READ32_MEMBER ( cx_clkdiv_r );
	DECLARE_WRITE32_MEMBER ( cx_pllprescale_w );
	DECLARE_READ32_MEMBER ( cx_pllprescale_r );

	DECLARE_WRITE32_MEMBER ( cx_chipcontrol_w );
	DECLARE_READ32_MEMBER ( cx_chipcontrol_r );

	DECLARE_WRITE32_MEMBER ( cx_intctrl_w );
	DECLARE_READ32_MEMBER ( cx_intctrl_r );

	DECLARE_WRITE32_MEMBER ( cx_ss_w );
	DECLARE_READ32_MEMBER ( cx_ss_r );

	DECLARE_WRITE32_MEMBER ( cx_i2c0_w );
	DECLARE_READ32_MEMBER ( cx_i2c0_r );
	DECLARE_WRITE32_MEMBER ( cx_i2c1_w );
	DECLARE_READ32_MEMBER ( cx_i2c1_r );
	DECLARE_WRITE32_MEMBER ( cx_i2c2_w );
	DECLARE_READ32_MEMBER ( cx_i2c2_r );

	DECLARE_WRITE32_MEMBER ( cx_mc_cfg_w );
	DECLARE_READ32_MEMBER ( cx_mc_cfg_r );

	DECLARE_WRITE32_MEMBER ( cx_drm0_w );
	DECLARE_READ32_MEMBER ( cx_drm0_r );
	DECLARE_WRITE32_MEMBER ( cx_drm1_w );
	DECLARE_READ32_MEMBER ( cx_drm1_r );

	DECLARE_WRITE32_MEMBER ( cx_hdmi_w );
	DECLARE_READ32_MEMBER ( cx_hdmi_r );

	DECLARE_WRITE32_MEMBER ( cx_gxa_w );
	DECLARE_READ32_MEMBER ( cx_gxa_r );

	DECLARE_READ32_MEMBER ( dummy_flash_r );

	UINT32 m_romdescr_reg;
	UINT32 m_isaromdescr_regs[0x0C/4];
	UINT32 m_isadescr_regs[0x10/4];
	UINT32 m_rommode_reg;
	UINT32 m_xoemask_reg;
	UINT32 m_pci_regs[0x08/4];
	UINT32 m_extdesc_regs[0x80/4];

	UINT32 m_scratch_reg;
	cx_timer_regs_t m_timer_regs;

	UINT32 m_uart2_regs[0x30/4];

	UINT32 m_pll_regs[0x14/4];
	UINT32 m_clkdiv_regs[0x18/4];
	UINT32 m_pllprescale_reg;

	UINT32 m_intctrl_regs[0x38/4];

	UINT32 m_ss_regs[0x18/4];
	UINT8 m_ss_tx_fifo[8];              // 8 entries (size hardcoded to 8 bits per entry - TODO)

	UINT32 m_i2c0_regs[0x20/4];
	UINT32 m_i2c1_regs[0x20/4];
	required_device<i2cmem_device> m_i2cmem;
	UINT32 m_i2c2_regs[0x20/4];

	void i2cmem_start();
	void i2cmem_stop();
	UINT8 i2cmem_read_byte(int last);
	void i2cmem_write_byte(UINT8 data);

	UINT32 m_mccfg_regs[0x0C/4];

	UINT32 m_chipcontrol_regs[0x74/4];

	UINT32 m_drm0_regs[0xfc/4];
	UINT32 m_drm1_regs[0xfc/4];

	UINT32 m_hdmi_regs[0x400/4];

	UINT32 m_gxa_cmd_regs[0x130/4];
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_cxhumax(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(timer_tick);
};

#define INTDEST         0   // Interrupt destination (1=IRQ, 0=FIQ)
#define INTENABLE       1   // Enables the interrupt generation
#define INTIRQ          2   // Normal interrupt
#define INTFIQ          3   // Fast interrupt
#define INTSTATCLR      4   // Read: interrupt status, Write: clear pending interrupt
#define INTSTATSET      5   // Read: interrupt status, Write: sets a pending interrupt
#define INTGROUP1       0
#define INTGROUP2       1

#define INTREG(group, index)    (((group) << 3) | (index))

#define GXA_CMD_RW_REGISTER             0x00
#define GXA_CMD_QMARK                   0x02
#define GXA_CMD_PALETTE_FETCH           0x03
#define GXA_CMD_VFILTER_COEFF_FETCH     0x04
#define GXA_CMD_HFILTER_COEFF_FETCH     0x05
#define GXA_CMD_BLT_21                  0x21
#define GXA_CMD_BLT_23                  0x23
#define GXA_CMD_BLT_25                  0x25
#define GXA_CMD_BLT_27                  0x27
#define GXA_CMD_BLT_2B                  0x2b
#define GXA_CMD_BLT_2F                  0x2f
#define GXA_CMD_LINE_30                 0x30
#define GXA_CMD_LINE_32                 0x32
#define GXA_CMD_BLT_31                  0x31
#define GXA_CMD_BLT                     0x33
#define GXA_CMD_LINE_34                 0x34
#define GXA_CMD_LINE_36                 0x36
#define GXA_CMD_BLT_35                  0x35
#define GXA_CMD_BLT_37                  0x37
#define GXA_CMD_LINE_3A                 0x3a
#define GXA_CMD_BLT_3B                  0x3b
#define GXA_CMD_LINE_3E                 0x3e
#define GXA_CMD_BLT_3F                  0x3f
#define GXA_CMD_SBLT_ABLEND             0x71
#define GXA_CMD_SBLT_ROP                0x7b

#define GXA_CMD_REG                     0x07

#define GXA_CFG2_REG                    0x3f
#define IRQ_STAT_QMARK                  21
#define IRQ_EN_QMARK                    17

#define INT_UART2_BIT                   (1<<1)
#define INT_TIMER_BIT                   (1<<7)
#define INT_PWM_BIT                     (1 << 14)
#define INT_PIO103_BIT                  (1 << 15)

#define PCI_CFG_ADDR_REG                0
#define PCI_CFG_DATA_REG                1

#define TIMER_VALUE                     0
#define TIMER_LIMIT                     1
#define TIMER_MODE                      2
#define TIMER_TIMEBASE                  3

#define UART_FIFO_REG                   0
#define UART_IRQE_REG                   1
#define UART_IRQE_TIDE_BIT              (1<<6)
#define UART_BRDL_REG                   0
#define UART_BRDU_REG                   1
#define UART_FIFC_REG                   2
#define UART_FRMC_REG                   3
#define UART_FRMC_BDS_BIT               (1<<7)
#define UART_STAT_REG                   5
#define UART_STAT_TSR_BIT               (1<<5)
#define UART_STAT_TID_BIT               (1<<6)

#define SREG_MPG_0_INTFRAC_REG          0
#define SREG_MPG_1_INTFRAC_REG          1
#define SREG_ARM_INTFRAC_REG            2
#define SREG_MEM_INTFRAC_REG            3
#define SREG_USB_INTFRAC_REG            4

#define SREG_DIV_0_REG                  0
#define SREG_DIV_1_REG                  1
#define SREG_DIV_2_REG                  2
#define SREG_DIV_3_REG                  3
#define SREG_DIV_4_REG                  4
#define SREG_DIV_5_REG                  5

#define PIN_CONFIG_0_REG                0   // Pin Configuration 0 Register
#define SREG_MODE_REG                   3   // SREG Mode Register
#define PIN_ALT_FUNC_REG                4   // Alternate Pin Function Select Register
#define PLL_LOCK_STAT_0_REG             9   // Resource Lock Register
#define PLL_IO_CTL_REG                  20  // IO Control Register
#define SREG_TEST_REG                   28  // Test Register

#define I2C_MODE_REG                    0
#define I2C_CTRL_REG                    1
#define I2C_STAT_REG                    2
#define I2C_RDATA_REG                   3

#define I2C_WACK_BIT                    (1<<1)
#define I2C_INT_BIT                     (1<<0)

#define SS_CNTL_REG                     0
#define SS_FIFC_REG                     1
#define SS_BAUD_REG                     2
#define SS_FIFO_REG                     4
#define SS_STAT_REG                     5

#define MC_CFG0                         0
#define MC_CFG1                         1
#define MC_CFG2                         2

#define DRM_ACTIVE_X_REG                1
#define DRM_ACTIVE_Y_REG                2
#define DRM_BCKGND_REG                  3
#define DRM_OSD_PTR_REG                 32

#endif /* CXHUMAX_H_ */
