// license:BSD-3-Clause
// copyright-holders:Lukasz Markowski
#ifndef DM7000_H_
#define DM7000_H_

#include "emu.h"
#include "cpu/powerpc/ppc.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class dm7000_state : public driver_device
{
public:
	dm7000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	required_device<ppc4xx_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;

	DECLARE_WRITE8_MEMBER ( dm7000_iic0_w );
	DECLARE_READ8_MEMBER ( dm7000_iic0_r );
	DECLARE_WRITE8_MEMBER ( dm7000_iic1_w );
	DECLARE_READ8_MEMBER ( dm7000_iic1_r );

	DECLARE_WRITE8_MEMBER ( dm7000_scc0_w );
	DECLARE_READ8_MEMBER ( dm7000_scc0_r );
	DECLARE_WRITE8_MEMBER(kbd_put);
	UINT8 m_scc0_lcr;
	UINT8 m_scc0_lsr;
	UINT8 m_term_data;


	DECLARE_WRITE8_MEMBER ( dm7000_gpio0_w );
	DECLARE_READ8_MEMBER ( dm7000_gpio0_r );

	DECLARE_WRITE8_MEMBER ( dm7000_scp0_w );
	DECLARE_READ8_MEMBER ( dm7000_scp0_r );

	DECLARE_WRITE16_MEMBER ( dm7000_enet_w );
	DECLARE_READ16_MEMBER ( dm7000_enet_r );

	DECLARE_READ32_MEMBER( dcr_r );
	DECLARE_WRITE32_MEMBER( dcr_w );


	UINT16          m_enet_regs[32];

	UINT32          dcr[1024];
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_dm7000(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

/* */
#define UART_DLL    0
#define UART_RBR    0
#define UART_THR    0
#define UART_DLH    1
#define UART_IER    1
#define UART_IIR    2
#define UART_FCR    2
#define UART_LCR    3
#define     UART_LCR_DLAB   0x80
#define UART_MCR    4
#define UART_LSR    5
#define     UART_LSR_TEMT   0x20
#define     UART_LSR_THRE   0x40
#define UART_MSR    6
#define UART_SCR    7

/* */
#define SCP_SPMODE 0
#define SCP_RXDATA 1
#define SCP_TXDATA 2
#define SCP_SPCOM 3
#define SCP_STATUS 4
#define     SCP_STATUS_RXRDY 1
#define SCP_CDM 6

/* STB045xxx DCRs */

#define DCRSTB045_CICVCR            0x033       /* CIC Video Control Register */
#define DCRSTB045_SCCR              0x120       /* Serial Clock Control Register */
#define DCRSTB045_VIDEO_CNTL        0x140       /* Video Control Register */
#define DCRSTB045_CMD_STAT          0x14a       /* Command status */
#define DCRSTB045_DISP_MODE         0x154       /* Display Mode Register */
#define DCRSTB045_FRAME_BUFR_BASE   0x179       /* Frame Buffers Base Address Register */

#endif /* DM7000_H_ */
