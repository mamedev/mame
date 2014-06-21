// license:MAME
// copyright-holders:
/*
    Gimix 6809-Based Computers

    Representative group of GIMIX users included:  Government Research and Scientific Organizations, Universities, Industrial, Computer Mainframe and
    Peripheral Manufacturers and Software Houses.

    This system is most notable for being the development base of the "Vid Kidz", a pair of programmers (Eugene Jarvis and Larry DeMar) who formerly
    worked for Williams Electronics on the game, Defender.  They left Willams and continued work on other games eventually making a deal with Williams
    to continue to design games producing the titles: Stargate, Robotron: 2084 and Blaster.

    Information Link:  http://www.backglass.org/duncan/gimix/

    TODO:  Everything
*/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/mm58167.h"
#include "machine/6840ptm.h"
#include "machine/wd_fdc.h"
#include "machine/terminal.h"
#include "machine/bankdev.h"

#define DMA_DRQ         (m_dma_status & 0x80)
#define DMA_INTRQ       (m_dma_status & 0x40)
#define DMA_MOTOR_DELAY (m_dma_status & 0x20)
#define DMA_ENABLED     (m_dma_status & 0x10)
#define DMA_FAULT       (m_dma_status & 0x08)
#define DMA_DRIVE_SIZE  (m_dma_status & 0x04)
#define DMA_DIP_SENSE   (m_dma_status & 0x01)

#define DMA_CONNECT_SEL (m_dma_drive_select & 0x40)
#define DMA_DENSITY     (m_dma_drive_select & 0x20)
#define DMA_WR_PROTECT  (m_dma_drive_select & 0x10)
#define DMA_SEL_DRV3    (m_dma_drive_select & 0x08)
#define DMA_SEL_DRV2    (m_dma_drive_select & 0x04)
#define DMA_SEL_DRV1    (m_dma_drive_select & 0x02)
#define DMA_SEL_DRV0    (m_dma_drive_select & 0x01)

#define DMA_IRQ_ENABLE  (m_dma_ctrl & 0x80)
#define DMA_SIDE_SEL    (m_dma_ctrl & 0x40)
#define DMA_DIRECTION   (m_dma_ctrl & 0x20)
#define DMA_ENABLE      (m_dma_ctrl & 0x10)
#define DMA_BANK        (m_dma_ctrl & 0x0f)

#define DMA_START_ADDR  ((m_dma_ctrl & 0x0f) << 16) | m_dma_start_addr)

