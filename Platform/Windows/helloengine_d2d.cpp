//
// Created by creator on 19-3-17.
//

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>

#include <d2d1.h>

//我们使用D2D,对象是游离在程序之外的COM组建创建的，所以需要通过工厂创建，形如外包
ID2D1Factory    *pFactory = nullptr;
ID2D1HwndRenderTarget   *pRenderTarget = nullptr;
ID2D1SolidColorBrush    *pLightSlateGrayBrush = nullptr;
ID2D1SolidColorBrush    *pCornflowerBlueBrush = nullptr;

template<class T>
inline void SafeRelease(T **ppInterfaceToRelease)
{
    if(*ppInterfaceToRelease != nullptr)
    {
        (*ppInterfaceToRelease)->Release();
        (*ppInterfaceToRelease) = nullptr;
    }
}

//创建所需画布和画笔
HRESULT CreateGraphicsResources(HWND hWnd)
{
    HRESULT hr = S_OK;
    if(pRenderTarget == nullptr)
    {
        RECT rc;
        GetClientRect(hWnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right-rc.left, rc.bottom-rc.top);

        hr = pFactory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(hWnd, size),
                &pRenderTarget);

        if(SUCCEEDED(hr))
        {
            hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightSlateGray),
                                                      &pLightSlateGrayBrush);
        }

        if(SUCCEEDED(hr))
        {
            hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::CornflowerBlue),
                                                      &pCornflowerBlueBrush);
        }
    }

    return hr;
}

//释放画布和画笔资源
void DiscardGraphicResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pLightSlateGrayBrush);
    SafeRelease(&pCornflowerBlueBrush);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    //the handle for the window
    HWND hWnd;
    //this struct holds information for the window class
    WNDCLASSEX wc;

    //initialize COM
    if(FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
        return -1;

    //clear out the window class for use
    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    //fill in the struct with information
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    //_T是个宏定义在tchar.h,使得支持Unicode编码
    wc.lpszClassName = _T("WindowClass1");

    //register the window class
    RegisterClassEx(&wc);

    //create the window and use the result as the handle
    hWnd = CreateWindowEx(0, _T("WindowClass1"),
                          _T("Hello Engine![Direct 2D]"),
                          WS_OVERLAPPEDWINDOW,  //window style
                          100,  //x-position of the window
                          100,  //y-position of the window
                          960,  //width of the window
                          540,  //height of the window
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
    while(GetMessage(&msg, nullptr, 0, 0))
    {
        //translate kerstroke message into the right format
        TranslateMessage(&msg);

        //send message to the WindowProc function
        DispatchMessage(&msg);
    }

    //uninitialize COM
    CoUninitialize();

    //return this part of the WM_QUIT message to windows
    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    bool wasHandled = false;

    switch(message)
    {
        case WM_CREATE:
            if(FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
            {
                result = -1;	//Fail CreateWindowEx
                return result;
            }
            wasHandled = true;
            result = 0;
        break;

        case WM_PAINT:
        {
            HRESULT hr = CreateGraphicsResources(hWnd);
            if(SUCCEEDED(hr))
            {
                PAINTSTRUCT ps;
                BeginPaint(hWnd, &ps);

                //所有指令都是通过pRenderTarget借口调用传给D2D COM组件
                //start build GPU draw command
                pRenderTarget->BeginDraw();

                //clear the background with white color
                pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));


                //retrieve the size of drawing area
                D2D1_SIZE_F rtSize = pRenderTarget->GetSize();

                //draw a grid background
                int width = static_cast<int>(rtSize.width);
                int height = static_cast<int>(rtSize.height);

                for(int x=0;x<width;x+=10)
                {
                    pRenderTarget->DrawLine(
                        D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                        D2D1::Point2F(static_cast<FLOAT>(x), rtSize.height),
                        pLightSlateGrayBrush,
                        0.5f
                    );
                }

                for(int y=0;y<height;y+=10)
                {
                    pRenderTarget->DrawLine(
                        D2D1::Point2F(0.0f, static_cast<FLOAT>(y)),
                        D2D1::Point2F(rtSize.width, static_cast<FLOAT>(y)),
                        pLightSlateGrayBrush,
                        0.5f
                    );
                }

                //draw two rectangles
                D2D1_RECT_F rectangle1 = D2D1::RectF(
                    rtSize.width/2-50.0f,
                    rtSize.height/2-50.0f,
                    rtSize.width/2+50.0f,
                    rtSize.height/2+50.0f
                );

                D2D1_RECT_F rectangle2 = D2D1::RectF(
                    rtSize.width/2-100.0f,
                    rtSize.height/2-100.0f,
                    rtSize.width/2+100.0f,
                    rtSize.height/2+100.0f
                );

                //draw a filled rectangle
                pRenderTarget->FillRectangle(&rectangle1, pLightSlateGrayBrush);

                //draw a outline only rectangle
                pRenderTarget->DrawRectangle(&rectangle2, pCornflowerBlueBrush);

                //end GPU draw command building
                hr = pRenderTarget->EndDraw();
                if(FAILED(hr)||hr == D2DERR_RECREATE_TARGET)
                {
                    DiscardGraphicResources();
                }

                EndPaint(hWnd, &ps);

            }
        }     
        wasHandled = true;     
        break;

        //size变化实际上会抛弃之前的绘图资源，重新创建新的画布画笔
        case WM_SIZE:
            if(pRenderTarget != nullptr)
            {
                RECT rc;
                GetClientRect(hWnd, &rc);

                D2D1_SIZE_U size = D2D1::SizeU(rc.right-rc.left, rc.bottom-rc.top);

                pRenderTarget->Resize(size);
            }
            wasHandled = true;
        break;

        case WM_DESTROY:
            DiscardGraphicResources();
            if(pFactory) {pFactory->Release(); pFactory = nullptr;}
            PostQuitMessage(0);
            result = 0;
            wasHandled = true;
        break;

        //分辨率变化，InvalidateRect是重新绘制
        case WM_DISPLAYCHANGE:
            InvalidateRect(hWnd, nullptr, false);
            wasHandled = true;
            break;
    }

    //对于没有定制化操作的使用系统自带的处理方式处理
    //handle any messages the switch statement didn't
    if(!wasHandled) {result = DefWindowProc(hWnd, message, wParam, lParam);}
    return result;
}

