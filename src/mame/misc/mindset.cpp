// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "cpu/i86/i186.h"
#include "cpu/mcs48/mcs48.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"
#include "sound/dac.h"
#include "machine/ins8250.h"
#include "bus/rs232/rs232.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "mindset.lh"

// Missing:
// - Correct video timings
//   * screen_device does not handle interlaced screens yet

// - Correct timing of the specific-video-position interrupt
//   * see previous (so that partial screen update is correct) plus there's no "interrupt at a (x, y) position" interface

// - Interrupt from modules
//   * no use example, serial maybe?  And may be going through the system mcu

// - HD support module
//   * can't find the software that uses it.  Hand-drawn schematic at http://bitsavers.org/pdf/mindset/video_input/video_input_module_schematic.jpg (it's the same)

// - Modem modules
//   * no information on them, probably no software to use them either

// - Cartridges
//   * no dump of the rom ones, no users for the nvram ones.  gwbasic could use the nvram ones maybe?

// - Graphics CoProcessor timings
//   * need to count the accesses and estimate the mean access duration

// - GCP right-to-left blitting
//   * need to find a user, otherwise no way to know if the implementation is correct

// - GCP collision detection
//   * need to find a user, or at least a way to distinguish between the mask and the comparison value

// - GCP interrupt
//   * need to find a user, especially since it interacts with the system mcu

// - Genlock
//   * no osd support for video input

// - Capture card
//   * no osd support for video input.  Could try with the picture image device maybe?

// - Digitizing tablet
//   * no osd support, and a relatively rare device in the real world which is also hard to simulate on something else

// - Power control
//   * the system mcu controls the power to the system (some bit of p2 is seems), rocker switch on the keyboard resets the keyboard mcu for "on" and pretends sending a byte for "off".  Can be annoying UI-wise, and pretty much requires autosave.

class mindset_module_interface: public device_t {
public:
	virtual void map(address_map &map) = 0;
	virtual void idmap(address_map &map) = 0;

	auto irq_cb() { return m_irq_cb.bind(); }
	bool irq_r() const;

protected:
	bool m_irq_state;

	void irq_w(int state);

