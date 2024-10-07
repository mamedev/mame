// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Micro Technology Unlimited MTU-130

// Silently boots to floppy, so nothing will happen if you don't have
// a bootable disk in the drive.

// The backplane has 5 slots, one if which is used by the main cpu
// card, the Monomeg, and one by the FDC card.

// Unimplemented:
// - MTUTAPE, a kind of digital tape?
// - MTUNET, some proprietary network
// - Sound on user via cb2, it's weird

// Implemented extension boards:
// - DATAMOVER, a 68k-based board, used by BASIC 1.5 to accelerate floating point operations

// Unimplemented extension boards:
// - PROGRAMMOVER, a z80-based board
// - MultI-O, an i/o board

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/floppy.h"
#include "imagedev/cartrom.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "machine/mos6551.h"
#include "machine/upd765.h"
#include "bus/rs232/rs232.h"
#include "bus/mtu130/board.h"
#include "bus/mtu130/datamover.h"
#include "sound/dac.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


class mtu130_rom_device : public device_t, public device_rom_image_interface {
public:
	mtu130_rom_device(const machine_config &mconfig, char const *tag, device_t *owner, uint32_t clock);
	template <typename T> mtu130_rom_device(const machine_config &mconfig, char const *tag, device_t *owner, T &&romdata_tag, offs_t load_offset) :
		mtu130_rom_device(mconfig, tag, owner, 0)
	{
		m_load_offset = load_offset;
		m_romdata.set_tag(std::forward<T>(romdata_tag));
	}

	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "rom,bin"; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

private:
	required_shared_ptr<u8> m_romdata;
	offs_t m_load_offset;
};

