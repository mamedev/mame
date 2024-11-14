// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC1

    ASIC1 is the main system controller chip for the SIBO architecture. It connects
    directly to the 8086-based processor (i.e. the V30H) controlling all bus cycles
    to and from the processor. This configuration effectively forms a micro-controller
    like device that executes 8086 instruction codes. ASIC 1 is made up of a number of
    functional blocks including a bus controller, a programmable timer, an eight input
    interrupt controller, an LCD controller and the memory decoding circuitry.

******************************************************************************/

#include "emu.h"
#include "psion_asic1.h"
#include "screen.h"


#define VERBOSE 0
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(PSION_ASIC1, psion_asic1_device, "psion_asic1", "Psion ASIC1")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

psion_asic1_device::psion_asic1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSION_ASIC1, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_space_config("program", ENDIANNESS_LITTLE, 16, 20, 0)
	, m_tick_timer(nullptr)
	, m_frc_timer(nullptr)
	, m_watchdog_timer(nullptr)
	, m_int_cb(*this)
	, m_nmi_cb(*this)
	, m_frcovl_cb(*this)
	, m_laptop_mode(false)
{
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector psion_asic1_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_space_config)
	};
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psion_asic1_device::device_start()
{
	m_space = &space();

	m_tick_timer = timer_alloc(FUNC(psion_asic1_device::tick), this);
	m_frc_timer = timer_alloc(FUNC(psion_asic1_device::frc), this);
	m_watchdog_timer = timer_alloc(FUNC(psion_asic1_device::watchdog), this);

	m_a1_status = 0x00;

	save_item(NAME(m_a1_status));
	save_item(NAME(m_a1_lcd_size));
	save_item(NAME(m_a1_lcd_control));
	save_item(NAME(m_a1_interrupt_status));
	save_item(NAME(m_a1_interrupt_mask));
	save_item(NAME(m_a1_protection_mode));
	save_item(NAME(m_a1_protection_lower));
	save_item(NAME(m_a1_protection_upper));
	save_item(NAME(m_frc_count));
	save_item(NAME(m_frc_reload));
	save_item(NAME(m_watchdog_count));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void psion_asic1_device::device_reset()
{
	m_tick_timer->adjust(attotime::from_hz(4), 0, attotime::from_hz(4));
	m_frc_timer->adjust(attotime::from_hz(512000), 0, attotime::from_hz(512000));
	m_watchdog_timer->adjust(attotime::from_hz(4), 0, attotime::from_hz(4));

	m_frc_count = 0;
	m_frc_reload = 0;
	m_frc_ovl = 0;
	m_watchdog_count = 0;

	m_a1_interrupt_status = 0x00;
	m_a1_interrupt_mask = 0x00;

	m_a1_protection_mode = false;
	m_a1_protection_lower = 0x00;
	m_a1_protection_upper = 0x00;
}


TIMER_CALLBACK_MEMBER(psion_asic1_device::tick)
{
	m_a1_interrupt_status |= 0x01; // Timer
	update_interrupts();
}

TIMER_CALLBACK_MEMBER(psion_asic1_device::frc)
{
	switch (--m_frc_count)
	{
	case 0x0000:
		m_frcovl_cb(m_frc_ovl ^= 1);

		m_a1_interrupt_status |= 0x20; // FrcExpired
		update_interrupts();
		break;

	case 0xffff:
		if (BIT(m_a1_status, 0)) // FrcMode
			m_frc_count = m_frc_reload;
		break;
	}
}

TIMER_CALLBACK_MEMBER(psion_asic1_device::watchdog)
{
	m_watchdog_count++;
	m_watchdog_count &= 3;

	if (m_watchdog_count == 3)
	{
		m_a1_status |= 0x0100; // WatchDogNmi
		update_interrupts();
	}
}

void psion_asic1_device::eint1_w(int state)
{
	if (state)
		m_a1_interrupt_status |= 0x04; // ExpIntRightB
	else
		m_a1_interrupt_status &= ~0x04;

	update_interrupts();
}

void psion_asic1_device::eint2_w(int state)
{
	if (state)
		m_a1_interrupt_status |= 0x08; // ExpIntLeftA
	else
		m_a1_interrupt_status &= ~0x08;

	update_interrupts();
}

void psion_asic1_device::eint3_w(int state)
{
	if (state)
		m_a1_interrupt_status |= 0x10; // Asic2Int
	else
		m_a1_interrupt_status &= ~0x10;

	update_interrupts();
}

void psion_asic1_device::enmi_w(int state)
{
	if (state)
		m_a1_status |= 0x0200; // ExternalNmi
	else
		m_a1_status &= ~0x0200;

	update_interrupts();
}

void psion_asic1_device::update_interrupts(bool address_trap)
{
	bool irq = m_a1_interrupt_status & m_a1_interrupt_mask;
	bool nmi = (m_a1_status & 0x0300) || address_trap;

	m_int_cb(irq ? ASSERT_LINE : CLEAR_LINE);
	m_nmi_cb(nmi ? ASSERT_LINE : CLEAR_LINE);
}


IRQ_CALLBACK_MEMBER(psion_asic1_device::inta_cb)
{
	// IRQ  Vector  Name   Description
	//  0    0x78   TINT   Tick interrupt at 2 or 32 Hz
	//  1    0x79   EINT0  External interrupt usually connected to mains detect bit
	//  2    0x7A   EINT1  External interrupt from expansion port one
	//  3    0x7B   EINT2  External interrupt from expansion port two
	//  4    0x7C   EINT3  External interrupt from ASIC2
	//  5    0x7D   OVINT  Timer overflow interrupt
	//  6    0x7E   SRXI   SLD sound receive interrupt
	//  7    0x7F   STXI   SLD sound transmit interrupt
	uint8_t vector = 0x78;
	for (int irq = 0; irq < 8; irq++)
	{
		if (m_a1_interrupt_status & m_a1_interrupt_mask & (1 << irq))
		{
			vector += irq;
			break;
		}
	}
	return vector;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

bool psion_asic1_device::is_protected(offs_t offset)
{
	if (m_a1_protection_mode && (offset <= m_a1_protection_lower || offset > m_a1_protection_upper))
	{
		LOG("%s is_protected: %05x < %05x <= %05x\n", machine().describe_context(), m_a1_protection_lower, offset, m_a1_protection_upper);
		update_interrupts(true);
		return true;
	}
	return false;
}


uint16_t psion_asic1_device::mem_r(offs_t offset, uint16_t mem_mask)
{
	return m_space->read_word(offset << 1, mem_mask);
}

void psion_asic1_device::mem_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!is_protected(offset << 1))
		m_space->write_word(offset << 1, data, mem_mask);
}


