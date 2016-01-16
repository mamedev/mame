// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Miodrag Milanovic
/*
    Williams WPC (Alpha Numeric)
*/


#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "audio/s11c_bg.h"
#include "audio/wpcsnd.h"
#include "machine/wpc.h"
#include "wpc_an.lh"

#define LOG_WPC (1)

class wpc_an_state : public driver_device
{
public:
	wpc_an_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_bg(*this,"bg"),
			m_wpcsnd(*this,"wpcsnd"),
			m_cpubank(*this, "cpubank"),
			m_wpc(*this,"wpc")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;
	optional_device<s11c_bg_device> m_bg;  // only used with Dr. Dude
	optional_device<wpcsnd_device> m_wpcsnd;
	required_memory_bank m_cpubank;
	required_device<wpc_device> m_wpc;

	// driver_device overrides
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	static const device_timer_id TIMER_VBLANK = 0;
	static const device_timer_id TIMER_IRQ = 1;
public:
	DECLARE_DRIVER_INIT(wpc_an);
	DECLARE_READ8_MEMBER(ram_r);
	DECLARE_WRITE8_MEMBER(ram_w);
	DECLARE_WRITE_LINE_MEMBER(wpcsnd_reply_w);
	DECLARE_WRITE_LINE_MEMBER(wpc_irq_w);
	DECLARE_WRITE_LINE_MEMBER(wpc_firq_w);
	DECLARE_READ8_MEMBER(wpc_sound_ctrl_r);
	DECLARE_WRITE8_MEMBER(wpc_sound_ctrl_w);
	DECLARE_READ8_MEMBER(wpc_sound_data_r);
	DECLARE_WRITE8_MEMBER(wpc_sound_data_w);
	DECLARE_WRITE8_MEMBER(wpc_sound_s11_w);
	DECLARE_WRITE8_MEMBER(wpc_rombank_w);
private:
	UINT16 m_vblank_count;
	UINT32 m_irq_count;
	UINT8 m_bankmask;
	UINT8 m_ram[0x3000];
	emu_timer* m_vblank_timer;
	emu_timer* m_irq_timer;
};


static ADDRESS_MAP_START( wpc_an_map, AS_PROGRAM, 8, wpc_an_state )
	AM_RANGE(0x0000, 0x2fff) AM_READWRITE(ram_r,ram_w)
	AM_RANGE(0x3000, 0x3faf) AM_RAM
	AM_RANGE(0x3fb0, 0x3fff) AM_DEVREADWRITE("wpc",wpc_device,read,write) // WPC device
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("cpubank")
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION("fixed",0)
ADDRESS_MAP_END

static INPUT_PORTS_START( wpc_an )
	PORT_START("INP0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INP1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )  // left flipper
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )  // right flipper
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )

	PORT_START("INP2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  // always closed
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

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service / Escape") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_VOLUME_UP ) PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Begin Test / Enter") PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("DIPS")
	PORT_DIPNAME(0x01,0x01,"Switch 1") PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(0x00,DEF_STR( Off ))
	PORT_DIPSETTING(0x01,DEF_STR( On ))
	PORT_DIPNAME(0x02,0x02,"Switch 2") PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(0x00,DEF_STR( Off ))
	PORT_DIPSETTING(0x02,DEF_STR( On ))
	PORT_DIPNAME(0x04,0x00,"W20") PORT_DIPLOCATION("SWA:3")
	PORT_DIPSETTING(0x00,DEF_STR( Off ))
	PORT_DIPSETTING(0x04,DEF_STR( On ))
	PORT_DIPNAME(0x08,0x00,"W19") PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(0x00,DEF_STR( Off ))
	PORT_DIPSETTING(0x08,DEF_STR( On ))
	PORT_DIPNAME(0xf0,0x00,"Country") PORT_DIPLOCATION("SWA:5,6,7,8")
	PORT_DIPSETTING(0x00,"USA 1")
	PORT_DIPSETTING(0x10,"France 1")
	PORT_DIPSETTING(0x20,"Germany")
	PORT_DIPSETTING(0x30,"France 2")
	PORT_DIPSETTING(0x40,"Unknown 1")
	PORT_DIPSETTING(0x50,"Unknown 2")
	PORT_DIPSETTING(0x60,"Unknown 3")
	PORT_DIPSETTING(0x70,"Unknown 4")
	PORT_DIPSETTING(0x80,"Export 1")
	PORT_DIPSETTING(0x90,"France 3")
	PORT_DIPSETTING(0xa0,"Export 2")
	PORT_DIPSETTING(0xb0,"France 4")
	PORT_DIPSETTING(0xc0,"UK")
	PORT_DIPSETTING(0xd0,"Europe")
	PORT_DIPSETTING(0xe0,"Spain")
	PORT_DIPSETTING(0xf0,"USA 2")

