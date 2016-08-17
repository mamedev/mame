// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*
    DataEast/Sega Version 3b
*/


#include "emu.h"
#include "machine/decopincpu.h"
#include "video/decodmd3.h"
#include "audio/decobsmt.h"
#include "machine/genpin.h"
#include "machine/nvram.h"

extern const char layout_pinball[];
class de_3b_state : public driver_device
{
public:
	de_3b_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_decobsmt(*this, "decobsmt"),
			m_dmdtype3(*this, "decodmd")
	{ }

	// devices
	optional_device<decobsmt_device> m_decobsmt;
	optional_device<decodmd_type3_device> m_dmdtype3;

	DECLARE_WRITE8_MEMBER(lamp0_w) { };
	DECLARE_WRITE8_MEMBER(lamp1_w) { };
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_WRITE8_MEMBER(switch_w);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_READ8_MEMBER(dmd_status_r);
	DECLARE_WRITE8_MEMBER(pia2c_pa_w);
	DECLARE_READ8_MEMBER(pia2c_pb_r);
	DECLARE_WRITE8_MEMBER(pia2c_pb_w);

	// devcb callbacks
	DECLARE_READ8_MEMBER(display_r);
	DECLARE_WRITE8_MEMBER(display_w);
	DECLARE_WRITE8_MEMBER(lamps_w);

protected:

	// driver_device overrides
	virtual void machine_reset() override;
public:
	DECLARE_DRIVER_INIT(de_3b);

	UINT8 m_strobe;
	UINT8 m_kbdrow;
	UINT8 m_diag;
	bool m_ca1;
	bool m_irq_active;
	UINT8 m_sound_data;

};


static INPUT_PORTS_START( de_3b )
	PORT_START("INP0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INP1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )

	PORT_START("INP2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("INP4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)

	PORT_START("INP8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("INP10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)

	PORT_START("INP20")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("INP40")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INP80")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

READ8_MEMBER( de_3b_state::switch_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"INP%X",m_kbdrow);
	return ~ioport(kbdrow)->read();
}

WRITE8_MEMBER( de_3b_state::switch_w )
{
	int x;

	for(x=0;x<8;x++)
	{
		if(data & (1<<x))
			break;
	}
	m_kbdrow = data & (1<<x);
}

WRITE8_MEMBER( de_3b_state::sound_w )
{
	m_sound_data = data;
	if(m_sound_data != 0xfe)
		m_decobsmt->bsmt_comms_w(space,offset,m_sound_data);
}

READ8_MEMBER( de_3b_state::dmd_status_r )
{
	return m_dmdtype3->status_r(space,offset);
}

WRITE8_MEMBER( de_3b_state::pia2c_pa_w )
{
	/* DMD data */
	m_dmdtype3->data_w(space,offset,data);
	logerror("DMD: Data write %02x\n", data);
}

READ8_MEMBER( de_3b_state::pia2c_pb_r )
{
	return m_dmdtype3->busy_r(space,offset);
}

WRITE8_MEMBER( de_3b_state::pia2c_pb_w )
{
	/* DMD ctrl */
	m_dmdtype3->ctrl_w(space,offset,data);
	logerror("DMD: Control write %02x\n", data);
}
READ8_MEMBER(de_3b_state::display_r)
{
	UINT8 ret = 0x00;

	switch(offset)
	{
	case 0:
//      ret = pia28_w7_r(space,0);
		break;
	case 3:
		ret = pia2c_pb_r(space,0);
		break;
	}

	return ret;
}

WRITE8_MEMBER(de_3b_state::display_w)
{
	switch(offset)
	{
	case 0:
//      dig0_w(space,0,data);
		break;
	case 1:
//      dig1_w(space,0,data);
		break;
	case 2:
		pia2c_pa_w(space,0,data);
		break;
	case 3:
		pia2c_pb_w(space,0,data);
		break;
	case 4:
//      pia34_pa_w(space,0,data);
		break;
	}
}

WRITE8_MEMBER(de_3b_state::lamps_w)
{
	switch(offset)
	{
	case 0:
		lamp0_w(space,0,data);
		break;
	case 1:
		lamp1_w(space,0,data);
		break;
	}
}

void de_3b_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(de_3b_state,de_3b)
{
}

