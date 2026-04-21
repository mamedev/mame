// license:BSD-3-Clause
// copyright-holders:David Haywood

// preliminary emulation, just enough for mapacman to run for now

#include "emu.h"

#include "generalplus_gpce4_soc.h"

#define LOG_IO     (1U << 1)
#define LOG_TIMERS (1U << 2)
#define LOG_PWM    (1U << 3)
#define LOG_MISC   (1U << 4)
#define LOG_IRQ    (1U << 5)
#define LOG_SPI1   (1U << 6)
#define LOG_SPI2   (1U << 7)
#define LOG_ADC    (1U << 8)
#define LOG_DAC    (1U << 9)

#define VERBOSE     (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(GPCE4, generalplus_gpce4_soc_device, "gpce4", "GeneralPlus GPCE4 series SoC")

generalplus_gpce4_soc_device::generalplus_gpce4_soc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: unsp_20_device(mconfig, GPCE4, tag, owner, clock, address_map_constructor(FUNC(generalplus_gpce4_soc_device::internal_map), this))
	, m_porta_in(*this, 0)
	, m_portb_in(*this, 0)
	, m_portc_in(*this, 0)
	, m_porta_out(*this)
	, m_portb_out(*this)
	, m_portc_out(*this)
	, m_spi2_out(*this)
	, m_dac(*this, "dac")
	, m_bank(*this, "spibank")
{
}

void generalplus_gpce4_soc_device::device_start()
{
	unsp_20_device::device_start();

	save_item(NAME(m_ioa_buffer));
	save_item(NAME(m_iob_buffer));
	save_item(NAME(m_interrupt_ctrl));
	save_item(NAME(m_interrupt2_ctrl));
	save_item(NAME(m_timer_ctrl));
	save_item(NAME(m_io_ctrl));
	save_item(NAME(m_ioa_attribute));
	save_item(NAME(m_iob_attribute));
	save_item(NAME(m_ioc_attribute));
	save_item(NAME(m_ioa_direction));
	save_item(NAME(m_iob_direction));
	save_item(NAME(m_ioc_direction));
	save_item(NAME(m_cache_ctrl));
	save_item(NAME(m_spi_auto_ctrl));
	save_item(NAME(m_spi_bank));
	save_item(NAME(m_fiq_sel));
	save_item(NAME(m_fiq2_sel));
	save_item(NAME(m_spi2_ctrl));

	set_vectorbase(0x4010);

	m_bank->configure_entries(0, memregion("spi")->bytes() / 0x400000, memregion("spi")->base(), 0x400000);
	m_bank->set_entry(0);
}

void generalplus_gpce4_soc_device::device_reset()
{
	unsp_20_device::device_reset();

	m_ioa_buffer = 0;
	m_iob_buffer = 0;
	m_interrupt_ctrl = 0;
	m_interrupt2_ctrl = 0;
	m_timer_ctrl = 0;
	m_io_ctrl = 0;
	m_ioa_attribute = 0;
	m_iob_attribute = 0;
	m_ioc_attribute = 0;
	m_ioa_direction = 0;
	m_iob_direction = 0;
	m_ioc_direction = 0;
	m_cache_ctrl = 0;
	m_spi_auto_ctrl = 0;
	m_spi_bank = 0;
	m_fiq_sel = 0;
	m_fiq2_sel = 0;
	m_spi2_ctrl = 0;
}

void generalplus_gpce4_soc_device::device_add_mconfig(machine_config &config)
{
	TIMER(config, "timer_c").configure_periodic(FUNC(generalplus_gpce4_soc_device::timer_c_cb), attotime::from_hz(1000)); // game speed?
	TIMER(config, "timer_a").configure_periodic(FUNC(generalplus_gpce4_soc_device::timer_a_cb), attotime::from_hz(20000)); // audio
	TIMER(config, "timer_2hz").configure_periodic(FUNC(generalplus_gpce4_soc_device::timer_2hz_cb), attotime::from_hz(2));
	TIMER(config, "timer_64hz").configure_periodic(FUNC(generalplus_gpce4_soc_device::timer_64hz_cb), attotime::from_hz(64));

	SPEAKER(config, "speaker").front_center();
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 1.0); // unknown DAC
}

