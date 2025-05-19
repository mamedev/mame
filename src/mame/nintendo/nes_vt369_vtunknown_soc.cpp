// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "nes_vt369_vtunknown_soc.h"

#define LOG_VT3XX_SOUND     (1U << 1)

#define LOG_ALL     (LOG_VT3XX_SOUND)

#define VERBOSE     (0)

#include "logmacro.h"


// this has a new RGB555 mode
DEFINE_DEVICE_TYPE(VT369_SOC_INTROM_NOSWAP, vt369_soc_introm_noswap_device, "vt369_soc", "VT369 series System on a Chip")
DEFINE_DEVICE_TYPE(VT369_SOC_INTROM_SWAP,   vt369_soc_introm_swap_device,   "vt369_soc_swap",    "VT369 series System on a Chip (with D5/D6 opcode swapping)")
DEFINE_DEVICE_TYPE(VT369_SOC_INTROM_ALTSWAP,vt369_soc_introm_altswap_device,"vt369_soc_altswap", "VT369 series System on a Chip (with D1/D4 opcode swapping)")
DEFINE_DEVICE_TYPE(VT369_SOC_INTROM_VIBESSWAP,vt369_soc_introm_vibesswap_device,"vt369_soc_vibesswap", "VT369 series System on a Chip (with D4/D5 opcode swapping)")

// uncertain
DEFINE_DEVICE_TYPE(VT3XX_SOC, vt3xx_soc_base_device,          "vt3xx_unknown_soc_cy", "VT3xx series System on a Chip (CY)")
DEFINE_DEVICE_TYPE(VT3XX_SOC_UNK_BT, vt3xx_soc_unk_bt_device, "vt3xx_unknown_soc_bt", "VT3xx series System on a Chip (BT)")

DEFINE_DEVICE_TYPE(VT3XX_SOC_UNK_DG, vt3xx_soc_unk_dg_device, "vt3xx_unknown_soc_dg", "VT3xx series System on a Chip (DG)")
DEFINE_DEVICE_TYPE(VT3XX_SOC_UNK_FA, vt3xx_soc_unk_fa_device, "vt3xx_unknown_soc_fa", "VT3xx series System on a Chip (Family Pocket)")


vt3xx_soc_base_device::vt3xx_soc_base_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	vt3xx_soc_base_device(mconfig, VT3XX_SOC, tag, owner, clock)
{
}

vt3xx_soc_base_device::vt3xx_soc_base_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	nes_vt09_soc_device(mconfig, type, tag, owner, clock),
	m_alu(*this, "alu"),
	m_soundcpu(*this, "soundcpu"),
	m_sound_timer(nullptr),
	m_internal_rom(*this, "internal"),
	m_soundram(*this, "soundram")
{
}

vt3xx_soc_unk_bt_device::vt3xx_soc_unk_bt_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	vt3xx_soc_base_device(mconfig, VT3XX_SOC_UNK_BT, tag, owner, clock)
{
}


vt369_soc_introm_noswap_device::vt369_soc_introm_noswap_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	vt3xx_soc_base_device(mconfig, type, tag, owner, clock)
{
}

vt369_soc_introm_noswap_device::vt369_soc_introm_noswap_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	vt369_soc_introm_noswap_device(mconfig, VT369_SOC_INTROM_NOSWAP, tag, owner, clock)
{
}

vt369_soc_introm_swap_device::vt369_soc_introm_swap_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	vt369_soc_introm_noswap_device(mconfig, VT369_SOC_INTROM_SWAP, tag, owner, clock)
{
}

vt369_soc_introm_altswap_device::vt369_soc_introm_altswap_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	vt369_soc_introm_noswap_device(mconfig, VT369_SOC_INTROM_ALTSWAP, tag, owner, clock)
{
}

vt369_soc_introm_vibesswap_device::vt369_soc_introm_vibesswap_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	vt369_soc_introm_noswap_device(mconfig, VT369_SOC_INTROM_VIBESSWAP, tag, owner, clock)
{
}

vt3xx_soc_unk_dg_device::vt3xx_soc_unk_dg_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	vt3xx_soc_base_device(mconfig, type, tag, owner, clock)
{
}

vt3xx_soc_unk_dg_device::vt3xx_soc_unk_dg_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	vt3xx_soc_unk_dg_device(mconfig, VT3XX_SOC_UNK_DG, tag, owner, clock)
{
}

