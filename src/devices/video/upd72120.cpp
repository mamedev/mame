// license:BSD-3-Clause
// copyright-holders:O. Galibert

// uPD72120, the high-resolution successor to the uPD7220

#include "emu.h"
#include "upd72120.h"

DEFINE_DEVICE_TYPE(UPD72120, upd72120_device, "upd72120", "NEC uPD72120")

upd72120_device::upd72120_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, UPD72120, tag, owner, clock)
{
}

void upd72120_device::map(address_map &map)
{
	map(0x00, 0x7f).rw(FUNC(upd72120_device::unk_r), FUNC(upd72120_device::unk_w));

	map(0x00, 0x02). w(FUNC(upd72120_device::eadorg_w));
	map(0x03, 0x03). w(FUNC(upd72120_device::dadorg_w));
	map(0x04, 0x06). w(FUNC(upd72120_device::ead1_w));
	map(0x07, 0x07). w(FUNC(upd72120_device::dad1_w));
	map(0x08, 0x0a). w(FUNC(upd72120_device::ead2_w));
	map(0x0b, 0x0b). w(FUNC(upd72120_device::dad2_w));
	map(0x0c, 0x0e). w(FUNC(upd72120_device::pdisps_w));
	map(0x10, 0x12). w(FUNC(upd72120_device::pdispd_w));
	map(0x14, 0x15). w(FUNC(upd72120_device::pmax_w));
	map(0x16, 0x16). w(FUNC(upd72120_device::mod_w));
	map(0x18, 0x1a). w(FUNC(upd72120_device::ptnp_w));
	map(0x1c, 0x1e). w(FUNC(upd72120_device::stack_w));
	map(0x3c, 0x3c). w(FUNC(upd72120_device::bank_w));
	map(0x3d, 0x3d). w(FUNC(upd72120_device::control_w));
	map(0x40, 0x41). w(FUNC(upd72120_device::x_w));
	map(0x42, 0x43). w(FUNC(upd72120_device::y_w));
	map(0x44, 0x45). w(FUNC(upd72120_device::dx_w));
	map(0x46, 0x47). w(FUNC(upd72120_device::dy_w));
	map(0x48, 0x49). w(FUNC(upd72120_device::xs_w));
	map(0x4a, 0x4b). w(FUNC(upd72120_device::ys_w));
	map(0x4c, 0x4d). w(FUNC(upd72120_device::xe_w));
	map(0x4e, 0x4f). w(FUNC(upd72120_device::ye_w));
	map(0x50, 0x51). w(FUNC(upd72120_device::xc_w));
	map(0x52, 0x53). w(FUNC(upd72120_device::yc_w));
	map(0x54, 0x55). w(FUNC(upd72120_device::dh_w));
	map(0x56, 0x57). w(FUNC(upd72120_device::dv_w));
	map(0x58, 0x59). w(FUNC(upd72120_device::pitchs_w));
	map(0x5a, 0x5b). w(FUNC(upd72120_device::pitchd_w));
	map(0x5c, 0x5d). w(FUNC(upd72120_device::stmax_w));
	map(0x5e, 0x5f). w(FUNC(upd72120_device::planes_w));
	map(0x60, 0x61). w(FUNC(upd72120_device::ptn_cnt_w));
	map(0x62, 0x63). w(FUNC(upd72120_device::xclmin_w));
	map(0x64, 0x65). w(FUNC(upd72120_device::yclmin_w));
	map(0x66, 0x67). w(FUNC(upd72120_device::xclmax_w));
	map(0x68, 0x69). w(FUNC(upd72120_device::yclmax_w));
	map(0x6c, 0x6c). w(FUNC(upd72120_device::mag_w));
	map(0x6d, 0x6d). w(FUNC(upd72120_device::clip_w));
	map(0x6e, 0x6e). w(FUNC(upd72120_device::flags_w));
	map(0x6f, 0x6f). w(FUNC(upd72120_device::command_w));
	map(0x70, 0x71). w(FUNC(upd72120_device::display_flags_w));
	map(0x72, 0x73). w(FUNC(upd72120_device::display_pitch_w));
	map(0x74, 0x76). w(FUNC(upd72120_device::display_address_w));
	map(0x77, 0x77). w(FUNC(upd72120_device::wc_w));
	map(0x78, 0x79). w(FUNC(upd72120_device::gcsrx_w));
	map(0x7a, 0x7b). w(FUNC(upd72120_device::gcsrys_w));
	map(0x7c, 0x7d). w(FUNC(upd72120_device::gcsrye_w));
	map(0x7e, 0x7f). w(FUNC(upd72120_device::video_timings_w));
}

