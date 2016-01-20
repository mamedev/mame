// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*
    990_dk.c: emulation of a TI FD800 'Diablo' floppy disk controller
    controller, for use with any TI990 system (and possibly any system which
    implements the CRU bus).

    This floppy disk controller supports IBM-format 8" SSSD and DSSD floppies.

    Raphael Nabet 2003

    Rewritten as class
    Michael Zapf 2014

    TODO: Make it work
*/

#include "emu.h"

#include "990_dk.h"

/* status bits */
enum
{
	status_OP_complete  = 1 << 0,
	status_XFER_ready   = 1 << 1,
	status_drv_not_ready= 1 << 2,
	status_dat_chk_err  = 1 << 3,
	status_seek_err     = 1 << 4,
	status_invalid_cmd  = 1 << 5,
	status_no_addr_mark = 1 << 6,
	status_equ_chk_err  = 1 << 7,
	status_ID_chk_err   = 1 << 8,
	status_ID_not_found = 1 << 9,
	status_ctlr_busy    = 1 << 10,
	status_write_prot   = 1 << 11,
	status_del_sector   = 1 << 12,
	status_interrupt    = 1 << 15,

	status_unit_shift   = 13
};

const device_type FD800 = &device_creator<fd800_legacy_device>;

fd800_legacy_device::fd800_legacy_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, FD800, "TI FD800 Diablo floppy disk controller", tag, owner, clock, "fd800", __FILE__),
	m_recv_buf(0), m_stat_reg(0), m_xmit_buf(0), m_cmd_reg(0), m_interrupt_f_f(0),
	m_int_line(*this), m_buf_pos(0), m_buf_mode(), m_unit(0), m_sector(0)
{
}

void fd800_legacy_device::set_interrupt_line()
{
	if ((m_stat_reg & status_interrupt) && ! m_interrupt_f_f)
		m_int_line(ASSERT_LINE);
	else
		m_int_line(CLEAR_LINE);
}


/* void fd800_legacy_device::unload_proc(device_image_interface &image)
{
    int unit = floppy_get_drive(&image.device());

    m_drv[unit].log_cylinder[0] = m_drv[unit].log_cylinder[1] = -1;
}


void fd800_machine_init(void (*interrupt_callback)(running_machine &machine, int state))
{
    int i;

    m_machine = &machine;
    m_interrupt_callback = interrupt_callback;

    m_stat_reg = 0;
    m_interrupt_f_f = 1;

    m_buf_pos = 0;
    m_buf_mode = bm_off;

    for (i=0; i<MAX_FLOPPIES; i++)
    {
        m_drv[i].img = dynamic_cast<device_image_interface *>(floppy_get_device(machine, i));
        m_drv[i].phys_cylinder = -1;
        m_drv[i].log_cylinder[0] = m_drv[i].log_cylinder[1] = -1;
        m_drv[i].seclen = 64;
        floppy_install_unload_proc(&m_drv[i].img->device(), unload_proc);
    }

    set_interrupt_line();
}
*/

/*
    Read the first id field that can be found on the floppy disk.

    unit: floppy drive index
    head: selected head
    cylinder_id: cylinder ID read
    sector_id: sector ID read

    Return TRUE if an ID was found
*/
int fd800_legacy_device::read_id(int unit, int head, int *cylinder_id, int *sector_id)
{
	//UINT8 revolution_count;*/
	// chrn_id id;

	//revolution_count = 0;*/

	/*while (revolution_count < 2)*/
	/*{*/
	/*  if (m_drv[unit].img->floppy_drive_get_next_id(head, &id))
	    {
	        if (cylinder_id)
	            *cylinder_id = id.C;
	        if (sector_id)
	            *sector_id = id.R;
	        return TRUE;
	    }
	}*/

	return FALSE;
}

