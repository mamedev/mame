// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "nes_vt4ffx_soc.h"

#include "cpu/m6502/rp2a03.h"
#include "sound/nes_apu_vt.h"
#include "m6502_swap_op_d5_d6.h"
#include "video/ppu2c0x_vt.h"


DEFINE_DEVICE_TYPE(VT4FFX_SOC_NOSWAP,     vt4ffx_soc_noswap_device,     "vt4ffx_soc",           "VT series System on a Chip with $4FFx registers")
DEFINE_DEVICE_TYPE(VT4FFX_SOC_VIBESSWAP,  vt4ffx_soc_vibesswap_device,  "vt4ffx_soc_vibesswap", "VT series System on a Chip with $4FFx registers and D4/D5 opcode swapping")
DEFINE_DEVICE_TYPE(VT4FFX_SOC_GBOX2020,   vt4ffx_soc_gbox2020_device,   "vt4ffx_soc_gbox2020",  "VT series System on a Chip with $4FFx registers and D6/D7 + D1/D2 opcode swapping")
DEFINE_DEVICE_TYPE(VT4FFX_SOC_S10SWAP,    vt4ffx_soc_s10swap_device,    "vt4ffx_soc_s10swap",   "VT series System on a Chip with $4FFx registers and D4/D5 + D1/D2 opcode swapping")
DEFINE_DEVICE_TYPE(VT4FFX_SOC_RSPS300SWAP,vt4ffx_soc_rsps300swap_device,"vt4ffx_soc_rsps300",   "VT series System on a Chip with $4FFx registers and D6/D7 opcode swapping")


vt4ffx_soc_base_device::vt4ffx_soc_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	nes_vt02_vt03_soc_device(mconfig, type, tag, owner, clock),
	m_io_413x_write_callback(*this),
	m_io_413x_read_callback(*this, 0xff)
{
}


vt4ffx_soc_noswap_device::vt4ffx_soc_noswap_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	vt4ffx_soc_base_device(mconfig, type, tag, owner, clock)
{
}

vt4ffx_soc_noswap_device::vt4ffx_soc_noswap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	vt4ffx_soc_noswap_device(mconfig, VT4FFX_SOC_NOSWAP, tag, owner, clock)
{
}

vt4ffx_soc_vibesswap_device::vt4ffx_soc_vibesswap_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	vt4ffx_soc_noswap_device(mconfig, type, tag, owner, clock)
{
}

vt4ffx_soc_vibesswap_device::vt4ffx_soc_vibesswap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	vt4ffx_soc_vibesswap_device(mconfig, VT4FFX_SOC_VIBESSWAP, tag, owner, clock)
{
}

vt4ffx_soc_gbox2020_device::vt4ffx_soc_gbox2020_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	vt4ffx_soc_vibesswap_device(mconfig, VT4FFX_SOC_GBOX2020, tag, owner, clock)
{
}

vt4ffx_soc_s10swap_device::vt4ffx_soc_s10swap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	vt4ffx_soc_vibesswap_device(mconfig, VT4FFX_SOC_S10SWAP, tag, owner, clock)
{
}

vt4ffx_soc_rsps300swap_device::vt4ffx_soc_rsps300swap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	vt4ffx_soc_noswap_device(mconfig, VT4FFX_SOC_RSPS300SWAP, tag, owner, clock)
{
}


void vt4ffx_soc_base_device::device_add_mconfig(machine_config &config)
{
	nes_vt02_vt03_soc_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &vt4ffx_soc_base_device::vt369_map);

	PPU_VT3XX(config.replace(), m_ppu, RP2A03_NTSC_XTAL);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_ppu->read_bg().set(FUNC(vt4ffx_soc_base_device::chr_r));
	m_ppu->read_sp().set(FUNC(vt4ffx_soc_base_device::spr_r));
	m_ppu->read_onespace_with_relative().set(FUNC(vt4ffx_soc_base_device::read_onespace_bus_with_relative_offset));
	m_ppu->set_screen(m_screen);
}