uint16_t psion_asic1_device::io_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0x00;

	switch (offset << 1)
	{
	case 0x02: // A1Status
		//   b0    FrcMode
		//   b1    TickRate
		//   b2    FrcSource
		//   b3    Ram128
		//   b4    Ram512
		//   b5    LcdEnable
		//   b6    A1SldEnable
		//   b7    SldTx
		//   b8    WatchDogNmi
		//   b9    ExternalNmi
		//   b10   Rtc32Hz
		//   b11   ComboBusy
		//   b12   SldMsw
		//   b13   Rtc4Hz
		// b14-b15 LcdData - LCD id as follows:
		//                     0 = Lcd640X400
		//                     1 = Lcd640X200Small
		//                     2 = Lcd640X200Big
		//                     3 = Lcd720X348
		//                     4 = Lcd160X80
		data = m_a1_status;
		data |= lcd_type() << 14;
		LOG("%s io_r: A1Status => %04x\n", machine().describe_context(), data);
		break;

	case 0x06: // A1InterruptStatus
		data = m_a1_interrupt_status & m_a1_interrupt_mask;
		LOG("%s io_r: A1InterruptStatus => %02x\n", machine().describe_context(), data);
		break;

	case 0x08: // A1InterruptMask
		data = m_a1_interrupt_mask;
		LOG("%s io_r: A1InterruptMask => %02x\n", machine().describe_context(), data);
		break;

	case 0x12: // A1FrcControl
		data = m_frc_count;
		LOG("%s io_r: A1FrcControl => %04x\n", machine().describe_context(), data);
		break;

	case 0x14: // A1ProtectionOff
		if (!machine().side_effects_disabled())
		{
			//LOG("%s io_r: A1ProtectionOff => %04x\n", machine().describe_context(), data);
			m_a1_protection_mode = false;
		}
		break;

	default:
		data = 0xffff;
		LOG("%s io_r: Unhandled register %02x => %04x\n", machine().describe_context(), offset << 1, data);
		break;
	}
	return data;
}

