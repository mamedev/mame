/*********************************************************************\
*
*   SGI IP20 IRIS Indigo workstation
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
#include "cpu/mips/mips3.h"
#include "sound/cdda.h"
#include "machine/8530scc.h"
#include "machine/sgi.h"
#include "machine/eeprom.h"
#include "machine/scsibus.h"
#include "machine/scsicd.h"
#include "machine/wd33c93.h"

struct HPC_t 
{
	UINT8 nMiscStatus;
	UINT32 nParBufPtr;
	UINT32 nLocalIOReg0Mask;
	UINT32 nLocalIOReg1Mask;
	UINT32 nVMEIntMask0;
	UINT32 nVMEIntMask1;
	UINT32 nSCSI0Descriptor;
	UINT32 nSCSI0DMACtrl;
};

struct RTC_t 
{
	UINT8 nRAM[32];
	UINT8 nTemp;
};

class ip20_state : public driver_device
{
public:
	ip20_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_wd33c93(*this, "scsi:wd33c93"){ }

	required_device<wd33c93_device> m_wd33c93;

	HPC_t m_HPC;
	RTC_t m_RTC;
	DECLARE_READ32_MEMBER(hpc_r);
	DECLARE_WRITE32_MEMBER(hpc_w);
	DECLARE_READ32_MEMBER(int_r);
	DECLARE_WRITE32_MEMBER(int_w);
	DECLARE_DRIVER_INIT(ip204415);
	virtual void machine_start();
	virtual void video_start();
};


#define VERBOSE_LEVEL ( 2 )

INLINE void ATTR_PRINTF(3,4) verboselog(running_machine &machine, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%08x: %s", machine.device("maincpu")->safe_pc(), buf );
	}
}

void ip20_state::video_start()
{
}

static SCREEN_UPDATE_IND16( ip204415 )
{
	return 0;
}

static const eeprom_interface eeprom_interface_93C56 =
{
	7,					// address bits 7
	16,					// data bits    16
	"*110x",			// read         110x aaaaaaaa
	"*101x",			// write        101x aaaaaaaa dddddddd
	"*111x",			// erase        111x aaaaaaaa
	"*10000xxxxxxx",	// lock         100x 00xxxx
	"*10011xxxxxxx",	// unlock       100x 11xxxx
};




#define RTC_DAYOFWEEK	state->m_RTC.nRAM[0x0e]
#define RTC_YEAR		state->m_RTC.nRAM[0x0b]
#define RTC_MONTH		state->m_RTC.nRAM[0x0a]
#define RTC_DAY			state->m_RTC.nRAM[0x09]
#define RTC_HOUR		state->m_RTC.nRAM[0x08]
#define RTC_MINUTE		state->m_RTC.nRAM[0x07]
#define RTC_SECOND		state->m_RTC.nRAM[0x06]
#define RTC_HUNDREDTH	state->m_RTC.nRAM[0x05]

READ32_MEMBER(ip20_state::hpc_r)
{
	scc8530_t *scc;
	offset <<= 2;
	if( offset >= 0x0e00 && offset <= 0x0e7c )
	{
		verboselog(machine(), 2, "RTC RAM[0x%02x] Read: %02x\n", ( offset - 0xe00 ) >> 2, m_RTC.nRAM[ ( offset - 0xe00 ) >> 2 ] );
		return m_RTC.nRAM[ ( offset - 0xe00 ) >> 2 ];
	}
	switch( offset )
	{
	case 0x05c:
		verboselog(machine(), 2, "HPC Unknown Read: %08x (%08x) (returning 0x000000a5 as kludge)\n", 0x1fb80000 + offset, mem_mask );
		return 0x0000a500;
	case 0x00ac:
		verboselog(machine(), 2, "HPC Parallel Buffer Pointer Read: %08x (%08x)\n", m_HPC.nParBufPtr, mem_mask );
		return m_HPC.nParBufPtr;
	case 0x00c0:
		verboselog(machine(), 2, "HPC Endianness Read: %08x (%08x)\n", 0x0000001f, mem_mask );
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
		verboselog(machine(), 2, "HPC Misc. Status Read: %08x (%08x)\n", m_HPC.nMiscStatus, mem_mask );
		return m_HPC.nMiscStatus;
	case 0x01bc:
//      verboselog(machine, 2, "HPC CPU Serial EEPROM Read\n" );
		return ( (machine().device<eeprom_device>("eeprom")->read_bit() << 4 ) );
	case 0x01c4:
		verboselog(machine(), 2, "HPC Local IO Register 0 Mask Read: %08x (%08x)\n", m_HPC.nLocalIOReg0Mask, mem_mask );
		return m_HPC.nLocalIOReg0Mask;
	case 0x01cc:
		verboselog(machine(), 2, "HPC Local IO Register 1 Mask Read: %08x (%08x)\n", m_HPC.nLocalIOReg0Mask, mem_mask );
		return m_HPC.nLocalIOReg1Mask;
	case 0x01d4:
		verboselog(machine(), 2, "HPC VME Interrupt Mask 0 Read: %08x (%08x)\n", m_HPC.nLocalIOReg0Mask, mem_mask );
		return m_HPC.nVMEIntMask0;
	case 0x01d8:
		verboselog(machine(), 2, "HPC VME Interrupt Mask 1 Read: %08x (%08x)\n", m_HPC.nLocalIOReg0Mask, mem_mask );
		return m_HPC.nVMEIntMask1;
	case 0x0d00:
		verboselog(machine(), 2, "HPC DUART0 Channel B Control Read\n" );
//      return 0x00000004;
		return 0x7c; //scc->reg_r(space, 0);
	case 0x0d04:
		verboselog(machine(), 2, "HPC DUART0 Channel B Data Read\n" );
//      return 0;
		scc = machine().device<scc8530_t>("scc");
		return scc->reg_r(space, 2);
	case 0x0d08:
		verboselog(machine(), 2, "HPC DUART0 Channel A Control Read (%08x)\n", mem_mask	 );
//      return 0x40;
		return 0x7c; //scc->reg_r(space, 1);
	case 0x0d0c:
		verboselog(machine(), 2, "HPC DUART0 Channel A Data Read\n" );
//      return 0;
		scc = machine().device<scc8530_t>("scc");
		return scc->reg_r(space, 3);
	case 0x0d10:
//      verboselog(machine, 2, "HPC DUART1 Channel B Control Read\n" );
		return 0x00000004;
	case 0x0d14:
		verboselog(machine(), 2, "HPC DUART1 Channel B Data Read\n" );
		return 0;
	case 0x0d18:
		verboselog(machine(), 2, "HPC DUART1 Channel A Control Read\n" );
		return 0;
	case 0x0d1c:
		verboselog(machine(), 2, "HPC DUART1 Channel A Data Read\n" );
		return 0;
	case 0x0d20:
		verboselog(machine(), 2, "HPC DUART2 Channel B Control Read\n" );
		return 0x00000004;
	case 0x0d24:
		verboselog(machine(), 2, "HPC DUART2 Channel B Data Read\n" );
		return 0;
	case 0x0d28:
		verboselog(machine(), 2, "HPC DUART2 Channel A Control Read\n" );
		return 0;
	case 0x0d2c:
		verboselog(machine(), 2, "HPC DUART2 Channel A Data Read\n" );
		return 0;
	case 0x0d30:
		verboselog(machine(), 2, "HPC DUART3 Channel B Control Read\n" );
		return 0x00000004;
	case 0x0d34:
		verboselog(machine(), 2, "HPC DUART3 Channel B Data Read\n" );
		return 0;
	case 0x0d38:
		verboselog(machine(), 2, "HPC DUART3 Channel A Control Read\n" );
		return 0;
	case 0x0d3c:
		verboselog(machine(), 2, "HPC DUART3 Channel A Data Read\n" );
		return 0;
	}
	verboselog(machine(), 0, "Unmapped HPC read: 0x%08x (%08x)\n", 0x1fb80000 + offset, mem_mask );
	return 0;
}

WRITE32_MEMBER(ip20_state::hpc_w)
{
	scc8530_t *scc;
	eeprom_device *eeprom;
	eeprom = machine().device<eeprom_device>("eeprom");
	offset <<= 2;
	if( offset >= 0x0e00 && offset <= 0x0e7c )
	{
		verboselog(machine(), 2, "RTC RAM[0x%02x] Write: %02x\n", ( offset - 0xe00 ) >> 2, data & 0x000000ff );
		m_RTC.nRAM[ ( offset - 0xe00 ) >> 2 ] = data & 0x000000ff;
		switch( ( offset - 0xe00 ) >> 2 )
		{
		case 0:
			break;
		case 4:
			if( !( m_RTC.nRAM[0x00] & 0x80 ) )
			{
				if( data & 0x80 )
				{
					m_RTC.nRAM[0x19] = m_RTC.nRAM[0x06]; //RTC_SECOND;
					m_RTC.nRAM[0x1a] = m_RTC.nRAM[0x07]; //RTC_MINUTE;
					m_RTC.nRAM[0x1b] = m_RTC.nRAM[0x08]; //RTC_HOUR;
					m_RTC.nRAM[0x1c] = m_RTC.nRAM[0x09]; //RTC_DAY;
					m_RTC.nRAM[0x1d] = m_RTC.nRAM[0x0a]; //RTC_MONTH;
				}
			}
			break;
		}
		return;
	}
	switch( offset )
	{
	case 0x0090:	// SCSI0 next descriptor pointer
		m_HPC.nSCSI0Descriptor = data;
		break;

	case 0x0094:	// SCSI0 control flags
		m_HPC.nSCSI0DMACtrl = data;
		#if 0
		if (data & 0x80)
		{
			UINT32 next;

			mame_printf_info("DMA activated for SCSI0\n");
			mame_printf_info("Descriptor block:\n");
			mame_printf_info("CTL: %08x BUFPTR: %08x DESCPTR %08x\n",
				program_read_dword(m_HPC.nSCSI0Descriptor), program_read_dword(m_HPC.nSCSI0Descriptor+4),
				program_read_dword(m_HPC.nSCSI0Descriptor+8));

			next = program_read_dword(m_HPC.nSCSI0Descriptor+8);
			mame_printf_info("CTL: %08x BUFPTR: %08x DESCPTR %08x\n",
				program_read_dword(next), program_read_dword(next+4),
				program_read_dword(next+8));
		}
		#endif
		break;

	case 0x00ac:
		verboselog(machine(), 2, "HPC Parallel Buffer Pointer Write: %08x (%08x)\n", data, mem_mask );
		m_HPC.nParBufPtr = data;
		break;
	case 0x0120:
		if (ACCESSING_BITS_8_15)
		{
			verboselog(machine(), 2, "HPC SCSI Controller Register Write: %08x\n", ( data >> 8 ) & 0x000000ff );
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
			verboselog(machine(), 2, "HPC SCSI Controller Data Write: %08x\n", ( data >> 8 ) & 0x000000ff );
			m_wd33c93->write( space, 1, ( data >> 8 ) & 0x000000ff );
		}
		else
		{
			return;
		}
		break;
	case 0x01b0:
		verboselog(machine(), 2, "HPC Misc. Status Write: %08x (%08x)\n", data, mem_mask );
		if( data & 0x00000001 )
		{
			verboselog(machine(), 2, "  Force DSP hard reset\n" );
		}
		if( data & 0x00000002 )
		{
			verboselog(machine(), 2, "  Force IRQA\n" );
		}
		if( data & 0x00000004 )
		{
			verboselog(machine(), 2, "  Set IRQA polarity high\n" );
		}
		else
		{
			verboselog(machine(), 2, "  Set IRQA polarity low\n" );
		}
		if( data & 0x00000008 )
		{
			verboselog(machine(), 2, "  SRAM size: 32K\n" );
		}
		else
		{
			verboselog(machine(), 2, "  SRAM size:  8K\n" );
		}
		m_HPC.nMiscStatus = data;
		break;
	case 0x01bc:
//      verboselog(machine, 2, "HPC CPU Serial EEPROM Write: %08x (%08x)\n", data, mem_mask );
		if( data & 0x00000001 )
		{
			verboselog(machine(), 2, "    CPU board LED on\n" );
		}
		eeprom->write_bit((data & 0x00000008) ? 1 : 0 );
		eeprom->set_cs_line((data & 0x00000002) ? ASSERT_LINE : CLEAR_LINE );
		eeprom->set_clock_line((data & 0x00000004) ? CLEAR_LINE : ASSERT_LINE );
		break;
	case 0x01c4:
		verboselog(machine(), 2, "HPC Local IO Register 0 Mask Write: %08x (%08x)\n", data, mem_mask );
		m_HPC.nLocalIOReg0Mask = data;
		break;
	case 0x01cc:
		verboselog(machine(), 2, "HPC Local IO Register 1 Mask Write: %08x (%08x)\n", data, mem_mask );
		m_HPC.nLocalIOReg1Mask = data;
		break;
	case 0x01d4:
		verboselog(machine(), 2, "HPC VME Interrupt Mask 0 Write: %08x (%08x)\n", data, mem_mask );
		m_HPC.nVMEIntMask0 = data;
		break;
	case 0x01d8:
		verboselog(machine(), 2, "HPC VME Interrupt Mask 1 Write: %08x (%08x)\n", data, mem_mask );
		m_HPC.nVMEIntMask1 = data;
		break;
	case 0x0d00:
		verboselog(machine(), 2, "HPC DUART0 Channel B Control Write: %08x (%08x)\n", data, mem_mask );
		scc = machine().device<scc8530_t>("scc");
		scc->reg_w(space, 0, data);
		break;
	case 0x0d04:
		verboselog(machine(), 2, "HPC DUART0 Channel B Data Write: %08x (%08x)\n", data, mem_mask );
		scc = machine().device<scc8530_t>("scc");
		scc->reg_w(space, 2, data);
		break;
	case 0x0d08:
		verboselog(machine(), 2, "HPC DUART0 Channel A Control Write: %08x (%08x)\n", data, mem_mask );
		scc = machine().device<scc8530_t>("scc");
		scc->reg_w(space, 1, data);
		break;
	case 0x0d0c:
		verboselog(machine(), 2, "HPC DUART0 Channel A Data Write: %08x (%08x)\n", data, mem_mask );
		scc = machine().device<scc8530_t>("scc");
		scc->reg_w(space, 3, data);
		break;
	case 0x0d10:
		if( ( data & 0x000000ff ) >= 0x00000020 )
		{
//          verboselog(machine(), 2, "HPC DUART1 Channel B Control Write: %08x (%08x) %c\n", data, mem_mask, data & 0x000000ff );
			//mame_printf_info( "%c", data & 0x000000ff );
		}
		else
		{
//          verboselog(machine(), 2, "HPC DUART1 Channel B Control Write: %08x (%08x)\n", data, mem_mask );
		}
		break;
	case 0x0d14:
		if( ( data & 0x000000ff ) >= 0x00000020 || ( data & 0x000000ff ) == 0x0d || ( data & 0x000000ff ) == 0x0a )
		{
			verboselog(machine(), 2, "HPC DUART1 Channel B Data Write: %08x (%08x) %c\n", data, mem_mask, data & 0x000000ff );
			mame_printf_info( "%c", data & 0x000000ff );
		}
		else
		{
			verboselog(machine(), 2, "HPC DUART1 Channel B Data Write: %08x (%08x)\n", data, mem_mask );
		}
		break;
	case 0x0d18:
		mame_printf_info("HPC DUART1 Channel A Control Write: %08x (%08x)\n", data, mem_mask );
		break;
	case 0x0d1c:
		verboselog(machine(), 2, "HPC DUART1 Channel A Data Write: %08x (%08x)\n", data, mem_mask );
		break;
	case 0x0d20:
		mame_printf_info("HPC DUART2 Channel B Control Write: %08x (%08x)\n", data, mem_mask );
		break;
	case 0x0d24:
		verboselog(machine(), 2, "HPC DUART2 Channel B Data Write: %08x (%08x)\n", data, mem_mask );
		break;
	case 0x0d28:
		mame_printf_info("HPC DUART2 Channel A Control Write: %08x (%08x)\n", data, mem_mask );
		break;
	case 0x0d2c:
		verboselog(machine(), 2, "HPC DUART2 Channel A Data Write: %08x (%08x)\n", data, mem_mask );
		break;
	case 0x0d30:
		mame_printf_info("HPC DUART3 Channel B Control Write: %08x (%08x)\n", data, mem_mask );
		break;
	case 0x0d34:
		verboselog(machine(), 2, "HPC DUART3 Channel B Data Write: %08x (%08x)\n", data, mem_mask );
		break;
	case 0x0d38:
		mame_printf_info("HPC DUART3 Channel A Control Write: %08x (%08x)\n", data, mem_mask );
		break;
	case 0x0d3c:
		verboselog(machine(), 2, "HPC DUART3 Channel A Data Write: %08x (%08x)\n", data, mem_mask );
		break;
	default:
		mame_printf_info("Unmapped HPC write: 0x%08x (%08x): %08x\n", 0x1fb80000 + offset, mem_mask, data);
		break;
	}
}

// INT/INT2/INT3 interrupt controllers
READ32_MEMBER(ip20_state::int_r)
{
	mame_printf_info("INT: read @ ofs %x (mask %x) (PC=%x)\n", offset, mem_mask, space.device().safe_pc());
	return 0;
}

WRITE32_MEMBER(ip20_state::int_w)
{
	mame_printf_info("INT: write %x to ofs %x (mask %x) (PC=%x)\n", data, offset, mem_mask, space.device().safe_pc());
}

static ADDRESS_MAP_START( ip204415_map, AS_PROGRAM, 32, ip20_state )
	AM_RANGE( 0x00000000, 0x001fffff ) AM_RAM AM_SHARE("share10")
	AM_RANGE( 0x08000000, 0x08ffffff ) AM_RAM AM_SHARE("share5")
	AM_RANGE( 0x09000000, 0x097fffff ) AM_RAM AM_SHARE("share6")
	AM_RANGE( 0x0a000000, 0x0a7fffff ) AM_RAM AM_SHARE("share7")
	AM_RANGE( 0x0c000000, 0x0c7fffff ) AM_RAM AM_SHARE("share8")
	AM_RANGE( 0x10000000, 0x107fffff ) AM_RAM AM_SHARE("share9")
	AM_RANGE( 0x18000000, 0x187fffff ) AM_RAM AM_SHARE("share1")
	AM_RANGE( 0x1fa00000, 0x1fa1ffff ) AM_READWRITE_LEGACY(sgi_mc_r, sgi_mc_w )
	AM_RANGE( 0x1fb80000, 0x1fb8ffff ) AM_READWRITE(hpc_r, hpc_w )
	AM_RANGE( 0x1fbd9000, 0x1fbd903f ) AM_READWRITE(int_r, int_w )
	AM_RANGE( 0x1fc00000, 0x1fc7ffff ) AM_ROM AM_SHARE("share2") AM_REGION( "user1", 0 )
	AM_RANGE( 0x80000000, 0x801fffff ) AM_RAM AM_SHARE("share10")
	AM_RANGE( 0x88000000, 0x88ffffff ) AM_RAM AM_SHARE("share5")
	AM_RANGE( 0xa0000000, 0xa01fffff ) AM_RAM AM_SHARE("share10")
	AM_RANGE( 0xa8000000, 0xa8ffffff ) AM_RAM AM_SHARE("share5")
	AM_RANGE( 0xa9000000, 0xa97fffff ) AM_RAM AM_SHARE("share6")
	AM_RANGE( 0xaa000000, 0xaa7fffff ) AM_RAM AM_SHARE("share7")
	AM_RANGE( 0xac000000, 0xac7fffff ) AM_RAM AM_SHARE("share8")
	AM_RANGE( 0xb0000000, 0xb07fffff ) AM_RAM AM_SHARE("share9")
	AM_RANGE( 0xb8000000, 0xb87fffff ) AM_RAM AM_SHARE("share1")
	AM_RANGE( 0xbfa00000, 0xbfa1ffff ) AM_READWRITE_LEGACY(sgi_mc_r, sgi_mc_w )
	AM_RANGE( 0xbfb80000, 0xbfb8ffff ) AM_READWRITE(hpc_r, hpc_w )
	AM_RANGE( 0xbfbd9000, 0xbfbd903f ) AM_READWRITE(int_r, int_w )
	AM_RANGE( 0xbfc00000, 0xbfc7ffff ) AM_ROM AM_SHARE("share2") /* BIOS Mirror */
