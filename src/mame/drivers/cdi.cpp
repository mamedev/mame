// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    Philips CD-I-based games
    ------------------------

    Preliminary MAME driver by Ryan Holtz
    Help provided by CD-i Fan


*******************************************************************************

STATUS:

Quizard does not work for unknown reasons.

TODO:

- Proper abstraction of the 68070's internal devices (UART,DMA,Timers etc.)
- Mono-I: Full emulation of the CDIC, SLAVE and/or MCD212 customs
- Mono-II: SERVO and SLAVE I/O device hookup
- Mono-II: DSP56k hookup
- Mono-II: Move 68HC05 I/O device hookup into CPU core

*******************************************************************************/

#define CLOCK_A XTAL_30MHz
#define CLOCK_B XTAL_19_6608MHz

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6805/m6805.h"
#include "includes/cdi.h"
#include "machine/timekpr.h"
#include "sound/cdda.h"
#include "cdrom.h"
#include "cdi.lh"
#include "imagedev/chd_cd.h"
#include "softlist.h"

#if ENABLE_VERBOSE_LOG
static inline void ATTR_PRINTF(3,4) verboselog(device_t& device, int n_level, const char *s_fmt, ...)
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		device.logerror("%s: %s", device.machine().describe_context(), buf );
	}
}
#else
#define verboselog(x,y,z, ...)
#endif

/*************************
*      Memory maps       *
*************************/

static ADDRESS_MAP_START( cdimono1_mem, AS_PROGRAM, 16, cdi_state )
	AM_RANGE(0x00000000, 0x0007ffff) AM_RAM AM_SHARE("planea")
	AM_RANGE(0x00200000, 0x0027ffff) AM_RAM AM_SHARE("planeb")
#if ENABLE_UART_PRINTING
	AM_RANGE(0x00301400, 0x00301403) AM_DEVREAD("scc68070", cdi68070_device, uart_loopback_enable)
#endif
	AM_RANGE(0x00300000, 0x00303bff) AM_DEVREADWRITE("cdic", cdicdic_device, ram_r, ram_w)
	AM_RANGE(0x00303c00, 0x00303fff) AM_DEVREADWRITE("cdic", cdicdic_device, regs_r, regs_w)
	AM_RANGE(0x00310000, 0x00317fff) AM_DEVREADWRITE("slave_hle", cdislave_device, slave_r, slave_w)
	AM_RANGE(0x00318000, 0x0031ffff) AM_NOP
	AM_RANGE(0x00320000, 0x00323fff) AM_DEVREADWRITE8("mk48t08", timekeeper_device, read, write, 0xff00)    /* nvram (only low bytes used) */
	AM_RANGE(0x00400000, 0x0047ffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x004fffe0, 0x004fffff) AM_DEVREADWRITE("mcd212", mcd212_device, regs_r, regs_w)
	AM_RANGE(0x00500000, 0x0057ffff) AM_RAM
	AM_RANGE(0x00500000, 0x00ffffff) AM_NOP
	AM_RANGE(0x00e00000, 0x00efffff) AM_RAM // DVC
	AM_RANGE(0x80000000, 0x8000807f) AM_DEVREADWRITE("scc68070", cdi68070_device, periphs_r, periphs_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cdimono2_mem, AS_PROGRAM, 16, cdi_state )
	AM_RANGE(0x00000000, 0x0007ffff) AM_RAM AM_SHARE("planea")
	AM_RANGE(0x00200000, 0x0027ffff) AM_RAM AM_SHARE("planeb")
#if ENABLE_UART_PRINTING
	AM_RANGE(0x00301400, 0x00301403) AM_DEVREAD("scc68070", cdi68070_device, uart_loopback_enable)
#endif
	//AM_RANGE(0x00300000, 0x00303bff) AM_DEVREADWRITE("cdic", cdicdic_device, ram_r, ram_w)
	//AM_RANGE(0x00303c00, 0x00303fff) AM_DEVREADWRITE("cdic", cdicdic_device, regs_r, regs_w)
	//AM_RANGE(0x00310000, 0x00317fff) AM_DEVREADWRITE("slave", cdislave_device, slave_r, slave_w)
	//AM_RANGE(0x00318000, 0x0031ffff) AM_NOP
	AM_RANGE(0x00320000, 0x00323fff) AM_DEVREADWRITE8("mk48t08", timekeeper_device, read, write, 0xff00)    /* nvram (only low bytes used) */
	AM_RANGE(0x00400000, 0x0047ffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x004fffe0, 0x004fffff) AM_DEVREADWRITE("mcd212", mcd212_device, regs_r, regs_w)
	//AM_RANGE(0x00500000, 0x0057ffff) AM_RAM
	AM_RANGE(0x00500000, 0x00ffffff) AM_NOP
	//AM_RANGE(0x00e00000, 0x00efffff) AM_RAM // DVC
	AM_RANGE(0x80000000, 0x8000807f) AM_DEVREADWRITE("scc68070", cdi68070_device, periphs_r, periphs_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cdi910_mem, AS_PROGRAM, 16, cdi_state )
	AM_RANGE(0x00000000, 0x0007ffff) AM_RAM AM_SHARE("planea")
	AM_RANGE(0x00180000, 0x001fffff) AM_ROM AM_REGION("maincpu", 0) // boot vectors point here

	AM_RANGE(0x00200000, 0x0027ffff) AM_RAM AM_SHARE("planeb")
#if ENABLE_UART_PRINTING
	AM_RANGE(0x00301400, 0x00301403) AM_DEVREAD("scc68070", cdi68070_device, uart_loopback_enable)
#endif
//  AM_RANGE(0x00300000, 0x00303bff) AM_DEVREADWRITE("cdic", cdicdic_device, ram_r, ram_w)
//  AM_RANGE(0x00303c00, 0x00303fff) AM_DEVREADWRITE("cdic", cdicdic_device, regs_r, regs_w)
//  AM_RANGE(0x00310000, 0x00317fff) AM_DEVREADWRITE("slave_hle", cdislave_device, slave_r, slave_w)
//  AM_RANGE(0x00318000, 0x0031ffff) AM_NOP
	AM_RANGE(0x00320000, 0x00323fff) AM_DEVREADWRITE8("mk48t08", timekeeper_device, read, write, 0xff00)    /* nvram (only low bytes used) */
	AM_RANGE(0x004fffe0, 0x004fffff) AM_DEVREADWRITE("mcd212", mcd212_device, regs_r, regs_w)
