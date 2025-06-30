// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#include "emu.h"
#include "debuggerview.h"

#include "../xmlconfig.h"

#include "modules/lib/osdobj_common.h"

#include "xmlfile.h"

#include <QtCore/QMimeData>
#include <QtGui/QClipboard>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QScrollBar>


namespace osd::debugger::qt {

DebuggerView::DebuggerView(
		debug_view_type type,
		running_machine &machine,
		QWidget *parent) :
	QAbstractScrollArea(parent),
	m_machine(machine),
	m_view(nullptr),
	m_preferBottom(false)
{
	// I like setting the font per-view since it doesn't override the menu fonts.
	const char *const selectedFont(downcast<osd_options &>(m_machine.options()).debugger_font());
	const float selectedFontSize(downcast<osd_options &>(m_machine.options()).debugger_font_size());
	QFont viewFontRequest((!*selectedFont || !strcmp(selectedFont, OSDOPTVAL_AUTO)) ? "Courier New" : selectedFont);
	viewFontRequest.setFixedPitch(true);
	viewFontRequest.setStyleHint(QFont::TypeWriter);
	viewFontRequest.setPointSize((selectedFontSize <= 0) ? 11 : selectedFontSize);
	setFont(viewFontRequest);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	m_view = m_machine.debug_view().alloc_view(
			type,
			DebuggerView::debuggerViewUpdate,
			this);

	connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &DebuggerView::verticalScrollSlot);
	connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, &DebuggerView::horizontalScrollSlot);
}


DebuggerView::~DebuggerView()
{
	if (m_view)
		m_machine.debug_view().free_view(*m_view);
}


int DebuggerView::sourceIndex() const
{
	if (m_view)
	{
		debug_view_source const *const source = m_view->source();
		if (source)
			return m_view->source_index(*source);
	}
	return -1;
}


