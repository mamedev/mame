// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Magic the Gathering: Armageddon

    preliminary driver by Phil Bennett


PCB Information (needs tidying:)


TOP Board
.20u    27c4001 stickered   U20
                #1537 V1.0  a 1 was handwritten over the 0

.u7     stamped     (c) 1997
                ACCLAIM COINOP
                ARMAGEDDON
                SND3 P/N 1605
                9806 D


.u8     stamped     (c) 1997 11/25/97
                ACCLAIM COINOP
                ARMAGEDDON
                1534 SND0
                9752 D

.u13        stamped     (c) 1997 11/25/97
                ACCLAIM COINOP
                ARMAGEDDON
                1536 SND2
                9752 D

.u14        stamped     (c) 1997 11/25/97
                ACCLAIM COINOP
                ARMAGEDDON
                1535 SND1
                9752 D

Analog devices  ADSP 2181
Xilinx      XC5202
dt71256 x2
Analog Devices  AD1866R

Bottom board
.u32    27c801      stickered   4
.u33    27c801      stickered   3
.u34    27c801      stickered   2
.u35    27c801      stickered   1
.u58    AT17C128    stickered   CC3E
.u66    GAL16V8D

Xilinx  XC4005E
Xilinx  XC95108 stickered   ACCLAIM COIN-OP
                CSC2_10.JED
                B710/0B84
                U40 p/N 1611

3dFX    500-0003-03     x2
    BF1684.1
TI  TVP3409
    V53C16258HK40       x24
    V53C511816500K60    x4

U38 and U97 on main board   3DFX
                            500-0004-02
                            BF2733.1 TMU
                            9748 20001
                            TAIWAN 1001

U4 on daughter board        Zoran ZR36050PQC
                            -29.5
                            85 GF7B9726E

U11 on main board           Removed heatsink, Couldn't see anything...
                            On a second PCB: IDT 79RV5000-200BS272 YA9802C

U71 on main board           Galileo
                            GT-64010A-B-0
                            BB8018.1
                            TAIWAN
14.31818 Oscillator by the TI part
50.0000 Oscillator by EPROMS
33.0000 Oscillator by the V53C511816500K60
KM622560LG-7 by Battery

Bottom daughter board
All read as 29F800B
.u9     stamped     (c) 1997
                ACCLAIM COINOP
                ARMAGEDDON S0
                1514 11/25/97
                9803 D


.u10        stamped     (c) 1997
                ACCLAIM COINOP
                ARMAGEDDON S1
                1515 11/25/97
                9803 D

.u11        stamped     (c) 1997
                ACCLAIM COINOP
                ARMAGEDDON S3
                1517 11/25/97
                9803 D

.u12        stamped     (c) 1997
                ACCLAIM COINOP
                ARMAGEDDON S2
                1516 11/25/97
                9803 D

.u20        stamped     (c) 1997
                ACCLAIM COINOP
                ARMAGEDDON K0
                1543 11/25/97
                9752 D

Xilinx  XC4010E
Zoran   ZR36120PQC
Zoran   ZR36016PQC
Xilinx  XC3120A
    DT72811
    DT71256 x2
    DT72271
29.500000 oscillator by ZR36120PQC
Medium size chip with heat sink on it

***************************************************************************/

#include "emu.h"
#include "cpu/adsp2100/adsp2100.h"
#include "cpu/mips/mips3.h"
#include "machine/lpci.h"
#include "sound/dmadac.h"
#include "video/voodoo.h"
#include "screen.h"
#include "speaker.h"


namespace {

/* TODO: Two 3Dfx Voodoo chipsets are used in SLI configuration */
// #define USE_TWO_3DFX

class magictg_state : public driver_device
{
public:
	magictg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_mips(*this, "mips")
		, m_adsp(*this, "adsp")
		, m_pci(*this, "pcibus")
		, m_adsp_pram(*this, "adsp_pram")
		, m_voodoo(*this, "voodoo_%u", 0U)
	{ }

