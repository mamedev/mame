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

#include "cpu/powerpc/ppc.h"
#include "machine/terminal.h"
#include "screen.h"

#define VERBOSE ( 1 )
#include "logmacro.h"

namespace {

class dm7000_state : public driver_device
{
public:
	dm7000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
	{
	}

	void dm7000(machine_config &config);

private:
	required_device<ppc4xx_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;

	void dm7000_iic0_w(offs_t offset, uint8_t data);
	uint8_t dm7000_iic0_r(offs_t offset);
	void dm7000_iic1_w(offs_t offset, uint8_t data);
	uint8_t dm7000_iic1_r(offs_t offset);

	void dm7000_scc0_w(offs_t offset, uint8_t data);
	uint8_t dm7000_scc0_r(offs_t offset);
	void kbd_put(u8 data);
	uint8_t m_scc0_lcr = 0U;
	uint8_t m_scc0_lsr = 0U;
	uint8_t m_term_data = 0U;


	void dm7000_gpio0_w(offs_t offset, uint8_t data);
	uint8_t dm7000_gpio0_r(offs_t offset);

	void dm7000_scp0_w(offs_t offset, uint8_t data);
	uint8_t dm7000_scp0_r(offs_t offset);

	void dm7000_enet_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dm7000_enet_r(offs_t offset);

	uint32_t dcr_r(offs_t offset);
	void dcr_w(offs_t offset, uint32_t data);


	uint16_t          m_enet_regs[32]{};

	uint32_t          dcr[1024]{};
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_dm7000(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void dm7000_mem(address_map &map) ATTR_COLD;
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

uint8_t dm7000_state::dm7000_iic0_r(offs_t offset)
{
	uint8_t data = 0; // dummy
	LOG("%s: (IIC0) %08X -> %08X\n", machine().describe_context(), 0x40030000 + offset, data);
	return data;
}

void dm7000_state::dm7000_iic0_w(offs_t offset, uint8_t data)
{
	LOG("%s: (IIC0) %08X <- %08X\n", machine().describe_context(), 0x40030000 + offset, data);
}

uint8_t dm7000_state::dm7000_iic1_r(offs_t offset)
{
	uint8_t data = 0; // dummy
	LOG("%s: (IIC1) %08X -> %08X\n", machine().describe_context(), 0x400b0000 + offset, data);
	return data;
}

void dm7000_state::dm7000_iic1_w(offs_t offset, uint8_t data)
{
	LOG("%s: (IIC1) %08X <- %08X\n", machine().describe_context(), 0x400b0000 + offset, data);
}

uint8_t dm7000_state::dm7000_scc0_r(offs_t offset)
{
	uint8_t data = 0;
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
	LOG("%s: (SCC0) %08X -> %08X\n", machine().describe_context(), 0x40040000 + offset, data);
	return data;
}

void dm7000_state::dm7000_scc0_w(offs_t offset, uint8_t data)
{
	switch(offset) {
		case UART_THR:
			if(!(m_scc0_lcr & UART_LCR_DLAB)) {
				m_terminal->write(data);
				m_scc0_lsr = 1;
			}
			break;
		case UART_LCR:
			m_scc0_lcr = data;
			break;
	}
	LOG("%s: (SCC0) %08X <- %08X\n", machine().describe_context(), 0x40040000 + offset, data);
}

uint8_t dm7000_state::dm7000_gpio0_r(offs_t offset)
{
	uint8_t data = 0; // dummy
	LOG("%s: (GPIO0) %08X -> %08X\n", machine().describe_context(), 0x40060000 + offset, data);
	return data;
}

void dm7000_state::dm7000_gpio0_w(offs_t offset, uint8_t data)
{
	LOG("%s: (GPIO0) %08X <- %08X\n", machine().describe_context(), 0x40060000 + offset, data);
}

uint8_t dm7000_state::dm7000_scp0_r(offs_t offset)
{
	uint8_t data = 0; // dummy
	switch(offset) {
		case SCP_STATUS:
			data = SCP_STATUS_RXRDY;
			break;
	}
	LOG("%s: (SCP0) %08X -> %08X\n", machine().describe_context(), 0x400c0000 + offset, data);
	return data;
}

void dm7000_state::dm7000_scp0_w(offs_t offset, uint8_t data)
{
	LOG("%s: (SCP0) %08X <- %08X\n", machine().describe_context(), 0x400c0000 + offset, data);
	switch(offset) {
		case SCP_TXDATA:
			//printf("%02X ", data);
			break;
	}
}

uint16_t dm7000_state::dm7000_enet_r(offs_t offset)
{
	uint16_t data;
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
	LOG("%s: (ENET) %08X -> %08X\n", machine().describe_context(), 0x72000600 + (offset), data);
	return data;
}

void dm7000_state::dm7000_enet_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOG("%s: (ENET) %08X <- %08X\n", machine().describe_context(), 0x72000600 + (offset), data);
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
void dm7000_state::dm7000_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x01ffffff).ram();                                     // RAM page 0 - 32MB
	map(0x20000000, 0x21ffffff).ram();                                     // RAM page 1 - 32MB