vt3xx_soc_unk_fa_device::vt3xx_soc_unk_fa_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	vt3xx_soc_unk_dg_device(mconfig, VT3XX_SOC_UNK_FA, tag, owner, clock)
{
}

/***********************************************************************************************************************************************************/
/* VT369? */
/***********************************************************************************************************************************************************/

void vt3xx_soc_base_device::device_add_mconfig(machine_config& config)
{
	nes_vt02_vt03_soc_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &vt3xx_soc_base_device::vt369_map);

	PPU_VT3XX(config.replace(), m_ppu, RP2A03_NTSC_XTAL);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_ppu->read_bg().set(FUNC(vt3xx_soc_base_device::chr_r));
	m_ppu->read_sp().set(FUNC(vt3xx_soc_base_device::spr_r));
	m_ppu->read_newmode_sp().set(FUNC(vt3xx_soc_base_device::newmode_spr_r));
	m_ppu->set_screen(m_screen);

	VT_VT1682_ALU(config, m_alu, 0);

	VT3XX_SPU(config, m_soundcpu, RP2A03_NTSC_XTAL);
	m_soundcpu->set_addrmap(AS_PROGRAM, &vt3xx_soc_base_device::vt369_sound_map);
	m_soundcpu->set_addrmap(5, &vt3xx_soc_base_device::vt369_sound_external_map);	
}