void upd72120_device::device_start()
{
	save_item(NAME(m_eadorg));
	save_item(NAME(m_dadorg));
	save_item(NAME(m_ead1));
	save_item(NAME(m_dad1));
	save_item(NAME(m_ead2));
	save_item(NAME(m_dad2));
	save_item(NAME(m_pdisps));
	save_item(NAME(m_pdispd));
	save_item(NAME(m_pmax));
	save_item(NAME(m_mod));
	save_item(NAME(m_ptnp));
	save_item(NAME(m_stack));
	save_item(NAME(m_bank));
	save_item(NAME(m_control));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_dx));
	save_item(NAME(m_dy));
	save_item(NAME(m_xs));
	save_item(NAME(m_ys));
	save_item(NAME(m_xe));
	save_item(NAME(m_ye));
	save_item(NAME(m_xc));
	save_item(NAME(m_yc));
	save_item(NAME(m_dh));
	save_item(NAME(m_dv));
	save_item(NAME(m_pitchs));
	save_item(NAME(m_pitchd));
	save_item(NAME(m_stmax));
	save_item(NAME(m_planes));
	save_item(NAME(m_ptn_cnt));
	save_item(NAME(m_xclmin));
	save_item(NAME(m_yclmin));
	save_item(NAME(m_xclmax));
	save_item(NAME(m_yclmax));
	save_item(NAME(m_mag));
	save_item(NAME(m_clip));
	save_item(NAME(m_flags));
	save_item(NAME(m_command));
	save_item(NAME(m_display_flags));
	save_item(NAME(m_display_pitch));
	save_item(NAME(m_display_address));
	save_item(NAME(m_wc));
	save_item(NAME(m_video_timings));
	save_item(NAME(m_video_timings_index));
	save_item(NAME(m_gcsrx));
	save_item(NAME(m_gcsrys));
	save_item(NAME(m_gcsrye));
}

void upd72120_device::device_reset()
{
	m_eadorg = 0;
	m_dadorg = 0;
	m_ead1 = 0;
	m_dad1 = 0;
	m_ead2 = 0;
	m_dad2 = 0;
	m_pdisps = 0;
	m_pdispd = 0;
	m_pmax = 0;
	m_mod = 0;
	m_ptnp = 0;
	m_stack = 0;
	m_bank = 0;
	m_control = 0;
	m_x = 0;
	m_y = 0;
	m_dx = 0;
	m_dy = 0;
	m_xs = 0;
	m_ys = 0;
	m_xe = 0;
	m_ye = 0;
	m_xc = 0;
	m_yc = 0;
	m_dh = 0;
	m_dv = 0;
	m_pitchs = 0;
	m_pitchd = 0;
	m_stmax = 0;
	m_planes = 0;
	m_ptn_cnt = 0;
	m_xclmin = 0;
	m_yclmin = 0;
	m_xclmax = 0;
	m_yclmax = 0;
	m_mag = 0;
	m_clip = 0;
	m_flags = 0;
	m_command = 0;
	m_display_flags = 0;
	m_display_pitch = 0;
	m_display_address = 0;
	m_wc = 0;
	std::fill(m_video_timings.begin(), m_video_timings.end(), 0);
	m_video_timings_index = 0;
	m_gcsrx = 0;
	m_gcsrys = 0;
	m_gcsrye = 0;
}

void upd72120_device::unk_w(offs_t offset, u8 data)
{
	logerror("unk_w %02x, %02x\n", offset, data);
}

u8 upd72120_device::unk_r(offs_t offset)
{
	logerror("unk_r %02x\n", offset);
	return 0;
}

void upd72120_device::eadorg_w(offs_t offset, u8 data)
{
	int shift = 8*offset;
	m_eadorg = (m_eadorg & ~(0xff << shift)) | (data << shift);
	if(offset == 2)
		logerror("eadorg %06x\n", m_eadorg);
}

void upd72120_device::dadorg_w(u8 data)
{
	m_dadorg = data & 0xf;
	logerror("dadorg %d\n", m_dadorg);
}

void upd72120_device::ead1_w(offs_t offset, u8 data)
{
	int shift = 8*offset;
	m_ead1 = (m_ead1 & ~(0xff << shift)) | (data << shift);
	if(offset == 2)
		logerror("ead1 %06x\n", m_ead1);
}

void upd72120_device::dad1_w(u8 data)
{
	m_dad1 = data & 0xf;
	logerror("dadorg %d\n", m_dad1);
}

