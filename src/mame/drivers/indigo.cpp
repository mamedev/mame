// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************\
*
*   SGI Indigo workstation
*
*  Skeleton Driver
*
*  Todo: Everything
*
*  Note: Machine uses R4400, not R4600
*
*  Memory map:
*
*  1fa00000 - 1fa02047      Memory Controller
*  1fb80000 - 1fb9a7ff      HPC1 CHIP0
*  1fc00000 - 1fc7ffff      BIOS
*
\*********************************************************************/

#include "emu.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsicd.h"
#include "cpu/mips/mips3.h"
#include "cpu/mips/mips1.h"
#include "machine/8530scc.h"
#include "machine/eepromser.h"
#include "machine/sgi.h"
#include "machine/wd33c93.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class indigo_state : public driver_device
{
public:
	indigo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_wd33c93(*this, "wd33c93")
		, m_scc(*this, "scc")
		, m_eeprom(*this, "eeprom")
	{
	}

	void indigo4k(machine_config &config);
	void indigo3k(machine_config &config);

private:
	enum
	{
		TIMER_RTC
	};

	DECLARE_READ32_MEMBER(hpc_r);
	DECLARE_WRITE32_MEMBER(hpc_w);
	DECLARE_READ32_MEMBER(int_r);
	DECLARE_WRITE32_MEMBER(int_w);
	DECLARE_WRITE_LINE_MEMBER(scsi_irq);

	virtual void machine_start() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	static void cdrom_config(device_t *device);
	void indigo3k_map(address_map &map);
	void indigo4k_map(address_map &map);
	void indigo_map(address_map &map);

	struct hpc_t
	{
		uint8_t m_misc_status;
		uint32_t m_parbuf_ptr;
		uint32_t m_local_ioreg0_mask;
		uint32_t m_local_ioreg1_mask;
		uint32_t m_vme_intmask0;
		uint32_t m_vme_intmask1;
		uint32_t m_scsi0_descriptor;
		uint32_t m_scsi0_dma_ctrl;
	};

	struct rtc_t
	{
		uint8_t nRAM[32];
	};

	required_device<cpu_device> m_maincpu;
	required_device<wd33c93_device> m_wd33c93;
	required_device<scc8530_t> m_scc;
	required_device<eeprom_serial_93cxx_device> m_eeprom;

	hpc_t m_hpc;
	rtc_t m_rtc;

	void indigo_timer_rtc();

	inline void ATTR_PRINTF(3,4) verboselog(int n_level, const char *s_fmt, ... );

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};


#define VERBOSE_LEVEL (0)

inline void ATTR_PRINTF(3,4) indigo_state::verboselog(int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%08x: %s", m_maincpu->pc(), buf );
	}
}

void indigo_state::video_start()
{
}

uint32_t indigo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}



#define RTC_DAYOFWEEK   m_rtc.nRAM[0x0e]
#define RTC_YEAR        m_rtc.nRAM[0x0b]
#define RTC_MONTH       m_rtc.nRAM[0x0a]
#define RTC_DAY         m_rtc.nRAM[0x09]
#define RTC_HOUR        m_rtc.nRAM[0x08]
#define RTC_MINUTE      m_rtc.nRAM[0x07]
#define RTC_SECOND      m_rtc.nRAM[0x06]
#define RTC_HUNDREDTH   m_rtc.nRAM[0x05]

