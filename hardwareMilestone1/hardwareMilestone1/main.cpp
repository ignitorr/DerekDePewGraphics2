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
	// TODO: PART 1 STEP 2
	ID3D11Device *device;
	IDXGISwapChain *swap;
	ID3D11RenderTargetView *rtv;
	ID3D11DeviceContext *context;
	D3D11_VIEWPORT viewport;

	DXGI_SWAP_CHAIN_DESC sd = {};

	
	
	// TODO: PART 2 STEP 2
	ID3D11Buffer *buffer;
	unsigned int vertCount = 0;
	D3D11_BUFFER_DESC bd = {};

	ID3D11InputLayout *inputLayout;

	
	// BEGIN PART 5
	// TODO: PART 5 STEP 1
	ID3D11Buffer *boardBuffer;
	unsigned int boardCount = 0;
	D3D11_BUFFER_DESC bbd = {};
	
	// TODO: PART 2 STEP 4
	ID3D11VertexShader *vs;
	ID3D11PixelShader *ps;
	// BEGIN PART 3
	// TODO: PART 3 STEP 1
	ID3D11Buffer *cBuffer;
	D3D11_BUFFER_DESC cbd = {};
	XTime timer;
	// TODO: PART 3 STEP 2b
	struct SEND_TO_VRAM
	{
		DirectX::XMFLOAT4 constantColor;
		DirectX::XMFLOAT2 constantOffset;
		DirectX::XMFLOAT2 padding;
	};
	// TODO: PART 3 STEP 4a
	SEND_TO_VRAM toShader;
	SEND_TO_VRAM toGrid;




	DirectX::XMFLOAT2 vel = { 1.0f, 0.5f };

	ID3D11Buffer *cubeBuffer;
	D3D11_BUFFER_DESC cubeBD = {};

	ID3D11Buffer *sceneBuffer;
	D3D11_BUFFER_DESC sceneBD = {};

	struct MATRIX_DATA
	{
		XMFLOAT4X4 view;
		XMFLOAT4X4 proj;
	};

	MATRIX_DATA mData;

