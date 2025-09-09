// license:BSD-3-Clause
// copyright-holders:O. Galibert

// Sord Future 32

// According the the Sord timeline page,
// https://www.sord.co.jp/company/corporate/history.html
// there was:

//   1987: Sord Future 32
//   1988: Sord Future 32 HR
//   1989: Sord Future 32α
//   1991: Sord Future 32α II

// At this point, we only have a dump of the 32α
// Strangely the machine is badged "32α" but the internal strings
// in the prom seem to indicated it dates from 1991.  Maybe a revision,
// maybe a II in disguise...


#include "emu.h"
#include "bus/nscsi/hd.h"
#include "bus/nscsi/cd.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68030.h"
#include "imagedev/floppy.h"
#include "machine/6840ptm.h"
#include "machine/clock.h"
#include "machine/mb87030.h"
#include "machine/upd71071.h"
#include "machine/upd765.h"
#include "machine/z80scc.h"
#include "video/bt45x.h"
#include "video/mc6845.h"
#include "video/upd72120.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

//  HLE of the keyboard, the MCU is not yet dumped (and may not ever be)
class future32_kbd_device : public device_t
{
public:
	static constexpr u32 serial_clock = 9600;
	static constexpr u32 scan_clock = 100;

	future32_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	virtual ~future32_kbd_device() = default;

	auto tx_cb() { return m_tx_cb.bind(); }
	void rx_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	enum { TX_IDLE, TX_SEND };

	devcb_write_line m_tx_cb;
	required_ioport_array<8> m_keys;
	emu_timer *m_tx_timer, *m_rx_timer, *m_scan_timer;

	std::array<u32, 8> m_scan;
	std::array<u8, 64> m_buffer;
	u32 m_buffer_size;

	u32 m_tx_state;
	u16 m_tx;

	TIMER_CALLBACK_MEMBER(timer_rx);
	TIMER_CALLBACK_MEMBER(timer_tx);
	TIMER_CALLBACK_MEMBER(timer_scan);

	void push(u8 data);
	void tx_start();
};

DEFINE_DEVICE_TYPE(FUTURE32_KBD, future32_kbd_device, "future32_kbd", "Sord Future32 keyboard");

future32_kbd_device::future32_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, FUTURE32_KBD, tag, owner, clock)
	, m_tx_cb(*this)
	, m_keys(*this, "P%u", 0U)
{
}

void future32_kbd_device::rx_w(int state)
{
	// Doesn't seem to be used, so not implemented
	logerror("rx %d\n", state);
}

void future32_kbd_device::device_start()
{
	m_tx_timer = timer_alloc(FUNC(future32_kbd_device::timer_tx), this);
	m_rx_timer = timer_alloc(FUNC(future32_kbd_device::timer_rx), this);
	m_scan_timer = timer_alloc(FUNC(future32_kbd_device::timer_scan), this);

	save_item(NAME(m_buffer));
	save_item(NAME(m_buffer_size));
	save_item(NAME(m_tx_state));
	save_item(NAME(m_tx));
	save_item(NAME(m_scan));
}

void future32_kbd_device::device_reset()
{
	std::fill(m_buffer.begin(), m_buffer.end(), 0);
	std::fill(m_scan.begin(), m_scan.end(), 0xffffffff);
	m_buffer_size = 0;
	m_tx_state = TX_IDLE;
	m_tx = 0;

	m_scan_timer->adjust(attotime::from_ticks(1, scan_clock), 0, attotime::from_ticks(1, scan_clock));
}

void future32_kbd_device::push(u8 data)
{
	if(m_buffer_size == m_buffer.size())
		return;
	m_buffer[m_buffer_size++] = data;
	if(m_tx_state == TX_IDLE)
		tx_start();
}

void future32_kbd_device::tx_start()
{
	m_tx_state = TX_SEND;
	m_tx = 0x200 | (m_buffer[0] << 1);
	m_tx_timer->adjust(attotime::from_ticks(1, serial_clock), 0, attotime::from_ticks(1, serial_clock));
	memmove(m_buffer.data(), m_buffer.data()+1, m_buffer.size()-1);
	m_buffer_size --;
}

TIMER_CALLBACK_MEMBER(future32_kbd_device::timer_tx)
{
	m_tx_cb(m_tx & 1);
	m_tx >>= 1;
	if(m_tx)
		return;
	m_tx_state = TX_IDLE;
	m_tx_timer->adjust(attotime::never);
	if(m_buffer_size)
		tx_start();
}