INPUT_PORTS_END


void wpc_an_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int x;
	switch(id)
	{
	case TIMER_VBLANK:
		// update LED segments
		for(x=0;x<16;x++)
		{
			output().set_digit_value(x,BITSWAP16(m_wpc->get_alphanumeric(x), 15, 7, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0));
			output().set_digit_value(x+16,BITSWAP16(m_wpc->get_alphanumeric(20+x), 15, 7, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0));
		}
		m_wpc->reset_alphanumeric();
		m_vblank_count++;
		break;
	case TIMER_IRQ:
		m_maincpu->set_input_line(M6809_IRQ_LINE,ASSERT_LINE);
		break;
	}
}

WRITE8_MEMBER(wpc_an_state::wpc_rombank_w)
{
	m_cpubank->set_entry(data & m_bankmask);
}

WRITE_LINE_MEMBER(wpc_an_state::wpcsnd_reply_w)
{
	if(state)
		m_maincpu->set_input_line(M6809_FIRQ_LINE,ASSERT_LINE);
}

WRITE_LINE_MEMBER(wpc_an_state::wpc_irq_w)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE,CLEAR_LINE);
}

WRITE_LINE_MEMBER(wpc_an_state::wpc_firq_w)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE,CLEAR_LINE);
}

READ8_MEMBER(wpc_an_state::wpc_sound_ctrl_r)
{
	if(m_wpcsnd)
		return m_wpcsnd->ctrl_r();  // ack FIRQ?
	return 0;
}

WRITE8_MEMBER(wpc_an_state::wpc_sound_ctrl_w)
{
	if(m_bg)
	{
		m_bg->data_w(data);
		m_bg->ctrl_w(1);
	}
	else
		m_wpcsnd->ctrl_w(data);
}

READ8_MEMBER(wpc_an_state::wpc_sound_data_r)
{
	if(m_wpcsnd)
		return m_wpcsnd->data_r();
	return 0;
}

WRITE8_MEMBER(wpc_an_state::wpc_sound_data_w)
{
	if(m_bg)
	{
		m_bg->data_w(data);
		m_bg->ctrl_w(0);
	}
	else
		m_wpcsnd->data_w(data);
}

WRITE8_MEMBER(wpc_an_state::wpc_sound_s11_w)
{
	if(m_bg)
	{
		m_bg->data_w(data);
		m_bg->ctrl_w(0);
		m_bg->ctrl_w(1);
	}
}

READ8_MEMBER(wpc_an_state::ram_r)
{
	return m_ram[offset];
}

WRITE8_MEMBER(wpc_an_state::ram_w)
{
	if((!m_wpc->memprotect_active()) || ((offset & m_wpc->get_memprotect_mask()) != m_wpc->get_memprotect_mask()))
		m_ram[offset] = data;
	else
		if(LOG_WPC) logerror("WPC: Memory protection violation at 0x%04x (mask=0x%04x)\n",offset,m_wpc->get_memprotect_mask());
}

void wpc_an_state::machine_reset()
{
	m_cpubank->set_entry(0);
	m_vblank_count = 0;
	m_irq_count = 0;
}

