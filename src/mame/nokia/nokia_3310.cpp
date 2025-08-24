// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*
    Driver for Nokia phones based on Texas Instrument MAD2WD1 (ARM7TDMI + DSP)

    Driver based on documentations found here:
        http://nokix.sourceforge.net/help/blacksphere/sub_050main.htm
        http://tudor.rdslink.ro/MADos/

*/

// if anybody has solid information to aid in the emulation of this (or other phones) please contribute.

#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "machine/intelfsh.h"
#include "video/pcd8544.h"

#include "debugger.h"
#include "emupal.h"
#include "screen.h"

#define LOG_MAD2_REGISTER_ACCESS    (1U << 1)
#define LOG_CCONT_REGISTER_ACCESS   (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

class noki3310_state : public driver_device
{
public:
	noki3310_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pcd8544(*this, "pcd8544"),
		m_keypad(*this, "COL.%u", 0),
		m_pwr(*this, "PWR")
	{ }

	void noki3330(machine_config &config);
	void noki3410(machine_config &config);
	void noki7110(machine_config &config);
	void noki6210(machine_config &config);
	void noki3310(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(key_irq);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	PCD8544_SCREEN_UPDATE(pcd8544_screen_update);

	uint8_t mad2_io_r(offs_t offset);
	void mad2_io_w(offs_t offset, uint8_t data);
	uint8_t mad2_dspif_r(offs_t offset);
	void mad2_dspif_w(offs_t offset, uint8_t data);
	uint8_t mad2_mcuif_r(offs_t offset);
	void mad2_mcuif_w(offs_t offset, uint8_t data);

	TIMER_CALLBACK_MEMBER(timer0);
	TIMER_CALLBACK_MEMBER(timer1);
	TIMER_CALLBACK_MEMBER(timer_watchdog);
	TIMER_CALLBACK_MEMBER(timer_fiq8);

	uint16_t ram_r(offs_t offset, uint16_t mem_mask = ~0) { return m_ram[offset] & mem_mask; }
	void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { COMBINE_DATA(&m_ram[offset]); }
	uint16_t dsp_ram_r(offs_t offset);
	void dsp_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void noki3310_map(address_map &map) ATTR_COLD;

	void assert_fiq(int num);
	void assert_irq(int num);
	void ack_fiq(uint16_t mask);
	void ack_irq(uint16_t mask);
	void nokia_ccont_w(uint8_t data);
	uint8_t nokia_ccont_r();

	required_device<cpu_device> m_maincpu;
	required_device<pcd8544_device> m_pcd8544;
	required_ioport_array<5> m_keypad;
	required_ioport m_pwr;

	std::unique_ptr<uint16_t[]>   m_ram;
	std::unique_ptr<uint16_t[]>   m_dsp_ram;

	uint8_t       m_power_on;
	uint16_t      m_fiq_status;
	uint16_t      m_irq_status;
	uint16_t      m_timer1_counter;
	uint16_t      m_timer0_counter;

	emu_timer * m_timer0;
	emu_timer * m_timer1;
	emu_timer * m_timer_watchdog;
	emu_timer * m_timer_fiq8;

	// CCONT
	struct nokia_ccont
	{
		bool    dc;
		uint8_t   cmd;
		uint8_t   watchdog;
		uint8_t   regs[0x10];
	} m_ccont;

	uint8_t       m_mad2_regs[0x100];
};


static const char * nokia_mad2_reg_desc(uint8_t offset)
{
	switch(offset)
	{
	case 0x00:  return "[CTSI] DCT3 ASIC version Primary hardware version (r)";
	case 0x01:  return "[CTSI] MCU reset control register (rw)";
	case 0x02:  return "[CTSI] DSP reset control register (rw)";
	case 0x03:  return "[CTSI] ASIC watchdog write register (w)";
	case 0x04:  return "[CTSI] Sleep clock counter (MSB) (r)";
	case 0x05:  return "[CTSI] Sleep clock counter (LSB) (r)";
	case 0x06:  return "[CTSI] ? (sleep) clock destination (LSB) (r)";
	case 0x07:  return "[CTSI] ? (sleep) clock destination (MSB) (r)";
	case 0x08:  return "[CTSI] FIQ lines active (rw)";
	case 0x09:  return "[CTSI] IRQ lines active (rw)";
	case 0x0A:  return "[CTSI] FIQ lines mask (rw)";
	case 0x0B:  return "[CTSI] IRQ lines mask (rw)";
	case 0x0C:  return "[CTSI] Interrupt control register (rw)";
	case 0x0D:  return "[CTSI] Clock control register (rw)";
	case 0x0E:  return "[CTSI] Interrupt trigger register (r)";
	case 0x0F:  return "[CTSI] Programmable timer clock divider (rw)";
	case 0x10:  return "[CTSI] Programmable timer counter (MSB) (r)";
	case 0x11:  return "[CTSI] Programmable timer counter (LSB) (r)";
	case 0x12:  return "[CTSI] Programmable timer destination (MSB) (rw)";
	case 0x13:  return "[CTSI] Programmable timer destination (LSB) (rw)";
	case 0x15:  return "[PUP] PUP control (rw)";
	case 0x16:  return "[PUP] FIQ 8 (timer?) interrupt control (rw)";
	case 0x18:  return "[PUP] MBUS control (rw)";
	case 0x19:  return "[PUP] MBUS status (rw)";
	case 0x1A:  return "[PUP] MBUS RX/TX (rw)";
	case 0x1B:  return "[PUP] Vibrator (w)";
	case 0x1C:  return "[PUP] Buzzer clock divider (w)";
	case 0x1E:  return "[PUP] Buzzer volume (w)";
	case 0x20:  return "[PUP] McuGenIO signal lines (rw)";
	case 0x22:  return "[PUP] ? (?)";
	case 0x24:  return "[PUP] McuGenIO I/O direction (rw)";
	case 0x28:  return "[UIF/KBGPIO] Keyboard ROW signal lines (rw)";
	case 0x29:  return "[UIF/KBGPIO] Keyboard ROW ?? (rw)";
	case 0x2A:  return "[UIF/KBGPIO] Keyboard COL signal lines (rw)";
	case 0x2B:  return "[UIF/KBGPIO] Keyboard COL ?? (rw)";
	case 0x2C:  return "[UIF/GENSIO] CCont write (w)";
	case 0x2D:  return "[UIF/GENSIO] GENSIO start transaction (w)";
	case 0x2E:  return "[UIF/GENSIO] LCD data write (w)";
	case 0x32:  return "[UIF] CTRL I/O 2 (rw)";
	case 0x33:  return "[UIF] CTRL I/O 3 (rw)";
	case 0x36:  return "[SIMI] SIM UART TxD (w)";
	case 0x37:  return "[SIMI] SIM UART RxD (r)";
	case 0x38:  return "[SIMI] SIM UART Interrupt Identification (r)";
	case 0x39:  return "[SIMI] SIM Control (rw)";
	case 0x3A:  return "[SIMI] SIM Clock Control (rw)";
	case 0x3B:  return "[SIMI] SIM UART TxD Low Water Mark (?)";
	case 0x3C:  return "[SIMI] SIM UART RxD queue fill (r)";
	case 0x3D:  return "[SIMI] SIM RxD flags (?)";
	case 0x3E:  return "[SIMI] SIM TxD flags (?)";
	case 0x3F:  return "[SIMI] SIM UART TxD queue fill (r)";
	case 0x68:  return "[UIF/KBGPIO] Keyboard ROW ?? 2 (rw)";
	case 0x69:  return "[UIF/KBGPIO] Keyboard ROW interrupt (rw)";
	case 0x6A:  return "[UIF/KBGPIO] Keyboard COL ?? 2 (rw)";
	case 0x6B:  return "[UIF/KBGPIO] Keyboard COL interrupt mask (rw)";
	case 0x6C:  return "[UIF/GENSIO] CCont read (r)";
	case 0x6D:  return "[UIF/GENSIO] GENSIO status (r)";
	case 0x6E:  return "[UIF/GENSIO] LCD command write (w)";
	case 0x6F:  return "[UIF/GENSIO] GENSIO ?? (3/SELECT1) (?)";
	case 0x70:  return "[UIF] CTRL I/O 0 I/O direction (1) (rw)";
	case 0x71:  return "[UIF] CTRL I/O 1 I/O direction (1) (rw)";
	case 0x72:  return "[UIF] CTRL I/O 2 I/O direction (1) (rw)";
	case 0x73:  return "[UIF] CTRL I/O 3 I/O direction (1) (rw)";
	case 0xA8:  return "[UIF/KBGPIO] Keyboard ROW I/O direction (rw)";
	case 0xA9:  return "[UIF/KBGPIO] Keyboard ROW ?? 3 (rw)";
	case 0xAA:  return "[UIF/KBGPIO] Keyboard COL I/O direction 0=in 1=out (rw)";
	case 0xAB:  return "[UIF/KBGPIO] Keyboard COL ?? 3 (rw)";
	case 0xAD:  return "[UIF/GENSIO] GENSIO ?? (1/SELECT2) (?)";
	case 0xAE:  return "[UIF/GENSIO] GENSIO ?? (2/SELECT2) (?)";
	case 0xAF:  return "[UIF/GENSIO] GENSIO ?? (3/SELECT2) (?)";
	case 0xB0:  return "[UIF] CTRL I/O 0 I/O direction (2) (rw)";
	case 0xB1:  return "[UIF] CTRL I/O 1 I/O direction (2) (rw)";
	case 0xB2:  return "[UIF] CTRL I/O 2 I/O direction (2) (rw)";
	case 0xB3:  return "[UIF] CTRL I/O 3 I/O direction (2) (rw)";
	case 0xED:  return "[UIF/GENSIO] GENSIO ?? (1/SELECT3) (?)";
	case 0xEE:  return "[UIF/GENSIO] GENSIO ?? (2/SELECT3) (?)";
	case 0xEF:  return "[UIF/GENSIO] GENSIO ?? (3/SELECT3) (?)";
	case 0xF0:  return "[UIF] CTRL I/O 0 input (r)";
	case 0xF1:  return "[UIF] CTRL I/O 1 input (r)";
	case 0xF2:  return "[UIF] CTRL I/O 2 input (r)";
	case 0xF3:  return "[UIF] CTRL I/O 3 input (r)";
	default:    return "<Unknown>";
	}
}

static const char * nokia_ccont_reg_desc(uint8_t offset)
{
	switch(offset)
	{
	case 0x0:   return "Control register (w)";
	case 0x1:   return "PWM (charger) (w)";
	case 0x2:   return "A/D read (LSB) (r)";
	case 0x3:   return "A/D read (MSB) (rw)";
	case 0x4:   return "?";
	case 0x5:   return "Watchdog (WDReg) (w)";
	case 0x6:   return "RTC enabled (w)";
	case 0x7:   return "RTC second (rw)";
	case 0x8:   return "RTC minute (r)";
	case 0x9:   return "RTC hour (r)";
	case 0xA:   return "RTC day (rw)";
	case 0xB:   return "RTC alarm minute (rw)";
	case 0xC:   return "RTC alarm hour (rw)";
	case 0xD:   return "RTC calibration value (rw)";
	case 0xE:   return "Interrupt lines (rw)";
	case 0xF:   return "Interrupt mask (rw)";
	default:    return "<Unknown>";
	}
}

void noki3310_state::machine_start()
{
	m_ram = std::make_unique<uint16_t[]>(0x40000);
	m_dsp_ram = std::make_unique<uint16_t[]>(0x800);      // DSP shared RAM

	// allocate timers
	m_timer0 = timer_alloc(FUNC(noki3310_state::timer0), this);
	m_timer1 = timer_alloc(FUNC(noki3310_state::timer1), this);
	m_timer_watchdog = timer_alloc(FUNC(noki3310_state::timer_watchdog), this);
	m_timer_fiq8 = timer_alloc(FUNC(noki3310_state::timer_fiq8), this);
}

void noki3310_state::machine_reset()
{
	// according to the boot rom disassembly here http://www.nokix.pasjagsm.pl/help/blacksphere/sub_100hardware/sub_arm/sub_bootrom.htm
	// flash entry point is at 0x200040, we can probably reassemble the above code, but for now this should be enough.
	m_maincpu->set_state_int(arm7_cpu_device::ARM7_R15, 0x200040);

	memset(m_mad2_regs, 0, 0x100);
	m_mad2_regs[0x01] = 0x01;   // power-on flag
	m_mad2_regs[0x0c] = 0x0a;   // disable FIQ and IRQ
	m_mad2_regs[0x03] = 0xff;   // disable MAD2 watchdog
	m_ccont.watchdog  = 0;      // disable CCONT watchdog
	m_ccont.dc  = false;

	m_fiq_status = 0;
	m_irq_status = 0;
	m_timer1_counter = 0;
	m_timer0_counter = 0;

	m_timer0->adjust(attotime::from_hz(33055 / (255 + 1)), 0, attotime::from_hz(33055 / (255 + 1)));    // programmable through port 0x0f
	m_timer1->adjust(attotime::from_hz(1057), 0, attotime::from_hz(1057));
	m_timer_watchdog->adjust(attotime::from_hz(1), 0, attotime::from_hz(1));
	m_timer_fiq8->adjust(attotime::from_hz(1000), 0, attotime::from_hz(1000));

	// simulate power-on input
	if (machine().system().name[4] == '8' || machine().system().name[4] == '5')
		m_power_on = ~0x10;
	else
		m_power_on = ~0x02;
}

void noki3310_state::assert_fiq(int num)
{
	if ((m_mad2_regs[0x0c] & 0x01) == 0)    // check if FIQ is globally enabled
		return;

	if (num < 8)
	{
		int mask = 1 << num;
		if (!(m_mad2_regs[0x0a] & mask))
		{
			m_maincpu->set_input_line(1, ASSERT_LINE);
			m_fiq_status |= mask;
		}
	}
	else if (!(m_mad2_regs[0x16] & 0x04))
	{
		m_fiq_status |= 0x100;
		m_maincpu->set_input_line(1, ASSERT_LINE);
	}
}

void noki3310_state::assert_irq(int num)
{
	if ((m_mad2_regs[0x0c] & 0x04) == 0)    // check if IRQ is globally enabled
		return;

	if (num < 8)
	{
		int mask = 1 << num;
		if (!(m_mad2_regs[0x0b] & mask))
		{
			m_irq_status |= mask;
			m_maincpu->set_input_line(0, ASSERT_LINE);
		}
	}
	else if (!(m_mad2_regs[0x0c] & 0x40))
	{
		m_irq_status |= 0x100;
		m_maincpu->set_input_line(0, ASSERT_LINE);
	}
}

void noki3310_state::ack_fiq(uint16_t mask)
{
	m_fiq_status &= ~mask;

	if (m_fiq_status == 0)
		m_maincpu->set_input_line(1, CLEAR_LINE);
}

void noki3310_state::ack_irq(uint16_t mask)
{
	m_irq_status &= ~mask;

	if (m_irq_status == 0)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

void noki3310_state::nokia_ccont_w(uint8_t data)
{
	if (m_ccont.dc == false)
	{
		LOGMASKED(LOG_CCONT_REGISTER_ACCESS, "CCONT command %s %x\n", data & 4 ? "R" : "W", data>>3);
		m_ccont.cmd  = data;
	}
	else
	{
		uint8_t addr = (m_ccont.cmd >> 3) & 0x0f;

		switch(addr)
		{
			case 0x0:   // ADC
			{
				uint16_t ad_id = (data >> 4) & 0x07;
				uint16_t ad_value = 0;
				switch(ad_id)
				{
					case 0:     ad_value = 0x000;   break;  // Accessory Detect
					case 1:     ad_value = 0x3ff;   break;  // Received signal strength
					case 2:     ad_value = 0x3ff;   break;  // Battery voltage
					case 3:     ad_value = 0x280;   break;  // Battery type
					case 4:     ad_value = 0x000;   break;  // Battery temperature
					case 5:     ad_value = 0x000;   break;  // Charger voltage
					case 6:     ad_value = 0x000;   break;  // VCX0 (Voltage controlled oscilator) temperature
					case 7:     ad_value = 0x000;   break;  // Charging current
				}

				m_ccont.regs[addr] = data;
				m_ccont.regs[2] = ad_value & 0xff;
				m_ccont.regs[3] = ((ad_value >> 8) & 0x03);
				break;
			}
			case 0x5:   // CCONT watchdog
				if (data == 0x20)
					m_ccont.regs[addr] = data;
				else if (data == 0x31)
					m_ccont.watchdog = m_ccont.regs[addr];
				else if (data == 0x3f)
					m_ccont.watchdog = 0;
				else if (data == 0)
					printf("CCONT power-off\n");
				break;

			default:
				m_ccont.regs[addr] = data;
				break;
		}

		LOGMASKED(LOG_CCONT_REGISTER_ACCESS, "CCONT W %02x = %02x %s\n", addr, data, nokia_ccont_reg_desc(addr));
	}

	m_ccont.dc = !m_ccont.dc;
}

uint8_t noki3310_state::nokia_ccont_r()
{
	uint8_t addr = (m_ccont.cmd >> 3) & 0x0f;
	uint8_t data = m_ccont.regs[addr];

	system_time systime;
	machine().current_datetime(systime);

	switch(addr)
	{
		case 0x3:       data = 0xb0 | (m_ccont.regs[addr] & 0x03);  break;
		case 0x7:       data = systime.local_time.second;           break;
		case 0x8:       data = systime.local_time.minute;           break;
		case 0x9:       data = systime.local_time.hour;             break;
		case 0xa:       data = systime.local_time.mday;             break;
		case 0xe:       data |= 0x01;                               break;
	}

	m_ccont.dc = !m_ccont.dc;

	LOGMASKED(LOG_CCONT_REGISTER_ACCESS, "CCONT R %02x = %02x %s\n", addr, data, nokia_ccont_reg_desc(addr));
	return data;
}

PCD8544_SCREEN_UPDATE(noki3310_state::pcd8544_screen_update)
{
	for (int r = 0; r < 6; r++)
		for (int x = 0; x < 84; x++)
		{
			uint8_t gfx = vram[r*84 + x];

			for (int y = 0; y < 8; y++)
			{
				int p = BIT(gfx, y);
				bitmap.pix(r*8 + y, x) = p ^ inv;
			}
		}
}

TIMER_CALLBACK_MEMBER(noki3310_state::timer0)
{
	m_timer0_counter++;

	if (m_timer0_counter == ((m_mad2_regs[0x12] << 8) | m_mad2_regs[0x13]))
		assert_fiq(4);
}

TIMER_CALLBACK_MEMBER(noki3310_state::timer1)
{
	m_timer1_counter++;

	if (m_timer1_counter == 0x8000)
	{
		assert_fiq(5);
		m_timer1_counter = 0;
	}
}

TIMER_CALLBACK_MEMBER(noki3310_state::timer_fiq8)
{
	if (m_mad2_regs[0x16] & 0x01)
		assert_fiq(8);
}

TIMER_CALLBACK_MEMBER(noki3310_state::timer_watchdog)
{
	// CCONT watchdog
	if (m_ccont.watchdog != 0)
	{
		m_ccont.watchdog--;

		if (m_ccont.watchdog == 0)
		{
			m_maincpu->reset();
			machine_reset();
		}
	}

	// MAD2 watchdog
	if (m_mad2_regs[0x03] != 0xff)
	{
		m_mad2_regs[0x03]--;
		if (m_mad2_regs[0x03] == 0)
		{
			m_maincpu->reset();
			machine_reset();
			m_mad2_regs[0x01] |= 0x02;  // Last reset was by watchdog
		}
	}
}

uint16_t noki3310_state::dsp_ram_r(offs_t offset)
{
	// HACK: avoid hangs when ARM try to communicate with the DSP
	if (offset <= 0x004 >> 1)   return 0x01;
	if (offset == 0x0e0 >> 1)   return 0x00;
	if (offset == 0x0fe >> 1)   return 0x01;
	if (offset == 0x100 >> 1)   return 0x01;

	return m_dsp_ram[offset & 0x7ff];
}

void noki3310_state::dsp_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dsp_ram[offset & 0x7ff]);
}

