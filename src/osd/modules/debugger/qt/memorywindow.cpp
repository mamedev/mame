// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#include "emu.h"
#include "memorywindow.h"

#include "debugger.h"
#include "debug/dvmemory.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"

#include "util/xmlfile.h"

#include <QtGui/QClipboard>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtGui/QActionGroup>
#else
#include <QtWidgets/QActionGroup>
#endif
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QVBoxLayout>


namespace osd::debugger::qt {

MemoryWindow::MemoryWindow(DebuggerQt &debugger, QWidget *parent) :
	WindowQt(debugger, nullptr),
	m_inputHistory()
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
	connect(m_inputEdit, &QLineEdit::textEdited, this, &MemoryWindow::expressionEdited);
	m_inputEdit->installEventFilter(this);

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
	QAction *formatActOne  = new QAction("1-byte Chunks (Hex)", this);
	QAction *formatActTwo  = new QAction("2-byte Chunks (Hex)", this);
	QAction *formatActFour = new QAction("4-byte Chunks (Hex)", this);
	QAction *formatActEight = new QAction("8-byte Chunks (Hex)", this);
	QAction *formatActOneOctal = new QAction("1-byte Chunks (Octal)", this);
	QAction *formatActTwoOctal = new QAction("2-byte Chunks (Octal)", this);
	QAction *formatActFourOctal = new QAction("4-byte Chunks (Octal)", this);
	QAction *formatActEightOctal = new QAction("8-byte Chunks (Octal)", this);
	QAction *formatAct32bitFloat = new QAction("32-bit Floating Point", this);
	QAction *formatAct64bitFloat = new QAction("64-bit Floating Point", this);
	QAction *formatAct80bitFloat = new QAction("80-bit Floating Point", this);
	formatActOne->setData(int(debug_view_memory::data_format::HEX_8BIT));
	formatActTwo->setData(int(debug_view_memory::data_format::HEX_16BIT));
	formatActFour->setData(int(debug_view_memory::data_format::HEX_32BIT));
	formatActEight->setData(int(debug_view_memory::data_format::HEX_64BIT));
	formatActOneOctal->setData(int(debug_view_memory::data_format::OCTAL_8BIT));
	formatActTwoOctal->setData(int(debug_view_memory::data_format::OCTAL_16BIT));
	formatActFourOctal->setData(int(debug_view_memory::data_format::OCTAL_32BIT));
	formatActEightOctal->setData(int(debug_view_memory::data_format::OCTAL_64BIT));
	formatAct32bitFloat->setData(int(debug_view_memory::data_format::FLOAT_32BIT));
	formatAct64bitFloat->setData(int(debug_view_memory::data_format::FLOAT_64BIT));
	formatAct80bitFloat->setData(int(debug_view_memory::data_format::FLOAT_80BIT));
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
	addressActLogical->setData(false);
	addressActPhysical->setData(true);
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
	radixActHexadecimal->setData(16);
	radixActDecimal->setData(10);
	radixActOctal->setData(8);
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


void MemoryWindow::restoreConfiguration(util::xml::data_node const &node)
{
	WindowQt::restoreConfiguration(node);

	debug_view_memory &memView = *m_memTable->view<debug_view_memory>();

	auto const region = node.get_attribute_int(ATTR_WINDOW_MEMORY_REGION, m_memTable->sourceIndex());
	if ((0 <= region) && (m_memoryComboBox->count() > region))
		m_memoryComboBox->setCurrentIndex(region);

	auto const reverse = node.get_attribute_int(ATTR_WINDOW_MEMORY_REVERSE_COLUMNS, memView.reverse() ? 1 : 0);
	if (memView.reverse() != bool(reverse))
	{
		memView.set_reverse(bool(reverse));
		findChild<QAction *>("reverse")->setChecked(bool(reverse));
	}

	auto const mode = node.get_attribute_int(ATTR_WINDOW_MEMORY_ADDRESS_MODE, memView.physical() ? 1 : 0);
	QActionGroup *const addressGroup = findChild<QActionGroup *>("addressgroup");
	for (QAction *action : addressGroup->actions())
	{
		if (action->data().toBool() == mode)
		{
			action->trigger();
			break;
		}
	}

	auto const radix = node.get_attribute_int(ATTR_WINDOW_MEMORY_ADDRESS_RADIX, memView.address_radix());
	QActionGroup *const radixGroup = findChild<QActionGroup *>("radixgroup");
	for (QAction *action : radixGroup->actions())
	{
		if (action->data().toInt() == radix)
		{
			action->trigger();
			break;
		}
	}

	auto const format = node.get_attribute_int(ATTR_WINDOW_MEMORY_DATA_FORMAT, int(memView.get_data_format()));
	QActionGroup *const dataFormat = findChild<QActionGroup *>("dataformat");
	for (QAction *action : dataFormat->actions())
	{
		if (action->data().toInt() == format)
		{
			action->trigger();
			break;
		}
	}

	auto const chunks = node.get_attribute_int(ATTR_WINDOW_MEMORY_ROW_CHUNKS, memView.chunks_per_row());
	memView.set_chunks_per_row(chunks);

	util::xml::data_node const *const expression = node.get_child(NODE_WINDOW_EXPRESSION);
	if (expression && expression->get_value())
	{
		m_inputEdit->setText(QString::fromUtf8(expression->get_value()));
		expressionSubmitted();
	}

	m_memTable->restoreConfigurationFromNode(node);
	m_inputHistory.restoreConfigurationFromNode(node);
}


void MemoryWindow::saveConfigurationToNode(util::xml::data_node &node)
{
	WindowQt::saveConfigurationToNode(node);

	node.set_attribute_int(ATTR_WINDOW_TYPE, WINDOW_TYPE_MEMORY_VIEWER);

	debug_view_memory &memView = *m_memTable->view<debug_view_memory>();
	node.set_attribute_int(ATTR_WINDOW_MEMORY_REGION, m_memTable->sourceIndex());
	node.set_attribute_int(ATTR_WINDOW_MEMORY_REVERSE_COLUMNS, memView.reverse() ? 1 : 0);
	node.set_attribute_int(ATTR_WINDOW_MEMORY_ADDRESS_MODE, memView.physical() ? 1 : 0);
	node.set_attribute_int(ATTR_WINDOW_MEMORY_ADDRESS_RADIX, memView.address_radix());
	node.set_attribute_int(ATTR_WINDOW_MEMORY_DATA_FORMAT, int(memView.get_data_format()));
	node.set_attribute_int(ATTR_WINDOW_MEMORY_ROW_CHUNKS, memView.chunks_per_row());
	node.add_child(NODE_WINDOW_EXPRESSION, memView.expression());

	m_memTable->saveConfigurationToNode(node);
	m_inputHistory.saveConfigurationToNode(node);
}


void MemoryWindow::memoryRegionChanged(int index)
{
	if (index < m_memTable->view()->source_count())
	{
		m_memTable->view()->set_source(*m_memTable->view()->source(index));
		m_memTable->viewport()->update();

		// Update the data format radio buttons to the memory region's default
		debug_view_memory *const memView = m_memTable->view<debug_view_memory>();

		QActionGroup *const dataFormat = findChild<QActionGroup *>("dataformat");
		for (QAction *action : dataFormat->actions())
		{
			if (debug_view_memory::data_format(action->data().toInt()) == memView->get_data_format())
			{
				action->setChecked(true);
				break;
			}
		}

		QActionGroup *radixGroup = findChild<QActionGroup *>("radixgroup");
		for (QAction *action : radixGroup->actions())
		{
			if (action->data().toInt() == memView->address_radix())
			{
				action->setChecked(true);
				break;
			}
		}
	}
}


// Used to intercept the user hitting the up arrow in the input widget
bool MemoryWindow::eventFilter(QObject *obj, QEvent *event)
{
	// Only filter keypresses
	if (event->type() != QEvent::KeyPress)
		return QObject::eventFilter(obj, event);

	QKeyEvent const &keyEvent = *static_cast<QKeyEvent *>(event);

	// Catch up & down keys
	if (keyEvent.key() == Qt::Key_Escape)
	{
		m_inputEdit->setText(QString::fromUtf8(m_memTable->view<debug_view_memory>()->expression()));
		m_inputEdit->selectAll();
		m_inputHistory.reset();
		return true;
	}
	else if (keyEvent.key() == Qt::Key_Up)
	{
		QString const *const hist = m_inputHistory.previous(m_inputEdit->text());
		if (hist)
		{
			m_inputEdit->setText(*hist);
			m_inputEdit->setSelection(hist->size(), 0);
		}
		return true;
	}
	else if (keyEvent.key() == Qt::Key_Down)
	{
		QString const *const hist = m_inputHistory.next(m_inputEdit->text());
		if (hist)
		{
			m_inputEdit->setText(*hist);
			m_inputEdit->setSelection(hist->size(), 0);
		}
		return true;
	}
	else
	{
		return QObject::eventFilter(obj, event);
	}
}


void MemoryWindow::expressionSubmitted()
{
	const QString expression = m_inputEdit->text();
	m_memTable->view<debug_view_memory>()->set_expression(expression.toUtf8().data());
	m_inputEdit->selectAll();

	// Add history
	if (!expression.isEmpty())
		m_inputHistory.add(expression);
}


void MemoryWindow::expressionEdited(QString const &text)
{
	m_inputHistory.edit();
}


void MemoryWindow::formatChanged(QAction* changedTo)
{
	debug_view_memory *const memView = m_memTable->view<debug_view_memory>();
	memView->set_data_format(debug_view_memory::data_format(changedTo->data().toInt()));
	m_memTable->viewport()->update();
}


void MemoryWindow::addressChanged(QAction* changedTo)
{
	debug_view_memory *const memView = m_memTable->view<debug_view_memory>();
	memView->set_physical(changedTo->data().toBool());
	m_memTable->viewport()->update();
}


void MemoryWindow::radixChanged(QAction* changedTo)
{
	debug_view_memory *const memView = m_memTable->view<debug_view_memory>();
	memView->set_address_radix(changedTo->data().toInt());
	m_memTable->viewport()->update();
}


void MemoryWindow::reverseChanged(bool changedTo)
{
	debug_view_memory *const memView = m_memTable->view<debug_view_memory>();
	memView->set_reverse(changedTo);
	m_memTable->viewport()->update();
}


void MemoryWindow::increaseBytesPerLine(bool changedTo)
{
	debug_view_memory *const memView = m_memTable->view<debug_view_memory>();
	memView->set_chunks_per_row(memView->chunks_per_row() + 1);
	m_memTable->viewport()->update();
}


void MemoryWindow::decreaseBytesPerLine(bool checked)
{
	debug_view_memory *const memView = m_memTable->view<debug_view_memory>();
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


//=========================================================================
//  DebuggerMemView
//=========================================================================
void DebuggerMemView::addItemsToContextMenu(QMenu *menu)
{
	DebuggerView::addItemsToContextMenu(menu);

	if (view()->cursor_visible())
	{
		debug_view_memory &memView = *view<debug_view_memory>();
		debug_view_memory_source const &source = downcast<debug_view_memory_source const &>(*memView.source());
		address_space *const addressSpace = source.space();
		if (addressSpace)
		{
			// get the last known PC to write to this memory location
			debug_view_xy const pos = memView.cursor_position();
			offs_t const address = addressSpace->byte_to_address(memView.addressAtCursorPosition(pos));
			offs_t a = address & addressSpace->logaddrmask();
			bool good = false;
			address_space *tspace;
			if (!addressSpace->device().memory().translate(addressSpace->spacenum(), device_memory_interface::TR_READ, a, tspace))
			{
				m_lastPc = "Bad address";
			}
			else
			{
				uint64_t memValue = tspace->unmap();
				auto dis = tspace->device().machine().disable_side_effects();
				switch (tspace->data_width())
				{
				case  8: memValue = tspace->read_byte(a);            break;
				case 16: memValue = tspace->read_word_unaligned(a);  break;
				case 32: memValue = tspace->read_dword_unaligned(a); break;
				case 64: memValue = tspace->read_qword_unaligned(a); break;
				}

				offs_t const pc = source.device()->debug()->track_mem_pc_from_space_address_data(
						tspace->spacenum(),
						address,
						memValue);
				if (pc != offs_t(-1))
				{
					if (tspace->is_octal())
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

} // namespace osd::debugger::qt
