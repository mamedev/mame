/***************************************************************************

    none.c

    Empty shim for systems not supporting midi / portmidi

*******************************************************************c********/

#include "osdcore.h"

struct osd_midi_device
{
    int dummy;
};

bool osd_midi_init()
{
    return true;
}

void osd_midi_exit()
{
}

void osd_list_midi_devices(void)
{
    osd_printf_warning("\nMIDI is not supported in this build\n");
}

osd_midi_device *osd_open_midi_input(const char *devname)
{
    return NULL;
}

osd_midi_device *osd_open_midi_output(const char *devname)
{
    return NULL;
}

void osd_close_midi_channel(osd_midi_device *dev)
{
}

bool osd_poll_midi_channel(osd_midi_device *dev)
{
    return false;
}

int osd_read_midi_channel(osd_midi_device *dev, UINT8 *pOut)
{
    return 0;
}

void osd_write_midi_channel(osd_midi_device *dev, UINT8 data)
{
}
