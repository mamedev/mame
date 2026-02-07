// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay, O. Galibert

#include "emu.h"
#include "luna_68k_gpu.h"

luna_68k_gpu_device::luna_68k_gpu_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, LUNA_68K_GPU, tag, owner, clock)
	, device_luna_68k_video_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_dac(*this, "dac")
	, m_mfp(*this, "mfp")
	, m_tty(*this, "tty")
	, m_duart(*this, "duart%u", 0U)
	, m_screen(*this, "screen")
	, m_host(*this, "gpu_host")
	, m_fb(*this, "gpu_fb", 0x800000*2, ENDIANNESS_LITTLE)
{
}

void luna_68k_gpu_device::device_start()
{
	save_item(NAME(m_command));
	save_item(NAME(m_size));
	save_item(NAME(m_expected_size));
	save_item(NAME(m_bx0));
	save_item(NAME(m_bx1));
	save_item(NAME(m_vx));
	save_item(NAME(m_by0));
	save_item(NAME(m_by1));
	save_item(NAME(m_vy));
	save_item(NAME(m_blnk));
	save_item(NAME(m_plndsp));
	save_item(NAME(m_rop));
	save_item(NAME(m_pnkmsk));
	save_item(NAME(m_rplnslct));
	save_item(NAME(m_wplnslct));
	save_item(NAME(m_wplndat));
	if(0)
	m_cpu->space(AS_PROGRAM).install_read_tap(0xf00288f4, 0xf00288f7, "rectangle_copy",
											  [this](offs_t offset, u32 &, u32 mem_mask) {
												  if(machine().side_effects_disabled())
													  return;
												  u32 sp = m_cpu->state_int(M68K_SP);
												  address_space &tspace = m_cpu->space(0);
												  logerror("rectangle_copy source=(%d, %d) rect=(%d, %d) dest=(%d, %d) rop=%02x caller=%x (%x, %08x)\n",
														   tspace.read_dword(sp+4),
														   tspace.read_dword(sp+8),
														   tspace.read_dword(sp+12),
														   tspace.read_dword(sp+16),
														   tspace.read_dword(sp+20),
														   tspace.read_dword(sp+24),
														   m_rop,
														   tspace.read_dword(sp), offset, mem_mask);
											  });
}

void luna_68k_gpu_device::device_reset()
{
	std::fill(m_command.begin(), m_command.end(), 0);
	m_size = 0;
	m_expected_size = 0;
	m_bx0 = 0;
	m_bx1 = 0;
	m_vx = 0;
	m_by0 = 0;
	m_by1 = 0;
	m_vy = 0;
	m_blnk = 0;
	m_plndsp = 0;
	m_rop = 0;
	m_pnkmsk = 0;
	m_rplnslct = 0;
	m_wplnslct = 0;
	m_wplndat = 0;
}

void luna_68k_gpu_device::gpu_w(u32 data)
{
	if(m_expected_size == CMD_SIZE_UNLIMITED) {
		if(data & 0x80000000) {
			multiword_done();
			m_expected_size = 0;
			// no return there, we want the new command handled

		} else {
			if(m_size >= m_command.size())
				m_size--;
			m_command[m_size++] = data;
			return;
		}
	} else if(m_expected_size) {			
		m_command[m_size++] = data;
		if(m_size == m_expected_size) {
			multiword_done();
			m_expected_size = 0;
		}
		return;
	}
	m_command[0] = data;
	m_size = 1;
	switch(data >> 16) {
	case 0x0000: buschng(); break;
	case 0x0002: lintex(); break;
	case 0x0004: filtex(); break;
	case 0x0006: ltprst(); break;
	case 0x0008: filpat(); break;
	case 0x000a: pickmd(); break;
	case 0x000c: pick(); break;
	case 0x0100: wplndat(); break;
	case 0x0102: wplnslct(); break;
	case 0x0104: rplnslct(); break;
	case 0x0108: spset(); break;
	case 0x0200: wvecdat(); break;
	case 0x0202: pnkmsk1(); break;
	case 0x0204: pnkmsk2(); break;
	case 0x0206: unk0206(); break;
	case 0x82e0: veconoff(); break; // 0=vecoff, 1=volmlt, 3=vecon
	case 0x82e1: polonoff(); break;
	case 0x82e2: recoff(); break;
	case 0x82e6: boxend(); break;
	case 0x82e7: vmend(); break;
	case 0x8408: polend(); break;
	case 0x8804: linwid(); break;
	case 0x8c00: viewx0(); break;
	case 0x8c01: viewy0(); break;
	case 0x8c02: viewx1(); break;
	case 0x8c03: viewy1(); break;
	case 0x8ce0: altviewx0(); break;
	case 0x8ce1: altviewy0(); break;
	case 0x8ce2: altviewx1(); break;
	case 0x8ce3: altviewy1(); break;
	case 0x8e0e: drawwd(); break;
	case 0x8e0f: gencmd(); break;
	case 0x8e1f: m_expected_size = 9; break;
	case 0x8efe: mod(); break;
	case 0xc200: m_expected_size = 2; break; // mata
	case 0xc201: m_expected_size = 2; break; // matb
	case 0xc203: m_expected_size = 2; break; // matx
	case 0xc210: m_expected_size = 2; break; // matc
	case 0xc211: m_expected_size = 2; break; // matd
	case 0xc213: m_expected_size = 2; break; // maty
	default: logerror("Unknown gpu command %08x\n", data); break;
	}
}