class mtu130_state : public driver_device
{
public:
	mtu130_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_user_6522(*this, "user_6522"),
		m_sys1_6522(*this, "sys1_6522"),
		m_sys2_6522(*this, "sys2_6522"),
		m_acia(*this, "acia"),
		m_irq_merger(*this, "irq_merger"),
		m_rs232(*this, "rs232"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%d", 0U),
		m_ext(*this, "ext%d", 0U),
		m_roms(*this, "rom%d", 0U),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_dac(*this, "dac"),
		m_speaker(*this, "mono"),
		m_io_view(*this, "io_view"),
		m_se_view(*this, "sw_view"),
		m_rom_view(*this, "rom_view"),
		m_rof_view(*this, "rof_view"),
		m_mainram(*this, "mainram"),
		m_fdcram(*this, "fdcram"),
		m_videoram(*this, "videoram"),
		m_romdata(*this, "romdata"),
		m_ipl(*this, "ipl"),
		m_sequencer(*this, "sequencer"),
		m_id(*this, "id"),
		m_keyboard(*this, "K%X", 0L),
		m_keyboard_meta(*this, "KM"),
		m_jumpers(*this, "jumpers"),
		m_lightpen_w(*this, "lightpen_w"),
		m_lightpen_x(*this, "lightpen_x"),
		m_lightpen_y(*this, "lightpen_y")
	{ }

	void mtu130(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(nmi_w);
	DECLARE_INPUT_CHANGED_MEMBER(reset_w);
	DECLARE_INPUT_CHANGED_MEMBER(break_w);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	class memory_interface : public m6502_device::memory_interface {
	public:
		u8 m_sadr, m_write_count, m_banks;

		memory_access<18, 0, 0, ENDIANNESS_LITTLE>::specific m_program;

		const u8 *m_sequencer;

		memory_interface(const u8 *sequencer, address_space &space);
		virtual ~memory_interface() = default;
		virtual uint8_t read(uint16_t adr) override;
		virtual uint8_t read_sync(uint16_t adr) override;
		virtual uint8_t read_arg(uint16_t adr) override;
		virtual void write(uint16_t adr, uint8_t val) override;

		std::pair<u32, u8> normal_step(u16 adr);
		u32 banked_address(u16 adr, u8 sadr) const;

		void set_banks(u8 pbank, u8 dbank) { m_banks = (dbank << 2) | pbank; }
	};

	required_device<m6502_device> m_maincpu;
	required_device<via6522_device> m_user_6522;
	required_device<via6522_device> m_sys1_6522;
	required_device<via6522_device> m_sys2_6522;
	required_device<mos6551_device> m_acia;
	required_device<input_merger_device> m_irq_merger;
	required_device<rs232_port_device> m_rs232;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device_array<mtu130_extension_device, 3> m_ext;
	required_device_array<mtu130_rom_device, 4> m_roms;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<dac_byte_interface> m_dac;
	required_device<speaker_device> m_speaker;

	memory_view m_io_view;
	memory_view m_se_view;
	memory_view m_rom_view;
	memory_view m_rof_view;
	required_shared_ptr<u8> m_mainram;
	required_shared_ptr<u8> m_fdcram;
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_romdata;

	required_memory_region m_ipl;
	required_region_ptr<u8> m_sequencer;
	required_region_ptr<u8> m_id;
	required_ioport_array<16> m_keyboard;
	required_ioport m_keyboard_meta;
	required_ioport m_jumpers;
	required_ioport m_lightpen_w;
	required_ioport m_lightpen_x;
	required_ioport m_lightpen_y;

	memory_interface *m_maincpu_intf;

	emu_timer *m_timer_lightpen_hit;

	int m_lightpen_hit_x, m_lightpen_hit_y;

	uint16_t m_dma_adr;
	u8 m_keyboard_col;
	u8 m_dac_level;
	u8 m_id_adr;

	u8 m_lightpen_status, m_lightpen_low, m_lightpen_high;

	bool m_dma_direction;
	bool m_video_unblank;
	bool m_video_bw;
	bool m_fdc_irq_enabled;

	static void floppies(device_slot_interface &device);
	void map(address_map &map) ATTR_COLD;
	void extension_board(machine_config &config, int slot_id, const char *tag, const char *def);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u8 fdc_ctrl_r();
	void fdc_ctrl_w(u8 data);
	void fdc_irq_w(int state);
	void dma_adr_w(u8 data);
	void dma_drq_w(int state);

	TIMER_CALLBACK_MEMBER(lightpen_hit);
	void lightpen_trigger(int state);
	u8 lightpen_status_r();
	u8 lightpen_low_r();
	u8 lightpen_high_r();
	void lightpen_clear_w(u8);

	void tape_w(u8);

	void keyboard_col_clear_w(u8);
	void user_cb2_w(int line);
	void sys1_pb_w(u8 data);
	u8 sys1_pa_r();
	void sys1_ca2_w(int line);
	void sys2_pa_w(u8 data);

	void id_reset_w(u8);
	u8 id_r();

	void io_enable_w(u8);
	void io_disable_w(u8);
};

DEFINE_DEVICE_TYPE(MTU130_ROM, mtu130_rom_device, "mtu130_rom", "MTU130 rom slot")

	mtu130_rom_device::mtu130_rom_device(const machine_config &mconfig, char const *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MTU130_ROM, tag, owner, clock),
	device_rom_image_interface(mconfig, *this),
	m_romdata(*this, finder_base::DUMMY_TAG),
	m_load_offset(0)
{
}

void mtu130_rom_device::device_start()
{
	if(!exists())
		memset(m_romdata + m_load_offset, 0xff, 4096);
}

std::pair<std::error_condition, std::string> mtu130_rom_device::call_load()
{
	u32 len = !loaded_through_softlist() ? length() : get_software_region_length("rom");
	if(!len || len > 4096 || (4096 % len))
		return std::make_pair(image_error::INVALIDLENGTH, std::string());

	if (!loaded_through_softlist())
		fread(m_romdata + m_load_offset, len);
	else
		memcpy(m_romdata + m_load_offset, get_software_region("rom"), len);

	if(len < 4096) {
		offs_t delta = len;
		while(delta < 4096) {
			memcpy(m_romdata + m_load_offset + delta, m_romdata + m_load_offset, len);
			delta += len;
		}
	}

	return std::make_pair(std::error_condition(), std::string());
}

void mtu130_rom_device::call_unload()
{
	memset(m_romdata + m_load_offset, 0xff, 4096);
}

mtu130_state::memory_interface::memory_interface(const u8 *sequencer, address_space &space) : m_sadr(0), m_write_count(0), m_banks(0), m_sequencer(sequencer)
{
	space.specific(m_program);
}

u32 mtu130_state::memory_interface::banked_address(u16 adr, u8 sadr) const
{
	u32 cur_bank_slot = m_sequencer[sadr] & 3;
	u32 cur_bank = (m_banks >> (cur_bank_slot << 1)) & 3;
	return adr | (cur_bank << 16);
}

u8 mtu130_state::memory_interface::read_sync(u16 adr)
{
	m_write_count = 0;

	u8 sadr = m_sadr & 0xf0; // Cycle counter to zero
	if(adr >= 0x200)
		sadr |= 0x08;

	u8 data = m_program.read_byte(banked_address(adr, sadr));

	sadr &= 0x8f;

	// All of that happens after the read
	if((data & 0x0f) != 0x0c && (data & 0x0f) != 0x0d && (data & 0x0f) != 0x0e && (data & 0x17) != 0x01)
		sadr |= 0x10;

	if((data & 0x0f) != 0x01)
		sadr |= 0x20;

	if(adr >= 0x200)
		sadr |= 0x40;

	// rti recognition
	if(data == 0x40)
		sadr &= ~0x80;

	m_sadr = sadr;

	return data;
}

std::pair<u32, u8> mtu130_state::memory_interface::normal_step(u16 adr)
{
	u8 sadr = m_sadr;
	if(adr >= 0x200)
		sadr |= 0x08;
	else
		sadr &= ~0x08;

	u32 badr = banked_address(adr, sadr);

	sadr = ((sadr + 1) & 7) | (sadr & 0xf8);
	return std::make_pair(badr, sadr);
}

u8 mtu130_state::memory_interface::read(u16 adr)
{
	auto [badr, sadr] = normal_step(adr);
	u8 data = m_program.read_byte(badr);
	m_write_count = 0;
	m_sadr = sadr;
	return data;
}

u8 mtu130_state::memory_interface::read_arg(u16 adr)
{
	return read(adr);
}

void mtu130_state::memory_interface::write(u16 adr, u8 val)
{
	auto [badr, sadr] = normal_step(adr);
	m_program.write_byte(badr, val);
	m_write_count ++;
	if(m_write_count == 3)
		sadr |= 0x80;
	m_sadr = sadr;
}

void mtu130_state::machine_start()
{
	auto intf = std::make_unique<memory_interface>(m_sequencer, m_maincpu->space(AS_PROGRAM));
	m_maincpu_intf = intf.get();

	save_item(NAME(intf->m_sadr));
	save_item(NAME(intf->m_write_count));

	m_maincpu->set_custom_memory_interface(std::move(intf));

	m_dma_adr = 0;
	m_dma_direction = false;
	m_keyboard_col = 0;
	m_dac_level = 0x80;
	m_id_adr = 0;
	m_video_unblank = true;
	m_video_bw = true;
	m_fdc_irq_enabled = false;
	m_lightpen_hit_x = 0;
	m_lightpen_hit_y = 0;
	m_lightpen_status = 0;
	m_lightpen_low = 0;
	m_lightpen_high = 0;

	save_item(NAME(m_dma_adr));
	save_item(NAME(m_dma_direction));
	save_item(NAME(m_keyboard_col));
	save_item(NAME(m_dac_level));
	save_item(NAME(m_id_adr));
	save_item(NAME(m_video_unblank));
	save_item(NAME(m_video_bw));
	save_item(NAME(m_fdc_irq_enabled));
	save_item(NAME(m_lightpen_hit_x));
	save_item(NAME(m_lightpen_hit_y));
	save_item(NAME(m_lightpen_status));
	save_item(NAME(m_lightpen_low));
	save_item(NAME(m_lightpen_high));

	m_timer_lightpen_hit = timer_alloc(FUNC(mtu130_state::lightpen_hit), this);

	m_fdc->set_rate(500000);
	m_io_view.select(1);
	m_rom_view.disable();
	m_rof_view.disable();

	for(auto e : m_ext)
		e->map_io(m_io_view[1]);
}

void mtu130_state::machine_reset()
{
	m_se_view.select(m_jumpers->read() & 0x01);
}

void mtu130_state::video_start()
{
	m_palette->set_pen_color(0,   0,   0,   0);
	m_palette->set_pen_color(1,  85,  85,  85);
	m_palette->set_pen_color(2, 170, 170, 170);
	m_palette->set_pen_color(3, 255, 255, 255);
}

uint32_t mtu130_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if(m_video_unblank && m_video_bw) {
		int x0 = cliprect.left() - 120;
		int x1 = cliprect.right() - 120;
		for(int y = cliprect.top(); y <= cliprect.bottom(); y++) {
			const u8 *src = m_videoram + 60 * y + (x0 >> 3);
			u16 *dest = &bitmap.pix(y, cliprect.left());
			u8 v = 0;
			if(x0 & 7)
				v = *src++;
			for(int x = x0; x <= x1; x++) {
				if(!(x & 7))
					v = *src++;
				*dest++ = ((v >> (7 - (x & 7))) & 1) ? 3 : 0;
			}
		}

	} else if(m_video_unblank) {
		int x0 = cliprect.left() - 120;
		int x1 = cliprect.right() - 120;
		for(int y = cliprect.top(); y <= cliprect.bottom(); y++) {
			const u8 *src = m_videoram + 60 * y + (x0 >> 4);
			u16 *dest = &bitmap.pix(y, cliprect.left());
			u8 v = 0;
			if(x0 & 7)
				v = *src++;
			for(int x = x0; x <= x1; x++) {
				if(!(x & 7))
					v = *src++;
				*dest++ = (v >> (6 - 2*((x >> 1) & 3))) & 3;
			}
		}

	} else
		bitmap.fill(0, cliprect);

	return 0;
}

u8 mtu130_state::fdc_ctrl_r()
{
	return m_fdc->get_irq() ? 0x00 : 0x80;
}

void mtu130_state::fdc_ctrl_w(u8 data)
{
	m_dma_direction = data & 1;
	if(data & 2)
		m_rof_view.select(1);
	else
		m_rof_view.disable();
	m_fdc_irq_enabled = data & 4;
	m_irq_merger->in_w<0>(m_fdc_irq_enabled && m_fdc->get_irq());
}

void mtu130_state::fdc_irq_w(int state)
{
	m_irq_merger->in_w<0>(m_fdc_irq_enabled && state);
}

void mtu130_state::dma_adr_w(u8 data)
{
	m_dma_adr = data << 6;
}

void mtu130_state::dma_drq_w(int state)
{
	while(m_fdc->get_drq()) {
		if(m_dma_direction) {
		// Read from floppy
			u8 data = m_fdc->dma_r();
			m_fdcram[m_dma_adr & 0x3fff] = data;
			m_dma_adr ++;

		} else {
			// Write to floppy
			u8 data = m_fdcram[m_dma_adr & 0x3fff];
			m_fdc->dma_w(data);
			m_dma_adr ++;
		}
	}
}

void mtu130_state::id_reset_w(u8)
{
	m_id_adr = 0;
}

u8 mtu130_state::id_r()
{
	m_id_adr = (m_id_adr + 1) & 0xf;
	return m_id[m_id_adr];
}

void mtu130_state::user_cb2_w(int line)
{
	logerror("%s user cb2 %d\n", machine().time().to_string(), line);
}

void mtu130_state::sys1_pb_w(u8 data)
{
	m_maincpu_intf->set_banks((~data >> 2) & 3, (~data) & 3);

	// The jumper inverts the via-driven selection between external
	// and fdc rom, changing the reset default.
	if(data & 0x40)
		m_se_view.select(m_jumpers->read() & 0x01);
	else
		m_se_view.select((m_jumpers->read() & 0x01) ^ 1);

	if(data & 0x80)
		m_rom_view.disable();
	else
		m_rom_view.select(1);

	bool video_bw = data & 0x10;
	bool video_unblank = data & 0x20;

	if(video_bw != m_video_bw || video_unblank != m_video_unblank) {
		m_screen->update_now();

		m_video_bw = video_bw;
		m_video_unblank = video_unblank;
	}
}

u8 mtu130_state::sys1_pa_r()
{
	// A capacitor is used to pretend the MOD key is pressed at power-on time.
	return m_keyboard[m_keyboard_col & 0xf]->read() |
		((m_keyboard_meta->read() & 0x04) || (machine().time().as_double() < 0.1) ? 1 : 0);
}

void mtu130_state::sys2_pa_w(u8 data)
{
	m_dac_level = data;
	m_dac->write(data);
}

void mtu130_state::sys1_ca2_w(int state)
{
	if(state)
		m_keyboard_col ++;
}

void mtu130_state::io_enable_w(u8)
{
	m_io_view.select(1);
}

void mtu130_state::io_disable_w(u8)
{
	m_io_view.disable();
}

INPUT_CHANGED_MEMBER(mtu130_state::nmi_w)
{
	m_maincpu->set_input_line(m6502_device::NMI_LINE, newval ? ASSERT_LINE : CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(mtu130_state::reset_w)
{
	if(newval == 0)
		reset();
}

INPUT_CHANGED_MEMBER(mtu130_state::break_w)
{
	m_sys1_6522->write_ca1(newval);
}

void mtu130_state::keyboard_col_clear_w(u8)
{
	m_keyboard_col = 0;
}

u8 mtu130_state::lightpen_status_r()
{
	return m_lightpen_status;
}

u8 mtu130_state::lightpen_low_r()
{
	return m_lightpen_low;
}

u8 mtu130_state::lightpen_high_r()
{
	return m_lightpen_high;
}

void mtu130_state::lightpen_clear_w(u8)
{
	m_lightpen_status &= ~8;
}

void mtu130_state::lightpen_trigger(int state)
{
	if(state && m_lightpen_w->read()) {
		m_lightpen_hit_x = m_lightpen_x->read();
		m_lightpen_hit_y = m_lightpen_y->read();
		m_timer_lightpen_hit->adjust(m_screen->time_until_pos(m_lightpen_hit_y, m_lightpen_hit_x + 120));
	}
}

TIMER_CALLBACK_MEMBER(mtu130_state::lightpen_hit)
{
	if(m_lightpen_status & 8)
		return;

	// aaaa aaaa .ppp ...s  (a = address, p = pixel, s = skew)
	// The os-provided coordinates decoding routine cannot generate x >= 474
	static const uint16_t x_to_address[480] = {
		0x0261, 0x0271, 0x0300, 0x0310, 0x0320, 0x0330, 0x0340, 0x0350, 0x0360, 0x0370,
		0x0301, 0x0311, 0x0421, 0x0431, 0x0440, 0x0450, 0x0460, 0x0470, 0x0400, 0x0410,
		0x0420, 0x0430, 0x0540, 0x0550, 0x0560, 0x0570, 0x0501, 0x0511, 0x0521, 0x0531,
		0x0541, 0x0551, 0x0660, 0x0670, 0x0700, 0x0710, 0x0720, 0x0730, 0x0740, 0x0750,
		0x0760, 0x0770, 0x0801, 0x0811, 0x0821, 0x0831, 0x0840, 0x0850, 0x0860, 0x0870,
		0x0800, 0x0810, 0x0820, 0x0830, 0x0940, 0x0950, 0x0960, 0x0970, 0x0901, 0x0911,
		0x0921, 0x0931, 0x0a41, 0x0a51, 0x0a61, 0x0a71, 0x0a00, 0x0a10, 0x0a20, 0x0a30,
		0x0a40, 0x0a50, 0x0a60, 0x0a70, 0x0c00, 0x0c10, 0x0c20, 0x0c30, 0x0c40, 0x0c50,
		0x0c61, 0x0c71, 0x0d00, 0x0d10, 0x0d20, 0x0d30, 0x0d40, 0x0d50, 0x0d60, 0x0d70,
		0x0d01, 0x0d11, 0x0e21, 0x0e31, 0x0e40, 0x0e50, 0x0e60, 0x0e70, 0x0e00, 0x0e10,
		0x0e20, 0x0e30, 0x0f40, 0x0f50, 0x0f60, 0x0f70, 0x0f01, 0x0f11, 0x0f21, 0x0f31,
		0x0f41, 0x0f51, 0x1060, 0x1070, 0x1100, 0x1110, 0x1120, 0x1130, 0x1140, 0x1150,
		0x1160, 0x1170, 0x1201, 0x1211, 0x1221, 0x1231, 0x1240, 0x1250, 0x1260, 0x1270,
		0x1200, 0x1210, 0x1220, 0x1230, 0x1340, 0x1350, 0x1360, 0x1370, 0x1301, 0x1311,
		0x1321, 0x1331, 0x1441, 0x1451, 0x1461, 0x1471, 0x1400, 0x1410, 0x1420, 0x1430,
		0x1440, 0x1450, 0x1460, 0x1470, 0x1600, 0x1610, 0x1620, 0x1630, 0x1640, 0x1650,
		0x1661, 0x1671, 0x1700, 0x1710, 0x1720, 0x1730, 0x1740, 0x1750, 0x1760, 0x1770,
		0x1701, 0x1711, 0x1821, 0x1831, 0x1840, 0x1850, 0x1860, 0x1870, 0x1800, 0x1810,
		0x1820, 0x1830, 0x1940, 0x1950, 0x1960, 0x1970, 0x1901, 0x1911, 0x1921, 0x1931,
		0x1941, 0x1951, 0x1a60, 0x1a70, 0x1b00, 0x1b10, 0x1b20, 0x1b30, 0x1b40, 0x1b50,
		0x1b60, 0x1b70, 0x1c01, 0x1c11, 0x1c21, 0x1c31, 0x1c40, 0x1c50, 0x1c60, 0x1c70,
		0x1c00, 0x1c10, 0x1c20, 0x1c30, 0x1d40, 0x1d50, 0x1d60, 0x1d70, 0x1d01, 0x1d11,
		0x1d21, 0x1d31, 0x1e41, 0x1e51, 0x1e61, 0x1e71, 0x1e00, 0x1e10, 0x1e20, 0x1e30,
		0x1e40, 0x1e50, 0x1e60, 0x1e70, 0x2000, 0x2010, 0x2020, 0x2030, 0x2040, 0x2050,
		0x2061, 0x2071, 0x2100, 0x2110, 0x2120, 0x2130, 0x2140, 0x2150, 0x2160, 0x2170,
		0x2101, 0x2111, 0x2221, 0x2231, 0x2240, 0x2250, 0x2260, 0x2270, 0x2200, 0x2210,
		0x2220, 0x2230, 0x2340, 0x2350, 0x2360, 0x2370, 0x2301, 0x2311, 0x2321, 0x2331,
		0x2341, 0x2351, 0x2460, 0x2470, 0x2500, 0x2510, 0x2520, 0x2530, 0x2540, 0x2550,
		0x2560, 0x2570, 0x2601, 0x2611, 0x2621, 0x2631, 0x2640, 0x2650, 0x2660, 0x2670,
		0x2600, 0x2610, 0x2620, 0x2630, 0x2740, 0x2750, 0x2760, 0x2770, 0x2701, 0x2711,
		0x2721, 0x2731, 0x2841, 0x2851, 0x2861, 0x2871, 0x2800, 0x2810, 0x2820, 0x2830,
		0x2840, 0x2850, 0x2860, 0x2870, 0x2a00, 0x2a10, 0x2a20, 0x2a30, 0x2a40, 0x2a50,
		0x2a61, 0x2a71, 0x2b00, 0x2b10, 0x2b20, 0x2b30, 0x2b40, 0x2b50, 0x2b60, 0x2b70,
		0x2b01, 0x2b11, 0x2c21, 0x2c31, 0x2c40, 0x2c50, 0x2c60, 0x2c70, 0x2c00, 0x2c10,
		0x2c20, 0x2c30, 0x2d40, 0x2d50, 0x2d60, 0x2d70, 0x2d01, 0x2d11, 0x2d21, 0x2d31,
		0x2d41, 0x2d51, 0x2e60, 0x2e70, 0x2f00, 0x2f10, 0x2f20, 0x2f30, 0x2f40, 0x2f50,
		0x2f60, 0x2f70, 0x3001, 0x3011, 0x3021, 0x3031, 0x3040, 0x3050, 0x3060, 0x3070,
		0x3000, 0x3010, 0x3020, 0x3030, 0x3140, 0x3150, 0x3160, 0x3170, 0x3101, 0x3111,
		0x3121, 0x3131, 0x3241, 0x3251, 0x3261, 0x3271, 0x3200, 0x3210, 0x3220, 0x3230,
		0x3240, 0x3250, 0x3260, 0x3270, 0x3400, 0x3410, 0x3420, 0x3430, 0x3440, 0x3450,
		0x3461, 0x3471, 0x3500, 0x3510, 0x3520, 0x3530, 0x3540, 0x3550, 0x3560, 0x3570,
		0x3501, 0x3511, 0x3621, 0x3631, 0x3640, 0x3650, 0x3660, 0x3670, 0x3600, 0x3610,
		0x3620, 0x3630, 0x3740, 0x3750, 0x3760, 0x3770, 0x3701, 0x3711, 0x3721, 0x3731,
		0x3741, 0x3751, 0x3860, 0x3870, 0x3900, 0x3910, 0x3920, 0x3930, 0x3940, 0x3950,
		0x3960, 0x3970, 0x3a01, 0x3a11, 0x3a21, 0x3a31, 0x3a40, 0x3a50, 0x3a60, 0x3a70,
		0x3a00, 0x3a10, 0x3a20, 0x3a30, 0x3b40, 0x3b50, 0x3b60, 0x3b70, 0x3b01, 0x3b11,
		0x3b21, 0x3b31, 0x3c41, 0x3c51, 0x3c61, 0x3c71, 0x3c00, 0x3c10, 0x3c20, 0x3c30,
		0x3c40, 0x3c50, 0x3c60, 0x3c70, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	};

	uint16_t base = x_to_address[m_lightpen_hit_x];
	if(!base)
		return;

	uint16_t pixel = (base >> 4) & 7;
	uint16_t skew = base & 1;
	uint16_t address = (base >> 8) + 60*m_lightpen_hit_y;

	m_lightpen_status = pixel | 8;
	m_lightpen_low = address & 0xff;
	m_lightpen_high = ((address >> 8) & 0x3f) | (skew ? 0x80 : 0x00);
}

void mtu130_state::tape_w(u8)
{
	logerror("tape_w\n");
}

void mtu130_state::map(address_map &map)
{
	map(0x00000, 0x0bfff).ram().share(m_mainram);                   // Main ram, in a single block
	map(0x08000, 0x0bfff).view(m_rom_view);                         // View to write-protect the top of the main ram
	m_rom_view[1](0x08000, 0x0bfff).nopw();

	map(0x0be00, 0x0bfff).view(m_io_view);                          // I/O dynamically overrides part of the main ram
	m_io_view[1](0x0be00, 0x0bfff).unmaprw();                       // Fully mask out the ram when active
	m_io_view[1](0x0bfc0, 0x0bfc0).rw(FUNC(mtu130_state::lightpen_status_r), FUNC(mtu130_state::lightpen_clear_w)).mirror(4);
	m_io_view[1](0x0bfc1, 0x0bfc1).rw(FUNC(mtu130_state::lightpen_low_r), FUNC(mtu130_state::keyboard_col_clear_w)).mirror(4);
	m_io_view[1](0x0bfc2, 0x0bfc2).rw(FUNC(mtu130_state::lightpen_high_r), FUNC(mtu130_state::tape_w)).mirror(4);
	m_io_view[1](0x0bfc3, 0x0bfc3).rw(FUNC(mtu130_state::id_r), FUNC(mtu130_state::id_reset_w)).mirror(4);
	m_io_view[1](0x0bfc8, 0x0bfcb).rw(m_acia, FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	m_io_view[1](0x0bfd0, 0x0bfdf).m(m_user_6522, FUNC(via6522_device::map));
	m_io_view[1](0x0bfe0, 0x0bfef).m(m_sys1_6522, FUNC(via6522_device::map));
	m_io_view[1](0x0bff0, 0x0bfff).m(m_sys2_6522, FUNC(via6522_device::map));

	map(0x0c000, 0x0ffff).view(m_se_view);                          // System (fdc)/External view
	m_se_view[0](0x0c000, 0x0ffff).ram().share(m_fdcram);           // FDC ram is a single 16K block, fully adressible by the dma
	m_se_view[0](0x0e000, 0x0ffff).view(m_rof_view);                // View to write-protect the top half of the fdc ram
	m_rof_view[1](0x0e000, 0x0ffff).nopw();

	m_se_view[0](0x0ff00, 0x0ffff).rom().region(m_ipl, 0).unmapw(); // Bootrom overrides the end of the ram
	m_se_view[0](0x0ffe8, 0x0ffef).unmaprw();                       // Hole in the prom access for floppy i/o
	m_se_view[0](0x0ffe8, 0x0ffe8).rw(FUNC(mtu130_state::fdc_ctrl_r), FUNC(mtu130_state::fdc_ctrl_w));
	m_se_view[0](0x0ffea, 0x0ffea).w(FUNC(mtu130_state::dma_adr_w));
	m_se_view[0](0x0ffee, 0x0ffef).m(m_fdc, FUNC(upd765a_device::map));

	m_se_view[1](0x0c000, 0x0ffff).rom().share(m_romdata);          // External rom view, contents set by the MTU130_ROM subdevices

	map(0x0fffe, 0x0fffe).w(FUNC(mtu130_state::io_enable_w));
	map(0x0ffff, 0x0ffff).w(FUNC(mtu130_state::io_disable_w));

	map(0x1c000, 0x1ffff).ram().share(m_videoram);                  // 16k of video ram

	map(0x20000, 0x3ffff).lrw8(NAME([this](offs_t offset) -> u8 { return m_ext[0]->read23(offset) & m_ext[1]->read23(offset) & m_ext[2]->read23(offset); }),
							   NAME([this](offs_t offset, u8 data) { m_ext[0]->write23(offset, data); m_ext[1]->write23(offset, data); m_ext[2]->write23(offset, data); }));
}

void mtu130_state::floppies(device_slot_interface &device)
{
	device.option_add("8ssdd", FLOPPY_8_SSDD);
	device.option_add("8dsdd", FLOPPY_8_DSDD);
}

void mtu130_state::extension_board(machine_config &config, int slot_id, const char *tag, const char *def)
{
	MTU130_EXTENSION(config, m_ext[slot_id]);
	m_ext[slot_id]->set_slot_id(slot_id);
	m_ext[slot_id]->set_irq_merger(m_irq_merger);
	if(def)
		m_ext[slot_id]->set_default_option(def);

	m_ext[slot_id]->option_add("datamover", MTU130_DATAMOVER0);
	m_ext[slot_id]->option_add("datamover_sec", MTU130_DATAMOVER1); // Datamover using secondary i/o addresses
}

void mtu130_state::mtu130(machine_config &config)
{
	config.set_perfect_quantum(m_maincpu); // Needs tight sync with the 68000 in the datamover

	M6502(config, m_maincpu, 10_MHz_XTAL/10);
	m_maincpu->set_address_width(18, true);
	m_maincpu->set_addrmap(AS_PROGRAM, &mtu130_state::map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// HSync is between 40 and 79, VSync between 256 and 259, boundaries included
	m_screen->set_raw(10_MHz_XTAL, 620, 120, 600, 269, 0, 256);
	m_screen->set_screen_update(FUNC(mtu130_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(m_sys2_6522, FUNC(via6522_device::write_ca2));
	m_screen->screen_vblank().append(FUNC(mtu130_state::lightpen_trigger));

	PALETTE(config, m_palette).set_entries(4);

	MOS6522(config, m_user_6522, 10_MHz_XTAL/10);
	m_user_6522->irq_handler().set(m_irq_merger, FUNC(input_merger_device::in_w<1>));
	m_user_6522->cb2_handler().set(FUNC(mtu130_state::user_cb2_w));

	MOS6522(config, m_sys1_6522, 10_MHz_XTAL/10);
	m_sys1_6522->irq_handler().set(m_irq_merger, FUNC(input_merger_device::in_w<2>));
	m_sys1_6522->readpa_handler().set(FUNC(mtu130_state::sys1_pa_r));
	m_sys1_6522->writepb_handler().set(FUNC(mtu130_state::sys1_pb_w));
	m_sys1_6522->ca2_handler().set(FUNC(mtu130_state::sys1_ca2_w));

	MOS6522(config, m_sys2_6522, 10_MHz_XTAL/10);
	m_sys2_6522->irq_handler().set(m_irq_merger, FUNC(input_merger_device::in_w<3>));
	m_sys2_6522->writepa_handler().set(FUNC(mtu130_state::sys2_pa_w));

	MOS6551(config, m_acia, 3.68_MHz_XTAL/2);
	m_acia->irq_handler().set(m_irq_merger, FUNC(input_merger_device::in_w<4>));
	m_acia->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
	m_rs232->dcd_handler().set(m_acia, FUNC(mos6551_device::write_dcd));
	m_rs232->dsr_handler().set(m_acia, FUNC(mos6551_device::write_dsr));
	m_rs232->cts_handler().set(m_acia, FUNC(mos6551_device::write_cts));

	INPUT_MERGER_ANY_HIGH(config, m_irq_merger).output_handler().set_inputline(m_maincpu, m6502_device::IRQ_LINE);

	UPD765A(config, m_fdc, 10_MHz_XTAL/10*8, true, true); // *8 done through a PLL
	m_fdc->intrq_wr_callback().set(FUNC(mtu130_state::fdc_irq_w));
	m_fdc->drq_wr_callback().set(FUNC(mtu130_state::dma_drq_w));

	FLOPPY_CONNECTOR(config, m_floppy[0], mtu130_state::floppies, "8dsdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], mtu130_state::floppies, "8dsdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[2], mtu130_state::floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[3], mtu130_state::floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);

	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, m_speaker, 1.0);
	SPEAKER(config, m_speaker).front_center();

	extension_board(config, 0, "ext0", "datamover");
	extension_board(config, 1, "ext1", nullptr);
	extension_board(config, 2, "ext2", nullptr);

	MTU130_ROM(config, m_roms[0], m_romdata, 0x3000);
	MTU130_ROM(config, m_roms[1], m_romdata, 0x2000);
	MTU130_ROM(config, m_roms[2], m_romdata, 0x1000);
	MTU130_ROM(config, m_roms[3], m_romdata, 0x0000);

	SOFTWARE_LIST(config, "flop_list").set_original("mtu130_flop");
}


static INPUT_PORTS_START(mtu130)
	PORT_START("jumpers")
	PORT_CONFNAME(0x01, 0x00, "Boot rom")
	PORT_CONFSETTING(0x00, "FDC rom")
	PORT_CONFSETTING(0x01, "ROM1 rom (aka rom F)")

	PORT_START("lightpen_w")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1)
	PORT_BIT(0xfe, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("lightpen_x")
	PORT_BIT(0x1ff, 0x000, IPT_LIGHTGUN_X) PORT_MINMAX(0, 479) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)

	PORT_START("lightpen_y")
	PORT_BIT(0x0ff, 0x000, IPT_LIGHTGUN_Y) PORT_MINMAX(0, 255) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)

	PORT_START("K0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)      PORT_CHAR(UCHAR_SHIFT_2) PORT_NAME("RIGHT SHIFT")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)      PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("LEFT SHIFT")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)    PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)    PORT_NAME("CTRL")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_NAME("REPEAT")

	PORT_START("K1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)   PORT_NAME("Keypad -")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)          PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)         PORT_CHAR(9)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)         PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("K2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)       PORT_NAME("Keypad 6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)          PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)       PORT_CHAR(' ')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)           PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)           PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)           PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("K3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)    PORT_NAME("Keypad +")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)          PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)           PORT_CHAR('Z')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)           PORT_CHAR('S')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)           PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)           PORT_CHAR('2') PORT_CHAR('@')

	PORT_START("K4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)   PORT_NAME("Keypad /")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)          PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)           PORT_CHAR('X')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)           PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)           PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)           PORT_CHAR('3') PORT_CHAR('#')

	PORT_START("K5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)       PORT_NAME("Keypad 5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)          PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)           PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)           PORT_CHAR('F')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)           PORT_CHAR('R')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)           PORT_CHAR('4') PORT_CHAR('$')

	PORT_START("K6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)    PORT_NAME("Keypad *")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)          PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)           PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)           PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)           PORT_CHAR('T')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)           PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("K7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)       PORT_NAME("Keypad 9")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)          PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)           PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)           PORT_CHAR('H')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)           PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)           PORT_CHAR('6') PORT_CHAR('^')

	PORT_START("K8")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_NAME("PF2")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)          PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)           PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)           PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)           PORT_CHAR('U')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)           PORT_CHAR('7') PORT_CHAR('&')

	PORT_START("K9")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)       PORT_NAME("Keypad 1")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)        PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)           PORT_CHAR('M')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)           PORT_CHAR('K')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)           PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)           PORT_CHAR('8') PORT_CHAR('*')

	PORT_START("KA")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)       PORT_NAME("Keypad 4")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)       PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)           PORT_CHAR('L')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)           PORT_CHAR('O')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)           PORT_CHAR('9') PORT_CHAR('(')

	PORT_START("KB")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)       PORT_NAME("Keypad 3")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)   PORT_NAME("Keypad ENTER")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)        PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)       PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)           PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)           PORT_CHAR('0') PORT_CHAR(')')

	PORT_START("KC")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)       PORT_NAME("Keypad 7")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)        PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)       PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('[') PORT_CHAR(']')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)       PORT_CHAR('-') PORT_CHAR('_')

	PORT_START("KD")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_NAME("PF1")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)        PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHAR('{') PORT_CHAR('}')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)   PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)      PORT_CHAR('=') PORT_CHAR('+')

	PORT_START("KE")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)       PORT_NAME("Keypad 8")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)          PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)      PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)       PORT_NAME("RETURN")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_NAME("LINE FEED")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)       PORT_CHAR('`') PORT_CHAR('~')

	PORT_START("KF")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)       PORT_NAME("Keypad 2")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)       PORT_NAME("Keypad 0")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)     PORT_NAME("Keypad .")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_NAME("RUBOUT")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)   PORT_CHAR(8)

	PORT_START("KM")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INT")   PORT_CHANGED_MEMBER(DEVICE_SELF, mtu130_state, nmi_w, 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MOD")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET") PORT_CHANGED_MEMBER(DEVICE_SELF, mtu130_state, reset_w, 0)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CHANGED_MEMBER(DEVICE_SELF, mtu130_state, break_w, 0)
