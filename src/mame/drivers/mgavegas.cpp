// license:BSD-3-Clause
// copyright-holders:ANY
/***********************************************************************************

    mgavegas.c

    Coin pusher

    TODO
    -better analog audio out/mixer
    -some output (mostly not used)

Ver. 1.33 have no speech and no change funcion implemented in software
Ver. 2.1 and 2.3 have change function working and speech
Ver. 2.2 should exist

************************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/flt_rc.h"
#include "mgavegas.lh"

/****************************
*    LOG defines            *
****************************/

#define LOG_AY8910  0
#define LOG_MSM5205 0
#define LOG_CSO1    0
#define LOG_CSO2    0


/****************************
*    Clock defines          *
****************************/
#define MAIN_XTAL XTAL_8MHz
#define CPU_CLK MAIN_XTAL/2
#define AY_CLK  CPU_CLK/2
#define MSM_CLK   384000


class mgavegas_state : public driver_device
{
public:
	mgavegas_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ay(*this, "aysnd"),
		m_msm(*this, "5205"),
		m_ticket(*this, "hopper"),
		m_filter1(*this, "filter1"),
		m_filter2(*this, "filter2")


	{ }
	UINT8 m_int;

	//OUT1
	UINT8 m_ckmod;
	UINT8 m_dmod;
	UINT8 m_emod;
	UINT8 m_inh;
	UINT8 m_hop;
	UINT8 m_seg;
	UINT8 m_printer;
	UINT8 m_auxp;

	//helper...
	UINT8 m_old_ckmod;
	UINT8 m_old_emod;

	//OUT2
	UINT8 m_bobina_ctrl;
	UINT8 m_timbre;
	UINT8 m_coil_1;
	UINT8 m_coil_2;
	UINT8 m_coil_3;
	UINT8 m_cont_ent;
	UINT8 m_cont_sal;
	UINT8 m_cont_caj;

	//lamps out
	UINT64 m_custom_data;
	UINT8 m_auxs;
	UINT8 m_anal;
	UINT8 m_anacl;
	UINT8 m_anacr;
	UINT8 m_anar;
	UINT8 m_pl;
	UINT8 m_pc;
	UINT8 m_pr;
	UINT8 m_luz_250_rul;
	UINT8 m_luz_100_rul;
	UINT8 m_luz_50_rlul;
	UINT8 m_luz_25_lrul;
	UINT8 m_luz_25_rrul;
	UINT8 m_fl;
	UINT8 m_fc;
	UINT8 m_fr;
	UINT8 m_insert_coin;
	UINT8 m_no_cambio;
	UINT8 m_fuse;
	UINT8 m_falta;
	UINT8 m_anag;
	UINT8 m_cl;
	UINT8 m_cc;
	UINT8 m_cr;
	UINT8 m_premio_s;
	UINT8 m_100;
	UINT8 m_200;
	UINT8 m_300;
	UINT8 m_500;
	UINT8 m_ml;
	UINT8 m_mc;
	UINT8 m_mr;

	DECLARE_READ8_MEMBER(start_read);

	DECLARE_WRITE8_MEMBER(w_a0);
	DECLARE_READ8_MEMBER(r_a0);
	DECLARE_WRITE8_MEMBER(cso1_w);
	DECLARE_WRITE8_MEMBER(cso2_w);
	DECLARE_WRITE8_MEMBER(csoki_w);
	DECLARE_READ8_MEMBER(csoki_r);

	DECLARE_READ8_MEMBER(ay8910_a_r);
	DECLARE_READ8_MEMBER(ay8910_b_r);

	DECLARE_DRIVER_INIT(mgavegas);
	DECLARE_DRIVER_INIT(mgavegas21);
	DECLARE_DRIVER_INIT(mgavegas133);

	TIMER_DEVICE_CALLBACK_MEMBER(int_0);


protected:

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<ay8910_device> m_ay;
	required_device<msm5205_device> m_msm;
	required_device<ticket_dispenser_device> m_ticket;
	required_device<filter_rc_device> m_filter1;
	required_device<filter_rc_device> m_filter2;

	// driver_device overrides
	virtual void machine_reset() override;
	void update_custom();
	void update_lamp();


private:
};


