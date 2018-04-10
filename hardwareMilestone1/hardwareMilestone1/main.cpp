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

	ID3D11Texture2D				*depthStencil;
	ID3D11DepthStencilView		*depthStencilView;

	ID3D11Buffer				*vertexBuffer;
	ID3D11Buffer				*indexBuffer;
	ID3D11Buffer				*constantBuffer;

	XMMATRIX					worldM;
	XMMATRIX					viewM;
	XMMATRIX					projM;

	XMMATRIX					worldM2;

	XTime timer;

	struct MATRIX_DATA
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
	};

	float currentFOV = 75.0f;
	float minFOV = 50.0f;
	float maxFOV = 120.0f;

public:
	struct SIMPLE_VERTEX
	{
		XMFLOAT3 xyz;
		XMFLOAT4 color;
	};

	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
	bool ResizeWindow();
	bool MoveCamera();
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

	window = CreateWindow(L"DirectXApplication", L"CGS Hardware Project", WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX),
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

	context->RSSetViewports(1, &viewport);

	// CREATE DEPTH BUFFER
	D3D11_TEXTURE2D_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(depthDesc));
	depthDesc.Width = BACKBUFFER_WIDTH;
	depthDesc.Height = BACKBUFFER_HEIGHT;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.CPUAccessFlags = NULL;
	depthDesc.MiscFlags = NULL;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	device->CreateTexture2D(&depthDesc, NULL, &depthStencil);

	D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc;
	ZeroMemory(&viewDesc, sizeof(viewDesc));
	viewDesc.Format = depthDesc.Format;
	viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(depthStencil, &viewDesc, &depthStencilView);


	// DEFINE SHADERS
	device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &vs);
	device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &ps);

	// CREATE INPUT LAYOUT
	D3D11_INPUT_ELEMENT_DESC vLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	device->CreateInputLayout(vLayout, ARRAYSIZE(vLayout), Trivial_VS, sizeof(Trivial_VS), &inputLayout);

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
	worldM2 = XMMatrixIdentity();

	// view matrix & proj, REPLACE THIS!!!
	XMVECTOR Eye = XMVectorSet(2.5f, 1.0f, -2.5f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	viewM = XMMatrixLookAtLH(Eye, At, Up);

	projM = XMMatrixPerspectiveFovLH(XMConvertToRadians(currentFOV), BACKBUFFER_WIDTH / (FLOAT)BACKBUFFER_HEIGHT, 0.01f, 100.0f);

}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	timer.Signal();
	MoveCamera();
	context->OMSetRenderTargets(1, &rtv, depthStencilView);
	//context->RSSetViewports(1, &viewport);

	// CLEAR RTV TO BLUE
	FLOAT color[4] = { 0.0, 0.0f, 0.4f, 1.0f };
	context->ClearRenderTargetView(rtv, color);

	// SET INPUT LAYOUT
	context->IASetInputLayout(inputLayout);

	// CLEAR DEPTH TO 1.0
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// SET CB AND SHADERS
	context->VSSetConstantBuffers(0, 1, &constantBuffer);
	context->VSSetShader(vs, 0, 0);
	context->PSSetShader(ps, 0, 0);

	// UPDATE MATRIX DATA
	double time = timer.TotalTime();

	worldM = XMMatrixRotationY(-time);

	XMMATRIX spinM = XMMatrixRotationY(-time);
	XMMATRIX orbitM = XMMatrixRotationY(-time * 1.5f);
	XMMATRIX translateM = XMMatrixTranslation(-1.5f, 0.0f, 0.0f);
	XMMATRIX scaleM = XMMatrixScaling(0.25f, 0.25f, 0.25f);
	worldM2 = scaleM * spinM * translateM * orbitM;

	// DRAW FIRST CUBE
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

	// DRAW SECOND CUBE

	/*
	MATRIX_DATA cbData2;
	cbData2.world = worldM2;
	cbData2.view = viewM;
	cbData2.proj = projM;
	*/
	cbData.world = worldM2;
	D3D11_MAPPED_SUBRESOURCE cubeSub2;
	context->Map(constantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &cubeSub2);
	memcpy(cubeSub2.pData, &cbData, sizeof(cbData));
	context->Unmap(constantBuffer, NULL);
	context->DrawIndexed(36, 0, 0);


	swap->Present(0, 0);
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

	inputLayout->Release();
	vs->Release();
	ps->Release();
	depthStencil->Release();
	depthStencilView->Release();

	vertexBuffer->Release();
	indexBuffer->Release();
	constantBuffer->Release();


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
DEMO_APP *myApp;
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	srand(unsigned int(time(0)));
	myApp = new DEMO_APP(hInstance, (WNDPROC)WndProc);
	MSG msg; ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT && myApp->Run())
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	myApp->ShutDown();
	delete myApp;
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
	case WM_SIZE:
	{
		myApp->ResizeWindow();
		break;
	}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