uint8_t noki3310_state::mad2_io_r(offs_t offset)
{
	uint8_t data = m_mad2_regs[offset];

	switch(offset)
	{
		case 0x00:
			data = 0x40;    // ASIC version
			break;
		case 0x04:
			data = m_timer1_counter >> 8;
			break;
		case 0x05:
			data = m_timer1_counter;
			break;
		case 0x08:
			data = m_fiq_status & 0xff;
			break;
		case 0x09:
			data = m_irq_status & 0xff;
			break;
		case 0x0c:
			data = (data & (~0x20)) | ((m_irq_status >> 3) & 0x20);
			break;
		case 0x10:
			data = m_timer0_counter >> 8;
			break;
		case 0x11:
			data = m_timer0_counter;
			break;
		case 0x16:
			data = (data & (~0x02)) | ((m_fiq_status >> 7) & 0x02);
			break;
		case 0x18:
			data &= 0x7f;
			break;
		case 0x2a:
			data = 0xff;
			for(int i=0; i<5; i++)
				if (!(m_mad2_regs[0x28] & (1 <<i)))
					data &= m_keypad[i]->read();

			data &= m_pwr->read();

			if (m_power_on)
			{
				data &= m_power_on;
				m_power_on = 0;
			}
			break;
		case 0x6c:
			data = nokia_ccont_r();
			break;
		case 0x6d:
			data = 0x07;    // GENSIO ready
			break;
	}

	LOGMASKED(LOG_MAD2_REGISTER_ACCESS, "MAD2 R %02x = %02x %s\n", offset, data, nokia_mad2_reg_desc(offset));
	return data;
}

