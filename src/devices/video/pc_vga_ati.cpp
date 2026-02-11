// license:BSD-3-Clause
// copyright-holders:Barry Rodewald

#include "emu.h"
#include "pc_vga_ati.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)           LOGMASKED(LOG_WARN, __VA_ARGS__)

DEFINE_DEVICE_TYPE(ATI_VGA,    ati_vga_device,    "ati_vga",    "ATi VGA i/f")

ati_vga_device::ati_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ati_vga_device(mconfig, ATI_VGA, tag, owner, clock)
{
}

ati_vga_device::ati_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, type, tag, owner, clock)
	, m_eeprom_data_in(*this, 0)
	, m_eeprom_data_out(*this)
	, m_eeprom_clock_out(*this)
	, m_eeprom_chip_select_out(*this)
{
}

void ati_vga_device::device_start()
{
	svga_device::device_start();
	memset(&ati, 0, sizeof(ati));
	save_pointer(ati.ext_reg,"ATi Extended Registers",64);
	m_8514 = subdevice<mach8_device>("8514a");
	ati.vga_chip_id = 0x06;  // 28800-6
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void ati_vga_device::device_add_mconfig(machine_config &config)
{
	MACH8(config, "8514a", 0).set_vga_owner();
}

// TODO: fails VBETEST (after UNIVBE load)
uint16_t ati_vga_device::offset()
{
	//popmessage("Offset: %04x  %s %s %s %s",vga.crtc.offset,vga.crtc.dw?"DW":"--",vga.crtc.word_mode?"BYTE":"WORD",(ati.ext_reg[0x33] & 0x40) ? "PEL" : "---",(ati.ext_reg[0x30] & 0x20) ? "256" : "---");
	if(ati.ext_reg[0x30] & 0x20)  // likely wrong, gets 640x400/480 SVGA and tweaked 256 colour modes displaying correctly in Fractint.
		return vga_device::offset() << 3;
	if(ati.ext_reg[0x33] & 0x40)
		return vga_device::offset() << 2;
	return vga_device::offset();
}


void ati_vga_device::set_dot_clock()
{
	int clock;
	uint8_t clock_type;
	int div = ((ati.ext_reg[0x38] & 0xc0) >> 6) + 1;
	int divisor = 1;

	clock_type = ((ati.ext_reg[0x3e] & 0x10)>>1) | ((ati.ext_reg[0x39] & 0x02)<<1) | ((vga.miscellaneous_output & 0x0c)>>2);
	switch(clock_type)
	{
	case 0:
		clock = XTAL(42'954'545).value();
		break;
	case 1:
		clock = 48771000;
		break;
	case 2:
		clock = 16657000;
		break;
	case 3:
		clock = XTAL(36'000'000).value();
		break;
	case 4:
		clock = 50350000;
		break;
	case 5:
		clock = 56640000;
		break;
	case 6:
		clock = 28322000;
		break;
	case 7:
		clock = 44900000;
		break;
	case 8:
		clock = 30240000;
		break;
	case 9:
		clock = XTAL(32'000'000).value();
		break;
	case 10:
		clock = 37500000;
		break;
	case 11:
		clock = 39000000;
		break;
	case 12:
		clock = XTAL(40'000'000).value();
		break;
	case 13:
		clock = 56644000;
		break;
	case 14:
		clock = 75000000;
		break;
	case 15:
		clock = 65000000;
		break;
	default:
		clock = XTAL(42'954'545).value();
		LOGWARN( "Invalid dot clock %i selected.\n",clock_type);
		break;
	}
//  LOG("ATI: Clock select type %i (%iHz / %i)\n",clock_type,clock,div);
	recompute_params_clock(divisor,clock / div);

}

void ati_vga_device::ati_define_video_mode()
{
	svga.rgb8_en = 0;
	svga.rgb15_en = 0;
	svga.rgb16_en = 0;
	svga.rgb32_en = 0;

	if(ati.ext_reg[0x30] & 0x20)
		svga.rgb8_en = 1;

	set_dot_clock();
}

uint8_t ati_vga_device::mem_r(offs_t offset)
{
	if(svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en)
	{
		if(ati.ext_reg[0x3d] & 0x04)
		{
			offset &= 0x1ffff;
			return vga.memory[(offset+svga.bank_r*0x20000) % vga.svga_intf.vram_size];
		}
		else
		{
			offset &= 0xffff;
			return vga.memory[(offset+svga.bank_r*0x10000) % vga.svga_intf.vram_size];
		}
	}

	return vga_device::mem_r(offset);
}

void ati_vga_device::mem_w(offs_t offset, uint8_t data)
{
	if(svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en)
	{
		if(ati.ext_reg[0x3d] & 0x04)
		{
			offset &= 0x1ffff;
			vga.memory[(offset+svga.bank_w*0x20000) % vga.svga_intf.vram_size] = data;
		}
		else
		{
			offset &= 0xffff;
			vga.memory[(offset+svga.bank_w*0x10000) % vga.svga_intf.vram_size] = data;
		}
	}
	else
		vga_device::mem_w(offset,data);
}


uint8_t ati_vga_device::ati_port_ext_r(offs_t offset)
{
	uint8_t ret = 0xff;

	switch(offset)
	{
	case 0:
		break;
	case 1:
		switch(ati.ext_reg_select)
		{
		case 0x20:
			ret = 0x10;  // 16-bit ROM access
			LOG( "ATI20 read\n");
			break;
		case 0x28:  // Vertical line counter (high)
			ret = (screen().vpos() >> 8) & 0x03;
			LOG( "ATI28 (vertical line high) read\n");
			break;
		case 0x29:  // Vertical line counter (low)
			ret = screen().vpos() & 0xff;  // correct?
			LOG( "ATI29 (vertical line low) read\n");
			break;
		case 0x2a:
			ret = ati.vga_chip_id;  // Chip revision (6 for the 28800-6, 5 for the 28800-5) This register is not listed in ATI's mach32 docs
			LOG( "ATI2A (VGA ID) read\n");
			break;
		// EEPROM interface read
		case 0x37:
			{
				ret = 0;
				ret |= m_eeprom_data_in() << 3;
			}
			break;
		case 0x3d:
			ret = ati.ext_reg[ati.ext_reg_select] & 0x0f;
			ret |= 0x10;  // EGA DIP switch emulation
			LOG( "ATI3D (EGA DIP emulation) read\n");
			break;
		default:
			ret = ati.ext_reg[ati.ext_reg_select];
			LOG( "ATI: Extended VGA register 0x01CE index %02x read\n",ati.ext_reg_select);
			break;
		}
		break;
	}
	return ret;
}

void ati_vga_device::ati_port_ext_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
	case 0:
		ati.ext_reg_select = data & 0x3f;
		break;
	case 1:
		ati.ext_reg[ati.ext_reg_select] = data;
		switch(ati.ext_reg_select)
		{
		case 0x23:
			vga.crtc.start_addr_latch = (vga.crtc.start_addr_latch & ~0x10000) | (BIT(data, 4) << 16);
			vga.crtc.cursor_addr = (vga.crtc.cursor_addr & 0xfffdffff) | ((data & 0x08) << 14);
			ati.ext_reg[ati.ext_reg_select] = data & 0x1f;
			LOG( "ATI: ATI23 write %02x\n",data);
			break;
		case 0x26:
			ati.ext_reg[ati.ext_reg_select] = data & 0xc9;
			LOG( "ATI: ATI26 write %02x\n",data);
			break;
		case 0x2b:
			ati.ext_reg[ati.ext_reg_select] = data & 0xdf;
			LOG( "ATI: ATI2B write %02x\n",data);
			break;
		case 0x2d:
			if(data & 0x08)
			{
				vga.crtc.horz_total = (vga.crtc.horz_total & 0x00ff) | (data & 0x01) << 8;
				// bit 1 = bit 8 of horizontal blank start
				// bit 2 = bit 8 of horizontal retrace start
			}
			LOG( "ATI: ATI2D (extensions) write %02x\n",data);
			break;
		case 0x2e:
			ati.ext_reg[ati.ext_reg_select] = data;
			// N/A for this core, matters for MACH32
			// TODO: lift me on actual ext_space handling
			refresh_bank();
			break;
		case 0x30:
			vga.crtc.start_addr_latch = (vga.crtc.start_addr_latch & ~0x20000) | (BIT(data, 6) << 17);
			vga.crtc.cursor_addr = (vga.crtc.cursor_addr & 0xfffeffff) | ((data & 0x04) << 14);
			ati.ext_reg[ati.ext_reg_select] = data & 0x7d;
			LOG( "ATI: ATI30 write %02x\n",data);
			break;
		case 0x31:
			ati.ext_reg[ati.ext_reg_select] = data & 0x7f;
			LOG( "ATI: ATI31 write %02x\n",data);
			break;
		case 0x32:  // memory page select
			refresh_bank();
			//LOG( "ATI: Memory Page Select write %02x (read: %i write %i)\n",data,svga.bank_r,svga.bank_w);
			break;
		case 0x33:  // EEPROM
			// bit 4: <reserved>
			// bit 5: ISA bus 8/16-bit memory operation (depends on RMCE1B config pin)
			// bit 6: 4-bit PEL (mode 55h)
			// bit 7: Double Scan Enable
			ati.ext_reg[ati.ext_reg_select] = data & 0xef;
			// EEPROM Interface Enable
			// TODO: does it also pull reading high?
			if(BIT(data, 2))
			{
				// CS
				m_eeprom_chip_select_out(BIT(data, 3));
				// CLK
				m_eeprom_clock_out(BIT(data, 1));
				// DI
				m_eeprom_data_out(BIT(data, 0));
			}
			else
			{
				LOG( "ATI: ATI33 write %02x\n",data);
			}
			break;
		case 0x38:
			ati.ext_reg[ati.ext_reg_select] = data & 0xef;
			LOG( "ATI: ATI38 write %02x\n",data);
			break;
		case 0x39:
			ati.ext_reg[ati.ext_reg_select] = data & 0xfe;
			LOG( "ATI: ATI39 write %02x\n",data);
			break;
		case 0x3a:  // General purpose read-write bits
			ati.ext_reg[ati.ext_reg_select] = data & 0x07;
			LOG( "ATI: ATI3A write %02x\n",data);
			break;
		case 0x3c:  // Reserved, should be 0
			ati.ext_reg[ati.ext_reg_select] = 0;
			LOG( "ATI: ATI3C write %02x\n",data);
			break;
		case 0x3d:
			ati.ext_reg[ati.ext_reg_select] = data & 0xfd;
			LOG( "ATI: ATI3D write %02x\n",data);
			break;
		case 0x3e:
			ati.ext_reg[ati.ext_reg_select] = data & 0x1f;
			refresh_bank();
			LOG( "ATI: ATI3E write %02x\n",data);
			break;
		case 0x3f:
			ati.ext_reg[ati.ext_reg_select] = data & 0x0f;
			LOG( "ATI: ATI3F write %02x\n",data);
			break;
		default:
			LOG( "ATI: Extended VGA register 0x01CE index %02x write %02x\n",ati.ext_reg_select,data);
			break;
		}
		break;
	}
	ati_define_video_mode();
}

void ati_vga_device::refresh_bank()
{
	u8 bank = ati.ext_reg[0x32];
	if(ati.ext_reg[0x3e] & 0x08)
	{
		svga.bank_r = ((bank & 0x01) << 3) | ((bank & 0xe0) >> 5);
		svga.bank_w = ((bank & 0x1e) >> 1);
	}
	else
	{
		svga.bank_r = ((bank & 0x1e) >> 1);
		svga.bank_w = svga.bank_r;
	}
}
