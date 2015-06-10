// license:BSD-3-Clause
// copyright-holders:Raphael Nabet, Michael Zapf
/* Interface */

#ifndef __SMC92X4_H__
#define __SMC92X4_H__

#include "ti99_hd.h"
#include "imagedev/flopdrv.h"
#include "formats/imageutl.h"
#include "imagedev/harddriv.h"
#include "harddisk.h"

#define INPUT_STATUS    0x00
#define OUTPUT_DMA_ADDR 0x01
#define OUTPUT_OUTPUT1  0x02
#define OUTPUT_OUTPUT2  0x03

#define MFMHD_TRACK00   0x80
#define MFMHD_SEEKCOMP  0x40
#define MFMHD_WRFAULT   0x20
#define MFMHD_READY 0x10
#define MFMHD_INDEX 0x08

#define BAD_SECTOR  0x1000
#define BAD_CRC     0x2000

extern const device_type SMC92X4;
/*
    Definition of bits in the Disk-Status register
*/
#define DS_ECCERR   0x80        /* ECC error */
#define DS_INDEX    0x40        /* index point */
#define DS_SKCOM    0x20        /* seek complete */
#define DS_TRK00    0x10        /* track 0 */
#define DS_UDEF     0x08        /* user-defined */
#define DS_WRPROT   0x04        /* write protect */
#define DS_READY    0x02        /* drive ready bit */
#define DS_WRFAULT  0x01        /* write fault */