	mindset_module_interface(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_write_line m_irq_cb;
};

void mindset_module_interface::device_start()
{
	save_item(NAME(m_irq_state));
}

void mindset_module_interface::device_reset()
{
	m_irq_state = false;
	m_irq_cb(m_irq_state);
}

mindset_module_interface::mindset_module_interface(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_irq_cb(*this)
{
}

void mindset_module_interface::irq_w(int state)
{
	m_irq_state = state;
	m_irq_cb(m_irq_state);
}

class mindset_module: public device_t,
					  public device_slot_interface
{
public:
	template <typename T>
	mindset_module(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt, bool fixed = false)
		: mindset_module(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
	}
	mindset_module(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~mindset_module() = default;

	void map(address_space &space, offs_t base, bool id);

protected:
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
};

DEFINE_DEVICE_TYPE(MINDSET_MODULE, mindset_module,  "mindset_module", "MINDSET module")

mindset_module::mindset_module(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MINDSET_MODULE, tag, owner, clock),
	device_slot_interface(mconfig, *this)
{
}

void mindset_module::device_validity_check(validity_checker &valid) const
{
	device_t *const carddev = get_card_device();
	if (carddev && !dynamic_cast<mindset_module_interface *>(carddev))
		osd_printf_error("Card device %s (%s) does not implement mindset_module_interface\n", carddev->tag(), carddev->name());
}

void mindset_module::device_start()
{
}

void mindset_module::map(address_space &space, offs_t base, bool id)
{
	mindset_module_interface *module = dynamic_cast<mindset_module_interface *>(get_card_device());
	if(module)
		space.install_device(base, base+0x3f, *module, id ? &mindset_module_interface::idmap : &mindset_module_interface::map);
}


class mindset_sound_module: public mindset_module_interface {
public:
	mindset_sound_module(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~mindset_sound_module() = default;

	virtual void map(address_map &map) override ATTR_COLD;
	virtual void idmap(address_map &map) override ATTR_COLD;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 m_p1 = 0, m_p2 = 0;

	required_device<i8042_device> m_soundcpu;
	required_device<dac_byte_interface> m_dac;

	void p1_w(u8 data);
	void p2_w(u8 data);
	void update_dac();
};

DEFINE_DEVICE_TYPE(MINDSET_SOUND_MODULE, mindset_sound_module,  "mindset_sound_module", "MINDSET stereo sound module")

mindset_sound_module::mindset_sound_module(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mindset_module_interface(mconfig, MINDSET_SOUND_MODULE, tag, owner, clock),
	m_soundcpu(*this, "soundcpu"),
	m_dac(*this, "dac")
{
}

void mindset_sound_module::device_start()
{
	mindset_module_interface::device_start();
	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
}

void mindset_sound_module::device_reset()
{
	mindset_module_interface::device_reset();
	m_p1 = 0x80;
	m_p2 = 0;
	m_dac->write(0x80);
}

void mindset_sound_module::update_dac()
{
	// The p1 dac has the waveform (idle at 0x80), while the p2 one is used for the global volume (mute at 0x00, max at 0xff)
	m_dac->write((s8(m_p1-0x80)*m_p2/255 + 0x80) & 0xff);
}

void mindset_sound_module::p1_w(u8 data)
{
	m_p1 = data;
	update_dac();
}

void mindset_sound_module::p2_w(u8 data)
{
	m_p2 = data;
	update_dac();
}

void mindset_sound_module::map(address_map &map)
{
	map(0x00, 0x03).rw(m_soundcpu, FUNC(i8042_device::upi41_master_r), FUNC(i8042_device::upi41_master_w)).umask16(0x00ff).mirror(0x3c);
}

void mindset_sound_module::idmap(address_map &map)
{
	map(0x00, 0x3f).lr8(NAME([]() -> u8 { return 0x13; })).umask16(0x00ff);
}

ROM_START(mindset_sound_module)
	ROM_REGION(0x0800, "soundcpu", 0)
	ROM_LOAD("253006-001.u16", 0, 0x800, CRC(7bea5edd) SHA1(30cdc0dedaa5246f4952df452a99ca22e3cd0636))
ROM_END

const tiny_rom_entry *mindset_sound_module::device_rom_region() const
{
	return ROM_NAME(mindset_sound_module);
}

void mindset_sound_module::device_add_mconfig(machine_config &config)
{
	I8042(config, m_soundcpu, 12_MHz_XTAL/2);
	m_soundcpu->p1_out_cb().set(FUNC(mindset_sound_module::p1_w));
	m_soundcpu->p2_out_cb().set(FUNC(mindset_sound_module::p2_w));

	SPEAKER(config, "rspeaker").front_right();
	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "rspeaker", 0.5);
}


class mindset_rs232_module: public mindset_module_interface {
public:
	mindset_rs232_module(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~mindset_rs232_module() = default;

	virtual void map(address_map &map) override ATTR_COLD;
	virtual void idmap(address_map &map) override ATTR_COLD;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<ins8250_device> m_ins8250;
	required_device<rs232_port_device> m_rs232;
};

DEFINE_DEVICE_TYPE(MINDSET_RS232_MODULE, mindset_rs232_module,  "mindset_rs232_module", "MINDSET RS232 module")

mindset_rs232_module::mindset_rs232_module(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mindset_module_interface(mconfig, MINDSET_RS232_MODULE, tag, owner, clock),
	m_ins8250(*this, "ins8250"),
	m_rs232(*this, "rs232")
{
}

void mindset_rs232_module::map(address_map &map)
{
	map(0x00, 0x0f).rw(m_ins8250, FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w)).umask16(0x00ff).mirror(0x30);
}

void mindset_rs232_module::idmap(address_map &map)
{
	map(0x00, 0x3f).lr8(NAME([this]() -> u8 { return 0x73 | (m_irq_state ? 0x80 : 0x00); })).umask16(0x00ff);
}

void mindset_rs232_module::device_add_mconfig(machine_config &config)
{
	INS8250(config, m_ins8250, 12_MHz_XTAL/2/4); // Weird since there's no divider in the module
	m_ins8250->out_tx_callback().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_ins8250->out_int_callback().set(FUNC(mindset_rs232_module::irq_w));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_ins8250, FUNC(ins8250_device::rx_w));
	m_rs232->dsr_handler().set(m_ins8250, FUNC(ins8250_device::dsr_w));
}



class mindset_state: public driver_device
{
public:
	mindset_state(const machine_config &mconfig, device_type type, const char *tag);
	virtual ~mindset_state() = default;

	void mindset(machine_config &config);

protected:
	required_device<i80186_cpu_device> m_maincpu;
	required_device<i8042_device> m_syscpu, m_soundcpu;
	required_device<i8749_device> m_kbdcpu;
	required_device<screen_device> m_screen;
	required_device<i8272a_device> m_fdc;
	required_device<floppy_connector> m_fdco[2];
	required_shared_ptr<u16> m_vram;
	required_ioport_array<11> m_kbd_row;
	required_ioport_array<2> m_mouse_axis;
	required_ioport m_mouse_btn, m_joystick;
	output_finder<2> m_floppy_leds;
	output_finder<> m_red_led;
	output_finder<> m_yellow_led;
	output_finder<> m_green_led;
	required_device<dac_byte_interface> m_dac;
	required_device_array<mindset_module, 6> m_modules;

	memory_access<20, 1, 0, ENDIANNESS_LITTLE>::cache m_gcps;

	floppy_image_device *m_floppy[2];
	u32 m_palette[16];
	bool m_genlock[16];
	u16 m_dispctrl, m_screenpos, m_intpos, m_intaddr, m_fdc_dma_count;
	u8 m_kbd_p1, m_kbd_p2, m_borderidx, m_snd_p1, m_snd_p2, m_sys_p2;
	u8 m_mouse_last_read[2], m_mouse_counter[2];
	bool m_fdc_intext, m_fdc_int, m_fdc_drq, m_trap_int, m_trap_drq;

	u16 m_trap_data[8];
	u32 m_trap_pos, m_trap_len;

	static u16 gcp_blend_0(u16, u16);
	static u16 gcp_blend_1(u16, u16);
	static u16 gcp_blend_2(u16, u16);
	static u16 gcp_blend_3(u16, u16);
	static u16 gcp_blend_4(u16, u16);
	static u16 gcp_blend_5(u16, u16);
	static u16 gcp_blend_6(u16, u16);
	static u16 gcp_blend_7(u16, u16);

	static u16 (*const gcp_blend[8])(u16, u16);

	static inline u16 msk(int bit) { return (1U << bit) - 1; }
	static inline u16 sw(u16 data) { return (data >> 8) | (data << 8); }

	void maincpu_mem(address_map &map) ATTR_COLD;
	void maincpu_io(address_map &map) ATTR_COLD;

	void display_mode();
	void blit(u16 packet_seg, u16 packet_adr);

	void gcp_w(u16);
	u16 dispctrl_r();
	void dispctrl_w(u16 data);
	u16 dispreg_r();
	void dispreg_w(u16 data);

	int sys_t0_r();
	int sys_t1_r();
	u8 sys_p1_r();
	u8 sys_p2_r();
	void sys_p1_w(u8 data);
	void sys_p2_w(u8 data);

	void kbd_p1_w(u8 data);
	void kbd_p2_w(u8 data);
	int kbd_t1_r();
	u8 kbd_d_r();

	void snd_p1_w(u8 data);
	void snd_p2_w(u8 data);

	void fdc_ctrl_w(u8 data);
	void fdc_int_w(int state);
	u16 fdc_clear_interrupt();
	void fdc_dma_count_w(u16 data);
	u8 fdc_dma_r();
	void fdc_dma_w(u8 data);

	u16 trap_dma_r(offs_t, u16);
	u16 trap_r(offs_t offset);
	void trap_w(offs_t offset, u16 data);
	u16 trap_clear_interrupt();

	template<int floppy> void floppy_led_cb(floppy_image_device *, int state);

	u16 keyscan();
	void update_dac();
	void map_modules();

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
};


mindset_state::mindset_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_syscpu(*this, "syscpu"),
	m_soundcpu(*this, "soundcpu"),
	m_kbdcpu(*this, "kbdcpu"),
	m_screen(*this, "screen"),
	m_fdc(*this, "fdc"),
	m_fdco{{*this, "fdc:0"}, {*this, "fdc:1"}},
	m_vram(*this, "vram"),
	m_kbd_row(*this, "K%02u", 0U),
	m_mouse_axis(*this, "MOUSEAXIS%u", 0U),
	m_mouse_btn(*this, "MOUSEBTN"),
	m_joystick(*this, "JOYSTICK"),
	m_floppy_leds(*this, "drive%u_led", 0U),
	m_red_led(*this, "red_led"),
	m_yellow_led(*this, "yellow_led"),
	m_green_led(*this, "green_led"),
	m_dac(*this, "dac"),
	m_modules(*this, "m%d", 0U)
{
}

template<int floppy> void mindset_state::floppy_led_cb(floppy_image_device *, int state)
{
	m_floppy_leds[floppy] = state;
}

void mindset_state::machine_start()
{
	m_floppy_leds.resolve();
	m_red_led.resolve();
	m_yellow_led.resolve();
	m_green_led.resolve();

	m_maincpu->space(AS_PROGRAM).cache(m_gcps);
	for(int i=0; i<2; i++)
		m_floppy[i] = m_fdco[i]->get_device();

	if(m_floppy[0])
		m_floppy[0]->setup_led_cb(floppy_image_device::led_cb(&mindset_state::floppy_led_cb<0>, this));
	if(m_floppy[1])
		m_floppy[1]->setup_led_cb(floppy_image_device::led_cb(&mindset_state::floppy_led_cb<1>, this));

	save_item(NAME(m_fdc_intext));
	save_item(NAME(m_fdc_int));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_trap_int));
	save_item(NAME(m_trap_drq));
	save_item(NAME(m_trap_pos));
	save_item(NAME(m_trap_len));
	save_item(NAME(m_trap_data));
	save_item(NAME(m_palette));
	save_item(NAME(m_genlock));
	save_item(NAME(m_dispctrl));
	save_item(NAME(m_screenpos));
	save_item(NAME(m_intpos));
	save_item(NAME(m_intaddr));
	save_item(NAME(m_fdc_dma_count));
	save_item(NAME(m_kbd_p1));
	save_item(NAME(m_kbd_p2));
	save_item(NAME(m_borderidx));
	save_item(NAME(m_mouse_last_read));
	save_item(NAME(m_mouse_counter));
	save_item(NAME(m_snd_p1));
	save_item(NAME(m_snd_p2));
	save_item(NAME(m_sys_p2));
}

void mindset_state::map_modules()
{
	auto &space = m_maincpu->space(AS_IO);
	bool id = !(m_sys_p2 & 0x40);
	space.unmap_readwrite(0x8080, 0x81ff);
	for(int i=0; i<6; i++)
		m_modules[i]->map(space, 0x8080 + 0x40*i, id);
}

void mindset_state::machine_reset()
{
	m_sys_p2 = 0;
	map_modules();
	m_fdc_intext = m_fdc_int = m_trap_int = m_fdc_drq = m_trap_drq = false;
	m_trap_pos = m_trap_len = 0;
	memset(m_trap_data, 0, sizeof(m_trap_data));
	memset(m_palette, 0, sizeof(m_palette));
	memset(m_genlock, 0, sizeof(m_genlock));
	m_dispctrl = m_screenpos = m_intpos = m_intaddr = m_fdc_dma_count = 0;
	m_kbd_p1 = m_kbd_p2 = m_borderidx = 0;
	memset(m_mouse_last_read, 0, sizeof(m_mouse_last_read));
	memset(m_mouse_counter, 0, sizeof(m_mouse_counter));
	m_snd_p1 = 0x80;
	m_snd_p2 = 0;
	m_dac->write(0x80);
}

int mindset_state::sys_t0_r()
{
	//  logerror("SYS: %d read t0 %d (%03x)\n", m_kbdcpu->total_cycles(), (m_kbd_p2 & 0x40) != 0, m_syscpu->pc());
	return (m_kbd_p2 & 0x40) != 0;
}

int mindset_state::sys_t1_r()
{
	logerror("SYS: read t1\n");
	return !m_fdc_int;
}

u8 mindset_state::sys_p1_r()
{
	//  logerror("SYS: read p1\n");
	return 0xff;
}

u8 mindset_state::sys_p2_r()
{
	//  logerror("SYS: read p2 (%03x)\n", m_syscpu->pc());
	return 0xff;
}

void mindset_state::sys_p1_w(u8 data)
{
	//  m_maincpu->int0_w(!((data & 0x40) || m_fdc->get_irq()));
	logerror("SYS: fdc write p1 %02x irq %d\n", data, !!(data & 0x40));
}

void mindset_state::sys_p2_w(u8 data)
{
	u8 old = m_sys_p2;
	m_sys_p2 = data;
	m_yellow_led = !BIT(data, 0);
	m_green_led = !BIT(data, 1);
	m_red_led = !BIT(data, 2);
	if((m_sys_p2 ^ old) & 0x40)
		map_modules();
	m_maincpu->int3_w(!(data & 0x80));
	//  logerror("SYS: write p2 %02x\n", data);
}

void mindset_state::kbd_p1_w(u8 data)
{
	m_kbd_p1 = data;
}

void mindset_state::kbd_p2_w(u8 data)
{
	//  if((m_kbd_p2 ^ data) & 0x40)
	//      logerror("KBD: %d output bit %d\n", m_kbdcpu->total_cycles(), (m_kbd_p2 & 0x40) != 0);
	m_kbd_p2 = data;
}

u8 mindset_state::kbd_d_r()
{
	return keyscan();
}

int mindset_state::kbd_t1_r()
{
	return keyscan() & 0x100;
}

void mindset_state::update_dac()
{
	// The p1 dac has the waveform (idle at 0x80), while the p2 one is used for the global volume (mute at 0x00, max at 0xff)
	m_dac->write((s8(m_snd_p1-0x80)*m_snd_p2/255 + 0x80) & 0xff);
}

void mindset_state::snd_p1_w(u8 data)
{
	m_snd_p1 = data;
	update_dac();
}

void mindset_state::snd_p2_w(u8 data)
{
	m_snd_p2 = data;
	update_dac();
}

u16 mindset_state::keyscan()
{
	u16 src = (m_kbd_p2 << 8) | m_kbd_p1;
	u16 res = 0x1ff;
	for(unsigned int i=0; i<11; i++)
		if(!(src & (1 << i)))
			res &= m_kbd_row[i]->read();

	if(!(src & 0x8000)) {
		int axis = (src >> 12) & 1;
		u8 aval = m_mouse_axis[axis]->read();
		m_mouse_counter[axis] += aval - m_mouse_last_read[axis];
		m_mouse_last_read[axis] = aval;

		u8 nib;
		if((src >> 11) & 1) {
			nib = m_mouse_counter[axis] & 0xf;
			m_mouse_counter[axis] &= 0xf0;
		} else {
			nib = m_mouse_counter[axis] >> 4;
			m_mouse_counter[axis] &= 0x0f;
		}

		res &= m_mouse_btn->read() & (nib | 0x1f0);
	}

	if(!(src & 0x2000))
		res &= m_joystick->read();

	return res;
}



u16 mindset_state::dispctrl_r()
{
	return m_dispctrl;
}

void mindset_state::dispctrl_w(u16 data)
{
	u16 chg = m_dispctrl ^ data;
	m_dispctrl = data;
	if(chg & 0xff88)
		logerror("display control %s bank=%c %s %s h=%d ppx=%d w=%s interlace=%d rreg=%d indicator=%s wreg=%d\n",
				 m_dispctrl & 0x8000 ? "?15" : "?!15",
				 m_dispctrl & 0x4000 ? '1' : '0',
				 m_dispctrl & 0x2000 ? "ibm" : "native",
				 m_dispctrl & 0x1000 ? "?12" : "?!12",
				 m_dispctrl & 0x0800 ? "400" : "200",
				 (m_dispctrl & 0x0600) >> 9,
				 m_dispctrl & 0x0100 ? "320" : "640",
				 m_dispctrl & 0x0080 ? "on" : "off",
				 (m_dispctrl & 0x0070) >> 4,
				 m_dispctrl & 0x0008 ? "on" : "off",
				 m_dispctrl & 7);
}

u16 mindset_state::dispreg_r()
{
	switch((m_dispctrl >> 4) & 7) {
	case 1: { // Read vram at interrupt position
		u16 v = m_vram[m_intaddr >> 1];
		if(m_intaddr & 1)
			v >>= 8;
		return sw(v << 4);
		break;
	}
	case 5: {
		// wants 0080 set to be able to upload the palette
		// may be a field indicator
		return 0x0080;
	}
	}

	logerror("dispreg read %x\n", (m_dispctrl >> 4) & 7);
	return 0;
}

void mindset_state::dispreg_w(u16 data)
{
	switch(m_dispctrl & 0x7) {
	case 0:
		m_screenpos = data;
		logerror("screen position (%d, %d)\n", (data >> 8) & 15, (data >> 12) & 15);
		break;
	case 1:
		m_borderidx = (data >> 8) & 0xf;
		logerror("border color %x\n", m_borderidx);
		break;
	case 2: {
		m_intpos = data;
		int intx = (159 - ((m_intpos >> 8) & 255)) * ((m_dispctrl & 0x100) ? 2 : 4);
		int inty = 199 - (m_intpos & 255);
		m_intaddr = 0;
		bool bank = m_dispctrl & 0x4000;
		bool ibm_mode = m_dispctrl & 0x2000;
		//      bool interleave = m_dispctrl & 0x0800;
		int pixels_per_byte_order = (m_dispctrl & 0x0600) >> 9;
		bool large_pixels = m_dispctrl & 0x0100;
		if(ibm_mode)
			m_intaddr = (inty & 1) * 0x2000 + (inty >> 1) * 80 + (intx >> (3 - large_pixels)) ;
		else {
			int stepy = 0;
			if(pixels_per_byte_order == 2) stepy = 40;
			if(pixels_per_byte_order == 1) stepy = 80;
			if(pixels_per_byte_order == 0) stepy = 160;

			m_intaddr = inty * stepy + (intx >> (pixels_per_byte_order - large_pixels + 2));
		}

		if(bank)
			m_intaddr += 16000;

		logerror("interrupt position (%3d, %3d) ramdac address %04x\n", intx, inty, m_intaddr);
		break;
	}
	case 4: {
		data = sw(data);
		u8 r = (0x49*(data & 7)) >> 1;
		u8 g = (0x49*((data & 0x38) >> 3)) >> 1;
		u8 b = (0x49*((data & 0x1c0) >> 6)) >> 1;

		m_palette[m_borderidx] = (r << 16) | (g << 8) | b;
		m_genlock[m_borderidx] = data & 0x0200;
		logerror("palette[%x] = %04x -> %06x.%d\n", m_borderidx, data, m_palette[m_borderidx], m_genlock[m_borderidx]);
		m_borderidx = (m_borderidx + 1) & 0xf;
		break;
	}
	case 5: {
		logerror("genlock %s%s, extra=%c\n", data & 0x0200 ? "on" : "off", data & 0x0100 ? " fixed" : "", data & 0x0400 ? '1' : '0');
		break;
	}

	default:
		logerror("display reg[%x] = %04x\n", m_dispctrl & 0xf, data);
	}
}

u32 mindset_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// Temporary gross hack
	if(cliprect.max_y != 479)
		return 0;
	const u16 *bank = m_dispctrl & 0x4000 ? m_vram + 8000 : m_vram;
	bool ibm_mode = m_dispctrl & 0x2000;
	bool interleave = m_dispctrl & 0x0800;
	int pixels_per_byte_order = (m_dispctrl & 0x0600) >> 9;
	bool large_pixels = m_dispctrl & 0x0100;

	bitmap.fill(m_palette[m_borderidx]);

	int dx = ((m_screenpos >>  8) & 15) * (751 - 640) / 15;
	int dy = ((m_screenpos >> 12) & 15) * (480 - 400) / 15;

	if(ibm_mode) {
		if(large_pixels) {
			static int const palind[4] = { 0, 1, 4, 5 };
			for(int field=0; field<2; field++) {
				for(u32 yy=0; yy<2; yy++) {
					const u16 *src = bank + 4096*yy;
					for(u32 y=yy; y<200; y+=2) {
						u32 *dest = &bitmap.pix(2*y+field+dy, dx);
						for(u32 x=0; x<320; x+=8) {
							u16 sv = sw(*src++);
							for(u32 xx=0; xx<8; xx++) {
								u32 color = m_palette[palind[(sv >> (14-2*xx)) & 3]];
								*dest++ = color;
								*dest++ = color;
							}
						}
					}
				}
			}
			return 0;
		} else {
			static int const palind[2] = { 0, 4 };
			for(int field=0; field<2; field++) {
				for(u32 yy=0; yy<2; yy++) {
					const u16 *src = bank + 4096*yy;
					for(u32 y=yy; y<200; y+=2) {
						u32 *dest = &bitmap.pix(2*y+field+dy, dx);
						for(u32 x=0; x<640; x+=16) {
							u16 sv = sw(*src++);
							for(u32 xx=0; xx<16; xx++) {
								u32 color = m_palette[palind[(sv >> (15-xx)) & 1]];
								*dest++ = color;
							}
						}
					}
				}
			}
			return 0;
		}
	} else {
		if(large_pixels) {
			if(!interleave) {
				switch(pixels_per_byte_order) {
				case 0: {
					const u16 *src = bank;
					for(u32 y=0; y<200; y++) {
						u32 *dest0 = &bitmap.pix(2*y+dy, dx);
						u32 *dest1 = &bitmap.pix(2*y+1+dy, dx);
						for(u32 x=0; x<320; x+=4) {
							u16 sv = sw(*src++);
							for(u32 xx=0; xx<4; xx++) {
								u32 color = m_palette[(sv >> (12-4*xx)) & 15];
								*dest0++ = color;
								*dest0++ = color;
								*dest1++ = color;
								*dest1++ = color;
							}
						}
					}
					return 0;
				}
				case 1: {
					static int const palind[4] = { 0, 1, 4, 5 };
					const u16 *src = bank;
					for(u32 y=0; y<200; y++) {
						u32 *dest0 = &bitmap.pix(2*y+dy, dx);
						u32 *dest1 = &bitmap.pix(2*y+1+dy, dx);
						for(u32 x=0; x<320; x+=8) {
							u16 sv = sw(*src++);
							for(u32 xx=0; xx<8; xx++) {
								u32 color = m_palette[palind[(sv >> (14-2*xx)) & 3]];
								*dest0++ = color;
								*dest0++ = color;
								*dest1++ = color;
								*dest1++ = color;
							}
						}
					}
					return 0;
				}
				case 2: {
					const u16 *src = bank;
					for(u32 y=0; y<200; y++) {
						u32 *dest0 = &bitmap.pix(2*y+dy, dx);
						u32 *dest1 = &bitmap.pix(2*y+1+dy, dx);
						for(u32 x=0; x<320; x+=16) {
							u16 sv = sw(*src++);
							for(u32 xx=0; xx<16; xx++) {
								u32 color = m_palette[(sv >> (15-xx)) & 1];
								*dest0++ = color;
								*dest0++ = color;
								*dest1++ = color;
								*dest1++ = color;
							}
						}
					}
					return 0;
				}
				}
			} else {
				switch(pixels_per_byte_order) {
				case 0: {
					const u16 *src = bank;
					for(u32 y=0; y<400; y++) {
						u32 *dest = &bitmap.pix(y+dy, dx);
						for(u32 x=0; x<320; x+=4) {
							u16 sv = sw(*src++);
							for(u32 xx=0; xx<4; xx++) {
								u32 color = m_palette[(sv >> (12-4*xx)) & 15];
								*dest++ = color;
								*dest++ = color;
							}
						}
					}
					return 0;
				}
				case 1: {
					static int const palind[4] = { 0, 1, 4, 5 };
					const u16 *src = bank;
					for(u32 y=0; y<400; y++) {
						u32 *dest = &bitmap.pix(y+dy, dx);
						for(u32 x=0; x<320; x+=8) {
							u16 sv = sw(*src++);
							for(u32 xx=0; xx<8; xx++) {
								u32 color = m_palette[palind[(sv >> (14-2*xx)) & 3]];
								*dest++ = color;
								*dest++ = color;
							}
						}
					}
					return 0;
				}
				}
			}
		} else {
			if(!interleave) {
				switch(pixels_per_byte_order) {
				case 0: {
					static int const palind[4] = { 0, 4, 8, 12 };
					const u16 *src = bank;
					for(u32 y=0; y<200; y++) {
						u32 *dest0 = &bitmap.pix(2*y+dy, dx);
						u32 *dest1 = &bitmap.pix(2*y+1+dy, dx);
						for(u32 x=0; x<640; x+=8) {
							u16 sv = sw(*src++);
							for(u32 xx=0; xx<8; xx++) {
								u32 color = m_palette[palind[(sv >> (14-2*xx)) & 3]];
								*dest0++ = color;
								*dest1++ = color;
							}
						}
					}
					return 0;
				}
				case 1: {
					static int const palind[2] = { 0, 4 };
					const u16 *src = bank;
					for(u32 y=0; y<200; y++) {
						u32 *dest0 = &bitmap.pix(2*y+dy, dx);
						u32 *dest1 = &bitmap.pix(2*y+1+dy, dx);
						for(u32 x=0; x<640; x+=16) {
							u16 sv = sw(*src++);
							for(u32 xx=0; xx<16; xx++) {
								u32 color = m_palette[palind[(sv >> (15-xx)) & 1]];
								*dest0++ = color;
								*dest1++ = color;
							}
						}
					}
					return 0;
				}
				}
			} else {
				static int const palind[2] = { 0, 4 };
				const u16 *src = bank;
				for(u32 y=0; y<400; y++) {
					u32 *dest = &bitmap.pix(y+dy, dx);
					for(u32 x=0; x<640; x+=16) {
						u16 sv = sw(*src++);
						for(u32 xx=0; xx<16; xx++) {
							u32 color = m_palette[palind[(sv >> (15-xx)) & 1]];
							*dest++ = color;
						}
					}
				}
				return 0;
			}
		}

		logerror("Unimplemented native mode (%dx%d, ppb=%d)\n", large_pixels ? 320 : 640, interleave ? 400 : 200, 2 << pixels_per_byte_order);
	}

	bitmap.fill(0);

	return 0;
}

u16 mindset_state::gcp_blend_0(u16 src, u16)
{
	return src;
}

u16 mindset_state::gcp_blend_1(u16 src, u16 dst)
{
	return src & dst;
}

u16 mindset_state::gcp_blend_2(u16 src, u16 dst)
{
	return src | dst;
}

u16 mindset_state::gcp_blend_3(u16 src, u16 dst)
{
	return src ^ dst;
}

u16 mindset_state::gcp_blend_4(u16 src, u16)
{
	return ~src;
}

u16 mindset_state::gcp_blend_5(u16 src, u16 dst)
{
	return (~src) & dst;
}

u16 mindset_state::gcp_blend_6(u16 src, u16 dst)
{
	return (~src) | dst;
}

u16 mindset_state::gcp_blend_7(u16 src, u16 dst)
{
	return (~src) ^ dst;
}

u16 (*const mindset_state::gcp_blend[8])(u16, u16) = {
	gcp_blend_0,
	gcp_blend_1,
	gcp_blend_2,
	gcp_blend_3,
	gcp_blend_4,
	gcp_blend_5,
	gcp_blend_6,
	gcp_blend_7
};


void mindset_state::blit(u16 packet_seg, u16 packet_adr)
{
	u16 mode    = sw(m_gcps.read_word((packet_seg << 4) + ((packet_adr +  0) & 0xffff)));
	u16 src_adr = sw(m_gcps.read_word((packet_seg << 4) + ((packet_adr +  2) & 0xffff)));
	u16 src_sft = sw(m_gcps.read_word((packet_seg << 4) + ((packet_adr +  4) & 0xffff)));
	u16 dst_adr = sw(m_gcps.read_word((packet_seg << 4) + ((packet_adr +  6) & 0xffff)));
	u16 dst_sft = sw(m_gcps.read_word((packet_seg << 4) + ((packet_adr +  8) & 0xffff)));
	u16 width   = sw(m_gcps.read_word((packet_seg << 4) + ((packet_adr + 10) & 0xffff)));
	u16 height  = sw(m_gcps.read_word((packet_seg << 4) + ((packet_adr + 12) & 0xffff)));
	u16 sy      = sw(m_gcps.read_word((packet_seg << 4) + ((packet_adr + 14) & 0xffff)));
	u16 dy      = sw(m_gcps.read_word((packet_seg << 4) + ((packet_adr + 16) & 0xffff)));
	u16 rmask   = sw(m_gcps.read_word((packet_seg << 4) + ((packet_adr + 18) & 0xffff)));
	u16 src_seg = sw(m_gcps.read_word((packet_seg << 4) + ((packet_adr + 20) & 0xffff)));
	u16 dst_seg = sw(m_gcps.read_word((packet_seg << 4) + ((packet_adr + 22) & 0xffff)));
	u16 wmask   = sw(m_gcps.read_word((packet_seg << 4) + ((packet_adr + 24) & 0xffff)));
	u16 kmask   = sw(m_gcps.read_word((packet_seg << 4) + ((packet_adr + 26) & 0xffff)));

	// -f-w wnpi ktxm mm--
	// f = fast (pure word copy)
	// w = pixel width (1/2/4/8 bits)
	// n = invert collision flag (unimplemented)
	// p = pattern fill (used by fill_dest_buffer)
	// i/f = increment source / don't (used by blt_copy_word)
	// t/o = transparent/opaque
	// k = detect collision (unimplemented)
	// x = go right to left (unimplemented)
	// m = blending mode

	if(0)
	logerror("GCP: p src %04x:%04x.%x dst %04x:%04x.%x sz %xx%x step %x:%x mask %04x:%04x k %x:%x mode %c%d%c%c%c%c%c%c%d\n", src_seg, src_adr, src_sft, dst_seg, dst_adr, dst_sft, width, height, sy, dy, rmask, wmask, (kmask >> 8) & 15, kmask & 15,
			 mode & 0x4000 ? 'f' : '-',
			 (mode >> 11) & 3,
			 mode & 0x400 ? 'n' : '-',
			 mode & 0x200 ? 'p' : '-',
			 mode & 0x100 ? 'i' : 'f',
			 mode & 0x80 ? 'k' : '-',
			 mode & 0x40 ? 't' : 'o',
			 mode & 0x20 ? 'x' : '-',
			 (mode >> 2) & 7);

	if(mode & 0x200) {
		// pattern fill
		u16 src = m_gcps.read_word((src_seg << 4) + src_adr);
		for(u16 w=0; w != width/2; w++) {
			m_gcps.write_word((dst_seg << 4) + dst_adr, src);
			dst_adr += 2;
		}

	} else if(mode & 0x4000) {
		// fast mode
		u32 nw = (width+15) >> 4;
		for(u32 y=0; y<height; y++) {
			u16 src_cadr = src_adr;
			u16 dst_cadr = dst_adr;
			for(u32 w=0; w!=nw; w++) {
				m_gcps.write_word((dst_seg << 4) + dst_cadr, m_gcps.read_word((src_seg << 4) + src_cadr));
				src_cadr += 2;
				dst_cadr += 2;
			}
			src_adr += sy;
			dst_adr += dy;
		}

	} else {
		auto blend = gcp_blend[(mode >> 2) & 7];

		// Need to rotate rmask depending on the shifts too
		u16 awmask = ((wmask << 16) | wmask) >> (15 - dst_sft);
		u16 swmask, mwmask, ewmask;
		if(dst_sft >= width) {
			swmask = msk(dst_sft+1) & ~msk(dst_sft - width + 1);
			mwmask = 0xffff;
			ewmask = swmask;
		} else {
			swmask = msk(dst_sft+1);
			mwmask = 0xffff;
			ewmask = ~msk((dst_sft - width + 1) & 15);
		}

		swmask &= awmask;
		mwmask &= awmask;
		ewmask &= awmask;

		bool preload = dst_sft > src_sft;
		int src_do_sft = 15 - src_sft;
		int dst_do_sft = (preload ? 31 : 15) - dst_sft;

		u16 nw = ((width + (15 - dst_sft)) + 15) >> 4;

		for(u32 y=0; y<height; y++) {
			u16 src_cadr = src_adr;
			u16 dst_cadr = dst_adr;

			u16 cmask = swmask;
			u16 nw1 = nw;
			u32 srcs = 0;
			if(preload) {
				srcs = sw(m_gcps.read_word((src_seg << 4) + src_cadr)) << src_do_sft;
				if(mode & 0x100)
					src_cadr += 2;
			}
			do {
				srcs = (srcs << 16) | (sw(m_gcps.read_word((src_seg << 4) + src_cadr)) << src_do_sft);
				u16 src = (srcs >> dst_do_sft) & rmask;
				u16 dst = sw(m_gcps.read_word((dst_seg << 4) + dst_cadr));
				u16 res = blend(src, dst);
				if(mode & 0x40) {
					u16 tmask;
					switch((mode >> 11) & 3) {
					case 0:
					default:
						tmask = src;
						break;
					case 1:
						tmask = (src >> 1) | src;
						tmask = (tmask & 0x5555) * 0x3;
						break;
					case 2:
						tmask = (src >> 2) | src;
						tmask = (tmask >> 1) | tmask;
						tmask = (tmask & 0x1111) * 0xf;
						break;
					case 3:
						tmask = (src >> 4) | src;
						tmask = (tmask >> 2) | tmask;
						tmask = (tmask >> 1) | tmask;
						tmask = (tmask & 0x0101) * 0xff;
						break;
					}
					cmask &= tmask;
				}

				res = (dst & ~cmask) | (res & cmask);

				m_gcps.write_word((dst_seg << 4) + dst_cadr, sw(res));
				if(mode & 0x100)
					src_cadr += 2;
				dst_cadr += 2;

				nw1 --;

				cmask = nw1 == 1 ? ewmask : mwmask;
			} while(nw1);

			if(mode & 0x100)
				src_adr += sy;
			dst_adr += dy;
		}
	}
}

void mindset_state::gcp_w(u16)
{
	u16 packet_seg  = sw(m_gcps.read_word(0xbfd7a));
	u16 packet_adr  = sw(m_gcps.read_word(0xbfd78));
	u16 global_mode = sw(m_gcps.read_word(0xbfd76));

	if(0)
	logerror("GCP: start %04x:%04x mode %04x (%05x)\n", packet_seg, packet_adr, global_mode, m_maincpu->pc());

	switch(global_mode) {
	case 0x0005:
	case 0x0101:
		blit(packet_seg, packet_adr);
		break;
	}

	// 100 = done, 200 = done too???, 400 = collision?
	m_gcps.write_word(0xbfd74, m_gcps.read_word(0xbfd74) | 0x0700);

	// Can trigger an irq, on mode & 2 (or is it 200?) (0x40 on 8282, ack on 0x41, which means the system 8042...)
}

void mindset_state::fdc_ctrl_w(u8 data)
{
	logerror("fdc control %02x\n", data);
	if(data & 0x04)
		m_fdc->reset();
	m_floppy[data & 1]->mon_w(!(data & 2));
	m_floppy[(data & 1)^1]->mon_w(true);
}

void mindset_state::fdc_int_w(int state)
{
	if(!m_fdc_intext && state)
		m_fdc_int = true;
	m_fdc_intext = state;
	m_maincpu->int0_w(m_fdc_int || m_trap_int);
}

u16 mindset_state::fdc_clear_interrupt()
{
	m_fdc_int = false;
	m_maincpu->int0_w(m_fdc_int || m_trap_int);
	return 0x0000;
}

void mindset_state::fdc_dma_count_w(u16 data)
{
	m_fdc_dma_count = data;
	logerror("fdc dma count %x\n", m_fdc_dma_count);
}

u8 mindset_state::fdc_dma_r()
{
	u8 res = m_fdc->dma_r();
	if(!m_fdc_dma_count) {
		m_fdc->tc_w(1);
		m_fdc->tc_w(0);
	} else
		m_fdc_dma_count--;
	return res;
}

void mindset_state::fdc_dma_w(u8 data)
{
	m_fdc->dma_w(data);
	if(!m_fdc_dma_count) {
		m_fdc->tc_w(1);
		m_fdc->tc_w(0);
	} else
		m_fdc_dma_count--;
}

u16 mindset_state::trap_clear_interrupt()
{
	m_trap_int = false;
	m_maincpu->int0_w(m_fdc_int || m_trap_int);
	return 0x0000;
}

u16 mindset_state::trap_r(offs_t offset)
{
	//  machine().debug_break();
	logerror("trap_r %04x\n", offset << 1);
	m_trap_data[m_trap_len++] = (offset << 1) | 0x8000;
	m_trap_data[m_trap_len++] = 0;
	m_trap_drq = true;
	m_maincpu->drq1_w(m_fdc_drq || m_trap_drq);
	return 0;
}

void mindset_state::trap_w(offs_t offset, u16 data)
{
	//  machine().debug_break();
	logerror("trap_w %04x, %04x\n", offset << 1, data);
	m_trap_data[m_trap_len++] = offset << 1;
	m_trap_data[m_trap_len++] = data;
	m_trap_drq = true;
	m_maincpu->drq1_w(m_fdc_drq || m_trap_drq);
}

u16 mindset_state::trap_dma_r(offs_t, u16 mem_mask)
{
	u16 res = m_trap_pos < m_trap_len ? m_trap_data[m_trap_pos++] : 0;
	logerror("trap dma %04x @ %04x\n", res, mem_mask);
	if(m_trap_pos >= m_trap_len) {
		m_trap_drq = false;
		m_trap_int = true;
		m_maincpu->drq1_w(m_fdc_drq || m_trap_drq);
		m_maincpu->int0_w(m_fdc_int || m_trap_int);
		m_trap_pos = m_trap_len = 0;
	}
	return res;
}

void mindset_state::maincpu_mem(address_map &map)
{
	map(0x00000, 0x3ffff).ram();
	map(0xb8000, 0xbffff).ram().share("vram");
	map(0xf8000, 0xfffff).rom().region("maincpu", 0);
}

void mindset_state::maincpu_io(address_map &map)
{
	map(0x0000, 0x7fff).rw(FUNC(mindset_state::trap_r), FUNC(mindset_state::trap_w));

	map(0x8040, 0x8041).r(FUNC(mindset_state::trap_dma_r));
	map(0x8048, 0x8049).r(FUNC(mindset_state::trap_clear_interrupt));
	map(0x8050, 0x8050).w(FUNC(mindset_state::fdc_ctrl_w));
	map(0x8054, 0x8054).rw(FUNC(mindset_state::fdc_dma_r), FUNC(mindset_state::fdc_dma_w));
	map(0x8058, 0x8059).w(FUNC(mindset_state::fdc_dma_count_w));
	map(0x805c, 0x805d).r(FUNC(mindset_state::fdc_clear_interrupt));
	map(0x8060, 0x8060).r(m_fdc, FUNC(i8272a_device::msr_r));
	map(0x8062, 0x8062).rw(m_fdc, FUNC(i8272a_device::fifo_r), FUNC(i8272a_device::fifo_w));

#if 0
	map(0x8080, 0x8080).lr8("id13", []() -> u8 { return 0x13; }); // sound
	map(0x80c0, 0x80c0).lr8("id3f", []() -> u8 { return 0x3f; }); // serial type 1, maybe?
	map(0x8100, 0x8100).lr8("id5f", []() -> u8 { return 0x5f; }); // serial type 2
	map(0x8140, 0x8140).lr8("id70", []() -> u8 { return 0x70; }); // parallel printer, init writes 0x82 at +6
	map(0x8180, 0x8180).lr8("rs232-id", []() -> u8 { return 0x73; }); // rs232
#endif

	map(0x8280, 0x8283).rw(m_syscpu, FUNC(i8042_device::upi41_master_r), FUNC(i8042_device::upi41_master_w)).umask16(0x00ff);
	map(0x82a0, 0x82a3).rw(m_soundcpu, FUNC(i8042_device::upi41_master_r), FUNC(i8042_device::upi41_master_w)).umask16(0x00ff);
	map(0x8300, 0x8301).w(FUNC(mindset_state::gcp_w));
	map(0x8320, 0x8321).rw(FUNC(mindset_state::dispreg_r), FUNC(mindset_state::dispreg_w));
	map(0x8322, 0x8323).rw(FUNC(mindset_state::dispctrl_r), FUNC(mindset_state::dispctrl_w));
}

static void pc_dd_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

static void mindset_modules(device_slot_interface &device)
{
	device.option_add("stereo", MINDSET_SOUND_MODULE);
	device.option_add("rs232", MINDSET_RS232_MODULE);
}

void mindset_state::mindset(machine_config &config)
{
	config.set_perfect_quantum(m_syscpu);
	config.set_default_layout(layout_mindset);

	I80186(config, m_maincpu, 12_MHz_XTAL); // Divides internally by 2 to produce a clkout of 6MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &mindset_state::maincpu_mem);
	m_maincpu->set_addrmap(AS_IO,      &mindset_state::maincpu_io);

	I8042(config, m_syscpu, 14.318181_MHz_XTAL/2);
	m_syscpu->p1_in_cb().set(FUNC(mindset_state::sys_p1_r));
	m_syscpu->p2_in_cb().set(FUNC(mindset_state::sys_p2_r));
	m_syscpu->p1_out_cb().set(FUNC(mindset_state::sys_p1_w));
	m_syscpu->p2_out_cb().set(FUNC(mindset_state::sys_p2_w));
	m_syscpu->t0_in_cb().set(FUNC(mindset_state::sys_t0_r));
	m_syscpu->t1_in_cb().set(FUNC(mindset_state::sys_t1_r));

	I8042(config, m_soundcpu, 12_MHz_XTAL/2);
	m_soundcpu->p1_out_cb().set(FUNC(mindset_state::snd_p1_w));
	m_soundcpu->p2_out_cb().set(FUNC(mindset_state::snd_p2_w));

	I8749(config, m_kbdcpu, 6_MHz_XTAL);
	m_kbdcpu->p1_out_cb().set(FUNC(mindset_state::kbd_p1_w));
	m_kbdcpu->p2_out_cb().set(FUNC(mindset_state::kbd_p2_w));
	m_kbdcpu->bus_in_cb().set(FUNC(mindset_state::kbd_d_r));
	m_kbdcpu->t1_in_cb().set(FUNC(mindset_state::kbd_t1_r));

	// Should be NTSC actually... we'll see
	// Pretty sure the pixel clock is the 14.x one, the 12MHz one would only allow 630 pixels

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(100));
	m_screen->set_size(751, 480);
	m_screen->set_visarea(0, 750, 0, 479);
	m_screen->set_screen_update(FUNC(mindset_state::screen_update));
	// Should be at the position indicated by display reg 2
	m_screen->scanline().set([this](int scanline) { m_maincpu->int2_w(scanline == 398); });
	m_screen->screen_vblank().set(m_maincpu, FUNC(i80186_cpu_device::int1_w));

