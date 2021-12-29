// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#include "emu.h"
#include "memorywindow.h"

#include "debug/dvmemory.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"

#include <QtGui/QClipboard>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QActionGroup>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QVBoxLayout>

#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
#define horizontalAdvance width
#endif

MemoryWindow::MemoryWindow(running_machine &machine, QWidget *parent) :
	WindowQt(machine, nullptr)
{
	setWindowTitle("Debug: Memory View");

	if (parent)
	{
		QPoint parentPos = parent->pos();
		setGeometry(parentPos.x()+100, parentPos.y()+100, 800, 400);
	}

	//
	// The main frame and its input and log widgets
	//
	QFrame *mainWindowFrame = new QFrame(this);

	// The top frame & groupbox that contains the input widgets
	QFrame *topSubFrame = new QFrame(mainWindowFrame);

	// The input edit
	m_inputEdit = new QLineEdit(topSubFrame);
	connect(m_inputEdit, &QLineEdit::returnPressed, this, &MemoryWindow::expressionSubmitted);

	// The memory space combo box
	m_memoryComboBox = new QComboBox(topSubFrame);
	m_memoryComboBox->setObjectName("memoryregion");
	m_memoryComboBox->setMinimumWidth(300);
	connect(m_memoryComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MemoryWindow::memoryRegionChanged);

	// The main memory window
	m_memTable = new DebuggerMemView(DVT_MEMORY, m_machine, this);

	// Layout
	QHBoxLayout *subLayout = new QHBoxLayout(topSubFrame);
	subLayout->addWidget(m_inputEdit);
	subLayout->addWidget(m_memoryComboBox);
	subLayout->setSpacing(3);
	subLayout->setContentsMargins(2,2,2,2);

	QVBoxLayout *vLayout = new QVBoxLayout(mainWindowFrame);
	vLayout->setSpacing(3);
	vLayout->setContentsMargins(2,2,2,2);
	vLayout->addWidget(topSubFrame);
	vLayout->addWidget(m_memTable);

	setCentralWidget(mainWindowFrame);

	//
	// Menu bars
	//

	// Create a data format group
	QActionGroup *dataFormat = new QActionGroup(this);
	dataFormat->setObjectName("dataformat");
	QAction *formatActOne  = new QAction("1-byte hexadecimal", this);
	QAction *formatActTwo  = new QAction("2-byte hexadecimal", this);
	QAction *formatActFour = new QAction("4-byte hexadecimal", this);
	QAction *formatActEight = new QAction("8-byte hexadecimal", this);
	QAction *formatActOneOctal = new QAction("1-byte octal", this);
	QAction *formatActTwoOctal = new QAction("2-byte octal", this);
	QAction *formatActFourOctal = new QAction("4-byte octal", this);
	QAction *formatActEightOctal = new QAction("8-byte octal", this);
	QAction *formatAct32bitFloat = new QAction("32 bit floating point", this);
	QAction *formatAct64bitFloat = new QAction("64 bit floating point", this);
	QAction *formatAct80bitFloat = new QAction("80 bit floating point", this);
	formatActOne->setObjectName("formatActOne");
	formatActTwo->setObjectName("formatActTwo");
	formatActFour->setObjectName("formatActFour");
	formatActEight->setObjectName("formatActEight");
	formatActOneOctal->setObjectName("formatActOneOctal");
	formatActTwoOctal->setObjectName("formatActTwoOctal");
	formatActFourOctal->setObjectName("formatActFourOctal");
	formatActEightOctal->setObjectName("formatActEightOctal");
	formatAct32bitFloat->setObjectName("formatAct32bitFloat");
	formatAct64bitFloat->setObjectName("formatAct64bitFloat");
	formatAct80bitFloat->setObjectName("formatAct80bitFloat");
	formatActOne->setCheckable(true);
	formatActTwo->setCheckable(true);
	formatActFour->setCheckable(true);
	formatActEight->setCheckable(true);
	formatActOneOctal->setCheckable(true);
	formatActTwoOctal->setCheckable(true);
	formatActFourOctal->setCheckable(true);
	formatActEightOctal->setCheckable(true);
	formatAct32bitFloat->setCheckable(true);
	formatAct64bitFloat->setCheckable(true);
	formatAct80bitFloat->setCheckable(true);
	formatActOne->setActionGroup(dataFormat);
	formatActTwo->setActionGroup(dataFormat);
	formatActFour->setActionGroup(dataFormat);
	formatActEight->setActionGroup(dataFormat);
	formatActOneOctal->setActionGroup(dataFormat);
	formatActTwoOctal->setActionGroup(dataFormat);
	formatActFourOctal->setActionGroup(dataFormat);
	formatActEightOctal->setActionGroup(dataFormat);
	formatAct32bitFloat->setActionGroup(dataFormat);
	formatAct64bitFloat->setActionGroup(dataFormat);
	formatAct80bitFloat->setActionGroup(dataFormat);
	formatActOne->setShortcut(QKeySequence("Ctrl+1"));
	formatActTwo->setShortcut(QKeySequence("Ctrl+2"));
	formatActFour->setShortcut(QKeySequence("Ctrl+4"));
	formatActEight->setShortcut(QKeySequence("Ctrl+8"));
	formatActOneOctal->setShortcut(QKeySequence("Ctrl+3"));
	formatActTwoOctal->setShortcut(QKeySequence("Ctrl+5"));
	formatActFourOctal->setShortcut(QKeySequence("Ctrl+7"));
	formatActEightOctal->setShortcut(QKeySequence("Ctrl+9"));
	formatAct32bitFloat->setShortcut(QKeySequence("Ctrl+Shift+F"));
	formatAct64bitFloat->setShortcut(QKeySequence("Ctrl+Shift+D"));
	formatAct80bitFloat->setShortcut(QKeySequence("Ctrl+Shift+E"));
	formatActOne->setChecked(true);
	connect(dataFormat, &QActionGroup::triggered, this, &MemoryWindow::formatChanged);

	// Create an address display group
	QActionGroup *addressGroup = new QActionGroup(this);
	addressGroup->setObjectName("addressgroup");
	QAction *addressActLogical = new QAction("Logical Addresses", this);
	QAction *addressActPhysical = new QAction("Physical Addresses", this);
	addressActLogical->setCheckable(true);
	addressActPhysical->setCheckable(true);
	addressActLogical->setActionGroup(addressGroup);
	addressActPhysical->setActionGroup(addressGroup);
	addressActLogical->setShortcut(QKeySequence("Ctrl+G"));
	addressActPhysical->setShortcut(QKeySequence("Ctrl+Y"));
	addressActLogical->setChecked(true);
	connect(addressGroup, &QActionGroup::triggered, this, &MemoryWindow::addressChanged);

	// Create an address radix group
	QActionGroup *radixGroup = new QActionGroup(this);
	radixGroup->setObjectName("radixgroup");
	QAction *radixActHexadecimal = new QAction("Hexadecimal Addresses", this);
	QAction *radixActDecimal = new QAction("Decimal Addresses", this);
	QAction *radixActOctal = new QAction("Octal Addresses", this);
	radixActHexadecimal->setObjectName("radixHexadecimal");
	radixActDecimal->setObjectName("radixDecimal");
	radixActOctal->setObjectName("radixOctal");
	radixActHexadecimal->setCheckable(true);
	radixActDecimal->setCheckable(true);
	radixActOctal->setCheckable(true);
	radixActHexadecimal->setActionGroup(radixGroup);
	radixActDecimal->setActionGroup(radixGroup);
	radixActOctal->setActionGroup(radixGroup);
	radixActHexadecimal->setShortcut(QKeySequence("Ctrl+Shift+H"));
	radixActOctal->setShortcut(QKeySequence("Ctrl+Shift+O"));
	radixActHexadecimal->setChecked(true);
	connect(radixGroup, &QActionGroup::triggered, this, &MemoryWindow::radixChanged);

	// Create a reverse view radio
	QAction *reverseAct = new QAction("Reverse View", this);
	reverseAct->setObjectName("reverse");
	reverseAct->setCheckable(true);
	reverseAct->setShortcut(QKeySequence("Ctrl+R"));
	connect(reverseAct, &QAction::toggled, this, &MemoryWindow::reverseChanged);

	// Create increase and decrease bytes-per-line actions
	QAction *increaseBplAct = new QAction("Increase Bytes Per Line", this);
	QAction *decreaseBplAct = new QAction("Decrease Bytes Per Line", this);
	increaseBplAct->setShortcut(QKeySequence("Ctrl+P"));
	decreaseBplAct->setShortcut(QKeySequence("Ctrl+O"));
	connect(increaseBplAct, &QAction::triggered, this, &MemoryWindow::increaseBytesPerLine);
	connect(decreaseBplAct, &QAction::triggered, this, &MemoryWindow::decreaseBytesPerLine);

	// Assemble the options menu
	QMenu *optionsMenu = menuBar()->addMenu("&Options");
	optionsMenu->addActions(dataFormat->actions());
	optionsMenu->addSeparator();
	optionsMenu->addActions(addressGroup->actions());
	optionsMenu->addSeparator();
	optionsMenu->addActions(radixGroup->actions());
	optionsMenu->addSeparator();
	optionsMenu->addAction(reverseAct);
	optionsMenu->addSeparator();
	optionsMenu->addAction(increaseBplAct);
	optionsMenu->addAction(decreaseBplAct);


	//
	// Initialize
	//
	populateComboBox();

	// Set to the current CPU's memory view
	setToCurrentCpu();
}


