// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**************************************************************************************************

  pcfx.cpp

  Driver file to handle emulation of the NEC PC-FX.

**************************************************************************************************/


#include "emu.h"

#include "cpu/v810/v810.h"
#include "machine/nvram.h"
#include "machine/pcfx_intc.h"
#include "sound/huc6230.h"
#include "video/huc6261.h"
#include "video/huc6270.h"
#include "video/huc6271.h"
#include "video/huc6272.h"

#include "screen.h"
#include "speaker.h"
#include "softlist_dev.h"

#include <algorithm>


namespace {

// TODO: should really compose everything as devices
// - pcfxga needs to be exposed as a ISA16 and C-Bus boards, it's not a real stand-alone driver.
// - FX-SCSI requires either a specific NSCSI target with enough boilerplate comms (i.e. no video),
//   or multiple instances of MAME being capable to communicate via bitbanger.
class pcfx_state : public driver_device
{
public:
	pcfx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_huc6261(*this, "huc6261")
		, m_huc6270(*this, "huc6270_%c", 'a')
		, m_intc(*this, "intc")
		, m_pads(*this, "P%u", 1U)
	{ }

	void pcfx(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void int_w(offs_t line, u8 state);
	int bus_wait(offs_t address, int access, int size);
	void nvram_init(nvram_device &nvram, void *data, size_t size);

	template <int Pad> u16 pad_r(offs_t offset);
	template <int Pad> void pad_w(offs_t offset, u16 data);

	template <int Pad> TIMER_CALLBACK_MEMBER(pad_func);
	void pad_irq_update();
	u8 ar_hold(int chip) { const u8 ar = m_huc6270[chip]->get_ar(0); return (ar == 0x13) ? 0x03 : ar; }

	u16 timer_control_r();
	void timer_control_w(u16 data);
	u16 timer_period_r();
	void timer_period_w(u16 data);
	u16 timer_counter_r();
	TIMER_CALLBACK_MEMBER(timer_expired);
	u32 timer_effective_period() const { return (m_timer_period ? m_timer_period : 0x10000) * 15; }

	// the 16-bit video chips have their A1 on CPU A2, so a word access reaches the same register twice, low half first
	// TODO: byte accesses are undefined (HuC6261 manual 2.1.1) and are passed on as halfword accesses
	template <typename Read>
	static u32 read_halves(u32 mem_mask, Read &&read)
	{
		u32 data = 0;
		if (ACCESSING_BITS_0_15)
			data |= read();
		if (ACCESSING_BITS_16_31)
			data |= u32(read()) << 16;
		return data;
	}

	template <typename Write>
	static void write_halves(u32 data, u32 mem_mask, Write &&write)
	{
		if (ACCESSING_BITS_0_15)
			write(u16(data));
		if (ACCESSING_BITS_16_31)
			write(u16(data >> 16));
	}

	void pcfx_io(address_map &map) ATTR_COLD;
	void pcfx_mem(address_map &map) ATTR_COLD;

	struct pcfx_pad_t
	{
		u8 ctrl[2]{};
		u8 status[2]{};
		u32 latch[2]{};
	};

	pcfx_pad_t m_pad;
	emu_timer *m_pad_timers[2]{};

	u16 m_timer_control = 0;
	u16 m_timer_period = 0;
	u16 m_timer_counter = 0;
	emu_timer *m_timer = nullptr;

	u8 m_bram_control = 0;
	std::unique_ptr<u8[]> m_nvram_ptr;

	u32 m_dram_page = 0;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_io;

	required_device<v810_device> m_maincpu;
	required_device<huc6261_device> m_huc6261;
	required_device_array<huc6270_device, 2> m_huc6270;
	required_device<pcfx_intc_device> m_intc;
	required_ioport_array<2> m_pads;
};


template <int Pad>
u16 pcfx_state::pad_r(offs_t offset)
{
	u16 res;

	if (BIT(~offset, 5))
	{
		// status
		/*
		---- x--- END transfer finished
		---- -x-- IOS input
		---- --x- MOD multi-tap clear
		---- ---x TRG transfer in progress
		*/
		res = m_pad.status[Pad] | m_pad.ctrl[Pad];
		//logerror("%s: STATUS %d\n", machine().describe_context(), Pad);
	}
	else
	{
		// received data
		res = m_pad.latch[Pad] >> (BIT(offset, 0) << 4);

		if (BIT(~offset, 0))
		{
			if (!machine().side_effects_disabled())
			{
				m_pad.status[Pad] &= ~8; // clear latch on LSB read according to docs
				pad_irq_update();
			}
		}
	}

	return res;
}

template <int Pad>
TIMER_CALLBACK_MEMBER(pcfx_state::pad_func)
{
	m_pad.latch[Pad] = m_pads[Pad]->read();
	m_pad.status[Pad] |= 8;
	m_pad.ctrl[Pad] &= ~1; // ack TX line
	pad_irq_update();
}

// both ports share one interrupt, raised while either has END set
void pcfx_state::pad_irq_update()
{
	m_intc->irq_w<11>(BIT(m_pad.status[0] | m_pad.status[1], 3));
}

template <int Pad>
void pcfx_state::pad_w(offs_t offset, u16 data)
{
	if (BIT(~offset, 5))
	{
		// control
		/*
		---- -x-- receiver enable
		---- --x- enable multi-tap
		---- ---x enable send (0->1 transition)
		*/
		// only writable while TRG and END are both 0
		if (BIT(m_pad.ctrl[Pad], 0) || BIT(m_pad.status[Pad], 3))
			return;

		if (data & 1)
		{
			m_pad_timers[Pad]->adjust(attotime::from_usec(100)); // TODO: time
		}

		m_pad.ctrl[Pad] = data & 7;
		//logerror("%s: %04x CONTROL %d\n", machine().describe_context(), data, Pad);
	}
	else
	{
		// transmitted data
		//logerror("%s: %04x TX %d\n", machine().describe_context(), data, Pad);
	}
}

/*
 * Timer
 * counts down in units of 15 master clocks, reloads from the period register
 *
 * $f00 control
 * ---- ---- ---- -x-- interrupt request (write 0 to acknowledge)
 * ---- ---- ---- --x- counter enable
 * ---- ---- ---- ---x interrupt enable
 * $f80 period (0 = 65536)
 * $fc0 counter (read only)
 */
u16 pcfx_state::timer_control_r()
{
	return m_timer_control;
}

// TIREQ is set by every borrow; the interrupt is output while TIREQ, TCE and TIEN are all set
void pcfx_state::timer_control_w(u16 data)
{
	if (!BIT(m_timer_control, 1) && BIT(data, 1))
		m_timer->adjust(m_maincpu->clocks_to_attotime(timer_effective_period()));
	else if (BIT(m_timer_control, 1) && !BIT(data, 1))
	{
		// a stopped counter keeps its value
		m_timer_counter = timer_counter_r();
		m_timer->adjust(attotime::never);
	}
	m_timer_control = data & 7;
	m_intc->irq_w<9>((m_timer_control & 7) == 7);
}

u16 pcfx_state::timer_period_r()
{
	return m_timer_period;
}

void pcfx_state::timer_period_w(u16 data)
{
	m_timer_period = data;
}

u16 pcfx_state::timer_counter_r()
{
	if (!BIT(m_timer_control, 1))
		return m_timer_counter;
	const u64 clocks = m_maincpu->attotime_to_clocks(m_timer->remaining());
	return (clocks + 14) / 15;
}

TIMER_CALLBACK_MEMBER(pcfx_state::timer_expired)
{
	m_timer->adjust(m_maincpu->clocks_to_attotime(timer_effective_period()));
	m_timer_control |= 4;
	m_intc->irq_w<9>((m_timer_control & 7) == 7);
}

void pcfx_state::pcfx_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x001fffff).ram();   /* RAM */
	// memory image of the I/O space
	map(0x80000000, 0x807fffff).lrw32(
		NAME([this] (offs_t offset, u32 mem_mask) -> u32 { return m_io.read_dword(offset << 2, mem_mask); }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { m_io.write_dword(offset << 2, data, mem_mask); }));
//  map(0x80700000, 0x807fffff).rom().region("scsi_rom", 0); // EXTIO ROM area
	// data ports of the HuC6261, HuC6270 #0, HuC6270 #1 and HuC6272 for bit-string instructions, read at Axxxxxxx and written at Bxxxxxxx
	map(0xa0000000, 0xafffffff).lr32(
		NAME([this] (offs_t offset, u32 mem_mask) -> u32 { return m_io.read_dword(0x304 + ((offset >> 24) & 3) * 0x100, mem_mask); }));
	map(0xb0000000, 0xbfffffff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { m_io.write_dword(0x304 + ((offset >> 24) & 3) * 0x100, data, mem_mask); }));
	map(0xa0000000, 0xafffffff).nopw();
	map(0xb0000000, 0xbfffffff).lr32(NAME([] () -> u32 { return 0; }));
	// internal backup RAM, read and written only while enabled in C80h bit 0
	map(0xe0000000, 0xe7ffffff).lrw8(
		NAME([this] (offs_t offset) -> u8 {
			if (!BIT(m_bram_control, 0))
				return 0xff;
			return m_nvram_ptr[offset & 0x7fff];
		}),
		NAME([this] (offs_t offset, u8 data) {
			if (!BIT(m_bram_control, 0))
				return;
			m_nvram_ptr[offset & 0x7fff] = data;
		})
	).umask32(0x00ff00ff);
	map(0xe8000000, 0xe9ffffff).noprw();   /* Extended BackUp RAM */