/*
    Find a sector by id.

    unit: floppy drive index
    head: selected head
    sector: sector ID to search
    data_id: data ID to be used when calling sector read/write functions

    Return TRUE if the given sector ID was found
*/
int fd800_legacy_device::find_sector(int unit, int head, int sector, int *data_id)
{
/*  UINT8 revolution_count;
    chrn_id id;

    revolution_count = 0;

    while (revolution_count < 2)
    {
        if (m_drv[unit].img->floppy_drive_get_next_id(head, &id))
        {
            // compare id
            if ((id.R == sector) && (id.N == 0))
            {
                *data_id = id.data_id;
                // get ddam status
                // w->ddam = id.flags & ID_FLAG_DELETED_DATA;
                return TRUE;
            }
        }
    }
*/
	return FALSE;
}

/*
    Perform seek command

    unit: floppy drive index
    cylinder: track to seek for
    head: head for which the seek is performed

    Return FALSE if the seek was successful
*/
int fd800_legacy_device::do_seek(int unit, int cylinder, int head)
{
/*  int retries;

    if (cylinder > 76)
    {
        m_stat_reg |= status_invalid_cmd;
        return TRUE;
    }

    if (m_drv[unit].img == NULL || !m_drv[unit].img->exists())
    {
        m_stat_reg |= status_drv_not_ready;
        return TRUE;
    }

    if (m_drv[unit].log_cylinder[head] == -1)
    {
        if (!read_id(unit, head, &m_drv[unit].log_cylinder[head], NULL))
        {
            m_stat_reg |= status_ID_not_found;
            return TRUE;
        }
    }

    if (m_drv[unit].log_cylinder[head] == cylinder)
    {

        return FALSE;
    }
    for (retries=0; retries<10; retries++)
    {
        m_drv[unit].img->floppy_drive_seek(cylinder-m_drv[unit].log_cylinder[head]);

        if (m_drv[unit].phys_cylinder != -1)
            m_drv[unit].phys_cylinder += cylinder-m_drv[unit].log_cylinder[head];

        if (!read_id(unit, head, &m_drv[unit].log_cylinder[head], NULL))
        {
            m_drv[unit].log_cylinder[head] = -1;
            m_stat_reg |= status_ID_not_found;
            return TRUE;
        }

        if (m_drv[unit].log_cylinder[head] == cylinder)
        {

            return FALSE;
        }
    }

    m_stat_reg |= status_seek_err;
    */
	return TRUE;
}

/*
    Perform restore command

    unit: floppy drive index

    Return FALSE if the restore was successful
*/
int fd800_legacy_device::do_restore(int unit)
{
	int seek_complete = 0;
/*  int seek_count = 0;

    if (!m_drv[unit].img->exists())
    {
        m_stat_reg |= status_drv_not_ready;
        return TRUE;
    }


    while (!(seek_complete = !m_drv[unit].img->floppy_tk00_r()) && (seek_count < 76))
    {
        m_drv[unit].img->floppy_drive_seek(-1);
        seek_count++;
    }
    if (! seek_complete)
    {
        m_drv[unit].phys_cylinder = -1;
        m_stat_reg |= status_seek_err;
    }
    else
    {
        m_drv[unit].phys_cylinder = 0;

    }
*/
	return ! seek_complete;
}

/*
    Perform a read operation for one sector
*/
void fd800_legacy_device::do_read(void)
{
/*  int data_id;

    if ((m_sector == 0) || (m_sector > 26))
    {
        m_stat_reg |= status_invalid_cmd;
        return;
    }

    if (!find_sector(m_unit, m_head, m_sector, &data_id))
    {
        m_stat_reg |= status_ID_not_found;
        return;
    }

    m_drv[m_unit].img->floppy_drive_read_sector_data(m_head, data_id, m_buf, 128);
    m_buf_pos = 0;
    m_buf_mode = bm_read;
    m_recv_buf = (m_buf[m_buf_pos<<1] << 8) | m_buf[(m_buf_pos<<1)+1];

    m_stat_reg |= status_XFER_ready;
    m_stat_reg |= status_OP_complete;
*/
}