void upd72120_device::ead2_w(offs_t offset, u8 data)
{
	int shift = 8*offset;
	m_ead2 = (m_ead2 & ~(0xff << shift)) | (data << shift);
	if(offset == 2)
		logerror("ead2 %06x\n", m_ead2);
}

void upd72120_device::dad2_w(u8 data)
{
	m_dad2 = data & 0xf;
	logerror("dadorg %d\n", m_dad2);
}

void upd72120_device::pdisps_w(offs_t offset, u8 data)
{
	int shift = 8*offset;
	m_pdisps = (m_pdisps & ~(0xff << shift)) | (data << shift);
	if(offset == 2)
		logerror("pdisps %06x\n", m_pdisps);
}

void upd72120_device::pdispd_w(offs_t offset, u8 data)
{
	int shift = 8*offset;
	m_pdispd = (m_pdispd & ~(0xff << shift)) | (data << shift);
	if(offset == 2)
		logerror("pdispd %06x\n", m_pdispd);
}

void upd72120_device::pmax_w(offs_t offset, u8 data)
{
	if(offset)
		m_pmax = (m_pmax & 0x00ff) | (data << 8);
	else
		m_pmax = (m_pmax & 0xff00) | data;

	if(offset)
		logerror("pmax %d\n", m_pmax);
}

void upd72120_device::mod_w(u8 data)
{
	m_mod = data;
	logerror("mod0 %d mod1 %d\n", BIT(m_mod, 0, 4), BIT(m_mod, 4, 4));
}

void upd72120_device::ptnp_w(offs_t offset, u8 data)
{
	int shift = 8*offset;
	m_ptnp = (m_ptnp & ~(0xff << shift)) | (data << shift);
	if(offset == 2)
		logerror("ptnp %06x\n", m_ptnp);
}

void upd72120_device::stack_w(offs_t offset, u8 data)
{
	int shift = 8*offset;
	m_stack = (m_stack & ~(0xff << shift)) | (data << shift);
	if(offset == 2)
		logerror("stack %06x\n", m_stack);
}

void upd72120_device::bank_w(u8 data)
{
	m_bank = data;
	logerror("bank %02x\n", m_bank);
}

void upd72120_device::control_w(u8 data)
{
	m_control = data;
	logerror("control dbie=%d pbie=%d cie=%d abort=%d reset=%d\n", BIT(m_control, 7), BIT(m_control, 6), BIT(m_control, 5), BIT(m_control, 1), BIT(m_control, 0));
}

void upd72120_device::mag_w(u8 data)
{
	m_mag = data;
	logerror("magv %d magh %d\n", BIT(m_mag, 0, 4), BIT(m_mag, 4, 4));
}

void upd72120_device::clip_w(u8 data)
{
	m_clip = data & 3;
	logerror("clip %d\n", m_clip);
}
void upd72120_device::flags_w(u8 data)
{
	m_flags = data;
	logerror("flags %02x\n", m_flags);
}
void upd72120_device::command_w(u8 data)
{
	m_command = data;
	logerror("command %02x\n", m_command);
}

void upd72120_device::display_flags_w(offs_t offset, u8 data)
{
	if(offset)
		m_display_flags = (m_display_flags & 0x00ff) | (data << 8);
	else
		m_display_flags = (m_display_flags & 0xff00) | data;

	if(offset)
		logerror("display flags sw/dt=%d dad+=%d %s re=%d sc=%d focl=%d tccl=%d mask=%d %s %s leo=%s, write=%s, vs=%s\n",
				 BIT(m_display_flags, 14, 2),
				 BIT(m_display_flags, 11, 3),
				 BIT(m_display_flags, 10) ? "interlace" : "progressive",
				 BIT(m_display_flags,  9),
				 BIT(m_display_flags,  8),
				 BIT(m_display_flags,  7),
				 BIT(m_display_flags,  6),
				 BIT(m_display_flags,  5),
				 BIT(m_display_flags,  4) ? "slave" : "master",
				 BIT(m_display_flags,  3) ? "blank" : "normal",
				 BIT(m_display_flags,  2) ? "odd" : "even",
				 BIT(m_display_flags,  1) ? "on" : "off",
				 BIT(m_display_flags,  0) ? "odd" : "even");
}

void upd72120_device::display_pitch_w(offs_t offset, u8 data)
{
	if(offset)
		m_display_pitch = (m_display_pitch & 0x00ff) | (data << 8);
	else
		m_display_pitch = (m_display_pitch & 0xff00) | data;

	m_display_pitch &= 0xfff;
	if(offset)
		logerror("display pitch %d\n", m_display_pitch);
}

