/////////////////////////////////////////////////////////////////////
//////   HECTOR HEADER FILE /////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/*
        Hector 2HR+
        Victor
        Hector 2HR
        Hector HRX
        Hector MX40c
        Hector MX80c
        Hector 1
        Interact

        12/05/2009 Skeleton driver - Micko : mmicko@gmail.com
        31/06/2009 Video - Robbbert

        29/10/2009 Update skeleton to functional machine
                    by yo_fr            (jj.stac @ aliceadsl.fr)

                => add Keyboard,
                => add color,
                => add cassette,
                => add sn76477 sound and 1bit sound,
                => add joysticks (stick, pot, fire)
                => add BR/HR switching
                => add bank switch for HRX
                => add device MX80c and bank switching for the ROM
        03/01/2010 Update and clean prog  by yo_fr       (jj.stac@aliceadsl.fr)
                => add the port mapping for keyboard
        20/11/2010 : synchronization between uPD765 and Z80 are now OK, CP/M runnig! JJStacino
        11/11/2011 : add the minidisque support -3 pouces 1/2 driver-  JJStacino  (jj.stac @ aliceadsl.fr)

            don't forget to keep some information about these machine see DChector project : http://dchector.free.fr/ made by DanielCoulom
            (and thank's to Daniel!) and Yves site : http://hectorvictor.free.fr/ (thank's too Yves!)

    TODO :  Add the cartridge function,
            Adjust the one shot and A/D timing (sn76477)
*/

#include "machine/upd765.h"
#include "machine/wd17xx.h"

/* Enum status for high memory bank (c000 - ffff)*/
enum
{
	HECTOR_BANK_PROG = 0,				/* first BANK is program ram*/
	HECTOR_BANK_VIDEO					/* second BANK is Video ram */
};
/* Status for rom memory bank (0000 - 3fff) in MX machine*/
enum
{
	HECTORMX_BANK_PAGE0 = 0,			/* first BANK is base rom*/
	HECTORMX_BANK_PAGE1,				/* second BANK is basic rom */
	HECTORMX_BANK_PAGE2					/* 3 BANK is monitrix / assemblex rom */
};
/* Status for rom memory bank (0000 - 3fff) in Mini Disc machine*/
enum
{
	HECTOR_BANK_BASE = 0,				/* first BANK is normal rom*/
	HECTOR_BANK_DISC					/* second BANK is extra rom for mini disc use*/
};
/* Enum status for low memory bank (00000 - 0fff) for DISC II*/
enum
{
	DISCII_BANK_RAM = 0,			/* first BANK is program ram*/
	DISCII_BANK_ROM					/* second BANK is ROM */
};

class hec2hrp_state : public driver_device
{
public:
	hec2hrp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_videoram(*this,"videoram"),
		  m_hector_videoram(*this,"hector_videoram") { }