//  AM_RANGE(0x00500000, 0x0057ffff) AM_RAM
	AM_RANGE(0x00500000, 0x00ffffff) AM_NOP
//  AM_RANGE(0x00e00000, 0x00efffff) AM_RAM // DVC
	AM_RANGE(0x80000000, 0x8000807f) AM_DEVREADWRITE("scc68070", cdi68070_device, periphs_r, periphs_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( cdimono2_servo_mem, AS_PROGRAM, 8, cdi_state )
	AM_RANGE(0x0000, 0x001f) AM_READWRITE(servo_io_r, servo_io_w)
	AM_RANGE(0x0050, 0x00ff) AM_RAM
	AM_RANGE(0x0100, 0x1fff) AM_ROM AM_REGION("servo", 0x100)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cdimono2_slave_mem, AS_PROGRAM, 8, cdi_state )
	AM_RANGE(0x0000, 0x001f) AM_READWRITE(slave_io_r, slave_io_w)
	AM_RANGE(0x0050, 0x00ff) AM_RAM
	AM_RANGE(0x0100, 0x1fff) AM_ROM AM_REGION("slave", 0x100)
ADDRESS_MAP_END

/*************************
*      Input ports       *
*************************/

INPUT_CHANGED_MEMBER(cdi_state::mcu_input)
{
	bool send = false;

	switch((FPTR)param)
	{
		case 0x39:
			if(m_input1 && m_input1->read() & 0x01) send = true;
			break;
		case 0x37:
			if(m_input1 && m_input1->read() & 0x02) send = true;
			break;
		case 0x31:
			if(m_input1 && m_input1->read() & 0x04) send = true;
			break;
		case 0x32:
			if(m_input1 && m_input1->read() & 0x08) send = true;
			break;
		case 0x33:
			if(m_input1 && m_input1->read() & 0x10) send = true;
			break;

		case 0x30:
			if(m_input2 && m_input2->read() & 0x01) send = true;
			break;
		case 0x38:
			if(m_input2 && m_input2->read() & 0x02) send = true;
			break;
		case 0x34:
			if(m_input2 && m_input2->read() & 0x04) send = true;
			break;
		case 0x35:
			if(m_input2 && m_input2->read() & 0x08) send = true;
			break;
		case 0x36:
			if(m_input2 && m_input2->read() & 0x10) send = true;
			break;
	}

	if(send)
	{
		UINT8 data = (UINT8)((FPTR)param & 0x000000ff);
		m_scc->quizard_rx(data);
	}
}

static INPUT_PORTS_START( cdi )
	PORT_START("MOUSEX")
	PORT_BIT(0x3ff, 0x000, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0x3ff) PORT_KEYDELTA(2) PORT_CHANGED_MEMBER("slave_hle", cdislave_device, mouse_update, 0)

	PORT_START("MOUSEY")
	PORT_BIT(0x3ff, 0x000, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0x3ff) PORT_KEYDELTA(2) PORT_CHANGED_MEMBER("slave_hle", cdislave_device, mouse_update, 0)

	PORT_START("MOUSEBTN")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Mouse Button 1") PORT_CHANGED_MEMBER("slave_hle", cdislave_device, mouse_update, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Mouse Button 2") PORT_CHANGED_MEMBER("slave_hle", cdislave_device, mouse_update, 0)
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DEBUG")
	PORT_CONFNAME( 0x01, 0x00, "Plane A Disable")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_CONFNAME( 0x02, 0x00, "Plane B Disable")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x02, DEF_STR( On ) )
	PORT_CONFNAME( 0x04, 0x00, "Force Backdrop Color")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x04, DEF_STR( On ) )
	PORT_CONFNAME( 0xf0, 0x00, "Backdrop Color")
	PORT_CONFSETTING(    0x00, "Black" )
	PORT_CONFSETTING(    0x10, "Half-Bright Blue" )
	PORT_CONFSETTING(    0x20, "Half-Bright Green" )
	PORT_CONFSETTING(    0x30, "Half-Bright Cyan" )
	PORT_CONFSETTING(    0x40, "Half-Bright Red" )
	PORT_CONFSETTING(    0x50, "Half-Bright Magenta" )
	PORT_CONFSETTING(    0x60, "Half-Bright Yellow" )
	PORT_CONFSETTING(    0x70, "Half-Bright White" )
	PORT_CONFSETTING(    0x80, "Black (Alternate)" )
	PORT_CONFSETTING(    0x90, "Blue" )
	PORT_CONFSETTING(    0xa0, "Green" )
	PORT_CONFSETTING(    0xb0, "Cyan" )
	PORT_CONFSETTING(    0xc0, "Red" )
	PORT_CONFSETTING(    0xd0, "Magenta" )
	PORT_CONFSETTING(    0xe0, "Yellow" )
	PORT_CONFSETTING(    0xf0, "White" )
INPUT_PORTS_END

static INPUT_PORTS_START( cdimono2 )
	PORT_START("MOUSEX")
	PORT_BIT(0x3ff, 0x000, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0x3ff) PORT_KEYDELTA(2) //PORT_CHANGED_MEMBER("slave_hle", cdislave_device, mouse_update, 0)

	PORT_START("MOUSEY")
	PORT_BIT(0x3ff, 0x000, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0x3ff) PORT_KEYDELTA(2) //PORT_CHANGED_MEMBER("slave_hle", cdislave_device, mouse_update, 0)

	PORT_START("MOUSEBTN")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Mouse Button 1") //PORT_CHANGED_MEMBER("slave_hle", cdislave_device, mouse_update, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Mouse Button 2") //PORT_CHANGED_MEMBER("slave_hle", cdislave_device, mouse_update, 0)
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DEBUG")
	PORT_CONFNAME( 0x01, 0x00, "Plane A Disable")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_CONFNAME( 0x02, 0x00, "Plane B Disable")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x02, DEF_STR( On ) )
	PORT_CONFNAME( 0x04, 0x00, "Force Backdrop Color")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x04, DEF_STR( On ) )
	PORT_CONFNAME( 0xf0, 0x00, "Backdrop Color")
	PORT_CONFSETTING(    0x00, "Black" )
	PORT_CONFSETTING(    0x10, "Half-Bright Blue" )
	PORT_CONFSETTING(    0x20, "Half-Bright Green" )
	PORT_CONFSETTING(    0x30, "Half-Bright Cyan" )
	PORT_CONFSETTING(    0x40, "Half-Bright Red" )
	PORT_CONFSETTING(    0x50, "Half-Bright Magenta" )
	PORT_CONFSETTING(    0x60, "Half-Bright Yellow" )
	PORT_CONFSETTING(    0x70, "Half-Bright White" )
	PORT_CONFSETTING(    0x80, "Black (Alternate)" )
	PORT_CONFSETTING(    0x90, "Blue" )
	PORT_CONFSETTING(    0xa0, "Green" )
	PORT_CONFSETTING(    0xb0, "Cyan" )
	PORT_CONFSETTING(    0xc0, "Red" )
	PORT_CONFSETTING(    0xd0, "Magenta" )
	PORT_CONFSETTING(    0xe0, "Yellow" )
	PORT_CONFSETTING(    0xf0, "White" )