	void magictg(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<mips3_device>       m_mips;
	required_device<adsp2181_device>    m_adsp;
	required_device<pci_bus_legacy_device>      m_pci;


	/* ASIC */
	struct
	{
		uint32_t src_addr = 0;
		uint32_t dst_addr = 0;
		uint32_t ctrl = 0;
		uint32_t count = 0;
	} m_dma_ch[3];


	/* ADSP-2181 */
	required_shared_ptr<uint32_t> m_adsp_pram;

	struct
	{
		uint16_t bdma_internal_addr = 0;
		uint16_t bdma_external_addr = 0;
		uint16_t bdma_control = 0;
		uint16_t bdma_word_count = 0;
	} m_adsp_regs;


	/* 3Dfx Voodoo */
	required_device_array<generic_voodoo_device, 2> m_voodoo;

	struct
	{
		/* PCI */
		uint32_t command = 0;
		uint32_t base_addr = 0;

		uint32_t init_enable = 0;
	} m_voodoo_pci_regs[2];


	struct
	{
		/* PCI */
		uint32_t command = 0;
		uint32_t base_addr = 0;

		/* Memory-mapped */
		uint32_t as_regs[0x200]{}; // was 19, increased to 0x200 for coverity 315123, needed for zr36120_r/w, to stop crash at start.
	} m_zr36120;


	uint32_t zr36120_r(offs_t offset);
	void zr36120_w(offs_t offset, uint32_t data);

	uint32_t f0_r(offs_t offset, uint32_t mem_mask = ~0);
	void f0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t unk_r();
	uint32_t unk2_r();

	void serial_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t adsp_idma_data_r(offs_t offset, uint32_t mem_mask = ~0);
	void adsp_idma_data_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void adsp_idma_addr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t adsp_status_r();
	uint16_t adsp_control_r(offs_t offset);
	void adsp_control_w(offs_t offset, uint16_t data);

	void zr36120_reset();

	void adsp_data_map(address_map &map) ATTR_COLD;
	void adsp_io_map(address_map &map) ATTR_COLD;
	void adsp_program_map(address_map &map) ATTR_COLD;
	void magictg_map(address_map &map) ATTR_COLD;

	uint32_t pci_dev0_r(int function, int reg, uint32_t mem_mask);
	void pci_dev0_w(int function, int reg, uint32_t data, uint32_t mem_mask);
	uint32_t voodoo_0_pci_r(int function, int reg, uint32_t mem_mask);
	void voodoo_0_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);
#if defined(USE_TWO_3DFX)
	uint32_t voodoo_1_pci_r(int function, int reg, uint32_t mem_mask);
	void voodoo_1_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);
#endif
	uint32_t zr36120_pci_r(int function, int reg, uint32_t mem_mask);
	void zr36120_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);

	uint32_t screen_update_magictg(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


/*************************************
 *
 *  Machine initialization
 *
 *************************************/

void magictg_state::machine_start()
{
}


void magictg_state::machine_reset()
{
	uint8_t *adsp_boot = (uint8_t*)memregion("adsp")->base();

	zr36120_reset();

	/* Load 32 program words (96 bytes) via BDMA */
	for (int i = 0; i < 32; i ++)
	{
		uint32_t word;

		word = adsp_boot[i*3 + 0] << 16;
		word |= adsp_boot[i*3 + 1] << 8;
		word |= adsp_boot[i*3 + 2];

		m_adsp_pram[i] = word;
	}
}


/*************************************
 *
 *  Video
 *
 *************************************/

void magictg_state::video_start()
{
}

uint32_t magictg_state::screen_update_magictg(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return m_voodoo[0]->update(bitmap, cliprect) ? 0 : UPDATE_HAS_NOT_CHANGED;
}


/*************************************
 *
 *  3Dfx Voodoo
 *
 *************************************/

uint32_t magictg_state::pci_dev0_r(int function, int reg, uint32_t mem_mask)
{
	logerror("PCI[0] READ: %x\n", reg);
	return 0x00000000; // TODO
}

void magictg_state::pci_dev0_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
}


uint32_t magictg_state::voodoo_0_pci_r(int function, int reg, uint32_t mem_mask)
{
	uint32_t val = 0;

	switch (reg)
	{
		case 0:
			val = 0x0001121a;
			break;
		case 0x10:
			val = m_voodoo_pci_regs[0].base_addr;
			break;
		case 0x40:
			val = m_voodoo_pci_regs[0].init_enable;
			break;
		default:
			logerror("Voodoo[0] PCI R: %x\n", reg);
	}
	return val;
}

