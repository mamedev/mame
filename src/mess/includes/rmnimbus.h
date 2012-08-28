/*
    rmnimbus.c
    Machine driver for the Research Machines Nimbus.

    Phill Harvey-Smith
    2009-11-29.
*/

#include "machine/z80sio.h"
#include "machine/wd17xx.h"
#include "machine/scsibus.h"
#include "machine/6522via.h"

#define MAINCPU_TAG "maincpu"
#define IOCPU_TAG   "iocpu"

#define num_ioports 			0x80
#define NIMBUS_KEYROWS      	11
#define KEYBOARD_QUEUE_SIZE 	32

#define SCREEN_WIDTH_PIXELS     640
#define SCREEN_HEIGHT_LINES     250
#define SCREEN_NO_COLOURS       16

#define NO_VIDREGS      		(0x30/2)

/* Nimbus sub-bios structures for debugging */

typedef struct
{
	UINT16  ofs_brush;
	UINT16  seg_brush;
	UINT16  ofs_data;
	UINT16  seg_data;
	UINT16  count;
} t_area_params;

typedef struct
{
	UINT16  ofs_font;
	UINT16  seg_font;
	UINT16  ofs_data;
	UINT16  seg_data;
	UINT16  x;
	UINT16  y;
	UINT16  length;
} t_plot_string_params;

typedef struct
{
	UINT16  style;
	UINT16  style_index;
	UINT16  colour1;
	UINT16  colour2;
	UINT16  transparency;
	UINT16  boundary_spec;
	UINT16  boundary_colour;
	UINT16  save_colour;
} t_nimbus_brush;

#define SCREEN_WIDTH_PIXELS     640
#define SCREEN_HEIGHT_LINES     250
#define SCREEN_NO_COLOURS       16


/* 80186 internal stuff */
struct mem_state
{
	UINT16	    lower;
	UINT16	    upper;
	UINT16	    middle;
	UINT16	    middle_size;
	UINT16	    peripheral;
};

struct timer_state
{
	UINT16	    control;
	UINT16	    maxA;
	UINT16	    maxB;
	UINT16	    count;
	emu_timer   *int_timer;
	emu_timer   *time_timer;
	UINT8	    time_timer_active;
	attotime	last_time;
};

struct dma_state
{
	UINT32	    source;
	UINT32	    dest;
	UINT16	    count;
	UINT16	    control;
	UINT8	    finished;
	emu_timer   *finish_timer;
};

struct intr_state
{
	UINT8	pending;
	UINT16	ack_mask;
	UINT16	priority_mask;
	UINT16	in_service;
	UINT16	request;
	UINT16	status;
	UINT16	poll_status;
	UINT16	timer;
	UINT16	dma[2];
	UINT16	ext[4];
	UINT16  ext_vector[2]; // external vectors, when in cascade mode
};

typedef struct
{
	struct timer_state	timer[3];
	struct dma_state	dma[2];
	struct intr_state	intr;
	struct mem_state	mem;
} i186_state;

typedef struct
{
	UINT8       keyrows[NIMBUS_KEYROWS];
	emu_timer   *keyscan_timer;
	UINT8       queue[KEYBOARD_QUEUE_SIZE];
	UINT8       head;
	UINT8       tail;
}  keyboard_t;

// Static data related to Floppy and SCSI hard disks
typedef struct
{
	UINT8   reg400;
	UINT8   reg410_in;
	UINT8   reg410_out;
	UINT8   reg418;

	UINT8   drq_ff;
	UINT8   int_ff;
} nimbus_drives_t;

/* 8031 Peripheral controler */
typedef struct
{
	UINT8   ipc_in;
	UINT8   ipc_out;
	UINT8   status_in;
	UINT8   status_out;
	UINT8   int_8c_pending;
	UINT8   int_8e_pending;
	UINT8   int_8f_pending;
} ipc_interface_t;

