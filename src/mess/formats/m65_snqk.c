/******************************************************************************
 *  Microtan 65
 *
 *  Snapshot and quickload formats
 *
 *  Juergen Buchmueller <pullmoll@t-online.de>, Jul 2000
 *
 *  Thanks go to Geoff Macdonald <mail@geoff.org.uk>
 *  for his site http:://www.geo255.redhotant.com
 *  and to Fabrice Frances <frances@ensica.fr>
 *  for his site http://www.ifrance.com/oric/microtan.html
 *
 *****************************************************************************/
#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "includes/microtan.h"
#include "machine/6522via.h"
#include "machine/6551acia.h"
#include "sound/ay8910.h"
#include "formats/m65_snqk.h"

static int microtan_verify_snapshot(UINT8 *data, int size)
{
    if (size == 8263)
    {
        logerror("microtan_snapshot_id: magic size %d found\n", size);
        return IMAGE_VERIFY_PASS;
    }
    else
    {
        if (4 + data[2] + 256 * data[3] + 1 + 16 + 16 + 16 + 1 + 1 + 16 + 16 + 64 + 7 == size)
        {
            logerror("microtan_snapshot_id: header RAM size + structures matches filesize %d\n", size);
            return IMAGE_VERIFY_PASS;
        }
    }

    return IMAGE_VERIFY_FAIL;
}

static int parse_intel_hex(UINT8 *snapshot_buff, char *src)
{
    char line[128];
    int /*row = 0,*/ column = 0, last_addr = 0, last_size = 0;

    while (*src)
    {
        if (*src == '\r' || *src == '\n')
        {
            if (column)
            {
                unsigned int size, addr, null, b[32], cs, n;

                line[column] = '\0';
                /*row++;*/
                n = sscanf(line, ":%02x%04x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                    &size, &addr, &null,
                    &b[ 0], &b[ 1], &b[ 2], &b[ 3], &b[ 4], &b[ 5], &b[ 6], &b[ 7],
                    &b[ 8], &b[ 9], &b[10], &b[11], &b[12], &b[13], &b[14], &b[15],
                    &b[16], &b[17], &b[18], &b[19], &b[20], &b[21], &b[22], &b[23],
                    &b[24], &b[25], &b[26], &b[27], &b[28], &b[29], &b[30], &b[31],
                    &cs);
                if (n == 0)
                {
                    logerror("parse_intel_hex: malformed line [%s]\n", line);
                }
                else if (n == 1)
                {
                    logerror("parse_intel_hex: only size found [%s]\n", line);
                }
                else if (n == 2)
                {
                    logerror("parse_intel_hex: only size and addr found [%s]\n", line);
                }
                else if (n == 3)
                {
                    logerror("parse_intel_hex: only size, addr and null found [%s]\n", line);
                }
                else
                if (null != 0)
                {
                    logerror("parse_intel_hex: warning null byte is != 0 [%s]\n", line);
                }
                else
                {
                    int i, sum;

                    n -= 3;

                    sum = size + (addr & 0xff) + ((addr >> 8) & 0xff);
                    if (n != 32 + 1)
                        cs = b[n-1];

                    last_addr = addr;
                    last_size = n-1;
                    logerror("parse_intel_hex: %04X", addr);
                    for (i = 0; i < n-1; i++)
                    {
                        sum += b[i];
                        snapshot_buff[addr++] = b[i];
                    }
                    logerror("-%04X checksum %02X+%02X = %02X\n", addr-1, cs, sum & 0xff, (cs + sum) & 0xff);
                }
            }
            column = 0;
        }
        else
        {
            line[column++] = *src;
        }
        src++;
    }
    /* register preset? */
    if (last_size == 7)
    {
        logerror("parse_intel_hex: registers (?) at %04X\n", last_addr);
        memcpy(&snapshot_buff[8192+64], &snapshot_buff[last_addr], last_size);
    }
    return IMAGE_INIT_PASS;
}

