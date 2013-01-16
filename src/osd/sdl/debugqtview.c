#include <QtGui/QtGui>

#include "debugqtview.h"


DebuggerView::DebuggerView(const debug_view_type& type, 
                           running_machine* machine,
                           QWidget* parent) :
    QAbstractScrollArea(parent),
    m_preferBottom(false),
    m_view(NULL),
    m_machine(machine)
{
    // I like setting the font per-view since it doesn't override the menuing fonts.
    QFont viewFontRequest("Courier New");
    viewFontRequest.setFixedPitch(true);
    viewFontRequest.setPointSize(11);
    setFont(viewFontRequest);

    m_view = m_machine->debug_view().alloc_view(type, 
                                                DebuggerView::debuggerViewUpdate,
                                                this);

    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), 
            this, SLOT(verticalScrollSlot(int)));
    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), 
            this, SLOT(horizontalScrollSlot(int)));
}


void DebuggerView::paintEvent(QPaintEvent* event)
{
    // Tell the MAME debug view how much real estate is available
    QFontMetrics actualFont = fontMetrics();
    const int fontWidth = MAX(1, actualFont.maxWidth());
    const int fontHeight = MAX(1, actualFont.height());
    m_view->set_visible_size(debug_view_xy(width()/fontWidth, height()/fontHeight));


    // Handle the scroll bars
    const int verticalScrollCharDiff = m_view->total_size().y - m_view->visible_size().y;
    const int scrollSize = verticalScrollCharDiff < 0 ? 0 : verticalScrollCharDiff;
    bool atEnd = false;
    if (verticalScrollBar()->value() == verticalScrollBar()->maximum())
    {
        atEnd = true;
    }
    verticalScrollBar()->setRange(0, scrollSize);
    if (m_preferBottom && atEnd)
    {
        verticalScrollBar()->setValue(scrollSize);
    }


    // Draw the viewport widget
    QPainter painter(viewport());
    painter.fillRect(0, 0, width(), height(), QBrush(Qt::white));
    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.setBackground(QColor(255,255,255));

    // Background control
    QBrush bgBrush;
    bgBrush.setStyle(Qt::SolidPattern);
    painter.setPen(QPen(QColor(0,0,0)));

    size_t viewDataOffset = 0;
    const debug_view_xy& visibleCharDims = m_view->visible_size();
    for (int y = 0; y < visibleCharDims.y; y++)
    {
        for (int x = 0; x < visibleCharDims.x; x++)
        {
            const unsigned char textAttr = m_view->viewdata()[viewDataOffset].attrib;

            if (x == 0 || textAttr != m_view->viewdata()[viewDataOffset-1].attrib)
            {
                // Text color handling
                QColor fgColor(0,0,0);
                QColor bgColor(255,255,255);

                if(textAttr & DCA_ANCILLARY)
                {
                    bgColor.setRgb(0xe0, 0xe0, 0xe0);
                }
                if(textAttr & DCA_SELECTED)
                {
                    bgColor.setRgb(0xff, 0x80, 0x80);
                }
                if(textAttr & DCA_CURRENT)
                {
                    bgColor.setRgb(0xff, 0xff, 0x00);
                }
                if(textAttr & DCA_CHANGED)
                {
                    fgColor.setRgb(0xff, 0x00, 0x00);
                }
                if(textAttr & DCA_INVALID)
                {
                    fgColor.setRgb(0x00, 0x00, 0xff);
                }
                if(textAttr & DCA_DISABLED)
                {
                    fgColor.setRgb((fgColor.red()   + bgColor.red())   >> 1, 
                                   (fgColor.green() + bgColor.green()) >> 1,
                                   (fgColor.blue()  + bgColor.blue())  >> 1);
                }
                if(textAttr & DCA_COMMENT)
                {
                    fgColor.setRgb(0x00, 0x80, 0x00);
                }

                bgBrush.setColor(bgColor);
                painter.setBackground(bgBrush);
                painter.setPen(QPen(fgColor));
            }

            // Your character is not guaranteed to take up the entire fontWidth x fontHeight, so fill before.
            painter.fillRect(x*fontWidth, y*fontHeight, fontWidth, fontHeight, bgBrush);

            // There is a touchy interplay between font height, drawing difference, visible position, etc
            // Fonts don't get drawn "down and to the left" like boxes, so some wiggling is needed.
            painter.drawText(x*fontWidth, 
                             (y*fontHeight + (fontHeight*0.80)), 
                             QString(m_view->viewdata()[viewDataOffset].byte));
            viewDataOffset++;
        }
    }
}


void DebuggerView::keyPressEvent(QKeyEvent* event)
{
    if (m_view == NULL)
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
            if (ctrlDown) keyPress = DCH_CTRLLEFT;
            break;
        case Qt::Key_Right: 
            keyPress = DCH_RIGHT; 
            if (ctrlDown) keyPress = DCH_CTRLRIGHT;
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


void DebuggerView::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		QFontMetrics actualFont = fontMetrics();
		const int fontWidth = MAX(1, actualFont.maxWidth());
		const int fontHeight = MAX(1, actualFont.height());
		
		debug_view_xy topLeft = m_view->visible_position();
		debug_view_xy clickViewPosition;
		clickViewPosition.x = topLeft.x + (event->x() / fontWidth);
		clickViewPosition.y = topLeft.y + (event->y() / fontHeight);
		m_view->process_click(DCK_LEFT_CLICK, clickViewPosition);

		viewport()->update();
		update();
	}
}


void DebuggerView::verticalScrollSlot(int value)
{
    m_view->set_visible_position(debug_view_xy(horizontalScrollBar()->value(), value));
}


void DebuggerView::horizontalScrollSlot(int value)
{
    m_view->set_visible_position(debug_view_xy(value, verticalScrollBar()->value()));
}


void DebuggerView::debuggerViewUpdate(debug_view& debugView, void* osdPrivate)
{
    // Get a handle to the DebuggerView being updated & redraw
    DebuggerView* dView = (DebuggerView*)osdPrivate;
    dView->verticalScrollBar()->setValue(dView->view()->visible_position().y);
    dView->viewport()->update();
    dView->update();
}