/*
    Perform a write operation for one sector
*/
void fd800_legacy_device::do_write(void)
{
/*  int data_id;

    if (m_drv[m_unit].seclen < 64)
        memset(m_buf+(m_drv[m_unit].seclen<<1), 0, (64-m_drv[m_unit].seclen)<<1);

    if (!find_sector(m_unit, m_head, m_sector, &data_id))
    {
        m_stat_reg |= status_ID_not_found;
        return;
    }

    m_drv[m_unit].img->floppy_drive_write_sector_data(m_head, data_id, m_buf, 128, m_ddam);
    m_buf_pos = 0;
    m_buf_mode = bm_write;

    m_stat_reg |= status_XFER_ready;
    m_stat_reg |= status_OP_complete;
*/
}

/*
    Execute a fdc command
*/
void fd800_legacy_device::do_cmd(void)
{
/*
    int unit;
    int cylinder;
    int head;
    int seclen;
    int sector;


    if (m_buf_mode != bm_off)
    {   // All commands in the midst of read or write are interpreted as Stop
        unit = (m_cmd_reg >> 10) & 3;

        // reset status
        m_stat_reg = unit << status_unit_shift;

        m_buf_pos = 0;
        m_buf_mode = bm_off;

        m_stat_reg |= status_OP_complete;

        m_stat_reg |= status_interrupt;
        set_interrupt_line();

        return;
    }

    switch (m_cmd_reg >> 12)
    {
    case 0:     // select
                //    bits 16-25: 0s
                //    bits 26-27: unit number (0-3)
        unit = (m_cmd_reg >> 10) & 3;

        // reset status
        m_stat_reg = unit << status_unit_shift;

        if (!m_drv[unit].img->exists())
            m_stat_reg |= status_drv_not_ready; // right???
        else if (m_drv[unit].img->is_readonly())
            m_stat_reg |= status_write_prot;
        else
            m_stat_reg |= status_OP_complete;

        m_stat_reg |= status_interrupt;
        set_interrupt_line();
        break;

    case 1:     // seek
                    bits 16-22: cylinder number (0-76)
                    bits 23-24: 0s
                    bits 25: head number (1=upper)
                    bits 26-27: unit number (0-3)
        unit = (m_cmd_reg >> 10) & 3;
        head = (m_cmd_reg >> 9) & 1;
        cylinder = m_cmd_reg & 0x7f;

        // reset status
        m_stat_reg = unit << status_unit_shift;

        if (!do_seek(unit, cylinder, head))
            m_stat_reg |= status_OP_complete;

        m_stat_reg |= status_interrupt;
        set_interrupt_line();
        break;

    case 2:     // restore
                    bits 16-25: 0s
                    bits 26-27: unit number (0-3)
        unit = (m_cmd_reg >> 10) & 3;

        // reset status
        m_stat_reg = unit << status_unit_shift;

        if (!do_restore(unit))
            m_stat_reg |= status_OP_complete;

        m_stat_reg |= status_interrupt;
        set_interrupt_line();
        break;

    case 3:     // sector length
                    bits 16-22: sector word count (0-64)
                    bits 23-25: 0s
                    bits 26-27: unit number (0-3)
        unit = (m_cmd_reg >> 10) & 3;
        seclen = m_cmd_reg & 0x7f;

        // reset status
        m_stat_reg = unit << status_unit_shift;

        if ((seclen > 64) || (seclen == 0))
        {
            m_stat_reg |= status_invalid_cmd;
        }
        else
        {
            m_drv[unit].seclen = seclen;
            m_stat_reg |= status_OP_complete;
        }

        m_stat_reg |= status_interrupt;
        set_interrupt_line();
        break;

    case 4:     // read
                    bits 16-20: sector number (1-26)
                    bits 21-23: 0s
                    bit 24: no sequential sectoring (1=active)
                    bit 25: head number (1=upper)
                    bits 26-27: unit number (0-3)
        unit = (m_cmd_reg >> 10) & 3;
        head = (m_cmd_reg >> 9) & 1;
        //non_seq_mode = (m_cmd_reg >> 8) & 1;
        sector = m_cmd_reg & 0x1f;

        m_unit = unit;
        m_head = head;
        m_sector = sector;
        //m_non_seq_mode = non_seq_mode;

        // reset status
        m_stat_reg = unit << status_unit_shift;

        do_read();

        m_stat_reg |= status_interrupt;
        set_interrupt_line();
        break;

    case 5:     // read ID
                    bits 16-24: 0s
                    bit 25: head number (1=upper)
                    bits 26-27: unit number (0-3)
        unit = (m_cmd_reg >> 10) & 3;
        head = (m_cmd_reg >> 9) & 1;

        // reset status
        m_stat_reg = unit << status_unit_shift;

        if (!read_id(unit, head, &cylinder, &sector))
        {
            m_stat_reg |= status_ID_not_found;
        }
        else
        {
            m_recv_buf = (cylinder << 8) | sector;
            m_stat_reg |= status_OP_complete;
        }

        m_stat_reg |= status_interrupt;
        set_interrupt_line();
        break;

    case 6:     // read unformatted
                    bits 16-20: sector number (1-26)
                    bits 21-24: 0s
                    bit 25: head number (1=upper)
                    bits 26-27: unit number (0-3)
        // ...
        break;

    case 7:     // write
                    bits 16-20: sector number (1-26)
                    bits 21-24: 0s
                    bit 25: head number (1=upper)
                    bits 26-27: unit number (0-3)
        unit = (m_cmd_reg >> 10) & 3;
        head = (m_cmd_reg >> 9) & 1;
        sector = m_cmd_reg & 0x1f;

        // reset status
        m_stat_reg = unit << status_unit_shift;

        if ((m_sector == 0) || (m_sector > 26))
        {
            m_stat_reg |= status_invalid_cmd;
        }
        else
        {
            m_unit = unit;
            m_head = head;
            m_sector = sector;
            m_ddam = 0;

            m_buf_pos = 0;
            m_buf_mode = bm_write;
            m_stat_reg |= status_XFER_ready;
            m_stat_reg |= status_OP_complete;   // right???
        }

        m_stat_reg |= status_interrupt;
        set_interrupt_line();
        break;

    case 8:     // write delete
                    bits 16-20: sector number (1-26)
                    bits 21-24: 0s
                    bit 25: head number (1=upper)
                    bits 26-27: unit number (0-3)
        unit = (m_cmd_reg >> 10) & 3;
        head = (m_cmd_reg >> 9) & 1;
        sector = m_cmd_reg & 0x1f;

        // reset status
        m_stat_reg = unit << status_unit_shift;

        if ((m_sector == 0) || (m_sector > 26))
        {
            m_stat_reg |= status_invalid_cmd;
        }
        else
        {
            m_unit = unit;
            m_head = head;
            m_sector = sector;
            m_ddam = 1;

            m_buf_pos = 0;
            m_buf_mode = bm_write;
            m_stat_reg |= status_XFER_ready;
            m_stat_reg |= status_OP_complete;   // right???
        }

        m_stat_reg |= status_interrupt;
        set_interrupt_line();
        break;

    case 9:     // format track
                    bits 16-23: track ID (0-255, normally current cylinder index, or 255 for bad track)
                    bit 24: verify only (1 - verify, 0 - format & verify)
                    bit 25: head number (1=upper)
                    bits 26-27: unit number (0-3)
        // ...
        break;

    case 10:    // load int mask
                    bit 16: bad mask for interrupt (0 = unmask or enable interrupt)
                    bits 17-27: 0s
        m_interrupt_f_f = m_cmd_reg & 1;
        set_interrupt_line();
        break;

    case 11:    // stop
                    bits 16-25: 0s
                    bits 26-27: unit number (0-3)
        unit = (m_cmd_reg >> 10) & 3;

        // reset status
        m_stat_reg = unit << status_unit_shift;

        m_stat_reg |= status_OP_complete;

        m_stat_reg |= status_interrupt;
        set_interrupt_line();
        break;

    case 12:    // step head
                    bits 16-22: track number (0-76)
                    bits 23-25: 0s
                    bits 26-27: unit number (0-3)
        unit = (m_cmd_reg >> 10) & 3;
        cylinder = m_cmd_reg & 0x7f;

        if (cylinder > 76)
        {
            m_stat_reg |= status_invalid_cmd;
        }
        else if ((m_drv[unit].phys_cylinder != -1) || (!do_restore(unit)))
        {
            m_drv[unit].img->floppy_drive_seek(cylinder-m_drv[unit].phys_cylinder);
            m_stat_reg |= status_OP_complete;
        }

        m_stat_reg |= status_interrupt;
        set_interrupt_line();
        break;

    case 13:    // maintenance commands
                    bits 16-23: according to extended command code
                    bits 24-27: extended command code (0-7)
        switch ((m_cmd_reg >> 8) & 15)
        {
        case 0: // reset
                    bits 16-23: 0s
            // ...
            break;
        case 1: // retry inhibit
                    bits 16-23: 0s
            // ...
            break;
        case 2: // LED test
                    bit 16: 1
                    bits 17-19: 0s
                    bit 20: LED #2 enable
                    bit 21: LED #3 enable
                    bit 22: LED #4 enable
                    bit 23: enable LEDs
            // ...
            break;
        case 3: // program error (a.k.a. invalid command)
                    bits 16-23: 0s
            // ...
            break;
        case 4: // memory read
                    bits 16-20: controller memory address (shifted left by 8 to generate 9900 address)
                    bits 21-23: 0s
            // ...
            break;
        case 5: // RAM load
                    bit 16: 0
                    bits 17-23: RAM offset (shifted left by 1 and offset by >1800 to generate 9900 address)
            // ...
            break;
        case 6: // RAM run
                    bit 16: 0
                    bits 17-23: RAM offset (shifted left by 1 and offset by >1800 to generate 9900 address)
            // ...
            break;
        case 7: // power up simulation
                    bits 16-23: 0s
            // ...
            break;
        }
        // ...
        break;

    case 14:    // IPL
                    bits 16-22: track number (0-76)
                    bit 23: 0
                    bit 24: no sequential sectoring (1=active)
                    bit 25: head number (1=upper)
                    bits 26-27: unit number (0-3)
        unit = (m_cmd_reg >> 10) & 3;
        head = (m_cmd_reg >> 9) & 1;
        //non_seq_mode = (m_cmd_reg >> 8) & 1;
        cylinder = m_cmd_reg & 0x7f;

        if (!do_seek(unit, cylinder, head))
        {
            m_unit = unit;
            m_head = head;
            m_sector = 1;
            //m_non_seq_mode = non_seq_mode;

            do_read();
        }

        m_stat_reg |= status_interrupt;
        set_interrupt_line();
        break;

    case 15:    // Clear Status port
                    bits 16-27: 0s
        m_stat_reg = 0;
        set_interrupt_line();
        break;
    }
    */
}