READ32_MEMBER(indigo_state::hpc_r)
{
	offset <<= 2;
	if( offset >= 0x0e00 && offset <= 0x0e7c )
	{
		verboselog(2, "RTC RAM[0x%02x] Read: %02x\n", ( offset - 0xe00 ) >> 2, m_rtc.nRAM[ ( offset - 0xe00 ) >> 2 ] );
		return m_rtc.nRAM[ ( offset - 0xe00 ) >> 2 ];
	}
	switch( offset )
	{
	case 0x05c:
		verboselog(2, "HPC Unknown Read: %08x (%08x) (returning 0x000000a5 as kludge)\n", 0x1fb80000 + offset, mem_mask );
		return 0x0000a500;
	case 0x00ac:
		verboselog(2, "HPC Parallel Buffer Pointer Read: %08x (%08x)\n", m_hpc.m_parbuf_ptr, mem_mask );
		return m_hpc.m_parbuf_ptr;
	case 0x00c0:
		verboselog(2, "HPC Endianness Read: %08x (%08x)\n", 0x0000001f, mem_mask );
		return 0x0000001f;
	case 0x0120:
		if (ACCESSING_BITS_8_15)
		{
			return ( m_wd33c93->read( space, 0 ) << 8 );
		}
		else
		{
			return 0;
		}
	case 0x0124:
		if (ACCESSING_BITS_8_15)
		{
			return ( m_wd33c93->read( space, 1 ) << 8 );
		}
		else
		{
			return 0;
		}
	case 0x01b0:
		verboselog(2, "HPC Misc. Status Read: %08x (%08x)\n", m_hpc.m_misc_status, mem_mask );
		return m_hpc.m_misc_status;
	case 0x01bc:
//      verboselog(machine, 2, "HPC CPU Serial EEPROM Read\n" );
		return m_eeprom->do_read() << 4;
	case 0x01c4:
		verboselog(2, "HPC Local IO Register 0 Mask Read: %08x (%08x)\n", m_hpc.m_local_ioreg0_mask, mem_mask );
		return m_hpc.m_local_ioreg0_mask;
	case 0x01cc:
		verboselog(2, "HPC Local IO Register 1 Mask Read: %08x (%08x)\n", m_hpc.m_local_ioreg1_mask, mem_mask );
		return m_hpc.m_local_ioreg1_mask;
	case 0x01d4:
		verboselog(2, "HPC VME Interrupt Mask 0 Read: %08x (%08x)\n", m_hpc.m_vme_intmask0, mem_mask );
		return m_hpc.m_vme_intmask0;
	case 0x01d8:
		verboselog(2, "HPC VME Interrupt Mask 1 Read: %08x (%08x)\n", m_hpc.m_vme_intmask1, mem_mask );
		return m_hpc.m_vme_intmask1;
	case 0x0d00:
		verboselog(2, "HPC DUART0 Channel B Control Read\n" );
//      return 0x00000004;
		return 0x7c; //m_scc->reg_r(space, 0);
	case 0x0d04:
		verboselog(2, "HPC DUART0 Channel B Data Read\n" );
//      return 0;
		return m_scc->reg_r(space, 2);
	case 0x0d08:
		verboselog(2, "HPC DUART0 Channel A Control Read (%08x)\n", mem_mask  );
//      return 0x40;
		return 0x7c; //m_scc->reg_r(space, 1);
	case 0x0d0c:
		verboselog(2, "HPC DUART0 Channel A Data Read\n" );
//      return 0;
		return m_scc->reg_r(space, 3);
	case 0x0d10:
//      verboselog(machine, 2, "HPC DUART1 Channel B Control Read\n" );
		return 0x00000004;
	case 0x0d14:
		verboselog(2, "HPC DUART1 Channel B Data Read\n" );
		return 0;
	case 0x0d18:
		verboselog(2, "HPC DUART1 Channel A Control Read\n" );
		return 0;
	case 0x0d1c:
		verboselog(2, "HPC DUART1 Channel A Data Read\n" );
		return 0;
	case 0x0d20:
		verboselog(2, "HPC DUART2 Channel B Control Read\n" );
		return 0x00000004;
	case 0x0d24:
		verboselog(2, "HPC DUART2 Channel B Data Read\n" );
		return 0;
	case 0x0d28:
		verboselog(2, "HPC DUART2 Channel A Control Read\n" );
		return 0;
	case 0x0d2c:
		verboselog(2, "HPC DUART2 Channel A Data Read\n" );
		return 0;
	case 0x0d30:
		verboselog(2, "HPC DUART3 Channel B Control Read\n" );
		return 0x00000004;
	case 0x0d34:
		verboselog(2, "HPC DUART3 Channel B Data Read\n" );
		return 0;
	case 0x0d38:
		verboselog(2, "HPC DUART3 Channel A Control Read\n" );
		return 0;
	case 0x0d3c:
		verboselog(2, "HPC DUART3 Channel A Data Read\n" );
		return 0;
	}
	verboselog(0, "Unmapped HPC read: 0x%08x (%08x)\n", 0x1fb80000 + offset, mem_mask );
	return 0;
}