public:
	// BEGIN PART 2
	// TODO: PART 2 STEP 1
	struct SIMPLE_VERTEX
	{
		DirectX::XMFLOAT2 xy;
	};

	struct TEST_VERTEX
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 rgba;
		
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
    ZeroMemory( &wndClass, sizeof( wndClass ) );
    wndClass.cbSize         = sizeof( WNDCLASSEX );             
    wndClass.lpfnWndProc    = appWndProc;						
    wndClass.lpszClassName  = L"DirectXApplication";            
	wndClass.hInstance      = application;		               
    wndClass.hCursor        = LoadCursor( NULL, IDC_ARROW );    
    wndClass.hbrBackground  = ( HBRUSH )( COLOR_WINDOWFRAME ); 
	//wndClass.hIcon			= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FSICON));
    RegisterClassEx( &wndClass );

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(	L"DirectXApplication", L"CGS Hardware Project",	WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME|WS_MAXIMIZEBOX), 
							CW_USEDEFAULT, CW_USEDEFAULT, window_size.right-window_size.left, window_size.bottom-window_size.top,					
							NULL, NULL,	application, this );												

    ShowWindow( window, SW_SHOW );
	//********************* END WARNING ************************//

	// TODO: PART 1 STEP 3a
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
	// TODO: PART 1 STEP 4
	ID3D11Texture2D *backBuffer;
	swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);

	device->CreateRenderTargetView(backBuffer, NULL, &rtv);
	backBuffer->Release();
	// TODO: PART 1 STEP 5
	viewport.Width = 500;
	viewport.Height = 500;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	// TODO: PART 2 STEP 3a
	SIMPLE_VERTEX circleVerts[361];
	for (int i = 0; i <= 360; i++)
	{

		float x = sin(DirectX::XM_PI / 180.0f * (i));
		float y = cos(DirectX::XM_PI / 180.0f * (i));
		circleVerts[i].xy = { x, y };
	}
	// BEGIN PART 4
	// TODO: PART 4 STEP 1
	for (int i = 0; i <= 360; i++)
	{
		circleVerts[i].xy.x *= 0.2f;
		circleVerts[i].xy.y *= 0.2f;
	}
	// TODO: PART 2 STEP 3b
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = NULL;
	bd.ByteWidth = sizeof(SIMPLE_VERTEX) * 361;
	
    // TODO: PART 2 STEP 3c
	D3D11_SUBRESOURCE_DATA bufferData = {0};
	bufferData.pSysMem = circleVerts;
	// TODO: PART 2 STEP 3d
	device->CreateBuffer(&bd, &bufferData, &buffer);
	// TODO: PART 5 STEP 2a
	SIMPLE_VERTEX gridVerts[6 * 200];

	// TODO: PART 5 STEP 2b
	
	for (int y = 0; y < 20; y++)
	{
		for (int x = 0; x < 10; x++)
		{
			int index = 0 + (y * 60) + (x*6);
			bool offsetGrid = (y % 2 == 0) ? true : false;
			float offset = 0;
			if (offsetGrid)
			{
				offset = 0.1f;
			}
			float curX, curY;
			curX = offset + -1.0f + (0.2 * x);
			curY = 1.0f - (0.1 * y);
			//triangle 1
			gridVerts[index].xy = { curX, curY };
			gridVerts[index + 1].xy = { curX + 0.1f, curY - 0.1f };
			gridVerts[index + 2].xy = { curX, curY - 0.1f };
								
			//triangle 2		
			gridVerts[index + 3].xy = { curX, curY };
			gridVerts[index + 4].xy = { curX + 0.1f, curY };
			gridVerts[index + 5].xy = { curX + 0.1f, curY - 0.1f };
		}
	}


	// G2 CUBE
	TEST_VERTEX cube[] =
	{
		{ XMFLOAT3(-0.5f, 0.5f,-0.5f), XMFLOAT4(0, 1.0f, 0, 1.0f) },
		{ XMFLOAT3(0.5f, 0.5f,-0.5f), XMFLOAT4(0, 1.0f, 0, 1.0f) },
		{ XMFLOAT3(0.5f,-0.5f,-0.5f), XMFLOAT4(0, 1.0f, 0, 1.0f) },
		{ XMFLOAT3(-0.5f,-0.5f,-0.5f), XMFLOAT4(0, 1.0f, 0, 1.0f) },

		{ XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT4(0, 1.0f, 0, 1.0f) },
		{ XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT4(0, 1.0f, 0, 1.0f) },
		{ XMFLOAT3(0.5f,-0.5f, 0.5f), XMFLOAT4(0, 1.0f, 0, 1.0f) },
		{ XMFLOAT3(-0.5f,-0.5f, 0.5f), XMFLOAT4(0, 1.0f, 0, 1.0f) },
	};
	short cubeInd[] =
	{
		0,1,3, 3,1,2,
		0,4,5, 0,5,1,
		1,5,2, 2,5,6,
		4,0,7, 7,0,3,
		7,3,2, 7,2,6,
		5,4,6, 6,4,7
	};
	D3D11_SUBRESOURCE_DATA cubeBufferData = { 0 };
	cubeBufferData.pSysMem = cube;

	cubeBD.Usage = D3D11_USAGE_IMMUTABLE;
	cubeBD.BindFlags = D3D11_BIND_INDEX_BUFFER;
	cubeBD.CPUAccessFlags = NULL;
	cubeBD.ByteWidth = sizeof(short) * 36;

	XMMATRIX view = XMLoadFloat4x4(&mData.view);
	XMMatrix
	
	// TODO: PART 5 STEP 3

	bbd.Usage = D3D11_USAGE_IMMUTABLE;
	bbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bbd.CPUAccessFlags = NULL;
	bbd.ByteWidth = sizeof(SIMPLE_VERTEX) * 1200;
	//bbd.ByteWidth = sizeof(SIMPLE_VERTEX) * 6;
	D3D11_SUBRESOURCE_DATA bBufferData = { 0 };
	bBufferData.pSysMem = gridVerts;

	device->CreateBuffer(&bbd, &bBufferData, &boardBuffer);
	// TODO: PART 2 STEP 5
	// ADD SHADERS TO PROJECT, SET BUILD OPTIONS & COMPILE

	// TODO: PART 2 STEP 7
	device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &vs);
	device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &ps);
	
	// TODO: PART 2 STEP 8a
	D3D11_INPUT_ELEMENT_DESC vLayout =
	{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	// TODO: PART 2 STEP 8b
	device->CreateInputLayout(&vLayout, 1, Trivial_VS, sizeof(Trivial_VS), &inputLayout);
	// TODO: PART 3 STEP 3
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.ByteWidth = sizeof(SEND_TO_VRAM);

	device->CreateBuffer(&cbd, NULL, &cBuffer);
	// TODO: PART 3 STEP 4b
	toShader.constantOffset = { 0, 0 };
	toShader.constantColor = { 1.0f, 1.0f, 0.0f, 1.0f };
}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	// TODO: PART 4 STEP 2	
	timer.Signal();
	// TODO: PART 4 STEP 3
	toShader.constantOffset.x += vel.x * timer.Delta();
	toShader.constantOffset.y += vel.y * timer.Delta();

	// TODO: PART 4 STEP 5
	if (toShader.constantOffset.x >= 1.0f || toShader.constantOffset.x <= -1.0f)
	{
		vel.x = -vel.x;
	}
	if (toShader.constantOffset.y >= 1.0f || toShader.constantOffset.y <= -1.0f)
	{
		vel.y = -vel.y;
	}
	// END PART 4

	// TODO: PART 1 STEP 7a
	context->OMSetRenderTargets(1, &rtv, NULL);
	
	// TODO: PART 1 STEP 7b
	context->RSSetViewports(1, &viewport);
	// TODO: PART 1 STEP 7c
	FLOAT color[4] = { 0.0, 0.0f, 0.4f, 1.0f };
	context->ClearRenderTargetView(rtv, color);
	// TODO: PART 5 STEP 4
	toGrid.constantOffset = { 0, 0 };
	toGrid.constantColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	// TODO: PART 5 STEP 5
	D3D11_MAPPED_SUBRESOURCE bSub;
	context->Map(cBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &bSub);
	memcpy(bSub.pData, &toGrid, sizeof(toGrid));
	context->Unmap(cBuffer, NULL);
	// TODO: PART 5 STEP 6
	context->VSSetConstantBuffers(0, 1, &cBuffer);
	UINT gridStrides = sizeof(SIMPLE_VERTEX);
	UINT gridOffsets = 0;
	context->IASetVertexBuffers(0, 1, &boardBuffer, &gridStrides, &gridOffsets);
	context->IASetInputLayout(inputLayout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// TODO: PART 5 STEP 7
	context->VSSetShader(vs, 0, 0);
	context->Draw(1200, 0);
	// END PART 5
	
	// TODO: PART 3 STEP 5
	D3D11_MAPPED_SUBRESOURCE sub;
	context->Map(cBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &sub);
	memcpy(sub.pData, &toShader, sizeof(toShader));
	context->Unmap(cBuffer, NULL);
	// TODO: PART 3 STEP 6
	context->VSSetConstantBuffers(0, 1, &cBuffer);
	// TODO: PART 2 STEP 9a
	UINT strides = sizeof(SIMPLE_VERTEX);
	UINT offsets = 0;
	context->IASetVertexBuffers(0, 1, &buffer, &strides, &offsets);
	// TODO: PART 2 STEP 9b
	context->VSSetShader(vs, 0, 0);
	context->PSSetShader(ps, 0, 0);
	// TODO: PART 2 STEP 9c
	context->IASetInputLayout(inputLayout);
	// TODO: PART 2 STEP 9d
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	// TODO: PART 2 STEP 10
	context->PSSetShader(ps, 0, 0);
	context->Draw(361, 0);
	// END PART 2

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
	UnregisterClass( L"DirectXApplication", application ); 
	return true;
}

//************************************************************
//************ WINDOWS RELATED *******************************
//************************************************************

// ****************** BEGIN WARNING ***********************// 
// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY!
	
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,	int nCmdShow );						   
LRESULT CALLBACK WndProc(HWND hWnd,	UINT message, WPARAM wparam, LPARAM lparam );		
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE, LPTSTR, int )
{
	srand(unsigned int(time(0)));
	DEMO_APP myApp(hInstance,(WNDPROC)WndProc);	
    MSG msg; ZeroMemory( &msg, sizeof( msg ) );
    while ( msg.message != WM_QUIT && myApp.Run() )
    {	
	    if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        { 
            TranslateMessage( &msg );
            DispatchMessage( &msg ); 
        }
    }
	myApp.ShutDown(); 
	return 0; 
}
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    if(GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
    switch ( message )
    {
        case ( WM_DESTROY ): { PostQuitMessage( 0 ); }
        break;
    }
    return DefWindowProc( hWnd, message, wParam, lParam );
}
//********************* END WARNING ************************//