/*
    read one CRU bit

    0-15: receive buffer
    16-31: status:
        16: OP complete (1 -> complete???)
        17: Xfer ready (XFER) (1 -> ready???)
        18: drive not ready
        19: data check error
        20: seek error/??????
        21 invalid command/??????
        22: no address mark found/??????
        23: equipment check error/??????
        24: ID check error
        25: ID not found
        26: Controller busy (CTLBSY) (0 -> controller is ready)
        27: write protect
        28: deleted sector detected
        29: unit LSB
        30: unit MSB
        31: Interrupt (CBUSY???) (1 -> controller is ready)
*/
READ8_MEMBER( fd800_legacy_device::cru_r )
{
	int reply = 0;

	switch (offset)
	{
	case 0:
	case 1:
		// receive buffer
		reply = m_recv_buf >> (offset*8);
		break;

	case 2:
	case 3:
		// status register
		reply = m_stat_reg >> ((offset-2)*8);
		break;
	}

	return reply;
}

/*
    write one CRU bit

    0-15: controller data word (transmit buffer)
    16-31: controller command word (command register)
    16-23: parameter value
    24: flag bit/extended command code
    25: head select/extended command code
    26: FD unit number LSB/extended command code
    27: FD unit number MSB/extended command code
    28-31: command code
*/
WRITE8_MEMBER( fd800_legacy_device::cru_w )
{
	switch (offset)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
		// transmit buffer
		if (data)
			m_xmit_buf |= 1 << offset;
		else
			m_xmit_buf &= ~(1 << offset);
		if (offset == 15)
		{
			switch (m_buf_mode)
			{
			case bm_off:
				break;
			case bm_read:
				m_buf_pos++;
				if (m_buf_pos == m_drv[m_unit].seclen)
				{   // end of sector
					if (m_sector == 26)
					{   // end of track -> end command (right???)
						m_stat_reg &= ~status_XFER_ready;
						m_stat_reg |= status_OP_complete;
						m_stat_reg |= status_interrupt;
						m_buf_mode = bm_off;
						set_interrupt_line();
					}
					else
					{   // read next sector
						m_sector++;
						m_stat_reg &= ~status_XFER_ready | status_OP_complete | status_interrupt;
						do_read();
						m_stat_reg |= status_interrupt;
						set_interrupt_line();
					}
				}
				else
					m_recv_buf = (m_buf[m_buf_pos<<1] << 8) | m_buf[(m_buf_pos<<1)+1];
				break;

			case bm_write:
				m_buf[m_buf_pos<<1] = m_xmit_buf >> 8;
				m_buf[(m_buf_pos<<1)+1] = m_xmit_buf & 0xff;
				m_buf_pos++;
				if (m_buf_pos == m_drv[m_unit].seclen)
				{   // end of sector
					do_write();
					if (m_sector == 26)
					{
						// end of track -> end command (right???)
						m_stat_reg &= ~status_XFER_ready;
						m_stat_reg |= status_OP_complete;
						m_stat_reg |= status_interrupt;
						m_buf_mode = bm_off;
						set_interrupt_line();
					}
					else
					{   // increment to next sector
						m_sector++;
						m_stat_reg |= status_interrupt;
						set_interrupt_line();
					}
				}
				break;
			}
		}
		break;

	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
		// command register
		if (data)
			m_cmd_reg |= 1 << (offset-16);
		else
			m_cmd_reg &= ~(1 << (offset-16));
		if (offset == 31)
			do_cmd();
		break;
	}
}

#if 0
LEGACY_FLOPPY_OPTIONS_START(fd800)
	// SSSD 8"
	LEGACY_FLOPPY_OPTION(fd800, "dsk", "TI990 8\" SSSD disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([77])
		SECTORS([26])
		SECTOR_LENGTH([128])
		FIRST_SECTOR_ID([1]))

	// DSSD 8"
	LEGACY_FLOPPY_OPTION(fd800, "dsk", "TI990 8\" DSSD disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([77])
		SECTORS([26])
		SECTOR_LENGTH([128])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END
#endif

void fd800_legacy_device::device_start(void)
{
	logerror("fd800: start\n");
	m_int_line.resolve();

	for (auto & elem : m_drv)
	{
	//  m_drv[i].img = floppy_get_device(machine(), i);
		elem.phys_cylinder = -1;
		elem.log_cylinder[0] = elem.log_cylinder[1] = -1;
		elem.seclen = 64;
	}
}

void fd800_legacy_device::device_reset(void)
{
	logerror("fd800: reset\n");
	m_stat_reg = 0;
	m_interrupt_f_f = 1;

	m_buf_pos = 0;
	m_buf_mode = bm_off;
}