WRITE32_MEMBER(indigo_state::hpc_w)
{
	offset <<= 2;
	if( offset >= 0x0e00 && offset <= 0x0e7c )
	{
		verboselog(2, "RTC RAM[0x%02x] Write: %02x\n", ( offset - 0xe00 ) >> 2, data & 0x000000ff );
		m_rtc.nRAM[ ( offset - 0xe00 ) >> 2 ] = data & 0x000000ff;
		switch( ( offset - 0xe00 ) >> 2 )
		{
		case 0:
			break;
		case 4:
			if( !( m_rtc.nRAM[0x00] & 0x80 ) )
			{
				if( data & 0x80 )
				{
					m_rtc.nRAM[0x19] = m_rtc.nRAM[0x06]; //RTC_SECOND;
					m_rtc.nRAM[0x1a] = m_rtc.nRAM[0x07]; //RTC_MINUTE;
					m_rtc.nRAM[0x1b] = m_rtc.nRAM[0x08]; //RTC_HOUR;
					m_rtc.nRAM[0x1c] = m_rtc.nRAM[0x09]; //RTC_DAY;
					m_rtc.nRAM[0x1d] = m_rtc.nRAM[0x0a]; //RTC_MONTH;
				}
			}
			break;
		}
		return;
	}
	switch( offset )
	{
	case 0x0090:    // SCSI0 next descriptor pointer
		m_hpc.m_scsi0_descriptor = data;
		break;

	case 0x0094:    // SCSI0 control flags
		m_hpc.m_scsi0_dma_ctrl = data;
		#if 0
		if (data & 0x80)
		{
			uint32_t next;

			osd_printf_info("DMA activated for SCSI0\n");
			osd_printf_info("Descriptor block:\n");
			osd_printf_info("CTL: %08x BUFPTR: %08x DESCPTR %08x\n",
				program_read_dword(m_hpc.m_scsi0_descriptor), program_read_dword(m_hpc.m_scsi0_descriptor+4),
				program_read_dword(m_hpc.m_scsi0_descriptor+8));

			next = program_read_dword(m_hpc.m_scsi0_descriptor+8);
			osd_printf_info("CTL: %08x BUFPTR: %08x DESCPTR %08x\n",
				program_read_dword(next), program_read_dword(next+4),
				program_read_dword(next+8));
		}
		#endif
		break;

	case 0x00ac:
		verboselog(2, "HPC Parallel Buffer Pointer Write: %08x (%08x)\n", data, mem_mask );
		m_hpc.m_parbuf_ptr = data;
		break;
	case 0x0120:
		if (ACCESSING_BITS_8_15)
		{
			verboselog(2, "HPC SCSI Controller Register Write: %08x\n", ( data >> 8 ) & 0x000000ff );
			m_wd33c93->write( space, 0, ( data >> 8 ) & 0x000000ff );
		}
		else
		{
			return;
		}
		break;
	case 0x0124:
		if (ACCESSING_BITS_8_15)
		{
			verboselog(2, "HPC SCSI Controller Data Write: %08x\n", ( data >> 8 ) & 0x000000ff );
			m_wd33c93->write( space, 1, ( data >> 8 ) & 0x000000ff );
		}
		else
		{
			return;
		}
		break;
	case 0x01b0:
		verboselog(2, "HPC Misc. Status Write: %08x (%08x)\n", data, mem_mask );
		if( data & 0x00000001 )
		{
			verboselog(2, "  Force DSP hard reset\n" );
		}
		if( data & 0x00000002 )
		{
			verboselog(2, "  Force IRQA\n" );
		}
		if( data & 0x00000004 )
		{
			verboselog(2, "  Set IRQA polarity high\n" );
		}
		else
		{
			verboselog(2, "  Set IRQA polarity low\n" );
		}
		if( data & 0x00000008 )
		{
			verboselog(2, "  SRAM size: 32K\n" );
		}
		else
		{
			verboselog(2, "  SRAM size:  8K\n" );
		}
		m_hpc.m_misc_status = data;
		break;
	case 0x01bc:
//      verboselog(machine, 2, "HPC CPU Serial EEPROM Write: %08x (%08x)\n", data, mem_mask );
		if( data & 0x00000001 )
		{
			verboselog(2, "    CPU board LED on\n" );
		}
		m_eeprom->di_write((data & 0x00000008) ? 1 : 0 );
		m_eeprom->cs_write((data & 0x00000002) ? CLEAR_LINE : ASSERT_LINE );
		m_eeprom->clk_write((data & 0x00000004) ? CLEAR_LINE : ASSERT_LINE );
		break;
	case 0x01c4:
		verboselog(2, "HPC Local IO Register 0 Mask Write: %08x (%08x)\n", data, mem_mask );
		m_hpc.m_local_ioreg0_mask = data;
		break;
	case 0x01cc:
		verboselog(2, "HPC Local IO Register 1 Mask Write: %08x (%08x)\n", data, mem_mask );
		m_hpc.m_local_ioreg1_mask = data;
		break;
	case 0x01d4:
		verboselog(2, "HPC VME Interrupt Mask 0 Write: %08x (%08x)\n", data, mem_mask );
		m_hpc.m_vme_intmask0 = data;
		break;
	case 0x01d8:
		verboselog(2, "HPC VME Interrupt Mask 1 Write: %08x (%08x)\n", data, mem_mask );
		m_hpc.m_vme_intmask1 = data;
		break;
	case 0x0d00:
		verboselog(2, "HPC DUART0 Channel B Control Write: %08x (%08x)\n", data, mem_mask );
		m_scc->reg_w(space, 0, data);
		break;
	case 0x0d04:
		verboselog(2, "HPC DUART0 Channel B Data Write: %08x (%08x)\n", data, mem_mask );
		m_scc->reg_w(space, 2, data);
		break;
	case 0x0d08:
		verboselog(2, "HPC DUART0 Channel A Control Write: %08x (%08x)\n", data, mem_mask );
		m_scc->reg_w(space, 1, data);
		break;
	case 0x0d0c:
		verboselog(2, "HPC DUART0 Channel A Data Write: %08x (%08x)\n", data, mem_mask );
		m_scc->reg_w(space, 3, data);
		break;
	case 0x0d10:
		if( ( data & 0x000000ff ) >= 0x00000020 )
		{
//          verboselog(2, "HPC DUART1 Channel B Control Write: %08x (%08x) %c\n", data, mem_mask, data & 0x000000ff );
			//osd_printf_info( "%c", data & 0x000000ff );
		}
		else
		{
//          verboselog(2, "HPC DUART1 Channel B Control Write: %08x (%08x)\n", data, mem_mask );
		}
		break;
	case 0x0d14:
		if( ( data & 0x000000ff ) >= 0x00000020 || ( data & 0x000000ff ) == 0x0d || ( data & 0x000000ff ) == 0x0a )
		{
			verboselog(2, "HPC DUART1 Channel B Data Write: %08x (%08x) %c\n", data, mem_mask, data & 0x000000ff );
			osd_printf_info( "%c", data & 0x000000ff );
		}
		else
		{
			verboselog(2, "HPC DUART1 Channel B Data Write: %08x (%08x)\n", data, mem_mask );
		}
		break;
	case 0x0d18:
		osd_printf_info("HPC DUART1 Channel A Control Write: %08x (%08x)\n", data, mem_mask );
		break;
	case 0x0d1c:
		verboselog(2, "HPC DUART1 Channel A Data Write: %08x (%08x)\n", data, mem_mask );
		break;
	case 0x0d20:
		osd_printf_info("HPC DUART2 Channel B Control Write: %08x (%08x)\n", data, mem_mask );
		break;
	case 0x0d24:
		verboselog(2, "HPC DUART2 Channel B Data Write: %08x (%08x)\n", data, mem_mask );
		break;
	case 0x0d28:
		osd_printf_info("HPC DUART2 Channel A Control Write: %08x (%08x)\n", data, mem_mask );
		break;
	case 0x0d2c:
		verboselog(2, "HPC DUART2 Channel A Data Write: %08x (%08x)\n", data, mem_mask );
		break;
	case 0x0d30:
		osd_printf_info("HPC DUART3 Channel B Control Write: %08x (%08x)\n", data, mem_mask );
		break;
	case 0x0d34:
		verboselog(2, "HPC DUART3 Channel B Data Write: %08x (%08x)\n", data, mem_mask );
		break;
	case 0x0d38:
		osd_printf_info("HPC DUART3 Channel A Control Write: %08x (%08x)\n", data, mem_mask );
		break;
	case 0x0d3c:
		verboselog(2, "HPC DUART3 Channel A Data Write: %08x (%08x)\n", data, mem_mask );
		break;
	default:
		osd_printf_info("Unmapped HPC write: 0x%08x (%08x): %08x\n", 0x1fb80000 + offset, mem_mask, data);
		break;
	}
}