void noki3310_state::mad2_io_w(offs_t offset, uint8_t data)
{
	m_mad2_regs[offset] = data;

	switch(offset)
	{
		case 0x02:
			//printf("DSP %s\n", data & 1 ? "RUN" : "HOLD");
			//if (data & 0x01)  machine().debug_break();
			break;
		case 0x08:
			ack_fiq(data);
			break;
		case 0x09:
			ack_irq(data);
			break;
		case 0x0c:
			ack_irq((data << 3) & 0x100);
			break;
		case 0x0f:
			m_timer0->adjust(attotime::from_hz(33055 / (data + 1)), 0, attotime::from_hz(33055 / (data + 1)));
			break;
		case 0x16:
			ack_fiq((data << 7) & 0x100);
			break;
		case 0x2c:
			nokia_ccont_w(data);
			break;
		case 0x2e:
		case 0x6e:
			m_pcd8544->dc_w(offset & 0x40 ? CLEAR_LINE : ASSERT_LINE);
			for (int i=7; i>=0; i--)
			{
				m_pcd8544->sclk_w(CLEAR_LINE);
				m_pcd8544->sdin_w(BIT(data, i));
				m_pcd8544->sclk_w(ASSERT_LINE);
			}
			m_pcd8544->dc_w(ASSERT_LINE);
			break;
	}

	LOGMASKED(LOG_MAD2_REGISTER_ACCESS, "MAD2 W %02x = %02x %s\n", offset, data, nokia_mad2_reg_desc(offset));
}

