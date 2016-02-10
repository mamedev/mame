// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Chris Hardy, David Haywood, Tomasz Slanina
/******************************************************************************************

PC-XT (c) 1987 IBM

(Actually Arcade games running on more or less modified PC-XT HW)

driver by Angelo Salese & Chris Hardy
original tetriunk.c by David Haywood & Tomasz Slanina

Notes:
- The Korean Tetris is a blantant rip-off of the Mirrorsoft/Andromeda Software Tetris PC
  version;

TODO:
- 02851: tetriskr: Corrupt game graphics after some time of gameplay, caused by a wrong
  reading of the i/o $3c8 bit 1.
- Add a proper FDC device.
- Filetto: Add UM5100 sound chip, might be connected to the prototyping card;
- buzzer sound has issues in both games

********************************************************************************************
Filetto HW notes:
The PCB is a un-modified IBM-PC with a CGA adapter & a prototyping card that controls the
interface between the pc and the Jamma connectors.Additionally there's also a UM5100 sound
chip for the sound.
PCB Part Number: S/N 90289764 NOVARXT
PCB Contents:
1x UMC 8923S-UM5100 voice processor (upper board)
1x MMI PAL16L8ACN-940CRK9 (upper board)
1x AMD AMPAL16R8APC-8804DM (upper board)
1x AMD P8088-1 main processor 8.000MHz (lower board)
1x Proton PT8010AF PLCC 28.636MHz (lower board)
1x UMC 8928LP-UM8272A floppy disk controller (lower board)
1x UMC 8935CS-UM82C11 Printer Adapter Interface (lower board)
1x UMC 8936CS-UM8250B Programmable asynchronous communications element (lower board)
There isn't any keyboard found connected to the pcb.
********************************************************************************************
Filetto SW notes:
The software of this game can be extracted with a normal Windows program extractor.
The files names are:
-command.com  (1)
-ibmbio.com   (1)
-ibmdos.com   (1)
-ansi.sys     (1)
-config.sys   (2)
-autoexec.bat (3)
-x.exe        (4)
(1)This is an old Italian version of MS-DOS (v3.30 18th March 1987).
(2)Contains "device=ansi.sys",it's an hook-up for the graphics used by the BIOS.
(3)It has an Echo off (as you can notice from the game itself) and then the loading of the
main program (x.exe).
(4)The main program,done in plain Basic with several Italian comments in it.The date of
the main program is 9th October 1990.

******************************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/am9517a.h"
#include "machine/pic8259.h"
#include "sound/hc55516.h"
#include "sound/speaker.h"
#include "bus/isa/isa.h"
#include "bus/isa/cga.h"


class pcxt_state : public driver_device
{
public:
	pcxt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_pit8253(*this, "pit8253"),
			m_pic8259_1(*this, "pic8259_1"),
			m_dma8237_1(*this, "dma8237_1") ,
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker") { }

	int m_bank;
	int m_lastvalue;
	UINT8 m_disk_data[2];
	UINT8 m_port_b_data;
	UINT8 m_wss1_data;
	UINT8 m_wss2_data;
	UINT8 m_status;
	UINT8 m_clr_status;
	int m_dma_channel;
	UINT8 m_dma_offset[2][4];
	UINT8 m_at_pages[0x10];
	UINT8 m_pc_spkrdata, m_pit_out2;

	required_device<pit8253_device> m_pit8253;
	required_device<pic8259_device> m_pic8259_1;
	required_device<am9517a_device> m_dma8237_1;

	DECLARE_READ8_MEMBER(disk_iobank_r);
	DECLARE_WRITE8_MEMBER(disk_iobank_w);
	DECLARE_READ8_MEMBER(fdc765_status_r);
	DECLARE_READ8_MEMBER(fdc765_data_r);
	DECLARE_WRITE8_MEMBER(fdc765_data_w);
	DECLARE_WRITE8_MEMBER(fdc_dor_w);
	DECLARE_READ8_MEMBER(pc_dma_read_byte);
	DECLARE_WRITE8_MEMBER(pc_dma_write_byte);
	DECLARE_READ8_MEMBER(dma_page_select_r);
	DECLARE_WRITE8_MEMBER(dma_page_select_w);
	DECLARE_WRITE_LINE_MEMBER(ibm5150_pit8253_out2_changed);
	DECLARE_READ8_MEMBER(port_a_r);
	DECLARE_READ8_MEMBER(port_b_r);
	DECLARE_READ8_MEMBER(port_c_r);
	DECLARE_WRITE8_MEMBER(port_b_w);
	DECLARE_WRITE8_MEMBER(wss_1_w);
	DECLARE_WRITE8_MEMBER(wss_2_w);
	DECLARE_WRITE8_MEMBER(sys_reset_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dma_hrq_changed);
	DECLARE_WRITE_LINE_MEMBER(pc_dack0_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack1_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack2_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack3_w);
	DECLARE_DRIVER_INIT(tetriskr);
	DECLARE_DRIVER_INIT(filetto);
	virtual void machine_reset() override;
	UINT8 pcxt_speaker_get_spk();
	void pcxt_speaker_set_spkrdata(UINT8 data);
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
};


class isa8_cga_filetto_device : public isa8_cga_device
{
public:
	// construction/destruction
	isa8_cga_filetto_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual const rom_entry *device_rom_region() const override;
};

const device_type ISA8_CGA_FILETTO = &device_creator<isa8_cga_filetto_device>;

//-------------------------------------------------
//  isa8_cga_filetto_device - constructor
//-------------------------------------------------

isa8_cga_filetto_device::isa8_cga_filetto_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		isa8_cga_device( mconfig, ISA8_CGA_FILETTO, "ISA8_CGA_FILETTO", tag, owner, clock, "filetto_cga", __FILE__)
{
}

ROM_START( filetto_cga )
	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD("u67.bin", 0x0000, 0x2000, CRC(09710122) SHA1(de84bdd9245df287bbd3bb808f0c3531d13a3545) )
ROM_END

const rom_entry *isa8_cga_filetto_device::device_rom_region() const
{
	return ROM_NAME( filetto_cga );
}



class isa8_cga_tetriskr_device : public isa8_cga_superimpose_device
{
public:
	// construction/destruction
	isa8_cga_tetriskr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
	virtual void device_start() override;
	virtual const rom_entry *device_rom_region() const override;

	DECLARE_READ8_MEMBER(bg_bank_r);
	DECLARE_WRITE8_MEMBER(bg_bank_w);
private:
	UINT8 m_bg_bank;
};


/* for superimposing CGA over a different source video (i.e. tetriskr) */
const device_type ISA8_CGA_TETRISKR = &device_creator<isa8_cga_tetriskr_device>;