DRIVER_INIT_MEMBER(wpc_an_state,wpc_an)
{
	UINT8 *ROM = memregion("maincpu")->base();
	UINT8 *fixed = memregion("fixed")->base();
	UINT32 codeoff = memregion("maincpu")->bytes() - 0x8000;
	m_cpubank->configure_entries(0, 32, &ROM[0x10000], 0x4000);
	m_cpubank->set_entry(0);
	m_vblank_timer = timer_alloc(TIMER_VBLANK);
	m_vblank_timer->adjust(attotime::from_hz(60),0,attotime::from_hz(60));
	m_irq_timer = timer_alloc(TIMER_IRQ);
	m_irq_timer->adjust(attotime::from_hz(976),0,attotime::from_hz(976));
	m_bankmask = ((memregion("maincpu")->bytes()-0x10000) >> 14) - 1;
	logerror("WPC: ROM bank mask = %02x\n",m_bankmask);
	memset(m_ram,0,0x3000);
	memcpy(fixed,&ROM[codeoff],0x8000);  // copy static code from end of U6 ROM.
}

static MACHINE_CONFIG_FRAGMENT( wpc_an_base )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, 2000000)
	MCFG_CPU_PROGRAM_MAP(wpc_an_map)

	MCFG_WMS_WPC_ADD("wpc")
	MCFG_WPC_IRQ_ACKNOWLEDGE(WRITELINE(wpc_an_state,wpc_irq_w))
	MCFG_WPC_FIRQ_ACKNOWLEDGE(WRITELINE(wpc_an_state,wpc_firq_w))
	MCFG_WPC_ROMBANK(WRITE8(wpc_an_state,wpc_rombank_w))
	MCFG_WPC_SOUND_CTRL(READ8(wpc_an_state,wpc_sound_ctrl_r),WRITE8(wpc_an_state,wpc_sound_ctrl_w))
	MCFG_WPC_SOUND_DATA(READ8(wpc_an_state,wpc_sound_data_r),WRITE8(wpc_an_state,wpc_sound_data_w))
	MCFG_WPC_SOUND_S11C(WRITE8(wpc_an_state,wpc_sound_s11_w))

	MCFG_DEFAULT_LAYOUT(layout_wpc_an)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( wpc_an, wpc_an_state )
	MCFG_FRAGMENT_ADD(wpc_an_base)
	MCFG_WMS_WPC_SOUND_ADD("wpcsnd",":sound1")
	MCFG_WPC_SOUND_REPLY_CALLBACK(WRITELINE(wpc_an_state,wpcsnd_reply_w))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( wpc_an_dd, wpc_an_state )
	MCFG_FRAGMENT_ADD(wpc_an_base)
	MCFG_WMS_S11C_BG_ADD("bg",":sound1")
MACHINE_CONFIG_END