void generalplus_gpce4_soc_device::internal_map(address_map &map)
{
	map(0x000000, 0x000fff).ram(); // RAM
	//map(0x001000, 0x0017ff).ram(); // Cache area, can be configured as RAM instead

	map(0x003000, 0x003000).rw(FUNC(generalplus_gpce4_soc_device::ioa_data_r), FUNC(generalplus_gpce4_soc_device::ioa_data_w)); // IOA_Data
	map(0x003001, 0x003001).rw(FUNC(generalplus_gpce4_soc_device::ioa_buffer_r), FUNC(generalplus_gpce4_soc_device::ioa_buffer_w)); // IOA_Buffer
	map(0x003002, 0x003002).rw(FUNC(generalplus_gpce4_soc_device::ioa_direction_r), FUNC(generalplus_gpce4_soc_device::ioa_direction_w)); // IOA_Dir
	map(0x003003, 0x003003).rw(FUNC(generalplus_gpce4_soc_device::ioa_attribute_r), FUNC(generalplus_gpce4_soc_device::ioa_attribute_w)); // IOA_Attrib

	map(0x003004, 0x003004).rw(FUNC(generalplus_gpce4_soc_device::iob_data_r), FUNC(generalplus_gpce4_soc_device::iob_data_w)); // IOB_Data
	map(0x003005, 0x003005).rw(FUNC(generalplus_gpce4_soc_device::iob_buffer_r), FUNC(generalplus_gpce4_soc_device::iob_buffer_w)); // IOB_Buffer
	map(0x003006, 0x003006).rw(FUNC(generalplus_gpce4_soc_device::iob_direction_r), FUNC(generalplus_gpce4_soc_device::iob_direction_w)); // IOB_Dir
	map(0x003007, 0x003007).rw(FUNC(generalplus_gpce4_soc_device::iob_attribute_r), FUNC(generalplus_gpce4_soc_device::iob_attribute_w)); // IOB_Attrib

	map(0x003008, 0x003008).rw(FUNC(generalplus_gpce4_soc_device::ioc_data_r), FUNC(generalplus_gpce4_soc_device::ioc_data_w)); // IOC_Data
	map(0x003009, 0x003009).rw(FUNC(generalplus_gpce4_soc_device::ioc_buffer_r), FUNC(generalplus_gpce4_soc_device::ioc_buffer_w)); // IOC_Buffer
	map(0x00300a, 0x00300a).rw(FUNC(generalplus_gpce4_soc_device::ioc_direction_r), FUNC(generalplus_gpce4_soc_device::ioc_direction_w)); // IOC_Dir
	map(0x00300b, 0x00300b).rw(FUNC(generalplus_gpce4_soc_device::ioc_attribute_r), FUNC(generalplus_gpce4_soc_device::ioc_attribute_w)); // IOC_Attrib

	// 300c - IOA_WakeUp_Mask
	// 300d - IOB_WakeUp_Mask
	// 300e - IOC_WakeUp_Mask
	map(0x00300f, 0x00300f).rw(FUNC(generalplus_gpce4_soc_device::io_ctrl_r), FUNC(generalplus_gpce4_soc_device::io_ctrl_w)); // IO_Ctrl

	map(0x003010, 0x003010).w(FUNC(generalplus_gpce4_soc_device::timera_data_w)); // TimerA_Data
	map(0x003011, 0x003011).w(FUNC(generalplus_gpce4_soc_device::timera_counter_w)); // TimerA_CNTR
	map(0x003012, 0x003012).w(FUNC(generalplus_gpce4_soc_device::timerb_data_w)); // TimerB_Data
	map(0x003013, 0x003013).w(FUNC(generalplus_gpce4_soc_device::timerb_counter_w)); // TimerB_CNTR
	map(0x003014, 0x003014).w(FUNC(generalplus_gpce4_soc_device::timerc_data_w)); // TimerC_Data
	map(0x003015, 0x003015).w(FUNC(generalplus_gpce4_soc_device::timerc_counter_w)); // TimerC_CNTR
	map(0x003016, 0x003016).rw(FUNC(generalplus_gpce4_soc_device::timer_ctrl_r), FUNC(generalplus_gpce4_soc_device::timer_ctrl_w)); // Timer_Ctrl

	map(0x003020, 0x003020).w(FUNC(generalplus_gpce4_soc_device::pwm0_ctrl_w)); // PWM0_Ctrl
	map(0x003021, 0x003021).w(FUNC(generalplus_gpce4_soc_device::pwm1_ctrl_w)); // PWM1_Ctrl
	map(0x003022, 0x003022).w(FUNC(generalplus_gpce4_soc_device::pwm2_ctrl_w)); // PWM2_Ctrl
	map(0x003023, 0x003023).w(FUNC(generalplus_gpce4_soc_device::pwm3_ctrl_w)); // PWM3_Ctrl

	map(0x003030, 0x003030).w(FUNC(generalplus_gpce4_soc_device::system_clock_w)); // System_Clock
	// 3031 - System_Reset
	// 3032 - Reset_LVD_Ctrl
	map(0x003033, 0x003033).w(FUNC(generalplus_gpce4_soc_device::timebase_clear_w)); // Timebase_Clear
	map(0x003034, 0x003034).w(FUNC(generalplus_gpce4_soc_device::watchdog_clear_w)); // Watchdog_Clear
	map(0x003035, 0x003035).w(FUNC(generalplus_gpce4_soc_device::wait_ctrl_w)); // Wait_Ctrl
	// 3036 - System_Sleep

	map(0x003039, 0x003039).rw(FUNC(generalplus_gpce4_soc_device::cache_ctrl_r), FUNC(generalplus_gpce4_soc_device::cache_ctrl_w)); // Cache_Ctrl
	// 303a - Cache_Hit_Rate
	// 303b - Stack_INT_Level

	map(0x003040, 0x003040).w(FUNC(generalplus_gpce4_soc_device::dac_ctrl_w)); // DAC_Ctrl
	map(0x003041, 0x003041).w(FUNC(generalplus_gpce4_soc_device::dac_cha1_data_w)); // DAC_CH1_Data
	map(0x003042, 0x003042).w(FUNC(generalplus_gpce4_soc_device::dac_cha2_data_w)); // DAC_CH2_Data
	map(0x003043, 0x003043).w(FUNC(generalplus_gpce4_soc_device::ppam_ctrl_w)); // PPAMCtrl

	map(0x003050, 0x003050).rw(FUNC(generalplus_gpce4_soc_device::interrupt_ctrl_r), FUNC(generalplus_gpce4_soc_device::interrupt_ctrl_w)); // INT_Ctrl
	map(0x003051, 0x003051).rw(FUNC(generalplus_gpce4_soc_device::interrupt_status_r), FUNC(generalplus_gpce4_soc_device::interrupt_status_w)); // INT_Status
	map(0x003052, 0x003052).rw(FUNC(generalplus_gpce4_soc_device::fiq_sel_r), FUNC(generalplus_gpce4_soc_device::fiq_sel_w)); // FIQ_SEL
	map(0x003053, 0x003053).rw(FUNC(generalplus_gpce4_soc_device::interrupt2_ctrl_r), FUNC(generalplus_gpce4_soc_device::interrupt2_ctrl_w)); // INT2_Ctrl
	map(0x003054, 0x003054).rw(FUNC(generalplus_gpce4_soc_device::interrupt2_status_r), FUNC(generalplus_gpce4_soc_device::interrupt2_status_w)); // INT2_Status
	map(0x003055, 0x003055).rw(FUNC(generalplus_gpce4_soc_device::fiq2_sel_r), FUNC(generalplus_gpce4_soc_device::fiq2_sel_w)); // FIQ2_SEL

	map(0x003070, 0x003070).rw(FUNC(generalplus_gpce4_soc_device::adc_ctrl_r), FUNC(generalplus_gpce4_soc_device::adc_ctrl_w)); // ADC_Ctrl
	map(0x003071, 0x003071).rw(FUNC(generalplus_gpce4_soc_device::adc_data_r), FUNC(generalplus_gpce4_soc_device::adc_data_w)); // ADC_Data
	map(0x003072, 0x003072).rw(FUNC(generalplus_gpce4_soc_device::adc_linein_bitctrl_r), FUNC(generalplus_gpce4_soc_device::adc_linein_bitctrl_w)); // ADC_LineIn_BitCtrl
	// 3073 - ADC_PGA_Ctrl
	// 3074 - ADC_FIFO_Ctrl

	map(0x003090, 0x003090).rw(FUNC(generalplus_gpce4_soc_device::spi2_ctrl_r), FUNC(generalplus_gpce4_soc_device::spi2_ctrl_w)); // SPI2_Ctrl
	map(0x003091, 0x003091).rw(FUNC(generalplus_gpce4_soc_device::spi2_txstatus_r), FUNC(generalplus_gpce4_soc_device::spi2_txstatus_w)); // SPI2_TXStatus
	map(0x003092, 0x003092).w(FUNC(generalplus_gpce4_soc_device::spi2_txdata_w)); // SPI2_TXData
	map(0x003093, 0x003093).rw(FUNC(generalplus_gpce4_soc_device::spi2_rxstatus_r), FUNC(generalplus_gpce4_soc_device::spi2_rxstatus_w)); // SPI2_RXStatus
	map(0x003094, 0x003094).r(FUNC(generalplus_gpce4_soc_device::spi2_rxdata_r)); // SPI2_RXData 
	map(0x003095, 0x003095).rw(FUNC(generalplus_gpce4_soc_device::spi2_misc_r), FUNC(generalplus_gpce4_soc_device::spi2_misc_w)); // SPI2_Misc

	// 309a - SPI2_DMA_Start
	// 309b - SPI2_DMA_BC
	// 309c - SPI2_RX_DMA_ADDR
	// 309d - SPI2_TX_DMA_ADDR
	// 309e - SPI2_DMA_INT_Ctrl
	// 309f - SPI2_RX_ICE

	// 30b0 - BUF_ACTIVE
	// 30b1 - LINK_BUF10
	// 30b2 - LINK_BUF32
	// 30b3 - LINK_BUF54

	// 30d0 - CTS_Ctrl0
	// 30d1 - CTS_Ctrl1
	// 30d2 - CTS_TMADATA
	// 30d3 - CTS_TMACNT
	// 30d4 - CTS_TMBDATA
	// 30d5 - CTS_TMBCNT
	// 30d6 - CTS_CAPTMB
	// 30d7 - CTS_MUTCTRL

	map(0x0030e0, 0x0030e0).rw(FUNC(generalplus_gpce4_soc_device::spi_ctrl_r), FUNC(generalplus_gpce4_soc_device::spi_ctrl_w)); // SPI_Ctrl
	map(0x0030e1, 0x0030e1).w(FUNC(generalplus_gpce4_soc_device::spi_txstatus_w)); // SPI_TXStatus
	map(0x0030e2, 0x0030e2).w(FUNC(generalplus_gpce4_soc_device::spi_txdata_w)); // SPI_TXData
	map(0x0030e3, 0x0030e3).w(FUNC(generalplus_gpce4_soc_device::spi_rxstatus_w)); // SPI_RXStatus
	map(0x0030e4, 0x0030e4).r(FUNC(generalplus_gpce4_soc_device::spi_rxdata_r)); // SPI_RXData
	map(0x0030e5, 0x0030e5).rw(FUNC(generalplus_gpce4_soc_device::spi_misc_r), FUNC(generalplus_gpce4_soc_device::spi_misc_w)); // SPI_Misc
	// 30e6 - SPI_Man_Ctrl
	map(0x0030e7, 0x0030e7).rw(FUNC(generalplus_gpce4_soc_device::spi_auto_ctrl_r), FUNC(generalplus_gpce4_soc_device::spi_auto_ctrl_w)); //  SPI_Auto_Ctrl
	// 30e8 - SPI_PRGM_BC
	map(0x0030e9, 0x0030e9).rw(FUNC(generalplus_gpce4_soc_device::spi_bank_r), FUNC(generalplus_gpce4_soc_device::spi_bank_w)); // SPI_BANK
	// 30ea - SPI_DMA_Start
	// 30eb - SPI_DMA_BC
	// 30ec - SPI_RX_DMA_ADDR
	// 30ed - SPI_TX_DMA_ADDR
	// 30ee - SPI_DMA_INT_Ctrl
	// 30ef - SPI_RX_ICE

	map(0x004000, 0x00bfff).rom().region("internal", 0x0000); // 0x4000-0x400f are 'test' vectors, 0x4010 - 0x401f are user vectors, 0x4020 - 0x47ff is 'standard test program' 0x4800+ is custom internal ROM
	map(0x00c000, 0x00ffff).rom().region("internal", 0x0000);

	map(0x200000, 0x3fffff).bankr("spibank"); // has direct access to SPI ROM
}