MemoryWindow::~MemoryWindow()
{
}


void MemoryWindow::memoryRegionChanged(int index)
{
	if (index < m_memTable->view()->source_count())
	{
		m_memTable->view()->set_source(*m_memTable->view()->source(index));
		m_memTable->viewport()->update();

		// Update the data format radio buttons to the memory region's default
		debug_view_memory *memView = downcast<debug_view_memory*>(m_memTable->view());
		switch (memView->get_data_format())
		{
		case debug_view_memory::data_format::HEX_8BIT: dataFormatMenuItem("formatActOne")->setChecked(true); break;
		case debug_view_memory::data_format::HEX_16BIT: dataFormatMenuItem("formatActTwo")->setChecked(true); break;
		case debug_view_memory::data_format::HEX_32BIT: dataFormatMenuItem("formatActFour")->setChecked(true); break;
		case debug_view_memory::data_format::HEX_64BIT: dataFormatMenuItem("formatActEight")->setChecked(true); break;
		case debug_view_memory::data_format::OCTAL_8BIT: dataFormatMenuItem("formatActOneOctal")->setChecked(true); break;
		case debug_view_memory::data_format::OCTAL_16BIT: dataFormatMenuItem("formatActTwoOctal")->setChecked(true); break;
		case debug_view_memory::data_format::OCTAL_32BIT: dataFormatMenuItem("formatActFourOctal")->setChecked(true); break;
		case debug_view_memory::data_format::OCTAL_64BIT: dataFormatMenuItem("formatActEightOctal")->setChecked(true); break;
		case debug_view_memory::data_format::FLOAT_32BIT: dataFormatMenuItem("formatAct32bitFloat")->setChecked(true); break;
		case debug_view_memory::data_format::FLOAT_64BIT: dataFormatMenuItem("formatAct64bitFloat")->setChecked(true); break;
		case debug_view_memory::data_format::FLOAT_80BIT: dataFormatMenuItem("formatAct80bitFloat")->setChecked(true); break;
		default: break;
		}
		switch (memView->address_radix())
		{
		case 8: dataFormatMenuItem("radixOctal")->setChecked(true); break;
		case 10: dataFormatMenuItem("radixDecimal")->setChecked(true); break;
		case 16: dataFormatMenuItem("radixHexadecimal")->setChecked(true); break;
		default: break;
		}
	}
}