void vt3xx_soc_base_device::vt369_soundcpu_control_w(offs_t offset, uint8_t data)
{
	logerror("%s: write to sound cpu control reg (reset etc.) %02x\n", machine().describe_context(), data);

	if (data == 0x0d)
		m_soundcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	else
		m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

void vt3xx_soc_base_device::vt369_relative_w(offs_t offset, uint8_t data)
{
	printf("%s: vt369_relative_w %02x %02x\n", machine().describe_context().c_str(), offset,  data);
	m_relative[offset] = data;
}

uint8_t vt3xx_soc_base_device::read_internal(offs_t offset)
{
	if (!m_internal_rom)
	{
		if (!machine().side_effects_disabled())
			logerror("%s: read from internal ROM (offset %04x), but no internal ROM loaded\n", machine().describe_context(), offset);
		return 0x00;
	}

	return m_internal_rom[offset];
}

void vt3xx_soc_base_device::vt369_map(address_map &map)
{
	map(0x0000, 0x1fff).ram(); // 8k RAM?

	// ddrdismx relies on the mirroring, later SoCs have different mirroring?
	map(0x2000, 0x2007).rw(m_ppu, FUNC(ppu_vt3xx_device::read), FUNC(ppu_vt3xx_device::write));  // standard PPU registers
	map(0x2008, 0x2008).rw(m_ppu, FUNC(ppu_vt3xx_device::read_spritehigh), FUNC(ppu_vt3xx_device::write_spritehigh));  // standard PPU registers
	map(0x2010, 0x2010).rw(m_ppu, FUNC(ppu_vt3xx_device::extended_modes_enable_r), FUNC(ppu_vt3xx_device::extended_modes_enable_w));
	map(0x2011, 0x2011).rw(m_ppu, FUNC(ppu_vt3xx_device::extended_modes2_enable_r), FUNC(ppu_vt3xx_device::extended_modes2_enable_w));
	map(0x2012, 0x2012).rw(m_ppu, FUNC(ppu_vt3xx_device::videobank0_0_r), FUNC(ppu_vt3xx_device::videobank0_0_w));
	map(0x2013, 0x2013).rw(m_ppu, FUNC(ppu_vt3xx_device::videobank0_1_r), FUNC(ppu_vt3xx_device::videobank0_1_w));
	map(0x2014, 0x2014).rw(m_ppu, FUNC(ppu_vt3xx_device::videobank0_2_r), FUNC(ppu_vt3xx_device::videobank0_2_w));
	map(0x2015, 0x2015).rw(m_ppu, FUNC(ppu_vt3xx_device::videobank0_3_r), FUNC(ppu_vt3xx_device::videobank0_3_w));
	map(0x2016, 0x2016).rw(m_ppu, FUNC(ppu_vt3xx_device::videobank0_4_r), FUNC(ppu_vt3xx_device::videobank0_4_w));
	map(0x2017, 0x2017).rw(m_ppu, FUNC(ppu_vt3xx_device::videobank0_5_r), FUNC(ppu_vt3xx_device::videobank0_5_w));
	map(0x2018, 0x2018).rw(m_ppu, FUNC(ppu_vt3xx_device::videobank1_r), FUNC(ppu_vt3xx_device::videobank1_w));
	map(0x2019, 0x2019).rw(m_ppu, FUNC(ppu_vt3xx_device::read_2019), FUNC(ppu_vt3xx_device::gun_reset_w));
	map(0x201a, 0x201a).rw(m_ppu, FUNC(ppu_vt3xx_device::videobank0_extra_r), FUNC(ppu_vt3xx_device::videobank0_extra_w));
	map(0x201b, 0x201b).r(m_ppu, FUNC(ppu_vt3xx_device::read_201b));
	map(0x201c, 0x201c).rw(m_ppu, FUNC(ppu_vt3xx_device::read_201c_newvid), FUNC(ppu_vt3xx_device::write_201c_newvid));
	map(0x201d, 0x201d).rw(m_ppu, FUNC(ppu_vt3xx_device::read_201d_newvid), FUNC(ppu_vt3xx_device::write_201d_newvid));
	map(0x201e, 0x201e).rw(m_ppu, FUNC(ppu_vt3xx_device::read_201e_newvid), FUNC(ppu_vt3xx_device::write_201e_newvid));
	map(0x201f, 0x201f).r(m_ppu, FUNC(ppu_vt3xx_device::gun2_y_r));

	map(0x2020, 0x2023).rw(m_ppu, FUNC(ppu_vt3xx_device::read_202x_newvid), FUNC(ppu_vt3xx_device::write_202x_newvid));

	map(0x2040, 0x2049).w(m_ppu, FUNC(ppu_vt3xx_device::write_204x_screenregs));

	map(0x3000, 0x3fff).ram(); // 240in1ar clears this region (does it only exist on some SoCs?)

	map(0x4000, 0x4017).w(m_apu, FUNC(nes_apu_vt_device::write));

	map(0x4014, 0x4014).w(FUNC(vt3xx_soc_base_device::vt_dma_w));
	map(0x4015, 0x4015).r(m_apu, FUNC(nes_apu_vt_device::status_r)); // PSG status / first control register
	map(0x4016, 0x4016).rw(FUNC(vt3xx_soc_base_device::in0_r), FUNC(vt3xx_soc_base_device::in0_w));
	map(0x4017, 0x4017).r(FUNC(vt3xx_soc_base_device::in1_r));

	map(0x4024, 0x4024).w(FUNC(vt3xx_soc_base_device::vt3xx_4024_new_dma_middle_w));

	map(0x4034, 0x4034).w(FUNC(vt3xx_soc_base_device::vt03_4034_w));

	map(0x4100, 0x410b).r(FUNC(vt3xx_soc_base_device::vt03_410x_r)).w(FUNC(vt3xx_soc_base_device::vt03_410x_w));
	// 0x410c unused
	map(0x410d, 0x410d).w(FUNC(vt3xx_soc_base_device::extra_io_control_w));
	map(0x410e, 0x410e).rw(FUNC(vt3xx_soc_base_device::extrain_01_r), FUNC(vt3xx_soc_base_device::extraout_01_w));
	map(0x410f, 0x410f).rw(FUNC(vt3xx_soc_base_device::extrain_23_r), FUNC(vt3xx_soc_base_device::extraout_23_w));

	map(0x4112, 0x4112).w(FUNC(vt3xx_soc_base_device::vt369_4112_bank6000_select_w));

	// 0x4114 RS232 timer (low)
	// 0x4115 RS232 timer (high)
	// 0x4116 unused
	// 0x4117 unused
	// 0x4118 unused
	map(0x4119, 0x4119).r(FUNC(vt3xx_soc_base_device::rs232flags_region_r));
	// 0x411a RS232 TX data
	// 0x411b RS232 RX data
	map(0x411c, 0x411c).w(FUNC(vt3xx_soc_base_device::vt369_411c_bank6000_enable_w));

	// 412d

	// is the ALU really compatible with the 1682 one?
	map(0x4130, 0x4130).rw(m_alu, FUNC(vrt_vt1682_alu_device::alu_out_1_r), FUNC(vrt_vt1682_alu_device::alu_oprand_1_w));
	map(0x4131, 0x4131).rw(m_alu, FUNC(vrt_vt1682_alu_device::alu_out_2_r), FUNC(vrt_vt1682_alu_device::alu_oprand_2_w));
	map(0x4132, 0x4132).rw(m_alu, FUNC(vrt_vt1682_alu_device::alu_out_3_r), FUNC(vrt_vt1682_alu_device::alu_oprand_3_w));
	map(0x4133, 0x4133).rw(m_alu, FUNC(vrt_vt1682_alu_device::alu_out_4_r), FUNC(vrt_vt1682_alu_device::alu_oprand_4_w));
	map(0x4134, 0x4134).rw(m_alu, FUNC(vrt_vt1682_alu_device::alu_out_5_r), FUNC(vrt_vt1682_alu_device::alu_oprand_5_mult_w));
	map(0x4135, 0x4135).rw(m_alu, FUNC(vrt_vt1682_alu_device::alu_out_6_r), FUNC(vrt_vt1682_alu_device::alu_oprand_6_mult_w));
	map(0x4136, 0x4136).w(m_alu, FUNC(vrt_vt1682_alu_device::alu_oprand_5_div_w));
	map(0x4137, 0x4137).w(m_alu, FUNC(vrt_vt1682_alu_device::alu_oprand_6_div_w));

	// 4144
	// 4147

	map(0x414f, 0x414f).r(FUNC(vt3xx_soc_base_device::vt369_414f_r));

	// several games use these addresses for what seem to be extra protection data
	map(0x4150, 0x4150).rw(FUNC(vt3xx_soc_base_device::extra_rom_prot_4150_r), FUNC(vt3xx_soc_base_device::extra_rom_prot_4150_w));
	// 4151 also sometimes written
	map(0x4152, 0x4152).rw(FUNC(vt3xx_soc_base_device::extra_rom_prot_4152_r), FUNC(vt3xx_soc_base_device::extra_rom_prot_4152_w));
	map(0x4153, 0x4153).r(FUNC(vt3xx_soc_base_device::extra_rom_prot_4153_r)); // extra SPI? / SEEPROM port?
	// 0x4158 is written before the above

	map(0x415c, 0x415c).r(FUNC(vt3xx_soc_base_device::vt369_415c_r)); // related to getting into menus in some games

	map(0x4160, 0x4161).w(FUNC(vt3xx_soc_base_device::vt369_relative_w));
	map(0x4162, 0x4162).w(FUNC(vt3xx_soc_base_device::vt369_soundcpu_control_w));

	// 4175

	map(0x418a, 0x418a).r(FUNC(vt3xx_soc_base_device::vt369_418a_r));

	map(0x41b0, 0x41bf).r(FUNC(vt3xx_soc_base_device::vt369_41bx_r)).w(FUNC(vt3xx_soc_base_device::vt369_41bx_w));

	// 4304

	map(0x4800, 0x4fff).ram().share("soundram"); // sound program for 2nd CPU is uploaded here, but some sets aren't uploading anything, do they rely on an internal ROM? other DMA? possibility to map ROM?

	map(0x6000, 0x7fff).r(FUNC(vt3xx_soc_base_device::vt369_6000_r)).w(FUNC(vt3xx_soc_base_device::vt369_6000_w));

	map(0x8000, 0xffff).rw(FUNC(vt3xx_soc_base_device::external_space_read), FUNC(vt3xx_soc_base_device::external_space_write));
}

void vt3xx_soc_base_device::vt_dma_w(uint8_t data)
{
	const bool ALLOW_NEW_DMA = true; // seems OK for cases we have at the moment

	if ((m_bank6000_enable & 0x80) && ALLOW_NEW_DMA)
	{
		uint16_t src_addr = (m_4024_newdma) | data << 8;

		int length = (m_vdma_ctrl >> 1) & 7;
		if (length == 0) length = 8;
		length = 1 << length;

		logerror("%s: attempting to do NEW style dma src %04x length %04x dest type %d\n", machine().describe_context(), src_addr, length, m_vdma_ctrl & 1);

		for (int i = 0; i < length; i++)
		{
			uint8_t read_data = m_maincpu->space(AS_PROGRAM).read_byte(src_addr + i);
			if (m_vdma_ctrl & 1)
			{
				m_maincpu->space(AS_PROGRAM).write_byte(0x2007, read_data);
			}
			else
			{
				m_maincpu->space(AS_PROGRAM).write_byte(0x2004, read_data);
			}
		}
	}
	else
	{
		nes_vt09_soc_device::vt_dma_w(data);
	}
}

// this reads from the 'extra ROM' area (serial style protocol) and code is copied on gtct885 to e00 in RAM, jumps to it at EDF9: jsr $0e1c
// extra_rom_prot_4153_r is used by lxccminn, lxccplan and dgun2561 to check battery state instead? reports low battery and does nothing else if unhappy
uint8_t vt3xx_soc_base_device::extra_rom_prot_4153_r() { logerror("%s: extra_rom_prot_4153_r (protection? / extra SPI device?)\n", machine().describe_context()); return 0xff; }
// pactin and tetrtin use these for something similar, seems to want code/data for jumps?
uint8_t vt3xx_soc_base_device::extra_rom_prot_4150_r() { logerror("%s: extra_rom_prot_4150_r (protection? / extra SPI device?)\n", machine().describe_context()); return machine().rand(); }
uint8_t vt3xx_soc_base_device::extra_rom_prot_4152_r() { logerror("%s: extra_rom_prot_4152_r (protection? / extra SPI device?)\n", machine().describe_context()); return machine().rand(); }
void vt3xx_soc_base_device::extra_rom_prot_4152_w(uint8_t data) { logerror("%s: extra_rom_prot_4152_w %02x (protection? / extra SPI device?)\n", machine().describe_context(), data); }
void vt3xx_soc_base_device::extra_rom_prot_4150_w(uint8_t data) { logerror("%s: extra_rom_prot_4150_w %02x (protection? / extra SPI device?)\n", machine().describe_context(), data); }

void vt3xx_soc_base_device::update_timer()
{
	if (m_timercontrol & 0x01)
	{
		// TODO: use m_timerperiod
		m_sound_timer->adjust(attotime::from_hz(400), 0);
	}
	else
	{
		m_sound_timer->adjust(attotime::never);
	}
}

TIMER_CALLBACK_MEMBER(vt3xx_soc_base_device::sound_timer_expired)
{
	if (m_timercontrol & 0x02)
	{
		m_soundcpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
	}
	update_timer();
}

void vt3xx_soc_base_device::vt369_soundcpu_timer_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x00:
		m_timerperiod = (m_timerperiod & 0xff00) | data;
		LOGMASKED(LOG_VT3XX_SOUND, "%s: vt369_soundcpu_timer_w %02x %02x (period low byte)\n", machine().describe_context(), offset, data);
		break;

	case 0x01:
		m_timerperiod = (m_timerperiod & 0x00ff) | data << 8;
		LOGMASKED(LOG_VT3XX_SOUND, "%s: vt369_soundcpu_timer_w %02x %02x (period high byte)\n", machine().describe_context(), offset, data);
		break;

	case 0x02:
		// 0x01 - enable timer
		// 0x02 - enable timer IRQ
		LOGMASKED(LOG_VT3XX_SOUND, "%s: vt369_soundcpu_timer_w %02x %02x (control)\n", machine().describe_context(), offset, data);
		m_timercontrol = data;
		update_timer();
		break;

	case 0x03:
		LOGMASKED(LOG_VT3XX_SOUND, "%s: vt369_soundcpu_timer_w %02x %02x (clear IRQ)\n", machine().describe_context(), offset, data);
		break;

	default:
		LOGMASKED(LOG_VT3XX_SOUND, "%s: vt369_soundcpu_timer_w %02x %02x\n", machine().describe_context(), offset, data);
		break;
	}
}

