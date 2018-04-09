// CGS HW Project A "Line Land".
// Author: L.Norri CD CGS, FullSail University

// Introduction:
// Welcome to the hardware project of the Computer Graphics Systems class.
// This assignment is fully guided but still requires significant effort on your part. 
// Future classes will demand the habits & foundation you develop right now!  
// It is CRITICAL that you follow each and every step. ESPECIALLY THE READING!!!

// TO BEGIN: Open the word document that acompanies this project and start from the top.

//************************************************************
//************ INCLUDES & DEFINES ****************************
//************************************************************

#include <iostream>
#include <ctime>
#include "XTime.h"

using namespace std;

// BEGIN PART 1
// TODO: PART 1 STEP 1a
#include <d3d11.h>

// TODO: PART 1 STEP 1b
#include <DirectXMath.h>
using namespace DirectX;
// TODO: PART 2 STEP 6
#include "Trivial_VS.csh"
#include "Trivial_PS.csh"

#define BACKBUFFER_WIDTH	500
#define BACKBUFFER_HEIGHT	500

//************************************************************
//************ SIMPLE WINDOWS APP CLASS **********************
//************************************************************

class DEMO_APP
{
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;

	



	//*****************************************//
	//************ GRAPHICS 2 CODE ************//
	//*****************************************//

	ID3D11Device				*device;
	IDXGISwapChain				*swap;
	ID3D11RenderTargetView		*rtv;
	ID3D11DeviceContext			*context;
	D3D11_VIEWPORT				viewport;
	ID3D11InputLayout			*inputLayout;
	ID3D11VertexShader			*vs;
	ID3D11PixelShader			*ps;

	ID3D11Buffer				*vertexBuffer;
	ID3D11Buffer				*indexBuffer;
	ID3D11Buffer				*constantBuffer;

	XMMATRIX					worldM;
	XMMATRIX					viewM;
	XMMATRIX					projM;

	XTime timer;

	struct MATRIX_DATA
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
	};

public:
	struct SIMPLE_VERTEX
	{
		XMFLOAT3 xyz;
		XMFLOAT4 color;
	};

	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
};

//************************************************************
//************ CREATION OF OBJECTS & RESOURCES ***************
//************************************************************