void vt4ffx_soc_base_device::vt369_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();

	// ddrdismx relies on the mirroring, later SoCs have different mirroring?
	map(0x2000, 0x2007).rw(m_ppu, FUNC(ppu_vt3xx_device::read), FUNC(ppu_vt3xx_device::write));  // standard PPU registers
	map(0x2008, 0x2008).rw(m_ppu, FUNC(ppu_vt3xx_device::spritehigh_2008_r), FUNC(ppu_vt3xx_device::spritehigh_2008_w));  // standard PPU registers
	map(0x2010, 0x2010).rw(m_ppu, FUNC(ppu_vt3xx_device::extended_modes_enable_r), FUNC(ppu_vt3xx_device::extended_modes_enable_w));
	map(0x2011, 0x2011).rw(m_ppu, FUNC(ppu_vt3xx_device::extended_modes2_enable_r), FUNC(ppu_vt3xx_device::extended_modes2_enable_w));
	map(0x2012, 0x2012).rw(m_ppu, FUNC(ppu_vt3xx_device::videobank0_0_r), FUNC(ppu_vt3xx_device::videobank0_0_w));
	map(0x2013, 0x2013).rw(m_ppu, FUNC(ppu_vt3xx_device::videobank0_1_r), FUNC(ppu_vt3xx_device::videobank0_1_w));
	map(0x2014, 0x2014).rw(m_ppu, FUNC(ppu_vt3xx_device::videobank0_2_r), FUNC(ppu_vt3xx_device::videobank0_2_w));
	map(0x2015, 0x2015).rw(m_ppu, FUNC(ppu_vt3xx_device::videobank0_3_r), FUNC(ppu_vt3xx_device::videobank0_3_w));
	map(0x2016, 0x2016).rw(m_ppu, FUNC(ppu_vt3xx_device::videobank0_4_r), FUNC(ppu_vt3xx_device::videobank0_4_w));
	map(0x2017, 0x2017).rw(m_ppu, FUNC(ppu_vt3xx_device::videobank0_5_r), FUNC(ppu_vt3xx_device::videobank0_5_w));
	map(0x2018, 0x2018).rw(m_ppu, FUNC(ppu_vt3xx_device::videobank1_r), FUNC(ppu_vt3xx_device::videobank1_w));
	map(0x2019, 0x2019).rw(m_ppu, FUNC(ppu_vt3xx_device::unk_2019_r), FUNC(ppu_vt3xx_device::gun_reset_w));
	map(0x201a, 0x201a).rw(m_ppu, FUNC(ppu_vt3xx_device::videobank0_extra_r), FUNC(ppu_vt3xx_device::videobank0_extra_w));
	map(0x201b, 0x201b).r(m_ppu, FUNC(ppu_vt3xx_device::unk_201b_r));
	map(0x201c, 0x201c).rw(m_ppu, FUNC(ppu_vt3xx_device::extvidreg_201c_r), FUNC(ppu_vt3xx_device::extvidreg_201c_w));
	map(0x201d, 0x201d).rw(m_ppu, FUNC(ppu_vt3xx_device::extvidreg_201d_r), FUNC(ppu_vt3xx_device::extvidreg_201d_w));
	map(0x201e, 0x201e).rw(m_ppu, FUNC(ppu_vt3xx_device::extvidreg_201e_r), FUNC(ppu_vt3xx_device::extvidreg_201e_w));
	map(0x201f, 0x201f).r(m_ppu, FUNC(ppu_vt3xx_device::gun2_y_r));

	map(0x2020, 0x2023).rw(m_ppu, FUNC(ppu_vt3xx_device::tilebases_202x_r), FUNC(ppu_vt3xx_device::tilebases_202x_w));

	map(0x2040, 0x2049).w(m_ppu, FUNC(ppu_vt3xx_device::lcdc_regs_w));

	map(0x4000, 0x4017).nopr().w(m_apu, FUNC(nes_apu_vt_device::write));

	map(0x4014, 0x4014).w(FUNC(vt4ffx_soc_base_device::vt_dma_w));
	map(0x4015, 0x4015).r(m_apu, FUNC(nes_apu_vt_device::status_r)); // PSG status / first control register
	map(0x4016, 0x4016).rw(FUNC(vt4ffx_soc_base_device::in0_r), FUNC(vt4ffx_soc_base_device::in0_w));
	map(0x4017, 0x4017).r(FUNC(vt4ffx_soc_base_device::in1_r));

	map(0x4024, 0x4024).w(FUNC(vt4ffx_soc_base_device::vt3xx_4024_new_dma_middle_w));

	map(0x4034, 0x4034).w(FUNC(vt4ffx_soc_base_device::vt03_4034_w));

	map(0x4100, 0x410b).r(FUNC(vt4ffx_soc_base_device::vt03_410x_r)).w(FUNC(vt4ffx_soc_base_device::vt03_410x_w));
	// 0x410c unused
	map(0x410d, 0x410d).w(FUNC(vt4ffx_soc_base_device::extra_io_control_w));
	map(0x410e, 0x410e).rw(FUNC(vt4ffx_soc_base_device::extrain_01_r), FUNC(vt4ffx_soc_base_device::extraout_01_w));
	map(0x410f, 0x410f).rw(FUNC(vt4ffx_soc_base_device::extrain_23_r), FUNC(vt4ffx_soc_base_device::extraout_23_w));

	map(0x4112, 0x4112).w(FUNC(vt4ffx_soc_base_device::vt369_4112_bank6000_select_w));

	// 0x4114 RS232 timer (low)
	// 0x4115 RS232 timer (high)
	// 0x4116 unused
	// 0x4117 unused
	// 0x4118 unused
	map(0x4119, 0x4119).r(FUNC(vt4ffx_soc_base_device::rs232flags_region_r));
	// 0x411a RS232 TX data
	// 0x411b RS232 RX data
	map(0x411c, 0x411c).w(FUNC(vt4ffx_soc_base_device::vt369_411c_bank6000_enable_w));
	map(0x411d, 0x411d).w(FUNC(vt4ffx_soc_base_device::vt369_411d_w));
	map(0x411e, 0x411e).w(FUNC(vt4ffx_soc_base_device::vt369_411e_w));

	// 412c (rbbrite) = input port?
	// 412d
	// 4134
	// 4135 (t3_630, retro620, s10fake, f5_620) = input port?
	// 4136 (retro620, s10fake) = input port?
	// 4137 (gbox2020)

	map(0x4138, 0x4138).rw(FUNC(vt4ffx_soc_base_device::vt_413x_port_direction_r), FUNC(vt4ffx_soc_base_device::vt_413x_port_direction_w));
	map(0x4139, 0x4139).rw(FUNC(vt4ffx_soc_base_device::vt_413x_port_in_r), FUNC(vt4ffx_soc_base_device::vt_413x_port_out_w));
	// 413f is also written, could config the port?

	// 4242 (gbox2020)

	map(0x4ff0, 0x4ff0).w(FUNC(vt4ffx_soc_base_device::vt4ffx_ctrl_w));
	// 4ff1-4ff5
	// 4ff6
	map(0x4ff7, 0x4ff8).w(FUNC(vt4ffx_soc_base_device::vt4ffx_data_w));
	// 4ffa

	map(0x6000, 0x7fff).r(FUNC(vt4ffx_soc_base_device::vt369_6000_r)).w(FUNC(vt4ffx_soc_base_device::vt369_6000_w));

	map(0x8000, 0xffff).rw(FUNC(vt4ffx_soc_base_device::external_space_read), FUNC(vt4ffx_soc_base_device::external_space_write));
}