	I8272A(config, m_fdc, 16_MHz_XTAL/2, true);
	m_fdc->intrq_wr_callback().set(FUNC(mindset_state::fdc_int_w));
	m_fdc->drq_wr_callback().set([this](int state) { m_fdc_drq = state; m_maincpu->drq1_w(m_fdc_drq || m_trap_drq); });
	m_fdc->set_ready_line_connected(false);
	FLOPPY_CONNECTOR(config, m_fdco[0], pc_dd_floppies, "525dd", floppy_image_device::default_pc_floppy_formats);
	FLOPPY_CONNECTOR(config, m_fdco[1], pc_dd_floppies, "525dd", floppy_image_device::default_pc_floppy_formats);

	SPEAKER(config, "lspeaker").front_left();
	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "lspeaker", 0.5);

	MINDSET_MODULE(config, "m0", mindset_modules, "stereo", false);
	MINDSET_MODULE(config, "m1", mindset_modules, "rs232", false);
	MINDSET_MODULE(config, "m2", mindset_modules, nullptr, false);
	MINDSET_MODULE(config, "m3", mindset_modules, nullptr, false);
	MINDSET_MODULE(config, "m4", mindset_modules, nullptr, false);
	MINDSET_MODULE(config, "m5", mindset_modules, nullptr, false);

	SOFTWARE_LIST(config, "flop_list").set_original("mindset_flop");
}

