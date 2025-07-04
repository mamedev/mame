// license:GPL-2.0+
// copyright-holders:smf, pSXAuthor, R. Belmont
/***************************************************************************

  Sony PlayStation
  ================
  Preliminary driver by smf
  Additional development by pSXAuthor and R. Belmont

***************************************************************************/

#include "emu.h"

#include "bus/psx/ctlrport.h"
#include "bus/psx/parallel.h"
#include "cpu/m6805/hd6305.h"
#include "cpu/psx/psx.h"
#include "imagedev/cdromimg.h"
#include "imagedev/snapquik.h"
#include "psxcd.h"
#include "machine/ram.h"
#include "sound/spu.h"
#include "video/psx.h"

#include "debugger.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#include "multibyte.h"

#include <zlib.h>


namespace {

class psx1_state : public driver_device
{
public:
	psx1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, "maincpu:ram"),
		m_parallel(*this, "parallel"),
		m_psxcd(*this, "psxcd"),
		m_cd_softlist(*this, "cd_list")
	{
	}

	void psx_base(machine_config &config);
	void pse(machine_config &config);
	void psu(machine_config &config);
	void psj(machine_config &config);

private:
	std::vector<uint8_t> m_exe_buffer;
	void psxexe_conv32(uint32_t *uint32);
	int load_psxexe(std::vector<uint8_t> buffer);
	void cpe_set_register(int r, int v);
	int load_cpe(std::vector<uint8_t> buffer);
	int load_psf(std::vector<uint8_t> buffer);
	uint16_t parallel_r(offs_t offset);
	void parallel_w(offs_t offset, uint16_t data);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_exe);
	void cd_dma_read( uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size );
	void cd_dma_write( uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size );
	required_device<psxcpu_device> m_maincpu;
	required_device<ram_device> m_ram;

	void psx_map(address_map &map) ATTR_COLD;
	void subcpu_map(address_map &map) ATTR_COLD;

	required_device<psx_parallel_slot_device> m_parallel;
	required_device<psxcd_device> m_psxcd;
	required_device<software_list_device> m_cd_softlist;
};


void psx1_state::psxexe_conv32(uint32_t *uint32)
{
	uint8_t *uint8 = (uint8_t *)uint32;

	*(uint32) = uint8[0] | (uint8[1] << 8) | (uint8[2] << 16) | (uint8[3] << 24);
}