void vt4ffx_soc_base_device::vt_dma_w(u8 data)
{
	const bool ALLOW_NEW_DMA = true; // seems OK for cases we have at the moment

	if ((m_bank6000_enable & 0x80) && ALLOW_NEW_DMA)
	{
		u16 src_addr = (m_4024_newdma) | data << 8;

		int length = (m_vdma_ctrl >> 1) & 7;
		if (length == 0) length = 8;
		length = 1 << length;

		logerror("%s: attempting to do NEW style dma src %04x length %04x dest type %d\n", machine().describe_context(), src_addr, length, m_vdma_ctrl & 1);

		for (int i = 0; i < length; i++)
		{
			u8 read_data = m_maincpu->space(AS_PROGRAM).read_byte(src_addr + i);
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
		nes_vt02_vt03_soc_device::vt_dma_w(data);
	}
}

// goretrop seems to use a port with 4138 as direction, and 4139 as data for a similar protection

u8 vt4ffx_soc_base_device::vt_413x_port_direction_r()
{
	logerror("%s: vt_413x_port_direction_r (port 4139 direction)\n", machine().describe_context());
	return m_413x_port_direction;
}

void vt4ffx_soc_base_device::vt_413x_port_direction_w(u8 data)
{
	logerror("%s: vt_413x_port_direction_w %02x (port 4139 direction)\n", machine().describe_context(), data);
	m_413x_port_direction = data;
}

void vt4ffx_soc_base_device::vt_413x_port_out_w(u8 data)
{
	logerror("%s: vt_413x_port_out_w %02x (with direction register %02x)\n", machine().describe_context(), data, m_413x_port_direction);
	// TODO: pass direction register
	m_413x_port_data = data;
	m_io_413x_write_callback(data & m_413x_port_direction);
}

u8 vt4ffx_soc_base_device::vt_413x_port_in_r()
{
	logerror("%s: vt_413x_port_in_r (with direction register %02x)\n", machine().describe_context(), m_413x_port_direction);
	// TODO: pass the direction register
	u8 ret = m_io_413x_read_callback();
	return (ret & ~m_413x_port_direction) | (m_413x_port_data & m_413x_port_direction);
}


void vt4ffx_soc_base_device::vt4ffx_ctrl_w(u8 data)
{
	// GPIO outputs strobing CS and WR lines of external device?
	if ((data & 0x50) == 0 && (m_4ffx_ctrl & 0x50) == 0x10)
		logerror("%s: vt4ffx_ctrl_w %02x = %04x\n", machine().describe_context(), data, m_4ffx_data);
	m_4ffx_ctrl = data;
}

void vt4ffx_soc_base_device::vt4ffx_data_w(offs_t offset, u8 data)
{
	if (offset != 0)
		m_4ffx_data = data << 8 | (m_4ffx_data & 0x00ff);
	else
		m_4ffx_data = (m_4ffx_data & 0xff00) | data;
}


void vt4ffx_soc_base_device::vt369_411c_bank6000_enable_w(u8 data)
{
	if (m_bank6000_enable != data)
	{
		m_maincpu->set_clock(data & 0x80 ? NTSC_APU_CLOCK * 3 : NTSC_APU_CLOCK );
		logerror("%s: enable bank at 0x6000 + CPU clock multiplier (%02x)\n", machine().describe_context(), data);
	}

	m_bank6000_enable = data;
}

void vt4ffx_soc_base_device::vt369_411d_w(u8 data)
{
	// controls chram access and mapper emulation modes in later models
	// also written by rtvgc300 and rtvgc300fz (with the same value as 411e)
	// when external banking is needed?
	logerror("%s: vt369_411d_w  %02x\n", machine().describe_context(), data);
	m_411d = data;
	update_banks();
}

void vt4ffx_soc_base_device::vt369_411e_w(u8 data)
{
	logerror("%s: vt369_411e_w (%02x) (external bankswitch + more?)\n", machine().describe_context(), data);
	m_411e_write_cb(data);
}


void vt4ffx_soc_base_device::vt369_4112_bank6000_select_w(u8 data)
{
	logerror("%s: set bank at 0x6000 to %02x\n", machine().describe_context(), data);
	m_bank6000 = data;

	// 0x3c = 0x78000
}


u8 vt4ffx_soc_base_device::vt369_6000_r(offs_t offset)
{
	if (m_bank6000_enable & 0x40)
	{
		address_space& spc = this->space(AS_PROGRAM);
		// x the ball in lxcmcysp suggests we need to go through get_banks to get the higher bits
		int address = (get_banks(m_bank6000) * 0x2000) + (offset & 0x1fff);
		return spc.read_byte(get_relative() + address);
	}
	else
	{
		return m_6000_ram[offset];
	}
}

void vt4ffx_soc_base_device::vt369_6000_w(offs_t offset, u8 data)
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
u8 vt4ffx_soc_base_device::vt3xx_palette_r(offs_t offset)
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

void vt4ffx_soc_base_device::vt3xx_palette_w(offs_t offset, u8 data)
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

void vt4ffx_soc_base_device::device_start()
{
	nes_vt02_vt03_soc_device::device_start();

	m_6000_ram.resize(0x2000);
	m_bank6000 = 0;
	m_bank6000_enable = 0;

	save_item(NAME(m_6000_ram));
	save_item(NAME(m_bank6000));
	save_item(NAME(m_bank6000_enable));

	save_item(NAME(m_413x_port_direction));
	save_item(NAME(m_413x_port_data));
	save_item(NAME(m_4ffx_ctrl));
	save_item(NAME(m_4ffx_data));

	m_ppu->space(AS_PROGRAM).install_readwrite_handler(0x3c00, 0x3fff, read8sm_delegate(*this, FUNC(vt4ffx_soc_base_device::vt3xx_palette_r)), write8sm_delegate(*this, FUNC(vt4ffx_soc_base_device::vt3xx_palette_w)));
}

void vt4ffx_soc_base_device::device_reset()
{
	nes_vt02_vt03_soc_device::device_reset();

	m_413x_port_direction = 0x00;
	m_413x_port_data = 0x00;

	m_4ffx_ctrl = 0xff;
	m_4ffx_data = 0;
}



u8 vt4ffx_soc_base_device::read_onespace_bus_with_relative_offset(offs_t offset)
{
	address_space& spc = this->space(AS_PROGRAM);
	return spc.read_byte(get_relative() + offset);
}


void vt4ffx_soc_noswap_device::device_add_mconfig(machine_config &config)
{
	vt4ffx_soc_base_device::device_add_mconfig(config);

	RP2A03_CORE_SWAP_OP_D5_D6(config.replace(), m_maincpu, NTSC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &vt4ffx_soc_noswap_device::vt369_map);
}

void vt4ffx_soc_noswap_device::device_start()
{
	vt4ffx_soc_base_device::device_start();
	downcast<rp2a03_core_swap_op_d5_d6 &>(*m_maincpu).disable_encryption_on_reset();
	m_encryption_allowed = false;
}


void vt4ffx_soc_vibesswap_device::device_add_mconfig(machine_config &config)
{
	vt4ffx_soc_noswap_device::device_add_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &vt4ffx_soc_vibesswap_device::nes_vt_vibes_map);
}