void magictg_state::voodoo_0_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
	switch (reg)
	{
		case 0x04:
			m_voodoo_pci_regs[0].command = data & 0x3;
			break;
		case 0x10:
			if (data == 0xffffffff)
				m_voodoo_pci_regs[0].base_addr = 0xff000000;
			else
				m_voodoo_pci_regs[0].base_addr = data;
			break;
		case 0x40:
			m_voodoo_pci_regs[0].init_enable = data;
			m_voodoo[0]->set_init_enable(data);
			break;

		default:
			logerror("Voodoo [%x]: %x\n", reg, data);
	}
}

#if defined(USE_TWO_3DFX)
uint32_t magictg_state::voodoo_1_pci_r(int function, int reg, uint32_t mem_mask)
{
	uint32_t val = 0;

	switch (reg)
	{
		case 0:
			val = 0x0001121a;
			break;
		case 0x10:
			val = m_voodoo_pci_regs[1].base_addr;
			break;
		case 0x40:
			val = m_voodoo_pci_regs[1].init_enable;
			break;
		default:
			logerror("Voodoo[1] PCI R: %x\n", reg);
	}
	return val;
}

void magictg_state::voodoo_1_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
	switch (reg)
	{
		case 0x04:
			voodoo_pci_regs[1].command = data & 0x3;
			break;
		case 0x10:
			if (data == 0xffffffff)
				m_voodoo_pci_regs[1].base_addr = 0xff000000;
			else
				m_voodoo_pci_regs[1].base_addr = data;
			break;
		case 0x40:
			m_voodoo_pci_regs[1].init_enable = data;
			set_init_enable(state->m_voodoo[1], data);
			break;

		default:
			logerror("Voodoo [%x]: %x\n", reg, data);
	}
}
#endif


/*************************************
 *
 *  PinkEye (JPEG decoder)
 *
 *************************************/

void magictg_state::zr36120_reset()
{
	/* Reset PCI registers */
	m_zr36120.base_addr = 0;

	/* Reset application-specific registers */
	m_zr36120.as_regs[0x00/4] = (1 << 10) | 0x3ff;
	m_zr36120.as_regs[0x04/4] = (1 << 10) | 0x3ff;
	m_zr36120.as_regs[0x08/4] = (1 << 25) | (2 << 3) | 1;
	m_zr36120.as_regs[0x0c/4] = 0xfffffffc;
	m_zr36120.as_regs[0x10/4] = 0xfffffffc;
	m_zr36120.as_regs[0x14/4] = 0x00000000;
	m_zr36120.as_regs[0x18/4] = (7 << 25) | (0xf0 << 12) | 0x3ff;
	m_zr36120.as_regs[0x1c/4] = 0xfffffffc;
	m_zr36120.as_regs[0x20/4] = 0xfffffffc;
	m_zr36120.as_regs[0x24/4] = 0x000000ff;
	m_zr36120.as_regs[0x28/4] = 0x000000ff;
	m_zr36120.as_regs[0x2c/4] = 0xf0000000;
	m_zr36120.as_regs[0x30/4] = 0xfffffffc;
	m_zr36120.as_regs[0x34/4] = (1 << 29) | (1 << 28) | (3 << 12) | (1 << 8) | (6 << 1);
	m_zr36120.as_regs[0x38/4] = 0x00000000;
	m_zr36120.as_regs[0x3c/4] = 0x00000000;
	m_zr36120.as_regs[0x40/4] = 0x00000000;
	m_zr36120.as_regs[0x44/4] = 0x00000003;
	m_zr36120.as_regs[0x48/4] = 1 << 23;
}

uint32_t magictg_state::zr36120_pci_r(int function, int reg, uint32_t mem_mask)
{
	uint32_t val = 0;

	switch (reg)
	{
		case 0x00:
			val = 0x612011de;
			break;
		case 0x04:
			val = m_zr36120.command;
			break;
		case 0x08:
			val = 0x04000002;
			break;
		case 0x10:
			val = m_zr36120.base_addr;
			break;
		default:
			logerror("ZR36120 R[%x]\n", reg);
	}
	return val;
}

void magictg_state::zr36120_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
	switch (reg)
	{
		case 0x04:
			m_zr36120.command = data & 0x6;
			break;
		case 0x10:
			m_zr36120.base_addr = data & 0xfffff000;
			break;
		default:
			logerror("ZR36120 [%x]: %x\n", reg, data);
	}
}

