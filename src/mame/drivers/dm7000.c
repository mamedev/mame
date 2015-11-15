// license:BSD-3-Clause
// copyright-holders:Lukasz Markowski
/***************************************************************************

        Dream Multimedia Dreambox 7000/5620/500

        20/03/2010 Skeleton driver.


    DM7000 -    CPU STB04500 at 252 MHz
                RAM 64MB
                Flash 8MB
                1 x DVB-S
                1 x IDE interface
                1 x Common Interface (CI)
                1 x Compact flash
                2 x Smart card reader
                1 x USB
                1 x RS232
                1 x LAN 100Mbit/s
                1 x LCD display

    DM56x0 -    CPU STB04500 at 252 MHz
                RAM 64MB
                Flash 8MB
                1 x DVB-S
                2 x Common Interface (CI)
                1 x Smart card reader
                1 x LAN 100Mbit/s (just on 5620)
                1 x LCD display

    DM500 -     CPU STBx25xx at 252 MHz
                RAM 96MB
                Flash 32MB
                1 x DVB-S
                1 x Smart card reader
                1 x LAN 100Mbit/s

****************************************************************************/

#include "emu.h"
#include "includes/dm7000.h"

#define VERBOSE_LEVEL ( 9 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine &machine, int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt);
		vsprintf( buf, s_fmt, v);
		va_end( v);
		logerror( "%s: %s", machine.describe_context( ), buf);
	}
}

READ8_MEMBER( dm7000_state::dm7000_iic0_r )
{
	UINT8 data = 0; // dummy
	verboselog( machine(), 9, "(IIC0) %08X -> %08X\n", 0x40030000 + offset, data);
	return data;
}

WRITE8_MEMBER( dm7000_state::dm7000_iic0_w )
{
	verboselog( machine(), 9, "(IIC0) %08X <- %08X\n", 0x40030000 + offset, data);
}

READ8_MEMBER( dm7000_state::dm7000_iic1_r )
{
	UINT8 data = 0; // dummy
	verboselog( machine(), 9, "(IIC1) %08X -> %08X\n", 0x400b0000 + offset, data);
	return data;
}

WRITE8_MEMBER( dm7000_state::dm7000_iic1_w )
{
	verboselog( machine(), 9, "(IIC1) %08X <- %08X\n", 0x400b0000 + offset, data);
}

READ8_MEMBER( dm7000_state::dm7000_scc0_r )
{
	UINT8 data = 0;
	switch(offset) {
		case UART_THR:
			data = m_term_data;
			if(m_term_data == 0xd) {
				m_term_data = 0xa;
			} else {
				m_term_data = 0;
				m_scc0_lsr = 0;
			}
			break;
		case UART_LSR:
			data = UART_LSR_THRE | UART_LSR_TEMT | m_scc0_lsr;
			break;
	}
	verboselog( machine(), 9, "(SCC0) %08X -> %08X\n", 0x40040000 + offset, data);
	return data;
}

WRITE8_MEMBER( dm7000_state::dm7000_scc0_w )
{
	switch(offset) {
		case UART_THR:
			if(!(m_scc0_lcr & UART_LCR_DLAB)) {
				m_terminal->write(space, 0, data);
				m_scc0_lsr = 1;
			}
			break;
		case UART_LCR:
			m_scc0_lcr = data;
			break;
	}
	verboselog( machine(), 9, "(SCC0) %08X <- %08X\n", 0x40040000 + offset, data);
}

READ8_MEMBER( dm7000_state::dm7000_gpio0_r )
{
	UINT8 data = 0; // dummy
	verboselog( machine(), 9, "(GPIO0) %08X -> %08X\n", 0x40060000 + offset, data);
	return data;
}

WRITE8_MEMBER( dm7000_state::dm7000_gpio0_w )
{
	verboselog( machine(), 9, "(GPIO0) %08X <- %08X\n", 0x40060000 + offset, data);
}

READ8_MEMBER( dm7000_state::dm7000_scp0_r )
{
	UINT8 data = 0; // dummy
	switch(offset) {
		case SCP_STATUS:
			data = SCP_STATUS_RXRDY;
			break;
	}
	verboselog( machine(), 9, "(SCP0) %08X -> %08X\n", 0x400c0000 + offset, data);
	return data;
}

WRITE8_MEMBER( dm7000_state::dm7000_scp0_w )
{
	verboselog( machine(), 9, "(SCP0) %08X <- %08X\n", 0x400c0000 + offset, data);
	switch(offset) {
		case SCP_TXDATA:
			//printf("%02X ", data);
			break;
	}
}

