// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 */
#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "bus/rs232/rs232.h"
#include "machine/68230pit.h"
#include "machine/68153bim.h"
#include "machine/68561mpcc.h"
#include "machine/clock.h"
#include "vme_fccpu20.h"

#define LOG_GENERAL 0x01
#define LOG_SETUP   0x02
#define LOG_PRINTF  0x04
#define LOG_INT		0x08

#define VERBOSE 0 //(LOG_PRINTF | LOG_SETUP  | LOG_GENERAL)

#define LOGMASK(mask, ...)   do { if (VERBOSE & mask) logerror(__VA_ARGS__); } while (0)
#define LOGLEVEL(mask, level, ...) do { if ((VERBOSE & mask) >= level) logerror(__VA_ARGS__); } while (0)

#define LOG(...)      LOGMASK(LOG_GENERAL, __VA_ARGS__)
#define LOGSETUP(...) LOGMASK(LOG_SETUP,   __VA_ARGS__)
#define LOGINT(...)   LOGMASK(LOG_SETUP,   __VA_ARGS__)

#if VERBOSE & LOG_PRINTF
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//	GLOBAL VARIABLES
//**************************************************************************

const device_type VME_FCCPU20 = &device_creator<vme_fccpu20_card_device>;

#define CPU_CLOCK XTAL_20MHz /* HCJ */
#define DUSCC_CLOCK XTAL_14_7456MHz /* HCJ */

static ADDRESS_MAP_START (cpu20_mem, AS_PROGRAM, 32, vme_fccpu20_card_device)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE (0x00000000, 0x00000007) AM_ROM AM_READ (bootvect_r)   /* ROM mirror just during reset */
	AM_RANGE (0x00000000, 0x00000007) AM_RAM AM_WRITE (bootvect_w)   /* After first write we act as RAM */
	AM_RANGE (0x00000008, 0x003fffff) AM_RAM /* RAM  installed in machine start */
	AM_RANGE (0xff040000, 0xff04ffff) AM_RAM /* RAM  installed in machine start */
	AM_RANGE (0xff000000, 0xff00ffff) AM_ROM AM_REGION("roms", 0x0000)
	AM_RANGE (0xff800000, 0xff80001f) AM_DEVREADWRITE8("mpcc", mpcc68561_device, read, write, 0xffffffff)
	AM_RANGE (0xff800200, 0xff80021f) AM_DEVREADWRITE8("mpcc2", mpcc68561_device, read, write, 0xffffffff)
//	AM_RANGE (0xff800200, 0xff8003ff) AM_DEVREADWRITE8("pit2", pit68230_device, read, write, 0xff00ff00)
	AM_RANGE (0xff800600, 0xff80061f) AM_DEVREADWRITE8("mpcc3", mpcc68561_device, read, write, 0xffffffff)
	AM_RANGE (0xff800800, 0xff80080f) AM_DEVREADWRITE8("bim", bim68153_device, read, write, 0xff00ff00)
//	AM_RANGE (0xff800a00, 0xff800a0f) AM_DEVREADWRITE8("rtc", rtc_device, read, write, 0x00ff00ff)
	AM_RANGE (0xff800c00, 0xff800dff) AM_DEVREADWRITE8("pit", pit68230_device, read, write, 0xffffffff)
ADDRESS_MAP_END

/*
 * Machine configuration
 */
static MACHINE_CONFIG_FRAGMENT (fccpu20)
	/* basic machine hardware */
	MCFG_CPU_ADD ("maincpu", M68020, XTAL_16MHz) /* Crytstal not verified */
	MCFG_CPU_PROGRAM_MAP (cpu20_mem)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("bim", bim68153_device, iack)

//	MCFG_VME_DEVICE_ADD("vme")
//	MCFG_VME_SLOT_ADD ("vme", "vme1", fccpu20_vme_cards, nullptr)

	/* PIT Parallel Interface and Timer device, assumed strapped for on board clock */
	MCFG_DEVICE_ADD ("pit", PIT68230, XTAL_32MHz / 4) /* Crystal not verified */
	MCFG_PIT68230_PA_INPUT_CB(READ8(vme_fccpu20_card_device, pita_r))
	MCFG_PIT68230_PB_INPUT_CB(READ8(vme_fccpu20_card_device, pitb_r))
	MCFG_PIT68230_PC_INPUT_CB(READ8(vme_fccpu20_card_device, pitc_r))
	MCFG_PIT68230_TIMER_IRQ_CB(DEVWRITELINE("bim", bim68153_device, int2_w))

	/* BIM */
	MCFG_MC68153_ADD("bim", XTAL_32MHz / 8)
	MCFG_BIM68153_OUT_INT_CB(WRITELINE(vme_fccpu20_card_device, bim_irq_callback))
		/*INT0 - Abort switch */
		/*INT1 - MPCC@8.064 MHz aswell */
		/*INT2 - PI/T timer */
		/*INT3 - SYSFAIL/IRQVMX/ACFAIL/MPCC2/3 */

	/* MPCC */