static MACHINE_CONFIG_START( de_3b, de_3b_state )
	/* basic machine hardware */
	MCFG_DECOCPU_TYPE3B_ADD("decocpu",XTAL_8MHz / 2, ":maincpu")
	MCFG_DECOCPU_DISPLAY(READ8(de_3b_state,display_r),WRITE8(de_3b_state,display_w))
	MCFG_DECOCPU_SOUNDLATCH(WRITE8(de_3b_state,sound_w))
	MCFG_DECOCPU_SWITCH(READ8(de_3b_state,switch_r),WRITE8(de_3b_state,switch_w))
	MCFG_DECOCPU_LAMP(WRITE8(de_3b_state,lamps_w))
	MCFG_DECOCPU_DMDSTATUS(READ8(de_3b_state,dmd_status_r))

	MCFG_FRAGMENT_ADD( genpin_audio )

	/* sound hardware */
	MCFG_DECOBSMT_ADD(DECOBSMT_TAG)

	MCFG_DECODMD_TYPE3_ADD("decodmd",":cpu3")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( detest, de_3b_state )
	/* basic machine hardware */
	MCFG_DECOCPU_TYPE3B_ADD("decocpu",XTAL_8MHz / 2, ":maincpu")

	MCFG_FRAGMENT_ADD( genpin_audio )
MACHINE_CONFIG_END