int psx1_state::load_psxexe(std::vector<uint8_t> buffer)
{
	struct PSXEXE_HEADER
	{
		uint8_t id[8];
		uint32_t text;    /* SCE only */
		uint32_t data;    /* SCE only */
		uint32_t pc0;
		uint32_t gp0;     /* SCE only */
		uint32_t t_addr;
		uint32_t t_size;
		uint32_t d_addr;  /* SCE only */
		uint32_t d_size;  /* SCE only */
		uint32_t b_addr;  /* SCE only */
		uint32_t b_size;  /* SCE only */
		uint32_t s_addr;
		uint32_t s_size;
		uint32_t SavedSP;
		uint32_t SavedFP;
		uint32_t SavedGP;
		uint32_t SavedRA;
		uint32_t SavedS0;
		uint8_t dummy[0x800 - 76];
	};

	struct PSXEXE_HEADER *psxexe_header = reinterpret_cast<struct PSXEXE_HEADER *>(&buffer[0]);

	if (buffer.size() >= sizeof(struct PSXEXE_HEADER) &&
		memcmp(psxexe_header->id, "PS-X EXE", 8) == 0)
	{
		psxexe_conv32(&psxexe_header->text);
		psxexe_conv32(&psxexe_header->data);
		psxexe_conv32(&psxexe_header->pc0);
		psxexe_conv32(&psxexe_header->gp0);
		psxexe_conv32(&psxexe_header->t_addr);
		psxexe_conv32(&psxexe_header->t_size);
		psxexe_conv32(&psxexe_header->d_addr);
		psxexe_conv32(&psxexe_header->d_size);
		psxexe_conv32(&psxexe_header->b_addr);
		psxexe_conv32(&psxexe_header->b_size);
		psxexe_conv32(&psxexe_header->s_addr);
		psxexe_conv32(&psxexe_header->s_size);
		psxexe_conv32(&psxexe_header->SavedSP);
		psxexe_conv32(&psxexe_header->SavedFP);
		psxexe_conv32(&psxexe_header->SavedGP);
		psxexe_conv32(&psxexe_header->SavedRA);
		psxexe_conv32(&psxexe_header->SavedS0);

		/* todo: check size.. */

		logerror("psx_exe_load: pc    %08x\n", psxexe_header->pc0);
		logerror("psx_exe_load: org   %08x\n", psxexe_header->t_addr);
		logerror("psx_exe_load: len   %08x\n", psxexe_header->t_size);
		logerror("psx_exe_load: sp    %08x\n", psxexe_header->s_addr);
		logerror("psx_exe_load: len   %08x\n", psxexe_header->s_size);

		uint8_t *ram_pointer = m_ram->pointer();
		uint32_t ram_size = m_ram->size();

		uint8_t *text = reinterpret_cast<uint8_t *>(&buffer[sizeof(struct PSXEXE_HEADER)]);

		uint32_t address = psxexe_header->t_addr;
		uint32_t size = psxexe_header->t_size;
		while (size != 0)
		{
			ram_pointer[BYTE4_XOR_LE(address++) % ram_size] = *(text++);
			size--;
		}

		m_maincpu->set_state_int(PSXCPU_PC, psxexe_header->pc0);
		m_maincpu->set_state_int(PSXCPU_R28, psxexe_header->gp0);
		uint32_t stack = psxexe_header->s_addr + psxexe_header->s_size;
		if (stack != 0)
		{
			m_maincpu->set_state_int(PSXCPU_R29, stack);
			m_maincpu->set_state_int(PSXCPU_R30, stack);
		}

		return 1;
	}
	return 0;
}

void psx1_state::cpe_set_register(int r, int v)
{
	if (r < 0x80 && (r % 4) == 0)
	{
		logerror("psx_exe_load: r%-2d   %08x\n", r / 4, v);
		m_maincpu->set_state_int(PSXCPU_R0 + (r / 4), v);
	}
	else if (r == 0x80)
	{
		logerror("psx_exe_load: lo    %08x\n", v);
		m_maincpu->set_state_int(PSXCPU_LO, v);
	}
	else if (r == 0x84)
	{
		logerror("psx_exe_load: hi    %08x\n", v);
		m_maincpu->set_state_int(PSXCPU_HI, v);
	}
	else if (r == 0x88)
	{
		logerror("psx_exe_load: sr    %08x\n", v);
		m_maincpu->set_state_int(PSXCPU_CP0R12, v);
	}
	else if (r == 0x8c)
	{
		logerror("psx_exe_load: cause %08x\n", v);
		m_maincpu->set_state_int(PSXCPU_CP0R13, v);
	}
	else if (r == 0x90)
	{
		logerror("psx_exe_load: pc    %08x\n", v);
		m_maincpu->set_state_int(PSXCPU_PC, v);
	}
	else if (r == 0x94)
	{
		logerror("psx_exe_load: prid  %08x\n", v);
		m_maincpu->set_state_int(PSXCPU_CP0R15, v);
	}
	else
	{
		logerror("psx_exe_load: invalid register %04x/%08x\n", r, v);
	}
}