//  map(0xf8000000, 0xf8000007).noprw();   /* PIO, needed by pcfxga for reading backup "RAM" from DOS/V host */
	// the BIOS ROM is mirrored every 1 MB over the whole area
	map(0xf0000000, 0xf00fffff).mirror(0x0ff00000).rom().region("ipl", 0);  /* ROM */
}

void pcfx_state::pcfx_io(address_map &map)
{
	map(0x00000000, 0x0000007f).rw(FUNC(pcfx_state::pad_r<0>), FUNC(pcfx_state::pad_w<0>)); /* PAD */
	map(0x00000080, 0x000000ff).rw(FUNC(pcfx_state::pad_r<1>), FUNC(pcfx_state::pad_w<1>)); /* PAD */
	map(0x00000100, 0x000001ff).w("huc6230", FUNC(huc6230_device::write)).umask32(0x00ff00ff);   /* HuC6230 */
	map(0x00000200, 0x000002ff).m("huc6271", FUNC(huc6271_device::amap));   /* HuC6271 */
	map(0x00000300, 0x000003ff).lrw32(
		NAME([this] (offs_t offset, u32 mem_mask) -> u32 { return read_halves(mem_mask, [this, offset] { return m_huc6261->read(offset); }); }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { write_halves(data, mem_mask, [this, offset] (u16 d) { m_huc6261->write(offset, d); }); }));
	map(0x00000400, 0x000004ff).lrw32(
		NAME([this] (offs_t offset, u32 mem_mask) -> u32 { return read_halves(mem_mask, [this, offset] { return m_huc6270[0]->read16(offset); }); }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { write_halves(data, mem_mask, [this, offset] (u16 d) { m_huc6270[0]->write16(offset, d); }); }));
	map(0x00000500, 0x000005ff).lrw32(
		NAME([this] (offs_t offset, u32 mem_mask) -> u32 { return read_halves(mem_mask, [this, offset] { return m_huc6270[1]->read16(offset); }); }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { write_halves(data, mem_mask, [this, offset] (u16 d) { m_huc6270[1]->write16(offset, d); }); }));
	map(0x00000600, 0x00000607).mirror(0xf8).m("huc6272", FUNC(huc6272_device::amap)); // King
	// HuC6270 address hold registers: the last AR value written, with DVSSR ($13) reading as $03
	map(0x00000c00, 0x00000c00).lr8(NAME([this] () -> u8 { return ar_hold(0); }));
	map(0x00000c40, 0x00000c40).lr8(NAME([this] () -> u8 { return ar_hold(1); }));
	map(0x00000c80, 0x00000c80).lrw8(
		NAME([this] (offs_t offset) { return m_bram_control; }),
		NAME([this] (offs_t offset, u8 data) {
			m_bram_control = data & 3;
			// TODO: bit 1 to FX-BMP
		})
	);
	map(0x00000e00, 0x00000eff).rw(m_intc, FUNC(pcfx_intc_device::read), FUNC(pcfx_intc_device::write)).umask32(0x0000ffff);
	map(0x00000f00, 0x00000f03).mirror(0x3c).rw(FUNC(pcfx_state::timer_control_r), FUNC(pcfx_state::timer_control_w)).umask32(0x0000ffff);
	map(0x00000f80, 0x00000f83).mirror(0x3c).rw(FUNC(pcfx_state::timer_period_r), FUNC(pcfx_state::timer_period_w)).umask32(0x0000ffff);
	map(0x00000fc0, 0x00000fc3).mirror(0x3c).r(FUNC(pcfx_state::timer_counter_r)).umask32(0x0000ffff);
//  map(0x00500000, 0x005000ff) Aurora mirror
//  map(0x00600000, 0x006fffff).r(FUNC(pcfx_state::scsi_ctrl_r)); // SCSI control for EXTIO
//  map(0x00700000, 0x007fffff).rom().region("scsi_rom", 0); // EXTIO ROM area
	map(0x00700000, 0x007fffff).lr8(NAME([] () { return 0; })); // suppress logging
	map(0x80500000, 0x805000ff).noprw(); // pcfxga Kubota/Hudson HuC6273 "Aurora" 3d controller
}