INPUT_PORTS_END


static INPUT_PORTS_START( quizard )
	PORT_INCLUDE( cdi )

	PORT_START("INPUT1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Coin 1") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, (void*)0x39)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start 1") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, (void*)0x37)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Player 1 A") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, (void*)0x31)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Player 1 B") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, (void*)0x32)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Player 1 C") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, (void*)0x33)
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("INPUT2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_NAME("Service") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, (void*)0x30)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Start 2") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, (void*)0x38)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Player 2 A") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, (void*)0x34)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Player 2 B") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, (void*)0x35)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("Player 2 C") PORT_CHANGED_MEMBER(DEVICE_SELF, cdi_state,mcu_input, (void*)0x36)
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


INTERRUPT_GEN_MEMBER( cdi_state::mcu_frame )
{
	m_scc->mcu_frame();
}

MACHINE_RESET_MEMBER( cdi_state, cdimono1 )
{
	UINT16 *src   = (UINT16*)memregion("maincpu")->base();
	UINT16 *dst   = m_planea;
	memcpy(dst, src, 0x8);
	memset(m_servo_io_regs, 0, 0x20);
	memset(m_slave_io_regs, 0, 0x20);

	m_maincpu->reset();

	m_dmadac[0] = machine().device<dmadac_sound_device>("dac1");
	m_dmadac[1] = machine().device<dmadac_sound_device>("dac2");
}

MACHINE_RESET_MEMBER( cdi_state, cdimono2 )
{
	UINT16 *src   = (UINT16*)memregion("maincpu")->base();
	UINT16 *dst   = m_planea;
	memcpy(dst, src, 0x8);

	m_maincpu->reset();

	m_dmadac[0] = machine().device<dmadac_sound_device>("dac1");
	m_dmadac[1] = machine().device<dmadac_sound_device>("dac2");
}

MACHINE_RESET_MEMBER( cdi_state, quizard12 )
{
	MACHINE_RESET_CALL_MEMBER( cdimono1 );

	m_scc->set_quizard_mcu_value(0x021f);
	m_scc->set_quizard_mcu_ack(0x5a);
}

MACHINE_RESET_MEMBER( cdi_state, quizard17 )
{
	MACHINE_RESET_CALL_MEMBER( cdimono1 );

	m_scc->set_quizard_mcu_value(0x021f);
	m_scc->set_quizard_mcu_ack(0x5a);
}

/* Untested - copied from quizard17 */
MACHINE_RESET_MEMBER( cdi_state, quizard18 )
{
	MACHINE_RESET_CALL_MEMBER( cdimono1 );

	m_scc->set_quizard_mcu_value(0x021f);
	m_scc->set_quizard_mcu_ack(0x5a);
}

MACHINE_RESET_MEMBER( cdi_state, quizard22 )
{
	MACHINE_RESET_CALL_MEMBER( cdimono1 );

	// 0x2b1: Italian
	// 0x001: French
	// 0x188: German

	m_scc->set_quizard_mcu_value(0x188);
	m_scc->set_quizard_mcu_ack(0x59);
}

/* Untested - copied from quizard22 */
MACHINE_RESET_MEMBER( cdi_state, quizard23 )
{
	MACHINE_RESET_CALL_MEMBER( cdimono1 );

	// 0x2b1: Italian
	// 0x001: French
	// 0x188: German

	m_scc->set_quizard_mcu_value(0x188);
	m_scc->set_quizard_mcu_ack(0x59);
}

MACHINE_RESET_MEMBER( cdi_state, quizard32 )
{
	MACHINE_RESET_CALL_MEMBER( cdimono1 );

	m_scc->set_quizard_mcu_value(0x00ae);
	m_scc->set_quizard_mcu_ack(0x58);
}

/* Untested - copied from quizard32 */
MACHINE_RESET_MEMBER( cdi_state, quizard34 )
{
	MACHINE_RESET_CALL_MEMBER( cdimono1 );

	m_scc->set_quizard_mcu_value(0x00ae);
	m_scc->set_quizard_mcu_ack(0x58);
}

/* Untested - copied from quizard4r41 */
MACHINE_RESET_MEMBER( cdi_state, quizard4r40 )
{
	MACHINE_RESET_CALL_MEMBER( cdimono1 );

	//m_scc->set_quizard_mcu_value(0x0139);
	m_scc->set_quizard_mcu_value(0x011f);
	m_scc->set_quizard_mcu_ack(0x57);
}

MACHINE_RESET_MEMBER( cdi_state, quizard4r41 )
{
	MACHINE_RESET_CALL_MEMBER( cdimono1 );

	//m_scc->set_quizard_mcu_value(0x0139);
	m_scc->set_quizard_mcu_value(0x011f);
	m_scc->set_quizard_mcu_ack(0x57);
}

MACHINE_RESET_MEMBER( cdi_state, quizard4r42 )
{
	MACHINE_RESET_CALL_MEMBER( cdimono1 );

	m_scc->set_quizard_mcu_value(0x01ae);
	m_scc->set_quizard_mcu_ack(0x57);
}

/**************************
*     68HC05 Handlers     *
**************************/

