/***************************************************************************

	Sharp Zaurus PDA skeleton driver (SL, ARM/Linux based, 4th generation)

	TODO:
	- PXA-255 ID opcode fails on this
	- ARM TLB look-up errors?
	- RTC irq doesn't fire?
	- For whatever reason, after RTC check ARM executes invalid code at 0-0x200
	- Dumps are questionable to say the least

***************************************************************************/


#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "machine/pxa255.h"

#define MAIN_CLOCK XTAL_8MHz

class zaurus_state : public driver_device
{
public:
	zaurus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_ram(*this, "ram")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT32> m_ram;

	UINT8 m_rtc_tick;
	DECLARE_READ32_MEMBER(pxa255_ostimer_r);
	DECLARE_WRITE32_MEMBER(pxa255_ostimer_w);
	DECLARE_READ32_MEMBER(pxa255_rtc_r);
	DECLARE_WRITE32_MEMBER(pxa255_rtc_w);
	DECLARE_READ32_MEMBER(pxa255_intc_r);
	DECLARE_WRITE32_MEMBER(pxa255_intc_w);
	PXA255_OSTMR_Regs m_ostimer_regs;
	PXA255_INTC_Regs m_intc_regs;

	void pxa255_ostimer_irq_check();
	void pxa255_update_interrupts();
	void pxa255_set_irq_line(UINT32 line, int irq_state);
	TIMER_DEVICE_CALLBACK_MEMBER(rtc_irq_callback);

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
	virtual void palette_init();
};

#define VERBOSE_LEVEL ( 5 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine& machine, int n_level, const char* s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", machine.describe_context(), buf );
		//printf( "%s: %s", machine.describe_context(), buf );
	}
}


void zaurus_state::video_start()
{
}

UINT32 zaurus_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{


	return 0;
}