ADDRESS_MAP_END

static void scsi_irq(running_machine &machine, int state)
{
}

static const struct WD33C93interface wd33c93_intf =
{
	&scsi_irq,		/* command completion IRQ */
};

DRIVER_INIT_MEMBER(ip20_state,ip204415)
{
}

static TIMER_CALLBACK(ip20_timer)
{
	ip20_state *state = machine.driver_data<ip20_state>();

	// update RTC every 10 milliseconds
	state->m_RTC.nTemp++;
	if (state->m_RTC.nTemp >= 10)
	{
		state->m_RTC.nTemp = 0;
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
	}

	machine.scheduler().timer_set(attotime::from_msec(1), FUNC(ip20_timer));
}

void ip20_state::machine_start()
{

	sgi_mc_init(machine());

	m_HPC.nMiscStatus = 0;
	m_HPC.nParBufPtr = 0;
	m_HPC.nLocalIOReg0Mask = 0;
	m_HPC.nLocalIOReg1Mask = 0;
	m_HPC.nVMEIntMask0 = 0;
	m_HPC.nVMEIntMask1 = 0;

	m_RTC.nTemp = 0;

	machine().scheduler().timer_set(attotime::from_msec(1), FUNC(ip20_timer));
}

static INPUT_PORTS_START( ip204415 )
	PORT_START("unused")
	PORT_BIT ( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static const mips3_config config =
{
	32768,	/* code cache size */
	32768	/* data cache size */
};

static MACHINE_CONFIG_START( ip204415, ip20_state )
	MCFG_CPU_ADD( "maincpu", R4600BE, 50000000*3 )
	MCFG_CPU_CONFIG( config )
	MCFG_CPU_PROGRAM_MAP( ip204415_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( 60 )
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(800, 600)
	MCFG_SCREEN_VISIBLE_AREA(0, 799, 0, 599)
	MCFG_SCREEN_UPDATE_STATIC( ip204415 )

	MCFG_PALETTE_LENGTH(65536)


	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SCC8530_ADD("scc", 7000000, line_cb_t())

	MCFG_SCSIBUS_ADD("scsi")
	MCFG_SCSIDEV_ADD("scsi:cdrom", SCSICD, SCSI_ID_6)
	MCFG_WD33C93_ADD("scsi:wd33c93", wd33c93_intf)

	MCFG_EEPROM_ADD("eeprom", eeprom_interface_93C56)
MACHINE_CONFIG_END

ROM_START( ip204415 )
	ROM_REGION( 0x80000, "user1", 0 )
	ROM_LOAD( "ip204415.bin", 0x000000, 0x080000, CRC(940d960e) SHA1(596aba530b53a147985ff3f6f853471ce48c866c) )
ROM_END

/*    YEAR  NAME      PARENT    COMPAT    MACHINE   INPUT     INIT      COMPANY   FULLNAME */
COMP( 1993, ip204415, 0,        0,        ip204415, ip204415, ip20_state, ip204415, "Silicon Graphics Inc", "IRIS Indigo (R4400, 150MHz)", GAME_NOT_WORKING | GAME_NO_SOUND )