static int parse_zillion_hex(UINT8 *snapshot_buff, char *src)
{
    char line[128];
    int parsing = 0, /*row = 0,*/ column = 0;

    while (*src)
    {
        if (parsing)
        {
            if (*src == '}')
                parsing = 0;
            else
            {
                if (*src == '\r' || *src == '\n')
                {
                    if (column)
                    {
                        unsigned int addr, b[8], n;

                        line[column] = '\0';
                        /*row++;*/
                        n = sscanf(line, "%x %x %x %x %x %x %x %x %x", &addr, &b[0], &b[1], &b[2], &b[3], &b[4], &b[5], &b[6], &b[7]);
                        if (n == 0)
                        {
                            logerror("parse_zillion_hex: malformed line [%s]\n", line);
                        }
                        else if (n == 1)
                        {
                            logerror("parse_zillion_hex: only addr found [%s]\n", line);
                        }
                        else
                        {
                            int i;

                            logerror("parse_zillion_hex: %04X", addr);
                            for (i = 0; i < n-1; i++)
                                snapshot_buff[addr++] = b[i];
                            logerror("-%04X\n", addr-1);
                        }
                    }
                    column = 0;
                }
                else
                {
                    line[column++] = *src;
                }
            }
        }
        else
        {
            if (*src == '\r' || *src == '\n')
            {
                if (column)
                {
                    int addr, n;

                    /*row++;*/
                    line[column] = '\0';
                    n = sscanf(line, "G%x", (unsigned int *) &addr);
                    if (n == 1 && !snapshot_buff[8192+64+0] && !snapshot_buff[8192+64+1])
                    {
                        logerror("microtan_hexfile_init: go addr %04X\n", addr);
                        snapshot_buff[8192+64+0] = addr & 0xff;
                        snapshot_buff[8192+64+1] = (addr >> 8) & 0xff;
                    }
                }
                column = 0;
            }
            else
            {
                line[column++] = *src;
            }
            if (*src == '{')
            {
                parsing = 1;
                column = 0;
            }
        }
        src++;
    }
    return IMAGE_INIT_PASS;
}

static void microtan_set_cpu_regs(running_machine &machine,const UINT8 *snapshot_buff, int base)
{
    logerror("microtan_snapshot_copy: PC:%02X%02X P:%02X A:%02X X:%02X Y:%02X SP:1%02X",
        snapshot_buff[base+1], snapshot_buff[base+0], snapshot_buff[base+2], snapshot_buff[base+3],
        snapshot_buff[base+4], snapshot_buff[base+5], snapshot_buff[base+6]);
    machine.device("maincpu")->state().set_state_int(M6502_PC, snapshot_buff[base+0] + 256 * snapshot_buff[base+1]);
    machine.device("maincpu")->state().set_state_int(M6502_P, snapshot_buff[base+2]);
    machine.device("maincpu")->state().set_state_int(M6502_A, snapshot_buff[base+3]);
    machine.device("maincpu")->state().set_state_int(M6502_X, snapshot_buff[base+4]);
    machine.device("maincpu")->state().set_state_int(M6502_Y, snapshot_buff[base+5]);
    machine.device("maincpu")->state().set_state_int(M6502_S, snapshot_buff[base+6]);
}