/*-------------------------------------------------------------
/ Batman Forever 4.0
/------------------------------------------------------------*/
ROM_START(batmanf)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnova.401", 0x0000, 0x10000, CRC(4e62df4e) SHA1(6c3be65fc8825f47cd08755b58fdcf3652ede702))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("bfdrom0a.401", 0x00000000, 0x00080000, CRC(8a3c20ad) SHA1(37415ac7ba178981dffce3a17502f39ab29d90ea))
	ROM_LOAD16_BYTE("bfdrom3a.401", 0x00000001, 0x00080000, CRC(5ef46847) SHA1(a80f241db3d309f0bcb455051e33fc2b74e2ddcd))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(batmanf3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batcpua.302", 0x0000, 0x10000, CRC(5ae7ce69) SHA1(13409c7c993bd9940f3a72f3bac8c8c57a665b3f))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("bmfrom0a.300", 0x00000000, 0x00080000, CRC(764bb217) SHA1(2923d2d2924faa4bdc6e67087fb8ce694d27809a))
	ROM_LOAD16_BYTE("bmfrom3a.300", 0x00000001, 0x00080000, CRC(b4e3b515) SHA1(0f8bf08bc480eed575da54bfc0135f38a86302d4))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_uk)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnove.401", 0x0000, 0x10000, CRC(80f6e4af) SHA1(dd233d2150dcb50b74a70e6ff89c74a3f0d8fae1))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("bfdrom0a.401", 0x00000000, 0x00080000, CRC(8a3c20ad) SHA1(37415ac7ba178981dffce3a17502f39ab29d90ea))
	ROM_LOAD16_BYTE("bfdrom3a.401", 0x00000001, 0x00080000, CRC(5ef46847) SHA1(a80f241db3d309f0bcb455051e33fc2b74e2ddcd))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_cn)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovc.401", 0x0000, 0x10000, CRC(99936537) SHA1(08ff9c6a1fcb3f198190d24bbc75ea1178427fda))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("bfdrom0a.401", 0x00000000, 0x00080000, CRC(8a3c20ad) SHA1(37415ac7ba178981dffce3a17502f39ab29d90ea))
	ROM_LOAD16_BYTE("bfdrom3a.401", 0x00000001, 0x00080000, CRC(5ef46847) SHA1(a80f241db3d309f0bcb455051e33fc2b74e2ddcd))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_no)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovn.401", 0x0000, 0x10000, CRC(79dd48b4) SHA1(eefdf423f9638e293e51bd31413de898ec4eb83a))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("bfdrom0a.401", 0x00000000, 0x00080000, CRC(8a3c20ad) SHA1(37415ac7ba178981dffce3a17502f39ab29d90ea))
	ROM_LOAD16_BYTE("bfdrom3a.401", 0x00000001, 0x00080000, CRC(5ef46847) SHA1(a80f241db3d309f0bcb455051e33fc2b74e2ddcd))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_sv)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovt.401", 0x0000, 0x10000, CRC(854029ab) SHA1(044c2fff6f3e8995c48344f727c1cd9079f7e232))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("bfdrom0a.401", 0x00000000, 0x00080000, CRC(8a3c20ad) SHA1(37415ac7ba178981dffce3a17502f39ab29d90ea))
	ROM_LOAD16_BYTE("bfdrom3a.401", 0x00000001, 0x00080000, CRC(5ef46847) SHA1(a80f241db3d309f0bcb455051e33fc2b74e2ddcd))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_at)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovh.401", 0x0000, 0x10000, CRC(acba13d7) SHA1(b5e5dc5ffc926612ea3d592b6d4e8e02f6290bc7))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("bfdrom0g.401", 0x00000000, 0x00080000, CRC(3a2d7d53) SHA1(340107290d58bfb8b9a6613215eb556626fe2461))
	ROM_LOAD16_BYTE("bfdrom3g.401", 0x00000001, 0x00080000, CRC(94e424f1) SHA1(3a6daf9cbd38e21e2c6447ff1fb0e86b4c03f971))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_ch)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovs.401", 0x0000, 0x10000, CRC(4999d5f9) SHA1(61a9220da38e05360a9496504fa7b11aff14515d))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("bfdrom0g.401", 0x00000000, 0x00080000, CRC(3a2d7d53) SHA1(340107290d58bfb8b9a6613215eb556626fe2461))
	ROM_LOAD16_BYTE("bfdrom3g.401", 0x00000001, 0x00080000, CRC(94e424f1) SHA1(3a6daf9cbd38e21e2c6447ff1fb0e86b4c03f971))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_de)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovg.401", 0x0000, 0x10000, CRC(dd37e99a) SHA1(7949ed43df38849d927f6ed0afa8c3f77cd74b6a))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("bfdrom0g.401", 0x00000000, 0x00080000, CRC(3a2d7d53) SHA1(340107290d58bfb8b9a6613215eb556626fe2461))
	ROM_LOAD16_BYTE("bfdrom3g.401", 0x00000001, 0x00080000, CRC(94e424f1) SHA1(3a6daf9cbd38e21e2c6447ff1fb0e86b4c03f971))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_be)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovb.401", 0x0000, 0x10000, CRC(21309873) SHA1(cebd0c5c05dc5c0a2eb8563ad5c4759f78d6a4b9))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("bfdrom0f.401", 0x00000000, 0x00080000, CRC(e7473f6f) SHA1(f5951a9b6a8776073adf10e38b9d68d6d444240a))
	ROM_LOAD16_BYTE("bfdrom3f.401", 0x00000001, 0x00080000, CRC(f7951709) SHA1(ace5b374d1e382d6f612b2bafc0e9fdde9e21014))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_fr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovf.401", 0x0000, 0x10000, CRC(4baa793d) SHA1(4ba258d11f1bd7a2078ae6cd823a11e10ca96627))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("bfdrom0f.401", 0x00000000, 0x00080000, CRC(e7473f6f) SHA1(f5951a9b6a8776073adf10e38b9d68d6d444240a))
	ROM_LOAD16_BYTE("bfdrom3f.401", 0x00000001, 0x00080000, CRC(f7951709) SHA1(ace5b374d1e382d6f612b2bafc0e9fdde9e21014))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_nl)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovd.401", 0x0000, 0x10000, CRC(6ae4570c) SHA1(e863d6d0963910a993f2a0b8ddeefba48d304ca6))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("bfdrom0f.401", 0x00000000, 0x00080000, CRC(e7473f6f) SHA1(f5951a9b6a8776073adf10e38b9d68d6d444240a))
	ROM_LOAD16_BYTE("bfdrom3f.401", 0x00000001, 0x00080000, CRC(f7951709) SHA1(ace5b374d1e382d6f612b2bafc0e9fdde9e21014))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_it)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovi.401", 0x0000, 0x10000, CRC(7053ef9e) SHA1(918ab3e250b5965998ca0a38e1b8ba3cc012083f))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("bfdrom0i.401", 0x00000000, 0x00080000, CRC(23051253) SHA1(155669a3fecd6e67838b10e71a57a6b871c8762a))
	ROM_LOAD16_BYTE("bfdrom3i.401", 0x00000001, 0x00080000, CRC(82b61a41) SHA1(818c8fdbf44e29fe0ec5362a34ac948e98002efa))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_sp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnova.401", 0x0000, 0x10000, CRC(4e62df4e) SHA1(6c3be65fc8825f47cd08755b58fdcf3652ede702))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("bfdrom0l.401", 0x00000000, 0x00080000, CRC(b22b10d9) SHA1(c8f5637b00b0701d47a3b6bc0fdae08ae1a8df64))
	ROM_LOAD16_BYTE("bfdrom3l.401", 0x00000001, 0x00080000, CRC(016b8666) SHA1(c10b7fc2c1e5b8382ff5b021a6b70f3a550b190e))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_jp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovj.401", 0x0000, 0x10000, CRC(eef9bef0) SHA1(ac37ae12673351be939a969ecbc5b68c3995dca0))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("bfdrom0a.401", 0x00000000, 0x00080000, CRC(8a3c20ad) SHA1(37415ac7ba178981dffce3a17502f39ab29d90ea))
	ROM_LOAD16_BYTE("bfdrom3a.401", 0x00000001, 0x00080000, CRC(5ef46847) SHA1(a80f241db3d309f0bcb455051e33fc2b74e2ddcd))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_time)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnova.401", 0x0000, 0x10000, CRC(4e62df4e) SHA1(6c3be65fc8825f47cd08755b58fdcf3652ede702))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("bfdrom0t.401", 0x00000000, 0x00080000, CRC(b83b8d28) SHA1(b90e6a6fa55dadbf0e752745b87d1e8e9d7ccfa7))
	ROM_LOAD16_BYTE("bfdrom3t.401", 0x00000001, 0x00080000, CRC(a024b1a5) SHA1(2fc8697fa98b7de7a844ca4d6a162b96cc751447))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