void luna_68k_gpu_device::gencmd()
{
	switch(u16(m_command[0])) {
	case 0x00: m_expected_size = CMD_SIZE_UNLIMITED; break; // veccmd (until polend, veccmd?)
	case 0x01: m_expected_size = CMD_SIZE_UNLIMITED; break; // strcmd (until polend)
	case 0x0c: m_expected_size = CMD_SIZE_UNLIMITED; break; // polcmd (until polend, linwid)
	case 0x1a: m_expected_size = 9; break; // bitblt
	case 0x1c: m_expected_size = 9; break; // clear
	case 0x20: fbiocmd(); break;
	default: logerror("Unknown gpu generic command %08x\n", m_command[0]); break;
	}
}

void luna_68k_gpu_device::multiword_done()
{
	switch(m_command[0] >> 16) {
	case 0x8e0f: {
		switch(u16(m_command[0])) {
		case 0x00: veccmd(); break;
		case 0x01: strcmd(); break;
		case 0x0c: polcmd(); break;
		case 0x1a: bitblt2(); break;
		case 0x1c: clear(); break;
		}
		break;
	}
	case 0x8e1f: bitblt1(); break;
	case 0xc200: mata(); break;
	case 0xc201: matb(); break;
	case 0xc203: matx(); break;
	case 0xc210: matc(); break;
	case 0xc211: matd(); break;
	case 0xc213: maty(); break;
	}
}

