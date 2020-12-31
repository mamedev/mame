// license:BSD-3-Clause
// copyright-holders: Olivier Galibert

// A "virtual" driver to play vgm files
// Use with mame vgmplay -quik file.vgm

#include "emu.h"

#define QSOUND_LLE

#include "imagedev/snapquik.h"

#include "cpu/h6280/h6280.h"
#include "cpu/m6502/n2a03.h"
#include "cpu/m68000/m68000.h"
#include "cpu/sh/sh2.h"
#include "sound/2203intf.h"
#include "sound/2608intf.h"
#include "sound/2610intf.h"
#include "sound/2612intf.h"
#include "sound/262intf.h"
#include "sound/3526intf.h"
#include "sound/3812intf.h"
#include "sound/ay8910.h"
#include "sound/8950intf.h"
#include "sound/c140.h"
#include "sound/c352.h"
#include "sound/c6280.h"
#include "sound/es5503.h"
#include "sound/es5506.h"
#include "sound/gb.h"
#include "sound/iremga20.h"
#include "sound/k051649.h"
#include "sound/k053260.h"
#include "sound/k054539.h"
#include "sound/multipcm.h"
#include "sound/okim6258.h"
#include "sound/okim6295.h"
#include "sound/pokey.h"
#include "sound/qsound.h"
#include "sound/rf5c68.h"
#include "sound/saa1099.h"
#include "sound/scsp.h"
#include "sound/segapcm.h"
#include "sound/sn76496.h"
#include "sound/upd7759.h"
#include "sound/vgm_visualizer.h"
#include "sound/x1_010.h"
#include "sound/ym2151.h"
#include "sound/ym2413.h"
#include "sound/ymf271.h"
#include "sound/ymf278b.h"
#include "sound/ymz280b.h"
#include "audio/vboy.h"
#include "audio/wswan.h"
#include "machine/mega32x.h"

#include "vgmplay.lh"
#include "debugger.h"
#include "speaker.h"

#include <zlib.h>

#include <algorithm>
#include <iterator>
#include <list>
#include <memory>
#include <queue>
#include <utility>
#include <vector>

#define AS_IO16LE           1
#define AS_IO16BE           4

class vgmplay_disassembler : public util::disasm_interface
{
public:
	vgmplay_disassembler() = default;
	virtual ~vgmplay_disassembler() = default;

	virtual uint32_t opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

enum vgm_chip
{
	CT_SN76489 = 0,
	CT_YM2413,
	CT_YM2612,
	CT_YM2151,
	CT_SEGAPCM,
	CT_RF5C68,
	CT_YM2203,
	CT_YM2608,
	CT_YM2610,
	CT_YM3812,
	CT_YM3526,
	CT_Y8950,
	CT_YMF262,
	CT_YMF278B,
	CT_YMF271,
	CT_YMZ280B,
	CT_RF5C164,
	CT_SEGA32X,
	CT_AY8910,
	CT_GAMEBOY,
	CT_NESAPU,
	CT_MULTIPCM,
	CT_UPD7759,
	CT_OKIM6258,
	CT_OKIM6295,
	CT_K051649,
	CT_K054539,
	CT_C6280,
	CT_C140,
	CT_K053260,
	CT_POKEY,
	CT_QSOUND,
	CT_SCSP,
	CT_WSWAN,
	CT_VSU_VUE,
	CT_SAA1099,
	CT_ES5503,
	CT_ES5505,
	CT_X1_010,
	CT_C352,
	CT_GA20,

	CT_COUNT,
};

enum C140_TYPE
{
	C140_LINEAR = 0,
	C140_SYSTEM2,
	C140_SYSTEM21,
	C140_ASIC219
};

class vgmplay_device : public cpu_device
{
public:
	enum io8_t
	{
		REG_SIZE = 0xff000000,
		A_SN76489_0 = 0x00000000,
		A_SN76489_1 = 0x80000000,
		A_YM2413_0 = 0x01000000,
		A_YM2413_1 = 0x81000000,
		A_YM2612_0 = 0x02000000,
		A_YM2612_1 = 0x82000000,
		A_YM2151_0 = 0x03000000,
		A_YM2151_1 = 0x83000000,
		A_SEGAPCM_0 = 0x04000000,
		A_SEGAPCM_1 = 0x84000000,
		A_RF5C68 = 0x05000000,
		A_RF5C68_RAM = 0x85000000,
		A_YM2203_0 = 0x06000000,
		A_YM2203_1 = 0x86000000,
		A_YM2608_0 = 0x07000000,
		A_YM2608_1 = 0x87000000,
		A_YM2610_0 = 0x08000000,
		A_YM2610_1 = 0x88000000,
		A_YM3812_0 = 0x09000000,
		A_YM3812_1 = 0x89000000,
		A_YM3526_0 = 0x0a000000,
		A_YM3526_1 = 0x8a000000,
		A_Y8950_0 = 0x0b000000,
		A_Y8950_1 = 0x8b000000,
		A_YMF262_0 = 0x0c000000,
		A_YMF262_1 = 0x8c000000,
		A_YMF278B_0 = 0x0d000000,
		A_YMF278B_1 = 0x8d000000,
		A_YMF271_0 = 0x0e000000,
		A_YMF271_1 = 0x8e000000,
		A_YMZ280B_0 = 0x0f000000,
		A_YMZ280B_1 = 0x8f000000,
		A_RF5C164 = 0x10000000,
		A_RF5C164_RAM = 0x90000000,
		A_32X_PWM = 0x11000000,
		A_AY8910_0 = 0x12000000,
		A_AY8910_1 = 0x92000000,
		A_GAMEBOY_0 = 0x13000000,
		A_GAMEBOY_1 = 0x93000000,
		A_NESAPU_0 = 0x14000000,
		A_NES_RAM_0 = 0x14010000,
		A_NESAPU_1 = 0x94000000,
		A_NES_RAM_1 = 0x94010000,
		A_MULTIPCM_0 = 0x15000000,
		A_MULTIPCM_1 = 0x95000000,
		A_UPD7759_0 = 0x16000000,
		A_UPD7759_1 = 0x96000000,
		A_OKIM6258_0 = 0x17000000,
		A_OKIM6258_1 = 0x97000000,
		A_OKIM6295_0 = 0x18000000,
		A_OKIM6295_1 = 0x98000000,
		A_K051649_0 = 0x19000000,
		A_K051649_1 = 0x99000000,
		A_K054539_0 = 0x1a000000,
		A_K054539_1 = 0x9a000000,
		A_C6280_0 = 0x1b000000,
		A_C6280_1 = 0x9b000000,
		A_C140_0 = 0x1c000000,
		A_C140_1 = 0x9c000000,
		A_K053260_0 = 0x1d000000,
		A_K053260_1 = 0x9d000000,
		A_POKEY_0 = 0x1e000000,
		A_POKEY_1 = 0x9e000000,
		A_QSOUND = 0x1f000000,
		A_SCSP_0 = 0x20000000,
		A_SCSP_RAM_0 = 0x20010000,
		A_SCSP_1 = 0xa0000000,
		A_SCSP_RAM_1 = 0xa0010000,
		A_WSWAN_0 = 0x21000000,
		A_WSWAN_RAM_0 = 0x21000100,
		A_WSWAN_1 = 0xa1000000,
		A_WSWAN_RAM_1 = 0xa1000100,
		A_VSU_VUE_0 = 0x22000000,
		A_VSU_VUE_1 = 0xa2000000,
		A_SAA1099_0 = 0x23000000,
		A_SAA1099_1 = 0xa3000000,
		A_ES5503_0 = 0x24000000,
		A_ES5503_RAM_0 = 0x24000100,
		A_ES5503_1 = 0xa4000000,
		A_ES5503_RAM_1 = 0xa4000100,
		A_ES5505_0 = 0x25000000,
		A_ES5505_1 = 0xa5000000,
		A_X1_010_0 = 0x26000000,
		A_X1_010_1 = 0xa6000000,
		A_C352_0 = 0x27000000,
		A_C352_1 = 0xa7000000,
		A_GA20_0 = 0x28000000,
		A_GA20_1 = 0xa8000000,
	};

	vgmplay_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual uint32_t execute_input_lines() const noexcept override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	virtual space_config_vector memory_space_config() const override;

	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	template<int Index> uint8_t segapcm_rom_r(offs_t offset);
	template<int Index> uint8_t ym2608_rom_r(offs_t offset);
	template<int Index> uint8_t ym2610_adpcm_a_rom_r(offs_t offset);
	template<int Index> uint8_t ym2610_adpcm_b_rom_r(offs_t offset);
	template<int Index> uint8_t y8950_rom_r(offs_t offset);
	template<int Index> uint8_t ymf278b_rom_r(offs_t offset);
	template<int Index> uint8_t ymf271_rom_r(offs_t offset);
	template<int Index> uint8_t ymz280b_rom_r(offs_t offset);
	template<int Index> uint8_t multipcm_rom_r(offs_t offset);
	template<int Index> uint8_t upd7759_rom_r(offs_t offset);
	template<int Index> uint8_t okim6295_rom_r(offs_t offset);
	template<int Index> uint8_t k054539_rom_r(offs_t offset);
	template<int Index> uint16_t c140_rom_r(offs_t offset);
	template<int Index> uint16_t c219_rom_r(offs_t offset);
	template<int Index> uint8_t k053260_rom_r(offs_t offset);
	template<int Index> uint8_t qsound_rom_r(offs_t offset);
	template<int Index> uint8_t es5505_rom_r(offs_t offset);
	template<int Index> uint8_t x1_010_rom_r(offs_t offset);
	template<int Index> uint8_t c352_rom_r(offs_t offset);
	template<int Index> uint8_t ga20_rom_r(offs_t offset);

	template<int Index> void multipcm_bank_hi_w(offs_t offset, uint8_t data);
	template<int Index> void multipcm_bank_lo_w(offs_t offset, uint8_t data);

	template<int Index> void upd7759_bank_w(uint8_t data);

	template<int Index> void okim6295_nmk112_enable_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	template<int Index> void okim6295_bank_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	template<int Index> void okim6295_nmk112_bank_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);

	void set_c140_bank_type(int index, C140_TYPE type);
	C140_TYPE c140_bank(int index) { return m_c140_bank[index]; }

	void stop();
	void pause();
	bool paused() const { return m_paused; }
	void play();
	void toggle_loop() { m_loop = !m_loop; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum { ACT_LED_PERSIST_MS = 100 };

	enum { RESET, RUN, DONE };

	using led_expiry = std::pair<vgm_chip, attotime>;
	using led_expiry_list = std::list<led_expiry>;
	using led_expiry_iterator = led_expiry_list::iterator;

	struct rom_block
	{
		offs_t start_address;
		offs_t end_address;
		std::unique_ptr<uint8_t[]> data;

		rom_block(rom_block &&) = default;
		rom_block(offs_t start, offs_t end, std::unique_ptr<uint8_t[]> &&d) : start_address(start), end_address(end), data(std::move(d)) {}
	};

	struct stream
	{
		uint8_t byte_depth;
		uint32_t position;
		emu_timer *timer;
		// stream control
		vgm_chip chip_type;
		uint8_t port;
		uint8_t reg;
		// stream data
		uint8_t bank;
		uint8_t step_size;
		uint8_t step_base;
		// frequency
		uint32_t frequency;
		// start stream
		uint32_t offset;
		uint32_t length;
		bool loop;
		bool reverse;
	};

	TIMER_CALLBACK_MEMBER(stream_timer_expired);

	stream m_streams[0xff];

	void pulse_act_led(vgm_chip led);
	TIMER_CALLBACK_MEMBER(act_led_expired);

	uint8_t rom_r(int index, uint8_t type, offs_t offset);
	uint32_t handle_data_block(uint32_t address);
	uint32_t handle_pcm_write(uint32_t address);
	void blocks_clear();

	output_finder<CT_COUNT> m_act_leds;
	led_expiry_list m_act_led_expiries;
	std::unique_ptr<led_expiry_iterator[]> m_act_led_index;
	led_expiry_iterator m_act_led_off;
	emu_timer *m_act_led_timer = nullptr;

	address_space_config m_file_config, m_io_config, m_io16le_config, m_io16be_config;
	address_space *m_file = nullptr, *m_io = nullptr, *m_io16le = nullptr, *m_io16be = nullptr;

	int m_icount = 0;
	int m_state = RESET;
	bool m_paused = false;
	bool m_loop = false;

	uint32_t m_pc = 0U;

	std::list<rom_block> m_rom_blocks[2][0x40];

	struct data_block
	{
		uint32_t start;
		uint32_t size;
		data_block(uint32_t _start, uint32_t _size) { start = _start; size = _size; }
	};

	std::array<std::vector<uint8_t>, 0x40> m_data_streams;
	std::array<std::vector<data_block>, 0x40> m_data_stream_blocks;

	struct
	{
		uint8_t cmp_type;
		uint8_t cmp_sub_type;
		uint8_t bit_dec;
		uint8_t bit_cmp;
		std::vector<uint8_t> entries;
	} m_dec_table;

	uint32_t m_ym2612_stream_offset = 0U;

	uint32_t m_multipcm_bank_l[2];
	uint32_t m_multipcm_bank_r[2];
	uint32_t m_multipcm_banked[2];

	uint32_t m_upd7759_bank[2];

	uint32_t m_okim6295_nmk112_enable[2];
	uint32_t m_okim6295_bank[2];
	uint32_t m_okim6295_nmk112_bank[2][4];

	C140_TYPE m_c140_bank[2];

	int m_sega32x_channel_hack;
	int m_nes_apu_channel_hack[2];
	uint8_t m_c6280_channel[2];
};

DEFINE_DEVICE_TYPE(VGMPLAY, vgmplay_device, "vgmplay_core", "VGM Player engine")

enum vgmplay_inputs : uint8_t
{
	VGMPLAY_STOP,
	VGMPLAY_PAUSE,
	VGMPLAY_PLAY,
	VGMPLAY_RESTART,
	VGMPLAY_LOOP,
	VGMPLAY_VIZ,
	VGMPLAY_RATE_DOWN,
	VGMPLAY_RATE_UP,
	VGMPLAY_RATE_RST,
	VGMPLAY_HOLD,
};

class vgmplay_state : public driver_device
{
public:
	vgmplay_state(const machine_config &mconfig, device_type type, const char *tag);

	DECLARE_QUICKLOAD_LOAD_MEMBER(load_file);

	uint8_t file_r(offs_t offset);
	uint8_t file_size_r(offs_t offset);
	DECLARE_INPUT_CHANGED_MEMBER(key_pressed);

	template<int Index> void upd7759_reset_w(uint8_t data);
	template<int Index> void upd7759_data_w(uint8_t data);
	template<int Index> DECLARE_WRITE_LINE_MEMBER(upd7759_drq_w);
	template<int Index> void okim6258_clock_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	template<int Index> void okim6258_divider_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	template<int Index> void okim6295_clock_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	template<int Index> void okim6295_pin7_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	template<int Index> void scc_w(offs_t offset, uint8_t data);
	template<int Index> void c140_c219_w(offs_t offset, uint8_t data);

	void vgmplay(machine_config &config);
	void file_map(address_map &map);
	void soundchips_map(address_map &map);
	void soundchips16le_map(address_map &map);
	void soundchips16be_map(address_map &map);
	template<int Index> void segapcm_map(address_map &map);
	template<int Index> void rf5c68_map(address_map &map);
	template<int Index> void ym2608_map(address_map &map);
	template<int Index> void ym2610_adpcm_a_map(address_map &map);
	template<int Index> void ym2610_adpcm_b_map(address_map &map);
	template<int Index> void y8950_map(address_map &map);
	template<int Index> void ymf278b_map(address_map &map);
	template<int Index> void ymf271_map(address_map &map);
	template<int Index> void ymz280b_map(address_map &map);
	template<int Index> void rf5c164_map(address_map &map);
	template<int Index> void nescpu_map(address_map &map);
	template<int Index> void multipcm_map(address_map &map);
	template<int Index> void upd7759_map(address_map &map);
	template<int Index> void okim6295_map(address_map &map);
	template<int Index> void k054539_map(address_map &map);
	template<int Index> void c140_map(address_map &map);
	template<int Index> void c219_map(address_map &map);
	template<int Index> void k053260_map(address_map &map);
	template<int Index> void qsound_map(address_map &map);
	template<int Index> void scsp_map(address_map &map);
	template<int Index> void wswan_map(address_map &map);
	template<int Index> void es5503_map(address_map &map);
	template<int Index> void es5505_map(address_map &map);
	template<int Index> void x1_010_map(address_map &map);
	template<int Index> void c352_map(address_map &map);
	template<int Index> void ga20_map(address_map &map);

private:
	virtual void machine_start() override;

	uint32_t m_held_clock;
	std::vector<uint8_t> m_file_data;
	required_device<vgmplay_device> m_vgmplay;
	required_device<vgmviz_device> m_mixer;
	required_device<speaker_device> m_lspeaker;
	required_device<speaker_device> m_rspeaker;
	required_device_array<sn76489_device, 2> m_sn76489;
	required_device_array<ym2413_device, 2> m_ym2413;
	required_device_array<ym2612_device, 2> m_ym2612;
	required_device_array<ym2151_device, 2> m_ym2151;
	required_device_array<segapcm_device, 2> m_segapcm;
	required_device<rf5c68_device> m_rf5c68;
	required_device_array<ym2203_device, 2> m_ym2203;
	required_device_array<ym2608_device, 2> m_ym2608;
	required_device_array<ym2610_device, 2> m_ym2610;
	required_device_array<ym3812_device, 2> m_ym3812;
	required_device_array<ym3526_device, 2> m_ym3526;
	required_device_array<y8950_device, 2> m_y8950;
	required_device_array<ymf262_device, 2> m_ymf262;
	required_device_array<ymf278b_device, 2> m_ymf278b;
	required_device_array<ymf271_device, 2> m_ymf271;
	required_device_array<ymz280b_device, 2> m_ymz280b;
	required_device<rf5c164_device> m_rf5c164;
	required_device<sega_32x_ntsc_device> m_sega32x;
	required_device_array<ay8910_device, 2> m_ay8910;
	required_device_array<gameboy_sound_device, 2> m_dmg;
	required_device_array<n2a03_device, 2> m_nescpu;
	required_device_array<multipcm_device, 2> m_multipcm;
	required_device_array<upd7759_device, 2> m_upd7759;
	required_device_array<okim6258_device, 2> m_okim6258;
	required_device_array<okim6295_device, 2> m_okim6295;
	required_device_array<k051649_device, 2> m_k051649;
	required_device_array<k054539_device, 2> m_k054539;
	required_device_array<h6280_device, 2> m_huc6280;
	required_device_array<c140_device, 2> m_c140;
	required_device_array<c219_device, 2> m_c219;
	required_device_array<k053260_device, 2> m_k053260;
	required_device_array<pokey_device, 2> m_pokey;
	required_device<qsound_device> m_qsound;
	required_device_array<scsp_device, 2> m_scsp;
	required_device_array<wswan_sound_device, 2> m_wswan;
	required_device_array<vboysnd_device, 2> m_vsu_vue;
	required_device_array<saa1099_device, 2> m_saa1099;
	required_device_array<es5503_device, 2> m_es5503;
	required_device_array<es5505_device, 2> m_es5505;
	required_device_array<x1_010_device, 2> m_x1_010;
	required_device_array<c352_device, 2> m_c352;
	required_device_array<iremga20_device, 2> m_ga20;

	uint8_t m_okim6258_divider[2];
	uint8_t m_okim6295_pin7[2];
	uint8_t m_scc_reg[2];

	int m_upd7759_md[2];
	int m_upd7759_reset[2];
	int m_upd7759_drq[2];
	std::queue<uint8_t> m_upd7759_slave_data[2];

	uint32_t r32(int offset) const;
	uint8_t r8(int offset) const;
};

vgmplay_device::vgmplay_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	cpu_device(mconfig, VGMPLAY, tag, owner, clock),
	m_act_leds(*this, "led_act_%u", 0U),
	m_file_config("file", ENDIANNESS_LITTLE, 8, 32),
	m_io_config("io", ENDIANNESS_LITTLE, 8, 32),
	m_io16le_config("io16le", ENDIANNESS_LITTLE, 16, 32),
	m_io16be_config("io16be", ENDIANNESS_BIG, 16, 32)
{
}

void vgmplay_device::device_start()
{
	set_icountptr(m_icount);
	m_file = &space(AS_PROGRAM);
	m_io = &space(AS_IO);
	m_io16le = &space(AS_IO16LE);
	m_io16be = &space(AS_IO16BE);

	m_act_leds.resolve();
	m_act_led_index = std::make_unique<led_expiry_iterator[]>(CT_COUNT);
	for (vgm_chip led = vgm_chip(0); led != CT_COUNT; led = vgm_chip(led + 1))
		m_act_led_index[led] = m_act_led_expiries.emplace(m_act_led_expiries.end(), led, attotime::never);
	m_act_led_off = m_act_led_expiries.begin();
	m_act_led_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(vgmplay_device::act_led_expired), this));

	for (int i = 0; i < 0xff; i++)
		m_streams[i].timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(vgmplay_device::stream_timer_expired), this));

	save_item(NAME(m_pc));
	//save_item(NAME(m_streams));

	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
}

void vgmplay_device::device_reset()
{
	m_state = RESET;
	m_paused = false;

	m_ym2612_stream_offset = 0;
	std::fill(std::begin(m_upd7759_bank), std::end(m_upd7759_bank), 0);
	blocks_clear();

	for (int i = 0; i < 0xff; i++)
	{
		stream& s(m_streams[i]);
		s.chip_type = vgm_chip(0xff);
		s.bank = 0xff;
		s.frequency = 0;
		s.timer->enable(false);
	}

	m_sega32x_channel_hack = 0;
	m_nes_apu_channel_hack[0] = 0;
	m_nes_apu_channel_hack[1] = 0;
	m_c6280_channel[0] = 0;
	m_c6280_channel[1] = 0;
}