void MemoryWindow::expressionSubmitted()
{
	const QString expression = m_inputEdit->text();
	downcast<debug_view_memory *>(m_memTable->view())->set_expression(expression.toLocal8Bit().data());

	// Make the cursor pop
	m_memTable->view()->set_cursor_visible(true);

	// Check where the cursor is and adjust the scroll accordingly
	debug_view_xy cursorPosition = m_memTable->view()->cursor_position();
	// TODO: check if the region is already visible?
	m_memTable->verticalScrollBar()->setValue(cursorPosition.y);

	m_memTable->update();
	m_memTable->viewport()->update();
}


void MemoryWindow::formatChanged(QAction* changedTo)
{
	debug_view_memory *memView = downcast<debug_view_memory*>(m_memTable->view());

	if (changedTo->text() == "1-byte hexadecimal")
		memView->set_data_format(debug_view_memory::data_format::HEX_8BIT);
	else if (changedTo->text() == "2-byte hexadecimal")
		memView->set_data_format(debug_view_memory::data_format::HEX_16BIT);
	else if (changedTo->text() == "4-byte hexadecimal")
		memView->set_data_format(debug_view_memory::data_format::HEX_32BIT);
	else if (changedTo->text() == "8-byte hexadecimal")
		memView->set_data_format(debug_view_memory::data_format::HEX_64BIT);
	else if (changedTo->text() == "1-byte octal")
		memView->set_data_format(debug_view_memory::data_format::OCTAL_8BIT);
	else if (changedTo->text() == "2-byte octal")
		memView->set_data_format(debug_view_memory::data_format::OCTAL_16BIT);
	else if (changedTo->text() == "4-byte octal")
		memView->set_data_format(debug_view_memory::data_format::OCTAL_32BIT);
	else if (changedTo->text() == "8-byte octal")
		memView->set_data_format(debug_view_memory::data_format::OCTAL_64BIT);
	else if (changedTo->text() == "32 bit floating point")
		memView->set_data_format(debug_view_memory::data_format::FLOAT_32BIT);
	else if (changedTo->text() == "64 bit floating point")
		memView->set_data_format(debug_view_memory::data_format::FLOAT_64BIT);
	else if (changedTo->text() == "80 bit floating point")
		memView->set_data_format(debug_view_memory::data_format::FLOAT_80BIT);

	m_memTable->viewport()->update();
}