/*-------------------------------------------------------------
/ Baywatch - CPU Rev 3b /DMD  Type 3 2x512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(baywatch)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("baycpua.400", 0x0000, 0x10000, CRC(89facfda) SHA1(71720b1da227752b0e276390abd08c742bca9090))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("bayrom0a.400", 0x00000000, 0x00080000, CRC(43d615c6) SHA1(7c843b6d5215305b02a55c9fa1d62375ef0766ea))
	ROM_LOAD16_BYTE("bayrom3a.400", 0x00000001, 0x00080000, CRC(41bcb66b) SHA1(e6f0a9236e14c2e919881ca1ffe3356aaa121730))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("bayw.u7", 0x0000, 0x10000, CRC(90d6d8a8) SHA1(482c5643453f21a078257aa13398845ef19cab3c))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("bayw.u17", 0x000000, 0x80000, CRC(b20fde56) SHA1(2f2db49245e4a6a8251cbe896b2437fcec88d42d))
	ROM_LOAD("bayw.u21", 0x080000, 0x80000, CRC(b7598881) SHA1(19d1dde1cb6634a7c7b5cdb4fa01cd09cc7d7777))
ROM_END

ROM_START(bay_e400)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("baycpua2.400", 0x0000, 0x10000, CRC(07b77fe2) SHA1(4f81a5b3d821907e06d6b547117ad39c238a900c))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("bayrom0a.400", 0x00000000, 0x00080000, CRC(43d615c6) SHA1(7c843b6d5215305b02a55c9fa1d62375ef0766ea))
	ROM_LOAD16_BYTE("bayrom3a.400", 0x00000001, 0x00080000, CRC(41bcb66b) SHA1(e6f0a9236e14c2e919881ca1ffe3356aaa121730))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("bw-u7.u7", 0x0000, 0x10000, CRC(a5e57557) SHA1(a884c1118331b8724507b0a916127ce5df309fe4))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("bw-u17.bin", 0x000000, 0x80000, CRC(660e7f5d) SHA1(6dde294e728e596a6c455326793b65254139620e))
	ROM_LOAD("bw-u21.bin", 0x080000, 0x80000, CRC(5ec3a889) SHA1(f355f742de137344e6e4b5d3a4b2380a876c8cc3))
	ROM_LOAD("bw-u36.bin", 0x100000, 0x80000, CRC(1877abc5) SHA1(13ca231a486495a83cc1d9c6dde558a57eb4abe1))
ROM_END

/*-------------------------------------------------------------
/ Mary Shelley's Frankenstein - CPU Rev 3b /DMD  Type 3 2x512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(frankst)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("franka.103", 0x0000, 0x10000, CRC(a9aba9be) SHA1(1cc22fcbc0f51a17037637c04e606579956c9cba))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("frdspr0a.103", 0x00000000, 0x00080000, CRC(9dd09c7d) SHA1(c5668e53d6c914667a59538f82222ec2efc6f187))
	ROM_LOAD16_BYTE("frdspr3a.103", 0x00000001, 0x00080000, CRC(73b538bb) SHA1(07d7ae21f062d15711d72af03bfcd52608f75a5f))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("frsnd.u7", 0x0000, 0x10000, CRC(084f856c) SHA1(c91331a32b565c2ed3f96156f44143dc22009e8e))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("frsnd.u17", 0x000000, 0x80000, CRC(0da904d6) SHA1(e190f1a35147b2f39224832969ca7b1d4a30f6cc))
	ROM_LOAD("frsnd.u21", 0x080000, 0x80000, CRC(14d4bc12) SHA1(9e7005c5bd0afe7f9c9215b39878496640cdea77))
	ROM_LOAD("frsnd.u36", 0x100000, 0x80000, CRC(9964d721) SHA1(5ea0bc051d1909bee80d3feb6b7350b6307b6dcb))
ROM_END

ROM_START(frankstg)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("franka.103", 0x0000, 0x10000, CRC(a9aba9be) SHA1(1cc22fcbc0f51a17037637c04e606579956c9cba))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("frdspr0g.101", 0x00000000, 0x00080000, CRC(5e27ec02) SHA1(351d6f1b7d72e415f2bf5780b6533dbd67579261))
	ROM_LOAD16_BYTE("frdspr3g.101", 0x00000001, 0x00080000, CRC(d6c607b5) SHA1(876d4bd2a5b89f1a28ff7cd45494c7245f147d27))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("frsnd.u7", 0x0000, 0x10000, CRC(084f856c) SHA1(c91331a32b565c2ed3f96156f44143dc22009e8e))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("frsnd.u17", 0x000000, 0x80000, CRC(0da904d6) SHA1(e190f1a35147b2f39224832969ca7b1d4a30f6cc))
	ROM_LOAD("frsnd.u21", 0x080000, 0x80000, CRC(14d4bc12) SHA1(9e7005c5bd0afe7f9c9215b39878496640cdea77))
	ROM_LOAD("frsnd.u36", 0x100000, 0x80000, CRC(9964d721) SHA1(5ea0bc051d1909bee80d3feb6b7350b6307b6dcb))
ROM_END

/*-------------------------------------------------------------
/ Maverick - CPU Rev 3b /DMD  Type 3 2x512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(mav_402)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mavcpua.404", 0x0000, 0x10000, CRC(9f06bd8d) SHA1(3b931af5455ed9c40f2b6c884427a326bba8f75a))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("mavdisp0.402", 0x00000000, 0x00080000, CRC(4e643525) SHA1(30b91c91c2f1295cdd018023c5ac783570a0aeea))
	ROM_LOAD16_BYTE("mavdisp3.402", 0x00000001, 0x00080000, CRC(8c5f9460) SHA1(6369b4c98ec6fd5e769275b44631b2b6dd5c411b))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("mavu7.dat", 0x0000, 0x10000, CRC(427e6ab9) SHA1(6ad9295097f3d498383c91adf4ca667f797f29b1))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("mavu17.dat", 0x000000, 0x80000, CRC(cba377b8) SHA1(b7551b6cb64357cdacf1a96cedfccbabf4bd070a))
	ROM_LOAD("mavu21.dat", 0x080000, 0x80000, CRC(be0c6a6f) SHA1(4fee912d9f0d4b196dbfacf06a4202b2fa3037b1))
ROM_END

ROM_START(mav_401)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mavcpua.404", 0x0000, 0x10000, CRC(9f06bd8d) SHA1(3b931af5455ed9c40f2b6c884427a326bba8f75a))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("mavdsar0.401", 0x00000000, 0x00080000, CRC(35b811af) SHA1(1e235a0f16ef0eecca5b6ec7a2234ed1dc4e4440))
	ROM_LOAD16_BYTE("mavdsar3.401", 0x00000001, 0x00080000, CRC(c4c126ae) SHA1(b4841e83ec075bddc919217b65afaac97709e69b))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("mavu7.dat", 0x0000, 0x10000, CRC(427e6ab9) SHA1(6ad9295097f3d498383c91adf4ca667f797f29b1))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("mavu17.dat", 0x000000, 0x80000, CRC(cba377b8) SHA1(b7551b6cb64357cdacf1a96cedfccbabf4bd070a))
	ROM_LOAD("mavu21.dat", 0x080000, 0x80000, CRC(be0c6a6f) SHA1(4fee912d9f0d4b196dbfacf06a4202b2fa3037b1))
ROM_END

ROM_START(mav_400)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mavgc5.400", 0x0000, 0x10000, CRC(e2d0a88b) SHA1(d1571edba47aecc871ac0cfdaabca31774f70fa1))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("mavdisp0.400", 0x00000000, 0x00080000, CRC(b6069484) SHA1(2878d9a0151194bd4a0e12e2f75b02a5d7316b68))
	ROM_LOAD16_BYTE("mavdisp3.400", 0x00000001, 0x00080000, CRC(149f871f) SHA1(e29a8bf149b77bccaeed202786cf76d9a4fd51df))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("mavu7.dat", 0x0000, 0x10000, CRC(427e6ab9) SHA1(6ad9295097f3d498383c91adf4ca667f797f29b1))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("mavu17.dat", 0x000000, 0x80000, CRC(cba377b8) SHA1(b7551b6cb64357cdacf1a96cedfccbabf4bd070a))
	ROM_LOAD("mavu21.dat", 0x080000, 0x80000, CRC(be0c6a6f) SHA1(4fee912d9f0d4b196dbfacf06a4202b2fa3037b1))
ROM_END

ROM_START(mav_100)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mavcpu.100", 0x0000, 0x10000, CRC(13fdc959) SHA1(f8155f0fe5d4c3fe55000ab3b57f298fd9229fef))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("mavdsp0.100", 0x00000000, 0x00080000, CRC(3e01f5c8) SHA1(8e40f399c77aa17bebbefe04742ff2ff95508323))
	ROM_LOAD16_BYTE("mavdsp3.100", 0x00000001, 0x00080000, CRC(e2b623f2) SHA1(7b5a6d0db30f3deedb8fe0e1731c81ec836a66f5))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("mavu7.dat", 0x0000, 0x10000, CRC(427e6ab9) SHA1(6ad9295097f3d498383c91adf4ca667f797f29b1))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("mavu17.dat", 0x000000, 0x80000, CRC(cba377b8) SHA1(b7551b6cb64357cdacf1a96cedfccbabf4bd070a))
	ROM_LOAD("mavu21.dat", 0x080000, 0x80000, CRC(be0c6a6f) SHA1(4fee912d9f0d4b196dbfacf06a4202b2fa3037b1))
ROM_END

/*-------------------------------------------------------------
/ Cut The Cheese (Redemption, Data East hardware)
/------------------------------------------------------------*/
ROM_START(ctcheese)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ctcc5.bin", 0x0000, 0x10000, CRC(465d41de) SHA1(0e30b527d5b47f8823cbe6f196052b090e69e907))
	ROM_REGION(0x01000000, "cpu3", 0)
	ROM_LOAD16_BYTE("ctcdsp0.bin", 0x00000000, 0x00080000, CRC(6885734d) SHA1(9ac82c9c8bf4e66d2999fbfd08617ef6c266dfe8))
	ROM_LOAD16_BYTE("ctcdsp3.bin", 0x00000001, 0x00080000, CRC(0c2b3f3c) SHA1(cb730cc6fdd2a2786d25b46b1c45466ee56132d1))
	ROM_REGION(0x010000, "soundcpu", 0)
	ROM_LOAD("ctcu7.bin", 0x0000, 0x10000, CRC(406b9b9e) SHA1(f3f86c368c92ee0cb47323e6e0ca0fa05b6122bd))
	ROM_REGION(0x1000000, "bsmt", 0)
	ROM_LOAD("ctcu17.bin", 0x000000, 0x80000, CRC(ea125fb3) SHA1(2bc1d2a6138ff77ad19b7bcff784dba73f545883))
	ROM_LOAD("ctcu21.bin", 0x080000, 0x80000, CRC(1b3af383) SHA1(c6b57f3f0781954f75d164d909093e4ed8da440e))
