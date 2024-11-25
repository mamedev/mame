// license:BSD-3-Clause
// copyright-holders:Melissa Goad
/***************************************************************************

  iphone2g.cpp

  Driver file to handle emulation of the original iPhone

***************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "machine/vic_pl192.h"
#include "screen.h"

class iphone2g_spi_device : public device_t, public device_memory_interface
{
public:
	iphone2g_spi_device(const machine_config &mconfig, const char* tag, device_t *owner, uint32_t clock = 0);

	auto out_irq_cb() { return m_out_irq_func.bind(); }

	void map(address_map &map) ATTR_COLD;

protected:
	iphone2g_spi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

	TIMER_CALLBACK_MEMBER(send_irq);

private:
	address_space_config m_mmio_config;

	devcb_write_line m_out_irq_func;

	emu_timer *m_irq_timer;
	u8 m_cmd = 0;
	u8 m_tx_data = 0;
	u32 m_ctrl = 0;
	u16 m_status = 0;
};

DECLARE_DEVICE_TYPE(IPHONE2G_SPI, iphone2g_spi_device)

TIMER_CALLBACK_MEMBER(iphone2g_spi_device::send_irq)
{
	m_out_irq_func(1);
}

void iphone2g_spi_device::map(address_map &map)
{
	map(0x00,0x03).lrw32(NAME([this](offs_t offset){ return m_ctrl; }), NAME([this](offs_t offset, u32 data){
		if(data & 1)
		{
			m_status |= 0xfff2;
			m_cmd = m_tx_data;
			m_irq_timer->adjust(attotime::from_hz(1'000));
		}
		m_ctrl = data;
	}));
	map(0x08, 0x09).lr16([this](offs_t offset){ return m_status; }, "status").umask32(0x0000ffff);
	map(0x10, 0x10).lrw8(NAME([this](offs_t offset){ return m_tx_data; }), NAME([this](offs_t offset, u8 data){ m_tx_data = data; })).umask32(0x000000ff);
	map(0x20, 0x20).lr8([this](offs_t offset){
		// FIXME: make this less hacky
		switch(m_cmd)
		{
			case 0x95: return 0x01;
			case 0xda: return 0x71;
			case 0xdb: return 0xc2;
			case 0xdc: return 0x00;
		}
		return 0;
	}, "rx_data");
}

device_memory_interface::space_config_vector iphone2g_spi_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_mmio_config)
	};
}

void iphone2g_spi_device::device_start()
{
	save_item(NAME(m_cmd));
	save_item(NAME(m_ctrl));
	save_item(NAME(m_tx_data));
	save_item(NAME(m_status));

	m_irq_timer = timer_alloc(FUNC(iphone2g_spi_device::send_irq), this);
}

void iphone2g_spi_device::device_reset()
{
	m_cmd = m_ctrl= m_status = m_tx_data = 0;
}

DEFINE_DEVICE_TYPE(IPHONE2G_SPI, iphone2g_spi_device, "iphone2g_spi", "iPhone 2G SPI controller")

iphone2g_spi_device::iphone2g_spi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_mmio_config("mmio", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_out_irq_func(*this)
{
}

iphone2g_spi_device::iphone2g_spi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: iphone2g_spi_device(mconfig, IPHONE2G_SPI, tag, owner, clock)
{
}

class iphone2g_timer_device : public device_t, public device_memory_interface
{
public:
	iphone2g_timer_device(const machine_config &mconfig, const char* tag, device_t *owner, uint32_t clock = 0);

	auto out_irq_cb() { return m_out_irq_func.bind(); }

	void map(address_map &map) ATTR_COLD;
	void timer_map(address_map &map) ATTR_COLD;

protected:
	iphone2g_timer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

	TIMER_CALLBACK_MEMBER(send_irq);

private:
	address_space_config m_mmio_config;

	devcb_write_line m_out_irq_func;

	emu_timer *m_irq_timer;

	struct timer
	{
		u16 config = 0;
		u8 state = 0;
		u32 count_buffer[2]{}, count = 0;
	} timers[7];

	//u64 m_ticks = 0;   not used
};

DECLARE_DEVICE_TYPE(IPHONE2G_TIMER, iphone2g_timer_device)

TIMER_CALLBACK_MEMBER(iphone2g_timer_device::send_irq)
{
	m_out_irq_func(1);
}

void iphone2g_timer_device::map(address_map &map)
{
}

device_memory_interface::space_config_vector iphone2g_timer_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_mmio_config)
	};
}

void iphone2g_timer_device::device_start()
{
	m_irq_timer = timer_alloc(FUNC(iphone2g_timer_device::send_irq), this);
}

void iphone2g_timer_device::device_reset()
{
}

DEFINE_DEVICE_TYPE(IPHONE2G_TIMER, iphone2g_timer_device, "iphone2g_timer", "iPhone 2G timers")

iphone2g_timer_device::iphone2g_timer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_mmio_config("mmio", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_out_irq_func(*this)
{
}

iphone2g_timer_device::iphone2g_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: iphone2g_timer_device(mconfig, IPHONE2G_TIMER, tag, owner, clock)
{
}


namespace {

class iphone2g_state : public driver_device
{
public:
	iphone2g_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vic0(*this, "vic0"),
		m_vic1(*this, "vic1"),
		m_spi(*this, {"spi0", "spi1", "spi2"}),
		m_timers(*this, "timers"),
		m_ram(*this, "ram"),
		m_bios(*this, "bios"),
		m_screen(*this, "screen")
	{ }

	void iphone2g(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t clock1_r(offs_t offset);

private:
	required_device<cpu_device> m_maincpu;
	required_device<vic_pl192_device> m_vic0;
	required_device<vic_pl192_device> m_vic1;
	required_device_array<iphone2g_spi_device, 3> m_spi;
	required_device<iphone2g_timer_device> m_timers;
	optional_shared_ptr<uint32_t> m_ram;
	required_region_ptr<uint32_t> m_bios;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map) ATTR_COLD;
};

uint32_t iphone2g_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

uint32_t iphone2g_state::clock1_r(offs_t offset)
{
	uint32_t ret = 0;
	switch(offset)
	{
		case 0x40/4: ret = 1; break; //boot rom needs this to not infinite loop at startup.
	}

	logerror("%s: Clock1 read: offset %08x data %08x\n", machine().describe_context(), offset << 2, ret);

	return ret;
}

void iphone2g_state::mem_map(address_map &map)
{
	map(0x00000000, 0x0000ffff).mirror(0x20000000).rom().region("bios", 0);            /* BIOS */
	map(0x22000000, 0x224fffff).ram();                                                 /* SRAM */
	map(0x38e00000, 0x38e00fff).m(m_vic0, FUNC(vic_pl192_device::map));
	map(0x38e01000, 0x38e01fff).m(m_vic1, FUNC(vic_pl192_device::map));
	map(0x3c300000, 0x3c3000ff).m(m_spi[0], FUNC(iphone2g_spi_device::map));
	map(0x3c500000, 0x3c500fff).r(FUNC(iphone2g_state::clock1_r)).nopw();
	map(0x3ce00000, 0x3ce000ff).m(m_spi[1], FUNC(iphone2g_spi_device::map));
	map(0x3d200000, 0x3d2000ff).m(m_spi[2], FUNC(iphone2g_spi_device::map));
}