uint32_t magictg_state::zr36120_r(offs_t offset)
{
	uint32_t res = 0;

	offset <<= 2;

	if (offset < 0x200)
	{
		switch (offset)
		{
			default:
				res = m_zr36120.as_regs[offset];
		}
	}
	else
	{
		// PostOffice reads
		res = 0;//mame_rand(machine);//m_zr36120.as_regs[0x48/4];
	}
	logerror("PINKEYE_R[%x]\n", offset);
	return res;
}

void magictg_state::zr36120_w(offs_t offset, uint32_t data)
{
	offset <<= 2;

	if (offset < 0x200)
	{
		logerror("PINKEYE_W[%x] %x\n", offset, data);
		switch (offset)
		{
			case 0x00/4:
				m_zr36120.as_regs[0] = data & 0x400fffff;
				break;
			case 0x04/4:
				m_zr36120.as_regs[1] = data & 0x400003ff;
				break;
			default:
				m_zr36120.as_regs[offset] = data;
		}
	}
	else
	{
		// PostOffice writes
		// - Takes 32 PCI clocks for time out to happen compared to 64 in ZR36067
		// - Has 4 guests instead of 8
		uint32_t guest = (data >> 20) & 3;
		uint32_t g_data = data & 0xff;
		uint32_t g_reg = (data >> 16) & 7;

		/* Direction - 0 for read, 1 for write */
		//  zr36120_guest_write(guest, g_data, g_reg);
		// 2 - ZR36050 JPEG decoder
		// 3 - ZR36016 color-space converter
		logerror("GUEST (%.8x): %d  REG: %d  DATA: %x\n", data, guest, g_reg, g_data);
	}
}


/*************************************
 *
 *  System stuff
 *
 *************************************/

uint32_t magictg_state::unk_r()
{
	/* Will not boot otherwise */
	return 0x6000;
}

uint32_t magictg_state::unk2_r()
{
	return 0xffffffff;
}

void magictg_state::serial_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == 0)
	{
		if (mem_mask == 0xff000000)
			printf("%c", data >> 24);
	}
}

void magictg_state::f0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	int ch;

	offset *= 4;

	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);

	ch = ((offset >> 2) & 3) - 1;

	switch (offset)
	{
		case 0x804:
		case 0x808:
		case 0x80c:
			m_dma_ch[ch].count = data;
//          logerror("DMA%d COUNT: %.8x\n", ch, data);
			break;

		case 0x814:
		case 0x818:
		case 0x81c:
			m_dma_ch[ch].src_addr = data;
//          logerror("DMA%d SRC: %.8x\n", ch, data);
			break;

		case 0x824:
		case 0x828:
		case 0x82c:
			m_dma_ch[ch].dst_addr = data;
//          logerror("DMA%d DST: %.8x\n", ch, data);
			break;

		case 0x844:
		case 0x848:
		case 0x84c:
		{
			m_dma_ch[ch].ctrl = data;
//          logerror("DMA%d CTRL: %.8x\n", ch, data);

			if (data & 0x1000)
			{
				uint32_t src_addr = m_dma_ch[ch].src_addr;
				uint32_t dst_addr = m_dma_ch[ch].dst_addr;
				//device_t *voodoo = dst_addr > 0xa000000 voodoo0 : voodoo1;

				assert((src_addr & 3) == 0);
				assert((dst_addr & 3) == 0);

				while (m_dma_ch[ch].count > 3)
				{
					uint32_t src_dword = swapendian_int32(m_mips->space(AS_PROGRAM).read_dword(src_addr));
					m_mips->space(AS_PROGRAM).write_dword(dst_addr, src_dword);
					src_addr += 4;
					dst_addr += 4;
					m_dma_ch[ch].count -=4;
				}

				// FIXME!
				if (m_dma_ch[ch].count & 3)
				{
					uint32_t src_dword = swapendian_int32(m_mips->space(AS_PROGRAM).read_dword(src_addr));
					uint32_t dst_dword = m_mips->space(AS_PROGRAM).read_dword(dst_addr);
					uint32_t mask = 0xffffffff >> ((m_dma_ch[ch].count & 3) << 3);

					dst_dword = (dst_dword & ~mask) | (src_dword & mask);
					m_mips->space(AS_PROGRAM).write_dword(dst_addr, dst_dword);
					m_dma_ch[ch].count = 0;
				}
			}

			break;
		}
		case 0xcf8:
		{
			m_pci->write(0, data, mem_mask);
			break;
		}
		case 0xcfc:
		{
			m_pci->write(1, data, mem_mask);
			break;
		}
//      default:
//          logerror("W: %.8x: %.8x\n", 0x0f000000 + offset, data);
	}
}