// INT/INT2/INT3 interrupt controllers
READ32_MEMBER(indigo_state::int_r)
{
	osd_printf_info("INT: read @ ofs %x (mask %x) (PC=%x)\n", offset, mem_mask, m_maincpu->pc());
	return 0;
}

WRITE32_MEMBER(indigo_state::int_w)
{
	osd_printf_info("INT: write %x to ofs %x (mask %x) (PC=%x)\n", data, offset, mem_mask, m_maincpu->pc());
}

void indigo_state::indigo_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram().share("share10");
	map(0x08000000, 0x08ffffff).ram().share("share5");
	map(0x09000000, 0x097fffff).ram().share("share6");
	map(0x0a000000, 0x0a7fffff).ram().share("share7");
	map(0x0c000000, 0x0c7fffff).ram().share("share8");
	map(0x10000000, 0x107fffff).ram().share("share9");
	map(0x18000000, 0x187fffff).ram().share("share1");
	map(0x1fb80000, 0x1fb8ffff).rw(FUNC(indigo_state::hpc_r), FUNC(indigo_state::hpc_w));
	map(0x1fbd9000, 0x1fbd903f).rw(FUNC(indigo_state::int_r), FUNC(indigo_state::int_w));
}

void indigo_state::indigo3k_map(address_map &map)
{
	indigo_map(map);
	map(0x1fc00000, 0x1fc3ffff).rom().share("share2").region("user1", 0);
}

