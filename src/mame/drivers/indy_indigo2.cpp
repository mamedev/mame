// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************\
*
*   SGI IP22 Indigo2/Indy workstation
*
*   Todo: Fix tod clock set problem
*         Fix NVRAM saving
*         Fix SCSI DMA to handle chains properly
*         Probably many more things
*
*  Memory map:
*
*  18000000 - 1effffff      RESERVED - Unused
*  1f000000 - 1f3fffff      GIO - GFX
*  1f400000 - 1f5fffff      GIO - EXP0
*  1f600000 - 1f9fffff      GIO - EXP1 - Unused
*  1fa00000 - 1fa02047      Memory Controller
*  1fb00000 - 1fb1a7ff      HPC3 CHIP1
*  1fb80000 - 1fb9a7ff      HPC3 CHIP0
*  1fc00000 - 1fc7ffff      BIOS
*
*  References used:
*    MipsLinux: http://www.mips-linux.org/
*      linux-2.6.6/include/newport.h
*      linux-2.6.6/include/asm-mips/sgi/gio.h
*      linux-2.6.6/include/asm-mips/sgi/mc.h
*      linux-2.6.6/include/asm-mips/sgi/hpc3.h
*    NetBSD: http://www.netbsd.org/
*    gxemul: http://gavare.se/gxemul/
*
* Gentoo LiveCD r5 boot instructions:
*     mess -cdrom gentoor5.chd ip225015
*     enter the command interpreter and type "sashARCS".  press enter and
*     it'll autoboot.
*
* IRIX boot instructions:
*     mess -cdrom irix656inst1.chd ip225015
*     at the menu, choose either "run diagnostics" or "install system software"
*
\*********************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsicd.h"
#include "bus/scsi/scsihd.h"
#include "cpu/mips/mips3.h"
#include "machine/8042kbdc.h"
#include "machine/ds1386.h"
#include "machine/pc_lpt.h"
#include "machine/pckeybrd.h"
#include "machine/pit8253.h"
#include "machine/sgi.h"
#include "machine/wd33c93.h"
#include "machine/z80scc.h"
#include "sound/cdda.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/newport.h"
#include "screen.h"
#include "speaker.h"

#define SCC_TAG     "scc"
#define PI1_TAG     "pi1"
#define KBDC_TAG    "kbdc"
#define PIT_TAG     "pit"
#define RS232A_TAG  "rs232a"
#define RS232B_TAG  "rs232b"

#define SCC_PCLK    XTAL(10'000'000)
#define SCC_RXA_CLK XTAL(3'686'400) // Needs verification
#define SCC_TXA_CLK 0
#define SCC_RXB_CLK XTAL(3'686'400) // Needs verification
#define SCC_TXB_CLK 0

#define MCFG_IOC2_GUINNESS_ADD(_tag)  \
	MCFG_DEVICE_ADD(_tag, SGI_IOC2_GUINNESS, 0)

#define MCFG_IOC2_FULL_HOUSE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SGI_IOC2_FULL_HOUSE, 0)

class ioc2_device : public device_t
{
public:
	DECLARE_WRITE32_MEMBER( write );
	DECLARE_READ32_MEMBER( read );

	DECLARE_INPUT_CHANGED_MEMBER( power_button );
	DECLARE_INPUT_CHANGED_MEMBER( volume_down );
	DECLARE_INPUT_CHANGED_MEMBER( volume_up );

	void lower_local0_irq(uint8_t source_mask);
	void raise_local0_irq(uint8_t source_mask);
	void lower_local1_irq(uint8_t source_mask);
	void raise_local1_irq(uint8_t source_mask);

protected:
	ioc2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t id);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	required_device<mips3_device> m_maincpu;
	required_device<scc85230_device> m_scc;
	required_device<pc_lpt_device> m_pi1;   // we assume standard parallel port (SPP) mode
											// TODO: SGI parallel port (SGIPP), HP BOISE high speed parallel port (HPBPP), and Ricoh scanner modes
	required_device<kbdc8042_device> m_kbdc;
	required_device<pit8254_device> m_pit;

	virtual void handle_reset_reg_write(uint8_t data);

	uint8_t m_gen_ctrl_select_reg;
	uint8_t m_gen_ctrl_reg;
	uint8_t m_front_panel_reg;

	uint8_t m_read_reg;
	uint8_t m_dma_sel;
	uint8_t m_reset_reg;
	uint8_t m_write_reg;

	uint8_t m_int3_local0_status_reg;
	uint8_t m_int3_local0_mask_reg;
	uint8_t m_int3_local1_status_reg;
	uint8_t m_int3_local1_mask_reg;
	uint8_t m_int3_map_status_reg;
	uint8_t m_int3_map_mask0_reg;
	uint8_t m_int3_map_mask1_reg;
	uint8_t m_int3_map_pol_reg;
	uint8_t m_int3_timer_clear_reg;
	uint8_t m_int3_err_status_reg;

	uint32_t    m_par_read_cnt;
	uint32_t    m_par_cntl;
	uint8_t m_system_id;
};

class ioc2_guinness_device : public ioc2_device
{
public:
	ioc2_guinness_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	ioc2_guinness_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: ioc2_device(mconfig, type, tag, owner, clock, 0x01)
	{ }
};

class ioc2_full_house_device : public ioc2_device
{
public:
	ioc2_full_house_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	ioc2_full_house_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: ioc2_device(mconfig, type, tag, owner, clock, 0x20)
	{ }
};

DEFINE_DEVICE_TYPE(SGI_IOC2_GUINNESS,   ioc2_guinness_device,   "ioc2g", "SGI IOC2 (Guiness)")
DEFINE_DEVICE_TYPE(SGI_IOC2_FULL_HOUSE, ioc2_full_house_device, "ioc2f", "SGI IOC2 (Full House)")

ioc2_guinness_device::ioc2_guinness_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ioc2_guinness_device(mconfig, SGI_IOC2_GUINNESS, tag, owner, clock)
{
}

ioc2_full_house_device::ioc2_full_house_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ioc2_full_house_device(mconfig, SGI_IOC2_FULL_HOUSE, tag, owner, clock)
{
}

static INPUT_PORTS_START( front_panel )
	PORT_START("panel_buttons")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Power")        PORT_CHANGED_MEMBER(DEVICE_SELF, ioc2_device, power_button, 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Volume Down")  PORT_CHANGED_MEMBER(DEVICE_SELF, ioc2_device, volume_down, 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Volume Up")    PORT_CHANGED_MEMBER(DEVICE_SELF, ioc2_device, volume_up, 0)
INPUT_PORTS_END

ioport_constructor ioc2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(front_panel);
}