void DebuggerView::paintEvent(QPaintEvent *event)
{
	// Tell the MAME debug view how much real estate is available
	QFontMetrics actualFont = fontMetrics();
	double const fontWidth = actualFont.horizontalAdvance(QString(100, '_')) / 100.;
	int const fontHeight = std::max(1, actualFont.lineSpacing());
	int const contentWidth = width() - verticalScrollBar()->width();
	int const lineWidth = contentWidth / fontWidth;
	bool const fullWidth = lineWidth >= m_view->total_size().x;
	int const contentHeight = height() - (fullWidth ? 0 : horizontalScrollBar()->height());
	m_view->set_visible_size(debug_view_xy(lineWidth, contentHeight / fontHeight));

	updateScrollRangesAndValues();
	horizontalScrollBar()->setPageStep(lineWidth - 1);
	verticalScrollBar()->setPageStep((contentHeight / fontHeight) - 1);

	const auto palette = QApplication::palette();

	// Draw the viewport widget
	QPainter painter(viewport());
	painter.fillRect(0, 0, width(), height(), QBrush(palette.color(QPalette::Window)));
	painter.setBackgroundMode(Qt::OpaqueMode);
	painter.setBackground(palette.color(QPalette::Window));

	// Background control
	QBrush bgBrush;
	bgBrush.setStyle(Qt::SolidPattern);
	painter.setPen(QPen(QPalette::WindowText));

	const debug_view_xy visibleCharDims = m_view->visible_size();
	const debug_view_char *viewdata = m_view->viewdata();

	for (int y = 0; y < visibleCharDims.y; y++, viewdata += visibleCharDims.x)
	{
		int width = 1;
		for (int x = 0; x < visibleCharDims.x; x += width)
		{
			const unsigned char textAttr = viewdata[x].attrib;

			// Text color handling
			QColor fgColor(palette.color(QPalette::WindowText));
			QColor bgColor(palette.color(QPalette::Window));

			if (textAttr & DCA_VISITED)
				bgColor.setRgb(0xc6, 0xe2, 0xff);

			if (textAttr & DCA_ANCILLARY)
				bgColor.setRgb(palette.color(QPalette::Base).rgb());

			if (textAttr & DCA_SELECTED)
				bgColor.setRgb(0xff, 0x80, 0x80);

			if (textAttr & DCA_CURRENT)
				bgColor.setRgb(palette.color(QPalette::Highlight).rgb());

			if ((textAttr & DCA_SELECTED) && (textAttr & DCA_CURRENT))
			{
				#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
					bgColor.setRgb(palette.color(QPalette::Accent).rgb());
				#else
					bgColor.setRgb(0xff,0xc0,0x80);
				#endif
			}

			if (textAttr & DCA_CHANGED)
				fgColor.setRgb(0xff, 0x00, 0x00);

			if (textAttr & DCA_INVALID)
				fgColor.setRgb(0x00, 0x00, 0xff);

			if (textAttr & DCA_DISABLED)
			{
				fgColor.setRgb(
						(fgColor.red()   + bgColor.red())   >> 1,
						(fgColor.green() + bgColor.green()) >> 1,
						(fgColor.blue()  + bgColor.blue())  >> 1);
			}

			if (textAttr & DCA_COMMENT)
				fgColor.setRgb(0x00, 0x80, 0x00);

			bgBrush.setColor(bgColor);
			painter.setBackground(bgBrush);
			painter.setPen(QPen(fgColor));

			QString text(QChar(viewdata[x].byte));
			for (width = 1; (x + width) < visibleCharDims.x; width++)
			{
				if (textAttr != viewdata[x + width].attrib)
					break;
				text.append(QChar(viewdata[x + width].byte));
			}

			// Your characters are not guaranteed to take up the entire length x fontWidth x fontHeight, so fill before.
			painter.fillRect(
					x * fontWidth,
					y * fontHeight,
					((x + width) < visibleCharDims.x) ? (width * fontWidth) : (contentWidth - (x * fontWidth)),
					fontHeight,
					bgBrush);

			if (((y + 1) == visibleCharDims.y) && (contentHeight > (visibleCharDims.y * fontHeight)))
			{
				if (textAttr & DCA_ANCILLARY)
					bgColor.setRgb(palette.color(QPalette::Base).rgb());
				else
					bgColor.setRgb(palette.color(QPalette::Window).rgb());
				bgBrush.setColor(bgColor);
				painter.fillRect(
						x * fontWidth,
						visibleCharDims.y * fontHeight,
						((x + width) < visibleCharDims.x) ? (width * fontWidth) : (contentWidth - (x * fontWidth)),
						contentHeight - (visibleCharDims.y * fontHeight),
						bgBrush);
			}

			// There is a touchy interplay between font height, drawing difference, visible position, etc
			// Fonts don't get drawn "down and to the left" like boxes, so some wiggling is needed.
			// Second parameter (baseline) can't be too low or underscores will get overwritten by next line
			painter.drawText(x * fontWidth, (y * fontHeight + int(float(fontHeight) * 0.75)), text);
		}
	}
}


void DebuggerView::restoreConfigurationFromNode(util::xml::data_node const &node)
{
	if (m_view->cursor_supported())
	{
		util::xml::data_node const *const selection = node.get_child(NODE_WINDOW_SELECTION);
		if (selection)
		{
			debug_view_xy pos = m_view->cursor_position();
			m_view->set_cursor_visible(0 != selection->get_attribute_int(ATTR_SELECTION_CURSOR_VISIBLE, m_view->cursor_visible() ? 1 : 0));
			selection->get_attribute_int(ATTR_SELECTION_CURSOR_X, pos.x);
			selection->get_attribute_int(ATTR_SELECTION_CURSOR_Y, pos.y);
			m_view->set_cursor_position(pos);
		}
	}
}


void DebuggerView::saveConfigurationToNode(util::xml::data_node &node)
{
	if (m_view->cursor_supported())
	{
		util::xml::data_node *const selection = node.add_child(NODE_WINDOW_SELECTION, nullptr);
		if (selection)
		{
			debug_view_xy const pos = m_view->cursor_position();
			selection->set_attribute_int(ATTR_SELECTION_CURSOR_VISIBLE, m_view->cursor_visible() ? 1 : 0);
			selection->set_attribute_int(ATTR_SELECTION_CURSOR_X, pos.x);
			selection->set_attribute_int(ATTR_SELECTION_CURSOR_Y, pos.y);
		}
	}
}