void mgavegas_state::update_lamp(){
	//output().set_value("AUXS", m_auxs); //unused
	output().set_value("MGA4", m_anal&0x01);
	output().set_value("MGA3", m_anacl&0x01);
	output().set_value("MGA2", m_anacr&0x01);
	output().set_value("MGA", m_anar&0x01);
	output().set_value("PL", m_pl&0x01);
	output().set_value("PC", m_pc&0x01);
	output().set_value("PR", m_pr&0x01);
	output().set_value("250", m_luz_250_rul&0x01);
	output().set_value("1002", m_luz_100_rul&0x01);
	output().set_value("50", m_luz_50_rlul&0x01);
	output().set_value("252", m_luz_25_lrul&0x01);
	output().set_value("25", m_luz_25_rrul&0x01);
	output().set_value("FL", m_fl&0x01);
	output().set_value("FC", m_fc&0x01);
	output().set_value("FR", m_fr&0x01);
	output().set_value("INSERTCOIN", m_insert_coin&0x01);
	output().set_value("NOCAMBIO", m_no_cambio&0x01);
	output().set_value("FUSE", m_fuse&0x01);
	output().set_value("FALTA", m_falta&0x01);
	//output().set_value("ANAG", m_anag&0x01);    //unused
	output().set_value("CL", m_cl&0x01);
	output().set_value("CC", m_cc&0x01);
	output().set_value("CR", m_cr&0x01);
	output().set_value("PREMIOS", m_premio_s&0x01);
	output().set_value("100", m_100&0x01);
	output().set_value("200", m_200&0x01);
	output().set_value("300", m_300&0x01);
	output().set_value("500", m_500&0x01);
	output().set_value("ML", m_ml&0x01);
	output().set_value("MC", m_mc&0x01);
	output().set_value("MR", m_mr&0x01);
/*
    m_inh=BIT(data, 3);
    m_printer=BIT(data, 6); //not_used
    m_auxp=BIT(data, 7);    //not_used

    m_bobina_ctrl=BIT(data, 0);
    m_timbre=BIT(data, 1);
    m_coil_1=BIT(data, 2);
    m_coil_2=BIT(data, 3);
    m_coil_3=BIT(data, 4);
    m_cont_ent=BIT(data, 5);
    m_cont_sal=BIT(data, 6);
    m_cont_caj=BIT(data, 7);
*/
}


void mgavegas_state::update_custom(){
UINT64 tmp;

	if( (m_ckmod==1) & (m_old_ckmod==0) ){
		//vadid clock, sample the data
		m_custom_data=(m_custom_data<<1)|(m_dmod&0x01);
	}

	if( (m_emod==0) & (m_old_emod==1) ){
		//valid emod, check for valid data and updatae custom status    this is how the hw works
		if( (BIT(m_custom_data, 32)==0) && (BIT(m_custom_data, 33)==0) && (BIT(m_custom_data, 34)==0) && (BIT(m_custom_data, 35)==0) ){
				tmp=~m_custom_data;
				m_auxs=         tmp&0x00000001;
//              m_anal=         (tmp&0x00000002)>>1;    //schematics error!!!
//              m_anacl=        (tmp&0x00000004)>>2;    //schematics error!!!
				m_luz_50_rlul=  (tmp&0x00000002)>>1;
				m_luz_25_lrul=  (tmp&0x00000004)>>2;
				m_anacr=        (tmp&0x00000008)>>3;
				m_anar=         (tmp&0x00000010)>>4;
				m_pl=           (tmp&0x00000020)>>5;
				m_pc=           (tmp&0x00000040)>>6;
				m_pr=           (tmp&0x00000080)>>7;
				m_luz_250_rul=  (tmp&0x00000100)>>8;
				m_luz_100_rul=  (tmp&0x00000200)>>9;
//              m_luz_50_rlul=  (tmp&0x00000400)>>10;   //schematics error!!!
//              m_luz_25_lrul=  (tmp&0x00000800)>>11;   //schematics error!!!
				m_anacl=        (tmp&0x00000400)>>10;
				m_anal=         (tmp&0x00000800)>>11;
				m_luz_25_rrul=  (tmp&0x00001000)>>12;
				m_fl=           (tmp&0x00002000)>>13;
				m_fc=           (tmp&0x00004000)>>14;
				m_fr=           (tmp&0x00008000)>>15;
				m_insert_coin=  (tmp&0x00010000)>>16;
				m_no_cambio=    (tmp&0x00020000)>>17;
				m_fuse=         (tmp&0x00040000)>>18;
				m_falta=        (tmp&0x00080000)>>19;
				m_anag=         (tmp&0x00100000)>>20;
				m_cl=           (tmp&0x00200000)>>21;
				m_cc=           (tmp&0x00400000)>>22;
				m_cr=           (tmp&0x00800000)>>23;
				m_premio_s=     (tmp&0x01000000)>>24;
				m_100=          (tmp&0x02000000)>>25;
				m_200=          (tmp&0x04000000)>>26;
				m_300=          (tmp&0x08000000)>>27;
				m_500=          (tmp&0x10000000)>>28;
				m_ml=           (tmp&0x20000000)>>29;
				m_mc=           (tmp&0x40000000)>>30;
				m_mr=           (tmp&0x80000000)>>31;

				update_lamp();
		}
	}

	m_old_ckmod=m_ckmod;
	m_old_emod=m_emod;
}


