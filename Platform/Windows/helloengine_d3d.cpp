//
// Created by creator on 19-3-18.
//

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <stdint.h>

#include <d3d11.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

const uint32_t SCREEN_WIDTH = 960;
const uint32_t SCREEN_HEIGHT = 480;

//global declarations
IDXGISwapChain          *g_pSwapchain = nullptr;              // the pointer to the swap chain interface
ID3D11Device * g_pDev = nullptr;              // the pointer to our Direct3D device interface
ID3D11DeviceContext * g_pDevcon = nullptr;              // the pointer to our Direct3D device context

ID3D11RenderTargetView * g_pRTView = nullptr;

ID3D11InputLayout * g_pLayout = nullptr;              // the pointer to the input layout
ID3D11VertexShader * g_pVS = nullptr;              // the pointer to the vertex shader
ID3D11PixelShader * g_pPS = nullptr;              // the pointer to the pixel shader

ID3D11Buffer * g_pVBuffer = nullptr;              // Vertex Buffer

												  //vertex buffer structure
struct VERTEX {
	XMFLOAT3 Position;	//xyz
	XMFLOAT4 Color;		//rgba
};

template<class T>
inline void SafeRelease(T **ppInterfaceToRelease)
{
	if (*ppInterfaceToRelease != nullptr)
	{
		(*ppInterfaceToRelease)->Release();
		(*ppInterfaceToRelease) = nullptr;
	}
}
//依然是画布，rendertarget就是渲染目标
void CreateRenderTarget()
{
	HRESULT hr;
	ID3D11Texture2D *pBackBuffer;

	//Get a pointer to the backbuffer
	g_pSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	//Create a render-target view
	g_pDev->CreateRenderTargetView(pBackBuffer, NULL, &g_pRTView);

	pBackBuffer->Release();

	//Bind(捆绑) the view
	g_pDevcon->OMSetRenderTargets(1, &g_pRTView, NULL);
}

//设置视口，目前是把一个画布都分配到一个视口。多人同屏游戏就可能需要分配到多个视口，还有VR的左右眼
void SetViewPort()
{
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;

	g_pDevcon->RSSetViewports(1, &viewport);
}

//this is the function that loads and prepares the shaders 
//初始化渲染管道(GPU的工作流水线):顶点变换，像素化，像素填色
//我们能编程的就是第一步和第三步
void InitPipeLine()
{
	//load and compile the two shaders
	ID3DBlob *VS, *PS;

	D3DReadFileToBlob(L"copy.vso", &VS);	//顶点变换 "copy.vso"是GPU程序
	D3DReadFileToBlob(L"copy.pso", &PS);	//像素填色

	//encapsulate(封装) both shaders into shader objects
	g_pDev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &g_pVS);
	g_pDev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &g_pPS);

	//set the shader objects
	g_pDevcon->VSSetShader(g_pVS, 0, 0);
	g_pDevcon->PSSetShader(g_pPS, 0, 0);

	//create the input layout object

	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	g_pDev->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &g_pLayout);
	g_pDevcon->IASetInputLayout(g_pLayout);

	VS->Release();
	PS->Release();
}

//this is the function that create the shape to render
//传入绘制模型的顶点信息，x轴向右，y轴向上，z轴向里
void InitGraphics()
{
	//create a triangle using the VERTEX struct
	VERTEX OurVertices[] =
	{
		{ XMFLOAT3(0.0f,0.5f,0.0f),XMFLOAT4(1.0f,0.0f,0.0f,1.0f) },
		{ XMFLOAT3(0.45f,-0.5f,0.0f),XMFLOAT4(0.0f,1.0f,0.0f,1.0f) },
		{ XMFLOAT3(-0.45f,-0.5f,0.0f),XMFLOAT4(0.0f,0.0f,1.0f,1.0f) }
	};

	//create the vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));

	bd.Usage = D3D11_USAGE_DYNAMIC;		//write access by CPU and GPU
	bd.ByteWidth = sizeof(VERTEX) * 3;	//size
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;	//use as a vertex buffer
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	//allow CPU to write in buffer

	g_pDev->CreateBuffer(&bd, NULL, &g_pVBuffer);	//create the buffer

	//copy the vertices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;
	g_pDevcon->Map(g_pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);	//map the buffer
	memcpy(ms.pData, OurVertices, sizeof(VERTEX) * 3);	//copy the data(from OurVertices to ms.Data)
	g_pDevcon->Unmap(g_pVBuffer, NULL);		//unmap the buffer
}