void zaurus_state::pxa255_update_interrupts()
{
	PXA255_INTC_Regs *intc_regs = &m_intc_regs;

	intc_regs->icfp = (intc_regs->icpr & intc_regs->icmr) & intc_regs->iclr;
	intc_regs->icip = (intc_regs->icpr & intc_regs->icmr) & (~intc_regs->iclr);
	m_maincpu->set_input_line(ARM7_FIRQ_LINE, intc_regs->icfp ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(ARM7_IRQ_LINE,  intc_regs->icip ? ASSERT_LINE : CLEAR_LINE);
}

void zaurus_state::pxa255_set_irq_line(UINT32 line, int irq_state)
{
	PXA255_INTC_Regs *intc_regs = &m_intc_regs;

	intc_regs->icpr &= ~line;
	intc_regs->icpr |= irq_state ? line : 0;
	//printf( "Setting IRQ line %08x to %d\n", line, irq_state );
	pxa255_update_interrupts();
}

void zaurus_state::pxa255_ostimer_irq_check()
{
	PXA255_OSTMR_Regs *ostimer_regs = &m_ostimer_regs;

//	logerror("%08x OStimer irq check\n",ostimer_regs->oier);

	pxa255_set_irq_line(PXA255_INT_OSTIMER0, (ostimer_regs->oier & PXA255_OIER_E0) ? ((ostimer_regs->ossr & PXA255_OSSR_M0) ? 1 : 0) : 0);
	//pxa255_set_irq_line(PXA255_INT_OSTIMER1, (ostimer_regs->oier & PXA255_OIER_E1) ? ((ostimer_regs->ossr & PXA255_OSSR_M1) ? 1 : 0) : 0);
	//pxa255_set_irq_line(PXA255_INT_OSTIMER2, (ostimer_regs->oier & PXA255_OIER_E2) ? ((ostimer_regs->ossr & PXA255_OSSR_M2) ? 1 : 0) : 0);
	//pxa255_set_irq_line(PXA255_INT_OSTIMER3, (ostimer_regs->oier & PXA255_OIER_E3) ? ((ostimer_regs->ossr & PXA255_OSSR_M3) ? 1 : 0) : 0);
}

READ32_MEMBER(zaurus_state::pxa255_ostimer_r)
{
	PXA255_OSTMR_Regs *ostimer_regs = &m_ostimer_regs;

	switch(PXA255_OSTMR_BASE_ADDR | (offset << 2))
	{
		case PXA255_OSMR0:
			if (0) verboselog( machine(), 3, "pxa255_ostimer_r: OS Timer Match Register 0: %08x & %08x\n", ostimer_regs->osmr[0], mem_mask );
			return ostimer_regs->osmr[0];
		case PXA255_OSMR1:
			if (0) verboselog( machine(), 3, "pxa255_ostimer_r: OS Timer Match Register 1: %08x & %08x\n", ostimer_regs->osmr[1], mem_mask );
			return ostimer_regs->osmr[1];
		case PXA255_OSMR2:
			if (0) verboselog( machine(), 3, "pxa255_ostimer_r: OS Timer Match Register 2: %08x & %08x\n", ostimer_regs->osmr[2], mem_mask );
			return ostimer_regs->osmr[2];
		case PXA255_OSMR3:
			if (0) verboselog( machine(), 3, "pxa255_ostimer_r: OS Timer Match Register 3: %08x & %08x\n", ostimer_regs->osmr[3], mem_mask );
			return ostimer_regs->osmr[3];
		case PXA255_OSCR:
			if (0) verboselog( machine(), 4, "pxa255_ostimer_r: OS Timer Count Register: %08x & %08x\n", ostimer_regs->oscr, mem_mask );
			// free-running 3.something MHz counter.  this is a complete hack.
			ostimer_regs->oscr += 0x300;
			return ostimer_regs->oscr;
		case PXA255_OSSR:
			if (0) verboselog( machine(), 3, "pxa255_ostimer_r: OS Timer Status Register: %08x & %08x\n", ostimer_regs->ossr, mem_mask );
			return ostimer_regs->ossr;
		case PXA255_OWER:
			if (0) verboselog( machine(), 3, "pxa255_ostimer_r: OS Timer Watchdog Match Enable Register: %08x & %08x\n", ostimer_regs->ower, mem_mask );
			return ostimer_regs->ower;
		case PXA255_OIER:
			if (0) verboselog( machine(), 3, "pxa255_ostimer_r: OS Timer Interrupt Enable Register: %08x & %08x\n", ostimer_regs->oier, mem_mask );
			return ostimer_regs->oier;
		default:
			if (0) verboselog( machine(), 0, "pxa255_ostimer_r: Unknown address: %08x\n", PXA255_OSTMR_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

WRITE32_MEMBER(zaurus_state::pxa255_ostimer_w)
{
	PXA255_OSTMR_Regs *ostimer_regs = &m_ostimer_regs;

	switch(PXA255_OSTMR_BASE_ADDR | (offset << 2))
	{
		case PXA255_OSMR0:
			if (0) verboselog( machine(), 3, "pxa255_ostimer_w: OS Timer Match Register 0: %08x & %08x\n", data, mem_mask );
			ostimer_regs->osmr[0] = data;
			if(ostimer_regs->oier & PXA255_OIER_E0)
			{
				attotime period = attotime::from_hz(3846400) * (ostimer_regs->osmr[0] - ostimer_regs->oscr);

				//printf( "Adjusting one-shot timer to 200MHz * %08x\n", ostimer_regs->osmr[0]);
				ostimer_regs->timer[0]->adjust(period);
			}
			break;
		case PXA255_OSMR1:
			if (0) verboselog( machine(), 3, "pxa255_ostimer_w: OS Timer Match Register 1: %08x & %08x\n", data, mem_mask );
			ostimer_regs->osmr[1] = data;
			if(ostimer_regs->oier & PXA255_OIER_E1)
			{
				attotime period = attotime::from_hz(3846400) * (ostimer_regs->osmr[1] - ostimer_regs->oscr);

				ostimer_regs->timer[1]->adjust(period, 1);
			}
			break;
		case PXA255_OSMR2:
			if (0) verboselog( machine(), 3, "pxa255_ostimer_w: OS Timer Match Register 2: %08x & %08x\n", data, mem_mask );
			ostimer_regs->osmr[2] = data;
			if(ostimer_regs->oier & PXA255_OIER_E2)
			{
				attotime period = attotime::from_hz(3846400) * (ostimer_regs->osmr[2] - ostimer_regs->oscr);

				ostimer_regs->timer[2]->adjust(period, 2);
			}
			break;
		case PXA255_OSMR3:
			if (0) verboselog( machine(), 3, "pxa255_ostimer_w: OS Timer Match Register 3: %08x & %08x\n", data, mem_mask );
			ostimer_regs->osmr[3] = data;
			if(ostimer_regs->oier & PXA255_OIER_E3)
			{
				//attotime period = attotime::from_hz(3846400) * (ostimer_regs->osmr[3] - ostimer_regs->oscr);

				//ostimer_regs->timer[3]->adjust(period, 3);
			}
			break;
		case PXA255_OSCR:
			if (0) verboselog( machine(), 3, "pxa255_ostimer_w: OS Timer Count Register: %08x & %08x\n", data, mem_mask );
			ostimer_regs->oscr = data;
			break;
		case PXA255_OSSR:
			if (0) verboselog( machine(), 3, "pxa255_ostimer_w: OS Timer Status Register: %08x & %08x\n", data, mem_mask );
			ostimer_regs->ossr &= ~data;
			pxa255_ostimer_irq_check();
			break;
		case PXA255_OWER:
			if (0) verboselog( machine(), 3, "pxa255_ostimer_w: OS Timer Watchdog Enable Register: %08x & %08x\n", data, mem_mask );
			ostimer_regs->ower = data & 0x00000001;
			break;
		case PXA255_OIER:
		{
			int index = 0;
			if (0) verboselog( machine(), 3, "pxa255_ostimer_w: OS Timer Interrupt Enable Register: %08x & %08x\n", data, mem_mask );
			ostimer_regs->oier = data & 0x0000000f;
			for(index = 0; index < 4; index++)
			{
				if(ostimer_regs->oier & (1 << index))
				{
					//attotime period = attotime::from_hz(200000000) * ostimer_regs->osmr[index];

					//ostimer_regs->timer[index]->adjust(period, index);
				}
			}

			break;
		}
		default:
			verboselog( machine(), 0, "pxa255_ostimer_w: Unknown address: %08x = %08x & %08x\n", PXA255_OSTMR_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

READ32_MEMBER(zaurus_state::pxa255_intc_r)
{
	PXA255_INTC_Regs *intc_regs = &m_intc_regs;

	switch(PXA255_INTC_BASE_ADDR | (offset << 2))
	{
		case PXA255_ICIP:
			if (0) verboselog( machine(), 3, "pxa255_intc_r: Interrupt Controller IRQ Pending Register: %08x & %08x\n", intc_regs->icip, mem_mask );
			return intc_regs->icip;
		case PXA255_ICMR:
			if (0) verboselog( machine(), 3, "pxa255_intc_r: Interrupt Controller Mask Register: %08x & %08x\n", intc_regs->icmr, mem_mask );
			return intc_regs->icmr;
		case PXA255_ICLR:
			if (0) verboselog( machine(), 3, "pxa255_intc_r: Interrupt Controller Level Register: %08x & %08x\n", intc_regs->iclr, mem_mask );
			return intc_regs->iclr;
		case PXA255_ICFP:
			if (0) verboselog( machine(), 3, "pxa255_intc_r: Interrupt Controller FIQ Pending Register: %08x & %08x\n", intc_regs->icfp, mem_mask );
			return intc_regs->icfp;
		case PXA255_ICPR:
			if (0) verboselog( machine(), 3, "pxa255_intc_r: Interrupt Controller Pending Register: %08x & %08x\n", intc_regs->icpr, mem_mask );
			return intc_regs->icpr;
		case PXA255_ICCR:
			if (0) verboselog( machine(), 3, "pxa255_intc_r: Interrupt Controller Control Register: %08x & %08x\n", intc_regs->iccr, mem_mask );
			return intc_regs->iccr;
		default:
			verboselog( machine(), 0, "pxa255_intc_r: Unknown address: %08x\n", PXA255_INTC_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

WRITE32_MEMBER(zaurus_state::pxa255_intc_w)
{
	PXA255_INTC_Regs *intc_regs = &m_intc_regs;

	switch(PXA255_INTC_BASE_ADDR | (offset << 2))
	{
		case PXA255_ICIP:
			verboselog( machine(), 3, "pxa255_intc_w: (Invalid Write) Interrupt Controller IRQ Pending Register: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_ICMR:
			if (0) verboselog( machine(), 3, "pxa255_intc_w: Interrupt Controller Mask Register: %08x & %08x\n", data, mem_mask );
			intc_regs->icmr = data & 0xfffe7f00;
			break;
		case PXA255_ICLR:
			if (0) verboselog( machine(), 3, "pxa255_intc_w: Interrupt Controller Level Register: %08x & %08x\n", data, mem_mask );
			intc_regs->iclr = data & 0xfffe7f00;
			break;
		case PXA255_ICFP:
			if (0) verboselog( machine(), 3, "pxa255_intc_w: (Invalid Write) Interrupt Controller FIQ Pending Register: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_ICPR:
			if (0) verboselog( machine(), 3, "pxa255_intc_w: (Invalid Write) Interrupt Controller Pending Register: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_ICCR:
			if (0) verboselog( machine(), 3, "pxa255_intc_w: Interrupt Controller Control Register: %08x & %08x\n", data, mem_mask );
			intc_regs->iccr = data & 0x00000001;
			break;
		default:
			verboselog( machine(), 0, "pxa255_intc_w: Unknown address: %08x = %08x & %08x\n", PXA255_INTC_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

READ32_MEMBER(zaurus_state::pxa255_rtc_r)
{
	printf("%08x\n",offset << 2);

	return 0;
}

WRITE32_MEMBER(zaurus_state::pxa255_rtc_w)
{
	printf("%08x %08x\n",offset << 2,data);

}

static ADDRESS_MAP_START( zaurus_map, AS_PROGRAM, 32, zaurus_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_REGION("firmware", 0)
	AM_RANGE(0x40900000, 0x4090000f) AM_READWRITE(pxa255_rtc_r,pxa255_rtc_w)
	AM_RANGE(0x40a00000, 0x40a0001f) AM_READWRITE(pxa255_ostimer_r, pxa255_ostimer_w )
	AM_RANGE(0x40d00000, 0x40d00017) AM_READWRITE(pxa255_intc_r, pxa255_intc_w )
	AM_RANGE(0xa0000000, 0xa07fffff) AM_RAM AM_SHARE("ram")
ADDRESS_MAP_END


static INPUT_PORTS_START( zaurus )
INPUT_PORTS_END



void zaurus_state::machine_start()
{
}

void zaurus_state::machine_reset()
{
}


void zaurus_state::palette_init()
{
}

/* TODO: Hack */
TIMER_DEVICE_CALLBACK_MEMBER(zaurus_state::rtc_irq_callback)
{
	#if 0
	m_rtc_tick++;
	m_rtc_tick&=1;

	if(m_rtc_tick & 1)
		pxa255_set_irq_line(PXA255_INT_RTC_HZ,1);
	else
		pxa255_set_irq_line(PXA255_INT_RTC_HZ,0);
	#endif
}

// TODO: main CPU differs greatly between versions!
static MACHINE_CONFIG_START( zaurus, zaurus_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",PXA255,MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(zaurus_map)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("rtc_timer", zaurus_state, rtc_irq_callback, attotime::from_hz(XTAL_32_768kHz))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(zaurus_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MCFG_PALETTE_LENGTH(8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

/* was labeled SL-C500 */
ROM_START( zsl5500 )
	ROM_REGION32_LE( 0x200000, "firmware", ROMREGION_ERASE00 )
    ROM_LOAD( "sl-c500 v1.20 (zimage).bin", 0x000000, 0x13c000, BAD_DUMP CRC(dc1c259f) SHA1(8150744196a72821ae792462d0381182274c2ce0) )
ROM_END

ROM_START( zsl5600 )
	ROM_REGION32_LE( 0x200000, "firmware", ROMREGION_ERASE00 )
	ROM_LOAD( "zaurus sl-b500 - 5600 (zimage).bin", 0x000000, 0x11b6b0, BAD_DUMP CRC(779c70a1) SHA1(26824e3dc563b681f195029f220dfaa405613f9e) )
ROM_END

ROM_START( zslc750 )
	ROM_REGION32_LE( 0x200000, "firmware", ROMREGION_ERASE00 )
    ROM_LOAD( "zaurus sl-c750 (zimage).bin", 0x000000, 0x121544, BAD_DUMP CRC(56353f4d) SHA1(8e1fff6e93d560bd6572c5c163bbd81378693f68) )
ROM_END

ROM_START( zslc760 )
	ROM_REGION32_LE( 0x200000, "firmware", ROMREGION_ERASE00 )
	ROM_LOAD( "zaurus sl-c760 (zimage).bin", 0x000000, 0x120b44, BAD_DUMP CRC(feedcba3) SHA1(1821ad0fc03a8c3832ad5fe2221c21c1ca277508) )
ROM_END

ROM_START( zslc3000 )
	ROM_REGION32_LE( 0x200000, "firmware", ROMREGION_ERASE00 )
    ROM_LOAD( "openzaurus 3.5.3 - zimage-sharp sl-c3000-20050428091110.bin", 0x000000, 0x12828c, BAD_DUMP CRC(fd94510d) SHA1(901f8154b4228a448f5551f0c9f21c2153e1e3a1) )
ROM_END

ROM_START( zslc1000 )
	ROM_REGION32_LE( 0x200000, "firmware", ROMREGION_ERASE00 )
    ROM_LOAD( "openzaurus 3.5.3 - zimage-sharp sl-c1000-20050427214434.bin", 0x000000, 0x128980, BAD_DUMP  CRC(1e1a9279) SHA1(909ac3f00385eced55822d6a155b79d9d25f43b3) )
ROM_END

GAME( 2002, zsl5500,  0,   zaurus,  zaurus, driver_device,  0,       ROT0, "Sharp",      "Zaurus SL-5500 \"Collie\"", GAME_IS_SKELETON )
GAME( 2002, zsl5600,  0,   zaurus,  zaurus, driver_device,  0,       ROT0, "Sharp",      "Zaurus SL-5600 / SL-B500 \"Poodle\"", GAME_IS_SKELETON )
GAME( 2003, zslc750,  0,   zaurus,  zaurus, driver_device,  0,       ROT0, "Sharp",      "Zaurus SL-C750 \"Shepherd\" (Japan)", GAME_IS_SKELETON )
GAME( 2004, zslc760,  0,   zaurus,  zaurus, driver_device,  0,       ROT0, "Sharp",      "Zaurus SL-C760 \"Husky\" (Japan)", GAME_IS_SKELETON )
GAME( 200?, zslc3000, 0,   zaurus,  zaurus, driver_device,  0,       ROT0, "Sharp",      "Zaurus SL-C3000 \"Spitz\" (Japan)", GAME_IS_SKELETON )
GAME( 200?, zslc1000, 0,   zaurus,  zaurus, driver_device,  0,       ROT0, "Sharp",      "Zaurus SL-C3000 \"Akita\" (Japan)", GAME_IS_SKELETON )
