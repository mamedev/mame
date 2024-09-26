// license:BSD-3-Clause
// copyright-holders:Lukasz Markowski
#ifndef MAME_SKELETON_CXHUMAX_H
#define MAME_SKELETON_CXHUMAX_H

#pragma once

#include "cpu/arm7/arm7.h"
#include "machine/i2cmem.h"
#include "machine/intelfsh.h"
#include "machine/terminal.h"


class cxhumax_state : public driver_device
{
public:
	static constexpr unsigned MAX_CX_TIMERS = 16;

	struct cx_timer_t
	{
		uint32_t value = 0U;
		uint32_t limit = 0U;
		uint32_t mode = 0U;
		uint32_t timebase = 0U;
		emu_timer *timer = nullptr;
	};

	struct cx_timer_regs_t
	{
		cx_timer_t timer[MAX_CX_TIMERS]{};
		uint32_t timer_irq = 0U;
	};

	cxhumax_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_flash(*this, "flash"),
		m_ram(*this, "ram"),
		m_terminal(*this, "terminal"),
		m_i2cmem(*this, "eeprom")
	{
	}

	void cxhumax(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<intel_28f320j3d_device> m_flash;
	required_shared_ptr<uint32_t> m_ram;
	required_device<generic_terminal_device> m_terminal;

	void flash_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t flash_r(offs_t offset, uint32_t mem_mask = ~0);

	void cx_hsx_w(offs_t offset, uint32_t data);
	uint32_t cx_hsx_r(offs_t offset);

	void cx_romdescr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_romdescr_r(offs_t offset);
	void cx_isaromdescr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_isaromdescr_r(offs_t offset);
	void cx_isadescr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_isadescr_r(offs_t offset);
	void cx_rommap_w(offs_t offset, uint32_t data);
	uint32_t cx_rommap_r(offs_t offset);
	void cx_rommode_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_rommode_r(offs_t offset);
	void cx_xoemask_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_xoemask_r(offs_t offset);
	void cx_pci_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_pci_r(offs_t offset);
	void cx_extdesc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_extdesc_r(offs_t offset);

	void cx_remap_w(offs_t offset, uint32_t data);
	void cx_scratch_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_scratch_r(offs_t offset);

	void cx_timers_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_timers_r(offs_t offset);

	void cx_uart2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_uart2_r(offs_t offset);

	void cx_pll_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_pll_r(offs_t offset);
	void cx_clkdiv_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_clkdiv_r(offs_t offset);
	void cx_pllprescale_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_pllprescale_r(offs_t offset);

	void cx_chipcontrol_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_chipcontrol_r(offs_t offset);

	void cx_intctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_intctrl_r(offs_t offset);

	void cx_ss_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_ss_r(offs_t offset);

	void cx_i2c0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_i2c0_r(offs_t offset);
	void cx_i2c1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_i2c1_r(offs_t offset);
	void cx_i2c2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_i2c2_r(offs_t offset);

	void cx_mc_cfg_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_mc_cfg_r(offs_t offset);

	void cx_drm0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_drm0_r(offs_t offset);
	void cx_drm1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_drm1_r(offs_t offset);

	void cx_hdmi_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_hdmi_r(offs_t offset);

	void cx_gxa_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cx_gxa_r(offs_t offset);

	uint32_t dummy_flash_r();

	uint32_t m_romdescr_reg = 0U;
	uint32_t m_isaromdescr_regs[0x0C/4]{};
	uint32_t m_isadescr_regs[0x10/4]{};
	uint32_t m_rommode_reg = 0U;
	uint32_t m_xoemask_reg = 0U;
	uint32_t m_pci_regs[0x08/4]{};
	uint32_t m_extdesc_regs[0x80/4]{};

	uint32_t m_scratch_reg = 0U;
	cx_timer_regs_t m_timer_regs{};

	uint32_t m_uart2_regs[0x30/4]{};

	uint32_t m_pll_regs[0x14/4]{};
	uint32_t m_clkdiv_regs[0x18/4]{};
	uint32_t m_pllprescale_reg = 0U;

	uint32_t m_intctrl_regs[0x38/4]{};

	uint32_t m_ss_regs[0x18/4]{};
	uint8_t m_ss_tx_fifo[8]{};              // 8 entries (size hardcoded to 8 bits per entry - TODO)

	uint32_t m_i2c0_regs[0x20/4]{};
	uint32_t m_i2c1_regs[0x20/4]{};
	required_device<i2cmem_device> m_i2cmem;
	uint32_t m_i2c2_regs[0x20/4]{};

	void i2cmem_start();
	void i2cmem_stop();
	uint8_t i2cmem_read_byte(int last);
	void i2cmem_write_byte(uint8_t data);

	uint32_t m_mccfg_regs[0x0C/4]{};

	uint32_t m_chipcontrol_regs[0x74/4]{};

	uint32_t m_drm0_regs[0xfc/4]{};
	uint32_t m_drm1_regs[0xfc/4]{};

	uint32_t m_hdmi_regs[0x400/4]{};

	uint32_t m_gxa_cmd_regs[0x130/4]{};
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_cxhumax(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(timer_tick);
	void cxhumax_map(address_map &map) ATTR_COLD;
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

#endif // MAME_SKELETON_CXHUMAX_H