static INPUT_PORTS_START( pcfx )
	/*
	xxxx ---- ---- ---- ID (0xf = 6 button pad, 0xe = multitap, 0xd = mouse, 0 = none)
	*/
	PORT_START("P1")
	PORT_BIT( 0xf0000000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ID pad
	// pcfxga expects latches to go active high on main menu
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("P1 Button I")
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("P1 Button II")
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("P1 Button III")
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("P1 Button IV")
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_BUTTON5 )        PORT_PLAYER(1) PORT_NAME("P1 Button V")
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_BUTTON6 )        PORT_PLAYER(1) PORT_NAME("P1 Button VI")
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_SELECT )         PORT_PLAYER(1) PORT_NAME("P1 Select Button")
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_START1 )         PORT_PLAYER(1) PORT_NAME("P1 RUN Button")
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("P1 Up")
	PORT_BIT( 0x00000200, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("P1 Right")
	PORT_BIT( 0x00000400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("P1 Down")
	PORT_BIT( 0x00000800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("P1 Left")
	PORT_BIT( 0x00001000, IP_ACTIVE_HIGH, IPT_BUTTON7 )        PORT_PLAYER(1) PORT_NAME("P1 Switch 1")
	PORT_BIT( 0x00004000, IP_ACTIVE_HIGH, IPT_BUTTON8 )        PORT_PLAYER(1) PORT_NAME("P1 Switch 2")
	PORT_BIT( 0x0fffa000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0xf0000000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // ID unconnect
	PORT_BIT( 0x0fffffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void pcfx_state::machine_start()
{
	m_pad_timers[0] = timer_alloc(FUNC(pcfx_state::pad_func<0>), this);
	m_pad_timers[1] = timer_alloc(FUNC(pcfx_state::pad_func<1>), this);
	m_timer = timer_alloc(FUNC(pcfx_state::timer_expired), this);
	m_maincpu->space(AS_IO).specific(m_io);

	m_nvram_ptr = make_unique_clear<u8[]>(0x8000);
	subdevice<nvram_device>("nvram")->set_base(&m_nvram_ptr[0], 0x8000);

	save_pointer(NAME(m_nvram_ptr), 0x8000);
	save_item(NAME(m_pad.ctrl));
	save_item(NAME(m_pad.status));
	save_item(NAME(m_pad.latch));
	save_item(NAME(m_bram_control));
	save_item(NAME(m_dram_page));
	save_item(NAME(m_timer_control));
	save_item(NAME(m_timer_period));
	save_item(NAME(m_timer_counter));
}

void pcfx_state::machine_reset()
{
	for (int i = 0; i < 2; i++)
	{
		m_pad.ctrl[i] = 6;
		m_pad.status[i] = 0;
		m_pad.latch[i] = 0;
		m_pad_timers[i]->adjust(attotime::never);
	}
	pad_irq_update();
	m_bram_control = 0;
	m_timer_control = 0;
	m_timer_period = 0xffff;
	m_timer_counter = 0xffff;
	m_timer->adjust(attotime::never);
	m_intc->irq_w<9>(CLEAR_LINE);
}

// default backup RAM contents: an empty, formatted volume, as games refuse an unformatted one.
// The header and FAT bytes are the ones mednafen writes; the signature "\x24\x8a\xdfPCFXSram"
// is the one in the BIOS (v1.00, FFF5C9E4).
void pcfx_state::nvram_init(nvram_device &nvram, void *data, size_t size)
{
	static constexpr u8 HEADER[] =
	{
		0x24, 0x8a, 0xdf, 0x50, 0x43, 0x46, 0x58, 0x53, 0x72, 0x61, 0x6d, 0x80, 0x00, 0x01, 0x01, 0x00,
		0x01, 0x40, 0x00, 0x00, 0x01, 0xf9, 0x03, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	static constexpr u8 FAT[] = { 0xf9, 0xff, 0xff };

	u8 *const bram = static_cast<u8 *>(data);
	std::fill_n(bram, size, 0);
	std::copy(std::begin(HEADER), std::end(HEADER), &bram[0x00]);
	std::copy(std::begin(FAT), std::end(FAT), &bram[0x80]);
}

// bus wait states: the DRAM adds 3 when an access (opcode fetches included) leaves
// the 2 KB page of the previous one, the BIOS ROM 2 a halfword, the video and sound
// chips 4 a read and 2 a write, and writes to the gate array's own I/O registers 4;
// everything but the DRAM is on a 16-bit bus, so a 32-bit access takes two bus cycles
int pcfx_state::bus_wait(offs_t address, int access, int size)
{
	const bool io = access == v810_device::BUS_IO_READ || access == v810_device::BUS_IO_WRITE;
	int wait;

	if (!io && address < 0x01000000)
	{
		const u32 page = address & ~0x7ff;
		wait = (page != m_dram_page) ? 3 : 0;
		m_dram_page = page;
		return wait;
	}

	const bool write = access == v810_device::BUS_WRITE || access == v810_device::BUS_IO_WRITE;
	if (io || (address >= 0x80000000 && address < 0x80800000))
	{
		const offs_t port = address & 0x7fffff;
		// writes to the chips go through a one-entry write buffer, other I/O writes take 4 wait states
		if (port >= 0x100 && port < 0x700)
			wait = write ? 2 : 4;
		else
			wait = write ? 4 : 0;
	}
	else if (address >= 0xf0000000)
		wait = write ? 0 : 2;
	else if (address >= 0xa0000000 && address < 0xb0000000)
		wait = write ? 0 : 4;
	else if (address >= 0xb0000000 && address < 0xc0000000)
		wait = write ? 2 : 0;
	else
		wait = 0;

	if (size == 4)
		wait = wait * 2 + ((access == v810_device::BUS_FETCH) ? 1 : 2);
	return wait;
}

void pcfx_state::int_w(offs_t line, u8 state)
{
	m_maincpu->set_input_line(line, state ? ASSERT_LINE : CLEAR_LINE);
}


void pcfx_state::pcfx(machine_config &config)
{
	V810(config, m_maincpu, XTAL(21'477'272));
	m_maincpu->set_addrmap(AS_PROGRAM, &pcfx_state::pcfx_mem);
	m_maincpu->set_addrmap(AS_IO, &pcfx_state::pcfx_io);
	m_maincpu->set_bus_wait_callback(FUNC(pcfx_state::bus_wait));

	PCFX_INTC(config, m_intc);
	m_intc->int_cb().set(FUNC(pcfx_state::int_w));

	NVRAM(config, "nvram").set_custom_handler(FUNC(pcfx_state::nvram_init));

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_screen_update(m_huc6261, FUNC(huc6261_device::screen_update));
	screen.set_raw(XTAL(21'477'272) * 2, huc6261_device::WPF, 64, 64 + 1024 + 64, huc6261_device::SCREEN_LPF, 18 * 2, (18 + 242) * 2);

	HUC6270(config, m_huc6270[0], XTAL(21'477'272));
	m_huc6270[0]->set_vram_size(0x20000);
	m_huc6270[0]->irq().set(m_intc, FUNC(pcfx_intc_device::irq_w<12>));

	HUC6270(config, m_huc6270[1], XTAL(21'477'272));
	m_huc6270[1]->set_vram_size(0x20000);
	m_huc6270[1]->irq().set(m_intc, FUNC(pcfx_intc_device::irq_w<14>));

	HUC6261(config, m_huc6261, XTAL(21'477'272));
	m_huc6261->set_vdc1_tag(m_huc6270[0]);
	m_huc6261->set_vdc2_tag(m_huc6270[1]);
	m_huc6261->set_king_tag("huc6272");

	huc6272_device &huc6272(HUC6272(config, "huc6272", XTAL(21'477'272)));
	huc6272.irq_changed_callback().set(m_intc, FUNC(pcfx_intc_device::irq_w<13>));
	huc6272.set_rainbow_tag("huc6271");

	HUC6271(config, "huc6271", XTAL(21'477'272));

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	huc6230_device &huc6230(HuC6230(config, "huc6230", XTAL(21'477'272)));
	huc6230.adpcm_update_cb<0>().set("huc6272", FUNC(huc6272_device::adpcm_update_0));
	huc6230.adpcm_update_cb<1>().set("huc6272", FUNC(huc6272_device::adpcm_update_1));
	huc6230.add_route(0, "speaker", 1.0, 0);
	huc6230.add_route(1, "speaker", 1.0, 1);
	huc6272.add_route(0, huc6230, 1.0, 2);
	huc6272.add_route(1, huc6230, 1.0, 3);

	SOFTWARE_LIST(config, "cd_list").set_original("pcfx");
	SOFTWARE_LIST(config, "photocd_list").set_compatible("photo_cd");
}


ROM_START( pcfx )
	ROM_REGION32_LE( 0x100000, "ipl", 0 )
	ROM_SYSTEM_BIOS( 0, "v100", "BIOS v1.00 - 2 Sep 1994" )
	ROMX_LOAD( "pcfxbios.bin", 0x000000, 0x100000, CRC(76ffb97a) SHA1(1a77fd83e337f906aecab27a1604db064cf10074), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v101", "BIOS v1.01 - 5 Dec 1994" )
	ROMX_LOAD( "pcfxv101.bin", 0x000000, 0x100000, CRC(236102c9) SHA1(8b662f7548078be52a871565e19511ccca28c5c8), ROM_BIOS(1) )

	ROM_REGION32_LE( 0x100000, "scsi_rom", ROMREGION_ERASEFF )
	// TODO: "PC-FX EXTIO Boot", really belongs to FX-SCSI PC-FX expansion board
	// r/w to I/O $600000 SCSI if ROM enabled in both memory and I/O areas,
	// allows PC-FX to act as a CD drive for a PC-98 host.
	ROM_LOAD( "fx-scsi.rom", 0x00000, 0x80000, CRC(f3e60e5e) SHA1(65482a23ac5c10a6095aee1db5824cca54ead6e5) )
	ROM_RELOAD( 0x80000, 0x80000 )
ROM_END


ROM_START( pcfxga )
	ROM_REGION32_LE( 0x100000, "ipl", 0 )
	ROM_LOAD( "pcfxga.rom", 0x000000, 0x100000, CRC(41c3776b) SHA1(a9372202a5db302064c994fcda9b24d29bb1b41c) )

	ROM_REGION32_LE( 0x100000, "scsi_rom", ROMREGION_ERASEFF )
ROM_END

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY  FULLNAME                  FLAGS
CONS( 1994, pcfx,   0,      0,      pcfx,    pcfx,  pcfx_state, empty_init, "NEC / Hudson Soft",   "PC-FX",                  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 199?, pcfxga, pcfx,   0,      pcfx,    pcfx,  pcfx_state, empty_init, "NEC / Hudson Soft",   "PC-FX/GA (PC ISA Card)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