//-------------------------------------------------
//  isa8_cga_tetriskr_device - constructor
//-------------------------------------------------

isa8_cga_tetriskr_device::isa8_cga_tetriskr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		isa8_cga_superimpose_device( mconfig, ISA8_CGA_TETRISKR, "ISA8_CGA_TETRISKR", tag, owner, clock, "tetriskr_cga", __FILE__)
{
}


void isa8_cga_tetriskr_device::device_start()
{
	m_bg_bank = 0;
	isa8_cga_superimpose_device::device_start();
	m_isa->install_device(0x3c0, 0x3c0, 0, 0,  read8_delegate( FUNC(isa8_cga_tetriskr_device::bg_bank_r), this ), write8_delegate( FUNC(isa8_cga_tetriskr_device::bg_bank_w), this ) );
}

WRITE8_MEMBER(isa8_cga_tetriskr_device::bg_bank_w)
{
	m_bg_bank = (data & 0x0f) ^ 8;
}

READ8_MEMBER(isa8_cga_tetriskr_device::bg_bank_r)
{
	return 0xff;
}


UINT32 isa8_cga_tetriskr_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y;
	int yi;
	const UINT8 *bg_rom = memregion("gfx2")->base();

	//popmessage("%04x",m_start_offs);

	bitmap.fill(rgb_t::black, cliprect);

	for(y=0;y<200/8;y++)
	{
		for(yi=0;yi<8;yi++)
		{
			for(x=0;x<320/8;x++)
			{
				UINT8 color;
				int xi,pen_i;

				for(xi=0;xi<8;xi++)
				{
					color = 0;
					/* TODO: first byte seems bogus? */
					for(pen_i = 0;pen_i<4;pen_i++)
						color |= ((bg_rom[y*320/8+x+(pen_i*0x20000)+yi*0x400+m_bg_bank*0x2000+1] >> (7-xi)) & 1) << pen_i;

					if(cliprect.contains(x*8+xi, y*8+yi))
						bitmap.pix32(y*8+yi, x*8+xi) = m_palette->pen(color);
				}
			}
		}
	}

	isa8_cga_device::screen_update(screen, bitmap, cliprect);
	return 0;
}