READ8_MEMBER( mgavegas_state::start_read )
{
//  in HW it look for /IOREQ going down to clear the IRQ line
	if (m_int){
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
		m_int=0;
	}
	return 0xed;
}



/****************************
*    Read/Write Handlers    *
****************************/
READ8_MEMBER(mgavegas_state::r_a0)
{
UINT8 ret=0;


	switch (offset&0x03)
	{
		case 1: //bdir=0    BC1=1
				ret=m_ay->data_r(space,0);
				break;
		default:
				if (LOG_AY8910)
					logerror("AY 3-8910 area unknow read!!!\n");
				break;
	}

	if (LOG_AY8910)
		logerror("read from %04X return %02X\n",offset+0xa000,ret);
	return ret;
}

WRITE8_MEMBER(mgavegas_state::w_a0)
{
	if (LOG_AY8910)
		logerror("write to %04X data = %02X \n",offset+0xa000,data);

	switch (offset&0x03)
	{
		case 0: //bdir=1    bc1=1
				m_ay->address_w(space,0,data );
				break;
		case 2: //bdir=1    bc1=0
				m_ay->data_w(space,0,data );
				break;
/*
        case 1: //bdir=0    bc1=1
                break;
        case 3: //bdir=0    bc1=0
                break;
*/
		default:
				if (LOG_AY8910)
					logerror("AY 3-8910 area unknow write!!!\n");
				break;
	}
}





READ8_MEMBER(mgavegas_state::csoki_r)
{
UINT8 ret=0;

	if (LOG_MSM5205)
		logerror("read from %04X return %02X\n",offset+0xc800,ret);
	return ret;
}

WRITE8_MEMBER(mgavegas_state::csoki_w)
{
	if (LOG_MSM5205)
		logerror("MSM5205 write to %04X data = %02X \n",offset+0xc800,data);
	m_msm->reset_w(data&0x10>>4);
	m_msm->data_w(data&0x0f);
}


WRITE8_MEMBER(mgavegas_state::cso1_w)
{
	int hopper_data = 0x00;
	if (LOG_CSO1)
		logerror("write to CSO1 data = %02X\n",data);

	m_ckmod=BIT(data, 0);
	m_dmod=BIT(data, 1);
	m_emod=BIT(data, 2);
	m_inh=BIT(data, 3);
	m_hop=BIT(data, 4);
	m_seg=BIT(data, 5);
	m_printer=BIT(data, 6); //not_used
	m_auxp=BIT(data, 7);    //not_used

	update_custom();

	hopper_data=(m_hop&0x01)<<7;
	m_ticket->write(machine().driver_data()->generic_space(), 0, hopper_data);
}

WRITE8_MEMBER(mgavegas_state::cso2_w)
{
	if (LOG_CSO2)
		logerror("write to CSO2 data = %02X\n",data);

	m_bobina_ctrl=BIT(data, 0);
	m_timbre=BIT(data, 1);
	m_coil_1=BIT(data, 2);
	m_coil_2=BIT(data, 3);
	m_coil_3=BIT(data, 4);
	m_cont_ent=BIT(data, 5);
	m_cont_sal=BIT(data, 6);
	m_cont_caj=BIT(data, 7);

	update_lamp();
}


READ8_MEMBER(mgavegas_state::ay8910_a_r)
{
	UINT8 ret=0xff;

	ret=ioport("INA")->read();

	if (LOG_AY8910)
		logerror("read from port A return %02X\n",ret);

	return ret;
}

READ8_MEMBER(mgavegas_state::ay8910_b_r)
{
	UINT8 ret=0xff;

	ret=ioport("DSW1")->read();

	if (LOG_AY8910)
		logerror("read from port B return %02X\n",ret);

	return ret;
}