void vt3xx_soc_base_device::vt369_soundcpu_adder_data_address_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_VT3XX_SOUND, "%s: vt369_soundcpu_adder_data_address_w %02x %02x\n", machine().describe_context(), offset, data);
}

uint8_t vt3xx_soc_base_device::vt369_soundcpu_adder_result_r(offs_t offset)
{
	LOGMASKED(LOG_VT3XX_SOUND, "%s: vt369_soundcpu_adder_result_r %02x\n", machine().describe_context(), offset);
	return 0x00;
}

void vt3xx_soc_base_device::vt369_soundcpu_mult_data_address_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_VT3XX_SOUND, "%s: vt369_soundcpu_mult_data_address_w %02x %02x\n", machine().describe_context(), offset, data);
}

uint8_t vt3xx_soc_base_device::vt369_soundcpu_mult_result_r(offs_t offset)
{
	LOGMASKED(LOG_VT3XX_SOUND, "%s: vt369_soundcpu_mult_result_r %02x\n", machine().describe_context(), offset);
	return 0x00;
}

uint8_t vt3xx_soc_base_device::vt369_soundcpu_mult_status_r()
{
	LOGMASKED(LOG_VT3XX_SOUND, "%s: vt369_soundcpu_mult_status_r\n", machine().describe_context());
	return 0x00;
}

