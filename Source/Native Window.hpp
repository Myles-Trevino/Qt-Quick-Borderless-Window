#pragma once

#include <Windows.h>
#include <QWidget>


namespace Native_Window
{
    void initialize(int width, int height, int device_pixel_ratio,
        const COLORREF& background_color, int x, int y);
    void destroy();

    void bind_qml_window(QWidget* qml_widget, HWND qml_window);
    int reposition_and_resize_test(HWND handle, UINT message,
        WPARAM word_parameter, LPARAM long_parameter);

    void set_resize_area_thickness(int size);
    void set_grab_area(int height, int left_margin, int right_margin);
    void set_enable_left_and_top_resizing(bool enable);
    void set_display_qml_window_while_resizing(bool display);
    void set_background_color(const COLORREF& color);
    void set_minimum_size(int width, int height);
    void set_geometry(int x, int y, int width, int height);

    HWND get_handle();
    bool is_maximized();
};