void upd72120_device::display_address_w(offs_t offset, u8 data)
{
	int shift = 8*offset;
	m_display_address = (m_display_address & ~(0xff << shift)) | (data << shift);
	if(offset == 2)
		logerror("display address %06x\n", m_display_address);
}

void upd72120_device::wc_w(u8 data)
{
	m_wc = data;
	logerror("wc = %d\n", m_wc);
}

void upd72120_device::gcsrx_w(offs_t offset, u8 data)
{
	if(!offset)
		m_gcsrx = (m_gcsrx & 0xff00) | data;
	else
		m_gcsrx = (m_gcsrx & 0x00ff) | (data << 8);

	m_gcsrx &= 0xcfff;

	if(offset)
		logerror("crs=%d ce=%x gcsrx=%d\n", BIT(m_gcsrx, 15), BIT(m_gcsrx, 14), m_gcsrx & 0xfff);
}

void upd72120_device::gcsrys_w(offs_t offset, u8 data)
{
	if(!offset)
		m_gcsrys = (m_gcsrys & 0xff00) | data;
	else
		m_gcsrys = (m_gcsrys & 0x00ff) | (data << 8);

	m_gcsrys &= 0xfff;

	if(offset)
		logerror("gcsrys=%d\n", m_gcsrys);
}

void upd72120_device::gcsrye_w(offs_t offset, u8 data)
{
	if(!offset)
		m_gcsrye = (m_gcsrye & 0xff00) | data;
	else
		m_gcsrye = (m_gcsrye & 0x00ff) | (data << 8);

	m_gcsrye &= 0xfff;

	if(offset)
		logerror("gcsrye=%d\n", m_gcsrye);
}

void upd72120_device::video_timings_w(offs_t offset, u8 data)
{
	if(!offset)
		m_video_timings[m_video_timings_index] = (m_video_timings[m_video_timings_index] & 0xff00) | data;
	else {
		m_video_timings[m_video_timings_index] = (m_video_timings[m_video_timings_index] & 0x00ff) | (data << 8);
		m_video_timings_index ++;
		if(m_video_timings_index == 9) {
			m_video_timings_index = 0;
			logerror("video timings hs=%d hbp=%d hh=%d hd=%d hfp=%d htot=%d\n",
					 m_video_timings[0],
					 m_video_timings[1],
					 m_video_timings[2],
					 m_video_timings[3],
					 m_video_timings[4],
					 m_video_timings[0] + m_video_timings[1] + m_video_timings[3] + m_video_timings[4]);
			logerror("video timings vs=%d vbp=%d vd=%d vfp=%d vtot=%d\n",
					 m_video_timings[5],
					 m_video_timings[6],
					 m_video_timings[7],
					 m_video_timings[8],
					 m_video_timings[5] + m_video_timings[6] + m_video_timings[7] + m_video_timings[8]);
		}
	}
}

void upd72120_device::x_w(offs_t offset, u8 data)
{
	if(!offset)
		m_x = (m_x & 0xff00) | data;
	else
		m_x = (m_x & 0x00ff) | (data << 8);
	if(offset)
		logerror("x %d\n", m_x);
}

void upd72120_device::y_w(offs_t offset, u8 data)
{
	if(!offset)
		m_y = (m_y & 0xff00) | data;
	else
		m_y = (m_y & 0x00ff) | (data << 8);
	if(offset)
		logerror("y %d\n", m_y);
}

void upd72120_device::dx_w(offs_t offset, u8 data)
{
	if(!offset)
		m_dx = (m_dx & 0xff00) | data;
	else
		m_dx = (m_dx & 0x00ff) | (data << 8);
	if(offset)
		logerror("dx %d\n", m_dx);
}

void upd72120_device::dy_w(offs_t offset, u8 data)
{
	if(!offset)
		m_dy = (m_dy & 0xff00) | data;
	else
		m_dy = (m_dy & 0x00ff) | (data << 8);
	if(offset)
		logerror("dy %d\n", m_dy);
}

void upd72120_device::xs_w(offs_t offset, u8 data)
{
	if(!offset)
		m_xs = (m_xs & 0xff00) | data;
	else
		m_xs = (m_xs & 0x00ff) | (data << 8);
	if(offset)
		logerror("xs %d\n", m_xs);
}

void upd72120_device::ys_w(offs_t offset, u8 data)
{
	if(!offset)
		m_ys = (m_ys & 0xff00) | data;
	else
		m_ys = (m_ys & 0x00ff) | (data << 8);
	if(offset)
		logerror("ys %d\n", m_ys);
}