/*-----------------
/  Dr. Dude #2016
/------------------*/
ROM_START(dd_p7)
	ROM_REGION(0x30000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("dude_u6.p7", 0x10000, 0x20000, CRC(b6c35b98) SHA1(5e9d70ce40669e2f402561dc1d8aa70a8b8a2958))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("dude_u4.l1", 0x10000, 0x10000, CRC(3eeef714) SHA1(74dcc83958cb62819e0ac36ca83001694faafec7))
	ROM_LOAD("dude_u19.l1", 0x20000, 0x10000, CRC(dc7b985b) SHA1(f672d1f1fe1d1d887113ea6ccd745a78f7760526))
	ROM_LOAD("dude_u20.l1", 0x30000, 0x10000, CRC(a83d53dd) SHA1(92a81069c42c7760888201fb0787fa7ddfbf1658))
	ROM_REGION(0x8000, "fixed", 0)
	ROM_FILL(0x0000,0x8000,nullptr)
ROM_END

ROM_START(dd_p06)
	ROM_REGION(0x30000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("u6-pa6.wpc", 0x10000, 0x20000, CRC(fb72571b) SHA1(a12b32eac3141c881064e6de2f49d6d213248fde))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("dude_u4.l1", 0x10000, 0x10000, CRC(3eeef714) SHA1(74dcc83958cb62819e0ac36ca83001694faafec7))
	ROM_LOAD("dude_u19.l1", 0x20000, 0x10000, CRC(dc7b985b) SHA1(f672d1f1fe1d1d887113ea6ccd745a78f7760526))
	ROM_LOAD("dude_u20.l1", 0x30000, 0x10000, CRC(a83d53dd) SHA1(92a81069c42c7760888201fb0787fa7ddfbf1658))
	ROM_REGION(0x8000, "fixed", 0)
	ROM_FILL(0x0000,0x8000,nullptr)
ROM_END

/*-------------
/ Funhouse #50003
/--------------*/
ROM_START(fh_l9)
	ROM_REGION(0x50000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("funh_l9.rom", 0x10000, 0x40000, CRC(c8f90ff8) SHA1(8d200ea30a68f5e3ba1ac9232a516c44b765eb45))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("fh_u14.sl2", 0x000000, 0x20000, CRC(3394b69b) SHA1(34690688f00106b725b27a6975cdbf1e077e3bb3))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("fh_u15.sl2", 0x080000, 0x20000, CRC(0744b9f5) SHA1(b626601d82e6b1cf25f7fdcca31e623fc14a3f92))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("fh_u18.sl3", 0x100000, 0x20000, CRC(7f6c7045) SHA1(8c8d601e8e6598507d75b4955ccc51623124e8ab))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
	ROM_REGION(0x8000, "fixed", 0)
	ROM_FILL(0x0000,0x8000,nullptr)
ROM_END

ROM_START(fh_l9b)
	ROM_REGION(0x50000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("fh_l9ger.rom", 0x10000, 0x40000, CRC(e9b32a8f) SHA1(deb77f0d025001ddcc3045b4e49176c54896da3f))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("fh_u14.sl2", 0x000000, 0x20000, CRC(3394b69b) SHA1(34690688f00106b725b27a6975cdbf1e077e3bb3))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("fh_u15.sl2", 0x080000, 0x20000, CRC(0744b9f5) SHA1(b626601d82e6b1cf25f7fdcca31e623fc14a3f92))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("fh_u18.sl3", 0x100000, 0x20000, CRC(7f6c7045) SHA1(8c8d601e8e6598507d75b4955ccc51623124e8ab))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
	ROM_REGION(0x8000, "fixed", 0)
	ROM_FILL(0x0000,0x8000,nullptr)
ROM_END

ROM_START(fh_l3)
	ROM_REGION(0x30000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("u6-l3.rom", 0x10000, 0x20000, CRC(7a74d702) SHA1(91540cdc62c855b4139b202aa6ad5440b2dee141))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("fh_u14.sl2", 0x000000, 0x20000, CRC(3394b69b) SHA1(34690688f00106b725b27a6975cdbf1e077e3bb3))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("fh_u15.sl2", 0x080000, 0x20000, CRC(0744b9f5) SHA1(b626601d82e6b1cf25f7fdcca31e623fc14a3f92))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("fh_u18.sl2", 0x100000, 0x20000, CRC(11c8944a) SHA1(425d8da5a036c41e054d201b99856319fd5ef9e2))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
	ROM_REGION(0x8000, "fixed", 0)
	ROM_FILL(0x0000,0x8000,nullptr)
ROM_END

ROM_START(fh_l4)
	ROM_REGION(0x30000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("u6-l4.rom", 0x10000, 0x20000, CRC(f438aaca) SHA1(42bf75325a0e85a4334a5a710c2eddf99160ffbf))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("fh_u14.sl2", 0x000000, 0x20000, CRC(3394b69b) SHA1(34690688f00106b725b27a6975cdbf1e077e3bb3))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("fh_u15.sl2", 0x080000, 0x20000, CRC(0744b9f5) SHA1(b626601d82e6b1cf25f7fdcca31e623fc14a3f92))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("fh_u18.sl2", 0x100000, 0x20000, CRC(11c8944a) SHA1(425d8da5a036c41e054d201b99856319fd5ef9e2))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
	ROM_REGION(0x8000, "fixed", 0)
	ROM_FILL(0x0000,0x8000,nullptr)
ROM_END

ROM_START(fh_l5)
	ROM_REGION(0x30000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("u6-l5.rom", 0x10000, 0x20000, CRC(e2b25da4) SHA1(87129e18c60a65035ade2f4766c154d5d333696b))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("fh_u14.sl2", 0x000000, 0x20000, CRC(3394b69b) SHA1(34690688f00106b725b27a6975cdbf1e077e3bb3))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("fh_u15.sl2", 0x080000, 0x20000, CRC(0744b9f5) SHA1(b626601d82e6b1cf25f7fdcca31e623fc14a3f92))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("fh_u18.sl2", 0x100000, 0x20000, CRC(11c8944a) SHA1(425d8da5a036c41e054d201b99856319fd5ef9e2))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
	ROM_REGION(0x8000, "fixed", 0)
	ROM_FILL(0x0000,0x8000,nullptr)
ROM_END

ROM_START(fh_905h)
	ROM_REGION(0x90000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("fh_905h.rom", 0x10000, 0x80000, CRC(445b632a) SHA1(6e277027a1d025e2b93f0d7736b414ba3a68a4f8))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("fh_u14.sl2", 0x000000, 0x20000, CRC(3394b69b) SHA1(34690688f00106b725b27a6975cdbf1e077e3bb3))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("fh_u15.sl2", 0x080000, 0x20000, CRC(0744b9f5) SHA1(b626601d82e6b1cf25f7fdcca31e623fc14a3f92))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("fh_u18.sl3", 0x100000, 0x20000, CRC(7f6c7045) SHA1(8c8d601e8e6598507d75b4955ccc51623124e8ab))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
	ROM_REGION(0x8000, "fixed", 0)
	ROM_FILL(0x0000,0x8000,nullptr)
ROM_END


/*-----------------
/  Harley Davidson #20001
/------------------*/
ROM_START(hd_l3)
	ROM_REGION(0x30000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("harly_l3.rom", 0x10000, 0x20000, CRC(65f2e0b4) SHA1(a44216c13b9f9adf4161ff6f9eeceba28ef37963))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("hd_u18.rom", 0x100000, 0x20000, CRC(810d98c0) SHA1(8080cbbe0f346020b2b2b8e97015dbb615dbadb3))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
	ROM_LOAD("hd_u15.rom", 0x080000, 0x20000, CRC(e7870938) SHA1(b4f28146a5e7baa8522db65b41311afaf49604c6))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_REGION(0x8000, "fixed", 0)
	ROM_FILL(0x0000,0x8000,nullptr)
ROM_END

ROM_START(hd_l1)
	ROM_REGION(0x30000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("u6-l1.rom", 0x10000, 0x20000, CRC(a0bdcfbf) SHA1(f906ffa2d4d04e87225bf711a07dd3bee1655a40))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("u18-sp1.rom", 0x100000, 0x20000, CRC(708aa419) SHA1(cfc2692fb3bcbacceb85021e282bfbc8dcdf8fcc))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
	ROM_LOAD("hd_u15.rom", 0x080000, 0x20000, CRC(e7870938) SHA1(b4f28146a5e7baa8522db65b41311afaf49604c6))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_REGION(0x8000, "fixed", 0)
	ROM_FILL(0x0000,0x8000,nullptr)
ROM_END

/*-----------------
/  The Machine: Bride of Pinbot #50001
/------------------*/
ROM_START(bop_l7)
	ROM_REGION(0x50000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("tmbopl_7.rom", 0x10000, 0x40000, CRC(773e1488) SHA1(36e8957b3903b99844a76bf15ba393b17db0db59))
	ROM_REGION(0x180000, "sound1",0)
	ROM_LOAD("mach_u14.l1", 0x000000, 0x20000, CRC(be2a736a) SHA1(ebf7b26a86d3ffcc35eaa1da8e4f432bd281fe15))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("mach_u15.l1", 0x080000, 0x20000, CRC(fb49513b) SHA1(01f5243ff258adce3a28b24859eba3f465444bdf))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("mach_u18.l1", 0x100000, 0x20000, CRC(f3f53896) SHA1(4be5a8a27c5ac4718713c05ff2ddf51658a1be27))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
	ROM_REGION(0x8000, "fixed", 0)
	ROM_FILL(0x0000,0x8000,nullptr)
ROM_END

ROM_START(bop_l6)
	ROM_REGION(0x30000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("tmbopl_6.rom", 0x10000, 0x20000, CRC(96b844d6) SHA1(981194c249a8fc2534e24ef672380d751a5dc5fd))
	ROM_REGION(0x180000, "sound1",0)
	ROM_LOAD("mach_u14.l1", 0x000000, 0x20000, CRC(be2a736a) SHA1(ebf7b26a86d3ffcc35eaa1da8e4f432bd281fe15))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("mach_u15.l1", 0x080000, 0x20000, CRC(fb49513b) SHA1(01f5243ff258adce3a28b24859eba3f465444bdf))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("mach_u18.l1", 0x100000, 0x20000, CRC(f3f53896) SHA1(4be5a8a27c5ac4718713c05ff2ddf51658a1be27))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
	ROM_REGION(0x8000, "fixed", 0)
	ROM_FILL(0x0000,0x8000,nullptr)
ROM_END

ROM_START(bop_l5)
	ROM_REGION(0x30000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("tmbopl_5.rom", 0x10000, 0x20000, CRC(fd5c426d) SHA1(e006f8e39cf382249db0b969cf966fd8deaa344a))
	ROM_REGION(0x180000, "sound1",0)
	ROM_LOAD("mach_u14.l1", 0x000000, 0x20000, CRC(be2a736a) SHA1(ebf7b26a86d3ffcc35eaa1da8e4f432bd281fe15))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("mach_u15.l1", 0x080000, 0x20000, CRC(fb49513b) SHA1(01f5243ff258adce3a28b24859eba3f465444bdf))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("mach_u18.l1", 0x100000, 0x20000, CRC(f3f53896) SHA1(4be5a8a27c5ac4718713c05ff2ddf51658a1be27))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
	ROM_REGION(0x8000, "fixed", 0)
	ROM_FILL(0x0000,0x8000,nullptr)
ROM_END

ROM_START(bop_l4)
	ROM_REGION(0x30000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("tmbopl_4.rom", 0x10000, 0x20000, CRC(eea14ecd) SHA1(afd670bdc3680f12360561a1a5e5854718c099f7))
	ROM_REGION(0x180000, "sound1",0)
	ROM_LOAD("mach_u14.l1", 0x000000, 0x20000, CRC(be2a736a) SHA1(ebf7b26a86d3ffcc35eaa1da8e4f432bd281fe15))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("mach_u15.l1", 0x080000, 0x20000, CRC(fb49513b) SHA1(01f5243ff258adce3a28b24859eba3f465444bdf))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("mach_u18.l1", 0x100000, 0x20000, CRC(f3f53896) SHA1(4be5a8a27c5ac4718713c05ff2ddf51658a1be27))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
	ROM_REGION(0x8000, "fixed", 0)
	ROM_FILL(0x0000,0x8000,nullptr)
ROM_END

ROM_START(bop_l3)
	ROM_REGION(0x30000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("bop_l3.u6", 0x10000, 0x20000, CRC(cd4d219d) SHA1(4e73dca186867ebee07682deab058a45cee53be1))
	ROM_REGION(0x180000, "sound1",0)
	ROM_LOAD("mach_u14.l1", 0x000000, 0x20000, CRC(be2a736a) SHA1(ebf7b26a86d3ffcc35eaa1da8e4f432bd281fe15))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("mach_u15.l1", 0x080000, 0x20000, CRC(fb49513b) SHA1(01f5243ff258adce3a28b24859eba3f465444bdf))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("mach_u18.l1", 0x100000, 0x20000, CRC(f3f53896) SHA1(4be5a8a27c5ac4718713c05ff2ddf51658a1be27))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
	ROM_REGION(0x8000, "fixed", 0)
	ROM_FILL(0x0000,0x8000,nullptr)
ROM_END

ROM_START(bop_l2)
	ROM_REGION(0x30000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("bop_l2.u6", 0x10000, 0x20000, CRC(17ee1f56) SHA1(bee68ed5680455f23dc33e889acec83cba68b1dc))
	ROM_REGION(0x180000, "sound1",0)
	ROM_LOAD("mach_u14.l1", 0x000000, 0x20000, CRC(be2a736a) SHA1(ebf7b26a86d3ffcc35eaa1da8e4f432bd281fe15))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("mach_u15.l1", 0x080000, 0x20000, CRC(fb49513b) SHA1(01f5243ff258adce3a28b24859eba3f465444bdf))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("mach_u18.l1", 0x100000, 0x20000, CRC(f3f53896) SHA1(4be5a8a27c5ac4718713c05ff2ddf51658a1be27))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
	ROM_REGION(0x8000, "fixed", 0)
	ROM_FILL(0x0000,0x8000,nullptr)
ROM_END

/*===========
/  Test Fixture Alphanumeric
/============*/
ROM_START(tfa_13)
	ROM_REGION(0x30000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("u6_l3.rom", 0x10000, 0x20000, CRC(bf4a37b5) SHA1(91b8bba6182e818a34252a4b2a0b86a2a44d9c42))
	ROM_REGION(0x180000, "sound1",0)
	ROM_FILL(0x0000,0x180000,nullptr)
	ROM_REGION(0x8000, "fixed", 0)
	ROM_FILL(0x0000,0x8000,nullptr)
ROM_END

GAME(1990,  tfa_13,     0,      wpc_an, wpc_an, wpc_an_state,   wpc_an, ROT0,   "Bally",                "WPC Test Fixture: Alphanumeric (1.3)",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  dd_p7,      dd_l2,  wpc_an_dd, wpc_an, wpc_an_state,   wpc_an, ROT0,   "Bally",                "Dr. Dude (PA-7 WPC)",              MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  dd_p06,     dd_l2,  wpc_an_dd, wpc_an, wpc_an_state,   wpc_an, ROT0,   "Bally",                "Dr. Dude (PA-6 WPC)",              MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  fh_l9,      0,      wpc_an, wpc_an, wpc_an_state,   wpc_an, ROT0,   "Williams",             "Funhouse L-9 (SL-2m)",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  fh_l9b,     fh_l9,  wpc_an, wpc_an, wpc_an_state,   wpc_an, ROT0,   "Williams",             "Funhouse L-9 (SL-2m) Bootleg Improved German translation",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1996,  fh_905h,    fh_l9,  wpc_an, wpc_an, wpc_an_state,   wpc_an, ROT0,   "Williams",             "Funhouse 9.05H",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  fh_l3,      fh_l9,  wpc_an, wpc_an, wpc_an_state,   wpc_an, ROT0,   "Williams",             "Funhouse L-3",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  fh_l4,      fh_l9,  wpc_an, wpc_an, wpc_an_state,   wpc_an, ROT0,   "Williams",             "Funhouse L-4",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  fh_l5,      fh_l9,  wpc_an, wpc_an, wpc_an_state,   wpc_an, ROT0,   "Williams",             "Funhouse L-5",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  hd_l3,      0,      wpc_an, wpc_an, wpc_an_state,   wpc_an, ROT0,   "Bally",                "Harley Davidson (L-3)",                MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  hd_l1,      hd_l3,  wpc_an, wpc_an, wpc_an_state,   wpc_an, ROT0,   "Bally",                "Harley Davidson (L-1)",                MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  bop_l7,     0,      wpc_an, wpc_an, wpc_an_state,   wpc_an, ROT0,   "Williams",             "The Machine: Bride of Pinbot (L-7)",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  bop_l6,     bop_l7, wpc_an, wpc_an, wpc_an_state,   wpc_an, ROT0,   "Williams",             "The Machine: Bride of Pinbot (L-6)",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  bop_l5,     bop_l7, wpc_an, wpc_an, wpc_an_state,   wpc_an, ROT0,   "Williams",             "The Machine: Bride of Pinbot (L-5)",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  bop_l4,     bop_l7, wpc_an, wpc_an, wpc_an_state,   wpc_an, ROT0,   "Williams",             "The Machine: Bride of Pinbot (L-4)",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  bop_l3,     bop_l7, wpc_an, wpc_an, wpc_an_state,   wpc_an, ROT0,   "Williams",             "The Machine: Bride of Pinbot (L-3)",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  bop_l2,     bop_l7, wpc_an, wpc_an, wpc_an_state,   wpc_an, ROT0,   "Williams",             "The Machine: Bride of Pinbot (L-2)",               MACHINE_IS_SKELETON_MECHANICAL)
