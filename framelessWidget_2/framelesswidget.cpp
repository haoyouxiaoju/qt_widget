#include "framelesswidget.h"
#include <QEvent>
#include <QMouseEvent>
#include <QRect>
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QtMath>
#include <QPen>
#include <QPainter>
#include <QPainterPath>
#include "model/data.h"

FramelessWidget::FramelessWidget(QWidget* parent)
	: QWidget(parent), m_bIsPressed(false), m_bIsResizing(false), m_bIsDoublePressed(false),
	m_direction(NONE)
{
	setWindowFlags(Qt::FramelessWindowHint);    //隐藏标题栏（无边框）
	setAttribute(Qt::WA_StyledBackground);      //启用样式背景绘制
	//setAttribute(Qt::WA_TranslucentBackground); //背景透明
	setAttribute(Qt::WA_Hover);
	setAttribute(Qt::WA_StaticContents);
	this->setMinimumSize(50, 50);
	
	border = new TransparentBorder();//并没有让border挂在this下面,所以得析构时得delete
	border->hide();

}

FramelessWidget::~FramelessWidget()
{
	delete border;
}

bool FramelessWidget::event(QEvent* event)
{
	///
	// 使得移除窗口仍能进行鼠标移动的事件
	///
	if (event->type() == QEvent::HoverMove) {
		QHoverEvent* hoverEvent = static_cast<QHoverEvent*>(event);
		QMouseEvent mouseEvent(QEvent::MouseMove, hoverEvent->pos(),
			Qt::NoButton, Qt::NoButton, Qt::NoModifier);
		mouseMoveEvent(&mouseEvent);
		//LOG() << "hover move";
	}

	return QWidget::event(event);
}

void FramelessWidget::mousePressEvent(QMouseEvent* event)
{
	QWidget::mousePressEvent(event);
	if (event->button() == Qt::LeftButton) {
		m_bIsPressed = true;
		m_pressPoint = event->globalPos();//鼠标按下的绝对坐标
		m_movePoint = QPoint(0, 0);//使得上次移动的相对坐标清零

	}
	//*
	//如果m_direction不为NoNE 即 鼠标在窗口边界 那么就是要进行窗口拉伸
	//*
	if (m_direction != NONE) {
		m_bIsResizing = true;
	}
	//由于使用的是 额外创建一个boder边框使得能够预览窗口的位置
	// 所以得让boder知道要绑定谁，且知道他的geometry
	border->setParentRect(geometry());
	border->show();//显示边框
}

void FramelessWidget::mouseMoveEvent(QMouseEvent* event)
{

	QWidget::mouseMoveEvent(event);
	m_movePoint = event->globalPos() - m_pressPoint;
	//LOG() <<"m_bIsResizing"<< m_bIsResizing;
	//LOG() <<"m_bIsPressed"<< m_bIsPressed;
	
	//*
	//	鼠标没按下 且 不处于拉伸状态才来判断是不是在边界
	//*
	if (windowState() != Qt::WindowMaximized && !m_bIsPressed && !m_bIsResizing) {
		updateRegion(event);
	}

//	LOG() << "width" << minimumWidth();
//	LOG() << "height" << minimumHeight();

	//*
	//	鼠标按下 但不处于拉伸状态
	//*
	if (m_bIsPressed && !m_bIsResizing) {
		border->moveBorder(m_movePoint);
	}
	//拉伸状态
	else if (m_bIsResizing) {
		border->resizeBorder(m_movePoint, m_direction);
	}



}