MACHINE_CONFIG_START(ioc2_device::device_add_mconfig)
	MCFG_SCC85230_ADD(SCC_TAG, SCC_PCLK, SCC_RXA_CLK.value(), SCC_TXA_CLK, SCC_RXB_CLK.value(), SCC_TXB_CLK)
	MCFG_Z80SCC_OUT_TXDA_CB(DEVWRITELINE(RS232A_TAG, rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_DTRA_CB(DEVWRITELINE(RS232A_TAG, rs232_port_device, write_dtr))
	MCFG_Z80SCC_OUT_RTSA_CB(DEVWRITELINE(RS232A_TAG, rs232_port_device, write_rts))
	MCFG_Z80SCC_OUT_TXDB_CB(DEVWRITELINE(RS232B_TAG, rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_DTRB_CB(DEVWRITELINE(RS232B_TAG, rs232_port_device, write_dtr))
	MCFG_Z80SCC_OUT_RTSB_CB(DEVWRITELINE(RS232B_TAG, rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD(RS232A_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(SCC_TAG, scc85230_device, ctsa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(SCC_TAG, scc85230_device, dcda_w))
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(SCC_TAG, scc85230_device, rxa_w))

	MCFG_RS232_PORT_ADD(RS232B_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(SCC_TAG, scc85230_device, ctsb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(SCC_TAG, scc85230_device, dcdb_w))
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(SCC_TAG, scc85230_device, rxb_w))

	MCFG_DEVICE_ADD(PI1_TAG, PC_LPT, 0)

	MCFG_DEVICE_ADD(KBDC_TAG, KBDC8042, 0)
	MCFG_KBDC8042_KEYBOARD_TYPE(KBDC8042_STANDARD)
	MCFG_KBDC8042_SYSTEM_RESET_CB(INPUTLINE("^maincpu", INPUT_LINE_RESET))

	MCFG_DEVICE_ADD(PIT_TAG, PIT8254, 0)
	MCFG_PIT8253_CLK0(1000000)
	MCFG_PIT8253_CLK1(1000000)
	MCFG_PIT8253_CLK2(1000000)
	MCFG_PIT8253_OUT2_HANDLER(DEVWRITELINE(KBDC_TAG, kbdc8042_device, write_out2))
MACHINE_CONFIG_END


ioc2_device::ioc2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t id)
	: device_t(mconfig, type, tag, owner, clock)
	, m_maincpu(*this, "^maincpu")
	, m_scc(*this, SCC_TAG)
	, m_pi1(*this, PI1_TAG)
	, m_kbdc(*this, KBDC_TAG)
	, m_pit(*this, PIT_TAG)
	, m_gen_ctrl_select_reg(0)
	, m_gen_ctrl_reg(0)
	, m_front_panel_reg(0)
	, m_read_reg(0)
	, m_dma_sel(0)
	, m_reset_reg(0)
	, m_write_reg(0)
	, m_int3_local0_status_reg(0)
	, m_int3_local0_mask_reg(0)
	, m_int3_local1_status_reg(0)
	, m_int3_local1_mask_reg(0)
	, m_int3_map_status_reg(0)
	, m_int3_map_mask0_reg(0)
	, m_int3_map_mask1_reg(0)
	, m_int3_map_pol_reg(0)
	, m_int3_timer_clear_reg(0)
	, m_int3_err_status_reg(0)
	, m_par_read_cnt(0)
	, m_par_cntl(0)
	, m_system_id(id)
{
}

#define FRONT_PANEL_POWER_STATE         (0x01)
#define FRONT_PANEL_POWER_BUTTON_INT    (0x02)
#define FRONT_PANEL_VOL_DOWN_INT        (0x10)
#define FRONT_PANEL_VOL_DOWN_HOLD       (0x20)
#define FRONT_PANEL_VOL_UP_INT          (0x40)
#define FRONT_PANEL_VOL_UP_HOLD         (0x80)

void ioc2_device::device_start()
{
	m_front_panel_reg = FRONT_PANEL_POWER_STATE;
}

void ioc2_device::device_reset()
{
	m_par_read_cnt = 0;
	m_par_cntl = 0;

	m_gen_ctrl_select_reg = 0;
	m_gen_ctrl_reg = 0;
	m_front_panel_reg = FRONT_PANEL_POWER_STATE;

	m_read_reg = 0;
	m_dma_sel = 0;
	m_reset_reg = 0;
	m_write_reg = 0;

	m_int3_local0_status_reg = 0;
	m_int3_local0_mask_reg = 0;
	m_int3_local1_status_reg = 0;
	m_int3_local1_mask_reg = 0;
	m_int3_map_status_reg = 0;
	m_int3_map_mask0_reg = 0;
	m_int3_map_mask1_reg = 0;
	m_int3_map_pol_reg = 0;
	m_int3_timer_clear_reg = 0;
	m_int3_err_status_reg = 0;
}

#define INT3_LOCAL0_FIFO    (0x01)
#define INT3_LOCAL0_SCSI0   (0x02)
#define INT3_LOCAL0_SCSI1   (0x04)
#define INT3_LOCAL0_ETHERNET    (0x08)
#define INT3_LOCAL0_MC_DMA  (0x10)
#define INT3_LOCAL0_PARALLEL    (0x20)
#define INT3_LOCAL0_GRAPHICS    (0x40)
#define INT3_LOCAL0_MAPPABLE0   (0x80)

#define INT3_LOCAL1_GP0     (0x01)
#define INT3_LOCAL1_PANEL   (0x02)
#define INT3_LOCAL1_GP2     (0x04)
#define INT3_LOCAL1_MAPPABLE1   (0x08)
#define INT3_LOCAL1_HPC_DMA     (0x10)
#define INT3_LOCAL1_AC_FAIL     (0x20)
#define INT3_LOCAL1_VSYNC   (0x40)
#define INT3_LOCAL1_RETRACE (0x80)

void ioc2_device::raise_local0_irq(uint8_t source_mask)
{
	m_int3_local0_status_reg |= source_mask;
	m_maincpu->set_input_line(MIPS3_IRQ0, (m_int3_local0_mask_reg & m_int3_local0_status_reg) != 0 ? ASSERT_LINE : CLEAR_LINE);
}

void ioc2_device::lower_local0_irq(uint8_t source_mask)
{
	m_int3_local0_status_reg &= ~source_mask;
}

void ioc2_device::raise_local1_irq(uint8_t source_mask)
{
	m_int3_local1_status_reg |= source_mask;
	m_maincpu->set_input_line(MIPS3_IRQ1, (m_int3_local1_mask_reg & m_int3_local1_status_reg) != 0 ? ASSERT_LINE : CLEAR_LINE);
}

void ioc2_device::lower_local1_irq(uint8_t source_mask)
{
	m_int3_local1_status_reg &= ~source_mask;
}

READ32_MEMBER( ioc2_device::read )
{
	switch (offset)
	{
		case 0x00/4: case 0x04/4: case 0x08/4:
			return m_pi1->read(space, offset, 0xff);

		case 0x0c/4: // Parallel Port DMA Control Register [docs?]
		case 0x10/4: // Parallel Port Interrupt Status Register [docs?]
		case 0x14/4: // Parallel Port Interrupt Mask Register [docs?]
		case 0x18/4: // Parallel Port Timer 1 Register [docs?]
		case 0x1c/4: // Parallel Port Timer 2 Register [docs?]
		case 0x20/4: // Parallel Port Timer 3 Register [docs?]
		case 0x24/4: // Parallel Port Timer 4 Register [docs?]
			return 0;

		case 0x30/4: // Serial Port1 Command Transfer
		case 0x34/4: // Serial Port1 Data Transfer
		case 0x38/4: // Serial Port2 Command Transfer
		case 0x3c/4: // Serial Port2 Data Transfer
			return m_scc->ba_cd_r(space, (offset - 0x30/4) ^ 3);

		case 0x40/4: // Keyboard/Mouse Registers
		case 0x44/4: // Keyboard/Mouse Registers
			return m_kbdc->data_r(space, (offset - 0x40/4) * 4);

		case 0x50/4: // Front Panel Register
			return m_front_panel_reg;

		case 0x58/4: // System ID Register
			return m_system_id;

		case 0x60/4: // Read Register
			return m_read_reg;

		case 0x68/4: // DMA_SEL Register
			// Bits 2-0 not quite understood, seem to be copy/paste error in SGI's own documents:
			//
			// 2    RW      Parallel Port DMA Select. A high bit selects the Parallel Port DMA channel. 0\h is the default after reset. [this makes sense. -ed.]
			// 1    RW      ISDN Channel B DMA Select. A high bit selects the Parallel Port DMA channel. 0\h is the default after reset. [is this a copy/paste error? perhaps "Parallel Port" should be "ISDN Channel B"?]
			// 0    RW      [same text as above. Another copy/paste error, maybe? Should be channel A, with the bit selecting DMA channel 0/1 for ISDN channel A, the and the same for ISDN channel B in bit 1?]
			return m_dma_sel;

		case 0x70/4: // Reset Register
			return m_reset_reg;

		case 0x78/4: // Write Register
			// Not yet implemented, some bits unnecessary:
			//
			// Bit  Oper    Description
			// 7    RW      Margin High. Set low for normal +5V operation, high to step supply up to +5.5V. Cleared at reset.
			// 6    RW      Margin Low. Set lowf or normal +5V operation, high to step supply down to +4.5V. Cleared at reset.
			// 5    RW      UART1 PC Mode. Set low to configure Port1 for RS422 Mac mode, high to select RS232 PC mode. Cleared at reset.
			// 4    RW      UART2 PC Mode. Set low to configure Port2 for RS422 Mac mode, high to select RS232 PC mode. Cleared at reset.
			// 3    RW      Ethernet Auto Select (active high). Set low for manual mode, high to have LXT901 automatically select TP or AUI based on link integrity. Cleared at reset.
			// 2    RW      Ethernet Port Select. Set low for TP, high for AUI. This setting is only used when Auto Select is in manual mode. Cleared at reset.
			// 1    RW      Ethernet UTP/STP select. Set low to select 150 ohm termination fro shielded TP (default), set high to select 100 ohm termination for unshielded TP. Cleared at reset.
			// 0    RW      Ethernet Normal Threshold (NTH) select. Set low to select the normal TP squelch threshold (default), high to reduce threshold by 4.5 dB (set low when reset).
			return m_write_reg;

		case 0x80/4: // INT3 Local0 Status Register
			return m_int3_local0_status_reg;

		case 0x84/4: // INT3 Local0 Mask Register
			return m_int3_local0_mask_reg;

		case 0x88/4: // INT3 Local1 Status Register
			return m_int3_local1_status_reg;

		case 0x8c/4: // INT3 Local1 Mask Register
			return m_int3_local1_mask_reg;

		case 0x90/4: // INT3 Map Status Register [TODO]
			return m_int3_map_status_reg;

		case 0x94/4: // INT3 Map Mask0 Register [TODO]
			return m_int3_map_mask0_reg;

		case 0x98/4: // INT3 Map Mask1 Register [TODO]
			return m_int3_map_mask1_reg;

		case 0x9c/4: // INT3 Map Pol Register [TODO]
			return m_int3_map_pol_reg;

		case 0xa0/4: // INT3 Timer Clear Register [TODO]
			return m_int3_timer_clear_reg;

		case 0xa4/4: // INT3 Error Status Register [TODO]
			return m_int3_err_status_reg;

		case 0xb0/4: // Timer Counter 0 Register
		case 0xb4/4: // Timer Counter 1 Register
		case 0xb8/4: // Timer Counter 2 Register
		case 0xbc/4: // Timer Control Word Register
			return m_pit->read(space, offset - 0xb0/4);
	}

	return 0;
}

#define DMA_SEL_CLOCK_SEL_MASK      (0x30)
#define DMA_SEL_CLOCK_SEL_10MHz     (0x00)
#define DMA_SEL_CLOCK_SEL_6_67MHz   (0x10)
#define DMA_SEL_CLOCK_SEL_EXT       (0x20)

WRITE32_MEMBER( ioc2_device::write )
{
	switch (offset)
	{
		case 0x00/4: case 0x04/4: case 0x08/4:
			m_pi1->write(space, offset, data & 0xff, 0xff);
			return;

		case 0x0c/4: // Parallel Port DMA Control Register [docs?]
		case 0x10/4: // Parallel Port Interrupt Status Register [docs?]
		case 0x14/4: // Parallel Port Interrupt Mask Register [docs?]
		case 0x18/4: // Parallel Port Timer 1 Register [docs?]
		case 0x1c/4: // Parallel Port Timer 2 Register [docs?]
		case 0x20/4: // Parallel Port Timer 3 Register [docs?]
		case 0x24/4: // Parallel Port Timer 4 Register [docs?]
			return;

		case 0x30/4: // Serial Port1 Command Transfer
		case 0x34/4: // Serial Port1 Data Transfer
		case 0x38/4: // Serial Port2 Command Transfer
		case 0x3c/4: // Serial Port2 Data Transfer
			m_scc->ba_cd_w(space, (offset - 0x30/4) ^ 3, data & 0xff);
			return;

		case 0x40/4: // Keyboard/Mouse Registers
		case 0x44/4: // Keyboard/Mouse Registers
			m_kbdc->data_w(space, (offset - 0x40/4) * 4, data & 0xff);
			return;

		case 0x50/4: // Front Panel Register
			m_front_panel_reg &= ~(data & (FRONT_PANEL_VOL_UP_INT | FRONT_PANEL_VOL_DOWN_INT | FRONT_PANEL_POWER_BUTTON_INT));
			return;

		case 0x68/4: // DMA_SEL Register
		{
			// Bits 2-0 not quite understood, seem to be copy/paste error in SGI's own documents:
			//
			// 5:4  RW      Serial Port Clock Select: 00 selects a 10MHz internal clock (default), 01 selects a 6.67MHz internal clock, and 02 or 03 selects the external clock input.
			// 2    RW      Parallel Port DMA Select. A high bit selects the Parallel Port DMA channel. 0\h is the default after reset. [this makes sense. -ed.]
			// 1    RW      ISDN Channel B DMA Select. A high bit selects the Parallel Port DMA channel. 0\h is the default after reset. [is this a copy/paste error? perhaps "Parallel Port" should be "ISDN Channel B"?]
			// 0    RW      [same text as above. Another copy/paste error, maybe? Should be channel A, with the bit selecting DMA channel 0/1 for ISDN channel A, the and the same for ISDN channel B in bit 1?]
			uint8_t old = m_dma_sel;
			m_dma_sel = data;
			uint8_t diff = old ^ m_dma_sel;
			if (diff & DMA_SEL_CLOCK_SEL_MASK)
			{
				if (diff & DMA_SEL_CLOCK_SEL_EXT)
				{
					printf("External clock select %sselected\n", (old & DMA_SEL_CLOCK_SEL_EXT) != 0 ? "de" : "");
					// TODO: verify the external Rx/Tx clock, is it fixed or programmable?
				}
			}
			// TODO: Currently we always assume a 10MHz clock as PCLK
			return;
		}

		case 0x70/4: // Reset Register
			handle_reset_reg_write(data);
			return;

		case 0x78/4: // Write Register
			m_write_reg = data;
			return;

		case 0x80/4: // INT3 Local0 Status Register
		case 0x88/4: // INT3 Local1 Status Register
		case 0x90/4: // INT3 Map Status Register
		case 0xa4/4: // INT3 Error Stat Register
			// Read-only registers
			return;

		case 0x84/4: // INT3 Local0 Mask Register
		{
			uint8_t old = m_int3_local0_mask_reg;
			m_int3_local0_mask_reg = data;
			bool old_line = (old & m_int3_local0_status_reg) != 0;
			bool new_line = (m_int3_local0_mask_reg & m_int3_local0_status_reg) != 0;
			if (old_line != new_line)
				m_maincpu->set_input_line(MIPS3_IRQ0, (m_int3_local0_mask_reg & m_int3_local0_status_reg) != 0 ? ASSERT_LINE : CLEAR_LINE);
			return;
		}

		case 0x8c/4: // INT3 Local1 Mask Register
		{
			uint8_t old = m_int3_local0_mask_reg;
			m_int3_local0_mask_reg = data;
			bool old_line = (old & m_int3_local0_status_reg) != 0;
			bool new_line = (m_int3_local0_mask_reg & m_int3_local0_status_reg) != 0;
			if (old_line != new_line)
				m_maincpu->set_input_line(MIPS3_IRQ0, (m_int3_local0_mask_reg & m_int3_local0_status_reg) != 0 ? ASSERT_LINE : CLEAR_LINE);
			return;
		}

		case 0x94/4: // INT3 Map Mask0 Register
			// TODO: Implement mappable interrupts
			m_int3_map_mask0_reg = data;
			return;

		case 0x98/4: // INT3 Map Mask1 Register
			// TODO: Implement mappable interrupts
			m_int3_map_mask1_reg = data;
			return;

		case 0x9c/4: // INT3 Map Pol Register
			// TODO: Mappable interrupt polarity select
			m_int3_map_pol_reg = data;
			return;

		case 0xb0/4: // Timer Counter 0 Register
		case 0xb4/4: // Timer Counter 1 Register
		case 0xb8/4: // Timer Counter 2 Register
		case 0xbc/4: // Timer Control Word Register
			m_pit->write(space, offset - 0xb0/4, data & 0xff);
			return;
	}
}

void ioc2_device::handle_reset_reg_write(uint8_t data)
{
	// guinness/fullhouse-specific implementations can handle bit 3 being used for ISDN reset on Indy only and bit 2 for EISA reset on Indigo 2 only, but for now we do nothing with it
	m_reset_reg = data;
}

INPUT_CHANGED_MEMBER( ioc2_device::power_button )
{
	if (!newval)
	{
		m_front_panel_reg |= FRONT_PANEL_POWER_BUTTON_INT;
	}
}

INPUT_CHANGED_MEMBER( ioc2_device::volume_up )
{
	if (!newval)
	{
		m_front_panel_reg |= FRONT_PANEL_VOL_UP_INT;
		m_front_panel_reg |= FRONT_PANEL_VOL_UP_HOLD;
	}
	else
	{
		m_front_panel_reg &= ~FRONT_PANEL_VOL_UP_HOLD;
	}
}

INPUT_CHANGED_MEMBER( ioc2_device::volume_down )
{
	if (!newval)
	{
		m_front_panel_reg |= FRONT_PANEL_VOL_DOWN_INT;
		m_front_panel_reg |= FRONT_PANEL_VOL_DOWN_HOLD;
	}
	else
	{
		m_front_panel_reg &= ~FRONT_PANEL_VOL_DOWN_HOLD;
	}
}

#define IOC2_TAG    "ioc2"
#define RTC_TAG     "ds1386"

struct hpc3_t
{
	uint32_t m_enetr_nbdp;
	uint32_t m_enetr_cbp;
	uint32_t m_unk0;
	uint32_t m_unk1;
	uint32_t m_ic_unk0;
	uint32_t m_scsi0_desc;
	uint32_t m_scsi0_dma_ctrl;
};

struct hal2
{
	uint32_t m_iar;
	uint32_t m_idr[4];
};

struct pbus_dma_t
{
	uint8_t m_active;
	uint32_t m_cur_ptr;
	uint32_t m_desc_ptr;
	uint32_t m_next_ptr;
	uint32_t m_words_left;
};

class ip22_state : public driver_device
{
public:
	enum
	{
		TIMER_IP22_DMA
	};

	ip22_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_wd33c93(*this, "wd33c93")
		, m_unkpbus0(*this, "unkpbus0")
		, m_mainram(*this, "mainram")
		, m_sgi_mc(*this, "sgi_mc")
		, m_newport(*this, "newport")
		, m_dac(*this, "dac")
		, m_ioc2(*this, IOC2_TAG)
		, m_rtc(*this, RTC_TAG)
	{
	}

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ32_MEMBER(hpc3_hd_enet_r);
	DECLARE_WRITE32_MEMBER(hpc3_hd_enet_w);
	DECLARE_READ32_MEMBER(hpc3_hd0_r);
	DECLARE_WRITE32_MEMBER(hpc3_hd0_w);
	DECLARE_READ32_MEMBER(hpc3_pbus4_r);
	DECLARE_WRITE32_MEMBER(hpc3_pbus4_w);
	DECLARE_READ32_MEMBER(hpc3_pbusdma_r);
	DECLARE_WRITE32_MEMBER(hpc3_pbusdma_w);
	DECLARE_READ32_MEMBER(hpc3_unkpbus0_r);
	DECLARE_WRITE32_MEMBER(hpc3_unkpbus0_w);

	DECLARE_WRITE32_MEMBER(ip22_write_ram);

	DECLARE_READ32_MEMBER(hal2_r);
	DECLARE_WRITE32_MEMBER(hal2_w);

	DECLARE_WRITE_LINE_MEMBER(scsi_irq);

	DECLARE_DRIVER_INIT(ip225015);

	TIMER_CALLBACK_MEMBER(ip22_dma);

	static void cdrom_config(device_t *device);
	void ip225015(machine_config &config);
	void ip224613(machine_config &config);
	void ip244415(machine_config &config);
	void ip225015_map(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	required_device<mips3_device> m_maincpu;
	required_device<wd33c93_device> m_wd33c93;
	required_shared_ptr<uint32_t> m_unkpbus0;
	required_shared_ptr<uint32_t> m_mainram;
	required_device<sgi_mc_device> m_sgi_mc;
	required_device<newport_video_device> m_newport;
	required_device<dac_word_interface> m_dac;
	required_device<ioc2_device> m_ioc2;
	required_device<ds1386_device> m_rtc;

	void dump_chain(address_space &space, uint32_t ch_base);

	hpc3_t m_hpc3;

	hal2 m_hal2;

	pbus_dma_t m_pbus_dma;
	uint8_t m_dma_buffer[4096];

	inline void ATTR_PRINTF(3,4) verboselog(int n_level, const char *s_fmt, ... );
};


#define VERBOSE_LEVEL ( 0 )


inline void ATTR_PRINTF(3,4) ip22_state::verboselog(int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror("%08x: %s", m_maincpu->pc(), buf);
	}
}


READ32_MEMBER(ip22_state::hpc3_hd_enet_r)
{
	switch( offset )
	{
	case 0x0004/4:
		//verboselog((machine, 0, "HPC3 SCSI0DESC Read: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2), mem_mask, m_hpc3.m_scsi0_desc );
		return m_hpc3.m_scsi0_desc;
	case 0x1004/4:
		//verboselog((machine, 0, "HPC3 SCSI0DMACTRL Read: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2), mem_mask, m_hpc3.m_scsi0_dma_ctrl );
		return m_hpc3.m_scsi0_dma_ctrl;
	case 0x4000/4:
		//verboselog((machine, 2, "HPC3 ENETR CBP Read: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2), mem_mask, m_hpc3.m_enetr_nbdp );
		return m_hpc3.m_enetr_cbp;
	case 0x4004/4:
		//verboselog((machine, 2, "HPC3 ENETR NBDP Read: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2), mem_mask, m_hpc3.m_enetr_nbdp );
		return m_hpc3.m_enetr_nbdp;
	default:
		//verboselog((machine, 0, "Unknown HPC3 ENET/HDx Read: %08x (%08x)\n", 0x1fb90000 + ( offset << 2 ), mem_mask );
		return 0;
	}
}

WRITE32_MEMBER(ip22_state::hpc3_hd_enet_w)
{
	switch( offset )
	{
	case 0x0004/4:
		//verboselog((machine, 2, "HPC3 SCSI0DESC Write: %08x\n", data );
		m_hpc3.m_scsi0_desc = data;
		break;
	case 0x1004/4:
		//verboselog((machine, 2, "HPC3 SCSI0DMACTRL Write: %08x\n", data );
		m_hpc3.m_scsi0_dma_ctrl = data;
		break;
	case 0x4000/4:
		//verboselog((machine, 2, "HPC3 ENETR CBP Write: %08x\n", data );
		m_hpc3.m_enetr_cbp = data;
		break;
	case 0x4004/4:
		//verboselog((machine, 2, "HPC3 ENETR NBDP Write: %08x\n", data );
		m_hpc3.m_enetr_nbdp = data;
		break;
	default:
		//verboselog((machine, 0, "Unknown HPC3 ENET/HDx write: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2 ), mem_mask, data );
		break;
	}
}

READ32_MEMBER(ip22_state::hpc3_hd0_r)
{
	switch( offset )
	{
	case 0x0000/4:
	case 0x4000/4:
//      //verboselog((machine, 2, "HPC3 HD0 Status Read: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2), mem_mask, nHPC3_hd0_regs[0x17] );
		if (ACCESSING_BITS_0_7)
		{
			return m_wd33c93->read( space, 0 );
		}
		else
		{
			return 0;
		}
	case 0x0004/4:
	case 0x4004/4:
//      //verboselog((machine, 2, "HPC3 HD0 Register Read: %08x (%08x): %08x\n", 0x1fb90000 + ( offset << 2), mem_mask, nHPC3_hd0_regs[nHPC3_hd0_register] );
		if (ACCESSING_BITS_0_7)
		{
			return m_wd33c93->read( space, 1 );
		}
		else
		{
			return 0;
		}
	default:
		//verboselog((machine, 0, "Unknown HPC3 HD0 Read: %08x (%08x) [%x] PC=%x\n", 0x1fbc0000 + ( offset << 2 ), mem_mask, offset, m_maincpu->pc() );
		return 0;
	}
}

WRITE32_MEMBER(ip22_state::hpc3_hd0_w)
{
	switch( offset )
	{
	case 0x0000/4:
	case 0x4000/4:
//      //verboselog((machine, 2, "HPC3 HD0 Register Select Write: %08x\n", data );
		if (ACCESSING_BITS_0_7)
		{
			m_wd33c93->write( space, 0, data & 0x000000ff );
		}
		break;
	case 0x0004/4:
	case 0x4004/4:
//      //verboselog((machine, 2, "HPC3 HD0 Register %d Write: %08x\n", nHPC3_hd0_register, data );
		if (ACCESSING_BITS_0_7)
		{
			m_wd33c93->write( space, 1,  data & 0x000000ff );
		}
		break;
	default:
		//verboselog((machine, 0, "Unknown HPC3 HD0 Write: %08x (%08x): %08x\n", 0x1fbc0000 + ( offset << 2 ), mem_mask, data );
		break;
	}
}


READ32_MEMBER(ip22_state::hpc3_pbus4_r)
{
	switch( offset )
	{
	case 0x0004/4:
		//verboselog((machine, 2, "HPC3 PBUS4 Unknown 0 Read: (%08x): %08x\n", mem_mask, m_hpc3.m_unk0 );
		return m_hpc3.m_unk0;
	case 0x000c/4:
		//verboselog((machine, 2, "Interrupt Controller(?) Read: (%08x): %08x\n", mem_mask, m_hpc3.m_ic_unk0 );
		return m_hpc3.m_ic_unk0;
	case 0x0014/4:
		//verboselog((machine, 2, "HPC3 PBUS4 Unknown 1 Read: (%08x): %08x\n", mem_mask, m_hpc3.m_unk1 );
		return m_hpc3.m_unk1;
	default:
		//verboselog((machine, 0, "Unknown HPC3 PBUS4 Read: %08x (%08x)\n", 0x1fbd9000 + ( offset << 2 ), mem_mask );
		return 0;
	}
}

WRITE32_MEMBER(ip22_state::hpc3_pbus4_w)
{
	switch( offset )
	{
	case 0x0004/4:
		//verboselog((machine, 2, "HPC3 PBUS4 Unknown 0 Write: %08x (%08x)\n", data, mem_mask );
		m_hpc3.m_unk0 = data;
		break;
	case 0x000c/4:
		//verboselog((machine, 2, "Interrupt Controller(?) Write: (%08x): %08x\n", mem_mask, data );
		m_hpc3.m_ic_unk0 = data;
		break;
	case 0x0014/4:
		//verboselog((machine, 2, "HPC3 PBUS4 Unknown 1 Write: %08x (%08x)\n", data, mem_mask );
		m_hpc3.m_unk1 = data;
		break;
	default:
		//verboselog((machine, 0, "Unknown HPC3 PBUS4 Write: %08x (%08x): %08x\n", 0x1fbd9000 + ( offset << 2 ), mem_mask, data );
		break;
	}
}

// a bit hackish, but makes the memory detection work properly and allows a big cleanup of the mapping
WRITE32_MEMBER(ip22_state::ip22_write_ram)
{
	// if banks 2 or 3 are enabled, do nothing, we don't support that much memory
	if (m_sgi_mc->read(space, 0xc8/4, 0xffffffff) & 0x10001000)
	{
		// a random perturbation so the memory test fails
		data ^= 0xffffffff;
	}

	// if banks 0 or 1 have 2 membanks, also kill it, we only want 128 MB
	if (m_sgi_mc->read(space, 0xc0/4, 0xffffffff) & 0x40004000)
	{
		// a random perturbation so the memory test fails
		data ^= 0xffffffff;
	}
	COMBINE_DATA(&m_mainram[offset]);
}


#define H2_IAR_TYPE         0xf000
#define H2_IAR_NUM          0x0f00
#define H2_IAR_ACCESS_SEL   0x0080
#define H2_IAR_PARAM        0x000c
#define H2_IAR_RB_INDEX     0x0003

#define H2_ISR_TSTATUS      0x01
#define H2_ISR_USTATUS      0x02
#define H2_ISR_QUAD_MODE    0x04
#define H2_ISR_GLOBAL_RESET 0x08
#define H2_ISR_CODEC_RESET  0x10

READ32_MEMBER(ip22_state::hal2_r)
{
	switch( offset )
	{
	case 0x0010/4:
		//verboselog((machine, 0, "HAL2 Status read: 0x0004\n" );
		return 0x0004;
	case 0x0020/4:
		//verboselog((machine, 0, "HAL2 Revision read: 0x4011\n" );
		return 0x4011;
	}
	//verboselog((machine, 0, "Unknown HAL2 read: 0x%08x (%08x)\n", 0x1fbd8000 + offset*4, mem_mask );
	return 0;
}

WRITE32_MEMBER(ip22_state::hal2_w)
{
	switch( offset )
	{
	case 0x0010/4:
		//verboselog((machine, 0, "HAL2 Status Write: 0x%08x (%08x)\n", data, mem_mask );
		if( data & H2_ISR_GLOBAL_RESET )
		{
			//verboselog((machine, 0, "    HAL2 Global Reset\n" );
		}
		if( data & H2_ISR_CODEC_RESET )
		{
			//verboselog((machine, 0, "    HAL2 Codec Reset\n" );
		}
		break;
	case 0x0030/4:
		//verboselog((machine, 0, "HAL2 Indirect Address Register Write: 0x%08x (%08x)\n", data, mem_mask );
		m_hal2.m_iar = data;
		switch( data & H2_IAR_TYPE )
		{
		case 0x1000:
			//verboselog((machine, 0, "    DMA Port\n" );
			switch( data & H2_IAR_NUM )
			{
			case 0x0100:
				//verboselog((machine, 0, "        Synth In\n" );
				break;
			case 0x0200:
				//verboselog((machine, 0, "        AES In\n" );
				break;
			case 0x0300:
				//verboselog((machine, 0, "        AES Out\n" );
				break;
			case 0x0400:
				//verboselog((machine, 0, "        DAC Out\n" );
				break;
			case 0x0500:
				//verboselog((machine, 0, "        ADC Out\n" );
				break;
			case 0x0600:
				//verboselog((machine, 0, "        Synth Control\n" );
				break;
			}
			break;
		case 0x2000:
			//verboselog((machine, 0, "    Bresenham\n" );
			switch( data & H2_IAR_NUM )
			{
			case 0x0100:
				//verboselog((machine, 0, "        Bresenham Clock Gen 1\n" );
				break;
			case 0x0200:
				//verboselog((machine, 0, "        Bresenham Clock Gen 2\n" );
				break;
			case 0x0300:
				//verboselog((machine, 0, "        Bresenham Clock Gen 3\n" );
				break;
			}
			break;
		case 0x3000:
			//verboselog((machine, 0, "    Unix Timer\n" );
			switch( data & H2_IAR_NUM )
			{
			case 0x0100:
				//verboselog((machine, 0, "        Unix Timer\n" );
				break;
			}
			break;
		case 0x9000:
			//verboselog((machine, 0, "    Global DMA Control\n" );
			switch( data & H2_IAR_NUM )
			{
			case 0x0100:
				//verboselog((machine, 0, "        DMA Control\n" );
				break;
			}
			break;
		}
		switch( data & H2_IAR_ACCESS_SEL )
		{
		case 0x0000:
			//verboselog((machine, 0, "    Write\n" );
			break;
		case 0x0080:
			//verboselog((machine, 0, "    Read\n" );
			break;
		}
		//verboselog((machine, 0, "    Parameter: %01x\n", ( data & H2_IAR_PARAM ) >> 2 );
		return;
		/* FIXME: this code is never excuted */
		//verboselog((machine, 0, "    Read Back Index: %01x\n", ( data & H2_IAR_RB_INDEX ) );
		//break;
	case 0x0040/4:
		//verboselog((machine, 0, "HAL2 Indirect Data Register 0 Write: 0x%08x (%08x)\n", data, mem_mask );
		m_hal2.m_idr[0] = data;
		return;
	case 0x0050/4:
		//verboselog((machine, 0, "HAL2 Indirect Data Register 1 Write: 0x%08x (%08x)\n", data, mem_mask );
		m_hal2.m_idr[1] = data;
		return;
	case 0x0060/4:
		//verboselog((machine, 0, "HAL2 Indirect Data Register 2 Write: 0x%08x (%08x)\n", data, mem_mask );
		m_hal2.m_idr[2] = data;
		return;
	case 0x0070/4:
		//verboselog((machine, 0, "HAL2 Indirect Data Register 3 Write: 0x%08x (%08x)\n", data, mem_mask );
		m_hal2.m_idr[3] = data;
		return;
	}
	//verboselog((machine, 0, "Unknown HAL2 write: 0x%08x: 0x%08x (%08x)\n", 0x1fbd8000 + offset*4, data, mem_mask );
}

#define PBUS_CTRL_ENDIAN        0x00000002
#define PBUS_CTRL_RECV          0x00000004
#define PBUS_CTRL_FLUSH         0x00000008
#define PBUS_CTRL_DMASTART      0x00000010
#define PBUS_CTRL_LOAD_EN       0x00000020
#define PBUS_CTRL_REALTIME      0x00000040
#define PBUS_CTRL_HIGHWATER     0x0000ff00
#define PBUS_CTRL_FIFO_BEG      0x003f0000
#define PBUS_CTRL_FIFO_END      0x3f000000

#define PBUS_DMADESC_EOX        0x80000000
#define PBUS_DMADESC_EOXP       0x40000000
#define PBUS_DMADESC_XIE        0x20000000
#define PBUS_DMADESC_IPG        0x00ff0000
#define PBUS_DMADESC_TXD        0x00008000
#define PBUS_DMADESC_BC         0x00003fff


void ip22_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_IP22_DMA:
		ip22_dma(ptr, param);
		break;
	default:
		assert_always(false, "Unknown id in ip22_state::device_timer");
	}
}

TIMER_CALLBACK_MEMBER(ip22_state::ip22_dma)
{
	timer_set(attotime::never, TIMER_IP22_DMA);
#if 0
	if( m_pbus_dma.m_active )
	{
		uint16_t temp16 = ( m_mainram[(m_pbus_dma.m_cur_ptr - 0x08000000)/4] & 0xffff0000 ) >> 16;
		int16_t stemp16 = (int16_t)((temp16 >> 8) | (temp16 << 8));

		m_dac->write_signed16(stemp16);

		m_pbus_dma.m_cur_ptr += 4;

		m_pbus_dma.m_words_left -= 4;
		if( m_pbus_dma.m_words_left == 0 )
		{
			if( m_pbus_dma.m_next_ptr != 0 )
			{
				m_pbus_dma.m_desc_ptr = m_pbus_dma.m_next_ptr;
				m_pbus_dma.m_cur_ptr = m_mainram[(m_pbus_dma.m_desc_ptr - 0x08000000)/4];
				m_pbus_dma.m_words_left = m_mainram[(m_pbus_dma.m_desc_ptr - 0x08000000)/4+1];
				m_pbus_dma.m_next_ptr = m_mainram[(m_pbus_dma.m_desc_ptr - 0x08000000)/4+2];
			}
			else
			{
				m_pbus_dma.m_active = 0;
				return;
			}
		}
		timer_set(attotime::from_hz(44100), TIMER_IP22_DMA);
	}
#endif
}

READ32_MEMBER(ip22_state::hpc3_pbusdma_r)
{
	//uint32_t channel = offset / (0x2000/4);
	//verboselog((machine(), 0, "PBUS DMA Channel %d Read: 0x%08x (%08x)\n", channel, 0x1fb80000 + offset*4, mem_mask );
	return 0;
}

WRITE32_MEMBER(ip22_state::hpc3_pbusdma_w)
{
	uint32_t channel = offset / (0x2000/4);

	switch( offset & 0x07ff )
	{
	case 0x0000/4:
		//verboselog((machine, 0, "PBUS DMA Channel %d Buffer Pointer Write: 0x%08x\n", channel, data );
		return;
	case 0x0004/4:
		//verboselog((machine, 0, "PBUS DMA Channel %d Descriptor Pointer Write: 0x%08x\n", channel, data );
		if( channel == 1 )
		{
			m_pbus_dma.m_desc_ptr = data;
			m_pbus_dma.m_cur_ptr = m_mainram[(m_pbus_dma.m_desc_ptr - 0x08000000)/4];
			m_pbus_dma.m_words_left = m_mainram[(m_pbus_dma.m_desc_ptr - 0x08000000)/4+1];
			m_pbus_dma.m_next_ptr = m_mainram[(m_pbus_dma.m_desc_ptr - 0x08000000)/4+2];
			//verboselog((machine, 0, "nPBUS_DMA_DescPtr = %08x\n", m_pbus_dma.m_desc_ptr );
			//verboselog((machine, 0, "nPBUS_DMA_CurPtr = %08x\n", m_pbus_dma.m_cur_ptr );
			//verboselog((machine, 0, "nPBUS_DMA_WordsLeft = %08x\n", m_pbus_dma.m_words_left );
			//verboselog((machine, 0, "nPBUS_DMA_NextPtr = %08x\n", m_pbus_dma.m_next_ptr );
		}
		return;
	case 0x1000/4:
		//verboselog((machine, 0, "PBUS DMA Channel %d Control Register Write: 0x%08x\n", channel, data );
		if( data & PBUS_CTRL_ENDIAN )
		{
			//verboselog((machine, 0, "    Little Endian\n" );
		}
		else
		{
			//verboselog((machine, 0, "    Big Endian\n" );
		}
		if( data & PBUS_CTRL_RECV )
		{
			//verboselog((machine, 0, "    RX DMA\n" );
		}
		else
		{
			//verboselog((machine, 0, "    TX DMA\n" );
		}
		if( data & PBUS_CTRL_FLUSH )
		{
			//verboselog((machine, 0, "    Flush for RX\n" );
		}
		if( data & PBUS_CTRL_DMASTART )
		{
			//verboselog((machine, 0, "    Start DMA\n" );
		}
		if( data & PBUS_CTRL_LOAD_EN )
		{
			//verboselog((machine, 0, "    Load Enable\n" );
		}
		//verboselog((machine, 0, "    High Water Mark: %04x bytes\n", ( data & PBUS_CTRL_HIGHWATER ) >> 8 );
		//verboselog((machine, 0, "    FIFO Begin: Row %04x\n", ( data & PBUS_CTRL_FIFO_BEG ) >> 16 );
		//verboselog((machine, 0, "    FIFO End: Rowe %04x\n", ( data & PBUS_CTRL_FIFO_END ) >> 24 );
		if( ( data & PBUS_CTRL_DMASTART ) || ( data & PBUS_CTRL_LOAD_EN ) )
		{
			timer_set(attotime::from_hz(44100), TIMER_IP22_DMA);
			m_pbus_dma.m_active = 1;
		}
		return;
	}
	//verboselog((machine, 0, "Unknown PBUS DMA Channel %d Write: 0x%08x: 0x%08x (%08x)\n", channel, 0x1fb80000 + offset*4, data, mem_mask );
}

READ32_MEMBER(ip22_state::hpc3_unkpbus0_r)
{
	return 0;
	////verboselog((machine(), 0, "Unknown PBUS Read: 0x%08x (%08x)\n", 0x1fbc8000 + offset*4, mem_mask );
	//return m_unkpbus0[offset];
}

WRITE32_MEMBER(ip22_state::hpc3_unkpbus0_w)
{
	////verboselog((machine(), 0, "Unknown PBUS Write: 0x%08x = 0x%08x (%08x)\n", 0x1fbc8000 + offset*4, data, mem_mask );
	//COMBINE_DATA(&m_unkpbus0[offset]);
}

ADDRESS_MAP_START(ip22_state::ip225015_map)
	AM_RANGE( 0x00000000, 0x0007ffff ) AM_RAMBANK( "bank1" )    /* mirror of first 512k of main RAM */
	AM_RANGE( 0x08000000, 0x0fffffff ) AM_SHARE("mainram") AM_RAM_WRITE(ip22_write_ram)     /* 128 MB of main RAM */
	AM_RANGE( 0x1f0f0000, 0x1f0f1fff ) AM_DEVREADWRITE("newport", newport_video_device, rex3_r, rex3_w )
	AM_RANGE( 0x1fa00000, 0x1fa1ffff ) AM_DEVREADWRITE("sgi_mc", sgi_mc_device, read, write )
	AM_RANGE( 0x1fb90000, 0x1fb9ffff ) AM_READWRITE(hpc3_hd_enet_r, hpc3_hd_enet_w )
	AM_RANGE( 0x1fbb0000, 0x1fbb0003 ) AM_RAM   /* unknown, but read a lot and discarded */
	AM_RANGE( 0x1fbc0000, 0x1fbc7fff ) AM_READWRITE(hpc3_hd0_r, hpc3_hd0_w )
	AM_RANGE( 0x1fbc8000, 0x1fbcffff ) AM_READWRITE(hpc3_unkpbus0_r, hpc3_unkpbus0_w ) AM_SHARE("unkpbus0")
	AM_RANGE( 0x1fb80000, 0x1fb8ffff ) AM_READWRITE(hpc3_pbusdma_r, hpc3_pbusdma_w )
	AM_RANGE( 0x1fbd8000, 0x1fbd83ff ) AM_READWRITE(hal2_r, hal2_w )
	AM_RANGE( 0x1fbd8400, 0x1fbd87ff ) AM_RAM /* hack */
	AM_RANGE( 0x1fbd9000, 0x1fbd93ff ) AM_READWRITE(hpc3_pbus4_r, hpc3_pbus4_w )
	AM_RANGE( 0x1fbd9800, 0x1fbd9bff ) AM_DEVREADWRITE(IOC2_TAG, ioc2_device, read, write)
	AM_RANGE( 0x1fbdc000, 0x1fbdc7ff ) AM_RAM
	AM_RANGE( 0x1fbdd000, 0x1fbdd3ff ) AM_RAM
	AM_RANGE( 0x1fbe0000, 0x1fbe04ff ) AM_DEVREADWRITE8(RTC_TAG, ds1386_device, data_r, data_w, 0x000000ff)
	AM_RANGE( 0x1fc00000, 0x1fc7ffff ) AM_ROM AM_REGION( "user1", 0 )
	AM_RANGE( 0x20000000, 0x27ffffff ) AM_SHARE("mainram") AM_RAM_WRITE(ip22_write_ram)
ADDRESS_MAP_END


void ip22_state::machine_reset()
{
	m_hpc3.m_enetr_nbdp = 0x80000000;
	m_hpc3.m_enetr_cbp = 0x80000000;

	// set up low RAM mirror
	membank("bank1")->set_base(m_mainram);

	m_pbus_dma.m_active = 0;

	m_maincpu->mips3drc_set_options(MIPS3DRC_COMPATIBLE_OPTIONS | MIPS3DRC_CHECK_OVERFLOWS);
}

void ip22_state::dump_chain(address_space &space, uint32_t ch_base)
{
	printf("node: %08x %08x %08x (len = %x)\n", space.read_dword(ch_base), space.read_dword(ch_base+4), space.read_dword(ch_base+8), space.read_dword(ch_base+4) & 0x3fff);

	if ((space.read_dword(ch_base+8) != 0) && !(space.read_dword(ch_base+4) & 0x80000000))
	{
		dump_chain(space, space.read_dword(ch_base+8));
	}
}

// HPC3 SCSI DMA control register bits
#define HPC3_DMACTRL_IRQ    (0x01)
#define HPC3_DMACTRL_ENDIAN (0x02)
#define HPC3_DMACTRL_DIR    (0x04)
#define HPC3_DMACTRL_ENABLE (0x10)


WRITE_LINE_MEMBER(ip22_state::scsi_irq)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	if (state)
	{
		if (m_wd33c93->get_dma_count())
		{
			printf("m_wd33c93->get_dma_count() is %d\n", m_wd33c93->get_dma_count() );
			if (m_hpc3.m_scsi0_dma_ctrl & HPC3_DMACTRL_ENABLE)
			{
				if (m_hpc3.m_scsi0_dma_ctrl & HPC3_DMACTRL_IRQ) logerror("IP22: Unhandled SCSI DMA IRQ\n");
			}

			// HPC3 DMA: host to device
			if ((m_hpc3.m_scsi0_dma_ctrl & HPC3_DMACTRL_ENABLE) && (m_hpc3.m_scsi0_dma_ctrl & HPC3_DMACTRL_DIR))
			{
				uint32_t wptr, tmpword;
				int words, dptr, twords;

				words = m_wd33c93->get_dma_count();
				words /= 4;

				wptr = space.read_dword(m_hpc3.m_scsi0_desc);
				m_hpc3.m_scsi0_desc += words*4;
				dptr = 0;

				printf("DMA to device: %d words @ %x\n", words, wptr);

				dump_chain(space, m_hpc3.m_scsi0_desc);

				if (words <= (512/4))
				{
					// one-shot
					//m_wd33c93->dma_read_data(m_wd33c93->get_dma_count(), m_dma_buffer);

					while (words)
					{
						tmpword = space.read_dword(wptr);

						if (m_hpc3.m_scsi0_dma_ctrl & HPC3_DMACTRL_ENDIAN)
						{
							m_dma_buffer[dptr+3] = (tmpword>>24)&0xff;
							m_dma_buffer[dptr+2] = (tmpword>>16)&0xff;
							m_dma_buffer[dptr+1] = (tmpword>>8)&0xff;
							m_dma_buffer[dptr] = tmpword&0xff;
						}
						else
						{
							m_dma_buffer[dptr] = (tmpword>>24)&0xff;
							m_dma_buffer[dptr+1] = (tmpword>>16)&0xff;
							m_dma_buffer[dptr+2] = (tmpword>>8)&0xff;
							m_dma_buffer[dptr+3] = tmpword&0xff;
						}

						wptr += 4;
						dptr += 4;
						words--;
					}

					words = m_wd33c93->get_dma_count();
					m_wd33c93->dma_write_data(words, m_dma_buffer);
				}
				else
				{
					while (words)
					{
						//m_wd33c93->dma_read_data(512, m_dma_buffer);
						twords = 512/4;
						m_hpc3.m_scsi0_desc += 512;
						dptr = 0;

						while (twords)
						{
							tmpword = space.read_dword(wptr);

							if (m_hpc3.m_scsi0_dma_ctrl & HPC3_DMACTRL_ENDIAN)
							{
								m_dma_buffer[dptr+3] = (tmpword>>24)&0xff;
								m_dma_buffer[dptr+2] = (tmpword>>16)&0xff;
								m_dma_buffer[dptr+1] = (tmpword>>8)&0xff;
								m_dma_buffer[dptr] = tmpword&0xff;
							}
							else
							{
								m_dma_buffer[dptr] = (tmpword>>24)&0xff;
								m_dma_buffer[dptr+1] = (tmpword>>16)&0xff;
								m_dma_buffer[dptr+2] = (tmpword>>8)&0xff;
								m_dma_buffer[dptr+3] = tmpword&0xff;
							}

							wptr += 4;
							dptr += 4;
							twords--;
						}

						m_wd33c93->dma_write_data(512, m_dma_buffer);

						words -= (512/4);
					}
				}

				// clear DMA on the controller too
				m_wd33c93->clear_dma();
#if 0
				uint32_t dptr, tmpword;
				uint32_t bc = space.read_dword(m_hpc3.m_scsi0_desc + 4);
				uint32_t rptr = space.read_dword(m_hpc3.m_scsi0_desc);
				int length = bc & 0x3fff;
				int xie = (bc & 0x20000000) ? 1 : 0;
				int eox = (bc & 0x80000000) ? 1 : 0;

				dump_chain(space, m_hpc3.m_scsi0_desc);

				printf("PC is %08x\n", machine.device("maincpu")->safe_pc());
				printf("DMA to device: length %x xie %d eox %d\n", length, xie, eox);

				if (length <= 0x4000)
				{
					dptr = 0;
					while (length > 0)
					{
						tmpword = space.read_dword(rptr);
						if (m_hpc3.m_scsi0_dma_ctrl & HPC3_DMACTRL_ENDIAN)
						{
							m_dma_buffer[dptr+3] = (tmpword>>24)&0xff;
							m_dma_buffer[dptr+2] = (tmpword>>16)&0xff;
							m_dma_buffer[dptr+1] = (tmpword>>8)&0xff;
							m_dma_buffer[dptr] = tmpword&0xff;
						}
						else
						{
							m_dma_buffer[dptr] = (tmpword>>24)&0xff;
							m_dma_buffer[dptr+1] = (tmpword>>16)&0xff;
							m_dma_buffer[dptr+2] = (tmpword>>8)&0xff;
							m_dma_buffer[dptr+3] = tmpword&0xff;
						}

						dptr += 4;
						rptr += 4;
						length -= 4;
					}

					length = space.read_dword(m_hpc3.m_scsi0_desc+4) & 0x3fff;
					m_wd33c93->write_data(length, m_dma_buffer);

					// clear DMA on the controller too
					m_wd33c93->clear_dma();
				}
				else
				{
					logerror("IP22: overly large host to device transfer, can't handle!\n");
				}
#endif
			}

			// HPC3 DMA: device to host
			if ((m_hpc3.m_scsi0_dma_ctrl & HPC3_DMACTRL_ENABLE) && !(m_hpc3.m_scsi0_dma_ctrl & HPC3_DMACTRL_DIR))
			{
				uint32_t wptr, tmpword;
				int words, sptr, twords;

				words = m_wd33c93->get_dma_count();
				words /= 4;

				wptr = space.read_dword(m_hpc3.m_scsi0_desc);
				sptr = 0;

//              osd_printf_info("DMA from device: %d words @ %x\n", words, wptr);

				dump_chain(space, m_hpc3.m_scsi0_desc);

				if (words <= (1024/4))
				{
					// one-shot
					m_wd33c93->dma_read_data(m_wd33c93->get_dma_count(), m_dma_buffer);

					while (words)
					{
						if (m_hpc3.m_scsi0_dma_ctrl & HPC3_DMACTRL_ENDIAN)
						{
							tmpword = m_dma_buffer[sptr+3]<<24 | m_dma_buffer[sptr+2]<<16 | m_dma_buffer[sptr+1]<<8 | m_dma_buffer[sptr];
						}
						else
						{
							tmpword = m_dma_buffer[sptr]<<24 | m_dma_buffer[sptr+1]<<16 | m_dma_buffer[sptr+2]<<8 | m_dma_buffer[sptr+3];
						}

						space.write_dword(wptr, tmpword);
						wptr += 4;
						sptr += 4;
						words--;
					}
				}
				else
				{
					while (words)
					{
						m_wd33c93->dma_read_data(512, m_dma_buffer);
						twords = 512/4;
						sptr = 0;

						while (twords)
						{
							if (m_hpc3.m_scsi0_dma_ctrl & HPC3_DMACTRL_ENDIAN)
							{
								tmpword = m_dma_buffer[sptr+3]<<24 | m_dma_buffer[sptr+2]<<16 | m_dma_buffer[sptr+1]<<8 | m_dma_buffer[sptr];
							}
							else
							{
								tmpword = m_dma_buffer[sptr]<<24 | m_dma_buffer[sptr+1]<<16 | m_dma_buffer[sptr+2]<<8 | m_dma_buffer[sptr+3];
							}
							space.write_dword(wptr, tmpword);

							wptr += 4;
							sptr += 4;
							twords--;
						}

						words -= (512/4);
					}
				}

				// clear DMA on the controller too
				m_wd33c93->clear_dma();
			}
		}

		// clear HPC3 DMA active flag
		m_hpc3.m_scsi0_dma_ctrl &= ~HPC3_DMACTRL_ENABLE;

		// set the interrupt
		m_ioc2->raise_local0_irq(INT3_LOCAL0_SCSI0);
	}
	else
	{
		m_ioc2->lower_local0_irq(INT3_LOCAL0_SCSI0);
	}
}

void ip22_state::machine_start()
{
}

DRIVER_INIT_MEMBER(ip22_state, ip225015)
{
	// IP22 uses 2 pieces of PC-compatible hardware: the 8042 PS/2 keyboard/mouse
	// interface and the 8254 PIT.  Both are licensed cores embedded in the IOC custom chip.
}

static INPUT_PORTS_START( ip225015 )
	PORT_START("IN0")   // unused IN0
	PORT_START("DSW0")  // unused IN1
	PORT_START("DSW1")  // unused IN2
	PORT_START("DSW2")  // unused IN3
	PORT_INCLUDE( at_keyboard )     /* IN4 - IN11 */
INPUT_PORTS_END

void ip22_state::cdrom_config(device_t *device)
{
	device = device->subdevice("cdda");
	MCFG_SOUND_ROUTE(0, "^^^^lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "^^^^rspeaker", 1.0)
}

MACHINE_CONFIG_START(ip22_state::ip225015)
	MCFG_CPU_ADD( "maincpu", R5000BE, 50000000*3 )
	//MCFG_MIPS3_ICACHE_SIZE(32768)
	//MCFG_MIPS3_DCACHE_SIZE(32768)
	MCFG_CPU_PROGRAM_MAP( ip225015_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( 60 )
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(1280+64, 1024+64)
	MCFG_SCREEN_VISIBLE_AREA(0, 1279, 0, 1023)
	MCFG_SCREEN_UPDATE_DEVICE("newport", newport_video_device, screen_update)

	MCFG_PALETTE_ADD("palette", 65536)

	MCFG_NEWPORT_ADD("newport")

	MCFG_DEVICE_ADD("sgi_mc", SGI_MC, 0)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("dac", DAC_16BIT_R2R_TWOS_COMPLEMENT, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.25) // unknown DAC
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac", -1.0, DAC_VREF_NEG_INPUT)

	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_1)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE2, "cdrom", SCSICD, SCSI_ID_4)
	MCFG_SLOT_OPTION_MACHINE_CONFIG("cdrom", cdrom_config)

	MCFG_DEVICE_ADD("wd33c93", WD33C93, 0)
	MCFG_LEGACY_SCSI_PORT("scsi")
	MCFG_WD33C93_IRQ_CB(WRITELINE(ip22_state,scsi_irq))

	MCFG_IOC2_GUINNESS_ADD(IOC2_TAG)

	MCFG_DS1386_8K_ADD(RTC_TAG, 32768)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(ip22_state::ip224613)
	ip225015(config);
	MCFG_CPU_REPLACE( "maincpu", R4600BE, 133333333 )
	//MCFG_MIPS3_ICACHE_SIZE(32768)
	//MCFG_MIPS3_DCACHE_SIZE(32768)
	MCFG_CPU_PROGRAM_MAP( ip225015_map)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(ip22_state::ip244415)
	ip225015(config);
	MCFG_CPU_REPLACE( "maincpu", R4600BE, 150000000 )
	//MCFG_MIPS3_ICACHE_SIZE(32768)
	//MCFG_MIPS3_DCACHE_SIZE(32768)
	MCFG_CPU_PROGRAM_MAP( ip225015_map)
MACHINE_CONFIG_END

/* SCC init ip225015
 * Channel A
 * 09 <- c0 Master Interrup Control: Force HW reset + enable SWI INTACK
 * 04 <- 44 Clocks: x16 mode, 1 stop bits, no parity
 * 03 <- c0 Receiver: 8 bit data, auto enables, Rx disabled
 * 05 <- e2 Transmitter: DTR set, 8 bit data, RTS set, Tx disabled
 * 0b <- 50 Clock Mode: TRxC: XTAL output, TRxC: Output, TxC from BRG, RxC from BRG
 * 0c <- 0a Low const BRG  3.6864Mhz CLK => 9600 baud
 * 0d <- 00 High Const BRG = (CLK / (2 x Desired Rate x BR Clock period)) - 2
 * 0e <- 01 Mics: BRG enable
 * 03 <- c1 Receiver: as above + Receiver enable
 * 05 <- ea Transmitter: as above + Transmitter enable
 *
 * Channel A and B init - only BRG low const differs
 * 09 <- 80 channel A reset
 * 04 <- 44 Clocks: x16 mode, 1 stop bits, no parity
 * 0f <- 81 External/Status Control: Break/Abort enabled, WR7 prime enabled
 * 07p<- 40 External read enable (RR9=WR3, RR4=WR4, RR5=WR5, RR14=WR7 and RR11=WR10)
 * 03 <- c0 Receiver: 8 bit data, auto enables, Rx disabled
 * 05 <- e2 Transmitter: DTR set, 8 bit data, RTS set, Tx disabled
 * 0b <- 50 Clock Mode: TRxC: XTAL output, TRxC: Output, TxC from BRG, RxC from BRG
 * 0e <- 00 Mics: BRG disable
 * 0c <- 0a/04 Low const BRG, 3.6864Mhz CLK => Chan A:9600 Chan B:38400
 * 0d <- 00 High Const BRG = (CLK / (2 x Desired Rate x BR Clock period)) - 2
 * 0e <- 01 Mics: BRG enable
 * 03 <- c1 Receiver: as above + Receiver enable
 * 05 <- ea Transmitter: as above + Transmitetr enable
 * 00 <- 10 Reset External/status IE
*/
ROM_START( ip225015 )
	ROM_REGION( 0x80000, "user1", 0 )
	ROM_LOAD( "ip225015.bin", 0x000000, 0x080000, CRC(aee5502e) SHA1(9243fef0a3508790651e0d6d2705c887629b1280) )
ROM_END

ROM_START( ip224613 )
	ROM_REGION( 0x80000, "user1", 0 )
	ROM_LOAD( "ip224613.bin", 0x000000, 0x080000, CRC(f1868b5b) SHA1(0dcbbd776e671785b9b65f3c6dbd609794a40157) )
ROM_END

ROM_START( ip244415 )
	ROM_REGION( 0x80000, "user1", 0 )
	ROM_LOAD( "ip244415.bin", 0x000000, 0x080000, CRC(2f37825a) SHA1(0d48c573b53a307478820b85aacb57b868297ca3) )
ROM_END

//    YEAR  NAME      PARENT    COMPAT    MACHINE   INPUT     STATE       INIT      COMPANY                 FULLNAME                   FLAGS
COMP( 1993, ip225015, 0,        0,        ip225015, ip225015, ip22_state, ip225015, "Silicon Graphics Inc", "Indy (R5000, 150MHz)",    MACHINE_NOT_WORKING )
COMP( 1993, ip224613, 0,        0,        ip224613, ip225015, ip22_state, ip225015, "Silicon Graphics Inc", "Indy (R4600, 133MHz)",    MACHINE_NOT_WORKING )
COMP( 1994, ip244415, 0,        0,        ip244415, ip225015, ip22_state, ip225015, "Silicon Graphics Inc", "Indigo2 (R4400, 150MHz)", MACHINE_NOT_WORKING )
