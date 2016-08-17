// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 * Interrupt scheme and dmac hookup shamelessly based on esq5505.cpp
 *
 *  11/04/2016
 *  Force SYS68K ISCSI-1 driver - This driver will be converted into a slot device once the VME bus driver exists.
 *  The ISCSI-1 board is a VME slave board that reads command and returns results through dual ported RAM to the VME bus.
 *
 * http://bitsavers.trailing-edge.com/pdf/forceComputers/800114_Force_Introduction_to_the_SYS68K_ISCSI-1_Oct86.pdf
 *
 * Address Map
 * ----------------------------------------------------------
 * Address Range     Description
 * ----------------------------------------------------------
 * 00 0000 - 00 0007 Initialisation vectors from system EPROM
 * 00 0008 - 00 1FFF Local SRAM
 * 00 2000 - 01 FFFF Dynamic Dual Port RAM
 * C4 0000 - C4 001F SCSIbus controller
 * C8 0000 - C8 00FF DMAC
 * CC 0000 - CC 0007 FDC
 * CC 0009 - CC 0009 Control Register
 * D0 0000 - D0 003F PI/T
 * E0 0000 - E7 0000 EPROMs
 * ----------------------------------------------------------
 *
 * Interrupt sources
 * ----------------------------------------------------------
 * Description                  Device  Lvl  IRQ    VME board
 *                           /Board      Vector  Address
 * ----------------------------------------------------------
 * On board Sources
 * ABORT                        P3 p13  1    AV1
 * DMA controller               68450   2    DMAC/AV2
 * SCSI bus controller       NCR 5386S  3    AV3
 * Floppy Disk controller    WD 1772    4    AV4
 * Parallel interface and timer 68230   5    PIT timer
 *                              ---     6    ---
 * Parallel interface and timer 65230   7    PIT port
 * ----------------------------------------------------------
 *
 *  TODO:
 *  - Write and attach a NCR5386S SCSI device
 *  - Find a floppy image and present it to the WD1772 floppy controller
 *  - Add VME bus driver
 *  - Let a controller CPU board (eg CPU-1 or CPU-30) boot from floppy or SCSI disk
 *
 ****************************************************************************/
#define TODO "Driver for SCSI NCR5386s device needed\n"

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/68230pit.h"
#include "machine/wd_fdc.h"
#include "machine/hd63450.h" // compatible with MC68450
#include "machine/clock.h"

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)
#if VERBOSE == 2
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#define LLFORMAT "%I64%"
#else
#define FUNCNAME __PRETTY_FUNCTION__
#define LLFORMAT "%lld"
#endif

#define CPU_CRYSTAL XTAL_20MHz /* Jauch */
#define PIT_CRYSTAL XTAL_16MHz /* Jauch */

class fcscsi1_state : public driver_device
{
public:
fcscsi1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device (mconfig, type, tag),
		m_maincpu (*this, "maincpu")
		,m_fdc (*this, "fdc")
		,m_pit (*this, "pit")
		,m_dmac(*this, "mc68450")
		,m_tcr (0)
{
}
	DECLARE_READ16_MEMBER (bootvect_r);
	DECLARE_READ8_MEMBER (tcr_r);
	DECLARE_WRITE8_MEMBER (tcr_w);

	IRQ_CALLBACK_MEMBER(maincpu_irq_acknowledge_callback);

	//dmac
	DECLARE_WRITE8_MEMBER(dma_end);
	DECLARE_WRITE8_MEMBER(dma_error);

	//fdc
	DECLARE_WRITE8_MEMBER(fdc_irq);
	DECLARE_READ8_MEMBER(fdc_read_byte);
	DECLARE_WRITE8_MEMBER(fdc_write_byte);

	/* Dummy driver routines */
	DECLARE_READ8_MEMBER (not_implemented_r);
	DECLARE_WRITE8_MEMBER (not_implemented_w);

	UINT8 fdc_irq_state;
	UINT8 dmac_irq_state;
	int dmac_irq_vector;

	virtual void machine_start () override;
	void update_irq_to_maincpu();

protected:

private:
	required_device<cpu_device> m_maincpu;
	required_device<wd1772_t> m_fdc;
	required_device<pit68230_device> m_pit;
	required_device<hd63450_device> m_dmac;


	UINT8 m_tcr;

	// Pointer to System ROMs needed by bootvect_r
	UINT16  *m_sysrom;
};