void luna_68k_gpu_device::buschng()
{
	if(u16(m_command[0]) != 2 && u16(m_command[0]) != 3)
		logerror("buschng %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::lintex()
{
	logerror("lintex %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::filtex()
{
	logerror("filtex %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::ltprst()
{
	logerror("lptrst\n");
}

void luna_68k_gpu_device::filpat()
{
	logerror("filpat %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::pickmd()
{
	logerror("pickmd %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::pick()
{
	logerror("pick %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::wplndat()
{
	m_wplndat = m_command[0] & 0xffff;
}

void luna_68k_gpu_device::wplnslct()
{
	m_wplnslct = m_command[0] & 0xffff;
}

void luna_68k_gpu_device::rplnslct()
{
	m_rplnslct = m_command[0] & 0xffff;
}

void luna_68k_gpu_device::spset()
{
	logerror("spset %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::wvecdat()
{
	logerror("wvecdat %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::pnkmsk1()
{
	m_pnkmsk = (m_pnkmsk & 0x0000ffff) | (u16(m_command[0]) << 16);
}

void luna_68k_gpu_device::pnkmsk2()
{
	m_pnkmsk = (m_pnkmsk & 0xffff0000) | u16(m_command[0]);
}

void luna_68k_gpu_device::unk0206()
{
	logerror("unk0206 %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::veconoff()
{
	logerror("vec off/mlt/on %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::polonoff()
{
	logerror("pol off/on %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::recoff()
{
	logerror("recoff\n");
}

void luna_68k_gpu_device::boxend()
{
	logerror("boxend\n");
}

void luna_68k_gpu_device::vmend()
{
	logerror("vmend\n");
}

void luna_68k_gpu_device::polend()
{
	logerror("polend\n");
}

void luna_68k_gpu_device::linwid()
{
	logerror("linwid %x\n", u16(m_command[0]) << 1);
}

void luna_68k_gpu_device::viewx0()
{
	logerror("viewx0 %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::viewy0()
{
	logerror("viewy0 %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::viewx1()
{
	logerror("viewx1 %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::viewy1()
{
	logerror("viewy1 %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::altviewx0()
{
	logerror("altviewx0 %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::altviewy0()
{
	logerror("altviewy0 %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::altviewx1()
{
	logerror("altviewx1 %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::altviewy1()
{
	logerror("altviewy1 %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::drawwd()
{
	logerror("drawwd\n");
}

void luna_68k_gpu_device::mod()
{
	logerror("mod %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::fbiocmd()
{
	if(u16(m_command[0]) != 0x20)
		logerror("fbiocmd %x\n", u16(m_command[0]));
}

void luna_68k_gpu_device::veccmd()
{
	std::string s;
	for(u32 i=1; i != m_size; i++)
		s += util::string_format(" %x", m_command[i]);
	logerror("veccmd %s\n", s);
}

void luna_68k_gpu_device::strcmd()
{
	std::string s;
	for(u32 i=1; i != m_size; i++)
		s += util::string_format(" %x", m_command[i]);
	logerror("strcmd %s\n", s);
}

void luna_68k_gpu_device::polcmd()
{
	std::string s;
	for(u32 i=1; i != m_size; i++)
		s += util::string_format(" %x", m_command[i]);
	logerror("polcmd %s\n", s);
}

void luna_68k_gpu_device::bitblt2()
{
	std::string s;
	for(u32 i=1; i != m_size; i++)
		s += util::string_format(" %x", m_command[i]);
	logerror("bitblt2 %s\n", s);
}

void luna_68k_gpu_device::clear()
{
	std::string s;
	for(u32 i=1; i != m_size; i++)
		s += util::string_format(" %x", m_command[i]);
	logerror("clear %s\n", s);

	u32 x0 = m_command[5];
	u32 x1 = m_command[7];
	if(x1 > 2047)
		x1 = 2047;

	u32 y0 = m_command[3];
	u32 y1 = m_command[4];
	if(y1 > 4095)
		y1 = 4095;

	for(u32 y = y0; y <= y1; y++) {
		u16 *dest = m_fb + (y << 11);
		for(u32 x = x0; x <= x1; x++) {
			u16 c = *dest;
			*dest++ = (c & ~m_wplnslct) | (m_wplndat & m_wplnslct);
		}
	}
}

void luna_68k_gpu_device::bitblt1()
{
	std::string s;
	for(u32 i=1; i != m_size; i++)
		s += util::string_format(" %x", m_command[i]);
	logerror("bitblt1 %s\n", s);
}

void luna_68k_gpu_device::mata()
{
	logerror("mata %x %x\n", u16(m_command[0]), m_command[1]);
}

void luna_68k_gpu_device::matb()
{
	logerror("matb %x %x\n", u16(m_command[0]), m_command[1]);
}

void luna_68k_gpu_device::matx()
{
	logerror("matx %x %x\n", u16(m_command[0]), m_command[1]);
}

void luna_68k_gpu_device::matc()
{
	logerror("matc %x %x\n", u16(m_command[0]), m_command[1]);
}

void luna_68k_gpu_device::matd()
{
	logerror("matd %x %x\n", u16(m_command[0]), m_command[1]);
}

void luna_68k_gpu_device::maty()
{
	logerror("maty %x %x\n", u16(m_command[0]), m_command[1]);
}

u16 luna_68k_gpu_device::state1_r()
{
	// 1e00 != 0 = general busy of the pipeline
	// 2000 = vblank
	return m_screen->vblank() ? 0x2000 : 0;
}

u16 luna_68k_gpu_device::state2_r()
{
	// 8000 = !busy on picchk
	// 4000 = picchk result
	return 0x8000;
}

u16 luna_68k_gpu_device::state3_r()
{
	// 4000 = busy on boxchk
	// 2000 = boxchk result
	return 0x0000;
}

u16 luna_68k_gpu_device::bx0_r()
{
	return m_bx0;
}

void luna_68k_gpu_device::bx0_w(u16 data)
{
	m_bx0 = data;
	logerror("bx0 %x\n", m_bx0);
}

u16 luna_68k_gpu_device::bx1_r()
{
	return m_bx1;
}

void luna_68k_gpu_device::bx1_w(u16 data)
{
	m_bx1 = data;
	logerror("bx1 %x\n", m_bx1);
}

u16 luna_68k_gpu_device::by0_r()
{
	return m_by0;
}

void luna_68k_gpu_device::by0_w(u16 data)
{
	m_by0 = data;
	logerror("by0 %x\n", m_by0);
}

u16 luna_68k_gpu_device::by1_r()
{
	return m_by1;
}

void luna_68k_gpu_device::by1_w(u16 data)
{
	m_by1 = data;
	logerror("by1 %x\n", m_by1);
}

u16 luna_68k_gpu_device::vx_r()
{
	return m_vx;
}

void luna_68k_gpu_device::vx_w(u16 data)
{
	m_vx = data;
	logerror("vx %x\n", m_vx);
}

u16 luna_68k_gpu_device::vy_r()
{
	return m_vy;
}

void luna_68k_gpu_device::vy_w(u16 data)
{
	m_vy = data;
	logerror("vy %x\n", m_vy);
}

u16 luna_68k_gpu_device::blnk_r()
{
	return m_blnk;
}

void luna_68k_gpu_device::blnk_w(u16 data)
{
	m_blnk = data;
	logerror("blnk %x\n", m_blnk);
}

u16 luna_68k_gpu_device::plndsp_r()
{
	return m_plndsp;
}

void luna_68k_gpu_device::plndsp_w(u16 data)
{
	m_plndsp = data;
	logerror("plndsp %x\n", m_plndsp);
}

u16 luna_68k_gpu_device::rop_r()
{
	return m_rop;
}

void luna_68k_gpu_device::rop_w(u16 data)
{
	m_rop = data;
}

u32 luna_68k_gpu_device::fb_r(offs_t offset)
{
	u32 res = 0;
	const u16 *src = &m_fb[offset << 5];
	for(u32 x=0; x != 32; x++)
		if(BIT(*src++, m_rplnslct))
			res |= 0x80000000 >> x;
	if(0)
		logerror("fb_r %05x (%4d, %4d) = %08x rop %02x (%s)\n", offset, (offset & 63)*32, offset >> 6, res, m_rop, machine().describe_context());
	return res;
}

void luna_68k_gpu_device::fb_w(offs_t offset, u32 data)
{
	// Needs to take the rop into account, but no idea what the values
	// mean at this point

	if(0)
		logerror("fb_w %05x (%4d, %4d) = %08x & %08x rop %02x (%s)\n", offset, (offset & 63)*32, offset >> 6, data, m_pnkmsk, m_rop, machine().describe_context());

	// rops
	//  00 = dst = src   (used when scrolling the whole text screen)
	//  15 = ?           (used in the initial wall-of-letters)
	//  30 = dst = 0     (used when clearing part/all of the text screen)
	//  3a = dst = ~src? (used for the cursor, could be dst = ~dst too maybe)

	if(m_rop == 0x30)
		data = 0;
	if(m_rop == 0x3a)
		data = ~data;

	u16 *dest = &m_fb[offset << 5];
	for(u32 x=0; x != 32; x++)
		if(BIT(m_pnkmsk, 31-x)) {
			u16 c = *dest;
			if(BIT(data, 31-x))
				*dest++ = (c & ~m_wplnslct) | (m_wplndat & m_wplnslct);
			else
				*dest++ = c & ~m_wplnslct;
		} else
			dest++;
}

void luna_68k_gpu_device::host_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(m_host + offset);
	switch(offset) {
	case 0: m_mfp->i3_w(m_host[0] == 0); break;
	case 1: m_mfp->i4_w(m_host[1] == 0); break;
	case 2: m_mfp->i5_w(m_host[2] == 0); break;
	case 3: m_mfp->i6_w(m_host[3] == 0); break;
	}
}

u32 luna_68k_gpu_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for(int y=0; y != 1024; y++) {
		u16 *dest = &bitmap.pix(y^1023);
		for(int x=0; x != 1280; x++) {
			u16 c = m_fb[(y << 11) | x];
			*dest++ = BIT(c, 15) ? 259 : c & 0xff;
		}
	}
	return 0;
}


void luna_68k_gpu_device::vme_map(address_map &map)
{
	map(0xe1000000, 0xe1ffffff).unmaprw();
	map(0xe1f00000, 0xe1f0ffff).ram().share("gpu_host").w(FUNC(luna_68k_gpu_device::host_w));
}

void luna_68k_gpu_device::cpu_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).rom().region("gpu", 0);

	map(0x50000000, 0x5003ffff).rw(FUNC(luna_68k_gpu_device::fb_r), FUNC(luna_68k_gpu_device::fb_w));

	map(0x60000000, 0x60001fff).nopw(); // dither pattern?

	map(0x70000000, 0x70000003).w(FUNC(luna_68k_gpu_device::gpu_w));
	map(0x70000108, 0x7000010f).rw(FUNC(luna_68k_gpu_device::bx0_r), FUNC(luna_68k_gpu_device::bx0_w)).umask32(0x0000ffff);
	map(0x70000118, 0x7000011f).rw(FUNC(luna_68k_gpu_device::bx1_r), FUNC(luna_68k_gpu_device::bx1_w)).umask32(0x0000ffff);
	map(0x70000128, 0x7000012f).rw(FUNC(luna_68k_gpu_device::vx_r), FUNC(luna_68k_gpu_device::vx_w)).umask32(0x0000ffff);
	map(0x70000132, 0x70000133).r(FUNC(luna_68k_gpu_device::state1_r));
	map(0x70000136, 0x70000137).r(FUNC(luna_68k_gpu_device::state2_r));
	map(0x7000013a, 0x7000013b).r(FUNC(luna_68k_gpu_device::state3_r));
	map(0x70000188, 0x7000018f).rw(FUNC(luna_68k_gpu_device::by0_r), FUNC(luna_68k_gpu_device::by0_w)).umask32(0x0000ffff);
	map(0x70000198, 0x7000019f).rw(FUNC(luna_68k_gpu_device::by1_r), FUNC(luna_68k_gpu_device::by1_w)).umask32(0x0000ffff);
	map(0x700001a8, 0x700001af).rw(FUNC(luna_68k_gpu_device::vy_r), FUNC(luna_68k_gpu_device::vy_w)).umask32(0x0000ffff);
	map(0x70000300, 0x7000030f).m(m_dac, FUNC(bt458_device::map)).umask32(0x000000ff);
	map(0x70000312, 0x70000313).rw(FUNC(luna_68k_gpu_device::blnk_r), FUNC(luna_68k_gpu_device::blnk_w));
	map(0x70000316, 0x7000031b).rw(FUNC(luna_68k_gpu_device::plndsp_r), FUNC(luna_68k_gpu_device::plndsp_w));
	map(0x70000322, 0x70000323).rw(FUNC(luna_68k_gpu_device::rop_r), FUNC(luna_68k_gpu_device::rop_w));

	// Tests is there's ram at 80800000, 80c00000 and 81000000,
	// records it as 4M, 8M, 12M or 28M available
	// DRAM
	map(0x80000000, 0x80bfffff).ram(); // M5M41000BJ  1mb  (1m x 1) dynamic RAM (8x12) - 12MB

	map(0xb0001000, 0xb0001003).nopw();

	map(0xb0004000, 0xb0004003).portr("DIPS");
	map(0xb0080000, 0xb008001f).rw(m_mfp, FUNC(mc68901_device::read), FUNC(mc68901_device::write));
	map(0xb0081000, 0xb008100f).rw(m_duart[0], FUNC(mc68681_device::read), FUNC(mc68681_device::write));
	map(0xb0082000, 0xb008200f).rw(m_duart[1], FUNC(mc68681_device::read), FUNC(mc68681_device::write));

	map(0xb0090000, 0xb00900ff).ram().share("gpu_nvram"); // MBM2212-20 256x4 NVRAM x 2 - 256B

	// DP-RAM
	map(0xc0000000, 0xc000ffff).ram().share("gpu_host").w(FUNC(luna_68k_gpu_device::host_w)); // M5M5178P-55 64kb (8k x 8) static RAM (2x4)   -  64kB

	// SRAM
	map(0xf0000000, 0xf003ffff).ram(); // M5M5258P-35 256kb (64k x 4) static RAM (x8)  - 256kB
}

void luna_68k_gpu_device::cpuspace_map(address_map &map)
{
	map(0xfffffff0, 0xffffffff).m(m_cpu, FUNC(m68020_device::autovectors_map));
	map(0xfffffff5, 0xfffffff5).r(m_mfp, FUNC(mc68901_device::get_vector));
}

void luna_68k_gpu_device::device_add_mconfig(machine_config &config)
{
	M68020FPU(config, m_cpu, 33'340'000 / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &luna_68k_gpu_device::cpu_map);
	m_cpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &luna_68k_gpu_device::cpuspace_map);

	BT458(config, m_dac, 108_MHz_XTAL);

	MC68901(config, m_mfp, 3.6864_MHz_XTAL);
	m_mfp->set_timer_clock(3.6864_MHz_XTAL);
	m_mfp->out_tdo_cb().set(m_mfp, FUNC(mc68901_device::tc_w));
	m_mfp->out_tdo_cb().append(m_mfp, FUNC(mc68901_device::rc_w));
	m_mfp->out_irq_cb().set_inputline(m_cpu, M68K_IRQ_2);

	RS232_PORT(config, m_tty, default_rs232_devices, nullptr);
	m_mfp->out_so_cb().set(m_tty, FUNC(rs232_port_device::write_txd));
	m_tty->rxd_handler().set(m_mfp, FUNC(mc68901_device::si_w));

	MC68681(config, m_duart[0], 3.6864_MHz_XTAL);
	MC68681(config, m_duart[1], 3.6864_MHz_XTAL);

	NVRAM(config, "gpu_nvram");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// VBlank may be connected to mfp interrupt 0

	m_screen->set_raw(108_MHz_XTAL, 1728, 0, 1280, 1056, 0, 1024);
	m_screen->set_screen_update(FUNC(luna_68k_gpu_device::screen_update));
	m_screen->set_palette(m_dac);
}

static INPUT_PORTS_START(luna_68k_gpu)
	PORT_START("DIPS")
	PORT_DIPNAME(0x01000000, 0x00000000, "Monitor (on serial)")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x01000000, "On")
	PORT_DIPNAME(0x02000000, 0x00000000, "unk01")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x02000000, "On")
	PORT_DIPNAME(0x04000000, 0x00000000, "unk02")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x04000000, "On")
	PORT_DIPNAME(0x08000000, 0x08000000, "RAM test")
	PORT_DIPSETTING(         0x00000000, "On")
	PORT_DIPSETTING(         0x08000000, "Off")
	PORT_DIPNAME(0x10000000, 0x00000000, "unk04")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x10000000, "On")
	PORT_DIPNAME(0x20000000, 0x00000000, "Verbose errors")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x20000000, "On")
	PORT_DIPNAME(0x40000000, 0x00000000, "unk06")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x40000000, "On")
	PORT_DIPNAME(0x80000000, 0x00000000, "Rectangle copy")
	PORT_DIPSETTING(         0x00000000, "Software")
	PORT_DIPSETTING(         0x80000000, "Accelerated")
INPUT_PORTS_END

ROM_START( luna_68k_gpu )
	ROM_REGION32_BE(0x40000, "gpu", 0)
	ROM_LOAD("jaw-2500__rom0__v1.21.rom0", 0x00000, 0x20000, CRC(3aa5dfa8) SHA1(e703402b6d2271c303c6abe8833281e994c244de))
	ROM_LOAD("jaw-2500__rom1__v1.21.rom1", 0x20000, 0x20000, CRC(9881eecd) SHA1(4a87417d9bf801e797bf504d18a6c6b5d3911706))

ROM_END

ioport_constructor luna_68k_gpu_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(luna_68k_gpu);
}

const tiny_rom_entry *luna_68k_gpu_device::device_rom_region() const
{
	return ROM_NAME(luna_68k_gpu);
}

DEFINE_DEVICE_TYPE(LUNA_68K_GPU, luna_68k_gpu_device, "luna_68k_gpu", "Omron Luna 68k DPU/GPU combination VME board")
