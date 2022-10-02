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
// - Light pen, need working demo code

// Implemented extension boards:
// - DATAMOVER, a 68k-based board, used by BASIC 1.5 to accelerate floating point operations

// Unimplemented extension boards:
// - PROGRAMMOVER, a z80-based board
// - MultI-O, an i/o board

// Probable bug somewhere making the BASIC (light pen, game) demos
// fail in the demonstration disk.

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
	virtual void device_start() override;
	virtual image_init_result call_load() override;
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
		m_jumpers(*this, "jumpers")
	{ }

	void mtu130(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(nmi_w);
	DECLARE_INPUT_CHANGED_MEMBER(reset_w);
	DECLARE_INPUT_CHANGED_MEMBER(break_w);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

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

	memory_interface *m_maincpu_intf;

	uint16_t m_dma_adr;
	u8 m_keyboard_col;
	u8 m_dac_level;
	u8 m_id_adr;
	bool m_dma_direction;
	bool m_video_unblank;
	bool m_video_bw;
	bool m_fdc_irq_enabled;

	static void floppies(device_slot_interface &device);
	void map(address_map &map);
	void extension_board(machine_config &config, int slot_id, const char *tag, const char *def);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u8 fdc_ctrl_r();
	void fdc_ctrl_w(u8 data);
	void fdc_irq_w(int state);
	void dma_adr_w(u8 data);
	DECLARE_WRITE_LINE_MEMBER(dma_drq_w);

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

image_init_result mtu130_rom_device::call_load()
{
	u32 len = !loaded_through_softlist() ? length() : get_software_region_length("rom");
	if(!len || len > 4096 || (4096 % len))
		return image_init_result::FAIL;

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

	return image_init_result::PASS;
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

	save_item(NAME(m_dma_adr));
	save_item(NAME(m_dma_direction));
	save_item(NAME(m_keyboard_col));
	save_item(NAME(m_dac_level));
	save_item(NAME(m_id_adr));
	save_item(NAME(m_video_unblank));
	save_item(NAME(m_video_bw));
	save_item(NAME(m_fdc_irq_enabled));

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

WRITE_LINE_MEMBER(mtu130_state::dma_drq_w)
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


void mtu130_state::map(address_map &map)
{
	map(0x00000, 0x0bfff).ram().share(m_mainram);                   // Main ram, in a single block
	map(0x08000, 0x0bfff).view(m_rom_view);                         // View to write-protect the top of the main ram
	m_rom_view[1](0x08000, 0x0bfff).nopw();

	map(0x0be00, 0x0bfff).view(m_io_view);                          // I/O dynamically overrides part of the main ram
	m_io_view[1](0x0be00, 0x0bfff).unmaprw();                       // Fully mask out the ram when active
	m_io_view[1](0x0bfc3, 0x0bfc3).r(FUNC(mtu130_state::id_r));
	m_io_view[1](0x0bfc5, 0x0bfc5).w(FUNC(mtu130_state::keyboard_col_clear_w));
	m_io_view[1](0x0bfc7, 0x0bfc7).w(FUNC(mtu130_state::id_reset_w));
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
    // address c-f,0 user   00175 (used by BASIC 1.5 for system-locking)

	ROM_LOAD("6301.u24", 0, 0x100, CRC(7ebc5451) SHA1(402bd7bf343d995bc9c857fe4f3a23e0a8e7bd1c))
ROM_END

COMP(1981, mtu130, 0, 0, mtu130, mtu130, mtu130_state, empty_init, "Micro Technology Unlimited", "MTU-130", MACHINE_SUPPORTS_SAVE|MACHINE_IMPERFECT_SOUND)