READ16_MEMBER( dm7000_state::dm7000_enet_r )
{
	UINT16 data;
	switch (offset) {
		case 0x01:
			data = 0x1801;
			break;
		case 0x05:
			data = 0x3330;
			break;
		case 0x07:
			data = 0x3300;
			break;
		default:
			data = m_enet_regs[offset];
			break;
	}
	verboselog( machine(), 9, "(ENET) %08X -> %08X\n", 0x72000600 + (offset), data);
	return data;
}

WRITE16_MEMBER( dm7000_state::dm7000_enet_w )
{
	verboselog( machine(), 9, "(ENET) %08X <- %08X\n", 0x72000600 + (offset), data);
	COMBINE_DATA(&m_enet_regs[offset]);
}

/*
 Memory map for the IBM "Redwood-4" STB03xxx evaluation board.

 The  STB03xxx internal i/o addresses don't work for us 1:1,
 so we need to map them at a well know virtual address.

 4000 000x   uart1
 4001 00xx   ppu
 4002 00xx   smart card
 4003 000x   iic
 4004 000x   uart0
 4005 0xxx   timer
 4006 00xx   gpio
 4007 00xx   smart card
 400b 000x   iic
 400c 000x   scp
 400d 000x   modem

 STBx25xx

 4000 000x   Serial1 Controller
 4001 000x   Serial2 Controller
 4002 00xx   Smart Card Interface 0
 4003 000x   IIC Interface 0
 4004 000x   Serial0 Controller
 4005 0xxx   General Purpose Timers
 4006 00xx   General Purpose Input / Output
 4007 00xx   Smart Card Interface 1
 400c 000x   Serial Controller Port
 400d 00xx   Synchronous Serial Port

 STB04xxx

 4000 00xx   Serial1/Infrared Controller
 4001 00xx   Universal Serial Bus
 4002 00xx   Smart Card Interface 0
 4003 000x   IIC Interface 0
 4004 000x   Serial0/Uart750 Controller
 4005 0xxx   General Purpose Timers
 4006 00xx   General Purpose Input / Output
 4007 00xx   Smart Card Interface 1
 400b 000x   IIC Interface 1
 400c 000x   Serial Controller Port
 400d 00xx   Synchronous Serial Port
 400e 000x   Serial2/UART750 Controller
 400f 0xxx   IDE Controller

*/
static ADDRESS_MAP_START( dm7000_mem, AS_PROGRAM, 32, dm7000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x01ffffff) AM_RAM                                     // RAM page 0 - 32MB
	AM_RANGE(0x20000000, 0x21ffffff) AM_RAM                                     // RAM page 1 - 32MB

	AM_RANGE(0x40030000, 0x4003000f) AM_READWRITE8(dm7000_iic0_r, dm7000_iic0_w, 0xffffffff)
	AM_RANGE(0x40040000, 0x40040007) AM_READWRITE8(dm7000_scc0_r, dm7000_scc0_w, 0xffffffff)
	AM_RANGE(0x40060000, 0x40060047) AM_READWRITE8(dm7000_gpio0_r, dm7000_gpio0_w, 0xffffffff)
	AM_RANGE(0x400b0000, 0x400b000f) AM_READWRITE8(dm7000_iic1_r, dm7000_iic1_w, 0xffffffff)
	AM_RANGE(0x400c0000, 0x400c0007) AM_READWRITE8(dm7000_scp0_r, dm7000_scp0_w, 0xffffffff)

	/* ENET - ASIX AX88796 */
	AM_RANGE(0x72000300, 0x720003ff) AM_READWRITE16(dm7000_enet_r, dm7000_enet_w, 0xffffffff)

	AM_RANGE(0x7f800000, 0x7ffdffff) AM_ROM AM_REGION("user2",0)
	AM_RANGE(0x7ffe0000, 0x7fffffff) AM_ROM AM_REGION("user1",0)
	//AM_RANGE(0xfffe0000, 0xffffffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( dm7000 )
INPUT_PORTS_END


void dm7000_state::machine_reset()
{
	dcr[DCRSTB045_CICVCR] = 0x00000001;
	dcr[DCRSTB045_SCCR] = 0x00420080 /* default for serial divs */ | 0x3f /* undocumented?? used to print clocks */;
	dcr[DCRSTB045_VIDEO_CNTL] = 0x00009000;
	dcr[DCRSTB045_DISP_MODE] = 0x00880000;
	dcr[DCRSTB045_FRAME_BUFR_BASE] = 0x0f000000;
	m_scc0_lsr = UART_LSR_THRE | UART_LSR_TEMT;

	m_maincpu->ppc4xx_set_dcr_read_handler(read32_delegate(FUNC(dm7000_state::dcr_r),this));
	m_maincpu->ppc4xx_set_dcr_write_handler(write32_delegate(FUNC(dm7000_state::dcr_w),this));
}