#define RS232P1_TAG      "rs232p1"
#define RS232P2_TAG      "rs232p2"
#define RS232P3_TAG      "rs232p3"
	// MPCC
	MCFG_MPCC68561_ADD ("mpcc", XTAL_32MHz / 4, 0, 0)
	MCFG_MPCC_OUT_TXD_CB(DEVWRITELINE(RS232P1_TAG, rs232_port_device, write_txd))
	MCFG_MPCC_OUT_DTR_CB(DEVWRITELINE(RS232P1_TAG, rs232_port_device, write_dtr))
	MCFG_MPCC_OUT_RTS_CB(DEVWRITELINE(RS232P1_TAG, rs232_port_device, write_rts))
	MCFG_MPCC_OUT_INT_CB(DEVWRITELINE("bim", bim68153_device, int1_w))
	/* Additional MPCC sits on slave boards like SRAM-22 */
	// MPCC2
	MCFG_MPCC68561_ADD ("mpcc2", XTAL_32MHz / 4, 0, 0)
	MCFG_MPCC_OUT_TXD_CB(DEVWRITELINE(RS232P2_TAG, rs232_port_device, write_txd))
	MCFG_MPCC_OUT_DTR_CB(DEVWRITELINE(RS232P2_TAG, rs232_port_device, write_dtr))
	MCFG_MPCC_OUT_RTS_CB(DEVWRITELINE(RS232P2_TAG, rs232_port_device, write_rts))
	MCFG_MPCC_OUT_INT_CB(DEVWRITELINE("bim", bim68153_device, int3_w))
	// MPCC3
	MCFG_MPCC68561_ADD ("mpcc3", XTAL_32MHz / 4, 0, 0)
	MCFG_MPCC_OUT_TXD_CB(DEVWRITELINE(RS232P3_TAG, rs232_port_device, write_txd))
	MCFG_MPCC_OUT_DTR_CB(DEVWRITELINE(RS232P3_TAG, rs232_port_device, write_dtr))
	MCFG_MPCC_OUT_RTS_CB(DEVWRITELINE(RS232P3_TAG, rs232_port_device, write_rts))
	MCFG_MPCC_OUT_INT_CB(DEVWRITELINE("bim", bim68153_device, int3_w))

	// MPCC - RS232
	MCFG_RS232_PORT_ADD (RS232P1_TAG, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("mpcc", mpcc68561_device, write_rx))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("mpcc", mpcc68561_device, cts_w))

	// MPCC2 - RS232
	MCFG_RS232_PORT_ADD (RS232P2_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("mpcc2", mpcc68561_device, write_rx))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("mpcc2", mpcc68561_device, cts_w))

	// MPCC3 - RS232
	MCFG_RS232_PORT_ADD (RS232P3_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("mpcc3", mpcc68561_device, write_rx))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("mpcc3", mpcc68561_device, cts_w))
MACHINE_CONFIG_END

/* ROM definitions */
ROM_START (fccpu20) /* This is an original rom dump */
	ROM_REGION32_BE(0x10000, "roms", 0)
// Boots with Board ID set to: 0x36 (FGA002 BOOT on terminal P4, "Wait until harddisk is up to speed " on terminal P1)
	ROM_LOAD32_BYTE("L.BIN",  0x000002, 0x4000, CRC (174ab801) SHA1 (0d7b8ed29d5fdd4bd2073005008120c5f20128dd))
	ROM_LOAD32_BYTE("LL.BIN", 0x000003, 0x4000, CRC (9fd9e3e4) SHA1 (e5a7c87021e6be412dd5a8166d9f62b681169eda))
	ROM_LOAD32_BYTE("U.BIN",  0x000001, 0x4000, CRC (d1afe4c0) SHA1 (b5baf9798d73632f7bb843cbc4b306c8c03f4296))
	ROM_LOAD32_BYTE("UU.BIN", 0x000000, 0x4000, CRC (b54d623b) SHA1 (49b272184a04570b09004de71fae0ed0d1bf5929))
ROM_END

machine_config_constructor vme_fccpu20_card_device::device_mconfig_additions() const
{
	LOG("%s %s\n", tag(), FUNCNAME);
	return MACHINE_CONFIG_NAME( fccpu20 );
}

const tiny_rom_entry *vme_fccpu20_card_device::device_rom_region() const
{
	LOG("%s\n", FUNCNAME);
	return ROM_NAME( fccpu20 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************
vme_fccpu20_card_device::vme_fccpu20_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source)
	,device_vme_card_interface(mconfig, *this)
	, m_maincpu (*this, "maincpu")
	, m_pit (*this, "pit")
	, m_bim  (*this, "bim")
	, m_mpcc  (*this, "mpcc")
	, m_mpcc2  (*this, "mpcc2")
	, m_mpcc3  (*this, "mpcc3")
{
	LOG("%s\n", FUNCNAME);
}

vme_fccpu20_card_device::vme_fccpu20_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VME_FCCPU20, "Force Computer SYS68K/CPU-20 CPU Board", tag, owner, clock, "fccpu20", __FILE__)
	,device_vme_card_interface(mconfig, *this)
	, m_maincpu (*this, "maincpu")
	, m_pit (*this, "pit")
	, m_bim  (*this, "bim")
	, m_mpcc  (*this, "mpcc")
	, m_mpcc2  (*this, "mpcc2")
	, m_mpcc3  (*this, "mpcc3")
{
	LOG("%s %s\n", tag, FUNCNAME);
}