READ8_MEMBER( cdi_state::servo_io_r )
{
	if (space.debugger_access())
	{
		return 0;
	}

	UINT8 ret = m_servo_io_regs[offset];

	switch(offset)
	{
		case m68hc05eg_io_reg_t::PORT_A_DATA:
			verboselog(*this, 1, "SERVO Port A Data read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::PORT_B_DATA:
			ret = 0x08;
			verboselog(*this, 1, "SERVO Port B Data read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::PORT_C_DATA:
			ret |= INV_CADDYSWITCH_IN;
			verboselog(*this, 1, "SERVO Port C Data read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::PORT_D_INPUT:
			verboselog(*this, 1, "SERVO Port D Input read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::PORT_A_DDR:
			verboselog(*this, 1, "SERVO Port A DDR read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::PORT_B_DDR:
			verboselog(*this, 1, "SERVO Port B DDR read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::PORT_C_DDR:
			verboselog(*this, 1, "SERVO Port C DDR read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::SPI_CTRL:
			verboselog(*this, 1, "SERVO SPI Control read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::SPI_STATUS:
			verboselog(*this, 1, "SERVO SPI Status read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::SPI_DATA:
			verboselog(*this, 1, "SERVO SPI Data read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::SCC_BAUD:
			verboselog(*this, 1, "SERVO SCC Baud Rate read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::SCC_CTRL1:
			verboselog(*this, 1, "SERVO SCC Control 1 read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::SCC_CTRL2:
			verboselog(*this, 1, "SERVO SCC Control 2 read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::SCC_STATUS:
			verboselog(*this, 1, "SERVO SCC Status read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::SCC_DATA:
			verboselog(*this, 1, "SERVO SCC Data read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::TIMER_CTRL:
			verboselog(*this, 1, "SERVO Timer Control read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::TIMER_STATUS:
			verboselog(*this, 1, "SERVO Timer Status read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::ICAP_HI:
			verboselog(*this, 1, "SERVO Input Capture Hi read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::ICAP_LO:
			verboselog(*this, 1, "SERVO Input Capture Lo read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::OCMP_HI:
			verboselog(*this, 1, "SERVO Output Compare Hi read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::OCMP_LO:
			verboselog(*this, 1, "SERVO Output Compare Lo read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::COUNT_HI:
		{
			const UINT16 count = (m_servo->total_cycles() / 4) & 0x0000ffff;
			ret = count >> 8;
			verboselog(*this, 1, "SERVO Count Hi read (%02x)\n", ret);
			break;
		}
		case m68hc05eg_io_reg_t::COUNT_LO:
		{
			const UINT16 count = (m_servo->total_cycles() / 4) & 0x0000ffff;
			ret = count & 0x00ff;
			verboselog(*this, 1, "SERVO Count Lo read (%02x)\n", ret);
			break;
		}
		case m68hc05eg_io_reg_t::ACOUNT_HI:
		{
			const UINT16 count = (m_servo->total_cycles() / 4) & 0x0000ffff;
			ret = count >> 8;
			verboselog(*this, 1, "SERVO Alternate Count Hi read (%02x)\n", ret);
			break;
		}
		case m68hc05eg_io_reg_t::ACOUNT_LO:
		{
			const UINT16 count = (m_servo->total_cycles() / 4) & 0x0000ffff;
			ret = count & 0x00ff;
			verboselog(*this, 1, "SERVO Alternate Count Lo read (%02x)\n", ret);
			break;
		}
		default:
			verboselog(*this, 0, "Unknown SERVO I/O read (%02x)\n", offset);
			break;
	}

	return ret;
}


WRITE8_MEMBER( cdi_state::servo_io_w )
{
	switch(offset)
	{
		case m68hc05eg_io_reg_t::PORT_A_DATA:
			verboselog(*this, 1, "SERVO Port A Data write (%02x)\n", data);
			return;
		case m68hc05eg_io_reg_t::PORT_B_DATA:
			verboselog(*this, 1, "SERVO Port B Data write (%02x)\n", data);
			return;
		case m68hc05eg_io_reg_t::PORT_C_DATA:
			verboselog(*this, 1, "SERVO Port C Data write (%02x)\n", data);
			return;
		case m68hc05eg_io_reg_t::PORT_D_INPUT:
			verboselog(*this, 1, "SERVO Port D Input write (%02x)\n", data);
			return;
		case m68hc05eg_io_reg_t::PORT_A_DDR:
			verboselog(*this, 1, "SERVO Port A DDR write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::PORT_B_DDR:
			verboselog(*this, 1, "SERVO Port B DDR write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::PORT_C_DDR:
			verboselog(*this, 1, "SERVO Port C DDR write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::SPI_CTRL:
			verboselog(*this, 1, "SERVO SPI Control write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::SPI_STATUS:
			verboselog(*this, 1, "SERVO SPI Status write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::SPI_DATA:
			verboselog(*this, 1, "SERVO SPI Data write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::SCC_BAUD:
			verboselog(*this, 1, "SERVO SCC Baud Rate write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::SCC_CTRL1:
			verboselog(*this, 1, "SERVO SCC Control 1 write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::SCC_CTRL2:
			verboselog(*this, 1, "SERVO SCC Control 2 write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::SCC_STATUS:
			verboselog(*this, 1, "SERVO SCC Status write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::SCC_DATA:
			verboselog(*this, 1, "SERVO SCC Data write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::TIMER_CTRL:
			verboselog(*this, 1, "SERVO Timer Control write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::TIMER_STATUS:
			verboselog(*this, 1, "SERVO Timer Status write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::ICAP_HI:
			verboselog(*this, 1, "SERVO Input Capture Hi write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::ICAP_LO:
			verboselog(*this, 1, "SERVO Input Capture Lo write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::OCMP_HI:
			verboselog(*this, 1, "SERVO Output Compare Hi write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::OCMP_LO:
			verboselog(*this, 1, "SERVO Output Compare Lo write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::COUNT_HI:
			verboselog(*this, 1, "SERVO Count Hi write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::COUNT_LO:
			verboselog(*this, 1, "SERVO Count Lo write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::ACOUNT_HI:
			verboselog(*this, 1, "SERVO Alternate Count Hi write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::ACOUNT_LO:
			verboselog(*this, 1, "SERVO Alternate Count Lo write (%02x)\n", data);
			break;
		default:
			verboselog(*this, 0, "Unknown SERVO I/O write (%02x = %02x)\n", offset, data);
			break;
	}

	m_servo_io_regs[offset] = data;
}

READ8_MEMBER( cdi_state::slave_io_r )
{
	if (space.debugger_access())
	{
		return 0;
	}

	UINT8 ret = m_slave_io_regs[offset];

	switch(offset)
	{
		case m68hc05eg_io_reg_t::PORT_A_DATA:
			verboselog(*this, 1, "SLAVE Port A Data read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::PORT_B_DATA:
			verboselog(*this, 1, "SLAVE Port B Data read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::PORT_C_DATA:
			verboselog(*this, 1, "SLAVE Port C Data read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::PORT_D_INPUT:
			verboselog(*this, 1, "SLAVE Port D Input read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::PORT_A_DDR:
			verboselog(*this, 1, "SLAVE Port A DDR read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::PORT_B_DDR:
			verboselog(*this, 1, "SLAVE Port B DDR read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::PORT_C_DDR:
			verboselog(*this, 1, "SLAVE Port C DDR read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::SPI_CTRL:
			verboselog(*this, 1, "SLAVE SPI Control read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::SPI_STATUS:
			verboselog(*this, 1, "SLAVE SPI Status read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::SPI_DATA:
			verboselog(*this, 1, "SLAVE SPI Data read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::SCC_BAUD:
			verboselog(*this, 1, "SLAVE SCC Baud Rate read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::SCC_CTRL1:
			verboselog(*this, 1, "SLAVE SCC Control 1 read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::SCC_CTRL2:
			verboselog(*this, 1, "SLAVE SCC Control 2 read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::SCC_STATUS:
			verboselog(*this, 1, "SLAVE SCC Status read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::SCC_DATA:
			verboselog(*this, 1, "SLAVE SCC Data read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::TIMER_CTRL:
			verboselog(*this, 1, "SLAVE Timer Control read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::TIMER_STATUS:
			verboselog(*this, 1, "SLAVE Timer Status read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::ICAP_HI:
			verboselog(*this, 1, "SLAVE Input Capture Hi read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::ICAP_LO:
			verboselog(*this, 1, "SLAVE Input Capture Lo read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::OCMP_HI:
			verboselog(*this, 1, "SLAVE Output Compare Hi read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::OCMP_LO:
			verboselog(*this, 1, "SLAVE Output Compare Lo read (%02x)\n", ret);
			break;
		case m68hc05eg_io_reg_t::COUNT_HI:
		{
			const UINT16 count = (m_slave->total_cycles() / 4) & 0x0000ffff;
			ret = count >> 8;
			verboselog(*this, 1, "SLAVE Count Hi read (%02x)\n", ret);
			break;
		}
		case m68hc05eg_io_reg_t::COUNT_LO:
		{
			const UINT16 count = (m_slave->total_cycles() / 4) & 0x0000ffff;
			ret = count & 0x00ff;
			verboselog(*this, 1, "SLAVE Count Lo read (%02x)\n", ret);
			break;
		}
		case m68hc05eg_io_reg_t::ACOUNT_HI:
		{
			const UINT16 count = (m_slave->total_cycles() / 4) & 0x0000ffff;
			ret = count >> 8;
			verboselog(*this, 1, "SLAVE Alternate Count Hi read (%02x)\n", ret);
			break;
		}
		case m68hc05eg_io_reg_t::ACOUNT_LO:
		{
			const UINT16 count = (m_slave->total_cycles() / 4) & 0x0000ffff;
			ret = count & 0x00ff;
			verboselog(*this, 1, "SLAVE Alternate Count Lo read (%02x)\n", ret);
			break;
		}
		default:
			verboselog(*this, 0, "Unknown SLAVE I/O read (%02x)\n", offset);
			break;
	}

	return ret;
}

WRITE8_MEMBER( cdi_state::slave_io_w )
{
	switch(offset)
	{
		case m68hc05eg_io_reg_t::PORT_A_DATA:
			verboselog(*this, 1, "SLAVE Port A Data write (%02x)\n", data);
			return;
		case m68hc05eg_io_reg_t::PORT_B_DATA:
			verboselog(*this, 1, "SLAVE Port B Data write (%02x)\n", data);
			return;
		case m68hc05eg_io_reg_t::PORT_C_DATA:
			verboselog(*this, 1, "SLAVE Port C Data write (%02x)\n", data);
			return;
		case m68hc05eg_io_reg_t::PORT_D_INPUT:
			verboselog(*this, 1, "SLAVE Port D Input write (%02x)\n", data);
			return;
		case m68hc05eg_io_reg_t::PORT_A_DDR:
			verboselog(*this, 1, "SLAVE Port A DDR write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::PORT_B_DDR:
			verboselog(*this, 1, "SLAVE Port B DDR write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::PORT_C_DDR:
			verboselog(*this, 1, "SLAVE Port C DDR write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::SPI_CTRL:
			verboselog(*this, 1, "SLAVE SPI Control write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::SPI_STATUS:
			verboselog(*this, 1, "SLAVE SPI Status write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::SPI_DATA:
			verboselog(*this, 1, "SLAVE SPI Data write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::SCC_BAUD:
			verboselog(*this, 1, "SLAVE SCC Baud Rate write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::SCC_CTRL1:
			verboselog(*this, 1, "SLAVE SCC Control 1 write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::SCC_CTRL2:
			verboselog(*this, 1, "SLAVE SCC Control 2 write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::SCC_STATUS:
			verboselog(*this, 1, "SLAVE SCC Status write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::SCC_DATA:
			verboselog(*this, 1, "SLAVE SCC Data write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::TIMER_CTRL:
			verboselog(*this, 1, "SLAVE Timer Control write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::TIMER_STATUS:
			verboselog(*this, 1, "SLAVE Timer Status write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::ICAP_HI:
			verboselog(*this, 1, "SLAVE Input Capture Hi write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::ICAP_LO:
			verboselog(*this, 1, "SLAVE Input Capture Lo write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::OCMP_HI:
			verboselog(*this, 1, "SLAVE Output Compare Hi write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::OCMP_LO:
			verboselog(*this, 1, "SLAVE Output Compare Lo write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::COUNT_HI:
			verboselog(*this, 1, "SLAVE Count Hi write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::COUNT_LO:
			verboselog(*this, 1, "SLAVE Count Lo write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::ACOUNT_HI:
			verboselog(*this, 1, "SLAVE Alternate Count Hi write (%02x)\n", data);
			break;
		case m68hc05eg_io_reg_t::ACOUNT_LO:
			verboselog(*this, 1, "SLAVE Alternate Count Lo write (%02x)\n", data);
			break;
		default:
			verboselog(*this, 0, "Unknown SLAVE I/O write (%02x = %02x)\n", offset, data);
			break;
	}

	m_slave_io_regs[offset] = data;
}

/*************************
*    Machine Drivers     *
*************************/

// CD-i Mono-I system base
static MACHINE_CONFIG_START( cdimono1_base, cdi_state )
	MCFG_CPU_ADD("maincpu", SCC68070, CLOCK_A/2)
	MCFG_CPU_PROGRAM_MAP(cdimono1_mem)

	MCFG_MCD212_ADD("mcd212")
	MCFG_MCD212_SET_SCREEN("screen")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(384, 302)
	MCFG_SCREEN_VISIBLE_AREA(0, 384-1, 22, 302-1) // TODO: dynamic resolution
	MCFG_SCREEN_UPDATE_DRIVER(cdi_state, screen_update_cdimono1)

	MCFG_SCREEN_ADD("lcd", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(192, 22)
	MCFG_SCREEN_VISIBLE_AREA(0, 192-1, 0, 22-1)
	MCFG_SCREEN_UPDATE_DRIVER(cdi_state, screen_update_cdimono1_lcd)

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_DEFAULT_LAYOUT(layout_cdi)

	MCFG_CDI68070_ADD("scc68070")
	MCFG_CDICDIC_ADD("cdic")
	MCFG_CDISLAVE_ADD("slave_hle")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD( "dac1", DMADAC, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "lspeaker", 1.0 )

	MCFG_SOUND_ADD( "dac2", DMADAC, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "rspeaker", 1.0 )

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "lspeaker", 1.0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "rspeaker", 1.0 )

	MCFG_MK48T08_ADD( "mk48t08" )
MACHINE_CONFIG_END

// CD-i model 220 (Mono-II, NTSC)
static MACHINE_CONFIG_START( cdimono2, cdi_state )
	MCFG_CPU_ADD("maincpu", SCC68070, CLOCK_A/2)
	MCFG_CPU_PROGRAM_MAP(cdimono2_mem)

	MCFG_MCD212_ADD("mcd212")
	MCFG_MCD212_SET_SCREEN("screen")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(384, 302)
	MCFG_SCREEN_VISIBLE_AREA(0, 384-1, 22, 302-1) // TODO: dynamic resolution
	MCFG_SCREEN_UPDATE_DRIVER(cdi_state, screen_update_cdimono1)

	MCFG_SCREEN_ADD("lcd", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(192, 22)
	MCFG_SCREEN_VISIBLE_AREA(0, 192-1, 0, 22-1)
	MCFG_SCREEN_UPDATE_DRIVER(cdi_state, screen_update_cdimono1_lcd)

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_DEFAULT_LAYOUT(layout_cdi)

	MCFG_MACHINE_RESET_OVERRIDE( cdi_state, cdimono2 )

	MCFG_CDI68070_ADD("scc68070")
	MCFG_CPU_ADD("servo", M68HC05EG, 2000000) /* Unknown clock speed, docs say 2MHz internal clock */
	MCFG_CPU_PROGRAM_MAP(cdimono2_servo_mem)
	MCFG_CPU_ADD("slave", M68HC05EG, 2000000) /* Unknown clock speed, docs say 2MHz internal clock */
	MCFG_CPU_PROGRAM_MAP(cdimono2_slave_mem)

	MCFG_CDROM_ADD( "cdrom" )
	MCFG_CDROM_INTERFACE("cdi_cdrom")
	MCFG_SOFTWARE_LIST_ADD("cd_list","cdi")
	MCFG_SOFTWARE_LIST_FILTER("cd_list","!DVC")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD( "dac1", DMADAC, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "lspeaker", 1.0 )

	MCFG_SOUND_ADD( "dac2", DMADAC, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "rspeaker", 1.0 )

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "lspeaker", 1.0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "rspeaker", 1.0 )

	MCFG_MK48T08_ADD( "mk48t08" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( cdi910, cdi_state )
	MCFG_CPU_ADD("maincpu", SCC68070, CLOCK_A/2)
	MCFG_CPU_PROGRAM_MAP(cdi910_mem)

	MCFG_MCD212_ADD("mcd212")
	MCFG_MCD212_SET_SCREEN("screen")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(384, 302)
	MCFG_SCREEN_VISIBLE_AREA(0, 384-1, 22, 302-1) // TODO: dynamic resolution
	MCFG_SCREEN_UPDATE_DRIVER(cdi_state, screen_update_cdimono1)

	MCFG_SCREEN_ADD("lcd", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(192, 22)
	MCFG_SCREEN_VISIBLE_AREA(0, 192-1, 0, 22-1)
	MCFG_SCREEN_UPDATE_DRIVER(cdi_state, screen_update_cdimono1_lcd)

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_DEFAULT_LAYOUT(layout_cdi)

	MCFG_MACHINE_RESET_OVERRIDE( cdi_state, cdimono2 )

	MCFG_CDI68070_ADD("scc68070")
	MCFG_CPU_ADD("servo", M68HC05EG, 2000000) /* Unknown clock speed, docs say 2MHz internal clock */
	MCFG_CPU_PROGRAM_MAP(cdimono2_servo_mem)
	MCFG_CPU_ADD("slave", M68HC05EG, 2000000) /* Unknown clock speed, docs say 2MHz internal clock */
	MCFG_CPU_PROGRAM_MAP(cdimono2_slave_mem)

	MCFG_CDROM_ADD( "cdrom" )
	MCFG_CDROM_INTERFACE("cdi_cdrom")
	MCFG_SOFTWARE_LIST_ADD("cd_list","cdi")
	MCFG_SOFTWARE_LIST_FILTER("cd_list","!DVC")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD( "dac1", DMADAC, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "lspeaker", 1.0 )

	MCFG_SOUND_ADD( "dac2", DMADAC, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "rspeaker", 1.0 )

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "lspeaker", 1.0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "rspeaker", 1.0 )

	MCFG_MK48T08_ADD( "mk48t08" )
MACHINE_CONFIG_END

// CD-i Mono-I, with CD-ROM image device (MESS) and Software List (MESS)
static MACHINE_CONFIG_DERIVED( cdimono1, cdimono1_base )
	MCFG_MACHINE_RESET_OVERRIDE(cdi_state, cdimono1)

	MCFG_CDROM_ADD( "cdrom" )
	MCFG_CDROM_INTERFACE("cdi_cdrom")
	MCFG_SOFTWARE_LIST_ADD("cd_list","cdi")
	MCFG_SOFTWARE_LIST_FILTER("cd_list","!DVC")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quizard, cdimono1_base )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(cdimono1_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cdi_state, mcu_frame)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quizard12, quizard )
	MCFG_MACHINE_RESET_OVERRIDE(cdi_state, quizard12 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quizard17, quizard )
	MCFG_MACHINE_RESET_OVERRIDE(cdi_state, quizard17 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quizard18, quizard )
	MCFG_MACHINE_RESET_OVERRIDE(cdi_state, quizard18 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quizard22, quizard )
	MCFG_MACHINE_RESET_OVERRIDE(cdi_state, quizard22 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quizard23, quizard )
	MCFG_MACHINE_RESET_OVERRIDE(cdi_state, quizard23 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quizard32, quizard )
	MCFG_MACHINE_RESET_OVERRIDE(cdi_state, quizard32 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quizard3, quizard )
	MCFG_MACHINE_RESET_OVERRIDE(cdi_state, quizard34 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quizard4r40, quizard )
	MCFG_MACHINE_RESET_OVERRIDE(cdi_state, quizard4r40 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quizard4r41, quizard )
	MCFG_MACHINE_RESET_OVERRIDE(cdi_state, quizard4r41 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quizard4r42, quizard )
	MCFG_MACHINE_RESET_OVERRIDE(cdi_state, quizard4r42 )
MACHINE_CONFIG_END




/*************************
*        Rom Load        *
*************************/

ROM_START( cdimono1 )
	ROM_REGION(0x80000, "maincpu", 0) // these roms need byteswapping
	ROM_SYSTEM_BIOS( 0, "mcdi200", "Magnavox CD-i 200" )
	ROMX_LOAD( "cdi200.rom", 0x000000, 0x80000, CRC(40c4e6b9) SHA1(d961de803c89b3d1902d656ceb9ce7c02dccb40a), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "pcdi220", "Philips CD-i 220 F2" )
	ROMX_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "pcdi220_alt", "Philips CD-i 220?" ) // doesn't boot
	ROMX_LOAD( "cdi220.rom", 0x000000, 0x80000, CRC(584c0af8) SHA1(5d757ab46b8c8fc36361555d978d7af768342d47), ROM_BIOS(3) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping
ROM_END



ROM_START( cdi910 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "cdi910", "CD-I 910-17P Mini-MMC" )
	ROMX_LOAD( "philips__cd-i_2.1__mb834200b-15__26b_aa__9224_z01.tc574200.7211", 0x000000, 0x80000, CRC(4ae3bee3) SHA1(9729b4ee3ce0c17172d062339c47b1ab822b222b), ROM_BIOS(1) | ROM_GROUPWORD | ROM_REVERSE )
	ROM_SYSTEM_BIOS( 1, "cdi910_alt", "alt" )
	ROMX_LOAD( "cdi910.rom", 0x000000, 0x80000, CRC(2f3048d2) SHA1(11c4c3e602060518b52e77156345fa01f619e793), ROM_BIOS(2) | ROM_GROUPWORD | ROM_REVERSE )

	// cdic

	ROM_REGION(0x2000, "servo", 0)
	ROM_LOAD( "zx405037p__cdi_servo_2.1__b43t__llek9215.mc68hc705c8a_withtestrom.7201", 0x0000, 0x2000, CRC(7a3af407) SHA1(fdf8d78d6a0df4a56b5b963d72eabd39fcec163f) )

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "zx405042p__cdi_slave_2.0__b43t__zzmk9213.mc68hc705c8a_withtestrom.7206", 0x0000, 0x2000, CRC(688cda63) SHA1(56d0acd7caad51c7de703247cd6d842b36173079) )

	ROM_REGION(0x2000, "pals", 0)
	ROM_LOAD( "ti_portugal_206xf__tibpal20l8-15cnt__m7205n.7205.bin",      0x0000, 0x144, CRC(dd167e0d) SHA1(2ba82a4619d7a0f19e62e02a2841afd4d45d56ba) )
	ROM_LOAD( "ti_portugal_774_206xf__tibpal16l8-10cn_m7204n.7204.bin",    0x0000, 0x104, CRC(04e6bd37) SHA1(153d1a977291bedb7420484a9f889325dbd3628e) )
ROM_END

ROM_START( cdimono2 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "philips__cdi-220_ph3_r1.2__mb834200b-15__02f_aa__9402_z04.tc574200-le._1.7211", 0x000000, 0x80000, CRC(17d723e7) SHA1(6c317a82e35d60ca5e7a74fc99f665055693169d) )

	ROM_REGION(0x2000, "servo", 0)
	ROM_LOAD( "zc405351p__servo_cdi_4.1__0d67p__lluk9404.mc68hc705c8a.7490", 0x0000, 0x2000, CRC(2bc8e4e9) SHA1(8cd052b532fc052d6b0077261c12f800e8655bb1) )

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "zc405352p__slave_cdi_4.1__0d67p__lltr9403.mc68hc705c8a.7206", 0x0000, 0x2000, CRC(5b19da07) SHA1(cf02d84977050c71e87a38f1249e83c43a93949b) )
ROM_END


ROM_START( cdi490a )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "cdi490", "CD-i 490" )
	ROMX_LOAD( "cdi490a.rom", 0x000000, 0x80000, CRC(e2f200f6) SHA1(c9bf3c4c7e4fe5cbec3fe3fc993c77a4522ca547), ROM_BIOS(1) | ROM_GROUPWORD | ROM_REVERSE  )

	ROM_REGION(0x40000, "mpegs", 0) // keep these somewhere
	ROM_LOAD( "impega.rom", 0x0000, 0x40000, CRC(84d6f6aa) SHA1(02526482a0851ea2a7b582d8afaa8ef14a8bd914) )
	ROM_LOAD( "vmpega.rom", 0x0000, 0x40000, CRC(db264e8b) SHA1(be407fbc102f1731a0862554855e963e5a47c17b) )
ROM_END




ROM_START( cdibios ) // for the quizard sets
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "mcdi200", "Magnavox CD-i 200" )
	ROMX_LOAD( "cdi200.rom", 0x000000, 0x80000, CRC(40c4e6b9) SHA1(d961de803c89b3d1902d656ceb9ce7c02dccb40a), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "pcdi220", "Philips CD-i 220 F2" )
	ROMX_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e), ROM_BIOS(2) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping
ROM_END

// Quizard

ROM_START( quizard ) /* CD-ROM printed ??/?? */
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard18", 0, BAD_DUMP SHA1(ede873b22957f2a707bbd3039e962ef2ca5aedbd) )
ROM_END

ROM_START( quizard_17 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard17", 0, BAD_DUMP SHA1(4bd698f076505b4e17be978481bce027eb47123b) )
ROM_END

ROM_START( quizard_12 ) /* CD-ROM printed 01/95 */
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard12", 0, BAD_DUMP SHA1(6e41683b96b74e903040842aeb18437ad7813c82) )
ROM_END

ROM_START( quizard_10 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping


	// software: BurnAtOnce 0.99.5 / CHDMAN 0.163
	// Drive: TS-L633R
	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard10", 0, SHA1(5715db50f0d5ffe06f47c0943f4bf0481ab6048e) )
ROM_END


// Quizard 2

ROM_START( quizard2 ) /* CD-ROM printed ??/?? */
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard23", 0, BAD_DUMP SHA1(cd909d9a54275d6f2d36e03e83eea996e781b4d3) )
ROM_END

ROM_START( quizard2_22 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard22", 0, BAD_DUMP SHA1(03c8fdcf27ead6e221691111e8c679b551099543) )
ROM_END

// Quizard 3

ROM_START( quizard3 ) /* CD-ROM printed ??/?? */
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard34", 0, BAD_DUMP SHA1(37ad49b72b5175afbb87141d57bc8604347fe032) )
ROM_END

ROM_START( quizard3_32 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard32", 0, BAD_DUMP SHA1(31e9fa2169aa44d799c37170b238134ab738e1a1) )
ROM_END


ROM_START( quizard4 ) /* CD-ROM printed 09/98 */
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard4r42", 0, BAD_DUMP SHA1(a5d5c8950b4650b8753f9119dc7f1ccaa2aa5442) )
ROM_END

ROM_START( quizard4_41 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard4r41", 0, BAD_DUMP SHA1(2c0484c6545aac8e00b318328c6edce6f5dde43d) )
ROM_END

ROM_START( quizard4_40 ) /* CD-ROM printed 07/97 */
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

	ROM_REGION(0x2000, "cdic", 0)
	ROM_LOAD( "cdic.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "slave.bin", 0x0000, 0x2000, NO_DUMP ) // Undumped 68HC05 microcontroller, might need decapping

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard4r40", 0, BAD_DUMP SHA1(288cc37a994e4f1cbd47aa8c92342879c6fc0b87) )
ROM_END


/*************************
*      Game driver(s)    *
*************************/

/*    YEAR  NAME      PARENT    COMPAT    MACHINE   INPUT     DEVICE         INIT      COMPANY     FULLNAME */

// BIOS / System
CONS( 1991, cdimono1, 0,        0,        cdimono1, cdi,      driver_device, 0,        "Philips",  "CD-i (Mono-I) (PAL)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE  )
CONS( 1991, cdimono2, 0,        0,        cdimono2, cdimono2, driver_device, 0,        "Philips",  "CD-i (Mono-II) (NTSC)",   MACHINE_NOT_WORKING )
CONS( 1991, cdi910,   0,        0,        cdi910,   cdimono2, driver_device, 0,        "Philips",  "CD-i 910-17P Mini-MMC (PAL)",   MACHINE_NOT_WORKING  )
CONS( 1991, cdi490a,  0,        0,        cdimono1, cdi,      driver_device, 0,        "Philips",  "CD-i 490",   MACHINE_NOT_WORKING  )

// The Quizard games are RETAIL CD-i units, with additional JAMMA adapters & dongles for protection, hence being 'clones' of the system.

GAME( 1995, cdibios,  0,               cdimono1_base,  quizard, driver_device,      0, ROT0,     "Philips",      "CD-i (Mono-I) (PAL) BIOS", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IS_BIOS_ROOT )

GAME( 1995, quizard,     cdibios,      quizard18,      quizard, driver_device,      0, ROT0,     "TAB Austria",  "Quizard (v1.8)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 1995, quizard_17,  quizard,      quizard17,      quizard, driver_device,      0, ROT0,     "TAB Austria",  "Quizard (v1.7)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 1995, quizard_12,  quizard,      quizard12,      quizard, driver_device,      0, ROT0,     "TAB Austria",  "Quizard (v1.2)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 1995, quizard_10,  quizard,      quizard12,      quizard, driver_device,      0, ROT0,     "TAB Austria",  "Quizard (v1.0)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )

GAME( 1995, quizard2,    cdibios,      quizard23,      quizard, driver_device,      0, ROT0,     "TAB Austria",  "Quizard 2 (v2.3)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 1995, quizard2_22, quizard2,     quizard22,      quizard, driver_device,      0, ROT0,     "TAB Austria",  "Quizard 2 (v2.2)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )

GAME( 1995, quizard3,    cdibios,      quizard3,       quizard, driver_device,      0, ROT0,     "TAB Austria",  "Quizard 3 (v3.4)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 1996, quizard3_32, quizard3,     quizard32,      quizard, driver_device,      0, ROT0,     "TAB Austria",  "Quizard 3 (v3.2)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )

GAME( 1998, quizard4,    cdibios,      quizard4r42,    quizard, driver_device,      0, ROT0,     "TAB Austria",  "Quizard 4 Rainbow (v4.2)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION ) // doesn't boot
GAME( 1998, quizard4_41, quizard4,     quizard4r41,    quizard, driver_device,      0, ROT0,     "TAB Austria",  "Quizard 4 Rainbow (v4.1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 1997, quizard4_40, quizard4,     quizard4r40,    quizard, driver_device,      0, ROT0,     "TAB Austria",  "Quizard 4 Rainbow (v4.0)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )
