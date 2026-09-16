// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef MAME_DEBUGGER_QT_DEVICEINFORMATIONWINDOW_H
#define MAME_DEBUGGER_QT_DEVICEINFORMATIONWINDOW_H

#pragma once

#include "windowqt.h"

#include <QtCore/QAbstractTableModel>

#include <optional>
#include <vector>


namespace osd::debugger::qt {

//============================================================
//  The model for the address spaces list
//============================================================

class AddressSpacesListModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	explicit AddressSpacesListModel(device_memory_interface &memory, QObject *parent = nullptr);
	~AddressSpacesListModel();

	virtual QVariant data(const QModelIndex &index, int role) const override;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	address_space &space(const QModelIndex &index) const;

private:
	device_memory_interface &m_memory;
	std::vector<int> m_indices;
};


//============================================================
//  The Device Information Window.
//============================================================
class DeviceInformationWindow : public WindowQt
{
	Q_OBJECT

public:
	DeviceInformationWindow(DebuggerQt &debugger, device_t *device = nullptr, QWidget* parent=nullptr);
	virtual ~DeviceInformationWindow();

	void set_device(const char *tag);

	virtual void restoreConfiguration(util::xml::data_node const &node) override;

protected:
	virtual void saveConfigurationToNode(util::xml::data_node &node) override;

private slots:
	void openAddessSpace(const QModelIndex &index);

private:
	device_t *m_device;
	std::optional<AddressSpacesListModel> m_addrspaces_model;

	void fill_device_information();
};

} // namespace osd::debugger::qt

#endif // MAME_DEBUGGER_QT_DEVICEINFORMATIONWINDOW_H