void MemoryWindow::addressChanged(QAction* changedTo)
{
	debug_view_memory *memView = downcast<debug_view_memory *>(m_memTable->view());

	if (changedTo->text() == "Logical Addresses")
		memView->set_physical(false);
	else if (changedTo->text() == "Physical Addresses")
		memView->set_physical(true);

	m_memTable->viewport()->update();
}


void MemoryWindow::radixChanged(QAction* changedTo)
{
	debug_view_memory *memView = downcast<debug_view_memory *>(m_memTable->view());

	if (changedTo->text() == "Hexadecimal Addresses")
		memView->set_address_radix(16);
	else if (changedTo->text() == "Decimal Addresses")
		memView->set_address_radix(10);
	else if (changedTo->text() == "Octal Addresses")
		memView->set_address_radix(8);

	m_memTable->viewport()->update();
}


void MemoryWindow::reverseChanged(bool changedTo)
{
	debug_view_memory *memView = downcast<debug_view_memory*>(m_memTable->view());
	memView->set_reverse(changedTo);
	m_memTable->viewport()->update();
}


void MemoryWindow::increaseBytesPerLine(bool changedTo)
{
	debug_view_memory *memView = downcast<debug_view_memory*>(m_memTable->view());
	memView->set_chunks_per_row(memView->chunks_per_row() + 1);
	m_memTable->viewport()->update();
}


void MemoryWindow::decreaseBytesPerLine(bool checked)
{
	debug_view_memory *memView = downcast<debug_view_memory *>(m_memTable->view());
	memView->set_chunks_per_row(memView->chunks_per_row() - 1);
	m_memTable->viewport()->update();
}


void MemoryWindow::populateComboBox()
{
	if (!m_memTable)
		return;

	m_memoryComboBox->clear();
	for (auto &source : m_memTable->view()->source_list())
		m_memoryComboBox->addItem(source->name());
}