void iphone2g_state::machine_start()
{
	//std::copy_n(m_bios.target(), m_bios.length(), m_ram.target());
}

void iphone2g_state::machine_reset()
{
}

void iphone2g_state::iphone2g(machine_config &config)
{
	/* Basic machine hardware */
	ARM1176JZF_S(config, m_maincpu, XTAL(12'000'000) * 103 / 3); //412 MHz, downclocked from 600 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &iphone2g_state::mem_map);

	PL192_VIC(config, m_vic0);
	m_vic0->out_irq_cb().set_inputline("maincpu", arm7_cpu_device::ARM7_IRQ_LINE);
	m_vic0->out_fiq_cb().set_inputline("maincpu", arm7_cpu_device::ARM7_FIRQ_LINE);

	IPHONE2G_SPI(config, m_spi[0], XTAL(12'000'000));
	m_spi[0]->out_irq_cb().set(m_vic0, FUNC(vic_pl192_device::irq_w<0x09>));

	IPHONE2G_SPI(config, m_spi[1], XTAL(12'000'000));
	m_spi[1]->out_irq_cb().set(m_vic0, FUNC(vic_pl192_device::irq_w<0x0a>));

	IPHONE2G_SPI(config, m_spi[2], XTAL(12'000'000));
	m_spi[2]->out_irq_cb().set(m_vic0, FUNC(vic_pl192_device::irq_w<0x0b>));

	IPHONE2G_TIMER(config, m_timers, XTAL(12'000'000));
	m_timers->out_irq_cb().set(m_vic0, FUNC(vic_pl192_device::irq_w<0x07>));

	PL192_VIC(config, m_vic1);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(12'000'000), 320, 0, 320, 480, 0, 480); //Complete guess
	m_screen->set_screen_update(FUNC(iphone2g_state::screen_update));
}

ROM_START(iphone2g)
	ROM_REGION32_LE(0x10000, "bios", 0)
	ROM_LOAD("s5l8900-bootrom.bin", 0x00000, 0x10000, CRC(beb15cd1) SHA1(079a3acab577eb52cc349ea811af3cbd5d01b8f5))
ROM_END

} // anonymous namespace


/*    YEAR  NAME     PARENT  COMPAT  MACHINE     INPUT   STATE       INIT        COMPANY            FULLNAME      FLAGS */
// console section
CONS( 2007, iphone2g, 0, 0, iphone2g, 0, iphone2g_state, empty_init, "Apple", "iPhone (A1203)", MACHINE_IS_SKELETON )