static void microtan_snapshot_copy(running_machine &machine, UINT8 *snapshot_buff, int snapshot_size)
{
	microtan_state *state = machine.driver_data<microtan_state>();
    UINT8 *RAM = state->memregion("maincpu")->base();
    address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
    via6522_device *via_0 = machine.device<via6522_device>("via6522_0");
    via6522_device *via_1 = machine.device<via6522_device>("via6522_1");
    device_t *ay8910 = machine.device("ay8910.1");

    /* check for .DMP file format */
    if (snapshot_size == 8263)
    {
        int i, base;
        /********** DMP format
         * Lower 8k of RAM (0000 to 1fff)
         * 64 bytes of chunky graphics bits (first byte bit is for character at 0200, bit 1=0201, etc)
         * 7 bytes of CPU registers (PCL, PCH, PSW, A, IX, IY, SP)
         */
        logerror("microtan_snapshot_copy: magic size %d found, assuming *.DMP format\n", snapshot_size);

        base = 0;
        /* 8K of RAM from 0000 to 1fff */
        memcpy(RAM, &snapshot_buff[base], 8192);
        base += 8192;
        /* 64 bytes of chunky graphics info */
        for (i = 0; i < 32*16; i++)
        {
            state->m_chunky_buffer[i] = (snapshot_buff[base+i/8] >> (i&7)) & 1;
        }
        base += 64;
        microtan_set_cpu_regs(machine, snapshot_buff, base);
    }
    else
    {
        int i, ramend, base;
        /********** M65 format ************************************
         *  2 bytes: File version
         *  2 bytes: RAM size
         *  n bytes: RAM (0000 to RAM Size)
         * 16 bytes: 1st 6522 (0xbfc0 to 0xbfcf)
         * 16 bytes: 2ns 6522 (0xbfe0 to 0xbfef)
         * 16 bytes: Microtan IO (0xbff0 to 0xbfff)
         *  1 byte : Invaders sound (0xbc04)
         *  1 byte : Chunky graphics state (0=off, 1=on)
         * 16 bytes: 1st AY8910 registers
         * 16 bytes: 2nd AY8910 registers
         * 64 bytes: Chunky graphics bits (first byte bit 0 is for character at 0200, bit 1=0201, etc)
         *  7 bytes: CPU registers (PCL, PCH, PSW, A, IX, IY, SP)
         */
        ramend = snapshot_buff[2] + 256 * snapshot_buff[3];
        if (2 + 2 + ramend + 1 + 16 + 16 + 16 + 1 + 1 + 16 + 16 + 64 + 7 != snapshot_size)
        {
            logerror("microtan_snapshot_copy: size %d doesn't match RAM size %d + structure size\n", snapshot_size, ramend+1);
            return;
        }

        logerror("microtan_snapshot_copy: size %d found, assuming *.M65 format\n", snapshot_size);
        base = 4;
        memcpy(RAM, &snapshot_buff[base], snapshot_buff[2] + 256 * snapshot_buff[3] + 1);
        base += ramend + 1;

        /* first set of VIA6522 registers */
        for (i = 0; i < 16; i++ )
            via_0->write(*space, i, snapshot_buff[base++]);

        /* second set of VIA6522 registers */
        for (i = 0; i < 16; i++ )
            via_1->write(*space, i, snapshot_buff[base++]);

        /* microtan IO bff0-bfff */
        for (i = 0; i < 16; i++ )
        {
            RAM[0xbff0+i] = snapshot_buff[base++];
            if (i < 4)
                state->microtan_bffx_w(*space, i, RAM[0xbff0+i]);
        }

        state->microtan_sound_w(*space, 0, snapshot_buff[base++]);
        state->m_chunky_graphics = snapshot_buff[base++];

        /* first set of AY8910 registers */
        for (i = 0; i < 16; i++ )
        {
            ay8910_address_w(ay8910, 0, i);
            ay8910_data_w(ay8910, 0, snapshot_buff[base++]);
        }

        /* second set of AY8910 registers */
        for (i = 0; i < 16; i++ )
        {
            ay8910_address_w(ay8910, 0, i);
            ay8910_data_w(ay8910, 0, snapshot_buff[base++]);
        }

        for (i = 0; i < 32*16; i++)
        {
            state->m_chunky_buffer[i] = (snapshot_buff[base+i/8] >> (i&7)) & 1;
        }
        base += 64;

        microtan_set_cpu_regs(machine, snapshot_buff, base);
    }
}

SNAPSHOT_LOAD( microtan )
{
    UINT8 *snapshot_buff;

    snapshot_buff = (UINT8*)image.ptr();
    if (!snapshot_buff)
        return IMAGE_INIT_FAIL;

    if (microtan_verify_snapshot(snapshot_buff, snapshot_size)==IMAGE_VERIFY_FAIL)
        return IMAGE_INIT_FAIL;

    microtan_snapshot_copy(image.device().machine(), snapshot_buff, snapshot_size);
    return IMAGE_INIT_PASS;
}

QUICKLOAD_LOAD( microtan )
{
    int snapshot_size;
    UINT8 *snapshot_buff;
    char *buff;
    int rc;

    snapshot_size = 8263;   /* magic size */
    snapshot_buff = (UINT8*)malloc(snapshot_size);
    if (!snapshot_buff)
    {
        logerror("microtan_hexfile_load: could not allocate %d bytes of buffer\n", snapshot_size);
        return IMAGE_INIT_FAIL;
    }
    memset(snapshot_buff, 0, snapshot_size);

    buff = (char*)malloc(quickload_size + 1);
    if (!buff)
    {
        free(snapshot_buff);
        logerror("microtan_hexfile_load: could not allocate %d bytes of buffer\n", quickload_size);
        return IMAGE_INIT_FAIL;
    }
    image.fread( buff, quickload_size);

    buff[quickload_size] = '\0';

    if (buff[0] == ':')
        rc = parse_intel_hex(snapshot_buff, buff);
    else
        rc = parse_zillion_hex(snapshot_buff, buff);
    if (rc == IMAGE_INIT_PASS)
        microtan_snapshot_copy(image.device().machine(), snapshot_buff, snapshot_size);
    free(snapshot_buff);
    return rc;
}