INPUT_PORTS_END

ROM_START(mtu130)
	ROM_REGION(0x100, "ipl", 0)
	ROM_LOAD("ipl_prom.u6", 0, 0x100, CRC(edb1525a) SHA1(a16cce3b096f0fea9ac5c6993ee4241e4af1efde))

	ROM_REGION(0x100, "sequencer", 0) // 4-bit prom
	ROM_LOAD("6301.u55", 0, 0x100, CRC(1541eb91) SHA1(78ab124865dc6ffd646abd2fcab5b881edd619c1))

	ROM_REGION(0x100, "id", 0) // 4-bit prom, only first 16 nibbles reachable, rest all f
	// address 1     unused f
	// address 2-6   vendor 00102
	// address 7-b   group  00000
	// address c-f,0 user   00175 (used by BASIC 1.0 and 1.5 for system-locking)

	ROM_LOAD("6301.u24", 0, 0x100, CRC(7ebc5451) SHA1(402bd7bf343d995bc9c857fe4f3a23e0a8e7bd1c))
ROM_END

COMP(1981, mtu130, 0, 0, mtu130, mtu130, mtu130_state, empty_init, "Micro Technology Unlimited", "MTU-130", MACHINE_SUPPORTS_SAVE|MACHINE_IMPERFECT_SOUND)