/*************************
* Memory Map Information *
*************************/

static ADDRESS_MAP_START( mgavegas_map, AS_PROGRAM, 8, mgavegas_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xa000, 0xa003) AM_READWRITE(r_a0,w_a0)            // AY-3-8910
	AM_RANGE(0xc000, 0xc001) AM_WRITE(cso1_w)                   // /CSout1
	AM_RANGE(0xc400, 0xc401) AM_WRITE(cso2_w)                   // /CSout2
	AM_RANGE(0xc800, 0xc801) AM_READWRITE(csoki_r,csoki_w)      // /CSoki
	//AM_RANGE(0xcc00, 0xcc01) AM_READWRITE(cso3_r,cso3_w)      // /CSout3 unused
	//AM_RANGE(0xe000, 0xe003) AM_READWRITE(r_e0,w_e0)          // /CSaux unused
ADDRESS_MAP_END



/*************************
*      Input Ports       *
*************************/


static INPUT_PORTS_START( mgavegas )

	PORT_START("INA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) //200ptas in for change with 8 25 ptas coins
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) //25 ptas in to play
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) //100ptas in for change with 4 25 ptas coins
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("25 ptas level")     //"hack" hopper always full
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("Door")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("Channel")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x02, "Percentage" )
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPSETTING(    0x01, "70%" )
	PORT_DIPSETTING(    0x02, "72%" )
	PORT_DIPSETTING(    0x03, "74%" )
	PORT_DIPSETTING(    0x04, "76%" )
	PORT_DIPSETTING(    0x05, "78%" )
	PORT_DIPSETTING(    0x06, "80%" )
	PORT_DIPSETTING(    0x07, "82%" )

	PORT_DIPNAME( 0x08, 0x08, "Sound" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On )  )

	PORT_DIPNAME( 0x10, 0x10, "Speech" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On )  )

	PORT_DIPNAME( 0x60, 0x00, "Jackpot" )
	PORT_DIPSETTING(    0x40, "Jackpot 1%" )
	PORT_DIPSETTING(    0x60, "Jackpot 2,5%" )
	PORT_DIPSETTING(    0x00, "Jackpot 5%" )
	//PORT_DIPSETTING(    0x20, DEF_STR( On ) ) //unlisted

	PORT_DIPNAME( 0x80, 0x80, "Reset" )
	PORT_DIPSETTING(    0x80, "Normal Gameplay" )
	PORT_DIPSETTING(    0x00, "Reset" )

INPUT_PORTS_END

/******************************
*   machine reset             *
******************************/

void mgavegas_state::machine_reset()
{
	m_int=1;
	m_custom_data=U64(0xffffffffffffffff);

	m_old_ckmod=1;
	m_old_emod=0;

	m_ckmod=0;
	m_dmod=0;
	m_emod=0;
	m_inh=0;
	m_hop=0;
	m_seg=0;
	m_printer=0;
	m_auxp=0;


	m_bobina_ctrl=0;
	m_timbre=0;
	m_coil_1=0;
	m_coil_2=0;
	m_coil_3=0;
	m_cont_ent=0;
	m_cont_sal=0;
	m_cont_caj=0;

	m_auxs=0;
	m_anal=0;
	m_anacl=0;
	m_anacr=0;
	m_anar=0;
	m_pl=0;
	m_pc=0;
	m_pr=0;
	m_luz_250_rul=0;
	m_luz_100_rul=0;
	m_luz_50_rlul=0;
	m_luz_25_lrul=0;
	m_luz_25_rrul=0;
	m_fl=0;
	m_fc=0;
	m_fr=0;
	m_insert_coin=0;
	m_no_cambio=0;
	m_fuse=0;
	m_falta=0;
	m_anag=0;
	m_cl=0;
	m_cc=0;
	m_cr=0;
	m_premio_s=0;
	m_100=0;
	m_200=0;
	m_300=0;
	m_500=0;
	m_ml=0;
	m_mc=0;
	m_mr=0;

	m_filter1->filter_rc_set_RC(FLT_RC_LOWPASS, 1000, 0, 0, CAP_N(1) );     /* RC out of MSM5205 R=1K C=1nF */
	m_filter2->filter_rc_set_RC(FLT_RC_HIGHPASS, 3846, 0, 0, CAP_N(100 ));  /*ALP3B active-hybrid filter fc=2.6Khz 2poles???*/
}



/******************************
*   machine init             *
******************************/

