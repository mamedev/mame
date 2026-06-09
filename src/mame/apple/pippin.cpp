// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  pippin.cpp: Apple/Bandai Pippin
  Preliminary driver by R. Belmont (based on pippin.c skeleton by Angelo Salese)
  Big thanks to the DingusPPC team

  Apple ASICs identified:
  -----------------------
  343S0152 Aspen (Bandit derived PCI host bridge and RAM controller)
  343S0153 Taos (640x480 framebuffer with double-buffering and convolution to fix interlace flicker)
  343S1125 Grand Central (SWIM III, Sound, VIA)
  341S0060 Cuda (68HC05 MCU, handles ADB and parameter ("CMOS") RAM)
  343S1146 MESH (customized 539x SCSI)
  343S1191(x2) Athens Prime PLL Clock Generator

  Other chips
  -----------
  Z8530 SCC
  CS4217 audio DAC
  Bt856 composite video encoder

  00000000 : RAM
  F0000000 : 1 MB video RAM (mirrors every 2 MB)
  F0800000 : Taos (video) registers
  F1000000 : Palette for 256 color mode
  F3000000 : Grand Central I/O controller for next 128K

****************************************************************************/

#include "emu.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/mn1880/mn1880.h"
#include "imagedev/cdromimg.h"
#include "machine/input_merger.h"
#include "machine/ram.h"
#include "sound/cdda.h"

#include "endianness.h"

#include "athensprime.h"
#include "awacs_macrisc.h"
#include "bandit.h"
#include "cuda.h"
#include "heathrow.h"
#include "macadb.h"
#include "taos.h"

#include "softlist.h"
#include "speaker.h"
class pippin_state : public driver_device
{
public:
	void pippin(machine_config &config);

	pippin_state(const machine_config &mconfig, device_type type, const char *tag);

	required_device<ppc_device> m_maincpu;
	required_device<aspen_host_device> m_aspen;
	required_device<grandcentral_device> m_grandcentral;
	required_device<athensprime_device> m_athensprime;
	required_device<cuda_device> m_cuda;
	required_device<taos_device> m_taos;
	required_device<macadb_device> m_macadb;
	required_device<ram_device> m_ram;

private:
	void pippin_map(address_map &map) ATTR_COLD;
	void cdmcu_mem(address_map &map) ATTR_COLD;
	void cdmcu_data(address_map &map) ATTR_COLD;

	void cuda_reset_w(int state);

	u8 mesh_r(offs_t offset);
	void mesh_w(offs_t offset, u8 data);
	void mesh_sequence(u8 cmd);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u16 m_mesh_bus_stat;
	u8 m_mesh_seq, m_mesh_int_stat, m_mesh_int_mask, m_mesh_exception, m_mesh_src_id;
	u8 m_mesh_dst_id, m_mesh_sync;
};

pippin_state::pippin_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_aspen(*this, "pci:00.0"),
	m_grandcentral(*this, "pci:10.0"),
	m_athensprime(*this, "athensprime"),
	m_cuda(*this, "cuda"),
	m_taos(*this, "taos"),
	m_macadb(*this, "macadb"),
	m_ram(*this, RAM_TAG),
	m_mesh_bus_stat(0),
	m_mesh_seq(0),
	m_mesh_int_stat(0),
	m_mesh_int_mask(0),
	m_mesh_exception(0),
	m_mesh_src_id(7),
	m_mesh_dst_id(0),
	m_mesh_sync(0)
{
}

void pippin_state::machine_start()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	u8 *const bank0 = reinterpret_cast<u8 *>(memshare("bank0_ram")->ptr());
	u8 *const bank1 = reinterpret_cast<u8 *>(memshare("bank1_ram")->ptr());

	// The firmware does some really strange checks for mirroring in the memory sizing routine
	space.install_ram(0x00c0'1000, 0x00c0'1FFF, bank0 + 0x200);
	space.install_ram(0x00c0'0000, 0x00c0'0FFF, bank0 + 0x200);
	space.install_ram(0x0040'0000, 0x0040'0FFF, bank0 + 0x200);
	space.install_ram(0x01c0'1000, 0x01c0'1FFF, bank1);
	space.install_ram(0x01c0'0000, 0x01c0'0FFF, bank1);
	space.install_ram(0x0140'0000, 0x0140'0FFF, bank1);
}