void dm7000_state::video_start()
{
}

UINT32 dm7000_state::screen_update_dm7000(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

READ32_MEMBER( dm7000_state::dcr_r )
{
	osd_printf_debug("DCR %03X read\n", offset);
	if(offset>=1024) {printf("get %04X\n", offset); return 0;} else
	switch(offset) {
		case DCRSTB045_CMD_STAT:
			return 0; // assume that video dev is always ready
		default:
			return dcr[offset];
	}

}

WRITE32_MEMBER( dm7000_state::dcr_w )
{
	osd_printf_debug("DCR %03X write = %08X\n", offset, data);
	if(offset>=1024) {printf("get %04X\n", offset); } else
	dcr[offset] = data;
}

WRITE8_MEMBER( dm7000_state::kbd_put )
{
	//printf("%02X\n", data);
	m_term_data = data;
	m_scc0_lsr = 1;
}

static MACHINE_CONFIG_START( dm7000, dm7000_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",PPC405GP, 252000000 / 10) // Should be PPC405D4?
	// Slowed down 10 times in order to get normal response for now
	MCFG_PPC_BUS_FREQUENCY(252000000)
	MCFG_CPU_PROGRAM_MAP(dm7000_mem)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(dm7000_state, screen_update_dm7000)

	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(dm7000_state, kbd_put))

MACHINE_CONFIG_END

/* ROM definition */
ROM_START( dm7000 )
	ROM_REGION( 0x20000, "user1", ROMREGION_32BIT | ROMREGION_BE  )
	ROMX_LOAD( "dm7000.bin", 0x0000, 0x20000, CRC(8a410f67) SHA1(9d6c9e4f5b05b28453d3558e69a207f05c766f54), ROM_GROUPWORD )
	ROM_REGION( 0x800000, "user2", ROMREGION_32BIT | ROMREGION_BE | ROMREGION_ERASEFF  )
	ROM_LOAD( "rel108_dm7000.img", 0x0000, 0x5e0000, CRC(e78b6407) SHA1(aaa786d341c629eec92fcf04bfafc1de43f6dabf))
ROM_END

ROM_START( dm5620 )
	ROM_REGION( 0x20000, "user1", ROMREGION_32BIT | ROMREGION_BE  )
	ROMX_LOAD( "dm5620.bin", 0x0000, 0x20000, CRC(ccddb822) SHA1(3ecf553ced0671599438368f59d8d30df4d13ade), ROM_GROUPWORD )
	ROM_REGION( 0x800000, "user2", ROMREGION_32BIT | ROMREGION_BE | ROMREGION_ERASEFF  )
	ROM_LOAD( "rel106_dm5620.img", 0x0000, 0x57b000, CRC(2313d71d) SHA1(0d3d99ab3b3266624f237b7b67e045d7910c44a5))
ROM_END

ROM_START( dm500 )
	ROM_REGION( 0x20000, "user1", ROMREGION_32BIT | ROMREGION_BE )
	ROM_SYSTEM_BIOS( 0, "alps", "Alps" )
	ROMX_LOAD( "dm500-alps-boot.bin",   0x0000, 0x20000, CRC(daf2da34) SHA1(68f3734b4589fcb3e73372e258040bc8b83fd739), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "phil", "Philips" )
	ROMX_LOAD( "dm500-philps-boot.bin", 0x0000, 0x20000, CRC(af3477c7) SHA1(9ac918f6984e6927f55bea68d6daaf008787136e), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(2))
	ROM_REGION( 0x800000, "user2", ROMREGION_32BIT | ROMREGION_BE | ROMREGION_ERASEFF  )
	ROM_LOAD( "rel108_dm500.img", 0x0000, 0x5aa000, CRC(44be2376) SHA1(1f360572998b1bc4dc10c5210a2aed573a75e2fa))
ROM_END
/* Driver */

/*    YEAR  NAME     PARENT   COMPAT   MACHINE    INPUT    INIT     COMPANY                FULLNAME       FLAGS */
SYST( 2003, dm7000,  0,       0,       dm7000,    dm7000, driver_device,  0,   "Dream Multimedia",   "Dreambox 7000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
SYST( 2004, dm5620,  dm7000,  0,       dm7000,    dm7000, driver_device,  0,   "Dream Multimedia",   "Dreambox 5620", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
SYST( 2006, dm500,   dm7000,  0,       dm7000,    dm7000, driver_device,  0,   "Dream Multimedia",   "Dreambox 500", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