/* Mouse/Joystick */
typedef struct
{
	UINT8   m_mouse_px;
	UINT8   m_mouse_py;

	UINT8   m_mouse_x;
	UINT8   m_mouse_y;
	UINT8   m_mouse_pc;
	UINT8   m_mouse_pcx;
	UINT8   m_mouse_pcy;

	UINT8   m_intstate_x;
	UINT8   m_intstate_y;

	UINT8   m_reg0a4;

	emu_timer   *m_mouse_timer;
} _mouse_joy_state;


class rmnimbus_state : public driver_device
{
public:
	rmnimbus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT32 m_debug_machine;
	i186_state m_i186;
	keyboard_t m_keyboard;
	nimbus_drives_t m_nimbus_drives;
	ipc_interface_t m_ipc_interface;
	UINT8 m_mcu_reg080;
	UINT8 m_iou_reg092;
	UINT8 m_last_playmode;
	_mouse_joy_state m_nimbus_mouse;
	UINT8 m_ay8910_a;
	UINT16 m_IOPorts[num_ioports];
	UINT8 m_sio_int_state;
	UINT8 m_video_mem[SCREEN_WIDTH_PIXELS][SCREEN_HEIGHT_LINES];
	UINT16 m_vidregs[NO_VIDREGS];
	UINT8 m_bpp;
	UINT16 m_pixel_mask;
	UINT8 m_hs_count;
	UINT32 m_debug_video;
	DECLARE_READ16_MEMBER(nimbus_i186_internal_port_r);
	DECLARE_WRITE16_MEMBER(nimbus_i186_internal_port_w);
	DECLARE_READ8_MEMBER(nimbus_mcu_r);
	DECLARE_WRITE8_MEMBER(nimbus_mcu_w);
	DECLARE_READ16_MEMBER(nimbus_io_r);
	DECLARE_WRITE16_MEMBER(nimbus_io_w);
	DECLARE_READ8_MEMBER(nimbus_disk_r);
	DECLARE_WRITE8_MEMBER(nimbus_disk_w);
	DECLARE_READ8_MEMBER(nimbus_pc8031_r);
	DECLARE_WRITE8_MEMBER(nimbus_pc8031_w);
	DECLARE_READ8_MEMBER(nimbus_pc8031_iou_r);
	DECLARE_WRITE8_MEMBER(nimbus_pc8031_iou_w);
	DECLARE_READ8_MEMBER(nimbus_pc8031_port_r);
	DECLARE_WRITE8_MEMBER(nimbus_pc8031_port_w);
	DECLARE_READ8_MEMBER(nimbus_iou_r);
	DECLARE_WRITE8_MEMBER(nimbus_iou_w);
	DECLARE_READ8_MEMBER(nimbus_sound_ay8910_r);
	DECLARE_WRITE8_MEMBER(nimbus_sound_ay8910_w);
	DECLARE_WRITE8_MEMBER(nimbus_sound_ay8910_porta_w);
	DECLARE_WRITE8_MEMBER(nimbus_sound_ay8910_portb_w);
	DECLARE_READ8_MEMBER(nimbus_mouse_js_r);
	DECLARE_WRITE8_MEMBER(nimbus_mouse_js_w);
	DECLARE_READ16_MEMBER(nimbus_video_io_r);
	DECLARE_WRITE16_MEMBER(nimbus_video_io_w);
	DECLARE_DRIVER_INIT(nimbus);
};


/*----------- defined in drivers/rmnimbus.c -----------*/

extern const unsigned char nimbus_palette[SCREEN_NO_COLOURS][3];


/*----------- defined in machine/rmnimbus.c -----------*/

MACHINE_RESET(nimbus);
MACHINE_START(nimbus);

/* 80186 Internal */

/* external int priority masks */

#define EXTINT_CTRL_PRI_MASK    0x07
#define EXTINT_CTRL_MSK         0x08
#define EXTINT_CTRL_LTM         0x10
#define EXTINT_CTRL_CASCADE     0x20
#define EXTINT_CTRL_SFNM        0x40

/* DMA control register */