ROM_START( tetriskr_cga )
	ROM_REGION( 0x2000, "gfx1",ROMREGION_ERASE00 ) /* gfx - 1bpp font*/
	ROM_LOAD( "b-3.u36", 0x1800, 0x0800, CRC(1a636f9a) SHA1(a356cc57914d0c9b9127670b55d1f340e64b1ac9) )
	ROM_IGNORE( 0x1800 )

	ROM_REGION( 0x80000+1, "gfx2",ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "b-1.u59", 0x00000, 0x10000, CRC(4719d986) SHA1(6e0499944b968d96fbbfa3ead6237d69c769d634))
	ROM_LOAD( "b-2.u58", 0x10000, 0x10000, CRC(599e1154) SHA1(14d99f90b4fedeab0ac24ffa9b1fd9ad0f0ba699))
	ROM_LOAD( "b-4.u54", 0x20000, 0x10000, CRC(e112c450) SHA1(dfdecfc6bd617ec520b7563b7caf44b79d498bd3))
	ROM_LOAD( "b-5.u53", 0x30000, 0x10000, CRC(050b7650) SHA1(5981dda4ed43b6e81fbe48bfba90a8775d5ecddf))
	ROM_LOAD( "b-6.u49", 0x40000, 0x10000, CRC(d596ceb0) SHA1(8c82fb638688971ef11159a6b240253e63f0949d))
	ROM_LOAD( "b-7.u48", 0x50000, 0x10000, CRC(79336b6c) SHA1(7a95875f3071bdc3ee25c0e6a5a3c00ef02dc977))
	ROM_LOAD( "b-8.u44", 0x60000, 0x10000, CRC(1f82121a) SHA1(106da0f39f1260d0761217ed0a24c1611bfd7f05))
	ROM_LOAD( "b-9.u43", 0x70000, 0x10000, CRC(4ea22349) SHA1(14dfd3dbd51f8bd6f3290293b8ea1c165e8cf7fd))
ROM_END

const rom_entry *isa8_cga_tetriskr_device::device_rom_region() const
{
	return ROM_NAME( tetriskr_cga );
}

READ8_MEMBER(pcxt_state::disk_iobank_r)
{
	//printf("Read Prototyping card [%02x] @ PC=%05x\n",offset,space.device().safe_pc());
	//if(offset == 0) return ioport("DSW")->read();
	if(offset == 1) return ioport("IN1")->read();

	return m_disk_data[offset];
}

WRITE8_MEMBER(pcxt_state::disk_iobank_w)
{
/*
    BIOS does a single out $0310,$F0 on reset

    Then does 2 outs to set the bank..

        X1  X2

        $F0 $F2 = m0
        $F1 $F2 = m1
        $F0 $F3 = m2
        $F1 $F3 = m3

    The sequence of

    out $0310,X1
    out $0310,X2

    sets the selected rom that appears in $C0000-$CFFFF

*/
	int newbank = 0;

//  printf("bank %d set to %02X\n", offset,data);

	if (data == 0xF0)
	{
		newbank = 0;
	}
	else
	{
		if((m_lastvalue == 0xF0) && (data == 0xF2))
			newbank = 0;
		else if ((m_lastvalue == 0xF1) && (data == 0xF2))
			newbank = 1;
		else if ((m_lastvalue == 0xF0) && (data == 0xF3))
			newbank = 2;
		else if ((m_lastvalue == 0xF1) && (data == 0xF3))
			newbank = 3;
	}

//  printf("newbank = %d\n", newbank);

	if (newbank != m_bank)
	{
		m_bank = newbank;
		membank("bank1")->set_base(memregion("game_prg")->base() + 0x10000 * m_bank );
	}

	m_lastvalue = data;

	m_disk_data[offset] = data;
}