void MemoryWindow::setToCurrentCpu()
{
	device_t *curCpu = m_machine.debugger().console().get_visible_cpu();
	if (curCpu)
	{
		const debug_view_source *source = m_memTable->view()->source_for_device(curCpu);
		if (source)
		{
			const int listIndex = m_memTable->view()->source_index(*source);
			m_memoryComboBox->setCurrentIndex(listIndex);
		}
	}
}


// I have a hard time storing QActions as class members.  This is a substitute.
QAction *MemoryWindow::dataFormatMenuItem(const QString& itemName)
{
	QList<QMenu *> menus = menuBar()->findChildren<QMenu *>();
	for (int i = 0; i < menus.length(); i++)
	{
		if (menus[i]->title() != "&Options")
			continue;
		QList<QAction *> actions = menus[i]->actions();
		for (int j = 0; j < actions.length(); j++)
		{
			if (actions[j]->objectName() == itemName)
				return actions[j];
		}
	}
	return nullptr;
}


//=========================================================================
//  DebuggerMemView
//=========================================================================
void DebuggerMemView::addItemsToContextMenu(QMenu *menu)
{
	DebuggerView::addItemsToContextMenu(menu);

	if (view()->cursor_visible())
	{
		debug_view_memory &memView = downcast<debug_view_memory &>(*view());
		debug_view_memory_source const &source = downcast<debug_view_memory_source const &>(*memView.source());
		address_space *const addressSpace = source.space();
		if (addressSpace)
		{
			// get the last known PC to write to this memory location
			debug_view_xy const pos = memView.cursor_position();
			offs_t const address = addressSpace->byte_to_address(memView.addressAtCursorPosition(pos));
			offs_t a = address & addressSpace->logaddrmask();
			bool good = false;
			if (!addressSpace->device().memory().translate(addressSpace->spacenum(), TRANSLATE_READ_DEBUG, a))
			{
				m_lastPc = "Bad address";
			}
			else
			{
				uint64_t memValue = addressSpace->unmap();
				auto dis = addressSpace->device().machine().disable_side_effects();
				switch (addressSpace->data_width())
				{
				case  8: memValue = addressSpace->read_byte(a);            break;
				case 16: memValue = addressSpace->read_word_unaligned(a);  break;
				case 32: memValue = addressSpace->read_dword_unaligned(a); break;
				case 64: memValue = addressSpace->read_qword_unaligned(a); break;
				}

				offs_t const pc = source.device()->debug()->track_mem_pc_from_space_address_data(
						addressSpace->spacenum(),
						address,
						memValue);
				if (pc != offs_t(-1))
				{
					if (addressSpace->is_octal())
						m_lastPc = QString("Address %1 written at PC=%2").arg(address, 2, 8).arg(pc, 2, 8);
					else
						m_lastPc = QString("Address %1 written at PC=%2").arg(address, 2, 16).arg(pc, 2, 16);
					good = true;
				}
				else
				{
					m_lastPc = "Unknown PC";
				}
			}

			if (!menu->isEmpty())
				menu->addSeparator();
			QAction *const act = new QAction(m_lastPc, menu);
			act->setEnabled(good);
			connect(act, &QAction::triggered, this, &DebuggerMemView::copyLastPc);
			menu->addAction(act);
		}
	}
}


void DebuggerMemView::copyLastPc()
{
	QApplication::clipboard()->setText(m_lastPc);
}


