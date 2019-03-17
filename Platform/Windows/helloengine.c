//
// Created by creator on 19-3-17.
//

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    //the handle for the window
    HWND hWnd;
    //this struct holds information for the window class
    WNDCLASSEX wc;

    //clear out the window class for use
    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    //fill in the struct with information
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    //_T是个宏定义在tchar.h,使得支持Unicode编码
    wc.lpszClassName = _T("WindowClass1");

    //register the window class
    RegisterClassEx(&wc);

    //create the window and use the result as the handle
    hWnd = CreateWindowEx(0, _T("WindowClass1"),
                          _T("Hello Engine!"),
                          WS_OVERLAPPEDWINDOW,  //window style
                          300,  //x-position of the window
                          300,  //y-position of the window
                          500,  //width of the window
                          400,  //height of the window
                          NULL,  //no parent window
                          NULL,  //no menu
                          hInstance,  //application handle
                          NULL);  //use with multiple windows

    //display the window
    ShowWindow(hWnd, nCmdShow);

    //enter main loop

    //this struct holds Windows event message
    MSG msg;

    //wait for the next message in the queue
    while(GetMessage(&msg, NULL, 0, 0))
    {
        //translate kerstroke message into the right format
        TranslateMessage(&msg);

        //send message to the WindowProc function
        DispatchMessage(&msg);
    }

    //return this part of the WM_QUIT message to windows
    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rec = {20,20,60,80};
            HBRUSH brush = (HBRUSH)GetStockObject(BLACK_BRUSH);

            FillRect(hdc, &rec, brush);

            EndPaint(hWnd, &ps);
        }break;
    case WM_DESTROY:
        {
            //close the application entirely
            PostQuitMessage(0);
            return 0;
        }break;
    }

    //handle any messages the switch statement didn't
    return DefWindowProc(hWnd, message, wParam, lParam);
}