int psx1_state::load_cpe(std::vector<uint8_t> buffer)
{
	if (buffer.size() >= 4 &&
		memcmp(reinterpret_cast<void *>(&buffer[0]), "CPE\001", 4) == 0)
	{
		int offset = 4;

		for (;;)
		{
			if (offset >= buffer.size() || buffer[offset] > 8)
			{
				break;
			}

			switch (buffer[offset++])
			{
			case 0:
				/* end of file */
				return 1;

			case 1:
				/* read bytes */
				{
					uint32_t address = get_u32le(&buffer[offset]);
					uint32_t size = get_u32le(&buffer[offset + 4]);

					uint8_t *ram_pointer = m_ram->pointer();
					uint32_t ram_size = m_ram->size();

					offset += 8;

					logerror("psx_exe_load: org   %08x\n", address);
					logerror("psx_exe_load: len   %08x\n", size);

					while (size > 0)
					{
						ram_pointer[BYTE4_XOR_LE(address) % ram_size] = buffer[offset++];
						address++;
						size--;
					}
				}
				break;

			case 2:
				/* run address: not tested */
				{
					uint32_t v = get_u32le(&buffer[offset]);

					offset += 4;

					cpe_set_register(0x90, v);
				}
				break;

			case 3:
				/* set reg to longword */
				{
					uint16_t r = get_u16le(&buffer[offset]);
					uint32_t v = get_u32le(&buffer[offset + 2]);

					offset += 6;

					cpe_set_register(r, v);
				}
				break;

			case 4:
				/* set reg to word: not tested */
				{
					uint16_t r = get_u16le(&buffer[offset]);
					uint16_t v = get_u16le(&buffer[offset + 2]);

					offset += 4;

					cpe_set_register(r, v);
				}
				break;

			case 5:
				/* set reg to byte: not tested */
				{
					uint16_t r = get_u16le(&buffer[offset]);
					uint8_t v = buffer[offset + 2];

					offset += 3;

					cpe_set_register(r, v);
				}
				break;

			case 6:
				/* set reg to 3-byte: not tested */
				{
					uint16_t r = get_u16le(&buffer[offset]);
					uint32_t v = get_u24le(&buffer[offset + 2]);

					offset += 5;

					cpe_set_register(r, v);
				}
				break;

			case 7:
				/* workspace: not tested */
				offset += 4;
				break;

			case 8:
				/* unit */
				{
					int unit = buffer[offset + 0];

					offset++;

					logerror("psx_exe_load: unit  %08x\n", unit);
				}
				break;
			}
		}
	}

	return 0;
}

int psx1_state::load_psf(std::vector<uint8_t> buffer)
{
	unsigned long crc;
	unsigned long compressed_size;
	unsigned char *compressed_buffer;
	unsigned long uncompressed_size;
	std::vector<uint8_t> uncompressed_buffer;

	struct PSF_HEADER
	{
		unsigned char id[4];
		uint32_t reserved_size;
		uint32_t exe_size;
		uint32_t exe_crc;
	};

	struct PSF_HEADER *psf_header = reinterpret_cast<struct PSF_HEADER *>(&buffer[0]);

	if (buffer.size() >= sizeof(struct PSF_HEADER) &&
		memcmp(reinterpret_cast<void *>(&buffer[0]), "PSF", 3) == 0)
	{
		psxexe_conv32(&psf_header->reserved_size);
		psxexe_conv32(&psf_header->exe_size);
		psxexe_conv32(&psf_header->exe_crc);

		logerror("psx_exe_load: reserved_size %08x\n", psf_header->reserved_size);
		logerror("psx_exe_load: exe_size      %08x\n", psf_header->exe_size);
		logerror("psx_exe_load: exe_crc       %08x\n", psf_header->exe_crc);

		compressed_size = psf_header->exe_size;
		compressed_buffer = reinterpret_cast<Bytef *>(&buffer[sizeof(struct PSF_HEADER) + psf_header->reserved_size]);

		crc = crc32(crc32(0L, Z_NULL, 0), compressed_buffer, compressed_size);
		if (crc != psf_header->exe_crc)
		{
			logerror("psx_exe_load: psf invalid crc\n");
			return 0;
		}

		uncompressed_size = 0x200000;
		uncompressed_buffer.resize(uncompressed_size);

		if (uncompress(reinterpret_cast<Bytef *>(&uncompressed_buffer[0]), &uncompressed_size, compressed_buffer, compressed_size) != Z_OK)
		{
			logerror("psx_exe_load: psf uncompress failed\n");
		}
		else
		{
			uncompressed_buffer.resize(uncompressed_size);

			if (!load_psxexe(uncompressed_buffer))
			{
				logerror("psx_exe_load: psf load failed\n");
			}
			else
			{
				return 1;
			}
		}
	}

	return 0;
}

