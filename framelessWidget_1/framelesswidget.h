#pragma once

#include <QWidget>

class FramelessWidget : public QWidget
{
	Q_OBJECT
		enum Direction {
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
	enum {
		MARGIN_MIN_SIZE = 0,
		MARGIN_MAX_SIZE = 5
	};
public:
	FramelessWidget(QWidget* parent = nullptr);
	~FramelessWidget();
protected:
	bool event(QEvent* event) override;
	void leaveEvent(QEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void updateRegion(QMouseEvent* event);
	void resizeRegion(int marginTop, int marginBottom, int marginLeft, int marginRight);
	void createShadow();
	void maximizeWidget();
	void restoreWidget();

private:
	bool m_bIsPressed;
	bool m_bIsResizing;
	bool m_bIsDoublePressed;
	QPoint m_pressPoint;
	QPoint m_pressPoint_initial;
	QPoint m_movePoint;
	Direction m_direction;

	QRect rect;

};