// 用于识别是否是拉伸动作
void FramelessWidget::updateRegion(QMouseEvent* event)
{
	QRect mainRect = geometry();

	int marginTop = event->globalY() - mainRect.y();
	int marginBottom = mainRect.y() + mainRect.height() - event->globalY();
	int marginLeft = event->globalX() - mainRect.x();
	int marginRight = mainRect.x() + mainRect.width() - event->globalX();

	//LOG() << marginTop << "|" << marginBottom << "|" << marginLeft << "|" << marginRight;

	if (!m_bIsResizing && !m_bIsPressed) {
		if ((marginRight >= MARGIN_MIN_SIZE && marginRight <= MARGIN_MAX_SIZE)
			&& ((marginBottom <= MARGIN_MAX_SIZE) && marginBottom >= MARGIN_MIN_SIZE)) {
			m_direction = BOTTOMRIGHT;
			setCursor(Qt::SizeFDiagCursor);
		}
		else if ((marginTop >= MARGIN_MIN_SIZE && marginTop <= MARGIN_MAX_SIZE)
			&& (marginRight >= MARGIN_MIN_SIZE && marginRight <= MARGIN_MAX_SIZE)) {
			m_direction = TOPRIGHT;
			setCursor(Qt::SizeBDiagCursor);
		}
		else if ((marginLeft >= MARGIN_MIN_SIZE && marginLeft <= MARGIN_MAX_SIZE)
			&& (marginTop >= MARGIN_MIN_SIZE && marginTop <= MARGIN_MAX_SIZE)) {
			m_direction = TOPLEFT;
			setCursor(Qt::SizeFDiagCursor);
		}
		else if ((marginLeft >= MARGIN_MIN_SIZE && marginLeft <= MARGIN_MAX_SIZE)
			&& (marginBottom >= MARGIN_MIN_SIZE && marginBottom <= MARGIN_MAX_SIZE)) {
			m_direction = BOTTOMLEFT;
			setCursor(Qt::SizeBDiagCursor);
		}
		else if (marginBottom <= MARGIN_MAX_SIZE && marginBottom >= MARGIN_MIN_SIZE) {
			m_direction = DOWN;
			setCursor(Qt::SizeVerCursor);
		}
		else if (marginLeft <= MARGIN_MAX_SIZE - 1 && marginLeft >= MARGIN_MIN_SIZE - 1) {
			m_direction = LEFT;
			setCursor(Qt::SizeHorCursor);
		}
		else if (marginRight <= MARGIN_MAX_SIZE && marginRight >= MARGIN_MIN_SIZE) {
			m_direction = RIGHT;
			setCursor(Qt::SizeHorCursor);
		}
		else if (marginTop <= MARGIN_MAX_SIZE && marginTop >= MARGIN_MIN_SIZE) {
			m_direction = UP;
			setCursor(Qt::SizeVerCursor);
		}
		else {
			m_direction = NONE;
			setCursor(Qt::ArrowCursor);

		}
	}
	//LOG() << m_direction;

}