uint16_t psx1_state::parallel_r(offs_t offset)
{
	if (m_parallel->hascard())
	{
		uint16_t dat = m_parallel->exp_r(offset);
		return dat;
	}

	// all this could probably be a fake parallel device instead?
	const uint16_t bootloader[] =
	{
		0x00b0, 0x1f00, 0x694c, 0x6563, 0x736e, 0x6465, 0x6220, 0x2079,
		0x6f53, 0x796e, 0x4320, 0x6d6f, 0x7570, 0x6574, 0x2072, 0x6e45,
		0x6574, 0x7472, 0x6961, 0x6d6e, 0x6e65, 0x2074, 0x6e49, 0x2e63,
		0x1f00, 0x3c08, 0x8000, 0x3c09, 0x000c, 0x3529, 0x00fc, 0x8d0a,
		0xfffc, 0x2529, 0x0044, 0xad2a, 0xfffc, 0x0520, 0xfffc, 0x2508,
		0x8003, 0x3c08, 0x1800, 0x4088, 0xffff, 0x3c08, 0xffff, 0x3508,
		0x5800, 0x4088, 0xa180, 0x3c08, 0x0008, 0x03e0, 0x3800, 0x4088,
		0x1f00, 0x3c1a, 0x0100, 0x275a, 0x0008, 0x0340, 0x0010, 0x4200,
		0x3800, 0x4080, 0x1800, 0x4080, 0x5800, 0x4080, 0x1f00, 0x3c08,
		0x0000, 0xad00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
	};

	if (m_exe_buffer.size() != 0 && offset >= 0x40 && offset <= 0x8f)
	{
		return bootloader[offset - 0x40];
	}

	return 0;
}

void psx1_state::parallel_w(offs_t offset, uint16_t data)
{
	if (m_parallel->hascard())
	{
		m_parallel->exp_w(offset, data);
		return;
	}

	if (m_exe_buffer.size() != 0)
	{
		if (load_psxexe(m_exe_buffer) ||
			load_cpe(m_exe_buffer) ||
			load_psf(m_exe_buffer))
		{
			// stop the cpu from advancing to the next instruction, otherwise it would skip the exe's first instruction
			m_maincpu->set_state_int(PSXCPU_DELAYR, PSXCPU_DELAYR_PC);
			m_maincpu->set_state_int(PSXCPU_DELAYV, m_maincpu->state_int(PSXCPU_PC));

			// workround to fix controller in amidog tests that do not initialise the sio registers
			m_maincpu->space(AS_PROGRAM).write_word(0x1f80104e, 0x0088);
			m_maincpu->space(AS_PROGRAM).write_word(0x1f801048, 0x000d);

			// stop on the first instruction of the exe if the debugger is active
			machine().debug_break();
		}
		else
		{
			logerror("psx_exe_load: invalid exe\n");
			m_maincpu->reset();
		}

		m_exe_buffer.resize(0);
	}
}

QUICKLOAD_LOAD_MEMBER(psx1_state::quickload_exe)
{
	m_exe_buffer.resize(image.length());

	if (image.fread(reinterpret_cast<void *>(&m_exe_buffer[0]), image.length()) != image.length())
	{
		m_exe_buffer.resize(0);
		return std::make_pair(image_error::UNSPECIFIED, std::string());
	}

	return std::make_pair(std::error_condition(), std::string());
}

void psx1_state::cd_dma_read( uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size )
{
	uint8_t *psxram = (uint8_t *) p_n_psxram;
	m_psxcd->start_dma(psxram + n_address, n_size*4);
}