uint32_t magictg_state::f0_r(offs_t offset, uint32_t mem_mask)
{
	int ch;
	uint32_t val = 0;
	offset *= 4;

	ch = ((offset >> 2) & 3) - 1;

	switch (offset)
	{
		case 0x804:
		case 0x808:
		case 0x80c:
			val = m_dma_ch[ch].count;
			break;

		case 0x844:
		case 0x848:
		case 0x84c:
			val = 0x00000040; // Status of some sort
			break;

		case 0xcf8:
		{
			val = m_pci->read(0, swapendian_int32(mem_mask));
			break;
		}
		case 0xcfc:
		{
			val = m_pci->read(1, swapendian_int32(mem_mask));
			break;
		}
//      default:
//          logerror("R: %.8x\n", 0x0f000000 + offset);
	}

	return swapendian_int32(val);
}


/*************************************
 *
 *  ADSP-2181 internals
 *
 *************************************/

void magictg_state::adsp_idma_data_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_16_31)
		m_adsp->idma_addr_w(data >> 16);
	else
		m_adsp->idma_addr_w(data & 0xffff);
}

uint32_t magictg_state::adsp_idma_data_r(offs_t offset, uint32_t mem_mask)
{
	// TODO: Set /IACK appropriately
	if (ACCESSING_BITS_0_15)
	{
		//logerror("RD %.8x %.8x\n", offset, mem_mask);
		return m_adsp->idma_addr_r();
	}
	else
	{
		fatalerror("????\n");
	}
}

void magictg_state::adsp_idma_addr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// TODO: Set /IACK appropriately
	if (ACCESSING_BITS_16_31)
	{
		m_adsp->idma_addr_w(data >> 16);
		//logerror("WR %.8x %.8x %.8x\n", offset, mem_mask, data >> 16);
	}
	else
		fatalerror("????\n");
}

uint32_t magictg_state::adsp_status_r()
{
	// ADSP_IACK = Bit 2
	return (0 << 2) | (machine().rand() & 1);
}

uint16_t magictg_state::adsp_control_r(offs_t offset)
{
	uint16_t res = 0;

	switch (offset)
	{
		case 0x4:
			res = m_adsp_regs.bdma_word_count;
			break;
		case 0x5:
			res = machine().rand() & 0xff;
			break;
		default:
			logerror("Unhandled register: %x\n", 0x3fe0 + offset);
	}
	return res;
}

void magictg_state::adsp_control_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
		case 0x1:
			m_adsp_regs.bdma_internal_addr = data & 0x3fff;
			break;
		case 0x2:
			m_adsp_regs.bdma_external_addr = data & 0x3fff;
			break;
		case 0x3:
			m_adsp_regs.bdma_control = data & 0xff0f;
			break;
		case 0x4:
		{
			m_adsp_regs.bdma_word_count = data & 0x3fff;

			if (data > 0)
			{
				uint8_t* adsp_rom = (uint8_t*)memregion("adsp")->base();

				uint32_t page = (m_adsp_regs.bdma_control >> 8) & 0xff;
				uint32_t dir = (m_adsp_regs.bdma_control >> 2) & 1;
				uint32_t type = m_adsp_regs.bdma_control & 3;

				uint32_t src_addr = (page << 14) | m_adsp_regs.bdma_external_addr;

				address_space &addr_space = m_adsp->space((type == 0) ? AS_PROGRAM : AS_DATA);

				if (dir == 0)
				{
					while (m_adsp_regs.bdma_word_count)
					{
						if (type == 0)
						{
							uint32_t src_word =(adsp_rom[src_addr + 0] << 16) |
												(adsp_rom[src_addr + 1] << 8) |
												(adsp_rom[src_addr + 2]);

							addr_space.write_dword(m_adsp_regs.bdma_internal_addr * 4, src_word);

							src_addr += 3;
							m_adsp_regs.bdma_internal_addr ++;
						}
						else if (type == 1)
						{
							uint32_t src_word =(adsp_rom[src_addr + 0] << 8) | adsp_rom[src_addr + 1];

							addr_space.write_dword(m_adsp_regs.bdma_internal_addr * 2, src_word);

							src_addr += 2;
							m_adsp_regs.bdma_internal_addr ++;
						}
						else
						{
							fatalerror("Unsupported BDMA width\n");
						}

						--m_adsp_regs.bdma_word_count;
					}
				}

				/* Update external address count and page */
				m_adsp_regs.bdma_external_addr = src_addr & 0x3fff;
				m_adsp_regs.bdma_control &= ~0xff00;
				m_adsp_regs.bdma_control |= ((src_addr >> 14) & 0xff) << 8;

				if (m_adsp_regs.bdma_control & 8)
					m_adsp->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
			}
			break;
		}
		case 5:
			logerror("PFLAGS: %x\n", data);
			break;
		default:
			logerror("Unhandled register: %x %x\n", 0x3fe0 + offset, data);
	}
}