static ADDRESS_MAP_START (fcscsi1_mem, AS_PROGRAM, 16, fcscsi1_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE (0x000000, 0x000007) AM_ROM AM_READ (bootvect_r)       /* Vectors mapped from System EPROM */
	AM_RANGE (0x000008, 0x001fff) AM_RAM /* SRAM */
	AM_RANGE (0x002000, 0x01ffff) AM_RAM /* Dual Ported RAM */
	AM_RANGE (0xe00000, 0xe7ffff) AM_ROM /* System EPROM Area 32Kb DEBUGGER supplied */
	AM_RANGE (0xd00000, 0xd0003f) AM_DEVREADWRITE8 ("pit", pit68230_device, read, write, 0x00ff)
	//AM_RANGE (0xc40000, 0xc4001f) AM_DEVREADWRITE8("scsi", ncr5386s_t, read, write, 0x00ff) /* SCSI Controller interface - device support not yet available*/
	AM_RANGE (0xc80000, 0xc800ff) AM_DEVREADWRITE("mc68450", hd63450_device, read, write)  /* DMA Controller interface */
	AM_RANGE (0xc40000, 0xc800ff) AM_READWRITE8 (not_implemented_r, not_implemented_w, 0xffff)  /* Dummy mapping af address area to display message */

	AM_RANGE (0xcc0000, 0xcc0007) AM_DEVREADWRITE8("fdc", wd1772_t, read, write, 0x00ff)      /* Floppy Controller interface */
	AM_RANGE (0xcc0008, 0xcc0009) AM_READWRITE8 (tcr_r, tcr_w, 0x00ff) /* The Control Register, SCSI ID and FD drive select bits */
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START (fcscsi1)
INPUT_PORTS_END

/* Start it up */
void fcscsi1_state::machine_start ()
{
	LOG (("machine_start\n"));

	/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (UINT16*)(memregion ("maincpu")->base () + 0xe00000);

}

/* Boot vector handler, the PCB hardwires the first 8 bytes from 0x80000 to 0x0 */
READ16_MEMBER (fcscsi1_state::bootvect_r){
	return m_sysrom [offset];
}

/* The Control Register - descretelly implemented on the PCB
Bit #: 7 6 5 4 3 2 1 0
       \ \ \ \ \ \ \ \ Floppy Disk Side Select
        \ \ \ \ \ \ \ Floppy Disk Drive Select 0
         \ \ \ \ \ \ Floppy Disk Drive Select 1
          \ \ \ \ \ Floppy Disk Drive Select 2
           \ \ \ \ Floppy Disk Drive Select 3
            \ \ \ ISCSI-l I.D. Bit #0
             \ \ ISCSI-l I.D. Bit #1
              \ ISCSI-l 1.D. Bit #2
*/

READ8_MEMBER (fcscsi1_state::tcr_r){
	LOG(("%s\n", FUNCNAME));
	return (UINT8) m_tcr;
}

WRITE8_MEMBER (fcscsi1_state::tcr_w){
	LOG(("%s [%02x]\n", FUNCNAME, data));
	m_tcr = data;
	return;
}

WRITE8_MEMBER(fcscsi1_state::dma_end)
{
	if (data != 0)
	{
		dmac_irq_state = 1;
		dmac_irq_vector = m_dmac->get_vector(offset);
	}
	else
	{
		dmac_irq_state = 0;
	}

	update_irq_to_maincpu();
}

WRITE8_MEMBER(fcscsi1_state::dma_error)
{
	if(data != 0)
	{
		logerror("DMAC error, vector = %x\n", m_dmac->get_error_vector(offset));
		dmac_irq_state = 1;
		dmac_irq_vector = m_dmac->get_vector(offset);
	}
	else
	{
		dmac_irq_state = 0;
	}

	update_irq_to_maincpu();
}

WRITE8_MEMBER(fcscsi1_state::fdc_irq)
{
	if (data != 0)
	{
		fdc_irq_state = 1;
	}
	else
	{
		fdc_irq_state = 0;
	}
	update_irq_to_maincpu();
}

READ8_MEMBER(fcscsi1_state::fdc_read_byte)
{
	return m_fdc->data_r();
}

WRITE8_MEMBER(fcscsi1_state::fdc_write_byte)
{
	m_fdc->data_w(data & 0xff);
}

READ8_MEMBER (fcscsi1_state::not_implemented_r){
	static int been_here = 0;
	if (!been_here++){
		logerror(TODO);
		printf(TODO);
	}
	return (UINT8) 0;
}

WRITE8_MEMBER (fcscsi1_state::not_implemented_w){
	static int been_here = 0;
	if (!been_here++){
		logerror(TODO);
		printf(TODO);
	}
	return;
}

/*
----------------------------------------------------
 IRQ  IRQ
Level Source       B4l inserted     B4l removed (Def)
-----------------------------------------------------
 1     P3 Pin #13   AV1 Autovector   AV1 Autovector
 2     DMAC         DMAC             AV2 Autovector
 3     SCSIBC       AV3 Autovector   AV3 Autovector
 4     FDC          AV4 Autovector   AV4 Autovector
 5     PI/T Timer   PI/T Timer Vect  PI/T Timer Vect
 6     --           --               --
 7     PI/T Port    PI/T Port Vect   PI/T Port Vect
------------------------------------------------------
Default configuration: B41 jumper removed

The PI/T port interrupt can be used under software control to
cause non-maskable (Level 7) interrupts if the watchdog timer
elapses and/or if the VMEbus interrupt trigger call occurs.
*/

/* TODO: Add configurable B41 jumper */
#define B41 0

void fcscsi1_state::update_irq_to_maincpu() {
	if (fdc_irq_state) {
		m_maincpu->set_input_line(M68K_IRQ_3, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
	} else if (dmac_irq_state) {
		m_maincpu->set_input_line(M68K_IRQ_3, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
#if B41 == 1
		m_maincpu->set_input_line_and_vector(M68K_IRQ_2, ASSERT_LINE, dmac_irq_vector);
#else
		m_maincpu->set_input_line(M68K_IRQ_2, ASSERT_LINE);
#endif
	} else {
		m_maincpu->set_input_line(M68K_IRQ_3, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
	}
}

IRQ_CALLBACK_MEMBER(fcscsi1_state::maincpu_irq_acknowledge_callback)
{
	// We immediately update the interrupt presented to the CPU, so that it doesn't
	// end up retrying the same interrupt over and over. We then return the appropriate vector.
	int vector = 0;
	switch(irqline) {
	case 2:
		dmac_irq_state = 0;
		vector = dmac_irq_vector;
		break;
	default:
		logerror("\nUnexpected IRQ ACK Callback: IRQ %d\n", irqline);
		return 0;
	}
	update_irq_to_maincpu();
	return vector;
}

/*
 * Machine configuration
 */
static MACHINE_CONFIG_START (fcscsi1, fcscsi1_state)
	/* basic machine hardware */
	MCFG_CPU_ADD ("maincpu", M68010, CPU_CRYSTAL / 2) /* 7474 based frequency divide by 2 */
	MCFG_CPU_PROGRAM_MAP (fcscsi1_mem)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(fcscsi1_state, maincpu_irq_acknowledge_callback)

	/* FDC  */
	MCFG_WD1772_ADD("fdc", CPU_CRYSTAL / 2) /* Same clock divider as for the CPU */
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITE8(fcscsi1_state, fdc_irq))

	/* PIT Parallel Interface and Timer device */
	MCFG_DEVICE_ADD ("pit", PIT68230, PIT_CRYSTAL / 2) /* 7474 based frequency divide by 2 */

	/* DMAC it is really a M68450 but the HD63850 is upwards compatible */
	MCFG_DEVICE_ADD("mc68450", HD63450, 0)   // MC68450 compatible
	MCFG_HD63450_CPU("maincpu") // CPU - 68010
	MCFG_HD63450_CLOCKS(attotime::from_usec(32), attotime::from_nsec(450), attotime::from_usec(4), attotime::from_hz(15625/2))
	MCFG_HD63450_BURST_CLOCKS(attotime::from_usec(32), attotime::from_nsec(450), attotime::from_nsec(50), attotime::from_nsec(50))
	MCFG_HD63450_DMA_END_CB(WRITE8(fcscsi1_state, dma_end))
	MCFG_HD63450_DMA_ERROR_CB(WRITE8(fcscsi1_state, dma_error))
	//MCFG_HD63450_DMA_READ_0_CB(READ8(fcscsi1_state, scsi_read_byte))  // ch 0 = SCSI
	//MCFG_HD63450_DMA_WRITE_0_CB(WRITE8(fcscsi1_state, scsi_write_byte))
	MCFG_HD63450_DMA_READ_1_CB(READ8(fcscsi1_state, fdc_read_byte))  // ch 1 = fdc
	MCFG_HD63450_DMA_WRITE_1_CB(WRITE8(fcscsi1_state, fdc_write_byte))
MACHINE_CONFIG_END

/* ROM definitions */
ROM_START (fcscsi1)
	ROM_REGION (0x1000000, "maincpu", 0)

	/* Besta ROM:s - apparantly patched Force ROM:s */
	ROM_SYSTEM_BIOS(0, "Besta 88", "Besta 88")
	ROMX_LOAD ("besta88_scsi_lower.ROM", 0xe00001, 0x4000, CRC (fb3ab364) SHA1 (d79112100f1c4beaf358e006efd4dde5e300b0ba), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD ("besta88_scsi_upper.ROM", 0xe00000, 0x4000, CRC (41f9cdf4) SHA1 (66b998bbf9459f0a613718260e05e97749532073), ROM_SKIP(1) | ROM_BIOS(1))

	/* Force ROM:s  */
	ROM_SYSTEM_BIOS(1, "ISCSI-1 v3.7", "Force Computer SYS68K/ISCSI-1 firmware v3.7")
	ROMX_LOAD ("ISCSI-1_V3.7_L.BIN", 0xe00001, 0x4000, CRC (83d95ab7) SHA1 (bf249910bcb6cb0b04dda2a95a38a0f90b553352), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD ("ISCSI-1_V3.7_U.BIN", 0xe00000, 0x4000, CRC (58815831) SHA1 (074085ef96e1fe2a551938bdeee6a9cab40ff09c), ROM_SKIP(1) | ROM_BIOS(2))

ROM_END

/* Driver */
/*    YEAR  NAME          PARENT  COMPAT   MACHINE         INPUT     CLASS          INIT COMPANY                  FULLNAME          FLAGS */
COMP( 1986, fcscsi1,       0,      0,      fcscsi1,      fcscsi1, driver_device,     0,  "Force Computers Gmbh",  "SYS68K/SCSI-1",   MACHINE_IS_SKELETON )