uint8_t noki3310_state::mad2_dspif_r(offs_t offset)
{
	LOGMASKED(LOG_MAD2_REGISTER_ACCESS, "MAD2 R %02x DSPIF\n", offset);
	return 0;
}

void noki3310_state::mad2_dspif_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MAD2_REGISTER_ACCESS, "MAD2 W %02x = %02x DSPIF\n", offset, data);
}

uint8_t noki3310_state::mad2_mcuif_r(offs_t offset)
{
	LOGMASKED(LOG_MAD2_REGISTER_ACCESS, "MAD2 R %02x MCUIF\n", offset);
	return 0;
}

void noki3310_state::mad2_mcuif_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MAD2_REGISTER_ACCESS, "MAD2 W %02x = %02x MCUIF\n", offset, data);
}


void noki3310_state::noki3310_map(address_map &map)
{
	map.global_mask(0x00ffffff);
	map(0x00000000, 0x0000ffff).mirror(0x80000).rw(FUNC(noki3310_state::ram_r), FUNC(noki3310_state::ram_w));                // boot ROM / RAM
	map(0x00010000, 0x00010fff).mirror(0x8f000).rw(FUNC(noki3310_state::dsp_ram_r), FUNC(noki3310_state::dsp_ram_w));        // DSP shared memory
	map(0x00020000, 0x000200ff).mirror(0x8ff00).rw(FUNC(noki3310_state::mad2_io_r), FUNC(noki3310_state::mad2_io_w));         // IO (Primary I/O range, configures peripherals)
	map(0x00030000, 0x00030003).mirror(0x8fffc).rw(FUNC(noki3310_state::mad2_dspif_r), FUNC(noki3310_state::mad2_dspif_w));   // DSPIF (API control register)
	map(0x00040000, 0x00040003).mirror(0x8fffc).rw(FUNC(noki3310_state::mad2_mcuif_r), FUNC(noki3310_state::mad2_mcuif_w));   // MCUIF (Secondary I/O range, configures memory ranges)
	map(0x00100000, 0x0017ffff).rw(FUNC(noki3310_state::ram_r), FUNC(noki3310_state::ram_w));                                   // RAMSelX
	map(0x00200000, 0x005fffff).rw("flash", FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));     // ROM1SelX
	map(0x00600000, 0x009fffff).unmaprw();                                                                   // ROM2SelX
	map(0x00a00000, 0x00dfffff).unmaprw();                                                                   // EEPROMSelX
	map(0x00e00000, 0x00ffffff).unmaprw();                                                                   // Reserved
}