void vgmplay_device::pulse_act_led(vgm_chip led)
{
	m_act_leds[led] = 1;

	bool const was_first(m_act_led_expiries.begin() == m_act_led_index[led]);
	bool const all_off(m_act_led_expiries.begin() == m_act_led_off);
	attotime const now(machine().time());

	m_act_led_index[led]->second = now + attotime::from_msec(ACT_LED_PERSIST_MS);
	if (m_act_led_off != m_act_led_index[led])
		m_act_led_expiries.splice(m_act_led_off, m_act_led_expiries, m_act_led_index[led]);
	else
		++m_act_led_off;
	if (all_off)
		m_act_led_timer->adjust(attotime::from_msec(ACT_LED_PERSIST_MS));
	else if (was_first)
		m_act_led_timer->adjust(m_act_led_expiries.begin()->second - now);
}

TIMER_CALLBACK_MEMBER(vgmplay_device::act_led_expired)
{
	attotime const now(machine().time());

	while ((now + attotime::from_msec(1)) >= m_act_led_expiries.begin()->second)
	{
		led_expiry_iterator const expired(m_act_led_expiries.begin());
		m_act_leds[expired->first] = 0;
		expired->second = attotime::never;
		if (expired != m_act_led_off)
		{
			m_act_led_expiries.splice(m_act_led_off, m_act_led_expiries, expired);
			m_act_led_off = expired;
		}
	}

	if (m_act_led_expiries.begin() != m_act_led_off)
		m_act_led_timer->adjust(m_act_led_expiries.begin()->second - now);
}

void vgmplay_device::stop()
{
	device_reset();
	m_paused = true;
}

void vgmplay_device::pause()
{
	m_paused = !m_paused;
}

void vgmplay_device::play()
{
	if (m_paused && m_state != DONE)
		m_paused = false;
	else
		device_reset();
}

uint32_t vgmplay_device::execute_min_cycles() const noexcept
{
	return 0;
}

uint32_t vgmplay_device::execute_max_cycles() const noexcept
{
	return 65536;
}

uint32_t vgmplay_device::execute_input_lines() const noexcept
{
	return 0;
}

void vgmplay_device::blocks_clear()
{
	for (int i = 0; i < 0x40; i++)
	{
		m_rom_blocks[0][i].clear();
		m_rom_blocks[1][i].clear();
		m_data_streams[i].clear();
		m_data_stream_blocks[i].clear();
	}

	m_dec_table.entries.clear();
}

uint32_t vgmplay_device::handle_data_block(uint32_t address)
{
	uint32_t size = m_file->read_dword(m_pc + 3);
	int second = (size & 0x80000000) ? 1 : 0;
	size &= 0x7fffffff;

	uint8_t type = m_file->read_byte(m_pc + 2);
	if (type < 0x40)
	{
		uint32_t start = m_data_streams[type].size();
		m_data_stream_blocks[type].push_back(data_block(start, size));
		m_data_streams[type].resize(start + size);
		for (uint32_t i = 0; i<size; i++)
			m_data_streams[type][start + i] = m_file->read_byte(m_pc + 7 + i);
	}
	else if (type < 0x7f)
	{
		uint8_t cmp_type = m_file->read_byte(m_pc + 0x07);
		uint32_t out_size = m_file->read_dword(m_pc + 0x08);

		uint32_t start = m_data_streams[type - 0x40].size();
		m_data_stream_blocks[type - 0x40].push_back(data_block(start, out_size));
		m_data_streams[type - 0x40].resize(start + out_size);

		if (cmp_type == 0)
		{
			uint8_t bit_dec = m_file->read_byte(m_pc + 0x0c);
			uint8_t bit_cmp = m_file->read_byte(m_pc + 0x0d);
			uint8_t cmp_sub_type = m_file->read_byte(m_pc + 0x0e);
			uint16_t add_val = m_file->read_word(m_pc + 0x0f);

			if (cmp_sub_type == 0x02 && m_dec_table.entries.size() == 0)
				osd_printf_error("invalid n-bit compressed stream, no decompression table\n");
			else if (cmp_sub_type == 0x02 && (m_dec_table.cmp_type != cmp_type || m_dec_table.cmp_sub_type != cmp_sub_type || m_dec_table.bit_dec != bit_dec || m_dec_table.bit_cmp != bit_cmp ))
				osd_printf_error("invalid n-bit compressed stream, decompression table mismatch\n");
			else
			{
				int in_pos = 0x11 - 0x07;
				int in_shift = 0;
				int out_pos = 0;

				while (out_pos < out_size && in_pos < size)
				{
					int in_bits = bit_cmp;
					uint16_t in_val = 0;
					int out_bits = 0;

					while (in_bits > 0 && in_pos < size)
					{
						int bits = std::min(in_bits, 8);
						uint8_t mask = (1 << bits) - 1;
						in_shift += bits;
						in_bits -= bits;

						uint16_t val = (m_file->read_byte(m_pc + 0x7 + in_pos) << in_shift >> 8) & mask;
						if (in_shift >= 8)
						{
							in_pos++;

							in_shift -= 8;
							if (in_shift > 0)
							{
								if (in_pos < size)
									val |= (m_file->read_byte(m_pc + 0x7 + in_pos) << in_shift >> 8) & mask;
								else
									break;
							}
						}

						in_val |= val << out_bits;
						out_bits += bits;
					}

					if (out_bits == bit_cmp)
					{
						uint16_t out_val = 0;

						if (cmp_sub_type == 0)
							out_val = in_val + add_val;
						else if (cmp_sub_type == 1)
							out_val = (in_val << (bit_dec - bit_cmp)) + add_val;
						else if (cmp_sub_type == 2)
						{
							if (bit_dec <= 8)
								out_val = m_dec_table.entries[in_val];
							else if (bit_dec <= 16)
								out_val = m_dec_table.entries[in_val << 1] | (m_dec_table.entries[(in_val << 1) + 1] << 8);
						}
						else
							osd_printf_error("invalid n-bit compressed stream size %x->%x type %02x bit_dec %02x bit_cmp %02x unsupported cmp_sub_type %02x add_val %04x\n", size, out_size, type - 0x40, bit_dec, bit_cmp, cmp_sub_type, add_val);

						for (int i = 0; i < bit_dec; i += 8)
						{
							m_data_streams[type - 0x40][start + out_pos] = out_val;
							out_pos++;
							out_val >>= 8;
						}
					}
				}

				if (out_pos != out_size)
					osd_printf_error("invalid n-bit compressed stream %02x in %x/%x out %x/%x\n", type - 0x40, in_pos, size, out_pos, out_size);
			}
		}
		else if (cmp_type == 1)
			osd_printf_error("unhandled delta-t compressed stream size %x->%x type %02x\n", size, out_size, type - 0x40);
		else
			osd_printf_error("unhandled unknown %02x compressed stream size %x->%x type %02x\n", cmp_type, size, out_size, type - 0x40);
	}
	else if (type == 0x7f)
	{
		m_dec_table.cmp_type = m_file->read_byte(m_pc + 0x07);
		m_dec_table.cmp_sub_type = m_file->read_byte(m_pc + 0x08);
		m_dec_table.bit_dec = m_file->read_byte(m_pc + 0x09);
		m_dec_table.bit_cmp = m_file->read_byte(m_pc + 0x0a);

		m_dec_table.entries.resize(m_file->read_word(m_pc + 0x0b) * ((m_dec_table.bit_dec + 7) / 8));
		for (size_t i = 0; i < m_dec_table.entries.size(); i++)
			m_dec_table.entries[i] = m_file->read_byte(m_pc + 0x0d + i);
	}
	else if (type < 0xc0)
	{
		uint32_t rom_size = m_file->read_dword(m_pc + 7);
		uint32_t start = m_file->read_dword(m_pc + 11);

		uint32_t data_size = start < rom_size ? std::min(size - 8, rom_size - start) : 0;

		if (data_size)
		{
			std::unique_ptr<uint8_t[]> block = std::make_unique<uint8_t[]>(data_size);
			for (uint32_t i = 0; i < data_size; i++)
				block[i] = m_file->read_byte(m_pc + 15 + i);
			m_rom_blocks[second][type - 0x80].emplace_front(start, start + size - 9, std::move(block));
		}
	}
	else if (type <= 0xc2)
	{
		uint16_t start = m_file->read_word(m_pc + 7);
		uint32_t data_size = size - 2;
		if (type == 0xc0)
			for (int i = 0; i < data_size; i++)
				m_io->write_byte(A_RF5C68_RAM + start + i, m_file->read_byte(m_pc + 9 + i));
		else if (type == 0xc1)
			for (int i = 0; i < data_size; i++)
				m_io->write_byte(A_RF5C164_RAM + start + i, m_file->read_byte(m_pc + 9 + i));
		else if (type == 0xc2)
			for (int i = 0; i < data_size; i++)
				m_io->write_byte((second ? A_NES_RAM_1 : A_NES_RAM_0) + start + i, m_file->read_byte(m_pc + 9 + i));
	}
	else if (type >= 0xe0 && type <= 0xe1)
	{
		uint32_t start = m_file->read_dword(m_pc + 7);
		uint32_t data_size = size - 4;
		if (type == 0xe0)
			for (int i = 0; i < data_size; i++)
				m_io16be->write_byte((second ? A_SCSP_RAM_1 : A_SCSP_RAM_0) + ((start + i) ^ 1), m_file->read_byte(m_pc + 0xb + i));
		else if (type == 0xe1)
			for (int i = 0; i < data_size; i++)
				m_io->write_byte((second ? A_ES5503_RAM_1 : A_ES5503_RAM_0) + start + i, m_file->read_byte(m_pc + 0xb + i));
	}
	else
	{
		osd_printf_error("unhandled ram block size %x type %02x\n", size, type);
	}
	return 7 + size;
}

uint32_t vgmplay_device::handle_pcm_write(uint32_t address)
{
	uint8_t type = m_file->read_byte(m_pc + 2);
	uint32_t src = m_file->read_dword(m_pc + 3) & 0xffffff;
	uint32_t dst = m_file->read_dword(m_pc + 6) & 0xffffff;
	uint32_t size = m_file->read_dword(m_pc + 9) & 0xffffff;
	if (size == 0) size = 0x01000000;

	int second = (type & 0x80) ? 1 : 0;
	type &= 0x7f;

	if (m_data_streams.size() <= type || m_data_streams[type].size() < src + size)
		osd_printf_error("invalid pcm ram writes src %x dst %x size %x type %02x\n", src, dst, size, type);
	else if (type == 0x01 && !second)
	{
		for (int i = 0; i < size; i++)
			m_io->write_byte(A_RF5C68_RAM + dst + i, m_data_streams[type][src + i]);
	}
	else if (type == 0x02 && !second)
	{
		for (int i = 0; i < size; i++)
			m_io->write_byte(A_RF5C164_RAM + dst + i, m_data_streams[type][src + i]);
	}
	else if (type == 0x06)
	{
		for (int i = 0; i < size; i++)
			m_io16be->write_byte((second ? A_SCSP_RAM_1 : A_SCSP_RAM_0) + ((dst + i) ^ 1), m_data_streams[type][src + i]);
	}
	else if (type == 0x07)
	{
		for (int i = 0; i < size; i++)
			m_io->write_byte((second ? A_NES_RAM_1 : A_NES_RAM_0) + dst + i, m_data_streams[type][src + i]);
	}
	else
		osd_printf_error("unhandled pcm ram writes src %x dst %x size %x type %02x\n", src, dst, size, type);

	return 12;
}

TIMER_CALLBACK_MEMBER(vgmplay_device::stream_timer_expired)
{
	stream& s(m_streams[param]);

	uint32_t offset = s.offset;
	if (s.reverse)
		offset += (s.length - s.position - 1) * s.step_size * s.byte_depth;
	else
		offset += s.position * s.step_size * s.byte_depth;

	if (offset + s.byte_depth > m_data_streams[s.bank].size())
	{
		osd_printf_error("stream_timer_expired %02x: stream beyond end %d/%d %u>=%u\n", param, s.position, s.length, offset, uint32_t(m_data_streams[s.bank].size()));
		s.timer->enable(false);
	}
	else if (s.chip_type == CT_SN76489)
	{
		m_io->write_byte(A_SN76489_0, (s.reg & 0xf0) | (m_data_streams[s.bank][offset] & 0xf));
		if ((s.reg & 0x10) == 0)
			m_io->write_byte(A_SN76489_0, ((m_data_streams[s.bank][offset + 1] & 3) << 4) | (m_data_streams[s.bank][offset] >> 4));
	}
	else if (s.chip_type == CT_YM2612)
	{
		m_io->write_byte(A_YM2612_0 + 0 + ((s.port & 1) << 1), s.reg);
		m_io->write_byte(A_YM2612_0 + 1 + ((s.port & 1) << 1), m_data_streams[s.bank][offset]);
	}
	else if (s.chip_type == CT_YM2203)
	{
		m_io->write_byte(A_YM2203_0 + 0 + ((s.port & 1) << 1), s.reg);
		m_io->write_byte(A_YM2203_0 + 1 + ((s.port & 1) << 1), m_data_streams[s.bank][offset]);
	}
	else if (s.chip_type == CT_YM2608)
	{
		m_io->write_byte(A_YM2608_0 + 0 + ((s.port & 1) << 1), s.reg);
		m_io->write_byte(A_YM2608_0 + 1 + ((s.port & 1) << 1), m_data_streams[s.bank][offset]);
	}
	else if (s.chip_type == CT_SEGA32X)
	{
		if (m_sega32x_channel_hack >= 0)
		{
			osd_printf_error("bad rip detected, enabling sega32x channels\n");
			m_io16le->write_word(A_32X_PWM, 5);

			m_sega32x_channel_hack = -2;
		}

		m_io16le->write_word(A_32X_PWM + (s.reg << 1), ((m_data_streams[s.bank][offset + 1] & 0xf) << 8) | m_data_streams[s.bank][offset]);
	}
	else if (s.chip_type == CT_C6280)
	{
		if (s.port != 0xff)
			m_io->write_byte(A_C6280_0 + (s.reg >> 4), s.port);

		m_io->write_byte(A_C6280_0 + (s.reg & 0xf), m_data_streams[s.bank][offset]);

		if (s.port != 0xff && s.port != m_c6280_channel[0])
			m_io->write_byte(A_C6280_0 + (s.reg >> 4), m_c6280_channel[0]);
	}
	else if (s.chip_type == CT_OKIM6258)
		m_io->write_byte(A_OKIM6258_0 + s.reg, m_data_streams[s.bank][offset]);
	else
	{
		osd_printf_error("stream_timer_expired %02x: unsupported stream to chip %02x\n", param, s.chip_type);
		s.timer->enable(false);
	}

	s.position++;
	if (s.position >= s.length)
	{
		if (s.loop)
		{
			pulse_act_led(s.chip_type);
			s.position = 0;
		}
		else
			s.timer->enable(false);
	}
}