TIMER_DEVICE_CALLBACK_MEMBER( generalplus_gpce4_soc_device::timer_c_cb )
{
	request_interrupt(29);
}

TIMER_DEVICE_CALLBACK_MEMBER( generalplus_gpce4_soc_device::timer_a_cb )
{
	request_interrupt(31);
}

TIMER_DEVICE_CALLBACK_MEMBER( generalplus_gpce4_soc_device::timer_2hz_cb )
{
	request_interrupt(0);
}

TIMER_DEVICE_CALLBACK_MEMBER( generalplus_gpce4_soc_device::timer_64hz_cb )
{
	request_interrupt(2);
}

// IOA

u16 generalplus_gpce4_soc_device::ioa_data_r()
{
	LOGMASKED(LOG_IO, "%s: m_porta_in\n", machine().describe_context());
	return m_porta_in();
}

void generalplus_gpce4_soc_device::ioa_data_w(u16 data)
{
	LOGMASKED(LOG_IO, "%s: ioa_data_w %04x\n", machine().describe_context(), data);
	m_porta_out(data);
}

u16 generalplus_gpce4_soc_device::ioa_buffer_r()
{
	LOGMASKED(LOG_IO, "%s: ioa_buffer_r\n", machine().describe_context());
	return m_porta_in();
}