INPUT_CHANGED_MEMBER( noki3310_state::key_irq )
{
	if (!newval)    // TODO: COL/ROW IRQ mask
		assert_irq(0);
}

static INPUT_PORTS_START( noki3310 )
	PORT_START("COL.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP)       PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(noki3310_state::key_irq), 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0)        PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(noki3310_state::key_irq), 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL)      PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(noki3310_state::key_irq), 0)

	PORT_START("COL.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN)     PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(noki3310_state::key_irq), 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2)        PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(noki3310_state::key_irq), 0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1)        PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(noki3310_state::key_irq), 0)

	PORT_START("COL.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6)        PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(noki3310_state::key_irq), 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5)        PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(noki3310_state::key_irq), 0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4)        PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(noki3310_state::key_irq), 0)

	PORT_START("COL.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9)        PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(noki3310_state::key_irq), 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8)        PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(noki3310_state::key_irq), 0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7)        PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(noki3310_state::key_irq), 0)

	PORT_START("COL.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3)        PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(noki3310_state::key_irq), 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS)    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(noki3310_state::key_irq), 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER)    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(noki3310_state::key_irq), 0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(noki3310_state::key_irq), 0)

	PORT_START("PWR")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE)    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(noki3310_state::key_irq), 0)
	PORT_BIT( 0x1d, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


void noki3310_state::noki3310(machine_config &config)
{
	/* basic machine hardware */
	ARM7_BE(config, m_maincpu, 26000000 / 2);  // MAD2WD1 13 MHz, clock internally supplied to ARM core can be divided by 2, in sleep mode a 32768 Hz clock is used
	m_maincpu->set_addrmap(AS_PROGRAM, &noki3310_state::noki3310_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD, rgb_t::white()));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(84, 48);
	screen.set_visarea(0, 84-1, 0, 48-1);
	screen.set_screen_update("pcd8544", FUNC(pcd8544_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	PCD8544(config, m_pcd8544, 0);
	m_pcd8544->set_screen_update_cb(FUNC(noki3310_state::pcd8544_screen_update));

	INTEL_TE28F160(config, "flash");
}

void noki3310_state::noki3330(machine_config &config)
{
	noki3310(config);

	INTEL_TE28F320(config.replace(), "flash");
}

void noki3310_state::noki3410(machine_config &config)
{
	noki3330(config);

	subdevice<screen_device>("screen")->set_size(96, 65);    // Philips OM6206
}

void noki3310_state::noki7110(machine_config &config)
{
	noki3330(config);

	subdevice<screen_device>("screen")->set_size(96, 65);    // Epson SED1565
}

void noki3310_state::noki6210(machine_config &config)
{
	noki3330(config);

	subdevice<screen_device>("screen")->set_size(96, 60);
}


// MAD2 internal ROMS
#define MAD2_INTERNAL_ROMS \
	ROM_REGION16_BE(0x10000, "boot_rom", ROMREGION_ERASE00 )    \
	ROM_LOAD("boot_rom", 0x00000, 0x10000, NO_DUMP)             \
																\
	ROM_REGION16_BE(0x20000, "dsp", ROMREGION_ERASE00 )         \
	ROM_LOAD("dsp_prom" , 0x00000, 0xc000, NO_DUMP)             \
	ROM_LOAD("dsp_drom" , 0x0c000, 0x4000, NO_DUMP)             \
	ROM_LOAD("dsp_pdrom", 0x10000, 0x1000, NO_DUMP)

ROM_START( noki3210 )
	MAD2_INTERNAL_ROMS

	ROM_REGION16_BE(0x200000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "600", "v6.00")  // A 03-10-2000
	ROMX_LOAD("3210f600a.fls", 0x000000, 0x200000, CRC(6a978478) SHA1(6bdec2ec76aca15bc12b621be4402e455562454b), ROM_BIOS(0))

	ROM_REGION16_BE(0x04000, "eeprom", 0 )
	ROM_LOAD("3210 virgin eeprom,24c128.bin", 0x00000, 0x04000, CRC(af8d8f65) SHA1(33a24c04d81a2bd8abce4a6fd873029f0c633ecb))
ROM_END

ROM_START( noki3310 )
	MAD2_INTERNAL_ROMS

	ROM_REGION16_BE(0x200000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "607", "v6.07")  // C 17-06-2003
	ROM_SYSTEM_BIOS(1, "579", "v5.79")  // N 11-11-2002
	ROM_SYSTEM_BIOS(2, "513", "v5.13")  // C 11-01-2002
	ROMX_LOAD("3310_607_ppm_c.fls", 0x000000, 0x200000, CRC(5743f6ba) SHA1(0e80b5f1698909c9850be770c1289566582aa77a), ROM_BIOS(0))
	ROMX_LOAD("3310 nr1 v5.79.fls", 0x000000, 0x200000, CRC(26b4f0df) SHA1(649de05ed88205a080693b918cd1295ac691dff1), ROM_BIOS(1))
	ROMX_LOAD("3310 v. 5.13 c.fls", 0x000000, 0x1d0000, CRC(0f66d256) SHA1(04d8dabe2c454d6a1161f352d85c69c409895000), ROM_BIOS(2))
	ROM_LOAD("3310 virgin eeprom 003d0000.fls", 0x1d0000, 0x030000, CRC(8393b1f7) SHA1(ab6c05bfa54ecd7c2acbd172009ffe6c7f130cb8))

	// these 2 are apparently the 6.39 update firmware data
	ROM_REGION(0x0200000, "misc", 0 )
	ROM_LOAD( "nhm5ny06.390",   0x000000, 0x0131161, CRC(5dfb1af7) SHA1(3a8ad82dc239b0cd18be60f537c4d0e07881155d) )
	ROM_LOAD( "nhm5ny06.39i",   0x000000, 0x0090288, CRC(ec214ee4) SHA1(f5b3b9ceaa7280d5246dd70d5696f8f6983122fc) )
ROM_END

ROM_START( noki3330 )
	MAD2_INTERNAL_ROMS

	ROM_REGION16_BE(0x0400000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "450", "v4.50")  // C 12-10-2001
	ROMX_LOAD("3330f450c.fls", 0x000000, 0x350000, CRC(259313e7) SHA1(88bcc39d9358fd8a8562fe3a0280f0ce82f5897f), ROM_BIOS(0))
	ROM_LOAD("3330 virgin eeprom 005f0000.fls", 0x3f0000, 0x010000, CRC(23459c10) SHA1(68481effb39d90a1639e8f261009c66e97d3e668))
ROM_END

ROM_START( noki3410 )
	MAD2_INTERNAL_ROMS

	ROM_REGION16_BE(0x0400000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "506", "v5.06")  // C 29-11-2002
	ROMX_LOAD("3410_5-06c.fls", 0x000000, 0x370000, CRC(1483e094) SHA1(ef26026297c779de7b01923a364ded822e720c38), ROM_BIOS(0))
ROM_END

ROM_START( noki5210 )
	MAD2_INTERNAL_ROMS

	ROM_REGION16_BE(0x0400000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "540", "v5.40")  // C 11-10-2003
	ROM_SYSTEM_BIOS(1, "525", "v5.25")  // C 26-02-2003
	ROM_SYSTEM_BIOS(2, "520", "v5.20")  // C 12-08-2002
	ROMX_LOAD("5210_5.40_ppm_c.fls", 0x000000, 0x380000, CRC(e37d5beb) SHA1(726f000780dd67750b7d2859687f846ce17a1bf7), ROM_BIOS(0))
	ROMX_LOAD("5210_5.25_ppm_c.fls", 0x000000, 0x380000, CRC(13bba458) SHA1(3b5244244743fba48f9061e158f95fc46b86446e), ROM_BIOS(1))
	ROMX_LOAD("5210_520_c.fls", 0x000000, 0x380000, CRC(38648cd3) SHA1(9210e15e6bd780f86c467bec33ef54d6393abe5a), ROM_BIOS(2))
ROM_END

ROM_START( noki6210 )
	MAD2_INTERNAL_ROMS

	ROM_REGION16_BE(0x0400000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "556", "v5.56")  // C 25-01-2002
	ROMX_LOAD("6210_556c.fls", 0x000000, 0x3a0000, CRC(203fb962) SHA1(3d9ea319503e78ec69b60d72cda23e461e118ea9), ROM_BIOS(0))
	ROM_LOAD("6210 virgin eeprom 005fa000.fls", 0x3fa000, 0x006000, CRC(3c6d3437) SHA1(b3a527ede1be87bd715fb3741a81eef5bd422efa))
ROM_END

ROM_START( noki6250 )
	MAD2_INTERNAL_ROMS

	ROM_REGION16_BE(0x0400000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "503", "v5.03")  // C 06-12-2001
	ROMX_LOAD("6250-503mcuppmc.fls", 0x000000, 0x3a0000, CRC(8dffb91b) SHA1(95607ce39c383bda75f1e6aeae67a214b787b0a1), ROM_BIOS(0))
	ROM_LOAD("6250 virgin eeprom 005fa000.fls", 0x3fa000, 0x006000, CRC(6087ce70) SHA1(57c29c8387caf864603d94a22bfb63ace427b7f9))
ROM_END

ROM_START( noki7110 )
	MAD2_INTERNAL_ROMS

	ROM_REGION16_BE(0x0400000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "501", "v5.01")  // C 08-12-2000
	ROMX_LOAD("7110f501_ppmc.fls", 0x000000, 0x390000, CRC(919ac753) SHA1(53af8324919f455ba8199d2c05f7a921cfb811d5), ROM_BIOS(0))
	ROM_LOAD("7110 virgin eeprom 005fa000.fls", 0x3fa000, 0x006000, CRC(78e7d8c1) SHA1(8b4dd782fc9d1306268ba63124ee463ac646912b))
ROM_END

ROM_START( noki8210 )
	MAD2_INTERNAL_ROMS

	ROM_REGION16_BE(0x200000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "531", "v5.31")  // C 08-03-2002
	ROMX_LOAD("8210_5.31ppm_c.fls", 0x000000, 0x1d0000, CRC(927022b1) SHA1(c1a0fe95cedb89a92b19654208cc4855e1a4988e), ROM_BIOS(0))
	ROM_LOAD("8210 virgin eeprom 003d0000.fls", 0x1d0000, 0x030000, CRC(37fddeea) SHA1(1c01ad3948ff9919890498a84f31052369d93e1d))
ROM_END

ROM_START( noki8250 )
	MAD2_INTERNAL_ROMS

	ROM_REGION16_BE(0x200000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "502", "v5.02")  // K 28-01-2002
	ROMX_LOAD("8250-502mcuppmk.fls", 0x000000, 0x1d0000, CRC(2c58e48b) SHA1(f26c98ffcfffbbd5714889e10cfa41c5f6dd2529), ROM_BIOS(0))
	ROM_LOAD("8250 virgin eeprom 003d0000.fls", 0x1d0000, 0x030000, CRC(7ca585e0) SHA1(a974fb5fddcd0438ac4aaf32b431f1453e8d923c))
ROM_END

ROM_START( noki8850 )
	MAD2_INTERNAL_ROMS

	ROM_REGION16_BE(0x200000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "531", "v5.31")  // C 08-03-2002
	ROMX_LOAD("8850v531.fls", 0x000000, 0x1d0000, CRC(8864fcb3) SHA1(9f966787403b68a09530680ad911302403eb1521), ROM_BIOS(0))
	ROM_LOAD("8850 virgin eeprom 003d0000.fls", 0x1d0000, 0x030000, CRC(4823f27e) SHA1(b09455302d98fbedf35072c9ecfd7721a04924b0))
ROM_END

ROM_START( noki8890 )
	MAD2_INTERNAL_ROMS

	ROM_REGION16_BE(0x200000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "1220", "v12.20")    // C 19-03-2001
	ROMX_LOAD("8890_12.20_ppmc.fls", 0x000000, 0x1d0000, CRC(77206f78) SHA1(a214a0d69760ecd8eeca0b9d82f95c94bdfe70ed), ROM_BIOS(0))
	ROM_LOAD("8890 virgin eeprom 003d0000.fls", 0x1d0000, 0x030000, CRC(1d8ef3b5) SHA1(cc0924cfd4c0ce796fca157c640fc3183c2b5f2c))
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY  FULLNAME      FLAGS
SYST( 1999, noki3210, 0,      0,      noki3310, noki3310, noki3310_state, empty_init, "Nokia", "Nokia 3210", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST( 1999, noki7110, 0,      0,      noki7110, noki3310, noki3310_state, empty_init, "Nokia", "Nokia 7110", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST( 1999, noki8210, 0,      0,      noki3310, noki3310, noki3310_state, empty_init, "Nokia", "Nokia 8210", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST( 1999, noki8850, 0,      0,      noki3310, noki3310, noki3310_state, empty_init, "Nokia", "Nokia 8850", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST( 2000, noki3310, 0,      0,      noki3310, noki3310, noki3310_state, empty_init, "Nokia", "Nokia 3310", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST( 2000, noki6210, 0,      0,      noki6210, noki3310, noki3310_state, empty_init, "Nokia", "Nokia 6210", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST( 2000, noki6250, 0,      0,      noki6210, noki3310, noki3310_state, empty_init, "Nokia", "Nokia 6250", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST( 2000, noki8250, 0,      0,      noki3310, noki3310, noki3310_state, empty_init, "Nokia", "Nokia 8250", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST( 2000, noki8890, 0,      0,      noki3310, noki3310, noki3310_state, empty_init, "Nokia", "Nokia 8890", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST( 2001, noki3330, 0,      0,      noki3330, noki3310, noki3310_state, empty_init, "Nokia", "Nokia 3330", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST( 2002, noki3410, 0,      0,      noki3410, noki3310, noki3310_state, empty_init, "Nokia", "Nokia 3410", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST( 2002, noki5210, 0,      0,      noki3330, noki3310, noki3310_state, empty_init, "Nokia", "Nokia 5210", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