DRIVER_INIT_MEMBER(mgavegas_state,mgavegas21)
{
	//hack to clear the irq on reti instruction
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00ea, 0x00ea, read8_delegate(FUNC(mgavegas_state::start_read), this));
}

DRIVER_INIT_MEMBER(mgavegas_state,mgavegas)
{
	//hack to clear the irq on reti instruction
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00e2, 0x00e2, read8_delegate(FUNC(mgavegas_state::start_read), this));
}


TIMER_DEVICE_CALLBACK_MEMBER( mgavegas_state::int_0 )
{
	if(m_int==0){
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
	}
}

DRIVER_INIT_MEMBER(mgavegas_state,mgavegas133)
{
	//hack to clear the irq on reti instruction
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00dd, 0x00dd, read8_delegate(FUNC(mgavegas_state::start_read), this));
}

/*************************
*    Machine Drivers     *
*************************/


static MACHINE_CONFIG_START( mgavegas, mgavegas_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, CPU_CLK)
	MCFG_CPU_PROGRAM_MAP(mgavegas_map)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("int_0", mgavegas_state, int_0, attotime::from_hz(6000))  //6KHz from MSM5205 /VCK

	MCFG_NVRAM_ADD_1FILL("nvram")

	MCFG_TICKET_DISPENSER_ADD("hopper",attotime::from_msec(200),TICKET_MOTOR_ACTIVE_HIGH,TICKET_STATUS_ACTIVE_LOW);

	/* sound hardware */

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, AY_CLK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.3)
	MCFG_AY8910_PORT_A_READ_CB(READ8(mgavegas_state, ay8910_a_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(mgavegas_state, ay8910_b_r))

	MCFG_SOUND_ADD("5205", MSM5205, MSM_CLK)
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S64_4B)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "filter1", 2.0)


	MCFG_FILTER_RC_ADD("filter1", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "filter2",2.0)
	MCFG_FILTER_RC_ADD("filter2", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2.0)


	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_mgavegas)

MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/


ROM_START(mgavegas)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("vegas1-k2.3.bin", 0x0000, 0x8000, CRC(418b1d04) SHA1(27669a85ed52d5dab25d6ebea6ef3d9b01a4795d) )

	ROM_REGION( 0x2000, "nvram", 0 )
	ROM_LOAD( "mgavegas23.nv", 0x0000, 0x2000, CRC(d0a175b0) SHA1(7698135dbc020f459fdaa660bf488595b67b77d0) )  //default setting
ROM_END

ROM_START(mgavegas21)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("vegas1-2.1.bin", 0x0000, 0x8000, CRC(a7e988a4) SHA1(32fa1684c79f4a132553fa41006f243d4b51cef6) )

	ROM_REGION( 0x2000, "nvram", 0 )
	ROM_LOAD( "mgavegas21.nv", 0x0000, 0x2000, CRC(a4471550) SHA1(b8527e9158b5563460febd1009b44c8d74dbae4e) )  //default setting
ROM_END

ROM_START(mgavegas133)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("vegas1-1.33.bin", 0x0000, 0x8000, CRC(1eea7f0f) SHA1(6fb54b8e2ab19e5378a95192e5007175ad76bc7a) )

	ROM_REGION( 0x2000, "nvram", 0 )
	ROM_LOAD( "mgavegas133.nv", 0x0000, 0x2000, CRC(20fe4db7) SHA1(887b69468ac7e6490827a06cd1f0ff15228a9c73) )  //default setting
ROM_END


/*************************
*      Game Drivers      *
*************************/
/*    YEAR  NAME            PARENT      MACHINE   INPUT     STATE          INIT         ROT     COMPANY     FULLNAME    FLAGS*/
GAME( 1985, mgavegas,       0,          mgavegas, mgavegas, mgavegas_state, mgavegas,   ROT0,   "MGA",    "Vegas 1 (Ver 2.3 dual coin pulse, shorter)", MACHINE_MECHANICAL )
GAME( 1985, mgavegas21,     mgavegas,   mgavegas, mgavegas, mgavegas_state, mgavegas21, ROT0,   "MGA",    "Vegas 1 (Ver 2.1 dual coin pulse, longer)", MACHINE_MECHANICAL )
GAME( 1985, mgavegas133,    mgavegas,   mgavegas, mgavegas, mgavegas_state, mgavegas133,ROT0,   "MGA",    "Vegas 1 (Ver 1.33 single coin pulse)", MACHINE_MECHANICAL )