void generalplus_gpce4_soc_device::ioa_buffer_w(u16 data)
{
	m_ioa_buffer = data;
	LOGMASKED(LOG_IO, "%s: ioa_buffer_w %04x\n", machine().describe_context(), data);
	m_porta_out(data);
}

u16 generalplus_gpce4_soc_device::ioa_direction_r()
{
	LOGMASKED(LOG_IO, "%s: ioa_direction_r\n", machine().describe_context());
	return m_ioa_direction;
}

void generalplus_gpce4_soc_device::ioa_direction_w(u16 data)
{
	LOGMASKED(LOG_IO, "%s: ioa_direction_w %04x\n", machine().describe_context(), data);
	m_ioa_direction = data;
}

u16 generalplus_gpce4_soc_device::ioa_attribute_r()
{
	LOGMASKED(LOG_IO, "%s: ioa_attribute_r\n", machine().describe_context());
	return m_ioa_attribute;
}

void generalplus_gpce4_soc_device::ioa_attribute_w(u16 data)
{
	m_ioa_attribute = data;
	LOGMASKED(LOG_IO, "%s: ioa_attribute_w %04x\n", machine().describe_context(), data);
}

// IOB

u16 generalplus_gpce4_soc_device::iob_data_r()
{
	LOGMASKED(LOG_IO, "%s: iob_data_r\n", machine().describe_context());
	return m_portb_in();
}

void generalplus_gpce4_soc_device::iob_data_w(u16 data)
{
	LOGMASKED(LOG_IO, "%s: iob_data_w %04x\n", machine().describe_context(), data);
	m_portb_out(data);
}

u16 generalplus_gpce4_soc_device::iob_buffer_r()
{
	LOGMASKED(LOG_IO, "%s: iob_buffer_r\n", machine().describe_context());
	return m_iob_buffer;
}

void generalplus_gpce4_soc_device::iob_buffer_w(u16 data)
{
	m_iob_buffer = data;
	LOGMASKED(LOG_IO, "%s: iob_buffer_w %04x\n", machine().describe_context(), data);
	m_portb_out(data);
}