void vt3xx_soc_base_device::vt369_soundcpu_dac_w(offs_t offset, uint8_t data)
{
	// 2 16-bit channels?
	LOGMASKED(LOG_VT3XX_SOUND, "%s: vt369_soundcpu_dac_w %02x %02x\n", machine().describe_context(), offset, data);
}

uint8_t vt3xx_soc_base_device::vt369_soundcpu_vectors_r(offs_t offset)
{
	// timer IRQ (others are currently unused, point to rti, not clear what they're for or how they're enabled)
	if ((offset == 0x04) || (offset == 0x05))
		return m_soundram[0x7f8 + (offset & 1)];

	return m_soundram[0x7fa + offset];
}

void vt3xx_soc_base_device::vt369_sound_map(address_map &map)
{
	map(0x0000, 0x17ff).ram();
	map(0x1800, 0x1fff).ram().share("soundram");

	map(0x2100, 0x2103).w(FUNC(vt3xx_soc_base_device::vt369_soundcpu_timer_w));
	// 0x2204
	map(0x2205, 0x2206).w(FUNC(vt3xx_soc_base_device::vt369_soundcpu_adder_data_address_w));
	// 0x2207
	map(0x2210, 0x2211).r(FUNC(vt3xx_soc_base_device::vt369_soundcpu_adder_result_r));
	map(0x2400, 0x2401).w(FUNC(vt3xx_soc_base_device::vt369_soundcpu_mult_data_address_w));
	map(0x2402, 0x2403).r(FUNC(vt3xx_soc_base_device::vt369_soundcpu_mult_result_r));
	map(0x2404, 0x2404).r(FUNC(vt3xx_soc_base_device::vt369_soundcpu_mult_status_r));
	map(0x2800, 0x2803).w(FUNC(vt3xx_soc_base_device::vt369_soundcpu_dac_w));

	map(0x4000, 0x4fff).r(FUNC(vt369_soc_introm_noswap_device::read_internal)); // some lexibook sets suggest the internal ROM can also appear here?

	map(0xfffa, 0xffff).r(FUNC(vt3xx_soc_base_device::vt369_soundcpu_vectors_r));
}

