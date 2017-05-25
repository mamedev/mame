// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "legscsi.h"

legacy_scsi_host_adapter::legacy_scsi_host_adapter(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_selected(0)
	, m_scsi_port(*this, finder_base::DUMMY_TAG)
{
}

void legacy_scsi_host_adapter::device_start()
{
}

void legacy_scsi_host_adapter::reset_bus()
{
	for (int i = 0; i <= 7; i++)
	{
		scsihle_device *scsidev = get_device(i);
		if (scsidev != nullptr)
		{
			scsidev->reset();
		}
	}
}

bool legacy_scsi_host_adapter::select(int id)
{
	m_selected = id;

	scsihle_device *scsidev = get_device(m_selected);
	if (scsidev != nullptr)
	{
		return true;
	}

	return false;
}

void legacy_scsi_host_adapter::send_command(uint8_t *data, int bytes)
{
	scsihle_device *scsidev = get_device(m_selected);
	if (scsidev != nullptr)
	{
		scsidev->SetCommand(data, bytes);
		scsidev->ExecCommand();
	}
	else
	{
		logerror("send_command unknown SCSI id %d\n", m_selected);
	}
}

int legacy_scsi_host_adapter::get_length(void)
{
	scsihle_device *scsidev = get_device(m_selected);
	if (scsidev != nullptr)
	{
		int length;
		scsidev->GetLength(&length);
		return length;
	}
	else
	{
		logerror("get_length unknown SCSI id %d\n", m_selected);
		return 0;
	}
}

int legacy_scsi_host_adapter::get_phase(void)
{
	scsihle_device *scsidev = get_device(m_selected);
	if (scsidev != nullptr)
	{
		int phase;
		scsidev->GetPhase(&phase);
		return phase;
	}
	else
	{
		logerror("get_phase unknown SCSI id %d\n", m_selected);
		return 0;
	}
}

void legacy_scsi_host_adapter::read_data(uint8_t *data, int bytes)
{
	scsihle_device *scsidev = get_device(m_selected);
	if (scsidev != nullptr)
	{
		scsidev->ReadData(data, bytes);
	}
	else
	{
		logerror("read_data unknown SCSI id %d\n", m_selected);
	}
}

void legacy_scsi_host_adapter::write_data(uint8_t *data, int bytes)
{
	scsihle_device *scsidev = get_device(m_selected);
	if (scsidev != nullptr)
	{
		scsidev->WriteData(data, bytes);
	}
	else
	{
		logerror("write_data unknown SCSI id %d\n", m_selected);
	}
}

uint8_t legacy_scsi_host_adapter::get_status()
{
	scsihle_device *scsidev = get_device(m_selected);
	if (scsidev != nullptr)
	{
		void *image;

		scsidev->GetDevice(&image);
		if (image != nullptr)
			return 0x00;

		return 0x02;
	}
	else
	{
		logerror("get_status unknown SCSI id %d\n", m_selected);
		return 0;
	}
}

scsihle_device *legacy_scsi_host_adapter::get_device(int id)
{
	// steal SCSI devices from bus
	for (device_t &device : m_scsi_port->subdevices())
	{
		scsi_port_slot_device *slot = dynamic_cast<scsi_port_slot_device *>(&device);
		if (slot != nullptr)
		{
			scsihle_device *scsidev = dynamic_cast<scsihle_device *>(slot->dev());
			if (scsidev != nullptr)
			{
				if (scsidev->GetDeviceID() == id)
				{
					return scsidev;
				}
			}
		}
	}

	return nullptr;
}