void psx1_state::cd_dma_write( uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size )
{
	printf("cd_dma_write?!: addr %x, size %x\n", n_address, n_size);
}

void psx1_state::psx_map(address_map &map)
{
	map(0x1f000000, 0x1f07ffff).rw(FUNC(psx1_state::parallel_r), FUNC(psx1_state::parallel_w));
}

void psx1_state::subcpu_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void psx1_state::psx_base(machine_config &config)
{
	m_maincpu->set_addrmap(AS_PROGRAM, &psx1_state::psx_map);
	m_maincpu->cd_read().set(m_psxcd, FUNC(psxcd_device::read));
	m_maincpu->cd_write().set(m_psxcd, FUNC(psxcd_device::write));
	m_maincpu->subdevice<ram_device>("ram")->set_default_size("2M");

	psxcontrollerports_device &controllers(PSXCONTROLLERPORTS(config, "controllers", 0));
	controllers.rxd().set("maincpu:sio0", FUNC(psxsio0_device::write_rxd));
	controllers.dsr().set("maincpu:sio0", FUNC(psxsio0_device::write_dsr));
	PSX_CONTROLLER_PORT(config, "port1", psx_controllers, "digital_pad");
	PSX_CONTROLLER_PORT(config, "port2", psx_controllers, "digital_pad");

	auto &sio0(*m_maincpu->subdevice<psxsio0_device>("sio0"));
	sio0.dtr_handler().set("controllers", FUNC(psxcontrollerports_device::write_dtr));
	sio0.sck_handler().set("controllers", FUNC(psxcontrollerports_device::write_sck));
	sio0.txd_handler().set("controllers", FUNC(psxcontrollerports_device::write_txd));

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();
	spu_device &spu(SPU(config, "spu", XTAL(67'737'600)/2, m_maincpu.target()));
	spu.add_route(0, "speaker", 1.00, 0);
	spu.add_route(1, "speaker", 1.00, 1);

	QUICKLOAD(config, "quickload", "cpe,exe,psf,psx").set_load_callback(FUNC(psx1_state::quickload_exe));

	PSX_PARALLEL_SLOT(config, "parallel", psx_parallel_devices, nullptr);

	PSXCD(config, m_psxcd, m_maincpu, "spu");
	m_psxcd->irq_handler().set("maincpu:irq", FUNC(psxirq_device::intin2));
	subdevice<psxdma_device>("maincpu:dma")->install_read_handler(3, psxdma_device::read_delegate(&psx1_state::cd_dma_read, this));
	subdevice<psxdma_device>("maincpu:dma")->install_write_handler(3, psxdma_device::write_delegate(&psx1_state::cd_dma_write, this));

	SOFTWARE_LIST(config, m_cd_softlist).set_original("psx");
}

void psx1_state::psj(machine_config &config)
{
	CXD8530CQ(config, m_maincpu, XTAL(67'737'600));

	/* TODO: visible area and refresh rate */
	CXD8561Q(config, "gpu", XTAL(53'693'175), 0x100000, m_maincpu.target()).set_screen("screen");

	psx_base(config);

	m_cd_softlist->set_filter("NTSC-J");
}

void psx1_state::psu(machine_config &config)
{
	psj(config);

	HD63705Z0(config, "subcpu", 4166667).set_addrmap(AS_PROGRAM, &psx1_state::subcpu_map); // FIXME: actually MC68HC05G6

	m_cd_softlist->set_filter("NTSC-U");
}

void psx1_state::pse(machine_config &config)
{
	CXD8530AQ(config, m_maincpu, XTAL(67'737'600));

	/* TODO: visible area and refresh rate */
	CXD8561Q(config, "gpu", XTAL(53'693'175), 0x100000, m_maincpu.target()).set_screen("screen");

	psx_base(config);

	m_cd_softlist->set_filter("PAL-E");
}