//=========================================================================
//  MemoryWindowQtConfig
//=========================================================================
void MemoryWindowQtConfig::buildFromQWidget(QWidget *widget)
{
	WindowQtConfig::buildFromQWidget(widget);
	MemoryWindow *window = dynamic_cast<MemoryWindow *>(widget);
	QComboBox *memoryRegion = window->findChild<QComboBox*>("memoryregion");
	m_memoryRegion = memoryRegion->currentIndex();

	QAction *reverse = window->findChild<QAction *>("reverse");
	m_reverse = reverse->isChecked();

	QActionGroup *addressGroup = window->findChild<QActionGroup*>("addressgroup");
	if (addressGroup->checkedAction()->text() == "Logical Addresses")
		m_addressMode = 0;
	else if (addressGroup->checkedAction()->text() == "Physical Addresses")
		m_addressMode = 1;

	QActionGroup *radixGroup = window->findChild<QActionGroup*>("radixgroup");
	if (radixGroup->checkedAction()->text() == "Hexadecimal Addresses")
		m_addressRadix = 0;
	else if (radixGroup->checkedAction()->text() == "Decimal Addresses")
		m_addressRadix = 1;
	else if (radixGroup->checkedAction()->text() == "Octal Addresses")
		m_addressRadix = 2;

	QActionGroup *dataFormat = window->findChild<QActionGroup*>("dataformat");
	if (dataFormat->checkedAction()->text() == "1-byte hexadecimal")
		m_dataFormat = 0;
	else if (dataFormat->checkedAction()->text() == "2-byte hexadecimal")
		m_dataFormat = 1;
	else if (dataFormat->checkedAction()->text() == "4-byte hexadecimal")
		m_dataFormat = 2;
	else if (dataFormat->checkedAction()->text() == "8-byte hexadecimal")
		m_dataFormat = 3;
	else if (dataFormat->checkedAction()->text() == "1-byte octal")
		m_dataFormat = 4;
	else if (dataFormat->checkedAction()->text() == "2-byte octal")
		m_dataFormat = 5;
	else if (dataFormat->checkedAction()->text() == "4-byte octal")
		m_dataFormat = 6;
	else if (dataFormat->checkedAction()->text() == "8-byte octal")
		m_dataFormat = 7;
	else if (dataFormat->checkedAction()->text() == "32 bit floating point")
		m_dataFormat = 8;
	else if (dataFormat->checkedAction()->text() == "64 bit floating point")
		m_dataFormat = 9;
	else if (dataFormat->checkedAction()->text() == "80 bit floating point")
		m_dataFormat = 10;
}


void MemoryWindowQtConfig::applyToQWidget(QWidget *widget)
{
	WindowQtConfig::applyToQWidget(widget);
	MemoryWindow *window = dynamic_cast<MemoryWindow *>(widget);
	QComboBox *memoryRegion = window->findChild<QComboBox *>("memoryregion");
	memoryRegion->setCurrentIndex(m_memoryRegion);

	QAction *reverse = window->findChild<QAction *>("reverse");
	if (m_reverse)
		reverse->trigger();

	QActionGroup *addressGroup = window->findChild<QActionGroup*>("addressgroup");
	addressGroup->actions()[m_addressMode]->trigger();

	QActionGroup *radixGroup = window->findChild<QActionGroup*>("radixgroup");
	radixGroup->actions()[m_addressRadix]->trigger();

	QActionGroup *dataFormat = window->findChild<QActionGroup*>("dataformat");
	dataFormat->actions()[m_dataFormat]->trigger();
}


void MemoryWindowQtConfig::addToXmlDataNode(util::xml::data_node &node) const
{
	WindowQtConfig::addToXmlDataNode(node);
	node.set_attribute_int("memoryregion", m_memoryRegion);
	node.set_attribute_int("reverse", m_reverse);
	node.set_attribute_int("addressmode", m_addressMode);
	node.set_attribute_int("addressradix", m_addressRadix);
	node.set_attribute_int("dataformat", m_dataFormat);
}


void MemoryWindowQtConfig::recoverFromXmlNode(util::xml::data_node const &node)
{
	WindowQtConfig::recoverFromXmlNode(node);
	m_memoryRegion = node.get_attribute_int("memoryregion", m_memoryRegion);
	m_reverse = node.get_attribute_int("reverse", m_reverse);
	m_addressMode = node.get_attribute_int("addressmode", m_addressMode);
	m_addressRadix = node.get_attribute_int("addressradix", m_addressRadix);
	m_dataFormat = node.get_attribute_int("dataformat", m_dataFormat);
}