void psion_asic1_device::io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset << 1)
	{
	case 0x02: // A1Control
		//   b0    FrcMode
		//   b1    TickRate
		//   b2    FrcSource
		//   b3    Ram128
		//   b4    Ram512
		//   b5    LcdEnable
		//   b6    A1SldEnable
		//   b7    SldTx
		LOG("%s io_w: A1Control <= %04x\n", machine().describe_context(), data);
		if (BIT(data, 1) != BIT(m_a1_status, 1))
		{
			if (data & 0x02) // TickRate
				m_tick_timer->adjust(attotime::zero, 0, attotime::from_hz(32.768)); // RTC from PS34
			else
				m_tick_timer->adjust(attotime::zero, 0, attotime::from_hz(4));
		}
		m_a1_status = (m_a1_status & 0xff00) | (data & 0xff);
		break;

	case 0x04: // A1LcdSize
		//  b0-b9  LcdEndOfFrame     - (Total pixels in display / 128) - 1
		// b10-b14 LcdNumberOfPixels - (No. pixels in line / 32) - 1
		//   b15   LcdMLineEnable    - 1 to enable the M line magic.
		LOG("%s io_w: A1LcdSize <= %04x, Pixels in line %d, Total pixels in display %d\n", machine().describe_context(), data, (BIT(data, 10, 5) + 1) * 32, (BIT(data, 0, 10) + 1) * 128);
		m_a1_lcd_size = data;
		break;

	case 0x06: // A1LcdControl
		//  b0-b4  LcdRate      - LCDCLK = SYSCLK / (2*(n+1))
		//  b5-b9  LcdMLineRate - 13
		// b10-b11 LcdMode      - 3 (Dual Screen mode)
		LOG("%s io_w: A1LcdControl <= %04x, LCD clock %dHz, %s\n", machine().describe_context(), data, clock() * 4 / (screen().width() * screen().height() * 2 * (BIT(data, 0, 5) + 1)), BIT(data, 10, 2) == 3 ? "Dual screen" : "Single screen");
		m_a1_lcd_control = data;
		break;

	case 0x08: // A1InterruptMask
		// b0 Timer
		// b1 Mains
		// b2 ExpIntRightB
		// b3 ExpIntLeftA
		// b4 Asic2Int
		// b5 FrcExpired
		// b6 SldReceive
		// b7 SldTransmit
		LOG("%s io_w: A1InterruptMask <= %02x\n", machine().describe_context(), data);
		m_a1_interrupt_mask = data & 0xff;
		update_interrupts();
		break;

	case 0x0a: // A1NonSpecificEoi
		LOG("%s io_w: A1NonSpecificEoi <= %04x\n", machine().describe_context(), data);
		break;

	case 0x0c: // A1TimerEoi
		LOG("%s io_w: A1TimerEoi <= %04x\n", machine().describe_context(), data);
		m_a1_interrupt_status &= ~0x01; // Timer
		update_interrupts();
		break;

	case 0x0e: // A1FrcEoi
		LOG("%s io_w: A1FrcEoi <= %04x\n", machine().describe_context(), data);
		m_a1_interrupt_status &= ~0x20; // FrcExpired
		update_interrupts();
		break;

	case 0x10: // A1ResetWatchDog
		//LOG("% io_w: A1ResetWatchDog <= %04x\n", machine().describe_context(), data);
		m_watchdog_count = 0;
		break;

	case 0x12: // A1FrcControl
		LOG("%s io_w: A1FrcControl <= %04x\n", machine().describe_context(), data);
		m_frc_reload = data;
		m_frc_count = data;
		break;

	case 0x14: // A1ProtectionOn
		//LOG("%s io_w: A1ProtectionOn <= %04x\n", machine().describe_context(), data);
		m_a1_protection_mode = true;
		break;

	case 0x16: // A1ProtectionUpper
		//LOG("%s io_w: A1ProtectionUpper <= %04x\n", machine().describe_context(), data);
		m_a1_protection_upper = (data << 4) | 0x0f;
		break;

	case 0x18: // A1ProtectionLower
		//LOG("%s io_w: A1ProtectionLower <= %04x\n", machine().describe_context(), data);
		m_a1_protection_lower = data << 4;
		break;

	case 0x1a: // A1SoundLsw
		LOG("%s io_w: A1SoundLsw <= %04x\n", machine().describe_context(), data);
		break;

	case 0x1c: // A1SoundMsw
		LOG("%s io_w: A1SoundMsw <= %04x\n", machine().describe_context(), data);
		break;

	case 0x1e: // A1SoundControl
		LOG("%s io_w: A1SoundControl <= %04x\n", machine().describe_context(), data);
		break;

	default:
		LOG("%s io_w: Unhandled register %02x <= %04x\n", machine().describe_context(), offset << 1, data);
		break;
	}
}


//-------------------------------------------------
//  LCD Controller
//-------------------------------------------------

uint8_t psion_asic1_device::lcd_type()
{
	if (m_laptop_mode)
	{
		switch (screen().height())
		{
		case 400: return 0;
		case 200: return 1;
		}
	}
	return 0;
}

uint32_t psion_asic1_device::screen_update_single(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return screen_update(screen, bitmap, cliprect, 1);
}

uint32_t psion_asic1_device::screen_update_dual(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return screen_update(screen, bitmap, cliprect, 2);
}

uint32_t psion_asic1_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int screens)
{
	if (m_a1_status & 0x0020) // LCD enable bit
	{
		pen_t const *const pens = screen.palette().pens();

		offs_t videoram = m_laptop_mode ? 0xb8000 : 0x00400;
		int const width = (BIT(m_a1_lcd_size, 10, 5) + 1) * 32;
		int const height = screen.height() / screens;

		for (int vmap = 0; vmap < screens; vmap++)
		{
			for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y / screens; y++)
			{
				for (int x = screen.visible_area().min_x; x <= (screen.visible_area().max_x / 8); x++)
				{
					uint8_t const pixels = m_space->read_byte(videoram + (vmap << 14) + (y * (width / 8)) + x);
					uint16_t *p = &bitmap.pix((vmap * height) + y, x << 3);
					for (int i = 0; i < 8; i++)
						*p++ = pens[BIT(pixels, i)];
				}
			}
		}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}
	return 0;
}
