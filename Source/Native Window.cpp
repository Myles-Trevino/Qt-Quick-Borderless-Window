#include "Native Window.hpp"

#include <Windowsx.h>
#include <dwmapi.h>


namespace
{
    HWND handle;
    QWidget *qml_widget;
    HWND qml_window;
    int device_pixel_ratio;

    int qml_window_x;
    int qml_window_y;
    int qml_window_width;
    int qml_window_height;

    bool resize_area_hovered;
    bool resizing;
    bool first_resize;
    bool moving;
    bool maximized;

    int minimum_width;
    int minimum_height;
    COLORREF background_color;
    int resize_area_thickness;
    int grab_area_height;
    int grab_area_left_margin;
    int grab_area_right_margin;
    bool enable_left_and_top_resizing{true};
    bool display_qml_window_while_resizing;


    // Handles the window's events.
    LRESULT CALLBACK event_callback(HWND handle, UINT message,
        WPARAM word_parameter, LPARAM long_parameter)
    {
        switch(message)
        {
            // Hide the frame.
            case WM_NCCALCSIZE: return NULL;

            // Disable the F10 menu.
            case WM_SYSCOMMAND: if(word_parameter == SC_KEYMENU) return NULL;

            // When resizing starts, hide the QML window if displaying on resize is disabled.
            case WM_ENTERSIZEMOVE:
                resizing = (resize_area_hovered && (GetKeyState(VK_LBUTTON) & 0x100));
                moving = !resizing;
                first_resize = true;
            break;

            // When resizing finishes, update the QML window size.
            case WM_EXITSIZEMOVE:
                if(resizing)
                {
                    qml_widget->setGeometry(qml_window_x, qml_window_y,
                        qml_window_width, qml_window_height);
                    qml_widget->activateWindow();
                }

                resizing = false;
                moving = false;
            break;

            // Send the close messages to the QML window.
            case WM_CLOSE:
                if(!qml_window) break;
                SendMessage(qml_window, WM_CLOSE, 0, 0);
                return NULL;

            case WM_DESTROY: PostQuitMessage(0); break;

            // Handle the resize and reposition messages from the QML window.
            case WM_NCHITTEST:
            {
                int result = Native_Window::grab_and_resize_test(
                    handle, message, word_parameter, long_parameter);
                resize_area_hovered = (result != HTCAPTION);
                return result;
            }

            // Handle resizing.
            case WM_SIZE:
            {
                if(!qml_widget) break;

                // Get the window placement.
                WINDOWPLACEMENT window_placement;
                window_placement.length = sizeof(WINDOWPLACEMENT);
                GetWindowPlacement(handle, &window_placement);

                // Get the new window position and size.
                RECT rectangle;
                GetClientRect(handle, &rectangle);

                // Calculate the new QML window position and size.
                maximized = (window_placement.showCmd == SW_MAXIMIZE);
                qml_window_x = maximized ? 4 : 0;
                qml_window_y = qml_window_x;
                qml_window_width = rectangle.right/qml_widget->window()->
                    devicePixelRatio()-(maximized ? 8 : 0);
                qml_window_height = rectangle.bottom/qml_widget->window()->
                    devicePixelRatio()-(maximized ? 8 : 0);

                // Hide the QML window if aero snapped (prevents flicker).
                if(moving || (!display_qml_window_while_resizing && first_resize))
                    qml_widget->setGeometry(0, 0, 0, 0);

                // Resize the QML window if appropriate.
                if(!resizing || display_qml_window_while_resizing) qml_widget->setGeometry(
                    qml_window_x, qml_window_y, qml_window_width, qml_window_height);
                return NULL;
            }

            // Provide the minimum size.
            case WM_GETMINMAXINFO:
            {
                if(!qml_widget) break;
                MINMAXINFO *minMaxInfo{reinterpret_cast<MINMAXINFO *>(long_parameter)};
                minMaxInfo->ptMinTrackSize.x = minimum_width;
                minMaxInfo->ptMinTrackSize.y = minimum_height;
                return NULL;
            }
        }

        // Execute the default procedure.
        return DefWindowProc(handle, message, word_parameter, long_parameter);
    }
}