void upd72120_device::xe_w(offs_t offset, u8 data)
{
	if(!offset)
		m_xe = (m_xe & 0xff00) | data;
	else
		m_xe = (m_xe & 0x00ff) | (data << 8);
	if(offset)
		logerror("xe %d\n", m_xe);
}

void upd72120_device::ye_w(offs_t offset, u8 data)
{
	if(!offset)
		m_ye = (m_ye & 0xff00) | data;
	else
		m_ye = (m_ye & 0x00ff) | (data << 8);
	if(offset)
		logerror("ye %d\n", m_ye);
}

void upd72120_device::xc_w(offs_t offset, u8 data)
{
	if(!offset)
		m_xc = (m_xc & 0xff00) | data;
	else
		m_xc = (m_xc & 0x00ff) | (data << 8);
	if(offset)
		logerror("xc %d\n", m_xc);
}

void upd72120_device::yc_w(offs_t offset, u8 data)
{
	if(!offset)
		m_yc = (m_yc & 0xff00) | data;
	else
		m_yc = (m_yc & 0x00ff) | (data << 8);
	if(offset)
		logerror("yc %d\n", m_yc);
}

void upd72120_device::dh_w(offs_t offset, u8 data)
{
	if(!offset)
		m_dh = (m_dh & 0xff00) | data;
	else
		m_dh = (m_dh & 0x00ff) | (data << 8);
	if(offset)
		logerror("dh %d\n", m_dh);
}

void upd72120_device::dv_w(offs_t offset, u8 data)
{
	if(!offset)
		m_dv = (m_dv & 0xff00) | data;
	else
		m_dv = (m_dv & 0x00ff) | (data << 8);
	if(offset)
		logerror("dv %d\n", m_dv);
}

void upd72120_device::pitchs_w(offs_t offset, u8 data)
{
	if(!offset)
		m_pitchs = (m_pitchs & 0xff00) | data;
	else
		m_pitchs = (m_pitchs & 0x00ff) | (data << 8);
	if(offset)
		logerror("pitchs %d\n", m_pitchs);
}

void upd72120_device::pitchd_w(offs_t offset, u8 data)
{
	if(!offset)
		m_pitchd = (m_pitchd & 0xff00) | data;
	else
		m_pitchd = (m_pitchd & 0x00ff) | (data << 8);
	if(offset)
		logerror("pitchd %d\n", m_pitchd);
}

void upd72120_device::stmax_w(offs_t offset, u8 data)
{
	if(!offset)
		m_stmax = (m_stmax & 0xff00) | data;
	else
		m_stmax = (m_stmax & 0x00ff) | (data << 8);
	if(offset)
		logerror("stmax %d\n", m_stmax);
}

void upd72120_device::planes_w(offs_t offset, u8 data)
{
	if(!offset)
		m_planes = (m_planes & 0xff00) | data;
	else
		m_planes = (m_planes & 0x00ff) | (data << 8);
	if(offset)
		logerror("planes %d\n", m_planes);
}

void upd72120_device::ptn_cnt_w(offs_t offset, u8 data)
{
	if(!offset)
		m_ptn_cnt = (m_ptn_cnt & 0xff00) | data;
	else
		m_ptn_cnt = (m_ptn_cnt & 0x00ff) | (data << 8);
	if(offset)
		logerror("ptn_cnt %d\n", m_ptn_cnt);
}

void upd72120_device::xclmin_w(offs_t offset, u8 data)
{
	if(!offset)
		m_xclmin = (m_xclmin & 0xff00) | data;
	else
		m_xclmin = (m_xclmin & 0x00ff) | (data << 8);
	if(offset)
		logerror("xclmin %d\n", m_xclmin);
}

void upd72120_device::yclmin_w(offs_t offset, u8 data)
{
	if(!offset)
		m_yclmin = (m_yclmin & 0xff00) | data;
	else
		m_yclmin = (m_yclmin & 0x00ff) | (data << 8);
	if(offset)
		logerror("yclmin %d\n", m_yclmin);
}

void upd72120_device::xclmax_w(offs_t offset, u8 data)
{
	if(!offset)
		m_xclmax = (m_xclmax & 0xff00) | data;
	else
		m_xclmax = (m_xclmax & 0x00ff) | (data << 8);
	if(offset)
		logerror("x %d\n", m_xclmax);
}

void upd72120_device::yclmax_w(offs_t offset, u8 data)
{
	if(!offset)
		m_yclmax = (m_yclmax & 0xff00) | data;
	else
		m_yclmax = (m_yclmax & 0x00ff) | (data << 8);
	if(offset)
		logerror("x %d\n", m_yclmax);
}