u16 generalplus_gpce4_soc_device::iob_direction_r()
{
	LOGMASKED(LOG_IO, "%s: iob_direction_r\n", machine().describe_context());
	return m_iob_direction;
}

void generalplus_gpce4_soc_device::iob_direction_w(u16 data)
{
	LOGMASKED(LOG_IO, "%s: iob_direction_w %04x\n", machine().describe_context(), data);
	m_iob_direction = data;
}

void generalplus_gpce4_soc_device::iob_attribute_w(u16 data)
{
	LOGMASKED(LOG_IO, "%s: iob_attribute_w %04x\n", machine().describe_context(), data);
	m_iob_attribute = data;
}

u16 generalplus_gpce4_soc_device::iob_attribute_r()
{
	LOGMASKED(LOG_IO, "%s: iob_attribute_r\n", machine().describe_context());
	return m_iob_attribute;
}

// IOC

u16 generalplus_gpce4_soc_device::ioc_data_r()
{
	LOGMASKED(LOG_IO, "%s: ioc_data_r\n", machine().describe_context());
	return m_portc_in();
}

void generalplus_gpce4_soc_device::ioc_data_w(u16 data)
{
	LOGMASKED(LOG_IO, "%s: ioc_data_w %04x\n", machine().describe_context(), data);
	m_portc_out(data);
}

u16 generalplus_gpce4_soc_device::ioc_buffer_r()
{
	LOGMASKED(LOG_IO, "%s: ioc_buffer_r\n", machine().describe_context());
	return m_portc_in();
}

void generalplus_gpce4_soc_device::ioc_buffer_w(u16 data)
{
	LOGMASKED(LOG_IO, "%s: ioc_buffer_w %04x\n", machine().describe_context(), data);
	m_portc_out(data);
}

u16 generalplus_gpce4_soc_device::ioc_direction_r()
{
	LOGMASKED(LOG_IO, "%s: ioc_direction_r\n", machine().describe_context());
	return m_ioc_direction;
}

void generalplus_gpce4_soc_device::ioc_direction_w(u16 data)
{
	LOGMASKED(LOG_IO, "%s: ioc_direction_w %04x\n", machine().describe_context(), data);
	m_ioc_direction = data;
}

void generalplus_gpce4_soc_device::ioc_attribute_w(u16 data)
{
	LOGMASKED(LOG_IO, "%s: ioc_attribute_w %04x\n", machine().describe_context(), data);
	m_ioc_attribute = data;
}

u16 generalplus_gpce4_soc_device::ioc_attribute_r()
{
	LOGMASKED(LOG_IO, "%s: ioc_attribute_r\n", machine().describe_context());
	return m_ioc_attribute;
}


u16 generalplus_gpce4_soc_device::io_ctrl_r()
{
	LOGMASKED(LOG_IO, "%s: io_ctrl_r\n", machine().describe_context());
	return m_io_ctrl;
}

void generalplus_gpce4_soc_device::io_ctrl_w(u16 data)
{
	m_io_ctrl = data;
	LOGMASKED(LOG_IO, "%s: io_ctrl_w %04x\n", machine().describe_context(), data);
}

// TIMERS

u16 generalplus_gpce4_soc_device::timer_ctrl_r()
{
	LOGMASKED(LOG_TIMERS, "%s: timer_ctrl_r\n", machine().describe_context());
	return m_timer_ctrl;
}