// Initializes the window.
void Native_Window::initialize(int width, int height, int device_pixel_ratio,
    const COLORREF& background_color, int x, int y)
{
    ::device_pixel_ratio = device_pixel_ratio;
    ::background_color = background_color;
    width *= device_pixel_ratio;
    height *= device_pixel_ratio;

    minimum_width = 333*device_pixel_ratio;
    minimum_height = 333*device_pixel_ratio;
    resize_area_thickness = 6*device_pixel_ratio;
    grab_area_height = 40*device_pixel_ratio;

    // Create the window class.
    HINSTANCE module_handle{GetModuleHandle(nullptr)};
    WNDCLASSEX wcx{0};

    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_HREDRAW|CS_VREDRAW;
    wcx.hInstance = module_handle;
    wcx.lpfnWndProc = event_callback;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = 0;
    wcx.lpszClassName = L"WindowClass";
    wcx.hbrBackground = CreateSolidBrush(background_color);
    wcx.hCursor = LoadCursor(module_handle, IDC_ARROW);

    if(FAILED(RegisterClassEx(&wcx)))
        throw std::runtime_error("Failed to register the window class.");

    // Create the window.
    if(x < 0) x = GetSystemMetrics(SM_CXSCREEN)/2-width/2;
    if(y < 0) y = GetSystemMetrics(SM_CYSCREEN)/2-height/2;

    DWORD flags{WS_POPUP|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|
        WS_MAXIMIZEBOX|WS_THICKFRAME|WS_CLIPCHILDREN};

    handle = CreateWindow(L"WindowClass", L"WindowTitle",
        flags, x, y, width, height, 0, 0, module_handle, nullptr);

    if(!handle) throw std::runtime_error("Failed to create the window.");

    SetWindowPos(handle, 0, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE);
}


// Destroys the window.
void Native_Window::destroy()
{
    ShowWindow(handle, SW_HIDE);
    DestroyWindow(handle);
}

// Binds the QML window.
void Native_Window::bind_qml_window(QWidget *qml_widget, HWND qml_window)
{
    ::qml_widget = qml_widget;
    ::qml_window = qml_window;
}


// Tests if the mouse is on a grab or resize area.
int Native_Window::grab_and_resize_test(HWND handle,
    UINT message, WPARAM word_parameter, LPARAM long_parameter)
{
    // Get the relative mouse and window position.
    RECT rectangle;
    GetWindowRect(handle, &rectangle);
    int x{GET_X_LPARAM(long_parameter)-rectangle.left},
        y{GET_Y_LPARAM(long_parameter)-rectangle.top},
        x2{rectangle.right-rectangle.left},
        y2{rectangle.bottom-rectangle.top};

    // Grab area.
    if(x >= (enable_left_and_top_resizing ? resize_area_thickness : 0)+grab_area_left_margin &&
        x <= x2-resize_area_thickness-grab_area_right_margin &&
        y >= (enable_left_and_top_resizing ? resize_area_thickness : 0) &&
        y <= grab_area_height) return HTCAPTION;

    // Bottom right resize corner.
    if(x > x2-resize_area_thickness && y > y2-resize_area_thickness) return HTBOTTOMRIGHT;

    // If resizing from the top and left is enabled...
    if(enable_left_and_top_resizing)
    {
        // Top left resize corner.
        if(x < resize_area_thickness && y < resize_area_thickness) return HTTOPLEFT;

        // Top right resize corner.
        if(x > x2-resize_area_thickness && y < resize_area_thickness) return HTTOPRIGHT;

        // Bottom left resize corner.
        if(x < resize_area_thickness && y > y2-resize_area_thickness) return HTBOTTOMLEFT;
    }

    // Bottom resize border.
    if(y > y2-resize_area_thickness) return HTBOTTOM;

    // Right resize border.
    if(x > x2-resize_area_thickness) return HTRIGHT;

    // If resizing from the top and left is enabled...
    if(enable_left_and_top_resizing)
    {
        // Top resize border.
        if(y < resize_area_thickness) return HTTOP;

        // Left resize border.
        if(x < resize_area_thickness) return HTLEFT;
    }

    // The mouse is not over any grab or resize area.
    return NULL;
}


// Setters.
void Native_Window::set_resize_area_thickness(int size)
{ resize_area_thickness = size*device_pixel_ratio; }


void Native_Window::set_grab_area(int height, int left_margin, int right_margin)
{
    grab_area_height = height*device_pixel_ratio;
    grab_area_left_margin = left_margin*device_pixel_ratio;
    grab_area_right_margin = right_margin*device_pixel_ratio;
}

void Native_Window::set_enable_left_and_top_resizing(bool enable)
{ enable_left_and_top_resizing = enable; }


void Native_Window::set_display_qml_while_resizing(bool display)
{ display_qml_window_while_resizing = display; }


void Native_Window::set_background_color(const COLORREF& color)
{ background_color = color; }


void Native_Window::set_minimum_size(int width, int height)
{
    minimum_width = width;
    minimum_height = height;
}


void Native_Window::set_geometry(int x, int y, int width, int height)
{ MoveWindow(handle, x, y, width, height, true); }


// Getters.
HWND Native_Window::get_handle(){ return handle; }


bool Native_Window::is_maximized(){ return maximized; }