// Interrupt line. To be connected with the controller PCB.
#define MCFG_SMC92X4_INTRQ_CALLBACK(_write) \
	devcb = &smc92x4_device::set_intrq_wr_callback(*device, DEVCB_##_write);

// DMA in progress line. To be connected with the controller PCB.
#define MCFG_SMC92X4_DIP_CALLBACK(_write) \
	devcb = &smc92x4_device::set_dip_wr_callback(*device, DEVCB_##_write);

// Auxiliary Bus. These 8 lines need to be connected to external latches
// and to a counter circuitry which works together with the external RAM.
// We use the S0/S1 lines as address lines.
#define MCFG_SMC92X4_AUXBUS_OUT_CALLBACK(_write) \
	devcb = &smc92x4_device::set_auxbus_wr_callback(*device, DEVCB_##_write);

// Auxiliary Bus. This is only used for S0=S1=0.
#define MCFG_SMC92X4_AUXBUS_IN_CALLBACK(_read) \
	devcb = &smc92x4_device::set_auxbus_rd_callback(*device, DEVCB_##_read);

// Callback to read the contents of the external RAM via the data bus.
// Note that the address must be set and automatically increased
// by external circuitry.
#define MCFG_SMC92X4_DMA_IN_CALLBACK(_read) \
	devcb = &smc92x4_device::set_dma_rd_callback(*device, DEVCB_##_read);

// Callback to write the contents of the external RAM via the data bus.
// Note that the address must be set and automatically increased
// by external circuitry. */
#define MCFG_SMC92X4_DMA_OUT_CALLBACK(_write) \
	devcb = &smc92x4_device::set_dma_wr_callback(*device, DEVCB_##_write);

// Disk format support. This flag allows to choose between the full
// FM/MFM format and an abbreviated track layout. The difference results
// from legal variations of the layout. This is not part of
// the smc92x4 specification, but it allows to keep the image format
// simple without too much case checking. Should be removed as soon as
// the respective disk formats support the full format.
#define MCFG_SMC92X4_FULL_TRACK_LAYOUT(_lay) \
	smc92x4_device::set_full_track_layout(*device, _lay);


class smc92x4_device : public device_t
{
public:
	smc92x4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_intrq_wr_callback(device_t &device, _Object object) { return downcast<smc92x4_device &>(device).m_out_intrq.set_callback(object); }
	template<class _Object> static devcb_base &set_dip_wr_callback(device_t &device, _Object object) { return downcast<smc92x4_device &>(device).m_out_dip.set_callback(object); }
	template<class _Object> static devcb_base &set_auxbus_wr_callback(device_t &device, _Object object) { return downcast<smc92x4_device &>(device).m_out_auxbus.set_callback(object); }
	template<class _Object> static devcb_base &set_auxbus_rd_callback(device_t &device, _Object object) { return downcast<smc92x4_device &>(device).m_in_auxbus.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_rd_callback(device_t &device, _Object object) { return downcast<smc92x4_device &>(device).m_in_dma.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_wr_callback(device_t &device, _Object object) { return downcast<smc92x4_device &>(device).m_out_dma.set_callback(object); }

	static void set_full_track_layout(device_t &device, bool lay) { downcast<smc92x4_device &>(device).m_full_track_layout = lay; }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	// Used to turn off the delays.
	void set_timing(bool realistic);

	// Used to reconfigure the drive connections. Drive selection is done
	// using the select lines and maybe also the user-programmable outputs.
	void connect_floppy_drive(legacy_floppy_image_device *drive);
	void connect_hard_drive(mfm_harddisk_legacy_device *drive);

	void reset();

protected:
	virtual void device_start(void);
	virtual void device_reset(void);

private:
	devcb_write_line   m_out_intrq;    // INT line
	devcb_write_line   m_out_dip;      // DMA in progress line
	devcb_write8       m_out_auxbus;   // AB0-7 lines (using S0,S1 as address)
	devcb_read8        m_in_auxbus;    // AB0-7 lines (S0=S1=0)
	devcb_read8        m_in_dma;       // DMA read access to the cache buffer
	devcb_write8       m_out_dma;      // DMA write access to the cache buffer

	UINT8 m_output1;        // internal register "output1"
	UINT8 m_output2;        // internal register "output2"

	void    set_dip(line_state value);
	int     image_is_single_density();
	bool    in_single_density_mode();
	void    copyid(chrn_id id1, chrn_id_hd *id2);
	void    set_interrupt();
	void    clear_interrupt();
	void    set_command_done(int flags);
	void    set_command_done();
	void    set_dma_address(int pos2316, int pos1508, int pos0700);
	void    dma_add_offset(int offset);
	void    sync_status_in();
	void    sync_latches_out();
	void    timed_sector_read_request();
	void    timed_sector_write_request();
	void    timed_track_request();
	void    timed_seek_request();
	virtual void    device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	UINT8   cylinder_to_ident(int cylinder);
	int     ident_to_cylinder(UINT8 ident);
	void    update_id_regs(chrn_id_hd id);
	void    read_id_field(int *steps, int *direction, chrn_id_hd *id);
	int     verify(chrn_id_hd *id, bool check_sector);
	void    data_transfer_read(chrn_id_hd id, int transfer_enable);
	void    data_transfer_write(chrn_id_hd id, int deldata, int redcur, int precomp, bool write_long);
	void    read_write_sectors();
	void    read_sectors_continue();
	void    write_sectors_continue();

	void    restore_drive();
	void    restore_continue();

	void    step_in_out();
	void    step_continue();

	void    poll_drives();

	void    drive_select(int driveparm);

	void    seek_read_id();
	void    seek_read_id_continue();

	void    format_track();
	void    format_track_continue();
	void    format_floppy_track(int flags);
	void    format_harddisk_track(int flags);

	void    read_track();
	void    read_track_continue();
	void    read_floppy_track(bool transfer_only_ids);
	void    read_harddisk_track(bool transfer_only_ids);

	void    process_after_callback();
	void    process_command(UINT8 opcode);

	int m_selected_drive_type;
	// We need to store the types, although this is not the case in the
	// real hardware. The reason is that poll_drives wants to select the
	// drives without knowing their kinds. In reality this does not matter,
	// since we only check for the seek_complete line, but in our case,
	// floppy and harddrive are pretty different, and we need to know which
	// one to check.
	int m_types[4];
	int m_head_load_delay_enable;
	int m_register_pointer;

	UINT8 m_register_r[12];
	UINT8 m_register_w[12];

	/* timers to delay execution/completion of commands */
	emu_timer *m_timer_data, *m_timer_rs, *m_timer_ws, *m_timer_seek, *m_timer_track;

	/* Flag which determines whether to use realistic timing. */
	bool m_use_real_timing;

	/* Required to store the current iteration within restore. */
	int m_seek_count;

	/* Recent command. */
	UINT8 m_command;

	/* Stores the step direction. */
	int m_step_direction;

	/* Stores the buffered flag. */
	bool m_buffered;

	/* Stores the recent id field. */
	chrn_id_hd m_recent_id;

	/* Indicates whether an ID has been found. */
	bool m_found_id;

	/* Indicates whether we are already past the seek_id phase in read/write sector. */
	bool m_after_seek;

	/* Determines whether the command will be continued. */
	bool m_to_be_continued;

	bool m_full_track_layout;

	// Link to attached drive (hard / floppy disk)
	// Hard disks must be instances of mfmhd_device; floppy disks are the common drives.
	// We expect the embedding board to replace the drive according to the
	// select lines.
	legacy_floppy_image_device    *m_drive;
	mfm_harddisk_legacy_device    *m_harddisk;
};

#endif