void indigo_state::indigo4k_map(address_map &map)
{
	indigo_map(map);
	map(0x1fa00000, 0x1fa1ffff).rw("sgi_mc", FUNC(sgi_mc_device::read), FUNC(sgi_mc_device::write));
	map(0x1fc00000, 0x1fc7ffff).rom().share("share2").region("user1", 0);
}

WRITE_LINE_MEMBER(indigo_state::scsi_irq)
{
}

void indigo_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_RTC:
		indigo_timer_rtc();
		break;
	default:
		assert_always(false, "Unknown id in indigo_state::device_timer");
	}
}

void indigo_state::indigo_timer_rtc()
{
	// update RTC every 10 milliseconds
	RTC_HUNDREDTH++;

	if( ( RTC_HUNDREDTH & 0x0f ) == 0x0a )
	{
		RTC_HUNDREDTH -= 0x0a;
		RTC_HUNDREDTH += 0x10;
		if( ( RTC_HUNDREDTH & 0xa0 ) == 0xa0 )
		{
			RTC_HUNDREDTH = 0;
			RTC_SECOND++;

			if( ( RTC_SECOND & 0x0f ) == 0x0a )
			{
				RTC_SECOND -= 0x0a;
				RTC_SECOND += 0x10;
				if( RTC_SECOND == 0x60 )
				{
					RTC_SECOND = 0;
					RTC_MINUTE++;

					if( ( RTC_MINUTE & 0x0f ) == 0x0a )
					{
						RTC_MINUTE -= 0x0a;
						RTC_MINUTE += 0x10;
						if( RTC_MINUTE == 0x60 )
						{
							RTC_MINUTE = 0;
							RTC_HOUR++;

							if( ( RTC_HOUR & 0x0f ) == 0x0a )
							{
								RTC_HOUR -= 0x0a;
								RTC_HOUR += 0x10;
								if( RTC_HOUR == 0x24 )
								{
									RTC_HOUR = 0;
									RTC_DAY++;
								}
							}
						}
					}
				}
			}
		}
	}

	timer_set(attotime::from_msec(10), TIMER_RTC);
}