void pippin_state::machine_reset()
{
	// the PPC can't run until Cuda's ready
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

void pippin_state::pippin_map(address_map &map)
{
	map(0x0000'0000, 0x003f'ffff).ram().share("bank0_ram");
	map(0x0100'0000, 0x010f'ffff).ram().share("bank1_ram");

	map(0xf000'0000, 0xf100'ffff).m(m_taos, FUNC(taos_device::map));

	map(0xffc0'0000, 0xffff'ffff).rom().region("bootrom", 0);
}

void pippin_state::cuda_reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
	m_maincpu->set_input_line(INPUT_LINE_RESET, state);
}

// MESH SCSI slop-stub that's just good enough to get Pippin to show video.
// For entertainment purposes only.  Real nscsi device is coming completely separately.
namespace
{
	enum mesh_reg : u8
	{
		MESH_SEQUENCE = 0x3,
		MESH_BUSSTATUS0 = 0x4,
		MESH_BUSSTATUS1 = 0x5,
		MESH_EXCEPTION = 0x7,
		MESH_ERROR = 0x8,
		MESH_INTMASK = 0x9,
		MESH_INTERRUPT = 0xa,
		MESH_SOURCEID = 0xb,
		MESH_DESTID = 0xc,
		MESH_SYNCPARMS = 0xd,
		MESH_MESHID = 0xe
	};

	enum mesh_seqcmd : u8
	{
		SEQ_ARBITRATE = 0x1,
		SEQ_SELECT = 0x2,
		SEQ_BUSFREE = 0x9,
		SEQ_ENARESEL = 0xc,
		SEQ_DISRESEL = 0xd,
		SEQ_RESETMESH = 0xe,
		SEQ_FLUSHFIFO = 0xf
	};

	enum mesh_int : u8
	{
		MESH_INT_CMD_DONE = 1,
		MESH_INT_EXCEPTION = 2,
		MESH_INT_ERROR = 4
	};
	enum mesh_exc : u8
	{
		MESH_EXC_SEL_TIMEOUT = 1,
		MESH_EXC_PHASE_MM = 2,
		MESH_EXC_ARB_LOST = 4
	};

	static constexpr u16 SCSI_BSY = 1 << 14;
}

u8 pippin_state::mesh_r(offs_t offset)
{
	offset >>= 4;
	switch (offset)
	{
	case MESH_SEQUENCE:
		return m_mesh_seq;
	case MESH_BUSSTATUS0:
		return m_mesh_bus_stat & 0xff;
	case MESH_BUSSTATUS1:
		return (m_mesh_bus_stat >> 8) & 0xe0;
	case MESH_EXCEPTION:
		return m_mesh_exception;
	case MESH_ERROR:
		return 0;
	case MESH_INTMASK:
		return m_mesh_int_mask;
	case MESH_INTERRUPT:
		return m_mesh_int_stat;
	case MESH_SOURCEID:
		return m_mesh_src_id;
	case MESH_DESTID:
		return m_mesh_dst_id;
	case MESH_SYNCPARMS:
		return m_mesh_sync;
	case MESH_MESHID:
		return 0xe2;
	}
	return 0;
}

void pippin_state::mesh_w(offs_t offset, u8 data)
{
	offset >>= 4;
	switch (offset)
	{
	case MESH_SEQUENCE:
		mesh_sequence(data);
		break;
	case MESH_BUSSTATUS0:
		m_mesh_bus_stat = (m_mesh_bus_stat & 0xff00) | data;
		break;
	case MESH_BUSSTATUS1:
		m_mesh_bus_stat = (m_mesh_bus_stat & 0x00ff) | (u16(data) << 8);
		break;
	case MESH_INTMASK:
		m_mesh_int_mask = data;
		break;
	case MESH_INTERRUPT:
		m_mesh_int_stat &= ~(data & 0x07);
		break; // write-1-to-clear
	case MESH_SOURCEID:
		m_mesh_src_id = data;
		break;
	case MESH_DESTID:
		m_mesh_dst_id = data;
		break;
	case MESH_SYNCPARMS:
		m_mesh_sync = data;
		break;
	}
}

void pippin_state::mesh_sequence(u8 cmd)
{
	m_mesh_seq = cmd;
	m_mesh_int_stat &= ~MESH_INT_CMD_DONE; // re-asserted below on completion

	switch (cmd & 0x0f)
	{
	case SEQ_ARBITRATE:
		m_mesh_exception = 0;
		m_mesh_bus_stat = SCSI_BSY;
		m_mesh_int_stat |= MESH_INT_CMD_DONE;
		break;

	case SEQ_SELECT:
		m_mesh_exception |= MESH_EXC_SEL_TIMEOUT;
		m_mesh_bus_stat = 0;
		m_mesh_int_stat |= MESH_INT_EXCEPTION | MESH_INT_CMD_DONE;
		break;

	case SEQ_BUSFREE:
		m_mesh_bus_stat = 0;
		m_mesh_int_stat |= MESH_INT_CMD_DONE;
		break;

	case SEQ_RESETMESH:
		m_mesh_exception = 0;
		m_mesh_bus_stat = 0;
		m_mesh_int_stat |= MESH_INT_CMD_DONE;
		break;

	case SEQ_ENARESEL:
	case SEQ_DISRESEL:
		m_mesh_int_stat |= MESH_INT_CMD_DONE;
		break;

	case SEQ_FLUSHFIFO:
		break;

	default:
		m_mesh_int_stat |= MESH_INT_CMD_DONE;
		break;
	}
}