/*********************************
Pit8253
*********************************/

// pc_speaker_get_spk, pc_speaker_set_spkrdata, and pc_speaker_set_input already exists in MESS, can the implementations be merged?
UINT8 pcxt_state::pcxt_speaker_get_spk()
{
	return m_pc_spkrdata & m_pit_out2;
}

void pcxt_state::pcxt_speaker_set_spkrdata(UINT8 data)
{
	m_pc_spkrdata = data ? 1 : 0;
	m_speaker->level_w(pcxt_speaker_get_spk());
}


WRITE_LINE_MEMBER(pcxt_state::ibm5150_pit8253_out2_changed)
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(pcxt_speaker_get_spk());
}


READ8_MEMBER(pcxt_state::port_a_r)
{
	if(!(m_port_b_data & 0x80))//???
	{
		/*
		x--- ---- Undefined (Always 0)
		-x-- ---- B: Floppy disk drive installed.
		--xx ---- Default Display Mode
		---- xx-- Undefined (Always 1)
		---- --x- 8087 NDP installed
		---- ---x Undefined (Always 1)
		*/
		return m_wss1_data;
	}
	else//keyboard emulation
	{
		//m_maincpu->set_input_line(1, PULSE_LINE);
		return 0x00;//Keyboard is disconnected
		//return 0xaa;//Keyboard code
	}
}

READ8_MEMBER(pcxt_state::port_b_r)
{
	return m_port_b_data;
}

READ8_MEMBER(pcxt_state::port_c_r)
{
	if ( m_port_b_data & 0x01 )
	{
		m_wss2_data = ( m_wss2_data & ~0x10 ) | ( m_pit_out2 ? 0x10 : 0x00 );
	}
	m_wss2_data = ( m_wss2_data & ~0x20 ) | ( m_pit_out2 ? 0x20 : 0x00 );

	return m_wss2_data;//TODO
}

/*'buzzer' sound routes here*/
/* Filetto uses this for either beep and um5100 sound routing,probably there's a mux somewhere.*/
/* The Korean Tetris uses it as a regular buzzer,probably the sound is all in there...*/
WRITE8_MEMBER(pcxt_state::port_b_w)
{
	/* PPI controller port B*/
	m_pit8253->write_gate2(BIT(data, 0));
	pcxt_speaker_set_spkrdata( data & 0x02 );
	m_port_b_data = data;
// device_t *beep = machine().device<beep_device>("beep");
// device_t *cvsd = machine().device("cvsd");
//  hc55516_digit_w(cvsd, data);
//  popmessage("%02x\n",data);
//  beep->beep_set_state(0);
//  beep->beep_set_state(1);
//  beep->beep_set_clock(m_port_b_data);
}

WRITE8_MEMBER(pcxt_state::wss_1_w)
{
	m_wss1_data = data;
}

WRITE8_MEMBER(pcxt_state::wss_2_w)
{
	m_wss2_data = data;
}

WRITE8_MEMBER(pcxt_state::sys_reset_w)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
}


/*Floppy Disk Controller 765 device*/
/*Currently we only emulate it at a point that the BIOS will pass the checks*/

#define FDC_BUSY 0x10
#define FDC_WRITE 0x40
#define FDC_READ 0x00 /*~0x40*/

READ8_MEMBER(pcxt_state::fdc765_status_r)
{
	UINT8 tmp;
//  popmessage("Read FDC status @ PC=%05x",space.device().safe_pc());
	tmp = m_status | 0x80;
	m_clr_status++;
	if(m_clr_status == 0x10)
	{
		m_status = 0;
		m_clr_status = 0;
	}
	return tmp;
}

READ8_MEMBER(pcxt_state::fdc765_data_r)
{
	m_status = (FDC_READ);
	m_pic8259_1->ir6_w(0);
	return 0xc0;
}

WRITE8_MEMBER(pcxt_state::fdc765_data_w)
{
	m_status = (FDC_WRITE);
}


WRITE8_MEMBER(pcxt_state::fdc_dor_w)
{
	/* TODO: properly hook-up upd765 FDC there */
	m_pic8259_1->ir6_w(1);
}

/******************
DMA8237 Controller
******************/


WRITE_LINE_MEMBER(pcxt_state::pc_dma_hrq_changed)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237_1->hack_w( state );
}