void indigo_state::machine_start()
{
	m_hpc.m_misc_status = 0;
	m_hpc.m_parbuf_ptr = 0;
	m_hpc.m_local_ioreg0_mask = 0;
	m_hpc.m_local_ioreg1_mask = 0;
	m_hpc.m_vme_intmask0 = 0;
	m_hpc.m_vme_intmask1 = 0;

	timer_set(attotime::from_msec(10), TIMER_RTC);
}

static INPUT_PORTS_START( indigo )
	PORT_START("unused")
	PORT_BIT ( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void indigo_state::cdrom_config(device_t *device)
{
	device = device->subdevice("cdda");
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "^^mono", 1.0)
}

MACHINE_CONFIG_START(indigo_state::indigo3k)
	R3000A(config, m_maincpu, 33.333_MHz_XTAL, 32768, 32768);
	downcast<r3000a_device &>(*m_maincpu).set_endianness(ENDIANNESS_BIG);
	m_maincpu->set_addrmap(AS_PROGRAM, &indigo_state::indigo3k_map);

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( 60 )
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(800, 600)
	MCFG_SCREEN_VISIBLE_AREA(0, 799, 0, 599)
	MCFG_SCREEN_UPDATE_DRIVER(indigo_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 65536)

	SPEAKER(config, "mono").front_center();

	SCC8530(config, m_scc, 7000000);

	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "cdrom", SCSICD, SCSI_ID_6)
	MCFG_SLOT_OPTION_MACHINE_CONFIG("cdrom", cdrom_config)

	WD33C93(config, m_wd33c93);
	m_wd33c93->set_scsi_port("scsi");
	m_wd33c93->irq_cb().set(FUNC(indigo_state::scsi_irq));      /* command completion IRQ */

	EEPROM_93C56_16BIT(config, "eeprom");
MACHINE_CONFIG_END

MACHINE_CONFIG_START(indigo_state::indigo4k)
	indigo3k(config);
	MCFG_DEVICE_REPLACE("maincpu", R4600BE, 150000000) // Should be R4400
	MCFG_MIPS3_ICACHE_SIZE(32768)
	MCFG_MIPS3_DCACHE_SIZE(32768)
	MCFG_DEVICE_PROGRAM_MAP(indigo4k_map)

	MCFG_DEVICE_ADD("sgi_mc", SGI_MC, 0)
MACHINE_CONFIG_END

ROM_START( indigo3k )
	ROM_REGION32_BE( 0x40000, "user1", 0 )
	ROM_SYSTEM_BIOS( 0, "401-rev-c", "SGI Version 4.0.1 Rev C LG1/GR2, Jul 9, 1992" ) // dumped over serial connection from boot monitor and swapped
	ROMX_LOAD( "ip12prom.070-8088-xxx.u56", 0x000000, 0x040000, CRC(25ca912f) SHA1(94b3753d659bfe50b914445cef41290122f43880), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "401-rev-d", "SGI Version 4.0.1 Rev D LG1/GR2, Mar 24, 1992" ) // dumped with EPROM programmer
	ROMX_LOAD( "ip12prom.070-8088-002.u56", 0x000000, 0x040000, CRC(ea4329ef) SHA1(b7d67d0e30ae8836892f7170dd4757732a0a3fd6), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(1) )
ROM_END

ROM_START( indigo4k )
	ROM_REGION32_BE( 0x80000, "user1", 0 )
	ROMX_LOAD( "ip20prom.070-8116-004.bin", 0x000000, 0x080000, CRC(940d960e) SHA1(596aba530b53a147985ff3f6f853471ce48c866c), ROM_GROUPDWORD | ROM_REVERSE )
ROM_END

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT   CLASS         INIT        COMPANY                 FULLNAME                                          FLAGS
COMP( 1991, indigo3k, 0,      0,      indigo3k, indigo, indigo_state, empty_init, "Silicon Graphics Inc", "IRIS Indigo (R3000, 33MHz)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1993, indigo4k, 0,      0,      indigo4k, indigo, indigo_state, empty_init, "Silicon Graphics Inc", "IRIS Indigo (R4400, 150MHz, Ver. 4.0.5D Rev A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
