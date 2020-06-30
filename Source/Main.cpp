/*
	Copyright 2020 Myles Trevino
	Licensed under the Apache License, Version 2.0
	http://www.apache.org/licenses/LICENSE-2.0
*/


#include <QApplication>
#include "Native Window.hpp"
#include "QML Window.hpp"


int main(int argument_count, char *arguments[])
{
	// Initialize the Qt application.
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication application(argument_count, arguments);

	// Create the window.
	QML_Window window{"Example", 1280, 720, RGB(238, 238, 238)};

	// Set the native window's properties.
	Native_Window::set_grab_area(30, 0, 75);

	// Register the window with QML.
	qmlRegisterSingletonInstance("Laventh.QML_Window", 1, 0, "QML_Window", &window);

	// Add the QML widget to the window and show the window.
	window.add_widget(QUrl(QStringLiteral("qrc:/Main.qml")));
	window.show();

	return application.exec();
}
