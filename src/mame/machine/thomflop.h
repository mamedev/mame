// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Thomson 8-bit computers

**********************************************************************/

#ifndef THOMFLOP_H_
#define THOMFLOP_H_

#include "image.h"
#include "imagedev/flopdrv.h"
#include "machine/mc6843.h"
#include "machine/mc6854.h"

extern uint8_t to7_controller_type; /* set during init */
extern uint8_t to7_floppy_bank;

/* number of external floppy controller ROM banks */
#define TO7_NB_FLOP_BANK 9

/* external floppy / network controller active */
#define THOM_FLOPPY_EXT (to7_controller_type >= 1)

/* internal floppy controller active (no or network extension) */
#define THOM_FLOPPY_INT (to7_controller_type == 0 || to7_controller_type > 4)


/* external controllers */
/* TO9 internal (WD2793) & external controllers */
/* TO8 internal (THMFC1) controller */

class thmfc1_device : public device_t
{
public:
	// STAT0 flags
	enum : uint8_t
	{
		STAT0_SYNCHRO        = 0x01, // bit clock synchronized
		STAT0_BYTE_READY_OP  = 0x02, // byte ready (high-level operation)
		STAT0_CRC_ERROR      = 0x04,
		STAT0_FINISHED       = 0x08,
		STAT0_FINISHING      = 0x10, // (unemulated)
		STAT0_BYTE_READY_POL = 0x80  // polling mode
	};

	thmfc1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto floppy_active_cb() { return m_floppy_active_cb.bind(); }

	uint8_t floppy_r(offs_t offset);
	void floppy_w(offs_t offset, uint8_t data);

	void index_pulse_cb( int index, int state );
	void floppy_reset();

protected:
	virtual void device_resolve_objects() override;
	virtual void device_start() override;

private:
	// types of high-level operations
	enum thmfc1_op : uint8_t
	{
		OP_RESET         = 0,
		OP_WRITE_SECT    = 1,
		OP_READ_ADDR     = 2,
		OP_READ_SECT     = 3
	};

	TIMER_CALLBACK_MEMBER( floppy_cmd_complete_cb );
	legacy_floppy_image_device *get_floppy_image();
	bool floppy_is_qdd( legacy_floppy_image_device *image ) const;
	int floppy_find_sector( chrn_id* dst );
	void floppy_cmd_complete();
	uint8_t floppy_read_byte();
	uint8_t floppy_raw_read_byte();
	void floppy_qdd_write_byte( uint8_t data );
	void floppy_write_byte( uint8_t data );
	void floppy_format_byte( uint8_t data );

	required_device_array<legacy_floppy_image_device, 4> m_floppy_image;

	devcb_write_line m_floppy_active_cb;

	thmfc1_op m_op;
	uint8_t   m_sector;                  // target sector, in [1,16]
	uint32_t  m_sector_id;
	uint8_t   m_track;                   // current track, in [0,79]
	uint8_t   m_side;                    // current side, 0 or 1
	uint8_t   m_drive;                   // 0 to 3
	uint16_t  m_sector_size;             // 128 or 256 (512, 1024 not supported)
	uint8_t   m_formatting;
	uint8_t   m_ipl;                     // index pulse / QDD start
	uint8_t   m_wsync;                   // synchronization word
	int       m_motor_on;

	std::unique_ptr<uint8_t[]> m_data;   // enough for a whole track
	uint32_t  m_data_idx;                // reading / writing / formatting pos
	uint32_t  m_data_size;               // bytes to read / write
	uint32_t  m_data_finish;             // when to raise the finished flag
	uint32_t  m_data_raw_idx;            // byte index for raw track reading
	uint32_t  m_data_raw_size;           // size of track already cached in data
	uint8_t   m_data_crc;                // check-sum of written data

	uint8_t   m_stat0;                   // status register

	emu_timer *m_floppy_cmd;
};

DECLARE_DEVICE_TYPE(THMFC1, thmfc1_device)

#endif /* THOMFLOP_H_ */