static INPUT_PORTS_START(mindset)
	PORT_START("K00")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K01")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD)                               PORT_NAME("Start")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PAUSE)      PORT_NAME("Pause")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)        PORT_NAME("Sys config")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)        PORT_NAME("Reset")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K02")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K03")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(PGUP))
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD)                               PORT_NAME("Break")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K04")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR('\t')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K05")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)       PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_NAME("Caps lock")

	PORT_START("K06")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("Control")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K07")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13) PORT_NAME("Return")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K08")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_MAMEKEY(LALT)) PORT_NAME("Alt")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("Shift (Left)")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K09")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("Shift (Right)")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PRTSCR)     PORT_CHAR(UCHAR_MAMEKEY(PRTSCR)) PORT_NAME("Prt Scn")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SCRLOCK)    PORT_CHAR(UCHAR_MAMEKEY(SCRLOCK))

	PORT_START("K10")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("MOUSEAXIS1")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(50) PORT_KEYDELTA(1) PORT_MINMAX(0, 255) PORT_PLAYER(1)

	PORT_START("MOUSEAXIS0")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(50) PORT_KEYDELTA(1) PORT_MINMAX(0, 255) PORT_PLAYER(1)

	PORT_START("MOUSEBTN")
	PORT_BIT(0x1cf, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1)

	PORT_START("JOYSTICK")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x1c0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