ROM_END

/*-------------------------------------------------------------
/ Roach Racers
/------------------------------------------------------------*/


/*-------------------------------------------------------------
/ Data East Test Chip 64K ROM
/------------------------------------------------------------*/
ROM_START(detest)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("de_test.512", 0x0000, 0x10000, CRC(bade8ca8) SHA1(e7e9d6622b9c9b9381ba2793297f87f102214972))

	ROM_REGION(0x01000000, "cpu3", ROMREGION_ERASE00)
	ROM_REGION(0x010000, "soundcpu", ROMREGION_ERASE00)
	ROM_REGION(0x1000000, "bsmt", ROMREGION_ERASE00)
ROM_END


GAME(1995,  batmanf,    0,              de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Batman Forever (4.0)",                 MACHINE_IS_SKELETON_MECHANICAL)
GAME(1995,  batmanf3,   batmanf,        de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Batman Forever (3.0)",                 MACHINE_IS_SKELETON_MECHANICAL)
GAME(1995,  bmf_uk,     batmanf,        de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Batman Forever (English)",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1995,  bmf_cn,     batmanf,        de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Batman Forever (Canadian)",            MACHINE_IS_SKELETON_MECHANICAL)
GAME(1995,  bmf_no,     batmanf,        de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Batman Forever (Norwegian)",           MACHINE_IS_SKELETON_MECHANICAL)
GAME(1995,  bmf_sv,     batmanf,        de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Batman Forever (Swedish)",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1995,  bmf_at,     batmanf,        de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Batman Forever (Austrian)",            MACHINE_IS_SKELETON_MECHANICAL)
GAME(1995,  bmf_ch,     batmanf,        de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Batman Forever (Swiss)",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1995,  bmf_de,     batmanf,        de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Batman Forever (German)",              MACHINE_IS_SKELETON_MECHANICAL)
GAME(1995,  bmf_be,     batmanf,        de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Batman Forever (Belgian)",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1995,  bmf_fr,     batmanf,        de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Batman Forever (French)",              MACHINE_IS_SKELETON_MECHANICAL)
GAME(1995,  bmf_nl,     batmanf,        de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Batman Forever (Dutch)",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1995,  bmf_it,     batmanf,        de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Batman Forever (Italian)",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1995,  bmf_sp,     batmanf,        de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Batman Forever (Spanish)",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1995,  bmf_jp,     batmanf,        de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Batman Forever (Japanese)",            MACHINE_IS_SKELETON_MECHANICAL)
GAME(1995,  bmf_time,   batmanf,        de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Batman Forever (Timed Play)",          MACHINE_IS_SKELETON_MECHANICAL)
GAME(1995,  baywatch,   0,              de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Baywatch",                             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1995,  bay_e400,   baywatch,       de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Baywatch (England)",                   MACHINE_IS_SKELETON_MECHANICAL)
GAME(1994,  frankst,    0,              de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Mary Shelley's Frankenstein",          MACHINE_IS_SKELETON_MECHANICAL)
GAME(1995,  frankstg,   frankst,        de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Mary Shelley's Frankenstein (Germany)",MACHINE_IS_SKELETON_MECHANICAL)
GAME(1994,  mav_402,    0,              de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Maverick (Display Rev. 4.02)",         MACHINE_IS_SKELETON_MECHANICAL)
GAME(1994,  mav_401,    mav_402,        de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Maverick (Display Rev. 4.01)",         MACHINE_IS_SKELETON_MECHANICAL)
GAME(1994,  mav_400,    mav_402,        de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Maverick (Display Rev. 4.00)",         MACHINE_IS_SKELETON_MECHANICAL)
GAME(1994,  mav_100,    mav_402,        de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Maverick (1.00)",                      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1998,  detest,     0,              detest, de_3b, de_3b_state, de_3b,  ROT0,   "Data East",        "Data East Test Chip",                  MACHINE_IS_SKELETON_MECHANICAL)
GAME(1996,  ctcheese,   0,              de_3b,  de_3b, de_3b_state, de_3b,  ROT0,   "Sega",             "Cut The Cheese (Redemption)",          MACHINE_IS_SKELETON_MECHANICAL)
