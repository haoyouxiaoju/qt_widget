#pragma once

#include <QWidget>
class TransparentBorder;


class FramelessWidget : public QWidget
{
	Q_OBJECT
public:
		enum Direction {//鼠标处于哪个边界
		BOTTOMRIGHT,
		TOPRIGHT,
		TOPLEFT,
		BOTTOMLEFT,
		RIGHT,
		DOWN,
		LEFT,
		UP,
		NONE
	};
	enum {//距离边界多少时改变鼠标样式
		MARGIN_MIN_SIZE = 0,
		MARGIN_MAX_SIZE = 4
	};
public:
	FramelessWidget(QWidget* parent = nullptr);
	~FramelessWidget();
	
	
	void setBorderColor(const QColor& color);


protected:
	bool event(QEvent* event) override;
	void leaveEvent(QEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	//修改鼠标样式，且是否处于边界
	void updateRegion(QMouseEvent* event);
	//修改大小和位置，即geometry
	void resizeRegion(int marginTop, int marginBottom, int marginLeft, int marginRight);
	void createShadow();
	void maximizeWidget();
	void restoreWidget();


    void paintEvent(QPaintEvent* event) override;

private:
	bool m_bIsPressed;		//是否鼠标按下
	bool m_bIsResizing;		//是否要拉伸
	bool m_bIsDoublePressed;//没用到
	QPoint m_pressPoint;	//鼠标按下时的坐标
	QPoint m_pressPoint_initial;//没用到
	QPoint m_movePoint;		//鼠标移动了的相对坐标
	Direction m_direction;	//鼠标的状态即在哪个边界


	QRect rect;				//用于存放geometry
	TransparentBorder* border;

};

class TransparentBorder :public  QWidget {
public:
	TransparentBorder();
	~TransparentBorder();


	void resizeBorder(const QPoint& movePoint,FramelessWidget::Direction direction);
	void moveBorder(const QPoint& movePoint);

	void setParentRect(const QRect& rect);
	void setBorderColor(const QColor& color);
protected:

	void paintEvent(QPaintEvent* event) override;
	
private:
	QPoint marginOrigin;
	QRect parentRect;
	QColor borderColor;
};