void pippin_state::cdmcu_mem(address_map &map)
{
	map(0x0000, 0xffff).rom().region("cdrom", 0);
}

void pippin_state::cdmcu_data(address_map &map)
{
	map(0x0000, 0x0001).noprw();
	map(0x0003, 0x0003).noprw();
	map(0x0004, 0x0008).nopw();
	map(0x0008, 0x0008).nopr();
	map(0x0009, 0x0009).noprw();
	map(0x000f, 0x000f).noprw();
	map(0x001f, 0x0021).nopw();
	map(0x0031, 0x0031).noprw();
	map(0x0033, 0x0033).nopw();
	map(0x0036, 0x0036).noprw();
	map(0x0060, 0x031f).ram();
	map(0x802a, 0x802a).nopw();
	map(0x802f, 0x8034).nopw();
	map(0x8032, 0x8032).nopr();
	map(0x8035, 0x8035).lr8(NAME([]() { return 0x40; }));
	map(0x8037, 0x8037).noprw();
	map(0x8038, 0x8039).nopw();
}

/* Input ports */
static INPUT_PORTS_START( pippin )
INPUT_PORTS_END

void pippin_state::pippin(machine_config &config)
{
	/* basic machine hardware */
	PPC603(config, m_maincpu, 66000000);
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);
	m_maincpu->set_addrmap(AS_PROGRAM, &pippin_state::pippin_map);

	PCI_ROOT(config, "pci", 0);
	aspen_host_device &aspen(ASPEN(config, m_aspen, 66000000, "maincpu"));
	aspen.set_dev_offset(1);

	cdrom_image_device &cdrom(CDROM(config, "cdrom"));
	cdrom.set_interface("cdrom");
	SOFTWARE_LIST(config, "cd_list").set_original("pippin");

	RAM(config, m_ram);
	m_ram->set_default_size("32M");

	GRAND_CENTRAL(config, m_grandcentral);
	m_grandcentral->set_maincpu_tag("maincpu");
	m_grandcentral->irq_callback().set_inputline(m_maincpu, PPC_IRQ);
	m_grandcentral->scsi1_r_callback().set(FUNC(pippin_state::mesh_r));
	m_grandcentral->scsi1_w_callback().set(FUNC(pippin_state::mesh_w));

	APPLE_TAOS(config, m_taos, 31.3344_MHz_XTAL);

	awacs_macrisc_device &awacs(AWACS_MACRISC(config, "codec", 45.1584_MHz_XTAL / 2));
	awacs.dma_output().set(m_grandcentral, FUNC(heathrow_device::codec_dma_read));

	m_grandcentral->codec_r_callback().set(awacs, FUNC(awacs_macrisc_device::read_macrisc));
	m_grandcentral->codec_w_callback().set(awacs, FUNC(awacs_macrisc_device::write_macrisc));

	SPEAKER(config, "speaker", 2).front();
	awacs.add_route(0, "speaker", 1.0, 0);
	awacs.add_route(1, "speaker", 1.0, 1);

	APPLE_ATHENSPRIME(config, m_athensprime, 20_MHz_XTAL);
	m_athensprime->pclock_changed().set(m_taos, FUNC(taos_device::set_pixclock));

	MACADB(config, m_macadb, 15.6672_MHz_XTAL);

	CUDA_V2XX(config, m_cuda, XTAL(32'768));
	m_cuda->set_default_bios_tag("341s0060");
	m_cuda->reset_callback().set(FUNC(pippin_state::cuda_reset_w));
	m_cuda->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_cuda->via_clock_callback().set(m_grandcentral, FUNC(heathrow_device::cb1_w));
	m_cuda->via_data_callback().set(m_grandcentral, FUNC(heathrow_device::cb2_w));
	m_macadb->adb_data_callback().set(m_cuda, FUNC(cuda_device::set_adb_line));

	input_merger_device &sda_merger(INPUT_MERGER_ALL_HIGH(config, "sda"));
	sda_merger.output_handler().append(m_cuda, FUNC(cuda_device::set_iic_sda));

	m_athensprime->sda_callback().set(sda_merger, FUNC(input_merger_device::in_w<1>));

	m_cuda->iic_sda_callback().set(sda_merger, FUNC(input_merger_device::in_w<0>));
	m_cuda->iic_sda_callback().append(m_athensprime, FUNC(athensprime_device::sda_write));

	m_cuda->iic_scl_callback().set(m_athensprime, FUNC(athensprime_device::scl_write));

	config.set_perfect_quantum(m_maincpu);

	m_grandcentral->pb3_callback().set(m_cuda, FUNC(cuda_device::get_treq));
	m_grandcentral->pb4_callback().set(m_cuda, FUNC(cuda_device::set_byteack));
	m_grandcentral->pb5_callback().set(m_cuda, FUNC(cuda_device::set_tip));
	m_grandcentral->cb2_callback().set(m_cuda, FUNC(cuda_device::set_via_data));

	mn1880_device &cdmcu(MN1880(config, "cdmcu", 8388608)); // type and clock unknown
	cdmcu.set_addrmap(AS_PROGRAM, &pippin_state::cdmcu_mem);
	cdmcu.set_addrmap(AS_DATA, &pippin_state::cdmcu_data);
	cdmcu.set_disable();
}

/* ROM definition */
/*

    BIOS versions

    dev
    monitor 341S0241 - 245,247,248,250
    1.0     341S0251..254               U1-U4
    1.2     341S0297..300               U15,U20,U31,U35
    1.3     341S0328..331               U1/U31, U2/U35, U3/U15 and U4/U20

*/

ROM_START( pippin )
	ROM_REGION( 0x400000, "bootrom",  ROMREGION_64BIT | ROMREGION_BE )
	ROM_SYSTEM_BIOS(0, "v13", "Kinka v 1.3")
	ROMX_LOAD( "bandai pippin,19960920 - kinka 1.3,- 3e6b3ee4-a52528e9ce8c.rom", 0x000000, 0x400000, CRC(87a1337d) SHA1(8e512af6e34dd823f3defec77d43ecbff1ecad54), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS(1, "v12", "Kinka v 1.2")
	ROMX_LOAD( "bandai pippin,19960628 - kinka 1.2,- 3e10e14c-72c40c1af23a.rom", 0x000000, 0x400000, CRC(4fead4b3) SHA1(3fa02e9b0fa702ac6e02edc08911eac8b50e2d1f), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS(2, "v1", "Kinka v 1.0")
	ROMX_LOAD( "341s0251.u1", 0x000006, 0x100000, CRC(aaea2449) SHA1(2f63e215260a42fb7c5f2364682d5e8c0604646f),ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(6) | ROM_BIOS(2))
	ROMX_LOAD( "341s0252.u2", 0x000004, 0x100000, CRC(3d584419) SHA1(e29c764816755662693b25f1fb3c24faef4e9470),ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(6) | ROM_BIOS(2))
	ROMX_LOAD( "341s0253.u3", 0x000002, 0x100000, CRC(d8ae5037) SHA1(d46ce4d87ca1120dfe2cf2ba01451f035992b6f6),ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(6) | ROM_BIOS(2))
	ROMX_LOAD( "341s0254.u4", 0x000000, 0x100000, CRC(3e2851ba) SHA1(7cbf5d6999e890f5e9ab2bc4b10ca897c4dc2016),ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(6) | ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "vgm", "Kinka GM version")
	ROMX_LOAD( "bandai pippin,19960128 - kinka gm flash,- 2bf65931-318e40f6a1f4.rom", 0x000000, 0x400000, CRC(4ff875e6) SHA1(eb8739cab1807c6c7c51acc7f4a3afc1f9c6ddbb), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS(4, "pre", "Kinka pre-release")
	ROMX_LOAD( "kinka-pre.rom", 0x000000, 0x400000, CRC(4ff875e6) SHA1(eb8739cab1807c6c7c51acc7f4a3afc1f9c6ddbb),ROM_BIOS(4) )

	ROM_REGION( 0x10000, "cdrom", 0 ) /* MATSUSHITA CR504-L OEM */
	ROM_LOAD( "504par4.0i.ic7", 0x0000, 0x10000, CRC(25f7dd46) SHA1(ec3b3031742807924c6259af865e701827208fec) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY           FULLNAME        FLAGS */
COMP( 1996, pippin, 0,      0,      pippin,  pippin, pippin_state, empty_init, "Apple / Bandai", "Pippin @mark", MACHINE_NOT_WORKING)