/*************************************
 *
 *  Main CPU
 *
 *************************************/

void magictg_state::magictg_map(address_map &map)
{
	map(0x00000000, 0x007fffff).ram(); // 8MB RAM
	map(0x00800000, 0x0081003f).ram(); // ?
	map(0x0a000000, 0x0affffff).rw("voodoo_0", FUNC(generic_voodoo_device::read), FUNC(generic_voodoo_device::write));
#if defined(USE_TWO_3DFX)
	map(0x0b000000, 0x0bffffff).rw("voodoo_1", FUNC(voodoo_device_base::read), FUNC(voodoo_device_base::write));
	map(0x0c000000, 0x0c000fff).rw(FUNC(magictg_state::zr36120_r), FUNC(magictg_state::zr36120_w));
#else
	map(0x0b000000, 0x0b000fff).rw(FUNC(magictg_state::zr36120_r), FUNC(magictg_state::zr36120_w));
#endif
	map(0x0f000000, 0x0f000fff).rw(FUNC(magictg_state::f0_r), FUNC(magictg_state::f0_w)); // Split this up?
	map(0x14000100, 0x14000103).rw(FUNC(magictg_state::adsp_idma_data_r), FUNC(magictg_state::adsp_idma_data_w));
	map(0x14000104, 0x14000107).w(FUNC(magictg_state::adsp_idma_addr_w));
	map(0x1b001024, 0x1b001027).r(FUNC(magictg_state::adsp_status_r));
	map(0x1b001108, 0x1b00110b).r(FUNC(magictg_state::unk_r));
	map(0x1e000000, 0x1e002fff).ram(); // NVRAM?
	map(0x1e800000, 0x1e800007).rw(FUNC(magictg_state::unk2_r), FUNC(magictg_state::serial_w));
	map(0x1fc00000, 0x1fffffff).rom().region("mips", 0);
}


/*************************************
 *
 *  Mad Cow (IO/sound)
 *
 *************************************/

void magictg_state::adsp_program_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).ram().share("adsp_pram");
}

void magictg_state::adsp_data_map(address_map &map)
{
	map.unmap_value_high();
//  map(0x0000, 0x03ff).bankrw("databank");
	map(0x0400, 0x3fdf).ram();
	map(0x3fe0, 0x3fff).rw(FUNC(magictg_state::adsp_control_r), FUNC(magictg_state::adsp_control_w));
}