void DebuggerView::keyPressEvent(QKeyEvent* event)
{
	if (!m_view)
		return QWidget::keyPressEvent(event);

	Qt::KeyboardModifiers keyMods = QApplication::keyboardModifiers();
	const bool ctrlDown = keyMods.testFlag(Qt::ControlModifier);

	int keyPress = -1;
	switch (event->key())
	{
	case Qt::Key_Up:
		keyPress = DCH_UP;
		break;
	case Qt::Key_Down:
		keyPress = DCH_DOWN;
		break;
	case Qt::Key_Left:
		keyPress = DCH_LEFT;
		if (ctrlDown)
			keyPress = DCH_CTRLLEFT;
		break;
	case Qt::Key_Right:
		keyPress = DCH_RIGHT;
		if (ctrlDown)
			keyPress = DCH_CTRLRIGHT;
		break;
	case Qt::Key_PageUp:
		keyPress = DCH_PUP;
		break;
	case Qt::Key_PageDown:
		keyPress = DCH_PDOWN;
		break;
	case Qt::Key_Home:
		keyPress = DCH_HOME;
		if (ctrlDown) keyPress = DCH_CTRLHOME;
		break;
	case Qt::Key_End:
		keyPress = DCH_END;
		if (ctrlDown) keyPress = DCH_CTRLEND;
		break;
	case Qt::Key_0: keyPress = '0'; break;
	case Qt::Key_1: keyPress = '1'; break;
	case Qt::Key_2: keyPress = '2'; break;
	case Qt::Key_3: keyPress = '3'; break;
	case Qt::Key_4: keyPress = '4'; break;
	case Qt::Key_5: keyPress = '5'; break;
	case Qt::Key_6: keyPress = '6'; break;
	case Qt::Key_7: keyPress = '7'; break;
	case Qt::Key_8: keyPress = '8'; break;
	case Qt::Key_9: keyPress = '9'; break;
	case Qt::Key_A: keyPress = 'a'; break;
	case Qt::Key_B: keyPress = 'b'; break;
	case Qt::Key_C: keyPress = 'c'; break;
	case Qt::Key_D: keyPress = 'd'; break;
	case Qt::Key_E: keyPress = 'e'; break;
	case Qt::Key_F: keyPress = 'f'; break;
	default:
		return QWidget::keyPressEvent(event);
	}

	m_view->set_cursor_visible(true);
	m_view->process_char(keyPress);

	// Catch the view up with the cursor
	verticalScrollBar()->setValue(m_view->visible_position().y);

	viewport()->update();
	update();
}


void DebuggerView::mousePressEvent(QMouseEvent *event)
{
	if (!m_view)
		return;

	QFontMetrics actualFont = fontMetrics();
	const double fontWidth = actualFont.horizontalAdvance(QString(100, '_')) / 100.;
	const int fontHeight = std::max(1, actualFont.lineSpacing());

	debug_view_xy const topLeft = m_view->visible_position();
	debug_view_xy const visibleCharDims = m_view->visible_size();
	debug_view_xy clickViewPosition;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	const QPointF mousePosition = event->position();
#else
	const QPointF mousePosition = event->localPos();
#endif
	clickViewPosition.x = (std::min)(int(topLeft.x + (mousePosition.x() / fontWidth)), topLeft.x + visibleCharDims.x - 1);
	clickViewPosition.y = (std::min)(int(topLeft.y + (mousePosition.y() / fontHeight)), topLeft.y + visibleCharDims.y - 1);

	if (event->button() == Qt::LeftButton)
	{
		m_view->process_click(DCK_LEFT_CLICK, clickViewPosition);
	}
	else if (event->button() == Qt::MiddleButton)
	{
		m_view->process_click(DCK_MIDDLE_CLICK, clickViewPosition);
	}
	else if (event->button() == Qt::RightButton)
	{
		if (m_view->cursor_supported())
		{
			m_view->set_cursor_position(clickViewPosition);
			m_view->set_cursor_visible(true);
		}
	}

	viewport()->update();
	update();
}


void DebuggerView::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu *const menu = new QMenu(this);
	addItemsToContextMenu(menu);
	menu->popup(event->globalPos());
}


