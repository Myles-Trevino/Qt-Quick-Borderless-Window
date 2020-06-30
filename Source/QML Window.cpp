#include "QML Window.hpp"

#include <QApplication>
#include "Native Window.hpp"


// Initializes the QML window.
QML_Window::QML_Window(int width, int height,
	const COLORREF& background_color,
	int x, int y) : QWidget(nullptr)
{
	// Create the native window.
	Native_Window::initialize(width, height,
		window()->devicePixelRatio(),
		background_color, x, y);

	// Create the Qt window and embed it in the native window.
	setWindowFlags(Qt::FramelessWindowHint);
	setProperty("_q_embedded_native_parent_handle",
		reinterpret_cast<WId>(Native_Window::get_handle()));

	handle = reinterpret_cast<HWND>(winId());
	SetWindowLong(handle, GWL_STYLE, WS_CHILD);

	SetParent(handle, Native_Window::get_handle());
	QEvent event(QEvent::EmbeddingControl);
	QApplication::sendEvent(this, &event);

	Native_Window::bind_qml_window(this, handle);
}


// Destroys the window.
QML_Window::~QML_Window()
{
	if(layout) delete layout;
	if(qml_widget) delete qml_widget;
}


// Adds the QML widget to the window.
void QML_Window::add_widget(const QUrl& qml_source)
{
	// Clear the margins and create the layout.
	setContentsMargins(0, 0, 0, 0);
	layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	setLayout(layout);

	// Add the widget.
	qml_widget = new QQuickWidget(this);
	layout->addWidget(qml_widget);
	qml_widget->setParent(this, Qt::Widget);
	qml_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
	qml_widget->setSource(qml_source);

	// Update the widget size.
	SendMessage(Native_Window::get_handle(), WM_SIZE, 0, 0);
}


// Shows the window.
void QML_Window::show()
{
	ShowWindow(Native_Window::get_handle(), true);
	QWidget::show();
}


// Sets the position and size of the native window.
void QML_Window::setGeometry(int x, int y, int width, int height)
{
	Native_Window::set_geometry
	(
		x*window()->devicePixelRatio(),
		y*window()->devicePixelRatio(),
		width*window()->devicePixelRatio(),
		height*window()->devicePixelRatio()
	);
}


// Minimize the window.
void QML_Window::minimize()
{ SendMessage(Native_Window::get_handle(), WM_SYSCOMMAND, SC_MINIMIZE, 0); }


// Maximizes othe window.
void QML_Window::toggle_maximize()
{
	SendMessage(Native_Window::get_handle(), WM_SYSCOMMAND,
		Native_Window::is_maximized() ? SC_RESTORE : SC_MAXIMIZE, 0);
}


// Closes the application.
void QML_Window::close()
{
	ShowWindow(Native_Window::get_handle(), false);
	QApplication::quit();
}


// Handles the native window events.
bool QML_Window::nativeEvent(const QByteArray &, void *message, long *result)
{
	MSG *qt_message{reinterpret_cast<MSG *>(message)};

	// Check for resize or grab events.
	if(qt_message->message == WM_NCHITTEST)
	{
		if(!Native_Window::grab_and_resize_test(qt_message->hwnd,
			qt_message->message, qt_message->wParam, qt_message->lParam)) return false;
		*result = HTTRANSPARENT;
		return true;
	}

	return false;
}