ROM_START(mindset)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD16_BYTE("1.7_lo.u60", 0, 0x4000, CRC(00474dc1) SHA1(676f30f170c14174dbff3b5cbf98d0f23472b7c4))
	ROM_LOAD16_BYTE("1.7_hi.u59", 1, 0x4000, CRC(1434af10) SHA1(39105eacdd7ddc13e449e2c32743e828bef33595))

	ROM_REGION(0x0800, "syscpu", 0)
	ROM_LOAD("253002-001.u17", 0, 0x800, CRC(69da82c9) SHA1(2f0bf5b134dc703cbc72e0c6df5b7beda1b39e70))

	ROM_REGION(0x0800, "soundcpu", 0)
	ROM_LOAD("253006-001.u16", 0, 0x800, CRC(7bea5edd) SHA1(30cdc0dedaa5246f4952df452a99ca22e3cd0636))

	ROM_REGION(0x0800, "kbdcpu", 0)
	ROM_LOAD("kbd_v3.0.bin", 0, 0x800, CRC(1c6aa433) SHA1(1d01dbda4730f26125ba2564a608c2f8ddfc05b3))
ROM_END

COMP( 1984, mindset, 0, 0, mindset, mindset, mindset_state, empty_init, "Mindset Corporation", "Mindset Personal Computer", MACHINE_SUPPORTS_SAVE)