READ8_MEMBER(pcxt_state::pc_dma_read_byte)
{
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0xFF0000;

	return space.read_byte(page_offset + offset);
}


WRITE8_MEMBER(pcxt_state::pc_dma_write_byte)
{
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0xFF0000;

	space.write_byte(page_offset + offset, data);
}

READ8_MEMBER(pcxt_state::dma_page_select_r)
{
	UINT8 data = m_at_pages[offset % 0x10];

	switch(offset % 8) {
	case 1:
		data = m_dma_offset[(offset / 8) & 1][2];
		break;
	case 2:
		data = m_dma_offset[(offset / 8) & 1][3];
		break;
	case 3:
		data = m_dma_offset[(offset / 8) & 1][1];
		break;
	case 7:
		data = m_dma_offset[(offset / 8) & 1][0];
		break;
	}
	return data;
}


WRITE8_MEMBER(pcxt_state::dma_page_select_w)
{
	m_at_pages[offset % 0x10] = data;

	switch(offset % 8) {
	case 1:
		m_dma_offset[(offset / 8) & 1][2] = data;
		break;
	case 2:
		m_dma_offset[(offset / 8) & 1][3] = data;
		break;
	case 3:
		m_dma_offset[(offset / 8) & 1][1] = data;
		break;
	case 7:
		m_dma_offset[(offset / 8) & 1][0] = data;
		break;
	}
}

static void set_dma_channel(device_t *device, int channel, int state)
{
	pcxt_state *drvstate = device->machine().driver_data<pcxt_state>();
	if (!state) drvstate->m_dma_channel = channel;
}

WRITE_LINE_MEMBER(pcxt_state::pc_dack0_w){ set_dma_channel(m_dma8237_1, 0, state); }
WRITE_LINE_MEMBER(pcxt_state::pc_dack1_w){ set_dma_channel(m_dma8237_1, 1, state); }
WRITE_LINE_MEMBER(pcxt_state::pc_dack2_w){ set_dma_channel(m_dma8237_1, 2, state); }
WRITE_LINE_MEMBER(pcxt_state::pc_dack3_w){ set_dma_channel(m_dma8237_1, 3, state); }

/******************
8259 IRQ controller
******************/

static ADDRESS_MAP_START( filetto_map, AS_PROGRAM, 8, pcxt_state )
	AM_RANGE(0x00000, 0x9ffff) AM_RAM //work RAM 640KB
	AM_RANGE(0xa0000, 0xbffff) AM_RAM //CGA VRAM
	AM_RANGE(0xc0000, 0xcffff) AM_ROMBANK("bank1")
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pcxt_io_common, AS_IO, 8, pcxt_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3ff)
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE("dma8237_1", am9517a_device, read, write ) //8237 DMA Controller
	AM_RANGE(0x0020, 0x002f) AM_DEVREADWRITE("pic8259_1", pic8259_device, read, write ) //8259 Interrupt control
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE("pit8253", pit8253_device, read, write)    //8253 PIT
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)  //PPI 8255
	AM_RANGE(0x0064, 0x0066) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)  //PPI 8255
	AM_RANGE(0x0080, 0x0087) AM_READWRITE(dma_page_select_r,dma_page_select_w)
	AM_RANGE(0x0278, 0x027f) AM_RAM //printer (parallel) port latch
	AM_RANGE(0x02f8, 0x02ff) AM_RAM //Modem port
	AM_RANGE(0x0378, 0x037f) AM_RAM //printer (parallel) port
	AM_RANGE(0x03bc, 0x03bf) AM_RAM //printer port
	AM_RANGE(0x03f2, 0x03f2) AM_WRITE(fdc_dor_w)
	AM_RANGE(0x03f4, 0x03f4) AM_READ(fdc765_status_r) //765 Floppy Disk Controller (FDC) Status
	AM_RANGE(0x03f5, 0x03f5) AM_READWRITE(fdc765_data_r,fdc765_data_w)//FDC Data
	AM_RANGE(0x03f8, 0x03ff) AM_RAM //rs232c (serial) port
ADDRESS_MAP_END

static ADDRESS_MAP_START( filetto_io, AS_IO, 8, pcxt_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3ff)
	AM_IMPORT_FROM( pcxt_io_common )
