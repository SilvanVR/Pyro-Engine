#ifndef WINDOW_WIN32_H_
#define WINDOW_WIN32_H_

#include "window.h"
#include "Input/input_manager.h"
#include <assert.h>
#include <string>

namespace ENGINE
{


#if VK_USE_PLATFORM_WIN32_KHR

    class InputManager;

    // Microsoft Windows specific versions of window functions
    LRESULT CALLBACK WindowsEventHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        Window * window = reinterpret_cast<Window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

        switch (uMsg) {
        case WM_CLOSE:
            window->close();
            return 0;
        case WM_PAINT:
            ValidateRect(hWnd, NULL);
            break;
        case WM_SIZE:
        case WM_EXITSIZEMOVE:
            window->onSizeChanged(LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_MOUSEWHEEL:
            InputManager::setWheelDelta(GET_WHEEL_DELTA_WPARAM(wParam));
            break;
        case WM_SETFOCUS:
            break;
        case WM_KEYDOWN: //case WM_SYSKEYDOWN:  
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
            InputManager::setKeyDown((uint32_t)wParam);
            break;
        case WM_KEYUP: //case WM_SYSKEYUP: 
            InputManager::setKeyUp((uint32_t)wParam);
            break;
        case WM_LBUTTONUP: //wParam is 0 for this event
            InputManager::setKeyUp(1);
            break;
        case WM_RBUTTONUP: //wParam is 0 for this event
            InputManager::setKeyUp(2);
            break;
        case WM_MBUTTONUP: //wParam is 0 for this event
            InputManager::setKeyUp(16);
            break;
        case WM_MOUSEMOVE:
            InputManager::setMousePos(LOWORD(lParam), HIWORD(lParam));
            break;
        default:
            break;
        }

        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }


    void Window::initOSWindow()
    {
        WNDCLASSEX win_class{};
        assert(width > 0 && height > 0);

        hinstance = GetModuleHandle(nullptr);

        // Initialize the window class structure:
        win_class.cbSize = sizeof(WNDCLASSEX);
        win_class.style = CS_HREDRAW | CS_VREDRAW;
        win_class.lpfnWndProc = WindowsEventHandler;
        win_class.cbClsExtra = 0;
        win_class.cbWndExtra = 0;
        win_class.hInstance = hinstance; // hInstance
        win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
        win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
        win_class.lpszMenuName = NULL;
        win_class.lpszClassName = DEF_APPLICATION_NAME;
        win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
        // Register window class:
        if (!RegisterClassEx(&win_class)) {
            assert(0 && "Error in registering a win32-Windows!\n");
            std::exit(-1);
        }

        DWORD exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        DWORD style = WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

        // Create window with the registered class:
        RECT wr = { 0, 0, LONG(width), LONG(height) };
        AdjustWindowRectEx(&wr, style, FALSE, exStyle);
        hwnd = CreateWindowEx(0,
            DEF_APPLICATION_NAME,           // class name
            DEF_APPLICATION_NAME,           // app name
            style,                          // window style
            CW_USEDEFAULT, CW_USEDEFAULT,   // x/y coords
            wr.right - wr.left,             // width
            wr.bottom - wr.top,             // height
            NULL,                           // handle to parent
            NULL,                           // handle to menu
            hinstance,                      // hInstance
            NULL);                          // no extra parameters
        if (!hwnd) {
            assert(0 && "Error in creating a win32-Windows: 'HWND'\n");
            std::exit(-1);
        }
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

        ShowWindow(hwnd, SW_SHOW);
        SetForegroundWindow(hwnd);
        SetFocus(hwnd);

        //Center Window
        int xPos = (GetSystemMetrics(SM_CXSCREEN) - wr.right) / 2;
        int yPos = (GetSystemMetrics(SM_CYSCREEN) - wr.bottom) / 2;
        SetWindowPos(hwnd, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }


    void Window::deInitOSWindow()
    {
        DestroyWindow(hwnd);
        UnregisterClass(DEF_APPLICATION_NAME, hinstance);
    }

    void Window::updateOSWindow()
    {
        MSG msg;
        if (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void Window::setWindowTextOS(const char* text)
    {
        SetWindowText(hwnd, text);
    }


    void Window::initOSSurface(const VkInstance& instance)
    {
        VkWin32SurfaceCreateInfoKHR surfaceInfo{};
        surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceInfo.pNext = NULL;
        surfaceInfo.flags = 0;
        surfaceInfo.hinstance = hinstance;
        surfaceInfo.hwnd = hwnd;

        vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &surface);
    }

#endif


}




#endif // WINDOW_WIN32_H_