void vt3xx_soc_base_device::vt369_sound_external_map(address_map &map)
{
	map(0x000000, 0xffffff).r(FUNC(vt3xx_soc_base_device::sound_read_external));
}


void vt3xx_soc_base_device::vt369_411c_bank6000_enable_w(offs_t offset, uint8_t data)
{
	// and CPU clock scaling on bit 0x80?

	logerror("enable bank at 0x6000 (%02x)\n", data);
	m_bank6000_enable = data;
}


void vt3xx_soc_base_device::vt369_4112_bank6000_select_w(offs_t offset, uint8_t data)
{
	logerror("set bank at 0x6000 to %02x\n", data);
	m_bank6000 = data;

	// 0x3c = 0x78000
}


uint8_t vt3xx_soc_base_device::vt369_6000_r(offs_t offset)
{
	if (m_bank6000_enable & 0x40)
	{
		address_space& spc = this->space(AS_PROGRAM);
		int address = (m_bank6000 * 0x2000) + (offset & 0x1fff);
		return spc.read_byte(address);
	}
	else
	{
		return m_6000_ram[offset];
	}
}

void vt3xx_soc_base_device::vt369_6000_w(offs_t offset, uint8_t data)
{
	if (m_bank6000_enable & 0x40)
	{
		logerror("%s: write to 0x6xxx with ROM enabled? %04x %02x\n", machine().describe_context(), offset, data);
	}
	else
	{
		m_6000_ram[offset] = data;
	}
}


// vt3xx has a bigger palette, starting at 3c00, it seems unaffected the fallthrough / mirroring
uint8_t vt3xx_soc_base_device::vt3xx_palette_r(offs_t offset)
{
	if (m_ppu->is_v3xx_extended_mode()) // or maybe if in 'fast' CPU mode (m_bank6000_enable &0x80)
	{
		return m_ppu->vt3xx_extended_palette_r(offset);
	}
	else
	{
		if (offset < 0x300)
		{
			return nt_r(offset + 0x3c00);
		}
		else
		{
			return m_ppu->palette_read(offset - 0x300);
		}
	}
}