void vt4ffx_soc_vibesswap_device::device_start()
{
	vt4ffx_soc_base_device::device_start();
	downcast<rp2a03_core_swap_op_d5_d6 &>(*m_maincpu).set_which_crypt(2);
	m_encryption_allowed = true;
}

void vt4ffx_soc_gbox2020_device::device_start()
{
	vt4ffx_soc_base_device::device_start();
	downcast<rp2a03_core_swap_op_d5_d6 &>(*m_maincpu).set_which_crypt(3);
	m_encryption_allowed = true;
}

void vt4ffx_soc_s10swap_device::device_start()
{
	vt4ffx_soc_base_device::device_start();
	downcast<rp2a03_core_swap_op_d5_d6 &>(*m_maincpu).set_which_crypt(4);
	m_encryption_allowed = true;
}

void vt4ffx_soc_rsps300swap_device::device_start()
{
	vt4ffx_soc_base_device::device_start();
	downcast<rp2a03_core_swap_op_d5_d6 &>(*m_maincpu).set_which_crypt(5);
	m_encryption_allowed = true;
}

/////////

void vt4ffx_soc_vibesswap_device::vibes_411c_w(u8 data)
{
	if (m_encryption_allowed)
	{
		if (data == 0x05)
			downcast<rp2a03_core_swap_op_d5_d6&>(*m_maincpu).set_encryption_state(false);
		else if (data == 0x07 || data == 0x87) // why 0x07 to enable in some cases here?
			downcast<rp2a03_core_swap_op_d5_d6&>(*m_maincpu).set_encryption_state(true);
		else
			logerror("%s: vibes_411c_w %02x (unknown)\n", machine().describe_context(), data);
	}
}