//this function perpare graphic resources for use 
HRESULT CreateGraphicsResources(HWND hWnd)
{
	//对于已经存在了g_pSwapchain则直接返回S_OK
	HRESULT	hr = S_OK;

	if (g_pSwapchain == nullptr)
	{
		//create a struct to hold information about the swap chain
		DXGI_SWAP_CHAIN_DESC scd;

		//clear out the struct for use
		ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

		//fill the swap chain
		scd.BufferCount = 1;	//one back buffer
		scd.BufferDesc.Width = SCREEN_WIDTH;
		scd.BufferDesc.Height = SCREEN_HEIGHT;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;		//use 32-bit color
		scd.BufferDesc.RefreshRate.Numerator = 60;
		scd.BufferDesc.RefreshRate.Denominator = 1;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		
		scd.OutputWindow = hWnd;
		scd.SampleDesc.Count = 4;
		scd.Windowed = TRUE;	//windowed/full-screen mode
		scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;		//allow full-screen switching

		const D3D_FEATURE_LEVEL FeatureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,		//逗号界定
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1		//最后一个没有逗号
		};		//分号结束

		D3D_FEATURE_LEVEL FeatureLevelSupported;

		HRESULT hr = S_OK;

		//create a device, device context and swap chain using scd
		hr = D3D11CreateDeviceAndSwapChain(NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL, 0,
			FeatureLevels,
			_countof(FeatureLevels),
			D3D11_SDK_VERSION,
			&scd,
			&g_pSwapchain,
			&g_pDev,
			&FeatureLevelSupported,
			&g_pDevcon);

		if (hr == E_INVALIDARG)
		{
			hr = D3D11CreateDeviceAndSwapChain(NULL,
				D3D_DRIVER_TYPE_HARDWARE,
				NULL, 0,
				&FeatureLevelSupported,
				1,
				D3D11_SDK_VERSION,
				&scd,
				&g_pSwapchain,
				&g_pDev,
				NULL,
				&g_pDevcon);
		}

		if (hr == S_OK)
		{
			CreateRenderTarget();
			SetViewPort();
			InitPipeLine();
			InitGraphics();
		}
	}
	return hr;
}

//释放资源
void DiscardGraphicResources()
{
    SafeRelease(&g_pLayout);
    SafeRelease(&g_pVS);
    SafeRelease(&g_pPS);
	SafeRelease(&g_pVBuffer);
	SafeRelease(&g_pSwapchain);
	SafeRelease(&g_pRTView);
	SafeRelease(&g_pDev);
	SafeRelease(&g_pDevcon);
}

//this is the function used to render a single frame
void RenderFrame()
{
	//clear the back buffer to a deep blue
	const FLOAT clearcolor[] = { 0.0f,0.2f,0.4f,1.0f };
	g_pDevcon->ClearRenderTargetView(g_pRTView, clearcolor);

	//do 3D rendering on the back buffer here
	{
		//select which vertex buffer to display
		UINT stride = sizeof(VERTEX);
		UINT offset = 0;
		g_pDevcon->IASetVertexBuffers(0, 1, &g_pVBuffer, &stride, &offset);

		//select hich primitive type we are using 
		g_pDevcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//dra the vertex buffer to the back buffer
		g_pDevcon->Draw(3, 0);
	}

	//swap the back buffer and the front buffer
	g_pSwapchain->Present(0, 0);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    //the handle for the window
    HWND hWnd;
    //this struct holds information for the window class
    WNDCLASSEX wc;

    //把COM有关东西删除时因为D3D11中应该帮助我们完成了有关工作

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
                          _T("Hello Engine![Direct 3D]"),
                          WS_OVERLAPPEDWINDOW,  //window style
                          100,  //x-position of the window
                          100,  //y-position of the window
                          SCREEN_WIDTH,  //width of the window
                          SCREEN_HEIGHT,  //height of the window
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
            wasHandled = true;
        break;

        case WM_PAINT:
			result = CreateGraphicsResources(hWnd);
			RenderFrame();
			wasHandled = true;     
        break;

        //size变化实际上会抛弃之前的绘图资源，重新创建新的画布画笔
        case WM_SIZE:
            if(g_pSwapchain != nullptr)
            {
				DiscardGraphicResources();
				g_pSwapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
            }
            wasHandled = true;
        break;

        case WM_DESTROY:
            DiscardGraphicResources();
            PostQuitMessage(0);
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

