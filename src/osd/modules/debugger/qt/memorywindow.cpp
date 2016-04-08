// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
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

#include "memorywindow.h"

#include "debug/dvmemory.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"


MemoryWindow::MemoryWindow(running_machine* machine, QWidget* parent) :
	WindowQt(machine, NULL)
{
	setWindowTitle("Debug: Memory View");

	if (parent != NULL)
	{
		QPoint parentPos = parent->pos();
		setGeometry(parentPos.x()+100, parentPos.y()+100, 800, 400);
	}

	//
	// The main frame and its input and log widgets
	//
	QFrame* mainWindowFrame = new QFrame(this);

	// The top frame & groupbox that contains the input widgets
	QFrame* topSubFrame = new QFrame(mainWindowFrame);

	// The input edit
	m_inputEdit = new QLineEdit(topSubFrame);
	connect(m_inputEdit, &QLineEdit::returnPressed, this, &MemoryWindow::expressionSubmitted);

	// The memory space combo box
	m_memoryComboBox = new QComboBox(topSubFrame);
	m_memoryComboBox->setObjectName("memoryregion");
	m_memoryComboBox->setMinimumWidth(300);
	connect(m_memoryComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MemoryWindow::memoryRegionChanged);

	// The main memory window
	m_memTable = new DebuggerMemView(DVT_MEMORY, m_machine, this);

	// Layout
	QHBoxLayout* subLayout = new QHBoxLayout(topSubFrame);
	subLayout->addWidget(m_inputEdit);
	subLayout->addWidget(m_memoryComboBox);
	subLayout->setSpacing(3);
	subLayout->setContentsMargins(2,2,2,2);

	QVBoxLayout* vLayout = new QVBoxLayout(mainWindowFrame);
	vLayout->setSpacing(3);
	vLayout->setContentsMargins(2,2,2,2);
	vLayout->addWidget(topSubFrame);
	vLayout->addWidget(m_memTable);

	setCentralWidget(mainWindowFrame);

	//
	// Menu bars
	//

	// Create a data format group
	QActionGroup* dataFormat = new QActionGroup(this);
	dataFormat->setObjectName("dataformat");
	QAction* formatActOne  = new QAction("1-byte chunks", this);
	QAction* formatActTwo  = new QAction("2-byte chunks", this);
	QAction* formatActFour = new QAction("4-byte chunks", this);
	QAction* formatActEight = new QAction("8-byte chunks", this);
	QAction* formatAct32bitFloat = new QAction("32 bit floating point", this);
	QAction* formatAct64bitFloat = new QAction("64 bit floating point", this);
	QAction* formatAct80bitFloat = new QAction("80 bit floating point", this);
	formatActOne->setObjectName("formatActOne");
	formatActTwo->setObjectName("formatActTwo");
	formatActFour->setObjectName("formatActFour");
	formatActEight->setObjectName("formatActEight");
	formatAct32bitFloat->setObjectName("formatAct32bitFloat");
	formatAct64bitFloat->setObjectName("formatAct64bitFloat");
	formatAct80bitFloat->setObjectName("formatAct80bitFloat");
	formatActOne->setCheckable(true);
	formatActTwo->setCheckable(true);
	formatActFour->setCheckable(true);
	formatActEight->setCheckable(true);
	formatAct32bitFloat->setCheckable(true);
	formatAct64bitFloat->setCheckable(true);
	formatAct80bitFloat->setCheckable(true);
	formatActOne->setActionGroup(dataFormat);
	formatActTwo->setActionGroup(dataFormat);
	formatActFour->setActionGroup(dataFormat);
	formatActEight->setActionGroup(dataFormat);
	formatAct32bitFloat->setActionGroup(dataFormat);
	formatAct64bitFloat->setActionGroup(dataFormat);
	formatAct80bitFloat->setActionGroup(dataFormat);
	formatActOne->setShortcut(QKeySequence("Ctrl+1"));
	formatActTwo->setShortcut(QKeySequence("Ctrl+2"));
	formatActFour->setShortcut(QKeySequence("Ctrl+4"));
	formatActEight->setShortcut(QKeySequence("Ctrl+8"));
	formatAct32bitFloat->setShortcut(QKeySequence("Ctrl+9"));
	formatActOne->setChecked(true);
	connect(dataFormat, &QActionGroup::triggered, this, &MemoryWindow::formatChanged);
	// Create a address display group
	QActionGroup* addressGroup = new QActionGroup(this);
	addressGroup->setObjectName("addressgroup");
	QAction* addressActLogical = new QAction("Logical Addresses", this);
	QAction* addressActPhysical = new QAction("Physical Addresses", this);
	addressActLogical->setCheckable(true);
	addressActPhysical->setCheckable(true);
	addressActLogical->setActionGroup(addressGroup);
	addressActPhysical->setActionGroup(addressGroup);
	addressActLogical->setShortcut(QKeySequence("Ctrl+G"));
	addressActPhysical->setShortcut(QKeySequence("Ctrl+Y"));
	addressActLogical->setChecked(true);
	connect(addressGroup, &QActionGroup::triggered, this, &MemoryWindow::addressChanged);

	// Create a reverse view radio
	QAction* reverseAct = new QAction("Reverse View", this);
	reverseAct->setObjectName("reverse");
	reverseAct->setCheckable(true);
	reverseAct->setShortcut(QKeySequence("Ctrl+R"));
	connect(reverseAct, &QAction::toggled, this, &MemoryWindow::reverseChanged);

	// Create increase and decrease bytes-per-line actions
	QAction* increaseBplAct = new QAction("Increase Bytes Per Line", this);
	QAction* decreaseBplAct = new QAction("Decrease Bytes Per Line", this);
	increaseBplAct->setShortcut(QKeySequence("Ctrl+P"));
	decreaseBplAct->setShortcut(QKeySequence("Ctrl+O"));
	connect(increaseBplAct, &QAction::triggered, this, &MemoryWindow::increaseBytesPerLine);
	connect(decreaseBplAct, &QAction::triggered, this, &MemoryWindow::decreaseBytesPerLine);

	// Assemble the options menu
	QMenu* optionsMenu = menuBar()->addMenu("&Options");
	optionsMenu->addActions(dataFormat->actions());
	optionsMenu->addSeparator();
	optionsMenu->addActions(addressGroup->actions());
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
	m_memTable->view()->set_source(*m_memTable->view()->source_list().find(index));
	m_memTable->viewport()->update();

	// Update the data format radio buttons to the memory region's default
	debug_view_memory* memView = downcast<debug_view_memory*>(m_memTable->view());
	switch(memView->get_data_format())
	{
		case 1: dataFormatMenuItem("formatActOne")->setChecked(true); break;
		case 2: dataFormatMenuItem("formatActTwo")->setChecked(true); break;
		case 4: dataFormatMenuItem("formatActFour")->setChecked(true); break;
		case 8: dataFormatMenuItem("formatActEight")->setChecked(true); break;
		case 9: dataFormatMenuItem("formatAct32bitFloat")->setChecked(true); break;
		case 10: dataFormatMenuItem("formatAct64bitFloat")->setChecked(true); break;
		case 11: dataFormatMenuItem("formatAct80bitFloat")->setChecked(true); break;
		default: break;
	}
}