void vt3xx_soc_base_device::vt3xx_palette_w(offs_t offset, uint8_t data)
{
	if (m_ppu->is_v3xx_extended_mode()) // or maybe if in 'fast' CPU mode
	{
		m_ppu->vt3xx_extended_palette_w(offset,data);
	}
	else
	{
		if (offset < 0x300)
		{
			nt_w(offset + 0x3c00, data);
		}
		else
		{
			m_ppu->palette_write(offset - 0x300, data);
		}
	}
}

void vt3xx_soc_base_device::device_start()
{
	nes_vt02_vt03_soc_device::device_start();

	m_6000_ram.resize(0x2000);
	m_bank6000 = 0;
	m_bank6000_enable = 0;

	m_sound_timer = timer_alloc(FUNC(vt3xx_soc_base_device::sound_timer_expired), this);

	save_item(NAME(m_timerperiod));
	save_item(NAME(m_timercontrol));
	save_item(NAME(m_6000_ram));
	save_item(NAME(m_bank6000));
	save_item(NAME(m_bank6000_enable));

	m_ppu->space(AS_PROGRAM).install_readwrite_handler(0x3c00, 0x3fff, read8sm_delegate(*this, FUNC(vt3xx_soc_base_device::vt3xx_palette_r)), write8sm_delegate(*this, FUNC(vt3xx_soc_base_device::vt3xx_palette_w)));
}

void vt3xx_soc_base_device::device_reset()
{
	nes_vt02_vt03_soc_device::device_reset();
	m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_timerperiod = 0;
	m_timercontrol = 0;
	m_sound_timer->adjust(attotime::never);
}




uint8_t vt3xx_soc_base_device::vt369_41bx_r(offs_t offset)
{
	logerror("%s: vt369_41bx_r %02x (unknown)\n", machine().describe_context(), offset);

	switch (offset)
	{
	case 0x07:
		return 0x04;
	default:
		return 0x00;
	}
}

void vt3xx_soc_base_device::vt369_41bx_w(offs_t offset, uint8_t data)
{
	logerror("%s: vt369_41bx_w %02x %02x (unknown)\n", machine().describe_context(), offset, data);
}


uint8_t vt3xx_soc_base_device::vt369_414f_r()
{
	logerror("%s: vt369_414f_r (unknown)\n", machine().describe_context());
	return 0xff;
}

uint8_t vt3xx_soc_base_device::vt369_415c_r()
{
	logerror("%s: vt369_415c_r (unknown - important)\n", machine().describe_context());
	// returning 0x00 allows zonefusn and lexi30 (and many other lexibook sets) to show menus, but stops sealvt from showing anything
	// returning 0xff allows sealvt to show the boot screen
	// 0xf0 allows all those to boot?
	return 0xf0; 
}

uint8_t vt3xx_soc_base_device::vt369_418a_r()
{
	logerror("%s: vt369_418a_r (unknown)\n", machine().describe_context());
	return machine().rand();
}

uint8_t vt3xx_soc_base_device::newmode_spr_r(offs_t offset)
{
	address_space& spc = this->space(AS_PROGRAM);
	return spc.read_byte(get_relative() + offset);
}

/***********************************************************************************************************************************************************/
/* this might just be the same as vt369 but with the games not using all features */
/***********************************************************************************************************************************************************/

void vt369_soc_introm_noswap_device::device_add_mconfig(machine_config& config)
{
	vt3xx_soc_base_device::device_add_mconfig(config);

	RP2A03_CORE_SWAP_OP_D5_D6(config.replace(), m_maincpu, NTSC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &vt369_soc_introm_noswap_device::vt369_introm_map);
}

void vt369_soc_introm_noswap_device::device_start()
{
	vt3xx_soc_base_device::device_start();
	downcast<rp2a03_core_swap_op_d5_d6 &>(*m_maincpu).disable_encryption_on_reset();
	m_encryption_allowed = false;
}

void vt369_soc_introm_noswap_device::vtfp_411d_w(uint8_t data)
{
	// controls chram access and mapper emulation modes in later models
	logerror("vtfp_411d_w  %02x\n", data);
	m_411d = data;
	update_banks();
}

uint8_t vt369_soc_introm_noswap_device::vthh_414a_r()
{
	return 0x80;
}


void vt369_soc_introm_noswap_device::vt369_introm_map(address_map &map)
{
	vt3xx_soc_base_device::vt369_map(map);

	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1fff).r(FUNC(vt369_soc_introm_noswap_device::read_internal));

	map(0x414a, 0x414a).r(FUNC(vt369_soc_introm_noswap_device::vthh_414a_r));
	map(0x411d, 0x411d).w(FUNC(vt369_soc_introm_noswap_device::vtfp_411d_w));


	map(0x4169, 0x4169).w(FUNC(vt369_soc_introm_noswap_device::encryption_4169_w));
}