ROM_START( psj )
	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 )

	ROM_SYSTEM_BIOS( 0, "1.0j", "SCPH-1000/DTL-H1000" ) // 22091994
	ROMX_LOAD( "ps-10j.bin",    0x000000, 0x080000, CRC(3b601fc8) SHA1(343883a7b555646da8cee54aadd2795b6e7dd070), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS( 1, "1.1j", "SCPH-3000/DTL-H1000H (Version 1.1 01/22/95)" ) // 22091994
	ROMX_LOAD( "ps-11j.bin",    0x000000, 0x080000, CRC(3539def6) SHA1(b06f4a861f74270be819aa2a07db8d0563a7cc4e), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 2, "2.1j", "SCPH-3500 (Version 2.1 07/17/95 J)" ) // 22091994
	ROMX_LOAD( "ps-21j.bin",    0x000000, 0x080000, CRC(bc190209) SHA1(e38466a4ba8005fba7e9e3c7b9efeba7205bee3f), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS( 3, "2.2j", "SCPH-5000/DTL-H1200/DTL-H3000 (Version 2.2 12/04/95 J)" ) // 04121995
/*  ROMX_LOAD( "ps-22j.bad",    0x000000, 0x080000, BAD_DUMP CRC(8c93a399) SHA1(e340db2696274dda5fdc25e434a914db71e8b02b), ROM_BIOS(3) ) */
	ROMX_LOAD( "ps-22j.bin",    0x000000, 0x080000, CRC(24fc7e17) SHA1(ffa7f9a7fb19d773a0c3985a541c8e5623d2c30d), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS( 4, "2.2d", "DTL-H1100 (Version 2.2 03/06/96 D)" ) // 04121995
	ROMX_LOAD( "ps-22d.bin",    0x000000, 0x080000, CRC(decb22f5) SHA1(73107d468fc7cb1d2c5b18b269715dd889ecef06), ROM_BIOS(4) )

	ROM_SYSTEM_BIOS( 5, "3.0j", "SCPH-5500 (Version 3.0 09/09/96 J)" ) // 04121995
	ROMX_LOAD( "ps-30j.bin",    0x000000, 0x080000, CRC(ff3eeb8c) SHA1(b05def971d8ec59f346f2d9ac21fb742e3eb6917), ROM_BIOS(5) )

	ROM_SYSTEM_BIOS( 6, "4.0j", "SCPH-7000/SCPH-7500/SCPH-9000 (Version 4.0 08/18/97 J)" ) // 29051997
	ROMX_LOAD( "ps-40j.bin",    0x000000, 0x080000, CRC(ec541cd0) SHA1(77b10118d21ac7ffa9b35f9c4fd814da240eb3e9), ROM_BIOS(6) )

	ROM_SYSTEM_BIOS( 7, "4.1a", "SCPH-7000W (Version 4.1 11/14/97 A)" ) // 04121995
	ROMX_LOAD( "ps-41a,w.bin", 0x000000, 0x080000, CRC(b7c43dad) SHA1(1b0dbdb23da9dc0776aac58d0755dc80fea20975), ROM_BIOS(7) )

	ROM_SYSTEM_BIOS( 8, "4.3j", "SCPH-100 (Version 4.3 03/11/00 J)" ) // 04121995
	ROMX_LOAD( "psone-43j.bin", 0x000000, 0x080000, CRC(f2af798b) SHA1(339a48f4fcf63e10b5b867b8c93cfd40945faf6c), ROM_BIOS(8) )
ROM_END

ROM_START( psu )
	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 )

	ROM_SYSTEM_BIOS( 0, "2.0a", "DTL-H1001 (Version 2.0 05/07/95 A)" ) // 22091994
	ROMX_LOAD( "ps-20a.bin",    0x000000, 0x080000, CRC(55847d8c) SHA1(649895efd79d14790eabb362e94eb0622093dfb9), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS( 1, "2.1a", "DTL-H1101 (Version 2.1 07/17/95 A)" ) // 22091994
	ROMX_LOAD( "ps-21a.bin",    0x000000, 0x080000, CRC(aff00f2f) SHA1(ca7af30b50d9756cbd764640126c454cff658479), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 2, "2.2a", "SCPH-1001/DTL-H1201/DTL-H3001 (Version 2.2 12/04/95 A)" ) // 04121995
	ROMX_LOAD( "ps-22a.bin",    0x000000, 0x080000, CRC(37157331) SHA1(10155d8d6e6e832d6ea66db9bc098321fb5e8ebf), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS( 3, "3.0a", "SCPH-5501/SCPH-5503/SCPH-7003 (Version 3.0 11/18/96 A)" ) // 04121995
	ROMX_LOAD( "ps-30a.bin",    0x000000, 0x080000, CRC(8d8cb7e4) SHA1(0555c6fae8906f3f09baf5988f00e55f88e9f30b), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS( 4, "4.1a", "SCPH-7001/SCPH-7501/SCPH-7503/SCPH-9001/SCPH-9003/SCPH-9903 (Version 4.1 12/16/97 A)" ) // 04121995
	ROMX_LOAD( "ps-41a.bin",    0x000000, 0x080000, CRC(502224b6) SHA1(14df4f6c1e367ce097c11deae21566b4fe5647a9), ROM_BIOS(4) )

	ROM_SYSTEM_BIOS( 5, "4.5a", "SCPH-101 (Version 4.5 05/25/00 A)" ) // 04121995
	ROMX_LOAD( "psone-45a.bin", 0x000000, 0x080000, CRC(171bdcec) SHA1(dcffe16bd90a723499ad46c641424981338d8378), ROM_BIOS(5) )

	ROM_REGION( 0x10000, "subcpu", 0 )
	ROM_LOAD( "ram.ic304",    0x000000, 0x000400, CRC(e9fefc8b) SHA1(7ab4a498216fb9a3fca9daf6ad21b4553126e74f) )
	ROM_FILL( 0x0400, 0x0c00, 0xff )
	ROM_LOAD( "scea.ic304",   0x001000, 0x004000, CRC(82729934) SHA1(7d5f52eb9df1243dcdab32cb763a9eb6a22706d7) )
	ROM_FILL( 0x5000, 0xae00, 0xff )
	ROM_LOAD( "test.ic304",   0x00fe00, 0x000200, CRC(3b2f8041) SHA1(d7127cb4a9b5efe9deffab3b72ab4451cb30675b) )
ROM_END

ROM_START( pse )
	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 )

	ROM_SYSTEM_BIOS( 0, "2.0e", "DTL-H1002/SCPH-1002 (Version 2.0 05/10/95 E)" ) // 22091994
	ROMX_LOAD( "ps-20e.bin",    0x000000, 0x080000, CRC(9bb87c4b) SHA1(20b98f3d80f11cbf5a7bfd0779b0e63760ecc62c), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS( 1, "2.1e", "SCPH-1002/DTL-H1102 (Version 2.1 07/17/95 E)" ) // 22091994
	ROMX_LOAD( "ps-21e.bin",    0x000000, 0x080000, CRC(86c30531) SHA1(76cf6b1b2a7c571a6ad07f2bac0db6cd8f71e2cc), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 2, "2.2e", "SCPH-1002/DTL-H1202/DTL-H3002 (Version 2.2 12/04/95 E)" ) // 04121995
	ROMX_LOAD( "ps-22e.bin",    0x000000, 0x080000, CRC(1e26792f) SHA1(b6a11579caef3875504fcf3831b8e3922746df2c), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS( 3, "3.0e", "SCPH-5502/SCPH-5552 (Version 3.0 01/06/97 E)" ) // 04121995
/*  ROMX_LOAD( "ps-30e.bad",    0x000000, 0x080000, BAD_DUMP CRC(4d9e7c86) SHA1(f8de9325fc36fcfa4b29124d291c9251094f2e54), ROM_BIOS(3) ) */
	ROMX_LOAD( "ps-30e.bin",    0x000000, 0x080000, CRC(d786f0b9) SHA1(f6bc2d1f5eb6593de7d089c425ac681d6fffd3f0), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS( 4, "4.1e", "SCPH-7002/SCPH-7502/SCPH-9002 (Version 4.1 12/16/97 E)" ) // 04121995
	ROMX_LOAD( "ps-41e.bin",    0x000000, 0x080000, CRC(318178bf) SHA1(8d5de56a79954f29e9006929ba3fed9b6a418c1d), ROM_BIOS(4) )

	ROM_SYSTEM_BIOS( 5, "4.4e", "SCPH-102 (Version 4.4 03/24/00 E)" ) // 04121995
	ROMX_LOAD( "psone-44e.bin", 0x000000, 0x080000, CRC(0bad7ea9) SHA1(beb0ac693c0dc26daf5665b3314db81480fa5c7c), ROM_BIOS(5) )

	ROM_SYSTEM_BIOS( 6, "4.5e", "SCPH-102 (Version 4.5 05/25/00 E)" ) // 04121995
	ROMX_LOAD( "psone-45e.bin", 0x000000, 0x080000, CRC(76b880e5) SHA1(dbc7339e5d85827c095764fc077b41f78fd2ecae), ROM_BIOS(6) )
ROM_END

ROM_START( psa )
	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 )

	ROM_SYSTEM_BIOS( 0, "3.0a", "SCPH-5501/SCPH-5503/SCPH-7003 (Version 3.0 11/18/96 A)" ) // 04121995
	ROMX_LOAD( "ps-30a.bin",    0x000000, 0x080000, CRC(8d8cb7e4) SHA1(0555c6fae8906f3f09baf5988f00e55f88e9f30b), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS( 1, "4.1a", "SCPH-7001/SCPH-7501/SCPH-7503/SCPH-9001/SCPH-9003/SCPH-9903 (Version 4.1 12/16/97 A)" ) // 04121995
	ROMX_LOAD( "ps-41a.bin",    0x000000, 0x080000, CRC(502224b6) SHA1(14df4f6c1e367ce097c11deae21566b4fe5647a9), ROM_BIOS(1) )
ROM_END

/*
The version number & release date is stored in ascii text at the end of every bios, except for scph1000.
There is also a BCD encoded date at offset 0x100, but this is set to 22091994 for versions prior to 2.2
and 04121995 for all versions from 2.2 ( except Version 4.0J which is 29051997 ).

Consoles not dumped:

DTL-H1001H
SCPH-5001
SCPH-5002 (this is not mentioned in SCPH-102B.pdf so it's likely it doesn't exist)
SCPH-5003
SCPH-103

Holes in version numbers:

Version 2.0 J
Version 4.1 J (SCPH7000W uses 4.1 A)
Version 4.2 J
Version 4.4 J
Version 4.5 J
Version 4.0 A
Version 4.2 A
Version 4.3 A
Version 4.4 A
Version 4.0 E
Version 4.2 E
Version 4.3 E

*/

} // Anonymous namespace


//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY                            FULLNAME                            FLAGS
CONS( 1994, psj,  0,      0,      psj,     0,     psx1_state, empty_init, "Sony Computer Entertainment Inc", "Sony PlayStation (Japan)",         MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
CONS( 1995, pse,  psj,    0,      pse,     0,     psx1_state, empty_init, "Sony Computer Entertainment Inc", "Sony PlayStation (Europe)",        MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
CONS( 1995, psu,  psj,    0,      psu,     0,     psx1_state, empty_init, "Sony Computer Entertainment Inc", "Sony PlayStation (USA)",           MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
CONS( 1995, psa,  psj,    0,      psj,     0,     psx1_state, empty_init, "Sony Computer Entertainment Inc", "Sony PlayStation (Asia-Pacific)",  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