void MemoryWindow::expressionSubmitted()
{
	const QString expression = m_inputEdit->text();
	downcast<debug_view_memory*>(m_memTable->view())->set_expression(expression.toLocal8Bit().data());

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
	debug_view_memory* memView = downcast<debug_view_memory*>(m_memTable->view());
	if (changedTo->text() == "1-byte chunks")
	{
		memView->set_data_format(1);
	}
	else if (changedTo->text() == "2-byte chunks")
	{
		memView->set_data_format(2);
	}
	else if (changedTo->text() == "4-byte chunks")
	{
		memView->set_data_format(4);
	}
	else if (changedTo->text() == "8-byte chunks")
	{
		memView->set_data_format(8);
	}
	else if (changedTo->text() == "32 bit floating point")
	{
		memView->set_data_format(9);
	}
	else if (changedTo->text() == "64 bit floating point")
	{
		memView->set_data_format(10);
	}
	else if (changedTo->text() == "80 bit floating point")
	{
		memView->set_data_format(11);
	}
	m_memTable->viewport()->update();
}


void MemoryWindow::addressChanged(QAction* changedTo)
{
	debug_view_memory* memView = downcast<debug_view_memory*>(m_memTable->view());
	if (changedTo->text() == "Logical Addresses")
	{
		memView->set_physical(false);
	}
	else if (changedTo->text() == "Physical Addresses")
	{
		memView->set_physical(true);
	}
	m_memTable->viewport()->update();
}