//对窗口进行大小和位置进行设置
void FramelessWidget::resizeRegion(int marginTop, int marginBottom,
	int marginLeft, int marginRight)
{
	if (m_bIsPressed && m_bIsResizing) {
		//LOG() << "resize" << m_direction;
		switch (m_direction) {
		case BOTTOMRIGHT:
		{
			rect = geometry();
			rect.setBottomRight(rect.bottomRight() + m_movePoint);
			this->setGeometry(rect);
		}
		break;
		case TOPRIGHT:
		{
			rect = geometry();
			// 设置的宽度 小于 最小宽度 高度 小于 最小高度
			if (geometry().width() + m_movePoint.x() <= minimumWidth() && geometry().height() - m_movePoint.y() <= minimumHeight()) {
				rect.setRect(rect.x() ,
					rect.y() + rect.height() - minimumHeight(),
					minimumWidth(),
					minimumHeight());
				//LOG() << "1";
			}
			// 设置的宽度 小于 最小宽度 高度 大于 最小高度
			else if (geometry().width() + m_movePoint.x() <= minimumWidth() && geometry().height() - m_movePoint.y() > minimumHeight()) {
				rect.setRect(rect.x() ,
					rect.y() + m_movePoint.y(),
					minimumWidth(),
					rect.height() - m_movePoint.y());
				//LOG() << "2";
			}
			// 设置的宽度 大于 最小宽度 高度 小于 最小高度
			else if (geometry().height() - m_movePoint.y() <= minimumHeight() && geometry().width() + m_movePoint.x() > minimumWidth()) {
				rect.setRect(rect.x() ,
					rect.y() + rect.height() - minimumHeight(),
					rect.width() + m_movePoint.x(),
					minimumHeight());
				//LOG() << "3"<<rect;

			}
			// 设置的宽度 大于 最小宽度 高度 大于 最小高度
			else {
				rect.setTopRight(rect.topRight() + m_movePoint);
				//LOG() << "4";

			}


			this->setGeometry(rect);
		}
		break;
		case TOPLEFT:
		{
			rect = geometry();
			// 设置的宽度 小于 最小宽度 高度 小于 最小高度
			if (geometry().width() - m_movePoint.x() <= minimumWidth() && geometry().height() - m_movePoint.y() <= minimumHeight()) {
				rect.setRect(rect.x() +rect.width() -minimumWidth(),
					rect.y() + rect.height() -minimumHeight(),
					minimumWidth(),
					minimumHeight());
				//LOG() << "1";
			}
			// 设置的宽度 小于 最小宽度 高度 大于 最小高度
			else if (geometry().width() - m_movePoint.x() <= minimumWidth() && geometry().height() - m_movePoint.y() > minimumHeight()) {
				rect.setRect(rect.x() +rect.width() -minimumWidth() ,
					rect.y() + m_movePoint.y(),
					minimumWidth(),
					rect.height() - m_movePoint.y());
				//LOG() << "2";
			}
			// 设置的宽度 大于 最小宽度 高度 小于 最小高度
			else if (geometry().height() - m_movePoint.y() <= minimumHeight() && geometry().width() - m_movePoint.x() > minimumWidth()) {
				rect.setRect(rect.x() + m_movePoint.x(),
					rect.y() + rect.height() - minimumHeight(),
					rect.width() - m_movePoint.x(),
					minimumHeight());
				//LOG() << "3"<<rect;

			}
			// 设置的宽度 大于 最小宽度 高度 大于 最小高度
			else {
				rect.setTopLeft(rect.topLeft() + m_movePoint);
				//LOG() << "4";

			}


			this->setGeometry(rect);

		}
		break;
		case BOTTOMLEFT:
		{
			rect = geometry();
			// 设置的宽度 小于 最小宽度 高度 小于 最小高度
			if (geometry().width() - m_movePoint.x() <= minimumWidth() && geometry().height() + m_movePoint.y() <= minimumHeight()) {
				rect.setRect(rect.x() +rect.width() -minimumWidth(),
					rect.y(),
					minimumWidth(),
					minimumHeight());
				//LOG() << "1";
			}
			// 设置的宽度 小于 最小宽度 高度 大于 最小高度
			else if (geometry().width() - m_movePoint.x() <= minimumWidth() && geometry().height() + m_movePoint.y() > minimumHeight()) {
				rect.setRect(rect.x() +rect.width() -minimumWidth() ,
					rect.y(),
					minimumWidth(),
					rect.height() + m_movePoint.y());
				//LOG() << "2";
			}
			// 设置的宽度 大于 最小宽度 高度 小于 最小高度
			else if (geometry().height() + m_movePoint.y() <= minimumHeight() && geometry().width() - m_movePoint.x() > minimumWidth()) {
				rect.setRect(rect.x() + m_movePoint.x(),
					rect.y(),
					rect.width() - m_movePoint.x(),
					minimumHeight());
				//LOG() << "3"<<rect;

			}
			// 设置的宽度 大于 最小宽度 高度 大于 最小高度
			else {
				rect.setBottomLeft(rect.bottomLeft() + m_movePoint);
				//LOG() << "4";

			}


			this->setGeometry(rect);

		}
		break;
		case RIGHT:
		{
			rect = geometry();
			rect.setRight(rect.right() + m_movePoint.x());
			this->setGeometry(rect);
			//setFixedSize(rect.width(), rect.height());

		}
		break;
		case DOWN:
		{
			rect = geometry();
			rect.setBottom(rect.bottom() + m_movePoint.y());
			//rect.setHeight(rect.height() + m_movePoint.y());
			this->setGeometry(rect);
			//setFixedSize(rect.width(), rect.height());
			//LOG() << "down";
		}
		break;
		case LEFT:
		{
			if (geometry().width() - m_movePoint.x() < minimumWidth()) {
				rect = geometry();
				rect.setRect(rect.x() + rect.width() - minimumWidth(),
					rect.y() ,
					minimumWidth(),
					rect.height());
				this->setGeometry(rect);
			}
			else {

				rect = geometry();
				rect.setLeft(rect.left() + m_movePoint.x());
				//rect.setX(rect.width() - m_movePoint.x());
				this->setGeometry(rect);

				//setFixedSize(rect.width(), rect.height());
				//this->move(rect.x() + m_movePoint.x(), rect.y());
			}
		}
		break;
		case UP:
		{
			if (geometry().height() - m_movePoint.y() < minimumHeight()) {
				rect = geometry();
				rect.setRect(rect.x(),
					rect.y() + rect.height() - minimumHeight(),
					rect.width(),
					minimumHeight());
				this->setGeometry(rect);
			}
			else {
				rect = geometry();
				rect.setTop(rect.top() + m_movePoint.y());
				this->setGeometry(rect);
				//LOG() << "UP";
			}
			

		}
		break;
		default:
		{
		}
		break;
		}
	}
	else {
		m_bIsResizing = false;
		//当不在边界一定得设置NONE，不然会导致在边界后，下次不在边界会被判断成拉伸状态
		m_direction = NONE;
	}
}

void FramelessWidget::mouseReleaseEvent(QMouseEvent* event)
{
	///
	//	鼠标松开 需要判断 是否处于拉伸状态需要修改窗口
	//			是否是窗口需要移动
	///
    QWidget::mouseReleaseEvent(event);
	LOG() << m_direction;
	if (NONE != m_direction) {
		//LOG() << "resize";
		resizeRegion(0, 0, 0, 0);
	}
	// 鼠标松开，当鼠标按下的状态还没修改
	//	处于移动窗口的状态
	if (!m_bIsResizing && m_bIsPressed) {
		this->move(geometry().x() + m_movePoint.x(), geometry().y() + m_movePoint.y());
	}
	
	//LOG() << "1:" << geometry();

	// 修改鼠标的样式
	if (windowState() != Qt::WindowMaximized) {
		updateRegion(event);
	}

	//重置值，防止影响下次判断
    if (event->button() == Qt::LeftButton) {
        m_bIsPressed = false;
		m_bIsResizing = false;
		m_bIsDoublePressed = false;
		m_direction = NONE;
	}
	//完成 操作 取消显示边框
	border->hide();

	//LOG() << "move_point" << m_movePoint;
	//LOG()<<"2:" << geometry();
}

