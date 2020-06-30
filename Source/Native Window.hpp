/*
	Copyright 2020 Myles Trevino
	Licensed under the Apache License, Version 2.0
	http://www.apache.org/licenses/LICENSE-2.0
*/


#pragma once

#include <string>
#define NOMINMAX
#include <Windows.h>
#include <QWidget>


namespace Native_Window
{
	void initialize(const std::wstring& title, int width, int height,
		int device_pixel_ratio, const COLORREF& background_color, int x, int y);
	void destroy();

	void bind_qml_window(QWidget* qml_widget, HWND qml_window);
	int grab_and_resize_test(HWND handle, UINT message,
		WPARAM word_parameter, LPARAM long_parameter);

	void set_geometry(int x, int y, int width, int height);
	void set_background_color(const COLORREF& color);
	void set_minimum_size(int width, int height);
	void set_resize_area_thickness(int size);
	void set_grab_area(int height, int left_margin, int right_margin);
	void set_enable_left_and_top_resizing(bool enable);
	void set_display_qml_while_resizing(bool display);

	HWND get_handle();
	bool is_maximized();
};