void vt4ffx_soc_vibesswap_device::nes_vt_vibes_map(address_map &map)
{
	vt4ffx_soc_base_device::vt369_map(map);
	map(0x411c, 0x411c).w(FUNC(vt4ffx_soc_vibesswap_device::vibes_411c_w));
}

/////////

void vt4ffx_soc_gbox2020_device::gbox_411c_w(u8 data)
{
	if (m_encryption_allowed)
	{
		if (data == 0x05)
			downcast<rp2a03_core_swap_op_d5_d6&>(*m_maincpu).set_encryption_state(false);
		else if (data == 0xc5)
			downcast<rp2a03_core_swap_op_d5_d6&>(*m_maincpu).set_encryption_state(true);
		else
			logerror("%s: gbox_411c_w %02x (unknown)\n", machine().describe_context(), data);
	}
}

void vt4ffx_soc_gbox2020_device::nes_vt_gbox_map(address_map &map)
{
	vt4ffx_soc_base_device::vt369_map(map);
	map(0x411c, 0x411c).w(FUNC(vt4ffx_soc_gbox2020_device::gbox_411c_w));
}

void vt4ffx_soc_gbox2020_device::device_add_mconfig(machine_config &config)
{
	vt4ffx_soc_noswap_device::device_add_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &vt4ffx_soc_gbox2020_device::nes_vt_gbox_map);
}

/////////

void vt4ffx_soc_rsps300swap_device::rsps_411c_w(u8 data)
{
	if (m_encryption_allowed)
	{
		if (data == 0x05)
			downcast<rp2a03_core_swap_op_d5_d6&>(*m_maincpu).set_encryption_state(false);
		else
			logerror("%s: gbox_411c_w %02x (unknown)\n", machine().describe_context(), data);
	}
}

void vt4ffx_soc_rsps300swap_device::nes_vt_rsps_map(address_map &map)
{
	vt4ffx_soc_base_device::vt369_map(map);
	map(0x411c, 0x411c).w(FUNC(vt4ffx_soc_rsps300swap_device::rsps_411c_w));
}

void vt4ffx_soc_rsps300swap_device::device_add_mconfig(machine_config &config)
{
	vt4ffx_soc_noswap_device::device_add_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &vt4ffx_soc_rsps300swap_device::nes_vt_rsps_map);
}