void MemoryWindow::reverseChanged(bool changedTo)
{
	debug_view_memory* memView = downcast<debug_view_memory*>(m_memTable->view());
	memView->set_reverse(changedTo);
	m_memTable->viewport()->update();
}


void MemoryWindow::increaseBytesPerLine(bool changedTo)
{
	debug_view_memory* memView = downcast<debug_view_memory*>(m_memTable->view());
	memView->set_chunks_per_row(memView->chunks_per_row() + 1);
	m_memTable->viewport()->update();
}


void MemoryWindow::decreaseBytesPerLine(bool checked)
{
	debug_view_memory* memView = downcast<debug_view_memory*>(m_memTable->view());
	memView->set_chunks_per_row(memView->chunks_per_row() - 1);
	m_memTable->viewport()->update();
}


void MemoryWindow::populateComboBox()
{
	if (m_memTable == NULL)
		return;

	m_memoryComboBox->clear();
	for (const debug_view_source &source : m_memTable->view()->source_list())
	{
		m_memoryComboBox->addItem(source.name());
	}
}


void MemoryWindow::setToCurrentCpu()
{
	device_t* curCpu = debug_cpu_get_visible_cpu(*m_machine);
	const debug_view_source *source = m_memTable->view()->source_for_device(curCpu);
	const int listIndex = m_memTable->view()->source_list().indexof(*source);
	m_memoryComboBox->setCurrentIndex(listIndex);
}


// I have a hard time storing QActions as class members.  This is a substitute.
QAction* MemoryWindow::dataFormatMenuItem(const QString& itemName)
{
	QList<QMenu*> menus = menuBar()->findChildren<QMenu*>();
	for (int i = 0; i < menus.length(); i++)
	{
		if (menus[i]->title() != "&Options") continue;
		QList<QAction*> actions = menus[i]->actions();
		for (int j = 0; j < actions.length(); j++)
		{
			if (actions[j]->objectName() == itemName)
				return actions[j];
		}
	}
	return NULL;
}


//=========================================================================
//  DebuggerMemView
//=========================================================================
void DebuggerMemView::mousePressEvent(QMouseEvent* event)
{
	const bool leftClick = event->button() == Qt::LeftButton;
	const bool rightClick = event->button() == Qt::RightButton;

	if (leftClick || rightClick)
	{
		QFontMetrics actualFont = fontMetrics();
		const double fontWidth = actualFont.width(QString(100, '_')) / 100.;
		const int fontHeight = MAX(1, actualFont.height());

		debug_view_xy topLeft = view()->visible_position();
		debug_view_xy clickViewPosition;
		clickViewPosition.x = topLeft.x + (event->x() / fontWidth);
		clickViewPosition.y = topLeft.y + (event->y() / fontHeight);
		if (leftClick)
		{
			view()->process_click(DCK_LEFT_CLICK, clickViewPosition);
		}
		else if (rightClick)
		{
			// Display the last known PC to write to this memory location & copy it onto the clipboard
			debug_view_memory* memView = downcast<debug_view_memory*>(view());
			const offs_t address = memView->addressAtCursorPosition(clickViewPosition);
			const debug_view_memory_source* source = downcast<const debug_view_memory_source*>(memView->source());
			address_space* addressSpace = source->space();
			const int nativeDataWidth = addressSpace->data_width() / 8;
			const UINT64 memValue = debug_read_memory(*addressSpace,
														addressSpace->address_to_byte(address),
														nativeDataWidth,
														true);
			const offs_t pc = source->device()->debug()->track_mem_pc_from_space_address_data(addressSpace->spacenum(),
																								address,
																								memValue);
			if (pc != (offs_t)(-1))
			{
				// TODO: You can specify a box that the tooltip stays alive within - might be good?
				const QString addressAndPc = QString("Address %1 written at PC=%2").arg(address, 2, 16).arg(pc, 2, 16);
				QToolTip::showText(QCursor::pos(), addressAndPc, NULL);

				// Copy the PC into the clipboard as well
				QClipboard *clipboard = QApplication::clipboard();
				clipboard->setText(QString("%1").arg(pc, 2, 16));
			}
			else
			{
				QToolTip::showText(QCursor::pos(), "UNKNOWN PC", NULL);
			}
		}

		viewport()->update();
		update();
	}
}