void FramelessWidget::leaveEvent(QEvent *event)
{
   // m_bIsPressed = false;
   // m_bIsDoublePressed = false;
   // m_bIsResizing = false;

    QWidget::leaveEvent(event);
}

void FramelessWidget::createShadow()
{
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setColor(Qt::black);
    shadowEffect->setOffset(0, 0);
    shadowEffect->setBlurRadius(13);
    this->setGraphicsEffect(shadowEffect);
}

void FramelessWidget::maximizeWidget()
{
  
    showMaximized();
}
void FramelessWidget::restoreWidget()
{
   
    showNormal();
}

void FramelessWidget::setBorderColor(const QColor& color)
{
	this->border->setBorderColor(color);
}


void FramelessWidget::paintEvent(QPaintEvent* event)
{

    QWidget::paintEvent(event);


}


TransparentBorder::TransparentBorder():
	QWidget(),marginOrigin(0,0),parentRect(0,0,0,0),borderColor(Qt::white)
{
	setWindowOpacity(1);
	this->setAttribute(Qt::WA_TranslucentBackground, true);//透明
	this->setWindowFlags(Qt::FramelessWindowHint);//无边框

}

TransparentBorder::~TransparentBorder()
{
}

//边框的大小设置
void TransparentBorder::resizeBorder(const QPoint& m_movePoint, FramelessWidget::Direction direction)
{
	switch (direction) {
	case FramelessWidget::Direction::BOTTOMRIGHT:
		{
		LOG() << "BottomRight";
			QRect rect(parentRect);
			rect.setBottomRight(rect.bottomRight() + m_movePoint);
			this->setGeometry(rect);
		}
		break;
		case FramelessWidget::Direction::TOPRIGHT:
		{
			QRect rect(parentRect);
			rect.setTopRight(rect.topRight() + m_movePoint);
			this->setGeometry(rect);
		}
		break;
		case FramelessWidget::Direction::TOPLEFT:
		{
			QRect rect(parentRect);
			rect.setTopLeft(rect.topLeft() + m_movePoint);
			this->setGeometry(rect);
		}
		break;
		case FramelessWidget::Direction::BOTTOMLEFT:
		{
			QRect rect(parentRect);
			rect.setBottomLeft(rect.bottomLeft() + m_movePoint);
			this->setGeometry(rect);
		}
		break;
		case FramelessWidget::Direction::RIGHT:
		{
			QRect rect(parentRect);
			rect.setRight(rect.right() + m_movePoint.x());
			setGeometry(rect);

		}
		break;
		case FramelessWidget::Direction::DOWN:
		{
			//LOG() << "down";
			//LOG() << "parentRect:" << parentRect;
			QRect rect(parentRect);
			rect.setBottom(rect.bottom() + m_movePoint.y());
			//rect.setHeight(rect.height() + m_movePoint.y());
			setGeometry(rect);

		}
		break;
		case FramelessWidget::Direction::LEFT:
		{
			QRect rect(parentRect);
			rect.setLeft(rect.left() + m_movePoint.x());
			setGeometry(rect);

		}
		break;
		case FramelessWidget::Direction::UP:
		{
			QRect rect(parentRect);
			//rect.setHeight(rect.height() - m_movePoint.y());
			rect.setTop(rect.top() + m_movePoint.y());
			setGeometry(rect);
			//setFixedSize(rect.width(), rect.height());
			//LOG() << "UP";
			
		}
		break;
		default:
		{
		}
		break;
		}

}

//移动边框
void TransparentBorder::moveBorder(const QPoint& movePoint)
{
	this->move(parentRect.x()+movePoint.x(), parentRect.y()+movePoint.y());
}


//设置要绑定哪个窗口
void TransparentBorder::setParentRect(const QRect& rect)
{
	parentRect.setRect(rect.x(),rect.y(),rect.width(),rect.height());
	this->setGeometry(parentRect);

}

void TransparentBorder::setBorderColor(const QColor& color)
{
	borderColor = color;
}

// 渲染时画出出边框
void TransparentBorder::paintEvent(QPaintEvent *event)
{

	QRect rect = geometry();
	QPainter painter(this);
    painter.setBrush(QColor(9,151,247,1));//painter区域全部的背景色
    painter.setPen(QPen(borderColor,3,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    painter.setCompositionMode(QPainter::CompositionMode_Difference);
    painter.drawRect(0,0,rect.width(),rect.height());

	//this->resize(parentRect.width(),parentRect.height());
}