#define DEST_MIO                0x8000
#define DEST_DECREMENT          0x4000
#define DEST_INCREMENT          0x2000
#define DEST_NO_CHANGE          (DEST_DECREMENT | DEST_INCREMENT)
#define DEST_INCDEC_MASK        (DEST_DECREMENT | DEST_INCREMENT)
#define SRC_MIO                 0X1000
#define SRC_DECREMENT           0x0800
#define SRC_INCREMENT           0x0400
#define SRC_NO_CHANGE           (SRC_DECREMENT | SRC_INCREMENT)
#define SRC_INCDEC_MASK         (SRC_DECREMENT | SRC_INCREMENT)
#define TERMINATE_ON_ZERO       0x0200
#define INTERRUPT_ON_ZERO       0x0100
#define SYNC_MASK               0x00C0
#define SYNC_SOURCE             0x0040
#define SYNC_DEST               0x0080
#define CHANNEL_PRIORITY        0x0020
#define TIMER_DRQ               0x0010
#define CHG_NOCHG               0x0004
#define ST_STOP                 0x0002
#define BYTE_WORD               0x0001

/* Nimbus specific */

/* External int vectors for chained interupts */
#define EXTERNAL_INT_DISK       0x80
#define EXTERNAL_INT_MSM5205    0x84
#define EXTERNAL_INT_MOUSE_YU   0x88
#define EXTERNAL_INT_MOUSE_YD   0x89
#define EXTERNAL_INT_MOUSE_XL   0x8A
#define EXTERNAL_INT_MOUSE_XR   0x8B
#define EXTERNAL_INT_PC8031_8C  0x8c
#define EXTERNAL_INT_PC8031_8E  0x8E
#define EXTERNAL_INT_PC8031_8F  0x8F
#define EXTERNAL_INT_Z80SIO     0x9C


/* Memory controler */
#define RAM_BANK00_TAG  "bank0"
#define RAM_BANK01_TAG  "bank1"
#define RAM_BANK02_TAG  "bank2"
#define RAM_BANK03_TAG  "bank3"
#define RAM_BANK04_TAG  "bank4"
#define RAM_BANK05_TAG  "bank5"
#define RAM_BANK06_TAG  "bank6"
#define RAM_BANK07_TAG  "bank7"

#define HIBLOCK_BASE_MASK   0x08
#define HIBLOCK_SELECT_MASK 0x10



/* Z80 SIO for keyboard */

#define Z80SIO_TAG		    "z80sio"

extern const z80sio_interface nimbus_sio_intf;

/* Floppy/Fixed drive interface */

#define FDC_TAG                 "wd2793"
#define FDC_PAUSE				10000

extern const wd17xx_interface nimbus_wd17xx_interface;


#define NO_DRIVE_SELECTED   0xFF

/* SASI harddisk interface */
#define SCSIBUS_TAG             "scsibus"

void nimbus_scsi_linechange(device_t *device, UINT8 line, UINT8 state);

/* Masks for writes to port 0x400 */
#define FDC_DRIVE0_MASK 0x01
#define FDC_DRIVE1_MASK 0x02
#define FDC_DRIVE2_MASK 0x04
#define FDC_DRIVE3_MASK 0x08
#define FDC_SIDE_MASK   0x10
#define FDC_MOTOR_MASKO 0x20
#define HDC_DRQ_MASK    0x40
#define FDC_DRQ_MASK    0x80
#define FDC_DRIVE_MASK  (FDC_DRIVE0_MASK | FDC_DRIVE1_MASK | FDC_DRIVE2_MASK | FDC_DRIVE3_MASK)

#define FDC_SIDE()          ((state->m_nimbus_drives.reg400 & FDC_SIDE_MASK) >> 4)
#define FDC_MOTOR()         ((state->m_nimbus_drives.reg400 & FDC_MOTOR_MASKO) >> 5)
#define FDC_DRIVE()         (fdc_driveno(state->m_nimbus_drives.reg400 & FDC_DRIVE_MASK))
#define HDC_DRQ_ENABLED()   ((state->m_nimbus_drives.reg400 & HDC_DRQ_MASK) ? 1 : 0)
#define FDC_DRQ_ENABLED(state)   ((state->m_nimbus_drives.reg400 & FDC_DRQ_MASK) ? 1 : 0)