void generalplus_gpce4_soc_device::timer_ctrl_w(u16 data)
{
	m_timer_ctrl = data;
	LOGMASKED(LOG_TIMERS, "%s: timer_ctrl_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpce4_soc_device::timera_data_w(u16 data)
{
	LOGMASKED(LOG_TIMERS, "%s: timera_data_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpce4_soc_device::timera_counter_w(u16 data)
{
	LOGMASKED(LOG_TIMERS, "%s: timera_counter_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpce4_soc_device::timerb_data_w(u16 data)
{
	LOGMASKED(LOG_TIMERS, "%s: timerb_data_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpce4_soc_device::timerb_counter_w(u16 data)
{
	LOGMASKED(LOG_TIMERS, "%s: timerb_counter_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpce4_soc_device::timerc_data_w(u16 data)
{
	LOGMASKED(LOG_TIMERS, "%s: timerc_data_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpce4_soc_device::timerc_counter_w(u16 data)
{
	LOGMASKED(LOG_TIMERS, "%s: timerc_counter_w %04x\n", machine().describe_context(), data);
}

// PWM

void generalplus_gpce4_soc_device::pwm0_ctrl_w(u16 data)
{
	LOGMASKED(LOG_PWM, "%s: pwm0_ctrl_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpce4_soc_device::pwm1_ctrl_w(u16 data)
{
	LOGMASKED(LOG_PWM, "%s: pwm1_ctrl_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpce4_soc_device::pwm2_ctrl_w(u16 data)
{
	LOGMASKED(LOG_PWM, "%s: pwm2_ctrl_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpce4_soc_device::pwm3_ctrl_w(u16 data)
{
	LOGMASKED(LOG_PWM, "%s: pwm3_ctrl_w %04x\n", machine().describe_context(), data);
}

// MISC

void generalplus_gpce4_soc_device::system_clock_w(u16 data)
{
	LOGMASKED(LOG_MISC, "%s: system_clock_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpce4_soc_device::timebase_clear_w(u16 data)
{
	LOGMASKED(LOG_MISC, "%s: timebase_clear_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpce4_soc_device::watchdog_clear_w(u16 data)
{
	// writes a lot of 0x5555 here in a block
	// LOGMASKED(LOG_MISC, "%s: watchdog_clear_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpce4_soc_device::wait_ctrl_w(u16 data)
{
	LOGMASKED(LOG_MISC, "%s: wait_ctrl_w %04x\n", machine().describe_context(), data);
}

// P_Cache_Ctrl
//
// 15  ---
// 14  ---
// 13  ---
// 12  ---
//
// 11  ---
// 10  ---
//  9  ---
//  8  ---

//  7  ---
//  6  ---
//  5  ---
//  4  ---
// 
//  3  ---
//  2  ---
//  1  Cache_Clr
//  0  Cache_En

u16 generalplus_gpce4_soc_device::cache_ctrl_r()
{
	LOGMASKED(LOG_MISC, "%s: cache_ctrl_r\n", machine().describe_context());
	return m_cache_ctrl & ~0x0002;
}

void generalplus_gpce4_soc_device::cache_ctrl_w(u16 data)
{
	LOGMASKED(LOG_MISC, "%s: cache_ctrl_w %04x\n", machine().describe_context(), data);
	m_cache_ctrl = data;
}

// DAC

void generalplus_gpce4_soc_device::dac_ctrl_w(u16 data)
{
	LOGMASKED(LOG_DAC, "%s: dac_ctrl_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpce4_soc_device::dac_cha1_data_w(u16 data)
{
	// LOGMASKED(LOG_DAC, "%s: dac_cha1_data_w %04x\n", machine().describe_context(), data);
	// mapacman only writes 0000 / 7fff / 8000, but is known to have more limited sound than other units
	m_dac->data_w(data);
}

void generalplus_gpce4_soc_device::dac_cha2_data_w(u16 data)
{
	LOGMASKED(LOG_DAC, "%s: dac_cha2_data_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpce4_soc_device::ppam_ctrl_w(u16 data)
{
	LOGMASKED(LOG_MISC, "%s: ppam_ctrl_w %04x\n", machine().describe_context(), data);
}

u16 generalplus_gpce4_soc_device::interrupt_ctrl_r()
{
	return m_interrupt_ctrl;
}

void generalplus_gpce4_soc_device::interrupt_ctrl_w(u16 data)
{
	m_interrupt_ctrl = data;

	// 15 IRQ0_TMA          -- enabled in mapacman
	//    IRQ1_TMB
	//    IRQ2_TMC          -- enabled in mapacman, digicolr
	//    ---
	//    IRQ3_ADC
	//    0
	//    IRQ3_SPIDMA
	//  8 IRQ3_SPI2         -- enabled in mapacman
	//  7 IRQ_SPI
	//    IRQ3_SPI2DMA
	//    IRQ4_USB
	//    IRQ4_KEY
	//    ---
	//    0
	//    IRQ5_EXT1
	//  0 IRQ5_EXT2

	LOGMASKED(LOG_IRQ, "%s: interrupt_ctrl_w %04x\n", machine().describe_context(), data);
}

u16 generalplus_gpce4_soc_device::interrupt2_status_r()
{
	LOGMASKED(LOG_IRQ, "%s: interrupt2_status_r\n", machine().describe_context());
	return m_interrupt_status & 0xffff;
}

void generalplus_gpce4_soc_device::interrupt2_status_w(u16 data)
{
	LOGMASKED(LOG_IRQ, "%s: interrupt2_status_w %04x\n", machine().describe_context(), data);

	if (data & 0x0004)
		clear_interrupt(2);

	if (data & 0x0001)
		clear_interrupt(0);
}

u16 generalplus_gpce4_soc_device::interrupt2_ctrl_r()
{
	return m_interrupt2_ctrl;
}

void generalplus_gpce4_soc_device::interrupt2_ctrl_w(u16 data)
{
	m_interrupt2_ctrl = data;
	// 15  ---
	//     ---
	//     ---
	//     IRQ6_CTSTMA
	//     IRQ6_CTSTMB
	//     IRQ6_4KHz
	//     IRQ6_2KHz     -- enabled in digicolr
	//  8  IRQ6_512Hz
	//  7  ---
	//     ---
	//     IRQ7_STACK
	//     IRQ7_CPURX
	//     ---
	//     IRQ7_64Hz     -- enabled in mapacman, digicolr
	//     IRQ7_16Hz
	//  0  IRQ7_2Hz      -- enabled in digicolr

	LOGMASKED(LOG_IRQ, "%s: interrupt2_ctrl_w %04x\n", machine().describe_context(), data);
}

// SPI1

u16 generalplus_gpce4_soc_device::spi_ctrl_r()
{
	LOGMASKED(LOG_SPI1, "%s: spi_ctrl_r\n", machine().describe_context());
	return machine().rand();
}

void generalplus_gpce4_soc_device::spi_ctrl_w(u16 data)
{
	LOGMASKED(LOG_SPI1, "%s: spi_ctrl_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpce4_soc_device::spi_misc_w(u16 data)
{
	LOGMASKED(LOG_SPI1, "%s: spi_misc_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpce4_soc_device::spi_txstatus_w(u16 data)
{
	LOGMASKED(LOG_SPI1, "%s: spi_txstatus_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpce4_soc_device::spi_txdata_w(u16 data)
{
	LOGMASKED(LOG_SPI1, "%s: spi_txdata_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpce4_soc_device::spi_rxstatus_w(u16 data)
{
	LOGMASKED(LOG_SPI1, "%s: spi_rxstatus_w %04x\n", machine().describe_context(), data);
}

u16 generalplus_gpce4_soc_device::spi_rxdata_r()
{
	LOGMASKED(LOG_SPI1, "%s: spi_rxdata_r\n", machine().describe_context());
	return machine().rand() & 0x01;
}

u16 generalplus_gpce4_soc_device::spi_misc_r()
{
	LOGMASKED(LOG_SPI1, "%s: spi_misc_r\n", machine().describe_context());
	return 0x0000;
}

u16 generalplus_gpce4_soc_device::spi_auto_ctrl_r()
{
	LOGMASKED(LOG_SPI1, "%s: spi_auto_ctrl_r\n", machine().describe_context());
	return m_spi_auto_ctrl;
}

void generalplus_gpce4_soc_device::spi_auto_ctrl_w(u16 data)
{
	LOGMASKED(LOG_SPI1, "%s: spi_auto_ctrl_w %04x\n", machine().describe_context(), data);
	m_spi_auto_ctrl = data;
}

u16 generalplus_gpce4_soc_device::spi_bank_r()
{
	LOGMASKED(LOG_SPI1, "%s: spi_bank_r\n", machine().describe_context());
	return m_spi_bank;
}

void generalplus_gpce4_soc_device::spi_bank_w(u16 data)
{
	LOGMASKED(LOG_SPI1, "%s: spi_bank_w %04x\n", machine().describe_context(), data);
	m_spi_bank = data;
	m_bank->set_entry(data & 0xf);
}

void generalplus_gpce4_soc_device::update_interrupts()
{
	if ((m_interrupt_status & 0x8000'0000) && (m_interrupt_ctrl & 0x8000))
	{
		set_input_line(UNSP_IRQ0_LINE, ASSERT_LINE);
	}
	else
	{
		set_input_line(UNSP_IRQ0_LINE, CLEAR_LINE);
	}

	if ((m_interrupt_status & 0x2000'0000) && (m_interrupt_ctrl & 0x2000))
	{
		set_input_line(UNSP_IRQ2_LINE, ASSERT_LINE);
	}
	else
	{
		set_input_line(UNSP_IRQ2_LINE, CLEAR_LINE);
	}

	if ((m_interrupt_status & 0x0100'0000) && (m_interrupt_ctrl & 0x0100))
	{
		set_input_line(UNSP_IRQ3_LINE, ASSERT_LINE);
	}
	else
	{
		set_input_line(UNSP_IRQ3_LINE, CLEAR_LINE);
	}

	if ((m_interrupt_status & 0x0000'0001) && (m_interrupt2_ctrl & 0x0001))
	{
		set_input_line(UNSP_IRQ7_LINE, ASSERT_LINE);
	}
	else
	{
		set_input_line(UNSP_IRQ7_LINE, CLEAR_LINE);
	}

	if ((m_interrupt_status & 0x0000'0004) && (m_interrupt2_ctrl & 0x0004))
	{
		set_input_line(UNSP_IRQ7_LINE, ASSERT_LINE);
	}
	else
	{
		set_input_line(UNSP_IRQ7_LINE, CLEAR_LINE);
	}
}

void generalplus_gpce4_soc_device::request_interrupt(int which)
{
	int bit = 1 << which;
	m_interrupt_status = (m_interrupt_status & ~bit) | bit;
	update_interrupts();
}

void generalplus_gpce4_soc_device::clear_interrupt(int which)
{
	int bit = 1 << which;
	m_interrupt_status = (m_interrupt_status & ~bit);
	update_interrupts();
}

u16 generalplus_gpce4_soc_device::interrupt_status_r()
{
	LOGMASKED(LOG_IRQ, "%s: interrupt_status_r\n", machine().describe_context());
	return (m_interrupt_status >> 16) & 0xffff;
}

void generalplus_gpce4_soc_device::interrupt_status_w(u16 data)
{
	if (data & 0x8000)
		clear_interrupt(31);

	if (data & 0x2000)
		clear_interrupt(29);

	if (data & 0x0100)
		clear_interrupt(24);
}

u16 generalplus_gpce4_soc_device::fiq_sel_r()
{
	return m_fiq_sel;
}

void generalplus_gpce4_soc_device::fiq_sel_w(u16 data)
{
	// each bit refers to an interrupt source that can be redirected to FIQ instead
	LOGMASKED(LOG_IRQ, "%s: fiq_sel_w %04x\n", machine().describe_context(), data);
	m_fiq_sel = data;
}

u16 generalplus_gpce4_soc_device::fiq2_sel_r()
{
	return m_fiq2_sel;
}

void generalplus_gpce4_soc_device::fiq2_sel_w(u16 data)
{
	// each bit refers to an interrupt source that can be redirected to FIQ instead
	LOGMASKED(LOG_IRQ, "%s: fiq2_sel_w %04x\n", machine().describe_context(), data);
	m_fiq2_sel = data;
}

// P_SPI2_Ctrl
//
// 15  SPIEN
// 14  ---
// 13  LBM
// 12  SPICS_IO
//
// 11  SPIRST
// 10  ---
//  9  ---
//  8  MOD
// 
//  7  ---
//  6  ---
//  5  SCKPHA
//  4  SCKPOL
// 
//  3  ---
//  2  SCKSEL[2]
//  1  SCKSEL[1]
//  0  SCKSEL[0]

u16 generalplus_gpce4_soc_device::spi2_ctrl_r()
{
	return m_spi2_ctrl;
}

void generalplus_gpce4_soc_device::spi2_ctrl_w(u16 data)
{
	LOGMASKED(LOG_SPI2, "%s: spi2_ctrl_w %04x\n", machine().describe_context(), data);
	m_spi2_ctrl = data;
}


u16 generalplus_gpce4_soc_device::spi2_rxstatus_r()
{
	LOGMASKED(LOG_SPI2, "%s: spi2_rxstatus_r\n", machine().describe_context());
	return 0x0000;
}

void generalplus_gpce4_soc_device::spi2_rxstatus_w(u16 data)
{
	LOGMASKED(LOG_SPI2, "%s: spi2_rxstatus_w %04x\n", machine().describe_context(), data);
}

// SPI2_TXSTATUS
//
// 15  SPITXIF
// 14  SPITXIEN
// 13  --
// 12  --
//
// 11  --
// 10  --
//  9  --
//  8  --
// 
//  7  TXFLEV[3]
//  6  TXFLEV[2]
//  5  TXFLEV[1]
//  4  TXFLEX[0]
// 
//  3  TXFFLAG[3]
//  2  TXFFLAG[2]
//  1  TXFFLAG[1]
//  0  TXFFLAG[0]

u16 generalplus_gpce4_soc_device::spi2_txstatus_r()
{
	LOGMASKED(LOG_SPI2, "%s: spi2_txstatus_r\n", machine().describe_context());
	return 0x0000;
}

void generalplus_gpce4_soc_device::spi2_txstatus_w(u16 data)
{
	LOGMASKED(LOG_SPI2, "%s: spi2_txstatus_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpce4_soc_device::spi2_txdata_w(u16 data)
{
	// only 8-bits are written to FIFO
	// the SoC turns this into actual SPI signals
	m_spi2_out(data & 0xff);
}

u16 generalplus_gpce4_soc_device::spi2_rxdata_r()
{
	LOGMASKED(LOG_SPI2, "%s: spi2_rxdata_r\n", machine().describe_context());
	return 0x0000;
}

// P_SPI2_MISC
// 
// 15  ---
// 14  ---
// 13  ---
// 12  ---
// 
// 11  ---
// 10  ---
//  9  OVER
//  8  SMART
// 
//  7  ---
//  6  ---
//  5  ---
//  4  BSY
// 
//  3  RFF
//  2  RNE
//  1  TNF
//  0  TFE

u16 generalplus_gpce4_soc_device::spi2_misc_r()
{
	// loops on bit 0x0010 (BSY) after writing data to 0x3092

	// clrb [3005],12 is used before writing non-pixel data to 0x3092
	// setb [3005],12 is used after reading from 0x3094
	//LOGMASKED(LOG_SPI2, "%s: spi2_misc_r\n", machine().describe_context());
	return 0x0000;
}

void generalplus_gpce4_soc_device::spi2_misc_w(u16 data)
{
	LOGMASKED(LOG_SPI2, "%s: spi2_misc_w %04x\n", machine().describe_context(), data);
}

// ADC

u16 generalplus_gpce4_soc_device::adc_linein_bitctrl_r()
{
	LOGMASKED(LOG_ADC, "%s: adc_linein_bitctrl_r\n", machine().describe_context());
	return 0x0000;
}

void generalplus_gpce4_soc_device::adc_linein_bitctrl_w(u16 data)
{
	LOGMASKED(LOG_ADC, "%s: adc_linein_bitctrl_w %04x\n", machine().describe_context(), data);
}

u16 generalplus_gpce4_soc_device::adc_ctrl_r()
{
	LOGMASKED(LOG_ADC, "%s: adc_ctrl_r\n", machine().describe_context());
	return 0x0000;
}

void generalplus_gpce4_soc_device::adc_ctrl_w(u16 data)
{
	LOGMASKED(LOG_ADC, "%s: adc_ctrl_w %04x\n", machine().describe_context(), data);
}

u16 generalplus_gpce4_soc_device::adc_data_r()
{
	LOGMASKED(LOG_ADC, "%s: adc_data_r\n", machine().describe_context());
	return 0x0000;
}

void generalplus_gpce4_soc_device::adc_data_w(u16 data)
{
	LOGMASKED(LOG_ADC, "%s: adc_data_w %04x\n", machine().describe_context(), data);
}

