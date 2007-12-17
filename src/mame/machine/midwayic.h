/***************************************************************************

    Emulation of various Midway ICs

***************************************************************************/


/* 1st generation Midway serial PIC */
void midway_serial_pic_init(int upper);
void midway_serial_pic_reset_w(int state);
UINT8 midway_serial_pic_status_r(void);
UINT8 midway_serial_pic_r(void);
void midway_serial_pic_w(UINT8 data);


/* 2nd generation Midway serial/NVRAM/RTC PIC */
void midway_serial_pic2_init(int upper, int yearoffs);
void midway_serial_pic2_set_default_nvram(const UINT8 *nvram);
UINT8 midway_serial_pic2_status_r(void);
UINT8 midway_serial_pic2_r(void);
void midway_serial_pic2_w(UINT8 data);
NVRAM_HANDLER( midway_serial_pic2 );


/* I/O ASIC connected to 2nd generation PIC */
void midway_ioasic_init(int shuffle, int upper, int yearoffs, void (*irq_callback)(int));
void midway_ioasic_set_auto_ack(int auto_ack);
void midway_ioasic_set_shuffle_state(int state);
void midway_ioasic_reset(void);
void midway_ioasic_fifo_w(UINT16 data);
void midway_ioasic_fifo_reset_w(int state);
void midway_ioasic_fifo_full_w(UINT16 data);
READ32_HANDLER( midway_ioasic_r );
WRITE32_HANDLER( midway_ioasic_w );
READ32_HANDLER( midway_ioasic_packed_r );
WRITE32_HANDLER( midway_ioasic_packed_w );

enum
{
	MIDWAY_IOASIC_STANDARD = 0,
	MIDWAY_IOASIC_BLITZ99,
	MIDWAY_IOASIC_CARNEVIL,
	MIDWAY_IOASIC_CALSPEED,
	MIDWAY_IOASIC_MACE,
	MIDWAY_IOASIC_GAUNTDL,
	MIDWAY_IOASIC_VAPORTRX,
	MIDWAY_IOASIC_SFRUSHRK,
	MIDWAY_IOASIC_HYPRDRIV
};



/* IDE ASIC maps the IDE registers */
READ32_HANDLER( midway_ide_asic_r );
WRITE32_HANDLER( midway_ide_asic_w );