/* Masks for port 0x410 read*/

#define FDC_READY_MASK  0x01
#define FDC_INDEX_MASK  0x02
#define FDC_MOTOR_MASKI 0x04
#define HDC_MSG_MASK    0x08
#define HDC_BSY_MASK    0x10
#define HDC_IO_MASK     0X20
#define HDC_CD_MASK     0x40
#define HDC_REQ_MASK    0x80

#define FDC_BITS_410    (FDC_READY_MASK | FDC_INDEX_MASK | FDC_MOTOR_MASKI)
#define HDC_BITS_410    (HDC_MSG_MASK | HDC_BSY_MASK | HDC_IO_MASK | HDC_CD_MASK | HDC_REQ_MASK)
#define INV_BITS_410    (HDC_BSY_MASK | HDC_IO_MASK | HDC_CD_MASK | HDC_REQ_MASK)

#define HDC_INT_TRIGGER (HDC_IO_MASK | HDC_CD_MASK | HDC_REQ_MASK)

/* Masks for port 0x410 write*/

#define HDC_RESET_MASK  0x01
#define HDC_SEL_MASK    0x02
#define HDC_IRQ_MASK    0x04
#define HDC_IRQ_ENABLED(state)   ((state->m_nimbus_drives.reg410_out & HDC_IRQ_MASK) ? 1 : 0)


#define SCSI_ID_NONE    0x80


/* 8031/8051 Peripheral controler */

#define IPC_OUT_ADDR        0X01
#define IPC_OUT_READ_PEND   0X02
#define IPC_OUT_BYTE_AVAIL  0X04

#define IPC_IN_ADDR         0X01
#define IPC_IN_BYTE_AVAIL   0X02
#define IPC_IN_READ_PEND    0X04




#define ER59256_TAG             "er59256"

/* IO unit */

#define DISK_INT_ENABLE         0x01
#define MSM5205_INT_ENABLE      0x04
#define MOUSE_INT_ENABLE        0x08
#define PC8031_INT_ENABLE       0x10


/* Sound hardware */

#define AY8910_TAG              "ay8910"
#define MONO_TAG                "mono"




#define MSM5205_TAG             "msm5205"

void nimbus_msm5205_vck(device_t *device);

/* Mouse / Joystick */

#define JOYSTICK0_TAG           "joystick0"
#define MOUSE_BUTTON_TAG        "mousebtn"
#define MOUSEX_TAG              "mousex"
#define MOUSEY_TAG              "mousey"

enum
{
	MOUSE_PHASE_STATIC = 0,
	MOUSE_PHASE_POSITIVE,
	MOUSE_PHASE_NEGATIVE
};


#define MOUSE_INT_ENABLED(state)     (((state)->m_iou_reg092 & MOUSE_INT_ENABLE) ? 1 : 0)

/* Paralell / User port BBC compatible ! */

#define VIA_TAG					"via6522"
#define CENTRONICS_TAG			"centronics"

#define	VIA_INT					0x03

extern const via6522_interface nimbus_via;

WRITE_LINE_DEVICE_HANDLER(nimbus_ack_w);


/*----------- defined in video/rmnimbus.c -----------*/


VIDEO_START( nimbus );
SCREEN_VBLANK( nimbus );
SCREEN_UPDATE_IND16( nimbus );
VIDEO_RESET( nimbus );

#define RED                     0
#define GREEN                   1
#define BLUE                    2

#define LINEAR_ADDR(seg,ofs)    ((seg<<4)+ofs)

#define OUTPUT_SEGOFS(mess,seg,ofs)  logerror("%s=%04X:%04X [%08X]\n",mess,seg,ofs,((seg<<4)+ofs))