void DebuggerView::addItemsToContextMenu(QMenu *menu)
{
	QAction *const copyAct = new QAction("Copy Visible", menu);
	QAction *const pasteAct = new QAction("Paste", menu);
	pasteAct->setEnabled(QApplication::clipboard()->mimeData()->hasText());
	connect(copyAct, &QAction::triggered, this, &DebuggerView::copyVisibleSlot);
	connect(pasteAct, &QAction::triggered, this, &DebuggerView::pasteSlot);
	menu->addAction(copyAct);
	menu->addAction(pasteAct);
}


// Triggered from QScrollBar::valueChanged signal to send new GUI scrollbar
// value back to the debug_view
void DebuggerView::verticalScrollSlot(int value)
{
	m_view->set_visible_position(debug_view_xy(horizontalScrollBar()->value(), value));
}


// Triggered from QScrollBar::valueChanged signal to send new GUI scrollbar
// value back to the debug_view
void DebuggerView::horizontalScrollSlot(int value)
{
	m_view->set_visible_position(debug_view_xy(value, verticalScrollBar()->value()));
}


void DebuggerView::copyVisibleSlot()
{
	// get visible text
	debug_view_xy const visarea = m_view->visible_size();
	debug_view_char const *viewdata = m_view->viewdata();
	if (!viewdata)
		return;

	// turn into a plain string, trimming trailing whitespace
	std::string text;
	for (uint32_t row = 0; row < visarea.y; row++, viewdata += visarea.x)
	{
		std::string::size_type const start = text.length();
		for (uint32_t col = 0; col < visarea.x; ++col)
			text += wchar_t(viewdata[col].byte);
		std::string::size_type const nonblank = text.find_last_not_of("\t\n\v\r ");
		if (nonblank != std::string::npos)
			text.resize((std::max)(start, nonblank + 1));
		text += "\n";
	}

	// copy to the clipboard
	QApplication::clipboard()->setText(text.c_str());
}


void DebuggerView::pasteSlot()
{
	for (QChar ch : QApplication::clipboard()->text())
	{
		if ((32 <= ch.unicode()) && (127 >= ch.unicode()))
			m_view->process_char(ch.unicode());
	}
}


// Called to inform us when the debug_view has been updated
void DebuggerView::debuggerViewUpdate(debug_view &debugView, void *osdPrivate)
{
	// Get a handle to the DebuggerView being updated and redraw
	DebuggerView *dView = reinterpret_cast<DebuggerView *>(osdPrivate);

	dView->updateScrollRangesAndValues();
	dView->viewport()->update();
	dView->update();
	emit dView->updated();
}


// Update the range and current value of horizontal & vertical scrollbars.
// There's a sensitive ordering here, and the ranges should not be modified
// outside of this helper
void DebuggerView::updateScrollRangesAndValues()
{
	// m_view's m_topleft may get overwritten when calling setRange below
	// (as setRange can overwrite the scrollbar's prior value with something
	// safely in the new range, thus triggerring the QScrollBar::valueChanged signal
	// which will send that new "safe" scrollbar value back to m_view,
	// ovewriting m_topleft).  Cache it now before it's overwritten.
	debug_view_xy orig_visible_pos = m_view->visible_position();

	// Adjust scrollbar ranges
	int const horizontalScrollCharDiff = m_view->total_size().x - m_view->visible_size().x;
	horizontalScrollBar()->setRange(0, (std::max)(0, horizontalScrollCharDiff));
	int const verticalScrollCharDiff = m_view->total_size().y - m_view->visible_size().y;
	int const verticalScrollSize = (std::max)(0, verticalScrollCharDiff);
	verticalScrollBar()->setRange(0, verticalScrollSize);

	// Update the scrollbar values with what we cached from m_view
	verticalScrollBar()->setValue(orig_visible_pos.y);
	horizontalScrollBar()->setValue(orig_visible_pos.x);
	bool const atEnd = verticalScrollBar()->value() == verticalScrollBar()->maximum();
	if (m_preferBottom && atEnd)
		verticalScrollBar()->setValue(verticalScrollSize);
}

} // namespace osd::debugger::qt