DEMO_APP::DEMO_APP(HINSTANCE hinst, WNDPROC proc)
{
	// ****************** BEGIN WARNING ***********************// 
	// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY! 
	application = hinst;
	appWndProc = proc;

	WNDCLASSEX  wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = appWndProc;
	wndClass.lpszClassName = L"DirectXApplication";
	wndClass.hInstance = application;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
	//wndClass.hIcon			= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FSICON));
	RegisterClassEx(&wndClass);

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(L"DirectXApplication", L"CGS Hardware Project", WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
		CW_USEDEFAULT, CW_USEDEFAULT, window_size.right - window_size.left, window_size.bottom - window_size.top,
		NULL, NULL, application, this);

	ShowWindow(window, SW_SHOW);
	//********************* END WARNING ************************//

	// swapchain desc
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 1;
	sd.BufferDesc.Width = 500;
	sd.BufferDesc.Height = 500;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.Windowed = true;
	sd.OutputWindow = window;
	sd.SampleDesc.Count = 1;

	// TODO: PART 1 STEP 3b
	D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_DEBUG, //0 if not in debug mode
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&sd,
		&swap,
		&device,
		NULL,
		&context
	);
	// CREATE BACK BUFFER
	ID3D11Texture2D *backBuffer;
	swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);

	device->CreateRenderTargetView(backBuffer, NULL, &rtv);
	backBuffer->Release();
	// SET VIEWPORT
	viewport.Width = 500;
	viewport.Height = 500;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;


	// DEFINE SHADERS
	device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &vs);
	device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &ps);

	// CREATE INPUT LAYOUT
	D3D11_INPUT_ELEMENT_DESC vLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	device->CreateInputLayout(vLayout, 2, Trivial_VS, sizeof(Trivial_VS), &inputLayout);

	// DEFINE CUBE DATA

	SIMPLE_VERTEX cube[] =
	{
		{ XMFLOAT3(-0.5f, 0.5f,-0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(0.5f, 0.5f,-0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.5f,0.5f,0.5f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-0.5f,0.5f,0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },

		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.5f,-0.5f, 0.5f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-0.5f,-0.5f, 0.5f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
	};
	short cubeInd[] =
	{
		3,1,0, 2,1,3,
		0,5,4, 1,5,0,
		3,4,7, 0,4,3,
		1,6,5, 2,6,1,
		2,7,6, 3,7,2,
		6,4,5, 7,4,6,

	};
	// set vertex buffer
	D3D11_BUFFER_DESC cubeBD;
	ZeroMemory(&cubeBD, sizeof(cubeBD));
	cubeBD.Usage = D3D11_USAGE_IMMUTABLE;
	cubeBD.ByteWidth = sizeof(SIMPLE_VERTEX) * 8;
	cubeBD.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	cubeBD.CPUAccessFlags = NULL;

	D3D11_SUBRESOURCE_DATA cubeBufferData;
	ZeroMemory(&cubeBufferData, sizeof(cubeBufferData));
	cubeBufferData.pSysMem = cube;
	device->CreateBuffer(&cubeBD, &cubeBufferData, &vertexBuffer);

	// create index buffer
	cubeBD.Usage = D3D11_USAGE_IMMUTABLE;
	cubeBD.BindFlags = D3D11_BIND_INDEX_BUFFER;
	cubeBD.CPUAccessFlags = NULL;
	cubeBD.ByteWidth = sizeof(short) * 36;

	cubeBufferData.pSysMem = cubeInd;

	device->CreateBuffer(&cubeBD, &cubeBufferData, &indexBuffer);
	//context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// create constant buffer
	cubeBD.Usage = D3D11_USAGE_DYNAMIC;
	cubeBD.ByteWidth = sizeof(MATRIX_DATA);
	cubeBD.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cubeBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	device->CreateBuffer(&cubeBD, NULL, &constantBuffer);

	// create matrices
	worldM = XMMatrixIdentity();

	// view matrix & proj, REPLACE THIS!!!
	XMVECTOR Eye = XMVectorSet(-0.5f, 1.0f, -2.5f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	viewM = XMMatrixLookAtLH(Eye, At, Up);

	projM = XMMatrixPerspectiveFovLH(XM_PIDIV2, BACKBUFFER_WIDTH / (FLOAT)BACKBUFFER_HEIGHT, 0.01f, 100.0f);

}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	timer.Signal();

	context->OMSetRenderTargets(1, &rtv, NULL);
	context->RSSetViewports(1, &viewport);

	// CLEAR RTV TO BLUE
	FLOAT color[4] = { 0.0, 0.0f, 0.4f, 1.0f };
	context->ClearRenderTargetView(rtv, color);

	// SET INPUT LAYOUT
	context->IASetInputLayout(inputLayout);

	// SET CB AND SHADERS
	context->VSSetConstantBuffers(0, 1, &constantBuffer);
	context->VSSetShader(vs, 0, 0);
	context->PSSetShader(ps, 0, 0);


	worldM = XMMatrixRotationY(timer.TotalTime());

	MATRIX_DATA cbData;

	cbData.world = worldM;
	cbData.view = viewM;
	cbData.proj = projM;

	D3D11_MAPPED_SUBRESOURCE cubeSub;
	context->Map(constantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &cubeSub);
	memcpy(cubeSub.pData, &cbData, sizeof(cbData));
	context->Unmap(constantBuffer, NULL);

	UINT strides = sizeof(SIMPLE_VERTEX);
	UINT offsets = 0;
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offsets);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	context->DrawIndexed(36, 0, 0);

	// TODO: PART 1 STEP 8
	swap->Present(0, 0);
	// END OF PART 1
	return true;
}

//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool DEMO_APP::ShutDown()
{
	// TODO: PART 1 STEP 6

	swap->Release();
	device->Release();
	rtv->Release();
	context->Release();
	UnregisterClass(L"DirectXApplication", application);
	return true;
}

//************************************************************
//************ WINDOWS RELATED *******************************
//************************************************************

// ****************** BEGIN WARNING ***********************// 
// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY!

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	srand(unsigned int(time(0)));
	DEMO_APP myApp(hInstance, (WNDPROC)WndProc);
	MSG msg; ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT && myApp.Run())
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	myApp.ShutDown();
	return 0;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
	switch (message)
	{
	case (WM_DESTROY): { PostQuitMessage(0); }
					   break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
//********************* END WARNING ************************//