void magictg_state::adsp_io_map(address_map &map)
{
	map.unmap_value_high();
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( magictg )
	PORT_START("IPT_TEST")
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void magictg_state::magictg(machine_config &config)
{
	R5000BE(config, m_mips, 200000000); // exact model 79RV5000-200BS272 rated for 200MHz, clock not measured
	//m_mips->set_icache_size(16384); /* TODO: Unknown */
	//m_mips->set_dcache_size(16384); /* TODO: Unknown */
	m_mips->set_addrmap(AS_PROGRAM, &magictg_state::magictg_map);

	ADSP2181(config, m_adsp, 16000000);
	m_adsp->set_addrmap(AS_PROGRAM, &magictg_state::adsp_program_map);
	m_adsp->set_addrmap(AS_DATA, &magictg_state::adsp_data_map);
	m_adsp->set_addrmap(AS_IO, &magictg_state::adsp_io_map);

	SPEAKER(config, "speaker", 2).front();

	DMADAC(config, "dac1").add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
	DMADAC(config, "dac2").add_route(ALL_OUTPUTS, "speaker", 1.0, 0);

	pci_bus_legacy_device &pcibus(PCI_BUS_LEGACY(config, "pcibus", 0, 0));
	pcibus.set_device(0, FUNC(magictg_state::pci_dev0_r), FUNC(magictg_state::pci_dev0_w));
	pcibus.set_device(7, FUNC(magictg_state::voodoo_0_pci_r), FUNC(magictg_state::voodoo_0_pci_w));

#if defined(USE_TWO_3DFX)
	pcibus.set_device(8, FUNC(magictg_state::voodoo_1_pci_r), FUNC(magictg_state::voodoo_1_pci_w));
#endif
	pcibus.set_device(9, FUNC(magictg_state::zr36120_pci_r), FUNC(magictg_state::zr36120_pci_w)); // TODO: ZR36120 device

	VOODOO_1(config, m_voodoo[0], voodoo_1_device::NOMINAL_CLOCK);
	m_voodoo[0]->set_fbmem(2);
	m_voodoo[0]->set_tmumem(4,0);
	m_voodoo[0]->set_status_cycles(1000); // optimization to consume extra cycles when polling status
	m_voodoo[0]->set_screen("screen");
	m_voodoo[0]->set_cpu(m_mips);

	VOODOO_1(config, m_voodoo[1], voodoo_1_device::NOMINAL_CLOCK);
	m_voodoo[1]->set_fbmem(2);
	m_voodoo[1]->set_tmumem(4,0);
	m_voodoo[1]->set_status_cycles(1000); // optimization to consume extra cycles when polling status
	m_voodoo[1]->set_screen("screen");
	m_voodoo[1]->set_cpu(m_mips);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(1024, 1024);
	screen.set_visarea(0, 511, 16, 447);
	screen.set_screen_update(FUNC(magictg_state::screen_update_magictg));
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( magictg )
	ROM_REGION64_BE( 0x400000, "mips", 0 )
	ROM_LOAD16_BYTE( "magic.u34", 0x000000, 0x100000, CRC(2e8971e2) SHA1(9bdf433a7c7257389ebdf131317ef26a7d4e1ba2) )
	ROM_LOAD16_BYTE( "magic.u35", 0x000001, 0x100000, CRC(e2202143) SHA1(f07b7da81508cd4594f66e34dabd904a21eb03f0) )
	ROM_LOAD16_BYTE( "magic.u32", 0x200000, 0x100000, CRC(f1d530e3) SHA1(fcc392804cd6b98917a869cc5d3826278b7ba90b) )
	ROM_LOAD16_BYTE( "magic.u33", 0x200001, 0x100000, CRC(b2330cfc) SHA1(559c35426588b349ef31bf8b296b950912f6fcc7) )

	ROM_REGION16_LE( 0x80000, "adsp", 0 )
	ROM_LOAD( "magic.20u", 0x00000, 0x80000, CRC(50968301) SHA1(e9bdd0c942f0c66e18aa8de5a04edb51cdf1fee8) )

	ROM_REGION32_BE( 0x1000000, "adsp_data", 0 )
	ROM_LOAD( "magic.snd0.u8", 0x000000, 0x400000, CRC(3cb81717) SHA1(9d35796381ca57e9782e0338c456e63c31d11266) )
	ROM_LOAD( "magic.snd1.u14",0x400000, 0x400000, CRC(b4ef9977) SHA1(dedc79e5d506bb0d1649a41b9912dcc999e1da72) )
	ROM_LOAD( "magic.snd2.u13",0x800000, 0x400000, CRC(3728f16e) SHA1(6b7da30b100d053e95aa96edf74a0474f1493dfb) )
	ROM_LOAD( "magic.snd3.u7", 0xc00000, 0x400000, CRC(11a1cb63) SHA1(a1048d3cd580747c20eb0b4e816e7e4e0f5c8c2b) )

	ROM_REGION( 0x2000000, "jpeg", 0 )
	ROM_LOAD( "magic_s0.u9",  0x0000000, 0x800000, CRC(a01b5b99) SHA1(e77f2e9b08a97d6118e1e307b38ea79d0177e9b8) )
	ROM_LOAD( "magic_s1.u10", 0x0800000, 0x800000, CRC(d5a1a557) SHA1(2511ee8d08da765a2fa2d42fb504793f9e8b615c) )
	ROM_LOAD( "magic_s2.u12", 0x1000000, 0x800000, CRC(06ed6770) SHA1(884a3e4c97a50fa926546eb6def2c11c5732ba88) )
	ROM_LOAD( "magic_s3.u11", 0x1800000, 0x800000, CRC(71d4c252) SHA1(aeab2542b9d5fb63f4d60b808010a657a895c1d7) )

	ROM_REGION( 0x400000, "key", 0 )
	ROM_LOAD( "magic.k0.u20", 0x000000, 0x400000, CRC(63ab0e9e) SHA1(c4f0b009860ee499496ed7fc1f14ef1e221c1085) )
ROM_END

ROM_START( magictga )
	ROM_REGION64_BE( 0x400000, "mips", 0 )
	ROM_LOAD16_BYTE( "magic.u63", 0x000000, 0x100000, CRC(a10d45f1) SHA1(0ede10f19cf70baf7b43e3f672532b4be1a179f8) )
	ROM_LOAD16_BYTE( "magic.u64", 0x000001, 0x100000, CRC(8fdb6060) SHA1(b638244cad86dc60435a4a9150a5b639f5d61a3f) )
	ROM_LOAD16_BYTE( "magic.u61", 0x200000, 0x100000, CRC(968891d6) SHA1(67ab87039864bb148d20795333ffa7a23e3b84f2) )
	ROM_LOAD16_BYTE( "magic.u62", 0x200001, 0x100000, CRC(690946eb) SHA1(6c9b02367704309f4fde5cbd9d195a45c32c3861) )

	// this set was incomplete, none of these roms were dumped for it, are they the same?
	ROM_REGION32_BE( 0x80000, "adsp", 0 )
	ROM_LOAD( "magic.20u", 0x00000, 0x80000, BAD_DUMP CRC(50968301) SHA1(e9bdd0c942f0c66e18aa8de5a04edb51cdf1fee8) )

	ROM_REGION32_BE( 0x1000000, "adsp_data", 0 )
	ROM_LOAD( "magic.snd0.u8", 0x000000, 0x400000, BAD_DUMP CRC(3cb81717) SHA1(9d35796381ca57e9782e0338c456e63c31d11266) )
	ROM_LOAD( "magic.snd1.u14",0x400000, 0x400000, BAD_DUMP CRC(b4ef9977) SHA1(dedc79e5d506bb0d1649a41b9912dcc999e1da72) )
	ROM_LOAD( "magic.snd2.u13",0x800000, 0x400000, BAD_DUMP CRC(3728f16e) SHA1(6b7da30b100d053e95aa96edf74a0474f1493dfb) )
	ROM_LOAD( "magic.snd3.u7", 0xc00000, 0x400000, BAD_DUMP CRC(11a1cb63) SHA1(a1048d3cd580747c20eb0b4e816e7e4e0f5c8c2b) )

	ROM_REGION( 0x2000000, "jpeg", 0 )
	ROM_LOAD( "magic_s0.u9",  0x0000000, 0x800000, BAD_DUMP CRC(a01b5b99) SHA1(e77f2e9b08a97d6118e1e307b38ea79d0177e9b8) )
	ROM_LOAD( "magic_s1.u10", 0x0800000, 0x800000, BAD_DUMP CRC(d5a1a557) SHA1(2511ee8d08da765a2fa2d42fb504793f9e8b615c) )
	ROM_LOAD( "magic_s2.u12", 0x1000000, 0x800000, BAD_DUMP CRC(06ed6770) SHA1(884a3e4c97a50fa926546eb6def2c11c5732ba88) )
	ROM_LOAD( "magic_s3.u11", 0x1800000, 0x800000, BAD_DUMP CRC(71d4c252) SHA1(aeab2542b9d5fb63f4d60b808010a657a895c1d7) )

	ROM_REGION( 0x400000, "key", 0 )
	ROM_LOAD( "magic.k0.u20", 0x000000, 0x400000, BAD_DUMP CRC(63ab0e9e) SHA1(c4f0b009860ee499496ed7fc1f14ef1e221c1085) )
ROM_END

} // Anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1997, magictg,  0,       magictg, magictg, magictg_state, empty_init, ROT0, "Acclaim", "Magic the Gathering: Armageddon (set 1)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1997, magictga, magictg, magictg, magictg, magictg_state, empty_init, ROT0, "Acclaim", "Magic the Gathering: Armageddon (set 2)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