	optional_shared_ptr<UINT8> m_videoram;
	optional_shared_ptr<UINT8> m_hector_videoram;
	UINT8 m_hector_flag_hr;
	UINT8 m_hector_flag_80c;
	UINT8 m_hector_color[4];
	UINT8 m_hector_disc2_data_r_ready;
	UINT8 m_hector_disc2_data_w_ready;
	UINT8 m_hector_disc2_data_read;
	UINT8 m_hector_disc2_data_write;
	UINT8 m_hector_disc2_RNMI;
	UINT8 m_state3000;
	UINT8 m_write_cassette;
	emu_timer *m_Cassette_timer;
	UINT8 m_CK_signal ;
	UINT8 m_flag_clk;
	double m_Pin_Value[29][2];
	int m_AU[17];
	int m_ValMixer;
	int m_oldstate3000;
	int m_oldstate1000;
	UINT8 m_pot0;
	UINT8 m_pot1;
	UINT8 m_actions;
	UINT8 m_hector_port_a;
	UINT8 m_hector_port_b;
	UINT8 m_hector_port_c_h;
	UINT8 m_hector_port_c_l;
	UINT8 m_hector_port_cmd;
	UINT8 m_cassette_bit;
	UINT8 m_cassette_bit_mem;
	UINT8 m_Data_K7;
	int m_counter_write;
	emu_timer *m_DMA_timer;
	emu_timer *m_INT_timer;
	int m_NMI_current_state;
	int m_hector_cmd[10];
	int m_hector_nb_cde;
	int m_hector_flag_result;
	int m_print;
	UINT8 m_hector_videoram_hrx[0x04000];
	DECLARE_READ8_MEMBER(hector_179x_register_r);
	DECLARE_WRITE8_MEMBER(hector_179x_register_w);
	DECLARE_WRITE8_MEMBER(hector_switch_bank_w);
	DECLARE_WRITE8_MEMBER(hector_keyboard_w);
	DECLARE_READ8_MEMBER(hector_keyboard_r);
	DECLARE_WRITE8_MEMBER(hector_sn_2000_w);
	DECLARE_WRITE8_MEMBER(hector_sn_2800_w);
	DECLARE_READ8_MEMBER(hector_cassette_r);
	DECLARE_WRITE8_MEMBER(hector_sn_3000_w);
	DECLARE_WRITE8_MEMBER(hector_color_a_w);
	DECLARE_WRITE8_MEMBER(hector_color_b_w);
	DECLARE_READ8_MEMBER(hector_io_8255_r);
	DECLARE_WRITE8_MEMBER(hector_io_8255_w);
	DECLARE_WRITE8_MEMBER(hector_mx40_io_port_w);
	DECLARE_WRITE8_MEMBER(hector_mx80_io_port_w);
};

/*----------- defined in machine/hec2hrp.c -----------*/

/* Protoype of memory Handler*/
WRITE8_HANDLER( hector_switch_bank_rom_w );

void hector_init( running_machine &machine);
void hector_reset(running_machine &machine, int hr, int with_D2);
void hector_disc2_reset( running_machine &machine);

/* Prototype of I/O Handler*/
READ8_HANDLER( hector_mx_io_port_r );
/*----------- defined in video/hec2video.c -----------*/

void hector_80c(running_machine &machine, bitmap_ind16 &bitmap, UINT8 *page, int ymax, int yram) ;
void hector_hr(running_machine &machine, bitmap_ind16 &bitmap, UINT8 *page, int ymax, int yram) ;
VIDEO_START( hec2hrp );
SCREEN_UPDATE_IND16( hec2hrp );

/* Sound function*/
extern const sn76477_interface hector_sn76477_interface;

/*----------- defined in machine/hecdisk2.c -----------*/

// disc2 handling
WRITE_LINE_DEVICE_HANDLER( hector_disk2_fdc_interrupt );
READ8_HANDLER(  hector_disc2_io00_port_r);
WRITE8_HANDLER( hector_disc2_io00_port_w);
READ8_HANDLER(  hector_disc2_io20_port_r);
WRITE8_HANDLER( hector_disc2_io20_port_w);
READ8_HANDLER(  hector_disc2_io30_port_r);
WRITE8_HANDLER( hector_disc2_io30_port_w);
READ8_HANDLER(  hector_disc2_io40_port_r);
WRITE8_HANDLER( hector_disc2_io40_port_w);
READ8_HANDLER(  hector_disc2_io50_port_r);
WRITE8_HANDLER( hector_disc2_io50_port_w);
READ8_HANDLER(  hector_disc2_io61_port_r);
WRITE8_HANDLER( hector_disc2_io61_port_w);
READ8_HANDLER(  hector_disc2_io70_port_r);
WRITE8_HANDLER( hector_disc2_io70_port_w);

void hector_disc2_init( running_machine &machine);
void hector_minidisc_init( running_machine &machine);

extern const upd765_interface hector_disc2_upd765_interface;
extern const floppy_interface    hector_disc2_floppy_interface;
extern const wd17xx_interface hector_wd17xx_interface;  // Special for minidisc
extern const floppy_interface minidisc_floppy_interface;