void vt369_soc_introm_noswap_device::encryption_4169_w(uint8_t data)
{
	if (m_encryption_allowed)
	{
		if (data == 0x01)
			downcast<rp2a03_core_swap_op_d5_d6&>(*m_maincpu).set_encryption_state(false);
		else if (data == 0x00)
			downcast<rp2a03_core_swap_op_d5_d6&>(*m_maincpu).set_encryption_state(true);
		else
			logerror("%s: encryption_4169_w %02x\n", machine().describe_context(), data);
	}
	else
	{
		logerror("%s: encryption_4169_w %02x on SoC with no support (check!)\n", machine().describe_context(), data);
	}
}

void vt369_soc_introm_swap_device::device_start()
{
	vt3xx_soc_base_device::device_start();
	m_encryption_allowed = true;
}

void vt369_soc_introm_altswap_device::device_start()
{
	vt3xx_soc_base_device::device_start();
	downcast<rp2a03_core_swap_op_d5_d6 &>(*m_maincpu).set_which_crypt(1);
	m_encryption_allowed = true;
}

void vt369_soc_introm_vibesswap_device::device_start()
{
	vt3xx_soc_base_device::device_start();
	downcast<rp2a03_core_swap_op_d5_d6 &>(*m_maincpu).set_which_crypt(2);
	m_encryption_allowed = true;
}






/***********************************************************************************************************************************************************/
/* this might also just be the same as vt369 but with the games not using all features */
/***********************************************************************************************************************************************************/

void vt3xx_soc_unk_dg_device::device_add_mconfig(machine_config& config)
{
	vt3xx_soc_base_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &vt3xx_soc_unk_dg_device::nes_vt_dg_map);
}

void vt3xx_soc_unk_dg_device::vt03_411c_w(uint8_t data)
{
	logerror("vt03_411c_w  %02x\n", data);
	m_411c = data;
	update_banks();
}

void vt3xx_soc_unk_dg_device::nes_vt_dg_map(address_map &map)
{
	vt3xx_soc_base_device::vt369_map(map);
	map(0x411c, 0x411c).w(FUNC(vt3xx_soc_unk_dg_device::vt03_411c_w));
}

/***********************************************************************************************************************************************************/
/* 'BT' specifics (base = '4K') */
/***********************************************************************************************************************************************************/

void vt3xx_soc_unk_bt_device::device_add_mconfig(machine_config& config)
{
	vt3xx_soc_base_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &vt3xx_soc_unk_bt_device::nes_vt_bt_map);
}

void vt3xx_soc_unk_bt_device::nes_vt_bt_map(address_map &map)
{
	vt3xx_soc_base_device::vt369_map(map);
	map(0x412c, 0x412c).w(FUNC(vt3xx_soc_unk_bt_device::vt03_412c_extbank_w));
}

void vt3xx_soc_unk_bt_device::vt03_412c_extbank_w(uint8_t data)
{
	m_upper_write_412c_callback(data);
}


/***********************************************************************************************************************************************************/
/* 'FA' specifics (base = 'DG') */ // used by fapocket
/***********************************************************************************************************************************************************/

void vt3xx_soc_unk_fa_device::device_add_mconfig(machine_config& config)
{
	vt3xx_soc_base_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &vt3xx_soc_unk_fa_device::nes_vt_fa_map);
}

uint8_t vt3xx_soc_unk_fa_device::vtfa_412c_r()
{
	return m_upper_read_412c_callback();
}

void vt3xx_soc_unk_fa_device::vtfa_412c_extbank_w(uint8_t data)
{
	m_upper_write_412c_callback(data);

}

void vt3xx_soc_unk_fa_device::vtfp_4242_w(uint8_t data)
{
	logerror("vtfp_4242_w %02x\n", data);
	m_4242 = data;
}

void vt3xx_soc_unk_fa_device::nes_vt_fa_map(address_map &map)
{
	vt3xx_soc_base_device::vt369_map(map);
	map(0x412c, 0x412c).r(FUNC(vt3xx_soc_unk_fa_device::vtfa_412c_r)).w(FUNC(vt3xx_soc_unk_fa_device::vtfa_412c_extbank_w));
	map(0x4242, 0x4242).w(FUNC(vt3xx_soc_unk_fa_device::vtfp_4242_w));
}