/* Start it up */
void vme_fccpu20_card_device::device_start()
{
	LOG("%s\n", FUNCNAME);
	set_vme_device();

	save_pointer (NAME (m_sysrom), sizeof(m_sysrom));
	save_pointer (NAME (m_sysram), sizeof(m_sysram));

	/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (uint32_t*)(memregion ("roms")->base());

#if 0 // TODO: Setup VME access handlers for shared memory area
	uint32_t base = 0xFFFF5000;
	m_vme->install_device(base + 0, base + 1, // Channel B - Data
							 read8_delegate(FUNC(z80sio_device::db_r),  subdevice<z80sio_device>("pit")), write8_delegate(FUNC(z80sio_device::db_w), subdevice<z80sio_device>("pit")), 0x00ff);
	m_vme->install_device(base + 2, base + 3, // Channel B - Control
							 read8_delegate(FUNC(z80sio_device::cb_r),  subdevice<z80sio_device>("pit")), write8_delegate(FUNC(z80sio_device::cb_w), subdevice<z80sio_device>("pit")), 0x00ff);
#endif

}

void vme_fccpu20_card_device::device_reset()
{
	LOG("%s\n", FUNCNAME);
}

/* Boot vector handler, the PCB hardwires the first 8 bytes from 0xff800000 to 0x0 at reset*/
READ32_MEMBER (vme_fccpu20_card_device::bootvect_r){
	LOG("%s\n", FUNCNAME);
	return m_sysrom[offset];
}

WRITE32_MEMBER (vme_fccpu20_card_device::bootvect_w){
	LOG("%s\n", FUNCNAME);
	m_sysram[offset % sizeof(m_sysram)] &= ~mem_mask;
	m_sysram[offset % sizeof(m_sysram)] |= (data & mem_mask);
	m_sysrom = &m_sysram[0]; // redirect all upcomming accesses to masking RAM until reset.
}

WRITE_LINE_MEMBER(vme_fccpu20_card_device::bim_irq_callback)
{
	LOGINT("%s(%02x)\n", FUNCNAME, state);

	bim_irq_state = state;
	bim_irq_level = m_bim->get_irq_level();
	LOGINT(" - BIM irq level  %02x\n", bim_irq_level);
	update_irq_to_maincpu();
}

void vme_fccpu20_card_device::update_irq_to_maincpu()
{
	LOGINT("%s()\n", FUNCNAME);
	LOGINT(" - bim_irq_level: %02x\n", bim_irq_level);
	LOGINT(" - bim_irq_state: %02x\n", bim_irq_state);
	switch (bim_irq_level & 0x07)
	{
	case 1: m_maincpu->set_input_line(M68K_IRQ_1, bim_irq_state); break;
	case 2: m_maincpu->set_input_line(M68K_IRQ_2, bim_irq_state); break;
	case 3: m_maincpu->set_input_line(M68K_IRQ_3, bim_irq_state); break;
	case 4: m_maincpu->set_input_line(M68K_IRQ_4, bim_irq_state); break;
	case 5: m_maincpu->set_input_line(M68K_IRQ_5, bim_irq_state); break;
	case 6: m_maincpu->set_input_line(M68K_IRQ_6, bim_irq_state); break;
	case 7: m_maincpu->set_input_line(M68K_IRQ_7, bim_irq_state); break;
	default: logerror("Programmatic error in %s, please report\n", FUNCNAME);
	}
}

/* 8 configuration DIP switches

 Baud   B3 B2 B1 B0
   9600  0  0  0  1
  28800  0  0  1  0
  38400  1  0  1  0
  57600  0  0  1  1

 B3: 8 bit 38400 baud

 B4:

 B5:

 B6: Auto execute FF00C0000

 B7: memory size?
*/
#define BR7N9600   0x01
#define BR7N28800  0x02
#define BR7N38400  0x06
#define BR7N57600  0x03
#define BR8N38400  0x08
#define FORCEBUG   0x30
READ8_MEMBER (vme_fccpu20_card_device::pita_r){
	LOG("%s\n", FUNCNAME);

	return FORCEBUG | BR7N9600;
}

/* Enabling/Disabling of VME IRQ 1-7 */
READ8_MEMBER (vme_fccpu20_card_device::pitb_r){
	LOG("%s\n", FUNCNAME);
	return 0xff;
}

/* VME bus release software settings (output) (ROR, RAT, RATAR, RATBCLR, RORAT, RORRAT */
READ8_MEMBER (vme_fccpu20_card_device::pitc_r){
	LOG("%s\n", FUNCNAME);
	return 0xff;
}