//********************* END WARNING ************************//

//*****************************************//
//************ CAMERA MOVEMENT ************//
//*****************************************//
bool DEMO_APP::MoveCamera()
{
	float speed = 0.001f;

	unsigned int inputs[] = { 'W', 'A', 'S', 'D', 'Q', 'E', VK_UP, VK_DOWN};
	float activeKeys[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	float keyEffect[] = { 1, -1, -1, 1, 1, -1, -1, 1 };

	for (int i = 0; i < ARRAYSIZE(inputs); i++)
	{
		if (GetAsyncKeyState(inputs[i]))
		{
			activeKeys[i] = 1;
		}
	}
	// change FOV for zooming
	currentFOV += (30 * speed * ((activeKeys[6] * keyEffect[6]) + (activeKeys[7] * keyEffect[7])));
	if (currentFOV < minFOV)
		currentFOV = minFOV;
	if (currentFOV > maxFOV)
		currentFOV = maxFOV;

	// start camera movement
	XMMATRIX worldViewM = XMMatrixInverse(0, viewM);
	XMFLOAT3 translation(
		speed * ((activeKeys[1] * keyEffect[1]) + (activeKeys[3] * keyEffect[3])),
		speed * ((activeKeys[5] * keyEffect[5]) + (activeKeys[4] * keyEffect[4])),
		speed * ((activeKeys[0] * keyEffect[0]) + (activeKeys[2] * keyEffect[2]))
		);



	if (translation.x != 0.0f || translation.y != 0.0f || translation.z != 0.0f)
	{
		XMMATRIX translate = XMMatrixTranslation(translation.x, translation.y, translation.z);

		worldViewM = translate * worldViewM;

		viewM = XMMatrixInverse(0, worldViewM);
	}

	//update proj matrix with new FOV
	DXGI_SWAP_CHAIN_DESC current;
	swap->GetDesc(&current);
	projM = XMMatrixPerspectiveFovLH(XMConvertToRadians(currentFOV), (float)current.BufferDesc.Width / (float)current.BufferDesc.Width, 0.1f, 100.0f);

	return true;
}

//***************************************//
//************ RESIZE WINDOW ************//
//***************************************//
bool DEMO_APP::ResizeWindow()
{
	if (!this) return false;

	context->ClearState();
	rtv->Release();


	swap->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

	ID3D11Texture2D *tempBackBuffer;
	swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&tempBackBuffer);
	device->CreateRenderTargetView(tempBackBuffer, NULL, &rtv);

	DXGI_SWAP_CHAIN_DESC current;
	swap->GetDesc(&current);
	
	tempBackBuffer->Release();

	D3D11_VIEWPORT vp;
	vp.Width = float(current.BufferDesc.Width);
	vp.Height = float(current.BufferDesc.Height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	context->RSSetViewports(1, &vp);

	projM = XMMatrixPerspectiveFovLH(XMConvertToRadians(currentFOV), vp.Width / vp.Height, 0.1f, 100.0f);

	depthStencil->Release();
	depthStencilView->Release();

	//create new zbuffer
	D3D11_TEXTURE2D_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(depthDesc));
	depthDesc.Width = vp.Width;
	depthDesc.Height = vp.Height;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.CPUAccessFlags = NULL;
	depthDesc.MiscFlags = NULL;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	device->CreateTexture2D(&depthDesc, NULL, &depthStencil);

	device->CreateDepthStencilView(depthStencil, 0, &depthStencilView);

	context->OMSetRenderTargets(1, &rtv, depthStencilView);

	return true;
}