//=========================================================================
//  MemoryWindowQtConfig
//=========================================================================
void MemoryWindowQtConfig::buildFromQWidget(QWidget* widget)
{
	WindowQtConfig::buildFromQWidget(widget);
	MemoryWindow* window = dynamic_cast<MemoryWindow*>(widget);
	QComboBox* memoryRegion = window->findChild<QComboBox*>("memoryregion");
	m_memoryRegion = memoryRegion->currentIndex();

	QAction* reverse = window->findChild<QAction*>("reverse");
	m_reverse = reverse->isChecked();

	QActionGroup* addressGroup = window->findChild<QActionGroup*>("addressgroup");
	if (addressGroup->checkedAction()->text() == "Logical Addresses")
		m_addressMode = 0;
	else if (addressGroup->checkedAction()->text() == "Physical Addresses")
		m_addressMode = 1;

	QActionGroup* dataFormat = window->findChild<QActionGroup*>("dataformat");
	if (dataFormat->checkedAction()->text() == "1-byte chunks")
		m_dataFormat = 0;
	else if (dataFormat->checkedAction()->text() == "2-byte chunks")
		m_dataFormat = 1;
	else if (dataFormat->checkedAction()->text() == "4-byte chunks")
		m_dataFormat = 2;
	else if (dataFormat->checkedAction()->text() == "8-byte chunks")
		m_dataFormat = 3;
	else if (dataFormat->checkedAction()->text() == "32 bit floating point")
		m_dataFormat = 4;
	else if (dataFormat->checkedAction()->text() == "64 bit floating point")
		m_dataFormat = 5;
	else if (dataFormat->checkedAction()->text() == "80 bit floating point")
		m_dataFormat = 6;
}


void MemoryWindowQtConfig::applyToQWidget(QWidget* widget)
{
	WindowQtConfig::applyToQWidget(widget);
	MemoryWindow* window = dynamic_cast<MemoryWindow*>(widget);
	QComboBox* memoryRegion = window->findChild<QComboBox*>("memoryregion");
	memoryRegion->setCurrentIndex(m_memoryRegion);

	QAction* reverse = window->findChild<QAction*>("reverse");
	if (m_reverse) reverse->trigger();

	QActionGroup* addressGroup = window->findChild<QActionGroup*>("addressgroup");
	addressGroup->actions()[m_addressMode]->trigger();

	QActionGroup* dataFormat = window->findChild<QActionGroup*>("dataformat");
	dataFormat->actions()[m_dataFormat]->trigger();
}


void MemoryWindowQtConfig::addToXmlDataNode(xml_data_node* node) const
{
	WindowQtConfig::addToXmlDataNode(node);
	xml_set_attribute_int(node, "memoryregion", m_memoryRegion);
	xml_set_attribute_int(node, "reverse", m_reverse);
	xml_set_attribute_int(node, "addressmode", m_addressMode);
	xml_set_attribute_int(node, "dataformat", m_dataFormat);
}


void MemoryWindowQtConfig::recoverFromXmlNode(xml_data_node* node)
{
	WindowQtConfig::recoverFromXmlNode(node);
	m_memoryRegion = xml_get_attribute_int(node, "memoryregion", m_memoryRegion);
	m_reverse = xml_get_attribute_int(node, "reverse", m_reverse);
	m_addressMode = xml_get_attribute_int(node, "addressmode", m_addressMode);
	m_dataFormat = xml_get_attribute_int(node, "dataformat", m_dataFormat);
}