void vgmplay_device::execute_run()
{
	while (m_icount > 0)
	{
		switch (m_state)
		{
		case RESET:
		{
			uint32_t size = m_io->read_dword(REG_SIZE);
			if (!size)
			{
				logerror("zero length file\n");

				m_pc = 0;
				m_state = DONE;
				break;
			}

			uint32_t version = m_file->read_dword(8);
			m_pc = 0x34 + m_file->read_dword(0x34);

			if ((version < 0x150 && m_pc != 0x34) || (version >= 0x150 && m_pc == 0x34))
			{
				osd_printf_error("bad rip detected, v%x invalid header size 0x%x\n", version, m_pc);
				m_pc = 0x40;
			}
			else if (version < 0x150)
			{
				m_pc = 0x40;
			}

			m_state = RUN;
			break;
		}
		case RUN:
		{
			if (m_paused)
			{
				machine().sound().system_mute(1);
				m_icount = 0;
				return;
			}
			else
			{
				machine().sound().system_mute(0);
			}

			if (machine().debug_flags & DEBUG_FLAG_ENABLED)
				debugger_instruction_hook(m_pc);

			uint8_t code = m_file->read_byte(m_pc);
			switch (code)
			{
			case 0x30:
				pulse_act_led(CT_SN76489);
				m_io->write_byte(A_SN76489_1 + 0, m_file->read_byte(m_pc + 1));
				m_pc += 2;
				break;

			case 0x3f:
				pulse_act_led(CT_SN76489);
				m_io->write_byte(A_SN76489_1 + 1, m_file->read_byte(m_pc + 1));
				m_pc += 2;
				break;

			case 0x4f:
				pulse_act_led(CT_SN76489);
				m_io->write_byte(A_SN76489_0 + 1, m_file->read_byte(m_pc + 1));
				m_pc += 2;
				break;

			case 0x50:
				pulse_act_led(CT_SN76489);
				m_io->write_byte(A_SN76489_0 + 0, m_file->read_byte(m_pc + 1));
				m_pc += 2;
				break;

			case 0x51:
				pulse_act_led(CT_YM2413);
				m_io->write_byte(A_YM2413_0 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2413_0 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x52:
			case 0x53:
				pulse_act_led(CT_YM2612);
				m_io->write_byte(A_YM2612_0 + 0 + ((code & 1) << 1), m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2612_0 + 1 + ((code & 1) << 1), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x54:
				pulse_act_led(CT_YM2151);
				m_io->write_byte(A_YM2151_0 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2151_0 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x55:
				pulse_act_led(CT_YM2203);
				m_io->write_byte(A_YM2203_0 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2203_0 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x56:
			case 0x57:
				pulse_act_led(CT_YM2608);
				m_io->write_byte(A_YM2608_0 + 0 + ((code & 1) << 1), m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2608_0 + 1 + ((code & 1) << 1), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x58:
			case 0x59:
				pulse_act_led(CT_YM2610);
				m_io->write_byte(A_YM2610_0 + 0 + ((code & 1) << 1), m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2610_0 + 1 + ((code & 1) << 1), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x5a:
				pulse_act_led(CT_YM3812);
				m_io->write_byte(A_YM3812_0 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM3812_0 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x5b:
				pulse_act_led(CT_YM3526);
				m_io->write_byte(A_YM3526_0 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM3526_0 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x5c:
				pulse_act_led(CT_Y8950);
				m_io->write_byte(A_Y8950_0 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_Y8950_0 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x5d:
				pulse_act_led(CT_YMZ280B);
				m_io->write_byte(A_YMZ280B_0 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YMZ280B_0 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x5e:
			case 0x5f:
				pulse_act_led(CT_YMF262);
				m_io->write_byte(A_YMF262_0 + 0 + ((code & 1) << 1), m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YMF262_0 + 1 + ((code & 1) << 1), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x61:
			{
				uint32_t duration = m_file->read_word(m_pc + 1);
				m_icount -= duration;
				m_pc += 3;
				break;
			}

			case 0x62:
				m_icount -= 735;
				m_pc++;
				break;

			case 0x63:
				m_icount -= 882;
				m_pc++;
				break;

			case 0x66:
			{
				uint32_t loop_offset = m_file->read_dword(0x1c);
				if (!loop_offset)
				{
					if (m_loop)
						device_reset();
					else
					{
						logerror("done\n");
						m_state = DONE;
					}
					break;
				}

				m_pc = 0x1c + loop_offset;
				break;
			}

			case 0x67:
				m_pc += handle_data_block(m_pc);
				break;

			case 0x68:
				m_pc += handle_pcm_write(m_pc);
				break;

			case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
			case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
				m_icount -= 1 + (code & 0xf);
				m_pc += 1;
				break;

			case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
			case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
				pulse_act_led(CT_YM2612);
				if (!m_data_streams[0].empty())
				{
					if (m_ym2612_stream_offset >= int(m_data_streams[0].size()))
						m_ym2612_stream_offset = 0;

					m_io->write_byte(A_YM2612_0 + 0, 0x2a);
					m_io->write_byte(A_YM2612_0 + 1, m_data_streams[0][m_ym2612_stream_offset]);
					m_ym2612_stream_offset++;
				}
				m_pc += 1;
				m_icount -= code & 0xf;
				break;

			case 0x90:
			{
				uint8_t id = m_file->read_byte(m_pc + 1);
				if (id == 0xff)
					osd_printf_error("stream control invalid id\n");
				else
				{
					stream& s(m_streams[id]);

					s.chip_type = vgm_chip(m_file->read_byte(m_pc + 2));
					s.port = m_file->read_byte(m_pc + 3);
					s.reg = m_file->read_byte(m_pc + 4);

					s.byte_depth = ((s.chip_type == CT_SN76489 && (s.reg & 0x10) == 0) || s.chip_type == CT_SEGA32X) ? 2 : 1;

					if (s.timer->enabled())
					{
						osd_printf_error("stream %02x control while playing\n", id);
						s.timer->enable(false);
					}
				}
				m_pc += 5;
				break;
			}

			case 0x91:
			{
				uint8_t id = m_file->read_byte(m_pc + 1);
				if (id == 0xff)
					osd_printf_error("stream data invalid id\n");
				else
				{
					stream& s(m_streams[id]);

					s.bank = m_file->read_byte(m_pc + 2);
					s.step_size = m_file->read_byte(m_pc + 3);
					s.step_base = m_file->read_byte(m_pc + 4);

					if (s.step_size == 0)
					{
						osd_printf_error("stream %02x data invalid step size %d\n", id, s.step_size);
						s.step_size = 1;
					}

					if (s.step_base >= s.step_size)
					{
						osd_printf_error("stream %02x data step size %d invalid step base %d\n", id, s.step_size, s.step_base);
						s.step_base %= s.step_size;
					}

					if (s.timer->enabled())
					{
						osd_printf_error("stream %02x data while playing\n", id);
						s.timer->enable(false);
					}
				}
				m_pc += 5;
				break;
			}

			case 0x92:
			{
				uint8_t id = m_file->read_byte(m_pc + 1);
				if (id == 0xff)
					osd_printf_error("stream frequency invalid id\n");
				else
				{
					stream& s(m_streams[id]);

					s.frequency = m_file->read_dword(m_pc + 2);

					if (s.timer->enabled())
					{
						osd_printf_error("stream %02x frequency %d while playing\n", id, s.frequency);
						s.timer->enable(false);
					}
				}
				m_pc += 6;
				break;
			}

			case 0x93:
			{
				uint8_t id = m_file->read_byte(m_pc + 1);
				if (id == 0xff)
					osd_printf_error("stream start invalid id\n");
				else if (m_streams[id].chip_type >= CT_COUNT)
					osd_printf_error("stream start %02x invalid chip type %02x\n", id, m_streams[id].chip_type);
				else
				{
					stream& s(m_streams[id]);

					pulse_act_led(s.chip_type);

					uint32_t offset = m_file->read_dword(m_pc + 2);
					uint8_t flags = m_file->read_byte(m_pc + 6);
					uint32_t length = m_file->read_dword(m_pc + 7);

					if (s.bank >= m_data_stream_blocks.size())
						osd_printf_error("stream start %02x invalid bank %u>=%u\n", id, s.bank, uint8_t(m_data_stream_blocks.size()));
					else if (s.frequency == 0)
						osd_printf_error("stream start %02x invalid frequency\n", id);
					else
					{
						if (offset != 0xffffffff)
							s.offset = offset + (s.step_base * s.byte_depth);

						s.reverse = BIT(flags, 4);
						s.loop = BIT(flags, 7);

						switch (flags & 3)
						{
						case 0:
							break;
						case 1:
							s.length = length;
							break;
						case 2:
							s.length = (length * 1000) / s.frequency;
							break;
						case 3:
							s.length = (m_data_streams[s.bank].size() - (s.offset - (s.step_base * s.byte_depth))) / (s.step_size * s.byte_depth);
							break;
						}

						s.position = 0;
						s.timer->adjust(attotime::zero, id, attotime::from_hz(s.frequency));
					}
				}
				m_pc += 11;
				break;
			}

			case 0x94:
			{
				uint8_t id = m_file->read_byte(m_pc + 1);
				if (id == 0xff)
					for (int i = 0; i < 0xff; i++)
						m_streams[id].timer->enable(false);
				else
					m_streams[id].timer->enable(false);

				m_pc += 2;
				break;
			}

			case 0x95:
			{
				uint8_t id = m_file->read_byte(m_pc + 1);
				if (id == 0xff)
					osd_printf_error("stream start short invalid id\n");
				else if (m_streams[id].chip_type >= CT_COUNT)
					osd_printf_error("stream start short %02x invalid chip type %02x\n", id, m_streams[id].chip_type);
				else
				{
					stream& s(m_streams[id]);

					pulse_act_led(s.chip_type);

					uint8_t block = m_file->read_word(m_pc + 2);
					uint8_t flags = m_file->read_byte(m_pc + 4);

					if (s.bank >= m_data_stream_blocks.size())
						osd_printf_error("stream start short %02x invalid bank %u>=%u\n", id, s.bank, uint8_t(m_data_stream_blocks.size()));
					else if (block >= m_data_stream_blocks[s.bank].size())
						osd_printf_error("stream start short %02x bank %u invalid block %u>=%u\n", id, s.bank, block, uint8_t(m_data_stream_blocks[s.bank].size()));
					else if (s.frequency == 0)
						osd_printf_error("stream start %02x invalid frequency\n", id);
					else
					{
						s.loop = BIT(flags, 0);
						s.reverse = BIT(flags, 4);
						s.offset = m_data_stream_blocks[s.bank][block].start + (s.step_base * s.byte_depth);
						s.length = m_data_stream_blocks[s.bank][block].size / (s.step_size * s.byte_depth);

						s.position = 0;
						s.timer->adjust(attotime::zero, id, attotime::from_hz(s.frequency));
					}
				}
				m_pc += 5;
				break;
			}

			case 0xa0:
			{
				pulse_act_led(CT_AY8910);
				uint8_t reg = m_file->read_byte(m_pc + 1);
				if (reg & 0x80)
				{
					m_io->write_byte(A_AY8910_1 + 1, reg & 0x7f);
					m_io->write_byte(A_AY8910_1 + 0, m_file->read_byte(m_pc + 2));
				}
				else
				{
					m_io->write_byte(A_AY8910_0 + 1, reg & 0x7f);
					m_io->write_byte(A_AY8910_0 + 0, m_file->read_byte(m_pc + 2));
				}
				m_pc += 3;
				break;
			}

			case 0xa1:
				pulse_act_led(CT_YM2413);
				m_io->write_byte(A_YM2413_1 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2413_1 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xa2:
			case 0xa3:
				pulse_act_led(CT_YM2612);
				m_io->write_byte(A_YM2612_1 + 0 + ((code & 1) << 1), m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2612_1 + 1 + ((code & 1) << 1), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xa4:
				pulse_act_led(CT_YM2151);
				m_io->write_byte(A_YM2151_1 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2151_1 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xa5:
				pulse_act_led(CT_YM2203);
				m_io->write_byte(A_YM2203_1 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2203_1 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xa6:
			case 0xa7:
				pulse_act_led(CT_YM2608);
				m_io->write_byte(A_YM2608_0 + 0 + ((code & 1) << 1), m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2608_0 + 1 + ((code & 1) << 1), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xa8:
			case 0xa9:
				pulse_act_led(CT_YM2610);
				m_io->write_byte(A_YM2610_0 + 0 + ((code & 1) << 1), m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2610_0 + 1 + ((code & 1) << 1), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xaa:
				pulse_act_led(CT_YM3812);
				m_io->write_byte(A_YM3812_1 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM3812_1 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xab:
				pulse_act_led(CT_YM3526);
				m_io->write_byte(A_YM3526_1 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM3526_1 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xac:
				pulse_act_led(CT_Y8950);
				m_io->write_byte(A_Y8950_1 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_Y8950_1 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xad:
				pulse_act_led(CT_YMZ280B);
				m_io->write_byte(A_YMZ280B_1 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YMZ280B_1 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xae:
			case 0xaf:
				pulse_act_led(CT_YMF262);
				m_io->write_byte(A_YMF262_1 + 0 + ((code & 1) << 1), m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YMF262_1 + 1 + ((code & 1) << 1), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xb0:
				pulse_act_led(CT_RF5C68);
				m_io->write_byte(A_RF5C68 + m_file->read_byte(m_pc + 1), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xb1:
				pulse_act_led(CT_RF5C164);
				m_io->write_byte(A_RF5C164 + m_file->read_byte(m_pc + 1), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xb2:
			{
				pulse_act_led(CT_SEGA32X);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				uint8_t data = m_file->read_byte(m_pc + 2);

				if (m_sega32x_channel_hack >= 0)
				{
					if ((offset & 0xf0) == 0)
					{
						if (data != 0)
							m_sega32x_channel_hack = -1;
					}
					else
					{
						m_sega32x_channel_hack++;
						if (m_sega32x_channel_hack == 32)
						{
							osd_printf_error("bad rip detected, enabling sega32x channels\n");
							m_io16le->write_word(A_32X_PWM, 5);

							m_sega32x_channel_hack = -2;
						}
					}
				}

				m_io16le->write_word(A_32X_PWM + ((offset & 0xf0) >> 3), ((offset & 0xf) << 8) | data);
				m_pc += 3;
				break;
			}

			case 0xb3:
			{
				pulse_act_led(CT_GAMEBOY);
				uint8_t reg = m_file->read_byte(m_pc + 1);
				if (reg & 0x80)
					m_io->write_byte(A_GAMEBOY_1 + (reg & 0x7f), m_file->read_byte(m_pc + 2));
				else
					m_io->write_byte(A_GAMEBOY_0 + (reg & 0x7f), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;
			}

			case 0xb4:
			{
				pulse_act_led(CT_NESAPU);
				uint8_t offset = m_file->read_byte(m_pc + 1);

				int index = offset & 0x80 ? 1 : 0;
				if (m_nes_apu_channel_hack[index] >= 0)
				{
					if ((offset & 0x7f) == 0x15)
					{
						if ((m_file->read_byte(m_pc + 2) & 0x1f) != 0)
							m_nes_apu_channel_hack[index] = -1;
					}
					else
					{
						m_nes_apu_channel_hack[index]++;
						if (m_nes_apu_channel_hack[index] == 32)
						{
							osd_printf_error("bad rip detected, enabling nesapu.%d channels\n", index);
							if (index)
								m_io->write_byte(A_NESAPU_1 + 0x15, 0x0f);
							else
								m_io->write_byte(A_NESAPU_0 + 0x15, 0x0f);

							m_nes_apu_channel_hack[index] = -2;
						}
					}
				}
				//else if ((offset & 0x7f) == 0x15 && m_nes_apu_channel_hack[index] == -2 && (m_file->read_byte(m_pc + 2) & 0x1f) != 0)
				//{
				//  osd_printf_error("bad rip false positive, late enabling nesapu.%d channels %x/%x\n", index, m_pc, m_io->read_dword(REG_SIZE));
				//  m_nes_apu_channel_hack[index] = -1;
				//}

				if (offset & 0x80)
					m_io->write_byte(A_NESAPU_1 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				else
					m_io->write_byte(A_NESAPU_0 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;
			}

			case 0xb5:
			{
				pulse_act_led(CT_MULTIPCM);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
					m_io->write_byte(A_MULTIPCM_1 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				else
					m_io->write_byte(A_MULTIPCM_0 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;
			}

			case 0xb6:
			{
				pulse_act_led(CT_UPD7759);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
					m_io->write_byte(A_UPD7759_1 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				else
					m_io->write_byte(A_UPD7759_0 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;
			}

			case 0xb7:
			{
				pulse_act_led(CT_OKIM6258);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
					m_io->write_byte(A_OKIM6258_1 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				else
					m_io->write_byte(A_OKIM6258_0 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;
			}

			case 0xb8:
			{
				pulse_act_led(CT_OKIM6295);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
					m_io->write_byte(A_OKIM6295_1 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				else
					m_io->write_byte(A_OKIM6295_0 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;
			}

			case 0xb9:
			{
				pulse_act_led(CT_C6280);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if ((offset & 0x7f) == 0)
					m_c6280_channel[BIT(offset, 7)] = m_file->read_byte(m_pc + 2);
				if (offset & 0x80)
					m_io->write_byte(A_C6280_1 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				else
					m_io->write_byte(A_C6280_0 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;
			}

			case 0xba:
			{
				pulse_act_led(CT_K053260);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
					m_io->write_byte(A_K053260_1 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				else
					m_io->write_byte(A_K053260_0 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;
			}

			case 0xbb:
			{
				pulse_act_led(CT_POKEY);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
					m_io->write_byte(A_POKEY_1 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				else
					m_io->write_byte(A_POKEY_0 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;
			}

			case 0xbc:
			{
				pulse_act_led(CT_WSWAN);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
					m_io->write_byte(A_WSWAN_1 + (offset & 0x7f) + 0x80, m_file->read_byte(m_pc + 2));
				else
					m_io->write_byte(A_WSWAN_0 + (offset & 0x7f) + 0x80, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;
			}

			case 0xbd:
			{
				pulse_act_led(CT_SAA1099);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
				{
					m_io->write_byte(A_SAA1099_1 + 1, offset & 0x7f);
					m_io->write_byte(A_SAA1099_1 + 0, m_file->read_byte(m_pc + 2));
				}
				else
				{
					m_io->write_byte(A_SAA1099_0 + 1, offset & 0x7f);
					m_io->write_byte(A_SAA1099_0 + 0, m_file->read_byte(m_pc + 2));
				}
				m_pc += 3;
				break;
			}

			case 0xbe:
			{
				pulse_act_led(CT_ES5505);
				// TODO: es5505
				m_pc += 3;
				break;
			}

			case 0xbf:
			{
				pulse_act_led(CT_GA20);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
					m_io->write_byte(A_GA20_1 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				else
					m_io->write_byte(A_GA20_0 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;
			}

			case 0xc0:
			{
				pulse_act_led(CT_SEGAPCM);
				uint16_t offset = m_file->read_word(m_pc + 1);
				if (offset & 0x8000)
					m_io->write_byte(A_SEGAPCM_1 + (offset & 0x7fff), m_file->read_byte(m_pc + 3));
				else
					m_io->write_byte(A_SEGAPCM_0 + (offset & 0x7fff), m_file->read_byte(m_pc + 3));
				m_pc += 4;
				break;
			}

			case 0xc1:
				pulse_act_led(CT_RF5C68);
				m_io->write_byte(A_RF5C68_RAM + m_file->read_word(m_pc + 1), m_file->read_byte(m_pc + 3));
				m_pc += 4;
				break;

			case 0xc2:
				pulse_act_led(CT_RF5C164);
				m_io->write_byte(A_RF5C164_RAM + m_file->read_word(m_pc + 1), m_file->read_byte(m_pc + 3));
				m_pc += 4;
				break;

			case 0xc3:
			{
				pulse_act_led(CT_MULTIPCM);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
				{
					m_io->write_byte(A_MULTIPCM_1 + 4 + (offset & 0x7f), m_file->read_byte(m_pc + 3));
					m_io->write_byte(A_MULTIPCM_1 + 8 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				}
				else
				{
					m_io->write_byte(A_MULTIPCM_0 + 4 + (offset & 0x7f), m_file->read_byte(m_pc + 3));
					m_io->write_byte(A_MULTIPCM_0 + 8 + (offset & 0x7f), m_file->read_byte(m_pc + 2));
				}
				m_pc += 4;
				break;
			}

			case 0xc4:
				pulse_act_led(CT_QSOUND);
				m_io->write_byte(A_QSOUND + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_QSOUND + 1, m_file->read_byte(m_pc + 2));
				m_io->write_byte(A_QSOUND + 2, m_file->read_byte(m_pc + 3));
				m_pc += 4;
				break;

			case 0xc5:
			{
				pulse_act_led(CT_SCSP);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
					m_io16be->write_byte(A_SCSP_1 + ((offset & 0x7f) << 8) + (m_file->read_byte(m_pc + 2) ^ 1), m_file->read_byte(m_pc + 3));
				else
					m_io16be->write_byte(A_SCSP_0 + ((offset & 0x7f) << 8) + (m_file->read_byte(m_pc + 2) ^ 1), m_file->read_byte(m_pc + 3));
				m_pc += 4;
				break;
			}

			case 0xc6:
			{
				pulse_act_led(CT_WSWAN);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
					m_io->write_byte(A_WSWAN_RAM_1 + ((offset & 0x7f) << 8) + m_file->read_byte(m_pc + 2), m_file->read_byte(m_pc + 3));
				else
					m_io->write_byte(A_WSWAN_RAM_0 + ((offset & 0x7f) << 8) + m_file->read_byte(m_pc + 2), m_file->read_byte(m_pc + 3));
				m_pc += 4;
				break;
			}

			case 0xc7:
			{
				pulse_act_led(CT_VSU_VUE);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
					m_io->write_byte(A_VSU_VUE_1 + ((offset & 0x7f) << 10) + (m_file->read_byte(m_pc + 2) << 2), m_file->read_byte(m_pc + 3));
				else
					m_io->write_byte(A_VSU_VUE_0 + ((offset & 0x7f) << 10) + (m_file->read_byte(m_pc + 2) << 2), m_file->read_byte(m_pc + 3));
				m_pc += 4;
				break;
			}

			case 0xc8:
			{
				pulse_act_led(CT_X1_010);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
					m_io->write_byte(A_X1_010_1 + ((offset & 0x7f) << 8) + m_file->read_byte(m_pc + 2), m_file->read_byte(m_pc + 3));
				else
					m_io->write_byte(A_X1_010_0 + ((offset & 0x7f) << 8) + m_file->read_byte(m_pc + 2), m_file->read_byte(m_pc + 3));
				m_pc += 4;
				break;
			}

			case 0xd0:
			{
				pulse_act_led(CT_YMF278B);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
				{
					m_io->write_byte(A_YMF278B_1 + ((offset & 0x7f) << 1) + 0, m_file->read_byte(m_pc + 2));
					m_io->write_byte(A_YMF278B_1 + ((offset & 0x7f) << 1) + 1, m_file->read_byte(m_pc + 3));
				}
				else
				{
					m_io->write_byte(A_YMF278B_0 + ((offset & 0x7f) << 1) + 0, m_file->read_byte(m_pc + 2));
					m_io->write_byte(A_YMF278B_0 + ((offset & 0x7f) << 1) + 1, m_file->read_byte(m_pc + 3));
				}
				m_pc += 4;
				break;
			}

			case 0xd1:
			{
				pulse_act_led(CT_YMF271);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
				{
					m_io->write_byte(A_YMF271_1 + ((offset & 0x7f) << 1) + 0, m_file->read_byte(m_pc + 2));
					m_io->write_byte(A_YMF271_1 + ((offset & 0x7f) << 1) + 1, m_file->read_byte(m_pc + 3));
				}
				else
				{
					m_io->write_byte(A_YMF271_0 + ((offset & 0x7f) << 1) + 0, m_file->read_byte(m_pc + 2));
					m_io->write_byte(A_YMF271_0 + ((offset & 0x7f) << 1) + 1, m_file->read_byte(m_pc + 3));
				}
				m_pc += 4;
				break;
			}

			case 0xd2:
			{
				pulse_act_led(CT_K051649);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
				{
					m_io->write_byte(A_K051649_1 + ((offset & 0x7f) << 1) + 0, m_file->read_byte(m_pc + 2));
					m_io->write_byte(A_K051649_1 + ((offset & 0x7f) << 1) + 1, m_file->read_byte(m_pc + 3));
				}
				else
				{
					m_io->write_byte(A_K051649_0 + ((offset & 0x7f) << 1) + 0, m_file->read_byte(m_pc + 2));
					m_io->write_byte(A_K051649_0 + ((offset & 0x7f) << 1) + 1, m_file->read_byte(m_pc + 3));
				}
				m_pc += 4;
				break;
			}

			case 0xd3:
			{
				pulse_act_led(CT_K054539);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
					m_io->write_byte(A_K054539_1 + ((offset & 0x7f) << 8) + m_file->read_byte(m_pc + 2), m_file->read_byte(m_pc + 3));
				else
					m_io->write_byte(A_K054539_0 + ((offset & 0x7f) << 8) + m_file->read_byte(m_pc + 2), m_file->read_byte(m_pc + 3));
				m_pc += 4;
				break;
			}

			case 0xd4:
			{
				pulse_act_led(CT_C140);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
					m_io->write_byte(A_C140_1 + ((offset & 0x7f) << 8) + m_file->read_byte(m_pc + 2), m_file->read_byte(m_pc + 3));
				else
					m_io->write_byte(A_C140_0 + ((offset & 0x7f) << 8) + m_file->read_byte(m_pc + 2), m_file->read_byte(m_pc + 3));
				m_pc += 4;
				break;
			}

			case 0xd5:
			{
				pulse_act_led(CT_ES5503);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
					m_io->write_byte(A_ES5503_1 + ((offset & 0x7f) << 8) + m_file->read_byte(m_pc + 2), m_file->read_byte(m_pc + 3));
				else
					m_io->write_byte(A_ES5503_0 + ((offset & 0x7f) << 8) + m_file->read_byte(m_pc + 2), m_file->read_byte(m_pc + 3));
				m_pc += 4;
				break;
			}

			case 0xd6:
			{
				pulse_act_led(CT_ES5505);
				// TODO: es5505
				m_pc += 4;
				break;
			}

			case 0xe0:
				pulse_act_led(CT_YM2612);
				m_ym2612_stream_offset = m_file->read_dword(m_pc + 1);
				m_pc += 5;
				break;

			case 0xe1:
			{
				pulse_act_led(CT_C352);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				uint32_t addr = ((offset & 0x7f) << 8) + m_file->read_byte(m_pc + 2);
				uint16_t data = (m_file->read_byte(m_pc + 3) << 8) + m_file->read_byte(m_pc + 4);
				if (offset & 0x80)
					m_io16le->write_word(A_C352_1 + (addr << 1), data);
				else
					m_io16le->write_word(A_C352_0 + (addr << 1), data);
				m_pc += 5;
				break;
			}

			default:
				osd_printf_error("unhandled code %02x (%02x %02x %02x %02x)\n", code, m_file->read_byte(m_pc + 1), m_file->read_byte(m_pc + 2), m_file->read_byte(m_pc + 3), m_file->read_byte(m_pc + 4));

				if (machine().debug_flags & DEBUG_FLAG_ENABLED)
					debugger_instruction_hook(m_pc);

				m_state = DONE;
				m_icount = 0;
				break;
			}
			break;
		}
		case DONE:
		{
			machine().sound().system_mute(1);
			m_icount = 0;
			break;
		}
		}
	}
}

void vgmplay_device::execute_set_input(int inputnum, int state)
{
}

device_memory_interface::space_config_vector vgmplay_device::memory_space_config() const
{
	return space_config_vector
	{
		std::make_pair(AS_PROGRAM, &m_file_config),
		std::make_pair(AS_IO,      &m_io_config),
		std::make_pair(AS_IO16LE,  &m_io16le_config),
		std::make_pair(AS_IO16BE,  &m_io16be_config),
	};
}

void vgmplay_device::state_import(const device_state_entry &entry)
{
}

void vgmplay_device::state_export(const device_state_entry &entry)
{
}

void vgmplay_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
}

std::unique_ptr<util::disasm_interface> vgmplay_device::create_disassembler()
{
	return std::make_unique<vgmplay_disassembler>();
}

uint32_t vgmplay_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t vgmplay_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	static const char *const basic_types[8] =
	{
		"ym2612.%d pcm",
		"rf5c68 pcm",
		"rf5c164 pcm",
		"sega32x.%d pcm",
		"okim6258.%d adpcm",
		"huc6280.%d pcm",
		"scsp.%d pcm",
		"nesapu.%d dpcm",
	};

	switch (opcodes.r8(pc))
	{
	case 0x30:
		util::stream_format(stream, "psg.1 write %02x", opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x3f:
		util::stream_format(stream, "psg.1 r06 = %02x", opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x4f:
		util::stream_format(stream, "psg.0 r06 = %02x", opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x50:
		util::stream_format(stream, "psg.0 write %02x", opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x51:
		util::stream_format(stream, "ym2413.0 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x52:
	case 0x53:
		util::stream_format(stream, "ym2612.0 %d r%02x = %02x", opcodes.r8(pc) & 1, opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x54:
		util::stream_format(stream, "ym2151.0 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x55:
		util::stream_format(stream, "ym2203.0 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x56:
	case 0x57:
		util::stream_format(stream, "ym2608.0 %d r%02x = %02x", opcodes.r8(pc) & 1, opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x58:
	case 0x59:
		util::stream_format(stream, "ym2610.0 %d r%02x = %02x", opcodes.r8(pc) & 1, opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x5a:
		util::stream_format(stream, "ym3812.0 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x5b:
		util::stream_format(stream, "ym3526.0 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x5c:
		util::stream_format(stream, "y8950.0 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x5d:
		util::stream_format(stream, "ymz280b.0 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x5e:
	case 0x5f:
		util::stream_format(stream, "ymf262.0 %d r%02x = %02x", opcodes.r8(pc) & 1, opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x61:
	{
		uint32_t duration = opcodes.r8(pc + 1) | (opcodes.r8(pc + 2) << 8);
		util::stream_format(stream, "wait %d", duration);
		return 3 | SUPPORTED;
	}

	case 0x62:
		util::stream_format(stream, "wait 735");
		return 1 | SUPPORTED;

	case 0x63:
		util::stream_format(stream, "wait 882");
		return 1 | SUPPORTED;

	case 0x66:
		util::stream_format(stream, "end");
		return 1 | SUPPORTED;

	case 0x67:
	{
		static const char *const rom_types[20] =
		{
			"segapcm.%d rom",
			"ym2608.%d delta-t rom",
			"ym2610.%d adpcm rom",
			"ym2610.%d delta-t rom",
			"ymf278b.%d rom",
			"ymf271.%d rom",
			"ymz280b.%d rom",
			"ymf278b.%d ram",
			"y8950.%d delta-t rom",
			"multipcm.%d rom",
			"upd7759.%d rom",
			"okim6295.%d rom",
			"k054539.%d rom",
			"c140.%d rom",
			"k053260.%d rom",
			"qsound.%d rom",
			"es5505.%d rom",
			"x1-010.%d rom",
			"c352.%d rom",
			"ga20.%d rom"
		};

		static const char *const small_ram_types[3] =
		{
			"rf5c68 ram",
			"rf5c164 ram",
			"nesapu.%d ram"
		};

		static const char *const large_ram_types[2] =
		{
			"scsp.%d ram",
			"es5503.%d ram"
		};

		uint8_t type = opcodes.r8(pc + 2);
		uint32_t size = opcodes.r32(pc + 3);
		int second = (size & 0x80000000) ? 1 : 0;
		size &= 0x7fffffff;

		if (type < 0x8)
		{
			util::stream_format(stream, basic_types[type], second);  util::stream_format(stream, " data-block %x", size);
		}
		else if (type < 0x40)
			util::stream_format(stream, "unknown%02x.%d stream data-block %x", type, second, size);
		else if (type < 0x48)
		{
			util::stream_format(stream, basic_types[type - 0x40], second); util::stream_format(stream, " comp. data-block %x", size);
		}
		else if (type < 0x7f)
			util::stream_format(stream, "unknown%02x.%d stream comp. data-block %x", type - 0x40, second, size);
		else if (type == 0x7f)
			util::stream_format(stream, "decomp-table %x, %02x/%02x", size, opcodes.r8(pc + 7), opcodes.r8(pc + 8));
		else if (type < 0x94)
		{
			util::stream_format(stream, rom_types[type - 0x80], second); util::stream_format(stream, " data-block %x", size);
		}
		else if (type < 0xc0)
			util::stream_format(stream, "unknown%02x.%d rom data-block %x", type - 0x80, second, size);
		else if (type < 0xc3)
		{
			util::stream_format(stream, small_ram_types[type - 0xc0], second);  util::stream_format(stream, " data-block %x", size);
		}
		else if (type < 0xe0)
			util::stream_format(stream, "unknown%02x.%d small ram data-block %x", type - 0xc0, second, size);
		else if (type < 0xe2)
		{
			util::stream_format(stream, large_ram_types[type - 0xe0], second); util::stream_format(stream, " data-block %x", size);
		}
		else
			util::stream_format(stream, "unknown%02x.%d large ram data-block %x", type - 0xe0, second, size);
		return (7 + size) | SUPPORTED;
	}

	case 0x68:
	{
		uint8_t type = opcodes.r8(pc + 2);
		uint32_t src = opcodes.r32(pc + 3) & 0xffffff;
		uint32_t dst = opcodes.r32(pc + 6) & 0xffffff;
		uint32_t size = opcodes.r32(pc + 9) & 0xffffff;
		if (size == 0) size = 0x01000000;
		int second = (type & 0x80) ? 1 : 0;

		if (type < 8)
		{
			util::stream_format(stream, basic_types[type], second); util::stream_format(stream, " write src %x dst %x size %x\n", src, dst, size);
		}
		else
			util::stream_format(stream, "unknown%02x.%d pcm write src %x dst %x size %x\n", type, second, src, dst, size);

		return 12 | SUPPORTED;
	}

	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
	case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
		util::stream_format(stream, "wait %d", 1 + (opcodes.r8(pc) & 0x0f));
		return 1 | SUPPORTED;

	case 0x80:
		util::stream_format(stream, "ym2612.0 r2a = rom++");
		return 1 | SUPPORTED;

	case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
	case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
		util::stream_format(stream, "ym2612.0 r2a = rom++; wait %d", opcodes.r8(pc) & 0xf);
		return 1 | SUPPORTED;

	case 0x90:
		util::stream_format(stream, "stream control %02x %02x %02x %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2), opcodes.r8(pc + 3), opcodes.r8(pc + 4));
		return 5 | SUPPORTED;

	case 0x91:
		util::stream_format(stream, "stream data %02x %02x %02x %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2), opcodes.r8(pc + 3), opcodes.r8(pc + 4));
		return 5 | SUPPORTED;

	case 0x92:
		util::stream_format(stream, "stream frequency %02x %d", opcodes.r8(pc + 1), opcodes.r32(pc + 2));
		return 6 | SUPPORTED;

	case 0x93:
		util::stream_format(stream, "stream start %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2), opcodes.r8(pc + 3), opcodes.r8(pc + 4), opcodes.r8(pc + 5), opcodes.r8(pc + 6), opcodes.r8(pc + 7), opcodes.r8(pc + 8), opcodes.r8(pc + 9), opcodes.r8(pc + 10));
		return 11 | SUPPORTED;

	case 0x94:
		util::stream_format(stream, "stream stop %02x", opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x95:
		util::stream_format(stream, "stream start short %02x %02x %02x %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2), opcodes.r8(pc + 3), opcodes.r8(pc + 4));
		return 5 | SUPPORTED;

	case 0xa0:
		util::stream_format(stream, "ay8910.%d r%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xa1:
		util::stream_format(stream, "ym2413.1 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xa2:
	case 0xa3:
		util::stream_format(stream, "ym2612.1 %d r%02x = %02x", opcodes.r8(pc) & 1, opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xa4:
		util::stream_format(stream, "ym2151.1 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xa5:
		util::stream_format(stream, "ym2203.1 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xa6:
	case 0xa7:
		util::stream_format(stream, "ym2608.1 %d r%02x = %02x", opcodes.r8(pc) & 1, opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xa8:
	case 0xa9:
		util::stream_format(stream, "ym2610.1 %d r%02x = %02x", opcodes.r8(pc) & 1, opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xaa:
		util::stream_format(stream, "ym3812.1 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xab:
		util::stream_format(stream, "ym3526.1 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xac:
		util::stream_format(stream, "y8950.1 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xad:
		util::stream_format(stream, "ymz280b.1 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xae:
	case 0xaf:
		util::stream_format(stream, "ymf262.1 %d r%02x = %02x", opcodes.r8(pc) & 1, opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xb0:
		util::stream_format(stream, "rf5c68 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xb1:
		util::stream_format(stream, "rf5c164 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xb2:
		util::stream_format(stream, "32x_pwm r%x = %03x", opcodes.r8(pc + 1) >> 4, opcodes.r8(pc + 2) | ((opcodes.r8(pc + 1) & 0xf) << 8));
		return 3 | SUPPORTED;

	case 0xb3:
		util::stream_format(stream, "dmg.%d r%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xb4:
		util::stream_format(stream, "nesapu.%d r%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xb5:
		util::stream_format(stream, "multipcm.%d r%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xb6:
		util::stream_format(stream, "upd7759.%d r%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xb7:
		util::stream_format(stream, "okim6258.%d r%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xb8:
		util::stream_format(stream, "okim6295.%d r%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xb9:
		util::stream_format(stream, "huc6280.%d r%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xba:
		util::stream_format(stream, "k053260.%d r%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xbb:
		util::stream_format(stream, "pokey.%d r%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xbc:
		util::stream_format(stream, "wonderswan.%d r%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xbd:
		util::stream_format(stream, "saa1099.%d r%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xbe:
		util::stream_format(stream, "es5505.%d r%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xbf:
		util::stream_format(stream, "ga20.%d r%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xc0:
		util::stream_format(stream, "segapcm.%d %04x = %02x", BIT(opcodes.r8(pc + 2), 7), opcodes.r8(pc + 1) | ((opcodes.r8(pc + 2) & 0x7f) << 8), opcodes.r8(pc + 3));
		return 4 | SUPPORTED;

	case 0xc1:
		util::stream_format(stream, "rf5c68 %04x = %02x", opcodes.r8(pc + 1) | (opcodes.r8(pc + 2) << 8), opcodes.r8(pc + 3));
		return 4 | SUPPORTED;

	case 0xc2:
		util::stream_format(stream, "rf5c164 %04x = %02x", opcodes.r8(pc + 1) | (opcodes.r8(pc + 2) << 8), opcodes.r8(pc + 3));
		return 4 | SUPPORTED;

	case 0xc3:
		util::stream_format(stream, "multipcm.%d c%02x.off = %04x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2) | (opcodes.r8(pc + 3) << 8));
		return 4 | SUPPORTED;

	case 0xc4:
		util::stream_format(stream, "qsound %02x = %04x", opcodes.r8(pc + 3), opcodes.r8(pc + 2) | (opcodes.r8(pc + 1) << 8));
		return 4 | SUPPORTED;

	case 0xc5:
		util::stream_format(stream, "scsp.%d %04x = %02x", BIT(opcodes.r8(pc + 1), 7), ((opcodes.r8(pc + 1) & 0x7f) << 8) | opcodes.r8(pc + 2), opcodes.r8(pc + 3));
		return 4 | SUPPORTED;

	case 0xc6:
		util::stream_format(stream, "wswan.%d %04x = %02x", BIT(opcodes.r8(pc + 1), 7), ((opcodes.r8(pc + 1) & 0x7f) << 8) | opcodes.r8(pc + 2), opcodes.r8(pc + 3));
		return 4 | SUPPORTED;

	case 0xc7:
		util::stream_format(stream, "vsu-vue.%d %04x = %02x", BIT(opcodes.r8(pc + 1), 7), ((opcodes.r8(pc + 1) & 0x7f) << 8) | opcodes.r8(pc + 2), opcodes.r8(pc + 3));
		return 4 | SUPPORTED;

	case 0xc8:
		util::stream_format(stream, "x1-010.%d %04x = %02x", BIT(opcodes.r8(pc + 1), 7), ((opcodes.r8(pc + 1) & 0x7f) << 8) | opcodes.r8(pc + 2), opcodes.r8(pc + 3));
		return 4 | SUPPORTED;

	case 0xd0:
		util::stream_format(stream, "ymf278b.%d r%02x.%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2), opcodes.r8(pc + 3));
		return 4 | SUPPORTED;

	case 0xd1:
		util::stream_format(stream, "ymf271.%d r%02x.%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1), opcodes.r8(pc + 2), opcodes.r8(pc + 3));
		return 4 | SUPPORTED;

	case 0xd2:
		util::stream_format(stream, "scc1.%d r%02x.%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2), opcodes.r8(pc + 3));
		return 4 | SUPPORTED;

	case 0xd3:
		util::stream_format(stream, "k054539.%d r%02x.%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2), opcodes.r8(pc + 3));
		return 4 | SUPPORTED;

	case 0xd4:
		util::stream_format(stream, "c140.%d r%02x.%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2), opcodes.r8(pc + 3));
		return 4 | SUPPORTED;

	case 0xd5:
		util::stream_format(stream, "ess5503.%d r%02x.%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2), opcodes.r8(pc + 3));
		return 4 | SUPPORTED;

	case 0xd6:
		util::stream_format(stream, "ess5505.%d r%02x.%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc + 1) & 0x7f, opcodes.r8(pc + 2), opcodes.r8(pc + 3));
		return 4 | SUPPORTED;

	case 0xe0:
	{
		uint32_t off = opcodes.r8(pc + 1) | (opcodes.r8(pc + 2) << 8) | (opcodes.r8(pc + 3) << 16) | (opcodes.r8(pc + 4) << 24);
		util::stream_format(stream, "ym2612 offset = %x", off);
		return 5 | SUPPORTED;
	}

	case 0xe1:
	{
		uint16_t addr = (opcodes.r8(pc + 1) << 8) | opcodes.r8(pc + 2);
		uint16_t data = (opcodes.r8(pc + 3) << 8) | opcodes.r8(pc + 4);
		util::stream_format(stream, "c352 r%04x = %04x", addr, data);
		return 5 | SUPPORTED;
	}

	default:
		util::stream_format(stream, "?? %02x", opcodes.r8(pc));
		return 1 | SUPPORTED;
	}
}

uint8_t vgmplay_device::rom_r(int index, uint8_t type, offs_t offset)
{
	for (const auto &b : m_rom_blocks[index][type - 0x80])
	{
		if (offset >= b.start_address && offset <= b.end_address)
		{
			return b.data[offset - b.start_address];
		}
	}
	return 0;
}

template<int Index>
uint8_t vgmplay_device::segapcm_rom_r(offs_t offset)
{
	return rom_r(Index, 0x80, offset);
}

template<int Index>
uint8_t vgmplay_device::ym2608_rom_r(offs_t offset)
{
	return rom_r(Index, 0x81, offset);
}

template<int Index>
uint8_t vgmplay_device::ym2610_adpcm_a_rom_r(offs_t offset)
{
	return rom_r(Index, 0x82, offset);
}

template<int Index>
uint8_t vgmplay_device::ym2610_adpcm_b_rom_r(offs_t offset)
{
	return rom_r(Index, 0x83, offset);
}

template<int Index>
uint8_t vgmplay_device::ymf278b_rom_r(offs_t offset)
{
	return rom_r(Index, 0x84, offset);
}

template<int Index>
uint8_t vgmplay_device::ymf271_rom_r(offs_t offset)
{
	return rom_r(Index, 0x85, offset);
}

template<int Index>
uint8_t vgmplay_device::ymz280b_rom_r(offs_t offset)
{
	return rom_r(Index, 0x86, offset);
}

template<int Index>
uint8_t vgmplay_device::y8950_rom_r(offs_t offset)
{
	return rom_r(Index, 0x88, offset);
}

template<int Index>
uint8_t vgmplay_device::multipcm_rom_r(offs_t offset)
{
	if (m_multipcm_banked[Index] == 1)
	{
		offset &= 0x1fffff;
		if (offset & 0x100000)
		{
			if (m_multipcm_bank_l[Index] == m_multipcm_bank_r[Index])
			{
				offset = ((m_multipcm_bank_r[Index] & ~0xf) << 16) | (offset & 0xfffff);
			}
			else
			{
				if (offset & 0x80000)
				{
					offset = ((m_multipcm_bank_l[Index] & ~0x7) << 16) | (offset & 0x7ffff);
				}
				else
				{
					offset = ((m_multipcm_bank_r[Index] & ~0x7) << 16) | (offset & 0x7ffff);
				}
			}
		}
	}
	return rom_r(Index, 0x89, offset);
}

template<int Index>
uint8_t vgmplay_device::upd7759_rom_r(offs_t offset)
{
	return rom_r(Index, 0x8a, m_upd7759_bank[Index] | offset);
}

template<int Index>
uint8_t vgmplay_device::okim6295_rom_r(offs_t offset)
{
	if (m_okim6295_nmk112_enable[Index])
	{
		if ((offset < 0x400) && (m_okim6295_nmk112_enable[Index] & 0x80))
		{
			offset = (m_okim6295_nmk112_bank[Index][(offset >> 8) & 0x3] << 16) | (offset & 0x3ff);
		}
		else
		{
			offset = (m_okim6295_nmk112_bank[Index][(offset >> 16) & 0x3] << 16) | (offset & 0xffff);
		}
	}
	else
	{
		offset = (m_okim6295_bank[Index] * 0x40000) | offset;
	}
	return rom_r(Index, 0x8b, offset);
}

template<int Index>
uint8_t vgmplay_device::k054539_rom_r(offs_t offset)
{
	return rom_r(Index, 0x8c, offset);
}

template<int Index>
uint16_t vgmplay_device::c140_rom_r(offs_t offset)
{
	switch (m_c140_bank[Index])
	{
	case C140_SYSTEM2:
		offset = ((offset & 0x200000) >> 2) | (offset & 0x7ffff);
		return rom_r(Index, 0x8d, offset) << 8; // high 8 bit only
	case C140_SYSTEM21:
		offset = ((offset & 0x300000) >> 1) | (offset & 0x7ffff);
		return rom_r(Index, 0x8d, offset) << 8; // high 8 bit only
	case C140_ASIC219:
		return 0; // c140 not used in this mode
	default:
		return (rom_r(Index, 0x8d, offset * 2 + 1) << 8) | rom_r(Index, 0x8d, offset * 2); // 8 bit sample
	}
	return 0;
}

template<int Index>
uint16_t vgmplay_device::c219_rom_r(offs_t offset)
{
	if (m_c140_bank[Index] == C140_ASIC219)
		return (rom_r(Index, 0x8d, offset * 2 + 1) << 8) | rom_r(Index, 0x8d, offset * 2); // 8 bit sample

	return 0;
}

template<int Index>
uint8_t vgmplay_device::k053260_rom_r(offs_t offset)
{
	return rom_r(Index, 0x8e, offset);
}

template<int Index>
uint8_t vgmplay_device::qsound_rom_r(offs_t offset)
{
	return rom_r(Index, 0x8f, offset);
}

template<int Index>
uint8_t vgmplay_device::es5505_rom_r(offs_t offset)
{
	return rom_r(Index, 0x90, offset);
}

template<int Index>
uint8_t vgmplay_device::x1_010_rom_r(offs_t offset)
{
	return rom_r(Index, 0x91, offset);
}

template<int Index>
uint8_t vgmplay_device::c352_rom_r(offs_t offset)
{
	return rom_r(Index, 0x92, offset);
}

template<int Index>
uint8_t vgmplay_device::ga20_rom_r(offs_t offset)
{
	return rom_r(Index, 0x93, offset);
}

vgmplay_state::vgmplay_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, m_vgmplay(*this, "vgmplay")
	, m_mixer(*this, "mixer")
	, m_lspeaker(*this, "lspeaker")
	, m_rspeaker(*this, "rspeaker")
	, m_sn76489(*this, "sn76489.%d", 0)
	, m_ym2413(*this, "ym2413.%d", 0)
	, m_ym2612(*this, "ym2612.%d", 0)
	, m_ym2151(*this, "ym2151.%d", 0)
	, m_segapcm(*this, "segapcm.%d", 0)
	, m_rf5c68(*this, "rf5c68")
	, m_ym2203(*this, "ym2203.%d", 0)
	, m_ym2608(*this, "ym2608.%d", 0)
	, m_ym2610(*this, "ym2610.%d", 0)
	, m_ym3812(*this, "ym3812.%d", 0)
	, m_ym3526(*this, "ym3526.%d", 0)
	, m_y8950(*this, "y8950.%d", 0)
	, m_ymf262(*this, "ymf262.%d", 0)
	, m_ymf278b(*this, "ymf278b.%d", 0)
	, m_ymf271(*this, "ymf271.%d", 0)
	, m_ymz280b(*this, "ymz280b.%d", 0)
	, m_rf5c164(*this, "rf5c164")
	, m_sega32x(*this, "sega32x")
	, m_ay8910(*this, "ay8910.%d", 0)
	, m_dmg(*this, "dmg.%d", 0)
	, m_nescpu(*this, "nescpu.%d", 0)
	, m_multipcm(*this, "multipcm.%d", 0)
	, m_upd7759(*this, "upd7759.%d", 0)
	, m_okim6258(*this, "okim6258.%d", 0)
	, m_okim6295(*this, "okim6295.%d", 0)
	, m_k051649(*this, "k051649.%d", 0)
	, m_k054539(*this, "k054539.%d", 0)
	, m_huc6280(*this, "huc6280.%d", 0)
	, m_c140(*this, "c140.%d", 0)
	, m_c219(*this, "c219.%d", 0)
	, m_k053260(*this, "k053260.%d", 0)
	, m_pokey(*this, "pokey.%d", 0)
	, m_qsound(*this, "qsound")
	, m_scsp(*this, "scsp.%d", 0)
	, m_wswan(*this, "wswan.%d", 0)
	, m_vsu_vue(*this, "vsu_vue.%d", 0)
	, m_saa1099(*this, "saa1099.%d", 0)
	, m_es5503(*this, "es5503.%d", 0)
	, m_es5505(*this, "es5505.%d", 0)
	, m_x1_010(*this, "x1_010.%d", 0)
	, m_c352(*this, "c352.%d", 0)
	, m_ga20(*this, "ga20.%d", 0)
{
	std::fill(std::begin(m_upd7759_md), std::end(m_upd7759_md), 0);
	std::fill(std::begin(m_upd7759_reset), std::end(m_upd7759_drq), 0);
	std::fill(std::begin(m_upd7759_drq), std::end(m_upd7759_drq), 0);
}

void vgmplay_state::machine_start()
{
	save_item(NAME(m_held_clock));
}

uint32_t vgmplay_state::r32(int off) const
{
	if (off + 3 < int(m_file_data.size()))
		return m_file_data[off] | (m_file_data[off + 1] << 8) | (m_file_data[off + 2] << 16) | (m_file_data[off + 3] << 24);
	return 0;
}

uint8_t vgmplay_state::r8(int off) const
{
	if (off < int(m_file_data.size()))
		return m_file_data[off];
	return 0;
}

static const ay8910_device::psg_type_t vgm_ay8910_type(uint8_t vgm_type)
{
	return (vgm_type & 0x10) ? ay8910_device::PSG_TYPE_YM : ay8910_device::PSG_TYPE_AY;
}

static const uint8_t vgm_ay8910_flags(uint8_t vgm_flags)
{
	uint8_t flags = 0;
	if (vgm_flags & 1) flags |= AY8910_LEGACY_OUTPUT;
	if (vgm_flags & 2) flags |= AY8910_SINGLE_OUTPUT;
	if (vgm_flags & 4) flags |= AY8910_DISCRETE_OUTPUT;
	return flags;
}

static const C140_TYPE c140_bank_type(uint8_t vgm_type)
{
	switch (vgm_type)
	{
	case 0:
	default:
		return C140_SYSTEM2;
	case 1:
		return C140_SYSTEM21;
	case 2:
		return C140_ASIC219;
	}
}

void vgmplay_device::set_c140_bank_type(int index, C140_TYPE type)
{
	m_c140_bank[index] = type;
}

QUICKLOAD_LOAD_MEMBER(vgmplay_state::load_file)
{
	m_vgmplay->stop();

	m_file_data.resize(quickload_size);

	if (!quickload_size ||
		image.fread(&m_file_data[0], quickload_size) != quickload_size)
	{
		m_file_data.clear();
		return image_init_result::FAIL;
	}
	else
	{
		// Decompress gzip-compressed files (aka vgz)
		if(m_file_data[0] == 0x1f && m_file_data[1] == 0x8b)
		{
			std::vector<uint8_t> decomp;
			int bs = m_file_data.size();
			decomp.resize(2*bs);
			z_stream str;
			str.zalloc = nullptr;
			str.zfree = nullptr;
			str.opaque = nullptr;
			str.data_type = 0;
			str.next_in = &m_file_data[0];
			str.avail_in = m_file_data.size();
			str.total_in = 0;
			str.total_out = 0;
			int err = inflateInit2(&str, 31);
			if(err != Z_OK)
			{
				logerror("gzip header but not a gzip file\n");
				m_file_data.clear();
				return image_init_result::FAIL;
			}
			do
			{
				if(str.total_out >= decomp.size())
					decomp.resize(decomp.size() + bs);
				str.next_out = &decomp[str.total_out];
				str.avail_out = decomp.size() - str.total_out;
				err = inflate(&str, Z_SYNC_FLUSH);
			} while(err == Z_OK);

			if(err != Z_STREAM_END)
			{
				logerror("broken gzip file\n");
				m_file_data.clear();
				return image_init_result::FAIL;
			}
			m_file_data.resize(str.total_out);
			memcpy(&m_file_data[0], &decomp[0], str.total_out);
		}

		if(m_file_data.size() < 0x40 || r32(0) != 0x206d6756)
		{
			logerror("Not a vgm/vgz file\n");
			m_file_data.clear();
			return image_init_result::FAIL;
		}

		uint32_t version = r32(8);
		logerror("File version %x.%02x\n", version >> 8, version & 0xff);

		uint32_t data_start = version >= 0x150 ? r32(0x34) + 0x34 : 0x40;
		int volbyte = version >= 0x160 && data_start >= 0x7d ? r8(0x7c) : 0;
		logerror("Volume %02x\n", volbyte);

		if (volbyte == 0xc1) // 0x00~0xc0 0~192, 0xc1 -64, 0xc2~0xff -62~-1
			volbyte = -0x40;
		else if (volbyte > 0xc1)
			volbyte -= 0x100;

		float volume = version >= 0x160 && data_start >= 0x7d ? powf(2.0f, float(volbyte) / float(0x20)) : 1.0f;

		uint32_t extra_header_start = version >= 0x170 && data_start >= 0xc0 && r32(0xbc) ? r32(0xbc) + 0xbc : 0;
		uint32_t header_size = extra_header_start ? extra_header_start : data_start;

		uint32_t extra_header_size = extra_header_start ? r32(extra_header_start) : 0;
		uint32_t chip_clock_start = extra_header_size >= 4 && r32(extra_header_start + 4) ? r32(extra_header_start + 4) + extra_header_start + 4: 0;
		uint32_t chip_volume_start = extra_header_size >= 8 && r32(extra_header_start + 8) ? r32(extra_header_start + 8) + extra_header_start + 8 : 0;

		if (chip_volume_start != 0)
			osd_printf_warning("Warning: file has unsupported chip volumes\n");

		const auto&& setup_device([&](device_t &device, int chip_num, vgm_chip chip_type, uint32_t offset, uint32_t min_version = 0)
		{
			uint32_t c = 0;
			float chip_volume = volume;
			bool has_2chip = false;

			if (min_version <= version && offset + 4 <= header_size && (chip_num == 0 || (r32(offset) & 0x40000000) != 0))
			{
				c =  r32(offset);
				has_2chip = (c & 0x40000000) != 0;

				if (chip_clock_start && chip_num != 0)
					for (auto i(0); i < r8(chip_clock_start); i++)
					{
						if (r8(chip_clock_start + 1 + (i * 5)) == chip_type)
						{
							c = r32(chip_clock_start + 2 + (i * 5));
							break;
						}
					}
			}

			if (has_2chip)
			{
				chip_volume /= 2.0f;
			}
			device.set_unscaled_clock(c & ~0xc0000000);
			if (device.unscaled_clock() != 0)
				dynamic_cast<device_sound_interface *>(&device)->set_output_gain(ALL_OUTPUTS, chip_volume);
			else
				dynamic_cast<device_sound_interface *>(&device)->set_output_gain(ALL_OUTPUTS, 0);

			return (c & 0x80000000) != 0;
		});

		// Parse clocks
		if (setup_device(*m_sn76489[0], 0, CT_SN76489, 0x0c) ||
			setup_device(*m_sn76489[1], 1, CT_SN76489, 0x0c))
			osd_printf_warning("Warning: file requests an unsupported T6W28\n");

		if (setup_device(*m_ym2413[0], 0, CT_YM2413, 0x10) ||
			setup_device(*m_ym2413[1], 1, CT_YM2413, 0x10))
			osd_printf_warning("Warning: file requests an unsupported VRC7\n");

		if (setup_device(*m_ym2612[0], 0, CT_YM2612, version < 110 ? 0x10 : 0x2c) ||
			setup_device(*m_ym2612[1], 1, CT_YM2612, version < 110 ? 0x10 : 0x2c))
			osd_printf_warning("Warning: file requests an unsupported YM3438\n");

		setup_device(*m_ym2151[0], 0, CT_YM2151, version < 110 ? 0x10 : 0x30);
		setup_device(*m_ym2151[1], 1, CT_YM2151, version < 110 ? 0x10 : 0x30);

		setup_device(*m_segapcm[0], 0, CT_SEGAPCM, 0x38, 0x151);
		setup_device(*m_segapcm[1], 1, CT_SEGAPCM, 0x38, 0x151);
		m_segapcm[0]->set_bank(version >= 0x151 && header_size >= 0x40 ? r32(0x3c) : 0);
		m_segapcm[1]->set_bank(version >= 0x151 && header_size >= 0x40 ? r32(0x3c) : 0);

		setup_device(*m_rf5c68, 0, CT_RF5C68, 0x40, 0x151);
		setup_device(*m_ym2203[0], 0, CT_YM2203, 0x44, 0x151);
		setup_device(*m_ym2203[1], 1, CT_YM2203, 0x44, 0x151);
		setup_device(*m_ym2608[0], 0, CT_YM2608, 0x48, 0x151);
		setup_device(*m_ym2608[1], 1, CT_YM2608, 0x48, 0x151);

		if (setup_device(*m_ym2610[0], 0, CT_YM2610, 0x4c, 0x151) ||
			setup_device(*m_ym2610[1], 1, CT_YM2610, 0x4c, 0x151))
			osd_printf_warning("Warning: file requests an unsupported YM2610B\n");

		if (setup_device(*m_ym3812[0], 0, CT_YM3812, 0x50, 0x151) ||
			setup_device(*m_ym3812[1], 1, CT_YM3812, 0x50, 0x151))
			osd_printf_warning("Warning: file requests an unsupported SoundBlaster Pro\n");

		setup_device(*m_ym3526[0], 0, CT_YM3526, 0x54, 0x151);
		setup_device(*m_ym3526[1], 1, CT_YM3526, 0x54, 0x151);
		setup_device(*m_y8950[0], 0, CT_Y8950, 0x58, 0x151);
		setup_device(*m_y8950[1], 1, CT_Y8950, 0x58, 0x151);
		setup_device(*m_ymf262[0], 0, CT_YMF262, 0x5c, 0x151);
		setup_device(*m_ymf262[1], 1, CT_YMF262, 0x5c, 0x151);
		setup_device(*m_ymf278b[0], 0, CT_YMF278B, 0x60, 0x151);
		setup_device(*m_ymf278b[1], 1, CT_YMF278B, 0x60, 0x151);
		setup_device(*m_ymf271[0], 0, CT_YMF271, 0x64, 0x151);
		setup_device(*m_ymf271[1], 1, CT_YMF271, 0x64, 0x151);
		setup_device(*m_ymz280b[0], 0, CT_YMZ280B, 0x68, 0x151);
		setup_device(*m_ymz280b[1], 1, CT_YMZ280B, 0x68, 0x151);

		if (setup_device(*m_rf5c164, 0, CT_RF5C164, 0x6c, 0x151))
			osd_printf_warning("Warning: file requests an unsupported Cosmic Fantasy Stories HACK\n");

		setup_device(*m_sega32x, 0, CT_SEGA32X, 0x70, 0x151);

		setup_device(*m_ay8910[0], 0, CT_AY8910, 0x74, 0x151);
		setup_device(*m_ay8910[1], 1, CT_AY8910, 0x74, 0x151);
		m_ay8910[0]->set_psg_type(vgm_ay8910_type(version >= 0x151 && header_size >= 0x7c ? r8(0x78) : 0));
		m_ay8910[1]->set_psg_type(vgm_ay8910_type(version >= 0x151 && header_size >= 0x7c ? r8(0x78) : 0));
		m_ay8910[0]->set_flags(vgm_ay8910_flags(version >= 0x151 && header_size >= 0x7a ? r8(0x79) : 0));
		m_ay8910[1]->set_flags(vgm_ay8910_flags(version >= 0x151 && header_size >= 0x7a ? r8(0x79) : 0));
		m_ym2203[0]->set_flags(vgm_ay8910_flags(version >= 0x151 && header_size >= 0x7b ? r8(0x7a) : 0));
		m_ym2203[1]->set_flags(vgm_ay8910_flags(version >= 0x151 && header_size >= 0x7b ? r8(0x7a) : 0));
		m_ym2608[0]->set_flags(vgm_ay8910_flags(version >= 0x151 && header_size >= 0x7c ? r8(0x7b) : 0));
		m_ym2608[1]->set_flags(vgm_ay8910_flags(version >= 0x151 && header_size >= 0x7c ? r8(0x7b) : 0));

		setup_device(*m_dmg[0], 0, CT_GAMEBOY, 0x80, 0x161);
		setup_device(*m_dmg[1], 1, CT_GAMEBOY, 0x80, 0x161);

		if (setup_device(*m_nescpu[0], 0, CT_NESAPU, 0x84, 0x161) ||
			setup_device(*m_nescpu[1], 1, CT_NESAPU, 0x84, 0x161))
			osd_printf_warning("Warning: file requests an unsupported FDS sound addon\n");

		setup_device(*m_multipcm[0], 0, CT_MULTIPCM, 0x88, 0x161);
		setup_device(*m_multipcm[1], 1, CT_MULTIPCM, 0x88, 0x161);

		setup_device(*m_upd7759[0], 0, CT_UPD7759, 0x8c, 0x161);
		setup_device(*m_upd7759[1], 1, CT_UPD7759, 0x8c, 0x161);
		m_upd7759_md[0] = r32(0x8c) & 0x80000000 ? 0 : 1;
		m_upd7759_md[1] = r32(0x8c) & 0x80000000 ? 0 : 1;
		m_upd7759[0]->md_w(m_upd7759_md[0]);
		m_upd7759[1]->md_w(m_upd7759_md[1]);

		setup_device(*m_okim6258[0], 0, CT_OKIM6258, 0x90, 0x161);
		setup_device(*m_okim6258[1], 1, CT_OKIM6258, 0x90, 0x161);

		uint8_t okim6258_flags = version >= 0x161 && header_size >= 0x95 ? r8(0x94) : 0;
		m_okim6258_divider[0] = okim6258_flags & 3;
		m_okim6258[0]->set_divider(m_okim6258_divider[0]);
		m_okim6258[0]->set_outbits(BIT(okim6258_flags, 3) ? 12 : 10);
		m_okim6258[0]->set_type(BIT(okim6258_flags, 2));
		m_okim6258_divider[1] = okim6258_flags & 3;
		m_okim6258[1]->set_divider(m_okim6258_divider[1]);
		m_okim6258[1]->set_outbits(BIT(okim6258_flags, 3) ? 12 : 10);
		m_okim6258[1]->set_type(BIT(okim6258_flags, 2));

		m_k054539[0]->init_flags(version >= 0x161 && header_size >= 0x96 ? r8(0x95) : 0);
		m_k054539[1]->init_flags(version >= 0x161 && header_size >= 0x96 ? r8(0x95) : 0);

		C140_TYPE c140_type = c140_bank_type(version >= 0x161 && header_size >= 0x96 ? r8(0x96) : 0);
		m_vgmplay->set_c140_bank_type(0, c140_type);
		m_vgmplay->set_c140_bank_type(1, c140_type);

		m_okim6295_pin7[0] = setup_device(*m_okim6295[0], 0, CT_OKIM6295, 0x98, 0x161);
		m_okim6295_pin7[1] = setup_device(*m_okim6295[1], 1, CT_OKIM6295, 0x98, 0x161);
		m_okim6295[0]->set_pin7(m_okim6295_pin7[0] ? okim6295_device::PIN7_HIGH : okim6295_device::PIN7_LOW);
		m_okim6295[1]->set_pin7(m_okim6295_pin7[1] ? okim6295_device::PIN7_HIGH : okim6295_device::PIN7_LOW);

		if (setup_device(*m_k051649[0], 0, CT_K051649, 0x9c, 0x161) ||
			setup_device(*m_k051649[1], 1, CT_K051649, 0x9c, 0x161))
			osd_printf_warning("Warning: file requests an unsupported Konami SCC\n");

		setup_device(*m_k054539[0], 0, CT_K054539, 0xa0, 0x161);
		setup_device(*m_k054539[1], 1, CT_K054539, 0xa0, 0x161);

		// HACK: Some VGMs contain 48,000 instead of 18,432,000
		m_k054539[0]->set_clock_scale(m_k054539[0]->unscaled_clock() == 48000 ? 384.0 : 1.0);
		m_k054539[1]->set_clock_scale(m_k054539[1]->unscaled_clock() == 48000 ? 384.0 : 1.0);
		if (m_k054539[0]->unscaled_clock() == 48000 || m_k054539[1]->unscaled_clock() == 48000)
			osd_printf_error("bad rip detected, correcting k054539 clock\n");

		// HACK: VGM contain the halved clock speed of the sound core inside the HUC6280
		m_huc6280[0]->set_clock_scale(2);
		m_huc6280[1]->set_clock_scale(2);

		setup_device(*m_huc6280[0], 0, CT_C6280, 0xa4, 0x161);
		setup_device(*m_huc6280[1], 1, CT_C6280, 0xa4, 0x161);
		if (c140_type == C140_ASIC219)
		{
			setup_device(*m_c219[0], 0, CT_C140, 0xa8, 0x161);
			setup_device(*m_c219[1], 1, CT_C140, 0xa8, 0x161);
		}
		else
		{
			setup_device(*m_c140[0], 0, CT_C140, 0xa8, 0x161);
			setup_device(*m_c140[1], 1, CT_C140, 0xa8, 0x161);
		}
		setup_device(*m_k053260[0], 0, CT_K053260, 0xac, 0x161);
		setup_device(*m_k053260[1], 1, CT_K053260, 0xac, 0x161);
		setup_device(*m_pokey[0], 0, CT_POKEY, 0xb0, 0x161);
		setup_device(*m_pokey[1], 1, CT_POKEY, 0xb0, 0x161);

		setup_device(*m_qsound, 0, CT_QSOUND, 0xb4, 0x161);

		// HACK: VGMs contain 4,000,000 instead of 60,000,000
		m_qsound->set_clock_scale(m_qsound->unscaled_clock() == 4000000 ? 15.0 : 1.0);
		if (m_qsound->unscaled_clock() == 4000000)
			osd_printf_error("bad rip detected, correcting qsound clock\n");

		setup_device(*m_scsp[0], 0, CT_SCSP, 0xb8, 0x171);
		setup_device(*m_scsp[1], 1, CT_SCSP, 0xb8, 0x171);
		setup_device(*m_wswan[0], 0, CT_WSWAN, 0xc0, 0x171);
		setup_device(*m_wswan[1], 1, CT_WSWAN, 0xc0, 0x171);
		setup_device(*m_vsu_vue[0], 0, CT_VSU_VUE, 0xc4, 0x171);
		setup_device(*m_vsu_vue[1], 1, CT_VSU_VUE, 0xc4, 0x171);
		setup_device(*m_saa1099[0], 0, CT_SAA1099, 0xc8, 0x171);
		setup_device(*m_saa1099[1], 1, CT_SAA1099, 0xc8, 0x171);
		setup_device(*m_es5503[0], 0, CT_ES5503, 0xcc, 0x171);
		setup_device(*m_es5503[1], 1, CT_ES5503, 0xcc, 0x171);

		if (setup_device(*m_es5505[0], 0, CT_ES5505, 0xd0, 0x171) ||
			setup_device(*m_es5505[1], 1, CT_ES5503, 0xd0, 0x171))
			osd_printf_warning("Warning: file requests an unsupported ES5506\n");

		// TODO: dynamically remap es5503/es5505 channels?
		//m_es5503[0]->set_channels(version >= 0x171 && header_size >= 0xd5 ? r8(0xd4) : 0);
		//m_es5503[1]->set_channels(version >= 0x171 && header_size >= 0xd5 ? r8(0xd4) : 0);
		//m_es5505[0]->set_channels(version >= 0x171 && header_size >= 0xd6 ? r8(0xd5) : 0);
		//m_es5505[1]->set_channels(version >= 0x171 && header_size >= 0xd6 ? r8(0xd5) : 0);

		m_c352[0]->set_divider(version >= 0x171 && header_size >= 0xd7 && r8(0xd6) ? r8(0xd6) * 4 : 1);
		m_c352[1]->set_divider(version >= 0x171 && header_size >= 0xd7 && r8(0xd6) ? r8(0xd6) * 4 : 1);

		setup_device(*m_x1_010[0], 0, CT_X1_010, 0xd8, 0x171);
		setup_device(*m_x1_010[1], 1, CT_X1_010, 0xd8, 0x171);

		if (setup_device(*m_c352[0], 0, CT_C352, 0xdc, 0x171) ||
			setup_device(*m_c352[1], 1, CT_C352, 0xdc, 0x171))
			osd_printf_warning("Warning: file requests an unsupported disable rear speakers\n");

		setup_device(*m_ga20[0], 0, CT_GA20, 0xe0, 0x171);
		setup_device(*m_ga20[1], 1, CT_GA20, 0xe0, 0x171);

		for (device_t &child : subdevices())
			if (child.clock() != 0)
				logerror("%s %d\n", child.tag(), child.clock());

		//for (auto &stream : machine().sound().streams())
		//  if (stream->sample_rate() != 0)
		//      logerror("%s %d\n", stream->device().tag(), stream->sample_rate());

		machine().schedule_soft_reset();

		return image_init_result::PASS;
	}
}

uint8_t vgmplay_state::file_r(offs_t offset)
{
	if (offset < m_file_data.size())
		return m_file_data[offset];
	return 0;
}

uint8_t vgmplay_state::file_size_r(offs_t offset)
{
	uint32_t size = m_file_data.size();
	return size >> (8 * offset);
}

template<int Index>
void vgmplay_device::multipcm_bank_hi_w(offs_t offset, uint8_t data)
{
	if (offset & 1)
		m_multipcm_bank_l[Index] = (m_multipcm_bank_l[Index] & 0xff) | (data << 16);
	if (offset & 2)
		m_multipcm_bank_r[Index] = (m_multipcm_bank_r[Index] & 0xff) | (data << 16);
}

template<int Index>
void vgmplay_device::multipcm_bank_lo_w(offs_t offset, uint8_t data)
{
	if (offset & 1)
		m_multipcm_bank_l[Index] = (m_multipcm_bank_l[Index] & 0xff00) | data;
	if (offset & 2)
		m_multipcm_bank_r[Index] = (m_multipcm_bank_r[Index] & 0xff00) | data;

	m_multipcm_banked[Index] = 1;
}

template<int Index>
void vgmplay_state::upd7759_reset_w(uint8_t data)
{
	int reset = data != 0;

	m_upd7759[Index]->reset_w(reset);

	if (m_upd7759_reset[Index] != reset)
	{
		m_upd7759_reset[Index] = reset;

		if (!reset)
			std::queue<uint8_t>().swap(m_upd7759_slave_data[Index]);
	}
}

template<int Index>
void vgmplay_state::upd7759_data_w(uint8_t data)
{
	if (!m_upd7759_md[Index] && !m_upd7759_drq[Index])
	{
		m_upd7759_slave_data[Index].push(data);
	}
	else
	{
		m_upd7759[Index]->port_w(data);
		m_upd7759_drq[Index] = 0;
	}
}

template<int Index>
WRITE_LINE_MEMBER(vgmplay_state::upd7759_drq_w)
{
	if (m_upd7759_drq[Index] && !state)
		osd_printf_error("upd7759.%d underflow\n", Index);

	m_upd7759_drq[Index] = state;

	if (!m_upd7759_md[Index] && m_upd7759_drq[Index] && !m_upd7759_slave_data[Index].empty())
	{
		const uint8_t data(m_upd7759_slave_data[Index].front());
		m_upd7759_slave_data[Index].pop();
		m_upd7759[Index]->port_w(data);
		m_upd7759_drq[Index] = 0;
	}
}

template<int Index>
void vgmplay_device::upd7759_bank_w(uint8_t data)
{
	// TODO: upd7759 update stream
	m_upd7759_bank[Index] = data * 0x20000;
}

template<int Index>
void vgmplay_state::okim6258_clock_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	int shift = ((offset & 3) << 3);
	uint32_t c = (m_okim6258[Index]->unscaled_clock() & ~(mem_mask << shift)) | ((data & mem_mask) << shift);
	m_okim6258[Index]->set_unscaled_clock(c);

}

template<int Index>
void vgmplay_state::okim6258_divider_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if ((data & mem_mask) != (m_okim6258_divider[Index] & mem_mask))
	{
		COMBINE_DATA(&m_okim6258_divider[Index]);
		m_okim6258[Index]->set_divider(m_okim6258_divider[Index]);
	}
}

template<int Index>
void vgmplay_state::okim6295_clock_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	int shift = ((offset & 3) << 3);
	uint32_t c = (m_okim6295[Index]->unscaled_clock() & ~(mem_mask << shift)) | ((data & mem_mask) << shift);
	m_okim6295[Index]->set_unscaled_clock(c);

}

template<int Index>
void vgmplay_state::okim6295_pin7_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if ((data & mem_mask) != (m_okim6295_pin7[Index] & mem_mask))
	{
		COMBINE_DATA(&m_okim6295_pin7[Index]);
		m_okim6295[Index]->set_pin7(m_okim6295_pin7[Index]);
	}
}

template<int Index>
void vgmplay_device::okim6295_nmk112_enable_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_okim6295_nmk112_enable[Index]);
}

template<int Index>
void vgmplay_device::okim6295_bank_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if ((data & mem_mask) != (m_okim6295_bank[Index] & mem_mask))
	{
		COMBINE_DATA(&m_okim6295_bank[Index]);
	}
}

template<int Index>
void vgmplay_device::okim6295_nmk112_bank_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	offset &= 3;
	if ((data & mem_mask) != (m_okim6295_nmk112_bank[Index][offset] & mem_mask))
	{
		COMBINE_DATA(&m_okim6295_nmk112_bank[Index][offset]);
	}
}

template<int Index>
void vgmplay_state::scc_w(offs_t offset, uint8_t data)
{
	switch (offset & 1)
	{
	case 0x00:
		m_scc_reg[Index] = data;
		break;
	case 0x01:
		switch (offset >> 1)
		{
		case 0x00:
			m_k051649[Index]->k051649_waveform_w(m_scc_reg[Index], data);
			break;
		case 0x01:
			m_k051649[Index]->k051649_frequency_w(m_scc_reg[Index], data);
			break;
		case 0x02:
			m_k051649[Index]->k051649_volume_w(m_scc_reg[Index], data);
			break;
		case 0x03:
			m_k051649[Index]->k051649_keyonoff_w(data);
			break;
		case 0x04:
			m_k051649[Index]->k052539_waveform_w(m_scc_reg[Index], data);
			break;
		case 0x05:
			m_k051649[Index]->k051649_test_w(data);
			break;
		}
		break;
	}
}

template<int Index>
void vgmplay_state::c140_c219_w(offs_t offset, uint8_t data)
{
	if (m_vgmplay->c140_bank(Index) == C140_ASIC219)
		m_c219[Index]->c219_w(offset, data);
	else
		m_c140[Index]->c140_w(offset, data);
}

INPUT_CHANGED_MEMBER(vgmplay_state::key_pressed)
{
	if (!newval && param != VGMPLAY_HOLD)
		return;

	switch (param)
	{
	case VGMPLAY_STOP:
		m_vgmplay->stop();
		break;
	case VGMPLAY_PAUSE:
		m_vgmplay->pause();
		break;
	case VGMPLAY_PLAY:
		m_vgmplay->play();
		break;
	case VGMPLAY_RESTART:
		m_vgmplay->reset();
		break;
	case VGMPLAY_LOOP:
		m_vgmplay->toggle_loop();
		break;
	case VGMPLAY_VIZ:
		m_mixer->cycle_viz_mode();
		break;
	case VGMPLAY_RATE_DOWN:
		m_vgmplay->set_unscaled_clock((uint32_t)(m_vgmplay->clock() * 0.95f));
		break;
	case VGMPLAY_RATE_UP:
		m_vgmplay->set_unscaled_clock((uint32_t)(m_vgmplay->clock() / 0.95f));
		break;
	case VGMPLAY_RATE_RST:
		m_vgmplay->set_unscaled_clock(44100);
		break;
	case VGMPLAY_HOLD:
		if (newval)
		{
			m_held_clock = m_vgmplay->clock();
			m_vgmplay->set_unscaled_clock(0);
		}
		else
		{
			m_vgmplay->set_unscaled_clock(m_held_clock);
		}
	}
}

static INPUT_PORTS_START( vgmplay )
	PORT_START("CONTROLS")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1)  PORT_CHANGED_MEMBER(DEVICE_SELF, vgmplay_state, key_pressed, VGMPLAY_STOP)        PORT_NAME("Stop")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2)  PORT_CHANGED_MEMBER(DEVICE_SELF, vgmplay_state, key_pressed, VGMPLAY_PAUSE)       PORT_NAME("Pause")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3)  PORT_CHANGED_MEMBER(DEVICE_SELF, vgmplay_state, key_pressed, VGMPLAY_PLAY)        PORT_NAME("Play")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4)  PORT_CHANGED_MEMBER(DEVICE_SELF, vgmplay_state, key_pressed, VGMPLAY_RESTART)     PORT_NAME("Restart")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5)  PORT_CHANGED_MEMBER(DEVICE_SELF, vgmplay_state, key_pressed, VGMPLAY_LOOP)        PORT_NAME("Loop")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6)  PORT_CHANGED_MEMBER(DEVICE_SELF, vgmplay_state, key_pressed, VGMPLAY_VIZ)         PORT_NAME("Visualization Mode")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_BUTTON7)  PORT_CHANGED_MEMBER(DEVICE_SELF, vgmplay_state, key_pressed, VGMPLAY_RATE_DOWN)   PORT_CODE(KEYCODE_R) PORT_NAME("Rate Down")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_BUTTON8)  PORT_CHANGED_MEMBER(DEVICE_SELF, vgmplay_state, key_pressed, VGMPLAY_RATE_UP)     PORT_CODE(KEYCODE_T) PORT_NAME("Rate Up")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_BUTTON9)  PORT_CHANGED_MEMBER(DEVICE_SELF, vgmplay_state, key_pressed, VGMPLAY_RATE_RST)    PORT_CODE(KEYCODE_Y) PORT_NAME("Rate Reset")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_BUTTON10) PORT_CHANGED_MEMBER(DEVICE_SELF, vgmplay_state, key_pressed, VGMPLAY_HOLD)        PORT_CODE(KEYCODE_U) PORT_NAME("Rate Hold")
INPUT_PORTS_END

void vgmplay_state::file_map(address_map &map)
{
	map(0x00000000, 0xffffffff).r(FUNC(vgmplay_state::file_r));
}

void vgmplay_state::soundchips_map(address_map &map)
{
	map(vgmplay_device::REG_SIZE, vgmplay_device::REG_SIZE + 3).r(FUNC(vgmplay_state::file_size_r));
	map(vgmplay_device::A_SN76489_0 + 0, vgmplay_device::A_SN76489_0 + 0).w(m_sn76489[0], FUNC(sn76489_device::write));
	//map(vgmplay_device::A_SN76489_0 + 1, vgmplay_device::A_SN76489_0 + 1).w(m_sn76489[0], FUNC(sn76489_device::stereo_w)); // TODO: GG stereo
	map(vgmplay_device::A_SN76489_1 + 0, vgmplay_device::A_SN76489_1 + 0).w(m_sn76489[1], FUNC(sn76489_device::write));
	//map(vgmplay_device::A_SN76489_1 + 1, vgmplay_device::A_SN76489_1 + 1).w(m_sn76489[1], FUNC(sn76489_device::stereo_w)); // TODO: GG stereo
	map(vgmplay_device::A_YM2413_0, vgmplay_device::A_YM2413_0 + 1).w(m_ym2413[0], FUNC(ym2413_device::write));
	map(vgmplay_device::A_YM2413_1, vgmplay_device::A_YM2413_1 + 1).w(m_ym2413[1], FUNC(ym2413_device::write));
	map(vgmplay_device::A_YM2612_0, vgmplay_device::A_YM2612_0 + 3).w(m_ym2612[0], FUNC(ym2612_device::write));
	map(vgmplay_device::A_YM2612_1, vgmplay_device::A_YM2612_1 + 3).w(m_ym2612[1], FUNC(ym2612_device::write));
	map(vgmplay_device::A_YM2151_0, vgmplay_device::A_YM2151_0 + 1).w(m_ym2151[0], FUNC(ym2151_device::write));
	map(vgmplay_device::A_YM2151_1, vgmplay_device::A_YM2151_1 + 1).w(m_ym2151[1], FUNC(ym2151_device::write));
	map(vgmplay_device::A_SEGAPCM_0, vgmplay_device::A_SEGAPCM_0 + 0x7ff).w(m_segapcm[0], FUNC(segapcm_device::write));
	map(vgmplay_device::A_SEGAPCM_1, vgmplay_device::A_SEGAPCM_1 + 0x7ff).w(m_segapcm[1], FUNC(segapcm_device::write));
	map(vgmplay_device::A_RF5C68, vgmplay_device::A_RF5C68 + 0xf).w(m_rf5c68, FUNC(rf5c68_device::rf5c68_w));
	map(vgmplay_device::A_RF5C68_RAM, vgmplay_device::A_RF5C68_RAM + 0xffff).w(m_rf5c68, FUNC(rf5c68_device::rf5c68_mem_w));
	map(vgmplay_device::A_YM2203_0, vgmplay_device::A_YM2203_0 + 1).w(m_ym2203[0], FUNC(ym2203_device::write));
	map(vgmplay_device::A_YM2203_1, vgmplay_device::A_YM2203_1 + 1).w(m_ym2203[1], FUNC(ym2203_device::write));
	map(vgmplay_device::A_YM2608_0, vgmplay_device::A_YM2608_0 + 0x3).w(m_ym2608[0], FUNC(ym2608_device::write));
	map(vgmplay_device::A_YM2608_1, vgmplay_device::A_YM2608_1 + 0x3).w(m_ym2608[1], FUNC(ym2608_device::write));
	map(vgmplay_device::A_YM2610_0, vgmplay_device::A_YM2610_0 + 0x3).w(m_ym2610[0], FUNC(ym2610_device::write));
	map(vgmplay_device::A_YM2610_1, vgmplay_device::A_YM2610_1 + 0x3).w(m_ym2610[1], FUNC(ym2610_device::write));
	map(vgmplay_device::A_YM3812_0, vgmplay_device::A_YM3812_0 + 1).w(m_ym3812[0], FUNC(ym3812_device::write));
	map(vgmplay_device::A_YM3812_1, vgmplay_device::A_YM3812_1 + 1).w(m_ym3812[1], FUNC(ym3812_device::write));
	map(vgmplay_device::A_YM3526_0, vgmplay_device::A_YM3526_0 + 1).w(m_ym3526[0], FUNC(ym3526_device::write));
	map(vgmplay_device::A_YM3526_1, vgmplay_device::A_YM3526_1 + 1).w(m_ym3526[1], FUNC(ym3526_device::write));
	map(vgmplay_device::A_Y8950_0, vgmplay_device::A_Y8950_0 + 1).w(m_y8950[0], FUNC(y8950_device::write));
	map(vgmplay_device::A_Y8950_1, vgmplay_device::A_Y8950_1 + 1).w(m_y8950[1], FUNC(y8950_device::write));
	map(vgmplay_device::A_YMF262_0, vgmplay_device::A_YMF262_0 + 3).w(m_ymf262[0], FUNC(ymf262_device::write));
	map(vgmplay_device::A_YMF262_1, vgmplay_device::A_YMF262_1 + 3).w(m_ymf262[1], FUNC(ymf262_device::write));
	map(vgmplay_device::A_YMF278B_0, vgmplay_device::A_YMF278B_0 + 0xf).w(m_ymf278b[0], FUNC(ymf278b_device::write));
	map(vgmplay_device::A_YMF278B_1, vgmplay_device::A_YMF278B_1 + 0xf).w(m_ymf278b[1], FUNC(ymf278b_device::write));
	map(vgmplay_device::A_YMF271_0, vgmplay_device::A_YMF271_0 + 0xf).w(m_ymf271[0], FUNC(ymf271_device::write));
	map(vgmplay_device::A_YMF271_1, vgmplay_device::A_YMF271_1 + 0xf).w(m_ymf271[1], FUNC(ymf271_device::write));
	map(vgmplay_device::A_YMZ280B_0, vgmplay_device::A_YMZ280B_0 + 0x1).w(m_ymz280b[0], FUNC(ymz280b_device::write));
	map(vgmplay_device::A_YMZ280B_1, vgmplay_device::A_YMZ280B_1 + 0x1).w(m_ymz280b[1], FUNC(ymz280b_device::write));
	map(vgmplay_device::A_RF5C164, vgmplay_device::A_RF5C164 + 0xf).w(m_rf5c164, FUNC(rf5c68_device::rf5c68_w));
	map(vgmplay_device::A_RF5C164_RAM, vgmplay_device::A_RF5C164_RAM + 0xffff).w(m_rf5c164, FUNC(rf5c68_device::rf5c68_mem_w));
	map(vgmplay_device::A_AY8910_0, vgmplay_device::A_AY8910_0).w(m_ay8910[0], FUNC(ay8910_device::data_w));
	map(vgmplay_device::A_AY8910_0 + 1, vgmplay_device::A_AY8910_0 + 1).w(m_ay8910[0], FUNC(ay8910_device::address_w));
	map(vgmplay_device::A_AY8910_1, vgmplay_device::A_AY8910_1).w(m_ay8910[1], FUNC(ay8910_device::data_w));
	map(vgmplay_device::A_AY8910_1 + 1, vgmplay_device::A_AY8910_1 + 1).w(m_ay8910[1], FUNC(ay8910_device::address_w));
	map(vgmplay_device::A_GAMEBOY_0, vgmplay_device::A_GAMEBOY_0 + 0x16).w(m_dmg[0], FUNC(gameboy_sound_device::sound_w));
	map(vgmplay_device::A_GAMEBOY_0 + 0x20, vgmplay_device::A_GAMEBOY_0 + 0x2f).w(m_dmg[0], FUNC(gameboy_sound_device::wave_w));
	map(vgmplay_device::A_GAMEBOY_1, vgmplay_device::A_GAMEBOY_1 + 0x16).w(m_dmg[1], FUNC(gameboy_sound_device::sound_w));
	map(vgmplay_device::A_GAMEBOY_1 + 0x20, vgmplay_device::A_GAMEBOY_1 + 0x2f).w(m_dmg[1], FUNC(gameboy_sound_device::wave_w));
	map(vgmplay_device::A_NESAPU_0, vgmplay_device::A_NESAPU_0 + 0x1f).w("nescpu.0:nesapu", FUNC(nesapu_device::write));
	map(vgmplay_device::A_NES_RAM_0, vgmplay_device::A_NES_RAM_0 + 0xffff).ram().share("nesapu_ram.0");
	map(vgmplay_device::A_NESAPU_1, vgmplay_device::A_NESAPU_1 + 0x1f).w("nescpu.1:nesapu", FUNC(nesapu_device::write));
	map(vgmplay_device::A_NES_RAM_1, vgmplay_device::A_NES_RAM_1 + 0xffff).ram().share("nesapu_ram.1");
	map(vgmplay_device::A_MULTIPCM_0, vgmplay_device::A_MULTIPCM_0 + 3).w(m_multipcm[0], FUNC(multipcm_device::write));
	map(vgmplay_device::A_MULTIPCM_0 + 4, vgmplay_device::A_MULTIPCM_0 + 7).w("vgmplay", FUNC(vgmplay_device::multipcm_bank_hi_w<0>));
	map(vgmplay_device::A_MULTIPCM_0 + 8, vgmplay_device::A_MULTIPCM_0 + 11).w("vgmplay", FUNC(vgmplay_device::multipcm_bank_lo_w<0>));
	map(vgmplay_device::A_MULTIPCM_1, vgmplay_device::A_MULTIPCM_1 + 3).w(m_multipcm[1], FUNC(multipcm_device::write));
	map(vgmplay_device::A_MULTIPCM_1 + 4, vgmplay_device::A_MULTIPCM_1 + 7).w("vgmplay", FUNC(vgmplay_device::multipcm_bank_hi_w<1>));
	map(vgmplay_device::A_MULTIPCM_1 + 8, vgmplay_device::A_MULTIPCM_1 + 11).w("vgmplay", FUNC(vgmplay_device::multipcm_bank_lo_w<1>));
	map(vgmplay_device::A_UPD7759_0 + 0, vgmplay_device::A_UPD7759_0 + 0).w(FUNC(vgmplay_state::upd7759_reset_w<0>));
	map(vgmplay_device::A_UPD7759_0 + 1, vgmplay_device::A_UPD7759_0 + 1).lw8(NAME([this](uint8_t data) {m_upd7759[0]->start_w(data != 0); }));
	map(vgmplay_device::A_UPD7759_0 + 2, vgmplay_device::A_UPD7759_0 + 2).w(FUNC(vgmplay_state::upd7759_data_w<0>));
	map(vgmplay_device::A_UPD7759_0 + 3, vgmplay_device::A_UPD7759_0 + 3).w("vgmplay", FUNC(vgmplay_device::upd7759_bank_w<0>));
	map(vgmplay_device::A_UPD7759_1 + 0, vgmplay_device::A_UPD7759_1 + 0).w(FUNC(vgmplay_state::upd7759_reset_w<1>));
	map(vgmplay_device::A_UPD7759_1 + 1, vgmplay_device::A_UPD7759_1 + 1).lw8(NAME([this](uint8_t data) {m_upd7759[1]->start_w(data != 0); }));
	map(vgmplay_device::A_UPD7759_1 + 2, vgmplay_device::A_UPD7759_1 + 2).w(FUNC(vgmplay_state::upd7759_data_w<1>));
	map(vgmplay_device::A_UPD7759_1 + 3, vgmplay_device::A_UPD7759_1 + 3).w("vgmplay", FUNC(vgmplay_device::upd7759_bank_w<1>));
	map(vgmplay_device::A_OKIM6258_0 + 0x0, vgmplay_device::A_OKIM6258_0 + 0x0).w(m_okim6258[0], FUNC(okim6258_device::ctrl_w));
	map(vgmplay_device::A_OKIM6258_0 + 0x1, vgmplay_device::A_OKIM6258_0 + 0x1).w(m_okim6258[0], FUNC(okim6258_device::data_w));
	map(vgmplay_device::A_OKIM6258_0 + 0x2, vgmplay_device::A_OKIM6258_0 + 0x2).nopw(); // TODO: okim6258 pan
	map(vgmplay_device::A_OKIM6258_0 + 0x8, vgmplay_device::A_OKIM6258_0 + 0xb).w(FUNC(vgmplay_state::okim6258_clock_w<0>));
	map(vgmplay_device::A_OKIM6258_0 + 0xc, vgmplay_device::A_OKIM6258_0 + 0xc).w(FUNC(vgmplay_state::okim6258_divider_w<0>));
	map(vgmplay_device::A_OKIM6258_1 + 0x0, vgmplay_device::A_OKIM6258_1 + 0x0).w(m_okim6258[1], FUNC(okim6258_device::ctrl_w));
	map(vgmplay_device::A_OKIM6258_1 + 0x1, vgmplay_device::A_OKIM6258_1 + 0x1).w(m_okim6258[1], FUNC(okim6258_device::data_w));
	map(vgmplay_device::A_OKIM6258_1 + 0x2, vgmplay_device::A_OKIM6258_1 + 0x2).nopw(); // TODO: okim6258 pan
	map(vgmplay_device::A_OKIM6258_1 + 0x8, vgmplay_device::A_OKIM6258_1 + 0xb).w(FUNC(vgmplay_state::okim6258_clock_w<1>));
	map(vgmplay_device::A_OKIM6258_1 + 0xc, vgmplay_device::A_OKIM6258_1 + 0xc).w(FUNC(vgmplay_state::okim6258_divider_w<1>));
	map(vgmplay_device::A_OKIM6295_0, vgmplay_device::A_OKIM6295_0).w(m_okim6295[0], FUNC(okim6295_device::write));
	map(vgmplay_device::A_OKIM6295_0 + 0x8, vgmplay_device::A_OKIM6295_0 + 0xb).w(FUNC(vgmplay_state::okim6295_clock_w<0>));
	map(vgmplay_device::A_OKIM6295_0 + 0xc, vgmplay_device::A_OKIM6295_0 + 0xc).w(FUNC(vgmplay_state::okim6295_pin7_w<0>));
	map(vgmplay_device::A_OKIM6295_0 + 0xe, vgmplay_device::A_OKIM6295_0 + 0xe).w("vgmplay", FUNC(vgmplay_device::okim6295_nmk112_enable_w<0>));
	map(vgmplay_device::A_OKIM6295_0 + 0xf, vgmplay_device::A_OKIM6295_0 + 0xf).w("vgmplay", FUNC(vgmplay_device::okim6295_bank_w<0>));
	map(vgmplay_device::A_OKIM6295_0 + 0x10, vgmplay_device::A_OKIM6295_0 + 0x13).w("vgmplay", FUNC(vgmplay_device::okim6295_nmk112_bank_w<0>));
	map(vgmplay_device::A_OKIM6295_1, vgmplay_device::A_OKIM6295_1).w(m_okim6295[1], FUNC(okim6295_device::write));
	map(vgmplay_device::A_OKIM6295_1 + 0x8, vgmplay_device::A_OKIM6295_1 + 0xb).w(FUNC(vgmplay_state::okim6295_clock_w<1>));
	map(vgmplay_device::A_OKIM6295_1 + 0xc, vgmplay_device::A_OKIM6295_1 + 0xc).w(FUNC(vgmplay_state::okim6295_pin7_w<1>));
	map(vgmplay_device::A_OKIM6295_1 + 0xe, vgmplay_device::A_OKIM6295_1 + 0xe).w("vgmplay", FUNC(vgmplay_device::okim6295_nmk112_enable_w<1>));
	map(vgmplay_device::A_OKIM6295_1 + 0xf, vgmplay_device::A_OKIM6295_1 + 0xf).w("vgmplay", FUNC(vgmplay_device::okim6295_bank_w<1>));
	map(vgmplay_device::A_OKIM6295_1 + 0x10, vgmplay_device::A_OKIM6295_1 + 0x13).w("vgmplay", FUNC(vgmplay_device::okim6295_nmk112_bank_w<1>));
	map(vgmplay_device::A_K051649_0, vgmplay_device::A_K051649_0 + 0xf).w(FUNC(vgmplay_state::scc_w<0>));
	map(vgmplay_device::A_K051649_1, vgmplay_device::A_K051649_1 + 0xf).w(FUNC(vgmplay_state::scc_w<1>));
	map(vgmplay_device::A_K054539_0, vgmplay_device::A_K054539_0 + 0x22f).w(m_k054539[0], FUNC(k054539_device::write));
	map(vgmplay_device::A_K054539_1, vgmplay_device::A_K054539_1 + 0x22f).w(m_k054539[1], FUNC(k054539_device::write));
	map(vgmplay_device::A_C6280_0, vgmplay_device::A_C6280_0 + 0xf).w("huc6280.0:psg", FUNC(c6280_device::c6280_w));
	map(vgmplay_device::A_C6280_1, vgmplay_device::A_C6280_1 + 0xf).w("huc6280.1:psg", FUNC(c6280_device::c6280_w));
	map(vgmplay_device::A_C140_0, vgmplay_device::A_C140_0 + 0x1ff).w(FUNC(vgmplay_state::c140_c219_w<0>));
	map(vgmplay_device::A_C140_1, vgmplay_device::A_C140_1 + 0x1ff).w(FUNC(vgmplay_state::c140_c219_w<1>));
	map(vgmplay_device::A_K053260_0, vgmplay_device::A_K053260_0 + 0x2f).w(m_k053260[0], FUNC(k053260_device::write));
	map(vgmplay_device::A_K053260_1, vgmplay_device::A_K053260_1 + 0x2f).w(m_k053260[1], FUNC(k053260_device::write));
	map(vgmplay_device::A_POKEY_0, vgmplay_device::A_POKEY_0 + 0xf).w(m_pokey[0], FUNC(pokey_device::write));
	map(vgmplay_device::A_POKEY_1, vgmplay_device::A_POKEY_1 + 0xf).w(m_pokey[1], FUNC(pokey_device::write));
	map(vgmplay_device::A_QSOUND, vgmplay_device::A_QSOUND + 0x2).w(m_qsound, FUNC(qsound_device::qsound_w));
	map(vgmplay_device::A_VSU_VUE_0, vgmplay_device::A_VSU_VUE_0 + 0x5ff).w(m_vsu_vue[0], FUNC(vboysnd_device::write));
	map(vgmplay_device::A_VSU_VUE_1, vgmplay_device::A_VSU_VUE_1 + 0x5ff).w(m_vsu_vue[1], FUNC(vboysnd_device::write));
	map(vgmplay_device::A_SAA1099_0, vgmplay_device::A_SAA1099_0 + 1).w(m_saa1099[0], FUNC(saa1099_device::write));
	map(vgmplay_device::A_SAA1099_1, vgmplay_device::A_SAA1099_1 + 1).w(m_saa1099[1], FUNC(saa1099_device::write));
	map(vgmplay_device::A_ES5503_0, vgmplay_device::A_ES5503_0 + 0xe2).w(m_es5503[0], FUNC(es5503_device::write));
	map(vgmplay_device::A_ES5503_RAM_0, vgmplay_device::A_ES5503_RAM_0 + 0x1ffff).ram().share("es5503_ram.0");
	map(vgmplay_device::A_ES5503_1, vgmplay_device::A_ES5503_1 + 0xe2).w(m_es5503[1], FUNC(es5503_device::write));
	map(vgmplay_device::A_ES5503_RAM_1, vgmplay_device::A_ES5503_RAM_1 + 0x1ffff).ram().share("es5503_ram.1");
	// TODO: es5505
	map(vgmplay_device::A_X1_010_0, vgmplay_device::A_X1_010_0 + 0x1fff).w(m_x1_010[0], FUNC(x1_010_device::write));
	map(vgmplay_device::A_X1_010_1, vgmplay_device::A_X1_010_1 + 0x1fff).w(m_x1_010[1], FUNC(x1_010_device::write));
	map(vgmplay_device::A_GA20_0, vgmplay_device::A_GA20_0 + 0x1f).w(m_ga20[0], FUNC(iremga20_device::write));
	map(vgmplay_device::A_GA20_1, vgmplay_device::A_GA20_1 + 0x1f).w(m_ga20[1], FUNC(iremga20_device::write));
}

void vgmplay_state::soundchips16le_map(address_map &map)
{
	map(vgmplay_device::A_32X_PWM, vgmplay_device::A_32X_PWM + 0xf).w(m_sega32x, FUNC(sega_32x_device::pwm_w));
	map(vgmplay_device::A_C352_0, vgmplay_device::A_C352_0 + 0x7fff).w(m_c352[0], FUNC(c352_device::write));
	map(vgmplay_device::A_C352_1, vgmplay_device::A_C352_1 + 0x7fff).w(m_c352[1], FUNC(c352_device::write));
	map(vgmplay_device::A_WSWAN_0, vgmplay_device::A_WSWAN_0 + 0xff).w(m_wswan[0], FUNC(wswan_sound_device::port_w));
	map(vgmplay_device::A_WSWAN_1, vgmplay_device::A_WSWAN_1 + 0xff).w(m_wswan[1], FUNC(wswan_sound_device::port_w));
	map(vgmplay_device::A_WSWAN_RAM_0, vgmplay_device::A_WSWAN_RAM_0 + 0x3fff).ram().share("wswan_ram.0");
	map(vgmplay_device::A_WSWAN_RAM_1, vgmplay_device::A_WSWAN_RAM_1 + 0x3fff).ram().share("wswan_ram.1");
}

void vgmplay_state::soundchips16be_map(address_map &map)
{
	map(vgmplay_device::A_SCSP_0, vgmplay_device::A_SCSP_0 + 0xfff).w(m_scsp[0], FUNC(scsp_device::write));
	map(vgmplay_device::A_SCSP_1, vgmplay_device::A_SCSP_1 + 0xfff).w(m_scsp[1], FUNC(scsp_device::write));
	map(vgmplay_device::A_SCSP_RAM_0, vgmplay_device::A_SCSP_RAM_0 + 0xfffff).ram().share("scsp_ram.0");
	map(vgmplay_device::A_SCSP_RAM_1, vgmplay_device::A_SCSP_RAM_1 + 0xfffff).ram().share("scsp_ram.1");
}

template<int Index>
void vgmplay_state::segapcm_map(address_map &map)
{
	map(0, 0x1fffff).r("vgmplay", FUNC(vgmplay_device::segapcm_rom_r<Index>));
}

template<int Index>
void vgmplay_state::rf5c68_map(address_map &map)
{
	map(0, 0xffff).ram().share(Index ? "rf5c68_ram.1" : "rf5c68_ram.0");
}

template<int Index>
void vgmplay_state::ym2608_map(address_map &map)
{
	map(0, 0x1fffff).r("vgmplay", FUNC(vgmplay_device::ym2608_rom_r<Index>));
}

template<int Index>
void vgmplay_state::ym2610_adpcm_a_map(address_map &map)
{
	map(0, 0xffffff).r("vgmplay", FUNC(vgmplay_device::ym2610_adpcm_a_rom_r<Index>));
}

template<int Index>
void vgmplay_state::ym2610_adpcm_b_map(address_map &map)
{
	map(0, 0xffffff).r("vgmplay", FUNC(vgmplay_device::ym2610_adpcm_b_rom_r<Index>));
}

template<int Index>
void vgmplay_state::y8950_map(address_map &map)
{
	map(0, 0x1fffff).r("vgmplay", FUNC(vgmplay_device::y8950_rom_r<Index>));
}

template<int Index>
void vgmplay_state::ymf278b_map(address_map &map)
{
	map(0, 0x3fffff).r("vgmplay", FUNC(vgmplay_device::ymf278b_rom_r<Index>));
}

template<int Index>
void vgmplay_state::ymf271_map(address_map &map)
{
	map(0, 0x7fffff).r("vgmplay", FUNC(vgmplay_device::ymf271_rom_r<Index>));
}

template<int Index>
void vgmplay_state::ymz280b_map(address_map &map)
{
	map(0, 0xffffff).r("vgmplay", FUNC(vgmplay_device::ymz280b_rom_r<Index>));
}

template<int Index>
void vgmplay_state::nescpu_map(address_map &map)
{
	map(0, 0xffff).ram().share(Index ? "nesapu_ram.1" : "nesapu_ram.0");
}

template<int Index>
void vgmplay_state::multipcm_map(address_map &map)
{
	map(0, 0x3fffff).r("vgmplay", FUNC(vgmplay_device::multipcm_rom_r<Index>));
}

template<int Index>
void vgmplay_state::upd7759_map(address_map &map)
{
	map(0, 0x1ffff).r("vgmplay", FUNC(vgmplay_device::upd7759_rom_r<Index>));
}

template<int Index>
void vgmplay_state::okim6295_map(address_map &map)
{
	map(0, 0x3ffff).r("vgmplay", FUNC(vgmplay_device::okim6295_rom_r<Index>));
}

template<int Index>
void vgmplay_state::k054539_map(address_map &map)
{
	map(0, 0xffffff).r("vgmplay", FUNC(vgmplay_device::k054539_rom_r<Index>));
}

template<int Index>
void vgmplay_state::c140_map(address_map &map)
{
	map(0, 0x1ffffff).r("vgmplay", FUNC(vgmplay_device::c140_rom_r<Index>));
}

template<int Index>
void vgmplay_state::c219_map(address_map &map)
{
	map(0, 0x07ffff).r("vgmplay", FUNC(vgmplay_device::c219_rom_r<Index>));
}

template<int Index>
void vgmplay_state::k053260_map(address_map &map)
{
	map(0, 0x1fffff).r("vgmplay", FUNC(vgmplay_device::k053260_rom_r<Index>));
}

template<int Index>
void vgmplay_state::qsound_map(address_map &map)
{
	map(0, 0xffffff).r("vgmplay", FUNC(vgmplay_device::qsound_rom_r<Index>));
}

template<int Index>
void vgmplay_state::scsp_map(address_map &map)
{
	map(0, 0xfffff).ram().share(Index ? "scsp_ram.1" : "scsp_ram.0");
}

template<int Index>
void vgmplay_state::wswan_map(address_map &map)
{
	map(0, 0x3fff).ram().share(Index ? "wswan_ram.1" : "wswan_ram.0");
}

template<int Index>
void vgmplay_state::es5503_map(address_map &map)
{
	map(0, 0x1ffff).ram().share(Index ? "es5503_ram.1" : "es5503_ram.0");
}

template<int Index>
void vgmplay_state::es5505_map(address_map &map)
{
	map(0, 0xffffff).r("vgmplay", FUNC(vgmplay_device::es5505_rom_r<Index>));
}

template<int Index>
void vgmplay_state::x1_010_map(address_map &map)
{
	map(0, 0xfffff).r("vgmplay", FUNC(vgmplay_device::x1_010_rom_r<Index>));
}

template<int Index>
void vgmplay_state::c352_map(address_map &map)
{
	map(0, 0xffffff).r("vgmplay", FUNC(vgmplay_device::c352_rom_r<Index>));
}

template<int Index>
void vgmplay_state::ga20_map(address_map &map)
{
	map(0, 0xfffff).r("vgmplay", FUNC(vgmplay_device::ga20_rom_r<Index>));
}

template<int Index>
void vgmplay_state::rf5c164_map(address_map &map)
{
	map(0, 0xffff).ram().share("rf5c164_ram");
}

void vgmplay_state::vgmplay(machine_config &config)
{
	VGMPLAY(config, m_vgmplay, 44100);
	m_vgmplay->set_addrmap(AS_PROGRAM, &vgmplay_state::file_map);
	m_vgmplay->set_addrmap(AS_IO, &vgmplay_state::soundchips_map);
	m_vgmplay->set_addrmap(AS_IO16LE, &vgmplay_state::soundchips16le_map);
	m_vgmplay->set_addrmap(AS_IO16BE, &vgmplay_state::soundchips16be_map);

	quickload_image_device &quickload(QUICKLOAD(config, "quickload", "vgm,vgz"));
	quickload.set_load_callback(FUNC(vgmplay_state::load_file));
	quickload.set_interface("vgm_quik");

	SOFTWARE_LIST(config, "vgm_list").set_original("vgmplay");

	config.set_default_layout(layout_vgmplay);

	SN76489(config, m_sn76489[0], 0);
	m_sn76489[0]->add_route(0, m_mixer, 0.5, AUTO_ALLOC_INPUT, 0);
	m_sn76489[0]->add_route(0, m_mixer, 0.5, AUTO_ALLOC_INPUT, 1);

	SN76489(config, m_sn76489[1], 0);
	m_sn76489[1]->add_route(0, m_mixer, 0.5, AUTO_ALLOC_INPUT, 0);
	m_sn76489[1]->add_route(0, m_mixer, 0.5, AUTO_ALLOC_INPUT, 1);

	YM2413(config, m_ym2413[0], 0);
	m_ym2413[0]->add_route(ALL_OUTPUTS, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_ym2413[0]->add_route(ALL_OUTPUTS, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	YM2413(config, m_ym2413[1], 0);
	m_ym2413[1]->add_route(ALL_OUTPUTS, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_ym2413[1]->add_route(ALL_OUTPUTS, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	YM2612(config, m_ym2612[0], 0);
	m_ym2612[0]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_ym2612[0]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	YM2612(config, m_ym2612[1], 0);
	m_ym2612[1]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_ym2612[1]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	YM2151(config, m_ym2151[0], 0);
	m_ym2151[0]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_ym2151[0]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	YM2151(config, m_ym2151[1], 0);
	m_ym2151[1]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_ym2151[1]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	SEGAPCM(config, m_segapcm[0], 0);
	m_segapcm[0]->set_addrmap(0, &vgmplay_state::segapcm_map<0>);
	m_segapcm[0]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_segapcm[0]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	SEGAPCM(config, m_segapcm[1], 0);
	m_segapcm[1]->set_addrmap(0, &vgmplay_state::segapcm_map<1>);
	m_segapcm[1]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_segapcm[1]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	RF5C68(config, m_rf5c68, 0);
	m_rf5c68->set_addrmap(0, &vgmplay_state::rf5c68_map<0>);
	m_rf5c68->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_rf5c68->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	// TODO: prevent error.log spew
	YM2203(config, m_ym2203[0], 0);
	m_ym2203[0]->add_route(ALL_OUTPUTS, m_mixer, 0.25, AUTO_ALLOC_INPUT, 0);
	m_ym2203[0]->add_route(ALL_OUTPUTS, m_mixer, 0.25, AUTO_ALLOC_INPUT, 1);

	YM2203(config, m_ym2203[1], 0);
	m_ym2203[1]->add_route(ALL_OUTPUTS, m_mixer, 0.25, AUTO_ALLOC_INPUT, 0);
	m_ym2203[1]->add_route(ALL_OUTPUTS, m_mixer, 0.25, AUTO_ALLOC_INPUT, 1);

	// TODO: prevent error.log spew
	YM2608(config, m_ym2608[0], 0);
	m_ym2608[0]->set_addrmap(0, &vgmplay_state::ym2608_map<0>);
	m_ym2608[0]->add_route(0, m_mixer, 0.25, AUTO_ALLOC_INPUT, 0);
	m_ym2608[0]->add_route(0, m_mixer, 0.25, AUTO_ALLOC_INPUT, 1);
	m_ym2608[0]->add_route(1, m_mixer, 1.00, AUTO_ALLOC_INPUT, 0);
	m_ym2608[0]->add_route(2, m_mixer, 1.00, AUTO_ALLOC_INPUT, 1);

	YM2608(config, m_ym2608[1], 0);
	m_ym2608[1]->set_addrmap(0, &vgmplay_state::ym2608_map<1>);
	m_ym2608[1]->add_route(0, m_mixer, 0.25, AUTO_ALLOC_INPUT, 0);
	m_ym2608[1]->add_route(0, m_mixer, 0.25, AUTO_ALLOC_INPUT, 1);
	m_ym2608[1]->add_route(1, m_mixer, 1.00, AUTO_ALLOC_INPUT, 0);
	m_ym2608[1]->add_route(2, m_mixer, 1.00, AUTO_ALLOC_INPUT, 1);

	// TODO: prevent error.log spew
	YM2610(config, m_ym2610[0], 0);
	m_ym2610[0]->set_addrmap(0, &vgmplay_state::ym2610_adpcm_a_map<0>);
	m_ym2610[0]->set_addrmap(1, &vgmplay_state::ym2610_adpcm_b_map<0>);
	m_ym2610[0]->add_route(0, m_mixer, 0.25, AUTO_ALLOC_INPUT, 0);
	m_ym2610[0]->add_route(0, m_mixer, 0.25, AUTO_ALLOC_INPUT, 1);
	m_ym2610[0]->add_route(1, m_mixer, 0.50, AUTO_ALLOC_INPUT, 0);
	m_ym2610[0]->add_route(2, m_mixer, 0.50, AUTO_ALLOC_INPUT, 1);

	YM2610(config, m_ym2610[1], 0);
	m_ym2610[1]->set_addrmap(0, &vgmplay_state::ym2610_adpcm_a_map<1>);
	m_ym2610[1]->set_addrmap(1, &vgmplay_state::ym2610_adpcm_b_map<1>);
	m_ym2610[1]->add_route(0, m_mixer, 0.25, AUTO_ALLOC_INPUT, 0);
	m_ym2610[1]->add_route(0, m_mixer, 0.25, AUTO_ALLOC_INPUT, 1);
	m_ym2610[1]->add_route(1, m_mixer, 0.50, AUTO_ALLOC_INPUT, 0);
	m_ym2610[1]->add_route(2, m_mixer, 0.50, AUTO_ALLOC_INPUT, 1);

	YM3812(config, m_ym3812[0], 0);
	m_ym3812[0]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 0);
	m_ym3812[0]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 1);

	YM3812(config, m_ym3812[1], 0);
	m_ym3812[1]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 0);
	m_ym3812[1]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 1);

	YM3526(config, m_ym3526[0], 0);
	m_ym3526[0]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 0);
	m_ym3526[0]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 1);

	YM3526(config, m_ym3526[1], 0);
	m_ym3526[1]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 0);
	m_ym3526[1]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 1);

	Y8950(config, m_y8950[0], 0);
	m_y8950[0]->set_addrmap(0, &vgmplay_state::y8950_map<0>);
	m_y8950[0]->add_route(ALL_OUTPUTS, m_mixer, 0.40, AUTO_ALLOC_INPUT, 0);
	m_y8950[0]->add_route(ALL_OUTPUTS, m_mixer, 0.40, AUTO_ALLOC_INPUT, 1);

	Y8950(config, m_y8950[1], 0);
	m_y8950[1]->set_addrmap(0, &vgmplay_state::y8950_map<1>);
	m_y8950[1]->add_route(ALL_OUTPUTS, m_mixer, 0.40, AUTO_ALLOC_INPUT, 0);
	m_y8950[1]->add_route(ALL_OUTPUTS, m_mixer, 0.40, AUTO_ALLOC_INPUT, 1);

	YMF262(config, m_ymf262[0], 0);
	m_ymf262[0]->add_route(0, m_mixer, 1.00, AUTO_ALLOC_INPUT, 0);
	m_ymf262[0]->add_route(1, m_mixer, 1.00, AUTO_ALLOC_INPUT, 1);
	m_ymf262[0]->add_route(2, m_mixer, 1.00, AUTO_ALLOC_INPUT, 0);
	m_ymf262[0]->add_route(3, m_mixer, 1.00, AUTO_ALLOC_INPUT, 1);

	YMF262(config, m_ymf262[1], 0);
	m_ymf262[1]->add_route(0, m_mixer, 1.00, AUTO_ALLOC_INPUT, 0);
	m_ymf262[1]->add_route(1, m_mixer, 1.00, AUTO_ALLOC_INPUT, 1);
	m_ymf262[1]->add_route(2, m_mixer, 1.00, AUTO_ALLOC_INPUT, 0);
	m_ymf262[1]->add_route(3, m_mixer, 1.00, AUTO_ALLOC_INPUT, 1);

	// TODO: prevent error.log spew
	YMF278B(config, m_ymf278b[0], 0);
	m_ymf278b[0]->set_addrmap(0, &vgmplay_state::ymf278b_map<0>);
	m_ymf278b[0]->add_route(0, m_mixer, 1.00, AUTO_ALLOC_INPUT, 0);
	m_ymf278b[0]->add_route(1, m_mixer, 1.00, AUTO_ALLOC_INPUT, 1);
	m_ymf278b[0]->add_route(2, m_mixer, 1.00, AUTO_ALLOC_INPUT, 0);
	m_ymf278b[0]->add_route(3, m_mixer, 1.00, AUTO_ALLOC_INPUT, 1);
	m_ymf278b[0]->add_route(4, m_mixer, 1.00, AUTO_ALLOC_INPUT, 0);
	m_ymf278b[0]->add_route(5, m_mixer, 1.00, AUTO_ALLOC_INPUT, 1);

	YMF278B(config, m_ymf278b[1], 0);
	m_ymf278b[1]->set_addrmap(0, &vgmplay_state::ymf278b_map<1>);
	m_ymf278b[1]->add_route(0, m_mixer, 1.00, AUTO_ALLOC_INPUT, 0);
	m_ymf278b[1]->add_route(1, m_mixer, 1.00, AUTO_ALLOC_INPUT, 1);
	m_ymf278b[1]->add_route(2, m_mixer, 1.00, AUTO_ALLOC_INPUT, 0);
	m_ymf278b[1]->add_route(3, m_mixer, 1.00, AUTO_ALLOC_INPUT, 1);
	m_ymf278b[1]->add_route(4, m_mixer, 1.00, AUTO_ALLOC_INPUT, 0);
	m_ymf278b[1]->add_route(5, m_mixer, 1.00, AUTO_ALLOC_INPUT, 1);

	YMF271(config, m_ymf271[0], 0);
	m_ymf271[0]->set_addrmap(0, &vgmplay_state::ymf271_map<0>);
	m_ymf271[0]->add_route(0, m_mixer, 0.25, AUTO_ALLOC_INPUT, 0);
	m_ymf271[0]->add_route(1, m_mixer, 0.25, AUTO_ALLOC_INPUT, 1);
	m_ymf271[0]->add_route(2, m_mixer, 0.25, AUTO_ALLOC_INPUT, 0);
	m_ymf271[0]->add_route(3, m_mixer, 0.25, AUTO_ALLOC_INPUT, 1);

	YMF271(config, m_ymf271[1], 0);
	m_ymf271[1]->set_addrmap(0, &vgmplay_state::ymf271_map<0>);
	m_ymf271[1]->add_route(0, m_mixer, 0.25, AUTO_ALLOC_INPUT, 0);
	m_ymf271[1]->add_route(1, m_mixer, 0.25, AUTO_ALLOC_INPUT, 1);
	m_ymf271[1]->add_route(2, m_mixer, 0.25, AUTO_ALLOC_INPUT, 0);
	m_ymf271[1]->add_route(3, m_mixer, 0.25, AUTO_ALLOC_INPUT, 1);

	// TODO: prevent error.log spew
	YMZ280B(config, m_ymz280b[0], 0);
	m_ymz280b[0]->set_addrmap(0, &vgmplay_state::ymz280b_map<0>);
	m_ymz280b[0]->add_route(0, m_mixer, 0.50, AUTO_ALLOC_INPUT, 0);
	m_ymz280b[0]->add_route(1, m_mixer, 0.50, AUTO_ALLOC_INPUT, 1);

	YMZ280B(config, m_ymz280b[1], 0);
	m_ymz280b[1]->set_addrmap(0, &vgmplay_state::ymz280b_map<1>);
	m_ymz280b[1]->add_route(0, m_mixer, 0.50, AUTO_ALLOC_INPUT, 0);
	m_ymz280b[1]->add_route(1, m_mixer, 0.50, AUTO_ALLOC_INPUT, 1);

	RF5C164(config, m_rf5c164, 0);
	m_rf5c164->set_addrmap(0, &vgmplay_state::rf5c164_map<0>);
	m_rf5c164->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_rf5c164->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	/// TODO: rewrite to generate audio without using DAC devices
	SEGA_32X_NTSC(config, m_sega32x, 0, "sega32x_maincpu", "sega32x_scanline_timer");
	m_sega32x->add_route(0, m_mixer, 1.00, AUTO_ALLOC_INPUT, 0);
	m_sega32x->add_route(1, m_mixer, 1.00, AUTO_ALLOC_INPUT, 1);

	auto& sega32x_maincpu(M68000(config, "sega32x_maincpu", 0));
	sega32x_maincpu.set_disable();

	TIMER(config, "sega32x_scanline_timer", 0);

	m_sega32x->subdevice<cpu_device>("32x_master_sh2")->set_disable();
	m_sega32x->subdevice<cpu_device>("32x_slave_sh2")->set_disable();

	// TODO: prevent error.log spew
	AY8910(config, m_ay8910[0], 0);
	m_ay8910[0]->add_route(ALL_OUTPUTS, m_mixer, 0.33, AUTO_ALLOC_INPUT, 0);
	m_ay8910[0]->add_route(ALL_OUTPUTS, m_mixer, 0.33, AUTO_ALLOC_INPUT, 1);

	AY8910(config, m_ay8910[1], 0);
	m_ay8910[1]->add_route(ALL_OUTPUTS, m_mixer, 0.33, AUTO_ALLOC_INPUT, 0);
	m_ay8910[1]->add_route(ALL_OUTPUTS, m_mixer, 0.33, AUTO_ALLOC_INPUT, 1);

	DMG_APU(config, m_dmg[0], 0);
	m_dmg[0]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_dmg[0]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	DMG_APU(config, m_dmg[1], 0);
	m_dmg[1]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_dmg[1]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	N2A03(config, m_nescpu[0], 0);
	m_nescpu[0]->set_addrmap(AS_PROGRAM, &vgmplay_state::nescpu_map<0>);
	m_nescpu[0]->set_disable();
	m_nescpu[0]->add_route(ALL_OUTPUTS, m_mixer, 0.50, AUTO_ALLOC_INPUT, 0);
	m_nescpu[0]->add_route(ALL_OUTPUTS, m_mixer, 0.50, AUTO_ALLOC_INPUT, 1);

	N2A03(config, m_nescpu[1], 0);
	m_nescpu[1]->set_addrmap(AS_PROGRAM, &vgmplay_state::nescpu_map<1>);
	m_nescpu[1]->set_disable();
	m_nescpu[1]->add_route(ALL_OUTPUTS, m_mixer, 0.50, AUTO_ALLOC_INPUT, 0);
	m_nescpu[1]->add_route(ALL_OUTPUTS, m_mixer, 0.50, AUTO_ALLOC_INPUT, 1);

	MULTIPCM(config, m_multipcm[0], 0);
	m_multipcm[0]->set_addrmap(0, &vgmplay_state::multipcm_map<0>);
	m_multipcm[0]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_multipcm[0]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	MULTIPCM(config, m_multipcm[1], 0);
	m_multipcm[1]->set_addrmap(0, &vgmplay_state::multipcm_map<1>);
	m_multipcm[1]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_multipcm[1]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	UPD7759(config, m_upd7759[0], 0);
	m_upd7759[0]->drq().set(FUNC(vgmplay_state::upd7759_drq_w<0>));
	m_upd7759[0]->set_addrmap(0, &vgmplay_state::upd7759_map<0>);
	m_upd7759[0]->add_route(ALL_OUTPUTS, m_mixer, 1.0, AUTO_ALLOC_INPUT, 0);
	m_upd7759[0]->add_route(ALL_OUTPUTS, m_mixer, 1.0, AUTO_ALLOC_INPUT, 1);

	UPD7759(config, m_upd7759[1], 0);
	m_upd7759[1]->drq().set(FUNC(vgmplay_state::upd7759_drq_w<1>));
	m_upd7759[1]->set_addrmap(0, &vgmplay_state::upd7759_map<1>);
	m_upd7759[1]->add_route(ALL_OUTPUTS, m_mixer, 1.0, AUTO_ALLOC_INPUT, 0);
	m_upd7759[1]->add_route(ALL_OUTPUTS, m_mixer, 1.0, AUTO_ALLOC_INPUT, 1);

	OKIM6258(config, m_okim6258[0], 0);
	m_okim6258[0]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 0);
	m_okim6258[0]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 1);

	OKIM6258(config, m_okim6258[1], 0);
	m_okim6258[1]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 0);
	m_okim6258[1]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 1);

	OKIM6295(config, m_okim6295[0], 0, okim6295_device::PIN7_HIGH);
	m_okim6295[0]->set_addrmap(0, &vgmplay_state::okim6295_map<0>);
	m_okim6295[0]->add_route(ALL_OUTPUTS, m_mixer, 0.25, AUTO_ALLOC_INPUT, 0);
	m_okim6295[0]->add_route(ALL_OUTPUTS, m_mixer, 0.25, AUTO_ALLOC_INPUT, 1);

	OKIM6295(config, m_okim6295[1], 0, okim6295_device::PIN7_HIGH);
	m_okim6295[1]->set_addrmap(0, &vgmplay_state::okim6295_map<1>);
	m_okim6295[1]->add_route(ALL_OUTPUTS, m_mixer, 0.25, AUTO_ALLOC_INPUT, 0);
	m_okim6295[1]->add_route(ALL_OUTPUTS, m_mixer, 0.25, AUTO_ALLOC_INPUT, 1);

	K051649(config, m_k051649[0], 0);
	m_k051649[0]->add_route(ALL_OUTPUTS, m_mixer, 0.33, AUTO_ALLOC_INPUT, 0);
	m_k051649[0]->add_route(ALL_OUTPUTS, m_mixer, 0.33, AUTO_ALLOC_INPUT, 1);

	K051649(config, m_k051649[1], 0);
	m_k051649[1]->add_route(ALL_OUTPUTS, m_mixer, 0.33, AUTO_ALLOC_INPUT, 0);
	m_k051649[1]->add_route(ALL_OUTPUTS, m_mixer, 0.33, AUTO_ALLOC_INPUT, 1);

	K054539(config, m_k054539[0], 0);
	m_k054539[0]->set_addrmap(0, &vgmplay_state::k054539_map<0>);
	m_k054539[0]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_k054539[0]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	K054539(config, m_k054539[1], 0);
	m_k054539[1]->set_addrmap(0, &vgmplay_state::k054539_map<1>);
	m_k054539[1]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_k054539[1]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	// TODO: prevent error.log spew
	H6280(config, m_huc6280[0], 0);
	m_huc6280[0]->set_disable();
	m_huc6280[0]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_huc6280[0]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	H6280(config, m_huc6280[1], 0);
	m_huc6280[1]->set_disable();
	m_huc6280[1]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_huc6280[1]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	C140(config, m_c140[0], 0);
	m_c140[0]->set_addrmap(0, &vgmplay_state::c140_map<0>);
	m_c140[0]->add_route(0, m_mixer, 0.50, AUTO_ALLOC_INPUT, 0);
	m_c140[0]->add_route(1, m_mixer, 0.50, AUTO_ALLOC_INPUT, 1);

	C140(config, m_c140[1], 0);
	m_c140[1]->set_addrmap(0, &vgmplay_state::c140_map<1>);
	m_c140[1]->add_route(0, m_mixer, 0.50, AUTO_ALLOC_INPUT, 0);
	m_c140[1]->add_route(1, m_mixer, 0.50, AUTO_ALLOC_INPUT, 1);

	C219(config, m_c219[0], 0);
	m_c219[0]->set_addrmap(0, &vgmplay_state::c219_map<0>);
	m_c219[0]->add_route(0, m_mixer, 0.50, AUTO_ALLOC_INPUT, 0);
	m_c219[0]->add_route(1, m_mixer, 0.50, AUTO_ALLOC_INPUT, 1);

	C219(config, m_c219[1], 0);
	m_c219[1]->set_addrmap(0, &vgmplay_state::c219_map<1>);
	m_c219[1]->add_route(0, m_mixer, 0.50, AUTO_ALLOC_INPUT, 0);
	m_c219[1]->add_route(1, m_mixer, 0.50, AUTO_ALLOC_INPUT, 1);

	K053260(config, m_k053260[0], 0);
	m_k053260[0]->set_addrmap(0, &vgmplay_state::k053260_map<0>);
	m_k053260[0]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_k053260[0]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	K053260(config, m_k053260[1], 0);
	m_k053260[1]->set_addrmap(0, &vgmplay_state::k053260_map<1>);
	m_k053260[1]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_k053260[1]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	POKEY(config, m_pokey[0], 0);
	m_pokey[0]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 0);
	m_pokey[0]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 1);

	POKEY(config, m_pokey[1], 0);
	m_pokey[1]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 0);
	m_pokey[1]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 1);

	QSOUND(config, m_qsound, 0);
	m_qsound->set_addrmap(0, &vgmplay_state::qsound_map<0>);
	m_qsound->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_qsound->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	SCSP(config, m_scsp[0], 0);
	m_scsp[0]->set_addrmap(0, &vgmplay_state::scsp_map<0>);
	m_scsp[0]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_scsp[0]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	SCSP(config, m_scsp[1], 0);
	m_scsp[1]->set_addrmap(0, &vgmplay_state::scsp_map<1>);
	m_scsp[1]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_scsp[1]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	WSWAN_SND(config, m_wswan[0], 0);
	m_wswan[0]->set_addrmap(0, &vgmplay_state::wswan_map<0>);
	m_wswan[0]->add_route(0, m_mixer, 0.50, AUTO_ALLOC_INPUT, 0);
	m_wswan[0]->add_route(1, m_mixer, 0.50, AUTO_ALLOC_INPUT, 1);

	WSWAN_SND(config, m_wswan[1], 0);
	m_wswan[1]->set_addrmap(0, &vgmplay_state::wswan_map<1>);
	m_wswan[1]->add_route(0, m_mixer, 0.50, AUTO_ALLOC_INPUT, 0);
	m_wswan[1]->add_route(1, m_mixer, 0.50, AUTO_ALLOC_INPUT, 1);

	VBOYSND(config, m_vsu_vue[0], 0);
	m_vsu_vue[0]->add_route(0, m_mixer, 1.0, AUTO_ALLOC_INPUT, 0);
	m_vsu_vue[0]->add_route(1, m_mixer, 1.0, AUTO_ALLOC_INPUT, 1);

	VBOYSND(config, m_vsu_vue[1], 0);
	m_vsu_vue[1]->add_route(0, m_mixer, 1.0, AUTO_ALLOC_INPUT, 0);
	m_vsu_vue[1]->add_route(1, m_mixer, 1.0, AUTO_ALLOC_INPUT, 1);

	SAA1099(config, m_saa1099[0], 0);
	m_saa1099[0]->add_route(0, m_mixer, 1.0, AUTO_ALLOC_INPUT, 0);
	m_saa1099[0]->add_route(1, m_mixer, 1.0, AUTO_ALLOC_INPUT, 1);

	SAA1099(config, m_saa1099[1], 0);
	m_saa1099[1]->add_route(0, m_mixer, 1.0, AUTO_ALLOC_INPUT, 0);
	m_saa1099[1]->add_route(1, m_mixer, 1.0, AUTO_ALLOC_INPUT, 1);

	ES5503(config, m_es5503[0], 0);
	m_es5503[0]->set_channels(2);
	m_es5503[0]->set_addrmap(0, &vgmplay_state::es5503_map<0>);
	m_es5503[0]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 0);
	m_es5503[0]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 1);

	ES5503(config, m_es5503[1], 0);
	m_es5503[1]->set_channels(2);
	m_es5503[1]->set_addrmap(0, &vgmplay_state::es5503_map<1>);
	m_es5503[1]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 0);
	m_es5503[1]->add_route(ALL_OUTPUTS, m_mixer, 0.5, AUTO_ALLOC_INPUT, 1);

	ES5505(config, m_es5505[0], 0);
	// TODO m_es5505[0]->set_addrmap(0, &vgmplay_state::es5505_map<0>);
	// TODO m_es5505[0]->set_addrmap(1, &vgmplay_state::es5505_map<0>);
	m_es5505[0]->set_channels(1);
	m_es5505[0]->add_route(0, m_mixer, 0.5, AUTO_ALLOC_INPUT, 0);
	m_es5505[0]->add_route(1, m_mixer, 0.5, AUTO_ALLOC_INPUT, 1);

	ES5505(config, m_es5505[1], 0);
	// TODO m_es5505[1]->set_addrmap(0, &vgmplay_state::es5505_map<1>);
	// TODO m_es5505[1]->set_addrmap(1, &vgmplay_state::es5505_map<1>);
	m_es5505[1]->set_channels(1);
	m_es5505[1]->add_route(0, m_mixer, 0.5, AUTO_ALLOC_INPUT, 0);
	m_es5505[1]->add_route(1, m_mixer, 0.5, AUTO_ALLOC_INPUT, 1);

	X1_010(config, m_x1_010[0], 0);
	m_x1_010[0]->set_addrmap(0, &vgmplay_state::x1_010_map<0>);
	m_x1_010[0]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_x1_010[0]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	X1_010(config, m_x1_010[1], 0);
	m_x1_010[1]->set_addrmap(0, &vgmplay_state::x1_010_map<1>);
	m_x1_010[1]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_x1_010[1]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	C352(config, m_c352[0], 0, 1);
	m_c352[0]->set_addrmap(0, &vgmplay_state::c352_map<0>);
	m_c352[0]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_c352[0]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);
	m_c352[0]->add_route(2, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_c352[0]->add_route(3, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	C352(config, m_c352[1], 0, 1);
	m_c352[1]->set_addrmap(0, &vgmplay_state::c352_map<1>);
	m_c352[1]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_c352[1]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);
	m_c352[1]->add_route(2, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_c352[1]->add_route(3, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	IREMGA20(config, m_ga20[0], 0);
	m_ga20[0]->set_addrmap(0, &vgmplay_state::ga20_map<0>);
	m_ga20[0]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_ga20[0]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	IREMGA20(config, m_ga20[1], 0);
	m_ga20[1]->set_addrmap(0, &vgmplay_state::ga20_map<1>);
	m_ga20[1]->add_route(0, m_mixer, 1, AUTO_ALLOC_INPUT, 0);
	m_ga20[1]->add_route(1, m_mixer, 1, AUTO_ALLOC_INPUT, 1);

	VGMVIZ(config, m_mixer, 0);
	m_mixer->add_route(0, "lspeaker", 1);
	m_mixer->add_route(1, "rspeaker", 1);

	SPEAKER(config, m_lspeaker).front_left();
	SPEAKER(config, m_rspeaker).front_right();
}

ROM_START( vgmplay )
	// TODO: split up 32x to remove dependencies
	ROM_REGION( 0x4000, "master", ROMREGION_ERASE00 )
	ROM_REGION( 0x4000, "slave", ROMREGION_ERASE00 )
	ROM_REGION( 0x400000, "gamecart", ROMREGION_ERASE00 )
	ROM_REGION32_BE( 0x400000, "gamecart_sh2", ROMREGION_ERASE00 )
ROM_END

CONS( 2016, vgmplay, 0, 0, vgmplay, vgmplay, vgmplay_state, empty_init, "MAME", "VGM player", MACHINE_CLICKABLE_ARTWORK )