//  AM_RANGE(0x0200, 0x020f) AM_RAM //game port
	AM_RANGE(0x0201, 0x0201) AM_READ_PORT("COIN") //game port
	AM_RANGE(0x0310, 0x0311) AM_READWRITE(disk_iobank_r,disk_iobank_w) //Prototyping card
	AM_RANGE(0x0312, 0x0312) AM_READ_PORT("IN0") //Prototyping card,read only
ADDRESS_MAP_END

static ADDRESS_MAP_START( tetriskr_io, AS_IO, 8, pcxt_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3ff)
	AM_IMPORT_FROM( pcxt_io_common )
	AM_RANGE(0x0200, 0x020f) AM_RAM //game port
	AM_RANGE(0x03c8, 0x03c8) AM_READ_PORT("IN0")
	AM_RANGE(0x03c9, 0x03c9) AM_READ_PORT("IN1")
//  AM_RANGE(0x03ce, 0x03ce) AM_READ_PORT("IN1") //read then discarded?
ADDRESS_MAP_END

static INPUT_PORTS_START( filetto )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Extra Play" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, "Play at 6th match reached" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "pcvideo_cga_config" )
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( tetriskr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) //probably unused
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
	PORT_START("IN1") //dip-switches
	PORT_DIPNAME( 0x03, 0x03, "Starting Level" )
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x04, "Starting Bomb" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coinage ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) ) duplicate
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START( "pcvideo_cga_config" )
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void pcxt_state::machine_reset()
{
	m_bank = -1;
	m_lastvalue = -1;

	m_pc_spkrdata = 0;
	m_pit_out2 = 1;
	m_wss2_data = 0;
	m_speaker->level_w(0);
}

SLOT_INTERFACE_START( filetto_isa8_cards )
	SLOT_INTERFACE_INTERNAL("filetto",  ISA8_CGA_FILETTO)
	SLOT_INTERFACE_INTERNAL("tetriskr", ISA8_CGA_TETRISKR)
SLOT_INTERFACE_END


static MACHINE_CONFIG_FRAGMENT(pcxt)
	MCFG_CPU_ADD("maincpu", I8088, XTAL_14_31818MHz/3)
	MCFG_CPU_PROGRAM_MAP(filetto_map)
	MCFG_CPU_IO_MAP(filetto_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_1", pic8259_device, inta_cb)

	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_14_31818MHz/12) /* heartbeat IRQ */
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("pic8259_1", pic8259_device, ir0_w))
	MCFG_PIT8253_CLK1(XTAL_14_31818MHz/12) /* dram refresh */
	MCFG_PIT8253_CLK2(XTAL_14_31818MHz/12) /* pio port c pin 4, and speaker polling enough */
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(pcxt_state, ibm5150_pit8253_out2_changed))

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(pcxt_state, port_a_r))
	MCFG_I8255_IN_PORTB_CB(READ8(pcxt_state, port_b_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(pcxt_state, port_b_w))
	MCFG_I8255_IN_PORTC_CB(READ8(pcxt_state, port_c_r))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(pcxt_state, wss_1_w))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(pcxt_state, wss_2_w))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(pcxt_state, sys_reset_w))

	MCFG_DEVICE_ADD("dma8237_1", AM9517A, XTAL_14_31818MHz/3)
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(pcxt_state, pc_dma_hrq_changed))
	MCFG_I8237_IN_MEMR_CB(READ8(pcxt_state, pc_dma_read_byte))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(pcxt_state, pc_dma_write_byte))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(pcxt_state, pc_dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(pcxt_state, pc_dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(pcxt_state, pc_dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(pcxt_state, pc_dack3_w))

	MCFG_PIC8259_ADD( "pic8259_1", INPUTLINE("maincpu", 0), VCC, NULL )

	MCFG_DEVICE_ADD("isa", ISA8, 0)
	MCFG_ISA8_CPU(":maincpu")
	MCFG_ISA_OUT_IRQ2_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir2_w))
	MCFG_ISA_OUT_IRQ3_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir3_w))
	MCFG_ISA_OUT_IRQ4_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir4_w))
	MCFG_ISA_OUT_IRQ5_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir5_w))
	MCFG_ISA_OUT_IRQ6_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir6_w))
	MCFG_ISA_OUT_IRQ7_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir7_w))
	MCFG_ISA_OUT_DRQ1_CB(DEVWRITELINE("dma8237_1", am9517a_device, dreq1_w))
	MCFG_ISA_OUT_DRQ2_CB(DEVWRITELINE("dma8237_1", am9517a_device, dreq2_w))
	MCFG_ISA_OUT_DRQ3_CB(DEVWRITELINE("dma8237_1", am9517a_device, dreq3_w))

	/*Sound Hardware*/
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("voice", HC55516, 8000000/4)//8923S-UM5100 is a HC55536 with ROM hook-up
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