TIMER_CALLBACK_MEMBER(future32_kbd_device::timer_rx)
{
}

TIMER_CALLBACK_MEMBER(future32_kbd_device::timer_scan)
{
	for(u32 i=0; i != 8; i++) {
		u32 state = m_keys[i]->read();
		u32 diff = state ^ m_scan[i];
		m_scan[i] = state;
		if(diff)
			for(u32 j=0; j != 32; j++)
				if(BIT(diff, j)) {
					if(BIT(state, j))
						push((i << 5) | j);         // keyoff
					else
						push((i << 5) | j | 0x80);  // keyon
				}
	}
}


static INPUT_PORTS_START(future32_kbd)
	PORT_START("P0")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('^') PORT_CHAR('~') 
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\')
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_CODE(KEYCODE_DEL)
	PORT_BIT( 0x8000ffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@ `") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; +") PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_COLON) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LF") PORT_CHAR(10)
	PORT_BIT( 0xc000c000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/')

	PORT_BIT( 0xfffffc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P4")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P5")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P6")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P7")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


ioport_constructor future32_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(future32_kbd);
}



class future32a_state : public driver_device
{
public:
	future32a_state(const machine_config &mconfig, device_type type, const char *tag);
	virtual ~future32a_state();

	void future32a(machine_config &config);

protected:
	required_device<m68030_device> m_maincpu;
	required_device<hd6345_device> m_crtc;
	required_device<upd72120_device> m_agdc;
	required_device<upd71071_device> m_dma;
	required_device<scc8530_device> m_scc1;
	required_device<scc8530_device> m_scc2;
	required_device<ptm6840_device> m_ptm;
	required_device<upd72065_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_device<mb89352_device> m_scsi;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<bt451_device> m_ramdac;
	required_device<future32_kbd_device> m_kbd;
	required_shared_ptr<u32> m_textlayer;
	required_shared_ptr<u32> m_fontram;
	required_region_ptr<u8> m_fontrom;

	memory_passthrough_handler m_boot_tap;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void mem(address_map &map);
	MC6845_UPDATE_ROW(crtc_update_row);
	static void floppy_drives(device_slot_interface &device);
	static void scsi_devices(device_slot_interface &device);
	void mb89352(device_t *device);
};

future32a_state::future32a_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, m_maincpu(*this, "maincpu")
	, m_crtc(*this, "crtc")
	, m_agdc(*this, "agdc")
	, m_dma(*this, "dma")	
	, m_scc1(*this, "scc1")
	, m_scc2(*this, "scc2")
	, m_ptm(*this, "ptm")
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:0")
	, m_scsi(*this, "scsi:7:mb89352")
	, m_screen(*this, "screen")
	, m_gfxdecode(*this, "gfxdecode")
	, m_ramdac(*this, "ramdac")
	, m_kbd(*this, "kbd")
	, m_textlayer(*this, "textlayer")
	, m_fontram(*this, "fontram")
	, m_fontrom(*this, "font")
{
}

future32a_state::~future32a_state()
{
}

void future32a_state::mem(address_map &map)
{
	map(0x00000000, 0x003fffff).ram();  // Some magic at startup to see the rom there
	map(0x00400000, 0x009fffff).lr32(NAME([this]() { if(!machine().side_effects_disabled()) m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE); return 0; }));
	map(0x00a00000, 0x00a7ffff).ram().share(m_fontram);

	map(0x00e00000, 0x00e07fff).rom().region("maincpu", 0);
	map(0x00e10000, 0x00e1000f).rw(m_dma, FUNC(upd71071_device::read), FUNC(upd71071_device::write));
	map(0x00e20000, 0x00e20000).lw8(NAME([this](u8 data) { logerror("boot state %02x\n", data); }));
	map(0x00e30000, 0x00e31fff).lw16(NAME([this](offs_t offset, u16 data) { if(offset != data) logerror("%06x: %04x\n", 0xe30000+2*offset, data); }));
	map(0x00e40000, 0x00e4007f).m(m_agdc, FUNC(upd72120_device::map));
	map(0x00e50000, 0x00e50007).m(m_ramdac, FUNC(bt451_device::map)).umask16(0x00ff);
	map(0x00e60000, 0x00e61fff).ram().share(m_textlayer);
	map(0x00e62000, 0x00e6203f).lw16(NAME([this](offs_t offset, u16 data) { if(data != 0) logerror("e62000[%x] = %04x\n", offset, data); }));
	map(0x00e70000, 0x00e7000f).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write)).umask16(0x00ff);
	map(0x00e70201, 0x00e70201).rw(m_crtc, FUNC(hd6345_device::status_r), FUNC(hd6345_device::address_w));
	map(0x00e70203, 0x00e70203).rw(m_crtc, FUNC(hd6345_device::register_r), FUNC(hd6345_device::register_w));
	map(0x00e70301, 0x00e70301).rw(m_scc1, FUNC(scc8530_device::ca_r), FUNC(scc8530_device::ca_w));
	map(0x00e70303, 0x00e70303).rw(m_scc1, FUNC(scc8530_device::da_r), FUNC(scc8530_device::da_w));
	map(0x00e70305, 0x00e70305).rw(m_scc1, FUNC(scc8530_device::cb_r), FUNC(scc8530_device::cb_w));
	map(0x00e70307, 0x00e70307).rw(m_scc1, FUNC(scc8530_device::db_r), FUNC(scc8530_device::db_w));

	map(0x00e70901, 0x00e70901).lr8(NAME([this]() { return machine().rand() & 1 ? 0xc0 : 0x80; })); // beeper here, possibly timer too

	map(0x00e80101, 0x00e80101).rw(m_scc2, FUNC(scc8530_device::ca_r), FUNC(scc8530_device::ca_w));
	map(0x00e80103, 0x00e80103).rw(m_scc2, FUNC(scc8530_device::da_r), FUNC(scc8530_device::da_w));
	map(0x00e80105, 0x00e80105).rw(m_scc2, FUNC(scc8530_device::cb_r), FUNC(scc8530_device::cb_w));
	map(0x00e80107, 0x00e80107).rw(m_scc2, FUNC(scc8530_device::db_r), FUNC(scc8530_device::db_w));

	map(0x00ec0000, 0x00ec000f).m(m_scsi, FUNC(mb89352_device::map));
	map(0x00ec0100, 0x00ec0103).m(m_fdc, FUNC(upd72065_device::map));

	map(0x00ec0400, 0x00ec0403).portr("dips");

	map(0x00f00000, 0x00f0ffff).lr32(NAME([this]() { if(!machine().side_effects_disabled()) m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE); return 0; }));
}

static const gfx_layout textlayer_layout = {
	16, 32,
	1024,
	1,
	{ 0 },
	{ STEP16(0, 1) },
	{ STEP32(0, 16) },
	16*32
};

static GFXDECODE_START( gfx_textlayer )
	GFXDECODE_ENTRY("font", 0, textlayer_layout, 0x100, 1)
GFXDECODE_END

// void name(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t ma, uint8_t ra,
//           uint16_t y, uint8_t x_count, int8_t cursor_x, int de, int hbp, int vbp)

MC6845_UPDATE_ROW(future32a_state::crtc_update_row)
{
	const pen_t *palette = m_ramdac->pens();
	uint32_t *d = &bitmap.pix(y);	
	for(u8 x = 0; x != x_count; x++) {
		u32 code = m_textlayer[(ma+x) & 0x7ff];
		u16 data;
		if(code & 0x8000) {
			data = 0x5555;
		} else {
			offs_t base = (code & 0x3ff)*64 + ra * 2;
			data = (m_fontrom[base] << 8) | m_fontrom[base + 1];
		}
		if(x == cursor_x)
			data = data ^ 0xffff;
		for(int xx=0; xx != 16; xx++)
			*d++ = BIT(data, 15-xx) ? palette[0x103] : palette[0];
	}
}


void future32a_state::machine_start()
{
}

void future32a_state::machine_reset()
{
	m_boot_tap = m_maincpu->space(AS_PROGRAM)
		.install_read_tap(0x00000000, 0x00007fff, "boot",
						  [this](offs_t address, u32 &data, u32 mask) {
							  data = m_maincpu->space(AS_PROGRAM).read_dword(0xe00000 + address, mask);
							  if(address == 4 && mask == 0x0000ffff && !machine().side_effects_disabled())
								  m_boot_tap.remove();
						  });
}

void future32a_state::floppy_drives(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD);
}

void future32a_state::scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("cdrom", NSCSI_CDROM);
	device.option_add_internal("mb89352", MB89352);
}

void future32a_state::mb89352(device_t *device)
{
	mb89352_device &adapter = downcast<mb89352_device &>(*device);
	adapter.set_clock(32000000/4);
	adapter.out_dreq_callback().set([this](int state) { m_dma->dmarq(state, 1); });
}

void future32a_state::future32a(machine_config &config)
{
	// xtals are 50, 32 and 47.843
	// all values guessed

	M68030(config, m_maincpu, 50_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &future32a_state::mem);

	UPD71071(config, m_dma, 0);
	m_dma->set_cpu_tag(m_maincpu->tag());
	m_dma->set_clock((50_MHz_XTAL / 5).value());
	m_dma->dma_read_callback<1>().set(m_scsi, FUNC(mb89352_device::dma_r));
	m_dma->dma_write_callback<1>().set(m_scsi, FUNC(mb89352_device::dma_w));

	UPD72120(config, m_agdc, 32_MHz_XTAL / 2);

	PTM6840(config, m_ptm, 50_MHz_XTAL / 20);

	SCC8530(config, m_scc1, 32_MHz_XTAL / 8);
	m_scc1->configure_channels(9600*16, 9600*16, 0, 0);

	SCC8530(config, m_scc2, 32_MHz_XTAL / 8);
	m_scc2->configure_channels(9600*16, 9600*16, 9600*16, 9600*16);

	FUTURE32_KBD(config, m_kbd);
	m_kbd->tx_cb().set(m_scc1, FUNC(scc8530_device::rxa_w));
	m_scc1->out_txda_callback().set(m_kbd, FUNC(future32_kbd_device::rx_w));

	rs232_port_device &port(RS232_PORT(config, "serial", default_rs232_devices, nullptr));
	port.cts_handler().set(m_scc1, FUNC(scc8530_device::ctsb_w));
	port.dcd_handler().set(m_scc1, FUNC(scc8530_device::dcdb_w));
	port.rxd_handler().set(m_scc1, FUNC(scc8530_device::rxb_w));
	m_scc1->out_rtsb_callback().set(port, FUNC(rs232_port_device::write_rts));
	m_scc1->out_txdb_callback().set(port, FUNC(rs232_port_device::write_txd));

	HD6345(config, m_crtc, 50_MHz_XTAL / 10);
	m_crtc->set_show_border_area(false);
	m_crtc->set_screen(m_screen);
	m_crtc->set_char_width(16);
	m_crtc->set_update_row_callback(FUNC(future32a_state::crtc_update_row));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_physical_aspect(4, 3);
	m_screen->set_screen_update(m_crtc, FUNC(hd6345_device::screen_update));
	m_screen->set_raw(50_MHz_XTAL / 10 * 16, 1664, 0, 1279, 815, 0, 749);

	BT451(config, m_ramdac, 0);
	GFXDECODE(config, m_gfxdecode, m_ramdac, gfx_textlayer); // Only for F4 use

	UPD72065(config, m_fdc, 32_MHz_XTAL / 8);
	FLOPPY_CONNECTOR(config, m_floppy, floppy_drives, "35hd", floppy_image_device::default_mfm_floppy_formats);

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7", scsi_devices, "mb89352", true).set_option_machine_config("mb89352", [this] (device_t *device) { mb89352(device); });
}

static INPUT_PORTS_START(future32a)
	PORT_START("dips")
	PORT_DIPNAME( 0xf0000000, 0x00000000, "Boot mode")
	PORT_DIPSETTING( 0x00000000, "Normal" )
	PORT_DIPSETTING( 0x40000000, "Alternative" )
	PORT_DIPSETTING( 0xc0000000, "Forth console" )
	PORT_DIPSETTING( 0xe0000000, "Serial" )
INPUT_PORTS_END

ROM_START(future32a)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("ft2o1c.bin", 0x0000, 0x8000, CRC(82f9c0b0) SHA1(e30a350cc19edbf623fa37aa60f0215188cc55d6))

	ROM_REGION(0x10000, "font", 0)
	ROM_LOAD16_BYTE("ft2fe00a.u42", 0x0000, 0x8000, CRC(1fce9667) SHA1(ac4955afd9eb9401079c5e7ca8bf65de5bb826ab))
	ROM_LOAD16_BYTE("ft2fo00a.u43", 0x0001, 0x8000, CRC(26a708e2) SHA1(d53f1a2e368fa1d231b3989577129ffadcfda5aa))
ROM_END


COMP(1989, future32a, 0, 0, future32a, future32a, future32a_state, empty_init, "Sord", "Future 32α", MACHINE_NOT_WORKING|MACHINE_SUPPORTS_SAVE)