	map(0x40030000, 0x4003000f).rw(FUNC(dm7000_state::dm7000_iic0_r), FUNC(dm7000_state::dm7000_iic0_w));
	map(0x40040000, 0x40040007).rw(FUNC(dm7000_state::dm7000_scc0_r), FUNC(dm7000_state::dm7000_scc0_w));
	map(0x40060000, 0x40060047).rw(FUNC(dm7000_state::dm7000_gpio0_r), FUNC(dm7000_state::dm7000_gpio0_w));
	map(0x400b0000, 0x400b000f).rw(FUNC(dm7000_state::dm7000_iic1_r), FUNC(dm7000_state::dm7000_iic1_w));
	map(0x400c0000, 0x400c0007).rw(FUNC(dm7000_state::dm7000_scp0_r), FUNC(dm7000_state::dm7000_scp0_w));

	/* ENET - ASIX AX88796 */
	map(0x72000300, 0x720003ff).rw(FUNC(dm7000_state::dm7000_enet_r), FUNC(dm7000_state::dm7000_enet_w));

	map(0x7f800000, 0x7ffdffff).rom().region("user2", 0);
	map(0x7ffe0000, 0x7fffffff).rom().region("user1", 0);
	//map(0xfffe0000, 0xffffffff).rom().region("user1",0);
}

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

	m_maincpu->ppc4xx_set_dcr_read_handler(read32sm_delegate(*this, FUNC(dm7000_state::dcr_r)));
	m_maincpu->ppc4xx_set_dcr_write_handler(write32sm_delegate(*this, FUNC(dm7000_state::dcr_w)));
}

void dm7000_state::video_start()
{
}

uint32_t dm7000_state::screen_update_dm7000(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

uint32_t dm7000_state::dcr_r(offs_t offset)
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

void dm7000_state::dcr_w(offs_t offset, uint32_t data)
{
	osd_printf_debug("DCR %03X write = %08X\n", offset, data);
	if(offset>=1024) {printf("get %04X\n", offset); } else
	dcr[offset] = data;
}

void dm7000_state::kbd_put(u8 data)
{
	//printf("%02X\n", data);
	m_term_data = data;
	m_scc0_lsr = 1;
}

void dm7000_state::dm7000(machine_config &config)
{
	/* basic machine hardware */
	PPC405GP(config, m_maincpu, 252000000 / 10); // Should be PPC405D4?
	// Slowed down 10 times in order to get normal response for now
	m_maincpu->set_bus_frequency(252000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &dm7000_state::dm7000_mem);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(dm7000_state::screen_update_dm7000));

	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(dm7000_state::kbd_put));
}


/* ROM definition */
ROM_START( dm7000 )
	ROM_REGION( 0x20000, "user1", ROMREGION_32BIT | ROMREGION_BE  )
	ROMX_LOAD("dm7000.bin", 0x0000, 0x20000, CRC(8a410f67) SHA1(9d6c9e4f5b05b28453d3558e69a207f05c766f54), ROM_GROUPWORD)
	ROM_REGION( 0x800000, "user2", ROMREGION_32BIT | ROMREGION_BE | ROMREGION_ERASEFF  )
	ROM_LOAD("rel108_dm7000.img", 0x0000, 0x5e0000, CRC(e78b6407) SHA1(aaa786d341c629eec92fcf04bfafc1de43f6dabf))
ROM_END

ROM_START( dm5620 )
	ROM_REGION( 0x20000, "user1", ROMREGION_32BIT | ROMREGION_BE  )
	ROMX_LOAD("dm5620.bin", 0x0000, 0x20000, CRC(ccddb822) SHA1(3ecf553ced0671599438368f59d8d30df4d13ade), ROM_GROUPWORD)
	ROM_REGION( 0x800000, "user2", ROMREGION_32BIT | ROMREGION_BE | ROMREGION_ERASEFF  )
	ROM_LOAD("rel106_dm5620.img", 0x0000, 0x57b000, CRC(2313d71d) SHA1(0d3d99ab3b3266624f237b7b67e045d7910c44a5))
ROM_END

ROM_START( dm500 )
	ROM_REGION( 0x20000, "user1", ROMREGION_32BIT | ROMREGION_BE )
	ROM_SYSTEM_BIOS( 0, "alps", "Alps" )
	ROMX_LOAD("dm500-alps-boot.bin",   0x0000, 0x20000, CRC(daf2da34) SHA1(68f3734b4589fcb3e73372e258040bc8b83fd739), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "phil", "Philips" )
	ROMX_LOAD("dm500-philps-boot.bin", 0x0000, 0x20000, CRC(af3477c7) SHA1(9ac918f6984e6927f55bea68d6daaf008787136e), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(1))
	ROM_REGION( 0x800000, "user2", ROMREGION_32BIT | ROMREGION_BE | ROMREGION_ERASEFF  )
	ROM_LOAD("rel108_dm500.img", 0x0000, 0x5aa000, CRC(44be2376) SHA1(1f360572998b1bc4dc10c5210a2aed573a75e2fa))
ROM_END

} // anonymous namespace

/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY             FULLNAME         FLAGS
SYST( 2003, dm7000, 0,      0,      dm7000,  dm7000, dm7000_state, empty_init, "Dream Multimedia", "Dreambox 7000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
SYST( 2004, dm5620, dm7000, 0,      dm7000,  dm7000, dm7000_state, empty_init, "Dream Multimedia", "Dreambox 5620", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
SYST( 2006, dm500,  dm7000, 0,      dm7000,  dm7000, dm7000_state, empty_init, "Dream Multimedia", "Dreambox 500",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