//  PC "buzzer" sound
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( filetto, pcxt_state )
	MCFG_FRAGMENT_ADD( pcxt )
	MCFG_ISA8_SLOT_ADD("isa", "isa1", filetto_isa8_cards, "filetto", true)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( tetriskr, pcxt_state )
	MCFG_FRAGMENT_ADD( pcxt )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(tetriskr_io)

	MCFG_ISA8_SLOT_ADD("isa", "isa1", filetto_isa8_cards, "tetriskr", true)

	MCFG_DEVICE_REMOVE("voice")
MACHINE_CONFIG_END

ROM_START( filetto )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD("u49.bin", 0xfc000, 0x2000, CRC(1be6948a) SHA1(9c433f63d347c211ee4663f133e8417221bc4bf0))
	ROM_RELOAD(         0xf8000, 0x2000 )
	ROM_RELOAD(         0xf4000, 0x2000 )
	ROM_RELOAD(         0xf0000, 0x2000 )
	ROM_LOAD("u55.bin", 0xfe000, 0x2000, CRC(1e455ed7) SHA1(786d18ce0ab1af45fc538a2300853e497488f0d4) )
	ROM_RELOAD(         0xfa000, 0x2000 )
	ROM_RELOAD(         0xf6000, 0x2000 )
	ROM_RELOAD(         0xf2000, 0x2000 )

	ROM_REGION( 0x40000, "game_prg", 0 ) // program data
	ROM_LOAD( "m0.u1", 0x00000, 0x10000, CRC(2408289d) SHA1(eafc144a557a79b58bcb48545cb9c9778e61fcd3) )
	ROM_LOAD( "m1.u2", 0x10000, 0x10000, CRC(5b623114) SHA1(0d9a14e6b7f57ce4fa09762343b610a973910f58) )
	ROM_LOAD( "m2.u3", 0x20000, 0x10000, CRC(abc64869) SHA1(564fc9d90d241a7b7776160b3fd036fb08037355) )
	ROM_LOAD( "m3.u4", 0x30000, 0x10000, CRC(0c1e8a67) SHA1(f1b9280c65fcfcb5ec481cae48eb6f52d6cdbc9d) )

	ROM_REGION( 0x40000, "samples", 0 ) // UM5100 sample roms?
	ROM_LOAD16_BYTE("v1.u15",  0x00000, 0x20000, CRC(613ddd07) SHA1(ebda3d559315879819cb7034b5696f8e7861fe42) )
	ROM_LOAD16_BYTE("v2.u14",  0x00001, 0x20000, CRC(427e012e) SHA1(50514a6307e63078fe7444a96e39d834684db7df) )
ROM_END

ROM_START( tetriskr )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* code */
	ROM_LOAD( "b-10.u10", 0xf0000, 0x10000, CRC(efc2a0f6) SHA1(5f0f1e90237bee9b78184035a32055b059a91eb3) )
ROM_END

DRIVER_INIT_MEMBER(pcxt_state,filetto)
{
	//...
}

DRIVER_INIT_MEMBER(pcxt_state,tetriskr)
{
	//...
}

GAME( 1990, filetto,  0, filetto,  filetto, pcxt_state,  filetto,  ROT0,  "Novarmatic", "Filetto (v1.05 901009)",MACHINE_IMPERFECT_SOUND )
GAME( 1988?,tetriskr, 0, tetriskr, tetriskr, pcxt_state, tetriskr, ROT0,  "bootleg",    "Tetris (bootleg of Mirrorsoft PC-XT Tetris version)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