class gimix_state : public driver_device
{
public:
	gimix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
		, m_rom(*this, "roms")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
		, m_bank4(*this, "bank4")
		, m_bank5(*this, "bank5")
		, m_bank6(*this, "bank6")
		, m_bank7(*this, "bank7")
		, m_bank8(*this, "bank8")
		, m_bank9(*this, "bank9")
		, m_bank10(*this, "bank10")
		, m_bank11(*this, "bank11")
		, m_bank12(*this, "bank12")
		, m_bank13(*this, "bank13")
		, m_bank14(*this, "bank14")
		, m_bank15(*this, "bank15")
		, m_bank16(*this, "bank16")
		, m_rombank1(*this, "rombank1")
		, m_rombank2(*this, "rombank2")
		, m_rombank3(*this, "rombank3")
		, m_fixedrombank(*this, "fixedrombank")
	{}

	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(keyin_r);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_WRITE8_MEMBER(system_w);
	DECLARE_WRITE_LINE_MEMBER(irq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_irq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_READ8_MEMBER(dma_r);
	DECLARE_WRITE8_MEMBER(dma_w);

private:
	UINT8 m_term_data;
	UINT8 m_dma_status;
	UINT8 m_dma_ctrl;
	UINT8 m_dma_drive_select;
	UINT16 m_dma_start_addr;
	UINT8 m_task;
	UINT8 m_task_banks[16][16];

	virtual void machine_reset();
	virtual void machine_start();

	void refresh_memory();

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_memory_region m_rom;

	required_device<address_map_bank_device> m_bank1;
	required_device<address_map_bank_device> m_bank2;
	required_device<address_map_bank_device> m_bank3;
	required_device<address_map_bank_device> m_bank4;
	required_device<address_map_bank_device> m_bank5;
	required_device<address_map_bank_device> m_bank6;
	required_device<address_map_bank_device> m_bank7;
	required_device<address_map_bank_device> m_bank8;
	required_device<address_map_bank_device> m_bank9;
	required_device<address_map_bank_device> m_bank10;
	required_device<address_map_bank_device> m_bank11;
	required_device<address_map_bank_device> m_bank12;
	required_device<address_map_bank_device> m_bank13;
	required_device<address_map_bank_device> m_bank14;
	required_device<address_map_bank_device> m_bank15;
	required_device<address_map_bank_device> m_bank16;
	required_memory_bank m_rombank1;
	required_memory_bank m_rombank2;
	required_memory_bank m_rombank3;
	required_memory_bank m_fixedrombank;

};

static ADDRESS_MAP_START( gimix_banked_mem, AS_PROGRAM, 8, gimix_state)
	AM_RANGE(0x00000, 0x0dfff) AM_RAM
	AM_RANGE(0x0e004, 0x0e004) AM_READ(status_r)
	AM_RANGE(0x0e005, 0x0e005) AM_READ(keyin_r) AM_DEVWRITE("terminal", generic_terminal_device, write)
	AM_RANGE(0x0e100, 0x0e1ff) AM_RAM
	//AM_RANGE(0x0e200, 0x0e20f) // 9511A / 9512 Arithmetic Processor
	AM_RANGE(0x0e210, 0x0e21f) AM_DEVREADWRITE("timer",ptm6840_device,read,write)
	AM_RANGE(0x0e220, 0x0e23f) AM_DEVREADWRITE("rtc",mm58167_device,read,write)
	AM_RANGE(0x0e240, 0x0e3af) AM_RAM
	AM_RANGE(0x0e3b0, 0x0e3b3) AM_READWRITE(dma_r, dma_w)// DMA controller (custom?)
	AM_RANGE(0x0e3b4, 0x0e3b7) AM_DEVREADWRITE("fdc", fd1797_t,read,write)  // FD1797 FDC
	AM_RANGE(0x0e400, 0x0e7ff) AM_RAM
	AM_RANGE(0x0e800, 0x0efff) AM_ROMBANK("rombank3")
	AM_RANGE(0x0f000, 0x0f7ff) AM_ROMBANK("rombank2")
	AM_RANGE(0x0f800, 0x0ffff) AM_ROMBANK("rombank1")
	AM_RANGE(0x10000, 0x1ffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( gimix_mem, AS_PROGRAM, 8, gimix_state )
	AM_RANGE(0x0000, 0x0fff) AM_DEVREADWRITE("bank1", address_map_bank_device, read8, write8)
	AM_RANGE(0x1000, 0x1fff) AM_DEVREADWRITE("bank2", address_map_bank_device, read8, write8)
	AM_RANGE(0x2000, 0x2fff) AM_DEVREADWRITE("bank3", address_map_bank_device, read8, write8)
	AM_RANGE(0x3000, 0x3fff) AM_DEVREADWRITE("bank4", address_map_bank_device, read8, write8)
	AM_RANGE(0x4000, 0x4fff) AM_DEVREADWRITE("bank5", address_map_bank_device, read8, write8)
	AM_RANGE(0x5000, 0x5fff) AM_DEVREADWRITE("bank6", address_map_bank_device, read8, write8)
	AM_RANGE(0x6000, 0x6fff) AM_DEVREADWRITE("bank7", address_map_bank_device, read8, write8)
	AM_RANGE(0x7000, 0x7fff) AM_DEVREADWRITE("bank8", address_map_bank_device, read8, write8)
	AM_RANGE(0x8000, 0x8fff) AM_DEVREADWRITE("bank9", address_map_bank_device, read8, write8)
	AM_RANGE(0x9000, 0x9fff) AM_DEVREADWRITE("bank10", address_map_bank_device, read8, write8)
	AM_RANGE(0xa000, 0xafff) AM_DEVREADWRITE("bank11", address_map_bank_device, read8, write8)
	AM_RANGE(0xb000, 0xbfff) AM_DEVREADWRITE("bank12", address_map_bank_device, read8, write8)
	AM_RANGE(0xc000, 0xcfff) AM_DEVREADWRITE("bank13", address_map_bank_device, read8, write8)
	AM_RANGE(0xd000, 0xdfff) AM_DEVREADWRITE("bank14", address_map_bank_device, read8, write8)
	AM_RANGE(0xe000, 0xefff) AM_DEVREADWRITE("bank15", address_map_bank_device, read8, write8)
	AM_RANGE(0xf000, 0xfeff) AM_DEVREADWRITE("bank16", address_map_bank_device, read8, write8)
	AM_RANGE(0xff00, 0xffff) AM_ROMBANK("fixedrombank") AM_WRITE(system_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( gimix_io, AS_IO, 8, gimix_state )
ADDRESS_MAP_END

static INPUT_PORTS_START( gimix )
INPUT_PORTS_END

READ8_MEMBER( gimix_state::keyin_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( gimix_state::status_r )
{
	return (m_term_data) ? 3 : 2;
}

WRITE8_MEMBER( gimix_state::kbd_put )
{
	m_term_data = data;
}

void gimix_state::refresh_memory()
{
	int x;
	address_map_bank_device* banknum[16] = { m_bank1, m_bank2, m_bank3, m_bank4, m_bank5, m_bank6, m_bank7, m_bank8,
			m_bank9, m_bank10, m_bank11, m_bank12, m_bank13, m_bank14, m_bank15, m_bank16};

	for(x=0;x<16;x++) // for each bank
	{
		banknum[x]->set_bank(m_task_banks[m_task][x]);
	}
}

WRITE8_MEMBER( gimix_state::system_w )
{
	if(offset == 0x7f)  // task register
	{
		if(data & 0x20)  // FPLA software latch
		{
			m_rombank1->set_entry(2);
			m_rombank2->set_entry(3);
			m_rombank3->set_entry(1);
			m_fixedrombank->set_entry(2);
			logerror("SYS: FPLA software latch set\n");
		}
		else
		{
			m_rombank1->set_entry(0);
			m_rombank2->set_entry(1);
			m_rombank3->set_entry(2);
			m_fixedrombank->set_entry(0);
			logerror("SYS: FPLA software latch reset\n");
		}
		m_task = data & 0x0f;
		refresh_memory();
		logerror("SYS: Task set to %02x\n",data & 0x0f);
	}
	if(offset >= 0xf0)  // Dynamic Address Translation RAM (write only)
	{
		address_map_bank_device* banknum[16] = { m_bank1, m_bank2, m_bank3, m_bank4, m_bank5, m_bank6, m_bank7, m_bank8,
				m_bank9, m_bank10, m_bank11, m_bank12, m_bank13, m_bank14, m_bank15, m_bank16};

		banknum[offset-0xf0]->set_bank(data & 0x0f);
		m_task_banks[m_task][offset-0xf0] = data & 0x0f;
		logerror("SYS: Bank %i set to physical bank %02x\n",offset-0xf0,data);
	}
}

READ8_MEMBER(gimix_state::dma_r)
{
	switch(offset)
	{
	case 0:
		return m_dma_status;
	case 2:
		return (m_dma_start_addr & 0xff00) >> 8;
	case 3:
		return (m_dma_start_addr & 0x00ff);
	default:
		logerror("DMA: Unknown or invalid DMA register %02x read\n",offset);
	}
	return 0xff;
}

WRITE8_MEMBER(gimix_state::dma_w)
{
	switch(offset)
	{
	case 0:
		logerror("DMA: Drive select %02x\n",data);
		m_dma_drive_select = data;
		break;
	case 1:
		logerror("DMA: DMA control %02x\n",data);
		m_dma_ctrl = data;
		break;
	case 2:
		logerror("DMA: DMA start address MSB %02x\n",data);
		m_dma_start_addr = (m_dma_start_addr & 0x00ff) | (data << 8);
		break;
	case 3:
		logerror("DMA: DMA start address LSB %02x\n",data);
		m_dma_start_addr = (m_dma_start_addr & 0xff00) | data;
		break;
	default:
		logerror("DMA: Unknown or invalid DMA register %02x write %02x\n",offset,data);
	}
}

WRITE_LINE_MEMBER(gimix_state::irq_w)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE,state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(gimix_state::fdc_irq_w)
{
	if(state)
		m_dma_status |= 0x40;
	else
		m_dma_status &= ~0x40;
}

WRITE_LINE_MEMBER(gimix_state::fdc_drq_w)
{
	if(state)
		m_dma_status |= 0x80;
	else
		m_dma_status &= ~0x80;
	// TODO: actually do DMA transfers
}

void gimix_state::machine_reset()
{
	m_term_data = 0;
	m_rombank1->set_entry(0);  // RAM banks are undefined on startup
	m_rombank2->set_entry(1);
	m_rombank3->set_entry(2);
	m_fixedrombank->set_entry(0);
	m_dma_status = 0x00;
	m_dma_ctrl = 0x00;
	m_task = 0x00;
}

void gimix_state::machine_start()
{
	UINT8* ROM = m_rom->base();
	m_rombank1->configure_entries(0,4,ROM,0x800);
	m_rombank2->configure_entries(0,4,ROM,0x800);
	m_rombank3->configure_entries(0,4,ROM,0x800);
	m_fixedrombank->configure_entries(0,4,ROM+0x700,0x800);
	m_rombank1->set_entry(0);  // RAM banks are undefined on startup
	m_rombank2->set_entry(1);
	m_rombank3->set_entry(2);
	m_fixedrombank->set_entry(0);
}

static SLOT_INTERFACE_START( gimix_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

#define MCFG_ADDRESS_BANK(tag) \
MCFG_DEVICE_ADD(tag, ADDRESS_MAP_BANK, 0) \
MCFG_DEVICE_PROGRAM_MAP(gimix_banked_mem) \
MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE) \
MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8) \
MCFG_ADDRESS_MAP_BANK_STRIDE(0x1000)

static MACHINE_CONFIG_START( gimix, gimix_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", M6809E, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(gimix_mem)
	MCFG_CPU_IO_MAP(gimix_io)

	/* video hardware */
	MCFG_DEVICE_ADD("terminal", GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(gimix_state, kbd_put))

	/* rtc */
	MCFG_DEVICE_ADD("rtc", MM58167, XTAL_32_768kHz)
	MCFG_MM58167_IRQ_CALLBACK(WRITELINE(gimix_state,irq_w))

	/* timer */
	MCFG_DEVICE_ADD("timer", PTM6840, XTAL_2MHz)  // clock is a guess
	MCFG_PTM6840_IRQ_CB(WRITELINE(gimix_state,irq_w))  // PCB pictures show both the RTC and timer set to generate IRQs (are jumper configurable)

	/* floppy disks */
	MCFG_FD1797x_ADD("fdc",XTAL_2MHz)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(gimix_state,fdc_irq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(gimix_state,fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", gimix_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", gimix_floppies, "525dd", floppy_image_device::default_floppy_formats)

	/* banking */
	MCFG_ADDRESS_BANK("bank1")
	MCFG_ADDRESS_BANK("bank2")
	MCFG_ADDRESS_BANK("bank3")
	MCFG_ADDRESS_BANK("bank4")
	MCFG_ADDRESS_BANK("bank5")
	MCFG_ADDRESS_BANK("bank6")
	MCFG_ADDRESS_BANK("bank7")
	MCFG_ADDRESS_BANK("bank8")
	MCFG_ADDRESS_BANK("bank9")
	MCFG_ADDRESS_BANK("bank10")
	MCFG_ADDRESS_BANK("bank11")
	MCFG_ADDRESS_BANK("bank12")
	MCFG_ADDRESS_BANK("bank13")
	MCFG_ADDRESS_BANK("bank14")
	MCFG_ADDRESS_BANK("bank15")
	MCFG_ADDRESS_BANK("bank16")

MACHINE_CONFIG_END

ROM_START( gimix )
	ROM_REGION( 0x10000, "roms", 0)

	/* CPU board U4: gimixf8.bin  - checksum 68DB - 2716 - GMXBUG09 V2.1 | (c)1981 GIMIX | $F800 I2716 */
		ROM_LOAD( "gimixf8.u4",  0x000000, 0x000800, CRC(7d60f838) SHA1(eb7546e8bbf50d33e181f3e86c3e4c5c9032cab2) )
/* CPU board U5: gimixv14.bin - checksum 97E2 - 2716 - GIMIX 6809 | AUTOBOOT | V1.4 I2716 */
        ROM_LOAD( "gimixv14.u5", 0x000800, 0x000800, CRC(f795b8b9) SHA1(eda2de51cc298d94b36605437d900ce971b3b276) )
/* CPU board U6: os9l1v11.bin - checksum 2C84 - 2716 - OS-9tmL1 V1 | GIMIX P1 " (c)1982 MSC
   CPU board U7: os9l1v12.bin - checksum 7694 - 2716 - OS-9tmL1 V1 | GIMIX P2-68 | (c)1982 MSC */
        ROM_LOAD( "os9l1v11.u6", 0x001000, 0x000800, CRC(0d6527a0) SHA1(1435a22581c6e9e0ae338071a72eed646f429530) )
        ROM_LOAD( "os9l1v12.u7", 0x001800, 0x000800, CRC(b3c65feb) SHA1(19d1ea1e84473b25c95cbb8449e6b9828567e998) )

/* Hard drive controller board 2 (XEBEC board) 11H: gimixhd.bin - checksum 2436 - 2732 - 104521D */
   	ROM_REGION( 0x10000, "xebec", 0)
        ROM_LOAD( "gimixhd.h11",  0x000000, 0x001000, CRC(35c12201) SHA1(51ac9052f9757d79c7f5bd3aa5d8421e98cfcc37) )
ROM_END

COMP( 1980, gimix,    0,      0,      gimix,        gimix, driver_device, 0,      "Gimix",  "Gimix 6809 System",  GAME_IS_SKELETON | GAME_NOT_WORKING | GAME_NO_SOUND )
