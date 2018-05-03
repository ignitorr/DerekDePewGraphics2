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
#include "DDSTextureLoader.h"
#include "Barrel.h"
#include "test pyramid.h"
#include "penguin.h"

#include <vector>

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
#include "Skybox_PS.csh"
#include "Skybox_VS.csh"

#define BACKBUFFER_WIDTH	1200
#define BACKBUFFER_HEIGHT	800

#define MESH_COUNT 10
#define INSTANCE_MESH_COUNT 10
#define D_LIGHTS 1
#define P_LIGHTS 1
#define S_LIGHTS 1
#define SKYBOX_SCALE -1

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


	// SKYBOX STUFF
	//ID3D11RasterizerState		*rState;
	//D3D11_RASTERIZER_DESC		regularRDesc;
	//D3D11_RASTERIZER_DESC		skyboxRDesc;
	ID3D11PixelShader			*skyboxPS;
	ID3D11VertexShader			*skyboxVS;
	XMMATRIX					skyboxM;
	ID3D11Buffer				*skyboxVBuffer;
	ID3D11Buffer				*skyboxIBuffer;
	ID3D11ShaderResourceView	*skyboxRV;
	ID3D11SamplerState			*skyboxSampler;
	ID3D11Buffer				*skyboxConstantBuffer;

	// END SKYBOX

	///////////////////////
	// GENERAL MESH DATA //
	///////////////////////
	unsigned int				currentIndex = 0; // every mesh created will +1 this, used for indexing when adding to the arrays
	ID3D11Buffer				*vertexBuffers[MESH_COUNT]; // create array of vertexBuffers. if all arent used thats okay
	ID3D11Buffer				*indexBuffers[MESH_COUNT];
	unsigned int				numVertices[MESH_COUNT]; // store number of verts/indices for easy draw calls :)
	unsigned int				numIndices[MESH_COUNT];
	ID3D11ShaderResourceView	*textureRVs[MESH_COUNT];
	ID3D11SamplerState			*textureSamplers[MESH_COUNT];
	XMMATRIX					worldMatrices[MESH_COUNT]; // store matrices for each object.  
	ID3D11Buffer				*pixelConstantBuffer;
	ID3D11Buffer				*vertexConstantBuffer;

	/////////////////////////
	// INSTANCED MESH DATA //
	/////////////////////////
	unsigned int				currentInstanceIndex = 0;
	ID3D11Buffer				*instanceVertexBuffers[INSTANCE_MESH_COUNT];
	ID3D11Buffer				*instanceIndexBuffers[INSTANCE_MESH_COUNT];
	unsigned int				instanceNumVertices[INSTANCE_MESH_COUNT];
	unsigned int				instanceNumIndices[INSTANCE_MESH_COUNT];
	ID3D11ShaderResourceView	*instanceTextureRVs[INSTANCE_MESH_COUNT];
	ID3D11SamplerState			*instanceTextureSamplers[INSTANCE_MESH_COUNT];
	unsigned int				instanceCount[INSTANCE_MESH_COUNT];
	ID3D11Buffer				*instanceVConstantBuffer;

	XMMATRIX					viewM;
	XMMATRIX					projM;

	XTime						timer;

	struct MATRIX_DATA
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;

		XMFLOAT4 lightDirection;
		XMFLOAT4 lightColor;
		XMFLOAT4 lightPos;
		float lightRad;
		float coneRatio;
	};

	//////////////////////
	// LIGHTING STRUCTS //
	//////////////////////
	struct directional_light
	{
		XMFLOAT4 lightDirection;
		XMFLOAT4 lightColor;
	};

	struct point_light
	{
		XMFLOAT4 lightPos;
		XMFLOAT4 lightColor;
		float lightRad;
		XMFLOAT3 filler;
	};

	struct spot_light
	{
		XMFLOAT4 lightDirection;
		XMFLOAT4 lightPos;
		XMFLOAT4 lightColor;
		float lightRad;
		float outerConeRatio;
		float innerConeRatio;
		float filler;
	};
	
	////////////////////////////////////
	// VERTEX CONSTANT BUFFER STRUCTS //
	////////////////////////////////////
	struct VS_BUFFER_DATA
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
	};

	struct SKYBOX_VS_DATA
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
		XMFLOAT3 cameraPos;
	};
	//////////////////////////////////
	// PIXEL CONSTANT BUFFER STRUCT //
	//////////////////////////////////
	struct PS_BUFFER_DATA
	{
		directional_light directional[D_LIGHTS];
		point_light point[P_LIGHTS];
		spot_light spot[S_LIGHTS];
	};

	PS_BUFFER_DATA psData;

	float pLightMove = 1.0f;

	struct SIMPLE_VERTEX
	{
		XMFLOAT3 xyz;
		//XMFLOAT4 color;
		XMFLOAT2 uv;
		XMFLOAT3 normal;
	};

	bool LoadBuffers(SIMPLE_VERTEX verts[], short indices[], unsigned int numVerts, unsigned int numInds, ID3D11Buffer **vertBuffer, ID3D11Buffer **indBuffer);
	bool LoadTexture(const wchar_t *texturePath, ID3D11ShaderResourceView **textureRV, ID3D11SamplerState **textureSampler);
	bool LoadMeshFromHeader(const OBJ_VERT verts[], const unsigned int indices[], unsigned int numVerts, unsigned int numInd, const wchar_t *texturePath);
	bool LoadOBJ(const char *filePath, const wchar_t *texturePath);
	bool CreateIndexedCube(float scale, const wchar_t *texturePath);
	bool CreateSkybox(const wchar_t *texturePath);
	bool CreateInstancedCube(float scale, const wchar_t *texturePath, unsigned int count, XMFLOAT3 offset);
public:
	DEMO_APP(HINSTANCE hinst, WNDPROC proc);

	float currentFOV = 75.0f;
	float minFOV = 50.0f;
	float maxFOV = 120.0f;

	bool Run();
	bool ShutDown();
	bool ResizeWindow();
	bool MoveCamera();
};

//////////////////////////////////////////////////////////
// Fills out vertex and index buffers for given arrays. 
// Partner function for model creation.				    
// All mesh creation functions call this.			    
//////////////////////////////////////////////////////////
bool DEMO_APP::LoadBuffers(SIMPLE_VERTEX verts[], short indices[], unsigned int numVerts, unsigned int numInds, ID3D11Buffer **vertBuffer, ID3D11Buffer **indBuffer)
{
	// create vertex buffer
	D3D11_BUFFER_DESC headerBD;
	ZeroMemory(&headerBD, sizeof(headerBD));
	headerBD.Usage = D3D11_USAGE_IMMUTABLE;
	headerBD.ByteWidth = sizeof(SIMPLE_VERTEX) * numVerts;
	headerBD.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	headerBD.CPUAccessFlags = NULL;

	D3D11_SUBRESOURCE_DATA headerBufferData;
	ZeroMemory(&headerBufferData, sizeof(headerBufferData));
	headerBufferData.pSysMem = verts;
	//device->CreateBuffer(&headerBD, &headerBufferData, &vertexBuffers[currentIndex]);
	device->CreateBuffer(&headerBD, &headerBufferData, vertBuffer);
	// create index buffer
	headerBD.Usage = D3D11_USAGE_IMMUTABLE;
	headerBD.BindFlags = D3D11_BIND_INDEX_BUFFER;
	headerBD.CPUAccessFlags = NULL;
	headerBD.ByteWidth = sizeof(short) * numInds;

	headerBufferData.pSysMem = indices;
	//device->CreateBuffer(&headerBD, &headerBufferData, &indexBuffers[currentIndex]);
	device->CreateBuffer(&headerBD, &headerBufferData, indBuffer);

	return true;
}

////////////////////////////////////////////
// Loads texture from given texturePath.  
// Partner function for model creation.    
// All mesh creation functions call this.	
///////////////////////////////////////////
bool DEMO_APP::LoadTexture(const wchar_t *texturePath, ID3D11ShaderResourceView **textureRV, ID3D11SamplerState **textureSampler)
{
	CreateDDSTextureFromFile(device, texturePath, nullptr, textureRV);
	// will need to add char arg to function
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&samplerDesc, textureSampler);

	return true;
}

//////////////////////////////////////////////////////////////////////
// Loads in OBJ, and adds it to the vertex and index buffer arrays. 
//////////////////////////////////////////////////////////////////////
bool DEMO_APP::LoadOBJ(const char *filePath, const wchar_t *texturePath)
{
	// abort if file not found
	FILE *file = fopen(filePath, "r");
	if (file == NULL)
		return false;

	// create vectors for verts, uvs, normals
	vector<XMFLOAT3> verts;
	vector<XMFLOAT2> uvs;
	vector<XMFLOAT3> normals;
	vector<unsigned int> vertIndices;
	vector<unsigned int> uvIndices;
	vector<unsigned int> normIndices;

	while (true)
	{
		char line[128];
		int res = fscanf(file, "%s", line);
		if (res == EOF)
			break;

		// vertices
		if (strcmp(line, "v") == 0)
		{
			XMFLOAT3 newVert;
			fscanf(file, "%f %f %f\n", &newVert.x, &newVert.y, &newVert.z);
			verts.push_back(newVert);
		}
		// UVs
		else if (strcmp(line, "vt") == 0)
		{
			XMFLOAT2 newUV;
			fscanf(file, "%f %f\n", &newUV.x, &newUV.y);
			newUV.y *= -1.0f;
			uvs.push_back(newUV);
		}
		// normals
		else if (strcmp(line, "vn") == 0)
		{
			XMFLOAT3 newNorm;
			fscanf(file, "%f %f %f\n", &newNorm.x, &newNorm.y, &newNorm.z);
			normals.push_back(newNorm);
		}
		// triangle list, use for indices
		else if (strcmp(line, "f") == 0)
		{
			unsigned int vertInd[3], uvInd[3], normInd[3];
			fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
				&vertInd[0], &uvInd[0], &normInd[0],
				&vertInd[1], &uvInd[1], &normInd[1],
				&vertInd[2], &uvInd[2], &normInd[2]);

			vertIndices.push_back(vertInd[0]);
			vertIndices.push_back(vertInd[1]);
			vertIndices.push_back(vertInd[2]);
			uvIndices.push_back(uvInd[0]);
			uvIndices.push_back(uvInd[1]);
			uvIndices.push_back(uvInd[2]);
			normIndices.push_back(normInd[0]);
			normIndices.push_back(normInd[1]);
			normIndices.push_back(normInd[2]);
		}
	}

	// now take the parsed data and convert it to something usable
	SIMPLE_VERTEX *meshVerts = new SIMPLE_VERTEX[vertIndices.size()];
	short *meshIndices = new short[vertIndices.size()];

	// fill out the vertex data
	unsigned int numVerts = normals.size();
	unsigned int numInd = vertIndices.size();
	
	// since we're looping above the size of the vert array, need to use something else to index into it.
	unsigned int currentVertexIndex = 0;
	for (int i = 0; i < numInd; i++)
	{
		XMFLOAT3 newVert = verts[vertIndices[i] - 1];
		XMFLOAT2 newUV = uvs[uvIndices[i] - 1];
		XMFLOAT3 newNorm = normals[normIndices[i] - 1];

		SIMPLE_VERTEX tempVertex;
		tempVertex.xyz = newVert;
		tempVertex.uv = newUV;
		tempVertex.normal = newNorm;

		// loop through and check if we've made this vertex already
		bool isNewVert = true;
		for (int v = 0; v < currentVertexIndex; v++)
		{
			if (tempVertex.xyz.x == meshVerts[v].xyz.x && tempVertex.xyz.y == meshVerts[v].xyz.y && tempVertex.xyz.z == meshVerts[v].xyz.z)
			{
				if (tempVertex.uv.x == meshVerts[v].uv.x && tempVertex.uv.y == meshVerts[v].uv.y)
				{
					if (tempVertex.normal.x == meshVerts[v].normal.x && tempVertex.normal.y == meshVerts[v].normal.y && tempVertex.normal.z == meshVerts[v].normal.z)
					{
						isNewVert = false;
						// update index list
						meshIndices[i] = v;
					}
				}
			}
		}
		if (isNewVert)
		{
			meshVerts[currentVertexIndex].xyz = newVert;
			meshVerts[currentVertexIndex].uv = newUV;
			meshVerts[currentVertexIndex].normal = newNorm;
			// update index list with new vert
			meshIndices[i] = currentVertexIndex;
			// update current index
			currentVertexIndex++;
		}
	}
	LoadBuffers(meshVerts, meshIndices, normals.size(), vertIndices.size(), &vertexBuffers[currentIndex], &indexBuffers[currentIndex]);

	//update number arrays
	numVertices[currentIndex] = normals.size();
	numIndices[currentIndex] = vertIndices.size();

	// load the model's texture
	LoadTexture(texturePath, &textureRVs[currentIndex], &textureSamplers[currentIndex]);

	currentIndex += 1;
	return true;
}

// currently this requires atleast one header file to be loaded in that defines the OBJ_VERT class...
bool DEMO_APP::LoadMeshFromHeader(const OBJ_VERT verts[], const unsigned int indices[], unsigned int numVerts, unsigned int numInd, const wchar_t *texturePath)
{
	SIMPLE_VERTEX *meshVerts = new SIMPLE_VERTEX[numVerts];
	short* meshIndices = new short[numInd];
	// first copy the verts
	for (int i = 0; i < numVerts; i++)
	{
		meshVerts[i].xyz = XMFLOAT3(verts[i].pos[0], verts[i].pos[1], verts[i].pos[2]);
		meshVerts[i].uv = XMFLOAT2(verts[i].uvw[0], verts[i].uvw[1]);
		meshVerts[i].normal = XMFLOAT3(verts[i].nrm[0], verts[i].nrm[1], verts[i].nrm[2]);
	}
	// next copy the index list
	for (int i = 0; i < numInd; i++)
	{
		meshIndices[i] = indices[i];
	}

	LoadBuffers(meshVerts, meshIndices, numVerts, numInd, &vertexBuffers[currentIndex], &indexBuffers[currentIndex]);

	//update number arrays
	numVertices[currentIndex] = numVerts;
	numIndices[currentIndex] = numInd;

	// load the model's texture
	LoadTexture(texturePath, &textureRVs[currentIndex], &textureSamplers[currentIndex]);

	// update currentIndex before exiting
	currentIndex += 1;

	return true;
}

///////////////////////////////////////////////////
// Creates an indexed cube in the buffer arrays. 
///////////////////////////////////////////////////
bool DEMO_APP::CreateIndexedCube(float scale, const wchar_t *texturePath)
{
	SIMPLE_VERTEX cube[] =
	{
		{ XMFLOAT3(-0.5f, 0.5f,-0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(0.5f, 0.5f,-0.5f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(0.5f,0.5f,0.5f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(-0.5f,0.5f,0.5f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },

		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(0.5f,-0.5f, 0.5f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(-0.5f,-0.5f, 0.5f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },

		{ XMFLOAT3(-0.5f,-0.5f, 0.5f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-0.5f,-0.5f, -0.5f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-0.5f,0.5f, -0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-0.5f,0.5f, 0.5f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

		{ XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },

		{ XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },

	};
	short cubeInd[] =
	{

		3,1,0, 2,1,3,
		6,4,5, 7,4,6,
		11,9,8, 10,9,11,
		14,12,13, 15,12,14,
		19,17,16, 18,17,19,
		22,20,21, 23,20,22

	};

	LoadBuffers(cube, cubeInd, ARRAYSIZE(cube), ARRAYSIZE(cubeInd), &vertexBuffers[currentIndex], &indexBuffers[currentIndex]);

	// update number arrays
	numVertices[currentIndex] = ARRAYSIZE(cube);
	numIndices[currentIndex] = ARRAYSIZE(cubeInd);

	// load the model's texture
	LoadTexture(texturePath, &textureRVs[currentIndex], &textureSamplers[currentIndex]);

	// create worldMatrix
	worldMatrices[currentIndex] = XMMatrixScaling(scale, scale, scale) * XMMatrixIdentity();
	
	// update currentIndex before exiting
	currentIndex += 1;

	return true;
}

//////////////////////////////////////////////////////
// Creates a Skybox with the given cubemap texture. 
//////////////////////////////////////////////////////
bool DEMO_APP::CreateSkybox(const wchar_t *texturePath)
{
	SIMPLE_VERTEX cube[] =
	{
		{ XMFLOAT3(-0.5f, 0.5f,-0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(0.5f, 0.5f,-0.5f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(0.5f,0.5f,0.5f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(-0.5f,0.5f,0.5f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },

		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(0.5f,-0.5f, 0.5f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(-0.5f,-0.5f, 0.5f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },

		{ XMFLOAT3(-0.5f,-0.5f, 0.5f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-0.5f,-0.5f, -0.5f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-0.5f,0.5f, -0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-0.5f,0.5f, 0.5f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

		{ XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },

		{ XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },

	};
	short cubeInd[] =
	{

		3,1,0, 2,1,3,
		6,4,5, 7,4,6,
		11,9,8, 10,9,11,
		14,12,13, 15,12,14,
		19,17,16, 18,17,19,
		22,20,21, 23,20,22

	};

	LoadBuffers(cube, cubeInd, ARRAYSIZE(cube), ARRAYSIZE(cubeInd), &skyboxVBuffer, &skyboxIBuffer);

	LoadTexture(texturePath, &skyboxRV, &skyboxSampler);

	return true;
}

/////////////////////////////////////
// Create instanced cube.
// Basic stuff to test instancing.
/////////////////////////////////////
bool DEMO_APP::CreateInstancedCube(float scale, const wchar_t *texturePath, unsigned int count, XMFLOAT3 offset)
{
	SIMPLE_VERTEX cube[] =
	{
		{ XMFLOAT3(-0.5f, 0.5f,-0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(0.5f, 0.5f,-0.5f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(0.5f,0.5f,0.5f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(-0.5f,0.5f,0.5f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },

		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(0.5f,-0.5f, 0.5f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(-0.5f,-0.5f, 0.5f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },

		{ XMFLOAT3(-0.5f,-0.5f, 0.5f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-0.5f,-0.5f, -0.5f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-0.5f,0.5f, -0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-0.5f,0.5f, 0.5f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

		{ XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },

		{ XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },

	};
	short cubeInd[] =
	{

		3,1,0, 2,1,3,
		6,4,5, 7,4,6,
		11,9,8, 10,9,11,
		14,12,13, 15,12,14,
		19,17,16, 18,17,19,
		22,20,21, 23,20,22

	};

	D3D11_BUFFER_DESC headerBD;
	ZeroMemory(&headerBD, sizeof(headerBD));
	headerBD.Usage = D3D11_USAGE_IMMUTABLE;
	headerBD.ByteWidth = sizeof(SIMPLE_VERTEX) * ARRAYSIZE(cube);
	headerBD.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	headerBD.CPUAccessFlags = NULL;

	D3D11_SUBRESOURCE_DATA headerBufferData;
	ZeroMemory(&headerBufferData, sizeof(headerBufferData));
	headerBufferData.pSysMem = cube;
	//device->CreateBuffer(&headerBD, &headerBufferData, &vertexBuffers[currentIndex]);
	device->CreateBuffer(&headerBD, &headerBufferData, &instanceVertexBuffers[currentInstanceIndex]);
	// create index buffer
	headerBD.Usage = D3D11_USAGE_IMMUTABLE;
	headerBD.BindFlags = D3D11_BIND_INDEX_BUFFER;
	headerBD.CPUAccessFlags = NULL;
	headerBD.ByteWidth = sizeof(short) * ARRAYSIZE(cubeInd);

	headerBufferData.pSysMem = cubeInd;
	//device->CreateBuffer(&headerBD, &headerBufferData, &indexBuffers[currentIndex]);
	device->CreateBuffer(&headerBD, &headerBufferData, &instanceIndexBuffers[currentInstanceIndex]);
	//LoadBuffers(cube, cubeInd, ARRAYSIZE(cube), ARRAYSIZE(cubeInd), &instanceVertexBuffers[currentInstanceIndex], &instanceIndexBuffers[currentInstanceIndex]);

	LoadTexture(texturePath, &instanceTextureRVs[currentInstanceIndex], &instanceTextureSamplers[currentInstanceIndex]);

	instanceCount[currentInstanceIndex] = count;
	currentInstanceIndex++;

	return true;
}

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
	sd.BufferDesc.Width = BACKBUFFER_WIDTH;
	sd.BufferDesc.Height = BACKBUFFER_HEIGHT;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.Windowed = true;
	sd.OutputWindow = window;
	sd.SampleDesc.Count = 4; //1
	sd.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN; //0 for no MSAA

	// CREATE DEVICE AND SWAPCHAIN
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
	viewport.Width = BACKBUFFER_WIDTH;
	viewport.Height = BACKBUFFER_HEIGHT;
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
	depthDesc.SampleDesc.Count = 4; //1
	depthDesc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN; //0;
	device->CreateTexture2D(&depthDesc, NULL, &depthStencil);

	D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc;
	ZeroMemory(&viewDesc, sizeof(viewDesc));
	viewDesc.Format = depthDesc.Format;
	viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	viewDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(depthStencil, &viewDesc, &depthStencilView);


	// DEFINE SHADERS
	device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &vs);
	device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &ps);

	HRESULT rest = device->CreateVertexShader(Skybox_VS, sizeof(Skybox_VS), NULL, &skyboxVS);
	device->CreatePixelShader(Skybox_PS, sizeof(Skybox_PS), NULL, &skyboxPS);

	

	// CREATE INPUT LAYOUT
	D3D11_INPUT_ELEMENT_DESC vLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SV_INSTANCEID", 0, DXGI_FORMAT_R32_UINT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		//{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	device->CreateInputLayout(vLayout, ARRAYSIZE(vLayout), Trivial_VS, sizeof(Trivial_VS), &inputLayout);

	// NEW CONSTANT BUFFERS
	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));

	// FIRST VERTEX CONSTANT BUFFER
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.ByteWidth = sizeof(VS_BUFFER_DATA);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	device->CreateBuffer(&cbDesc, NULL, &vertexConstantBuffer);

	// NOW PIXEL CONSTANT BUFFER
	cbDesc.ByteWidth = sizeof(PS_BUFFER_DATA);
	device->CreateBuffer(&cbDesc, NULL, &pixelConstantBuffer);

	///////////////////////////////////
	// LOAD MESHES AND THEIR BUFFERS //
	///////////////////////////////////
	worldMatrices[currentIndex] = XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixIdentity() * 	XMMatrixTranslation(4.0f, 0.0f, 0.0f);
	//LoadMeshFromHeader(Barrel_data, Barrel_indicies, ARRAYSIZE(Barrel_data), ARRAYSIZE(Barrel_indicies), L"barrel.dds");
	LoadOBJ("Barrel.obj", L"barrel.dds");
	worldMatrices[currentIndex] = XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixIdentity() * 	XMMatrixTranslation(3.0f, 2.0f, 0.0f);
	LoadOBJ("Barrel.obj", L"barrel.dds");
	worldMatrices[currentIndex] = XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixIdentity() * 	XMMatrixTranslation(5.0f, 2.0f, 0.0f);
	LoadOBJ("Barrel.obj", L"barrel.dds");
	worldMatrices[currentIndex] = XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixIdentity() * 	XMMatrixTranslation(4.0f, 4.0f, 0.0f);
	LoadOBJ("Barrel.obj", L"barrel.dds");
	worldMatrices[currentIndex] = XMMatrixScaling(15.0f, 2.0f, 15.0f) * XMMatrixIdentity() * XMMatrixTranslation(0.0f, 9.0f, 0.0f);
	LoadMeshFromHeader(test_pyramid_data, test_pyramid_indicies, ARRAYSIZE(test_pyramid_data), ARRAYSIZE(test_pyramid_indicies), L"barrel.dds");

	worldMatrices[currentIndex] = XMMatrixScaling(0.3f, 0.3f, 0.3f) * XMMatrixIdentity() * XMMatrixTranslation(-5.0f, 0.0f, 0.0f);
	bool result = LoadOBJ("penguin.obj", L"peng.dds");
	worldMatrices[currentIndex] = XMMatrixScaling(0.3f, 0.3f, 0.3f) * XMMatrixIdentity() * XMMatrixTranslation(-7.0f, 0.0f, 0.0f);
	LoadMeshFromHeader(penguin_data, penguin_indicies, ARRAYSIZE(penguin_data), ARRAYSIZE(penguin_indicies), L"peng.dds");

	CreateIndexedCube(0.5f, L"barrel.dds");

	CreateInstancedCube(0.5f, L"barrel.dds", 5, XMFLOAT3(0, 0, 2));

	//////////////////////
	// END MESH LOADING //
	//////////////////////

	// view matrix & proj
	XMVECTOR eye = XMVectorSet(0.0f, 2.0f, -3.0f, 0.0f);
	XMVECTOR at = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	viewM = XMMatrixLookAtLH(eye, at, up);

	projM = XMMatrixPerspectiveFovLH(XMConvertToRadians(currentFOV), BACKBUFFER_WIDTH / (FLOAT)BACKBUFFER_HEIGHT, 0.01f, 100.0f);

	// for now, just hard code the lights
	psData.directional[0].lightDirection = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	psData.directional[0].lightColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);

	psData.point[0].lightColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	psData.point[0].lightPos = XMFLOAT4(3.75f, 2.0f, 0.0f, 1.0f);
	psData.point[0].lightRad = 5.0f;

	psData.spot[0].lightDirection = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	psData.spot[0].lightColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	psData.spot[0].lightPos = XMFLOAT4(-1.5f, 3.0f, 0.0f, 1.0f);
	psData.spot[0].lightRad = 100.0f;
	psData.spot[0].outerConeRatio = 0.96f;
	psData.spot[0].innerConeRatio = 0.98f;



	// WIP SKYBOX CODE:
	/*
	regularRDesc.FillMode = D3D11_FILL_SOLID;
	regularRDesc.CullMode = D3D11_CULL_FRONT;
	regularRDesc.FrontCounterClockwise = true;
	regularRDesc.DepthBias = false;
	regularRDesc.DepthBiasClamp = 0;
	regularRDesc.SlopeScaledDepthBias = 0;
	regularRDesc.DepthClipEnable = true;
	regularRDesc.ScissorEnable = true;
	regularRDesc.MultisampleEnable = false;
	regularRDesc.AntialiasedLineEnable = false;

	skyboxRDesc.FillMode = D3D11_FILL_SOLID;
	skyboxRDesc.CullMode = D3D11_CULL_NONE;
	skyboxRDesc.FrontCounterClockwise = true;
	skyboxRDesc.DepthBias = false;
	skyboxRDesc.DepthBiasClamp = 0;
	skyboxRDesc.SlopeScaledDepthBias = 0;
	skyboxRDesc.DepthClipEnable = false;
	skyboxRDesc.ScissorEnable = true;
	skyboxRDesc.MultisampleEnable = false;
	skyboxRDesc.AntialiasedLineEnable = false;

	device->CreateRasterizerState(&regularRDesc, &rState);
	*/
	CreateSkybox(L"skyBox.dds");
	skyboxM = XMMatrixIdentity() * XMMatrixScaling((float)SKYBOX_SCALE, (float)SKYBOX_SCALE, (float)SKYBOX_SCALE);

	cbDesc.ByteWidth = sizeof(SKYBOX_VS_DATA);
	device->CreateBuffer(&cbDesc, NULL, &skyboxConstantBuffer);

}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{

	// reset raster state
//	device->CreateRasterizerState(&regularRDesc, &rState);
	// for skybox

	timer.Signal();
	MoveCamera();
	context->OMSetRenderTargets(1, &rtv, depthStencilView);

	// CLEAR RTV TO BLUE
	FLOAT color[4] = { 0.0, 0.0f, 0.4f, 1.0f };
	context->ClearRenderTargetView(rtv, color);

	// SET INPUT LAYOUT
	context->IASetInputLayout(inputLayout);

	// CLEAR DEPTH TO 1.0
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// DRAW SKYBOX FIRST
	UINT strides = sizeof(SIMPLE_VERTEX);
	UINT offsets = 0;
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->VSSetShader(skyboxVS, 0, 0);
	context->PSSetShader(skyboxPS, 0, 0);
	context->VSSetConstantBuffers(0, 1, &skyboxConstantBuffer);
	context->IASetVertexBuffers(0, 1, &skyboxVBuffer, &strides, &offsets);
	context->IASetIndexBuffer(skyboxIBuffer, DXGI_FORMAT_R16_UINT, 0);
	context->PSSetShaderResources(0, 1, &skyboxRV);
	context->PSSetSamplers(0, 1, &skyboxSampler);

	SKYBOX_VS_DATA skyVSData;
	skyVSData.world = skyboxM;
	skyVSData.view = viewM;
	skyVSData.proj = projM;
	XMVECTOR camPos = skyboxM.r[3];
	XMFLOAT3 camPosF;
	XMStoreFloat3(&camPosF, camPos);
	skyVSData.cameraPos = camPosF;

	D3D11_MAPPED_SUBRESOURCE vsSub;
	context->Map(skyboxConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &vsSub);
	memcpy(vsSub.pData, &skyVSData, sizeof(skyVSData));
	context->Unmap(skyboxConstantBuffer, NULL);

	context->DrawIndexed(36, 0, 0);

	// RE-CLEAR DEPTH BUFFER
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// SET CB AND SHADERS
	context->VSSetConstantBuffers(0, 1, &vertexConstantBuffer);
	context->PSSetConstantBuffers(0, 1, &pixelConstantBuffer);
	context->VSSetShader(vs, 0, 0);
	context->PSSetShader(ps, 0, 0);

	///////////////////
	// LIGHT UPDATES //
	///////////////////
	double time = timer.TotalTime();

	// SPOTLIGHT
	XMMATRIX spinM = XMMatrixRotationX(-timer.Delta());
	XMMATRIX dirSpin = XMMatrixRotationY(-timer.Delta());
	XMMATRIX orbitM = XMMatrixRotationY(-time * 5);
	XMMATRIX translateM = XMMatrixTranslation(-1.5f, 3.0f, 0.0f);
	//worldM2 = scaleM * spinM * translateM * orbitM;

	XMMATRIX spotM = translateM * orbitM;
	XMStoreFloat4(&psData.spot[0].lightPos, spotM.r[3]);
	XMVECTOR spotDir = XMLoadFloat4(&(psData.spot[0].lightDirection));
	spotDir = XMVector3Transform(spotDir, spinM);
	XMStoreFloat4(&psData.spot[0].lightDirection, spotDir);
	
	// DIRECTIONAL LIGHT
	XMVECTOR dirDir = XMLoadFloat4(&(psData.directional[0].lightDirection));
	dirDir = XMVector3Transform(dirDir, dirSpin);
	XMStoreFloat4(&psData.directional[0].lightDirection, dirDir);

	// NEXT POINT LIGHT
	// starting Y = 2, max = 4
	if (psData.point[0].lightPos.y > 4.0f)
	{
		pLightMove = -1;
	}
	if (psData.point[0].lightPos.y < 2.0f)
	{
		pLightMove = 1;
	}
	psData.point[0].lightPos.y += timer.Delta() * pLightMove * 0.5f;
	///////////////////////
	// END LIGHT UPDATES //
	///////////////////////

	VS_BUFFER_DATA vsData;
	vsData.view = viewM;
	vsData.proj = projM;

	D3D11_MAPPED_SUBRESOURCE psSub;
	context->Map(pixelConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &psSub);
	memcpy(psSub.pData, &psData, sizeof(psData));
	context->Unmap(pixelConstantBuffer, NULL);
	// LOOP THRU ARRAYS AND DRAW
	for (int i = 0; i < currentIndex; i++)
	{
		context->IASetVertexBuffers(0, 1, &vertexBuffers[i], &strides, &offsets);
		context->IASetIndexBuffer(indexBuffers[i], DXGI_FORMAT_R16_UINT, 0);
		context->PSSetShaderResources(0, 1, &textureRVs[i]);
		context->PSSetSamplers(0, 1, &textureSamplers[i]);

		// update vertex shader with new info
		vsData.world = worldMatrices[i];
		D3D11_MAPPED_SUBRESOURCE vsSub;
		context->Map(vertexConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &vsSub);
		memcpy(vsSub.pData, &vsData, sizeof(vsData));
		context->Unmap(vertexConstantBuffer, NULL);

		context->DrawIndexed(numIndices[i], 0, 0);
	}
	/*
	// instancing stuff...
	for (int i = 0; i < currentInstanceIndex; i++)
	{
		context->IASetVertexBuffers(0, 1, &instanceVertexBuffers[i], &strides, &offsets);
		context->IASetIndexBuffer(instanceIndexBuffers[i], DXGI_FORMAT_R16_UINT, 0);
		context->PSSetShaderResources(0, 1, &instanceTextureRVs[i]);
		context->PSSetSamplers(0, 1, &instanceTextureSamplers[i]);

		vsData.world = XMMatrixIdentity();
		D3D11_MAPPED_SUBRESOURCE vsSub;
		context->Map(vertexConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &vsSub);
		memcpy(vsSub.pData, &vsData, sizeof(vsData));
		context->Unmap(vertexConstantBuffer, NULL);

		context->DrawIndexedInstanced(instanceNumIndices[i], instanceCount[i], 0, 0, 0);

	}
	*/
	context->DrawIndexedInstanced(numIndices[currentIndex - 1], 6, 0, 0, 0);

	swap->Present(0, 0);
	return true;
}


//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool DEMO_APP::ShutDown()
{
	swap->Release();
	device->Release();
	rtv->Release();
	context->Release();

	inputLayout->Release();
	vs->Release();
	ps->Release();
	depthStencil->Release();
	depthStencilView->Release();

	for (int i = 0; i < currentIndex; i++) //only loop to current index, otherwise out of bounds
	{
		vertexBuffers[i]->Release();
		indexBuffers[i]->Release();
		textureRVs[i]->Release();
		textureSamplers[i]->Release();
	}

	pixelConstantBuffer->Release();
	vertexConstantBuffer->Release();

	// release skybox stuff
	skyboxPS->Release();
	skyboxVS->Release();
	skyboxVBuffer->Release();
	skyboxIBuffer->Release();
	skyboxRV->Release();
	skyboxSampler->Release();
	skyboxConstantBuffer->Release();
//	rState->Release();

	// release instancing data
	for (int i = 0; i < currentInstanceIndex; i++) //only loop to current index, otherwise out of bounds
	{
		instanceVertexBuffers[i]->Release();
		instanceIndexBuffers[i]->Release();
		instanceTextureRVs[i]->Release();
		instanceTextureSamplers[i]->Release();
	}


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
	// speed modifier
	float speed = 6.0f;

	unsigned int inputs[] = { 'W', 'A', 'S', 'D', 'Q', 'E', VK_UP, VK_DOWN, VK_RBUTTON, VK_SHIFT};
	float activeKeys[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	float keyEffect[] = { 1, -1, -1, 1, 1, -1, -1, 1, 0, 3 };

	static POINT prev;
	if (prev.x == NULL)
	{
		prev.x = 0;
		prev.y = 0;
	}
	POINT cursorPos;
	
	GetPhysicalCursorPos(&cursorPos);
	float cursX = ((float)cursorPos.x - (float)prev.x) / 250.0f;
	float cursY = ((float)cursorPos.y - (float)prev.y) / -250.0f;

	// get active inputs
	for (int i = 0; i < ARRAYSIZE(inputs); i++)
	{
		if (GetAsyncKeyState(inputs[i]))
		{
			activeKeys[i] = 1;
		}
	}
	float time = timer.Delta();

	// change FOV for zooming
	currentFOV += (time * 10 * speed * ((activeKeys[6] * keyEffect[6]) + (activeKeys[7] * keyEffect[7])));
	if (currentFOV < minFOV)
		currentFOV = minFOV;
	if (currentFOV > maxFOV)
		currentFOV = maxFOV;

	XMMATRIX worldViewM = XMMatrixInverse(0, viewM);
	if (activeKeys[8])
	{
		//ShowCursor(false);
		XMMATRIX xSpin = XMMatrixRotationX(-cursY);
		XMMATRIX ySpin = XMMatrixRotationY(cursX);

		XMVECTOR temp = worldViewM.r[3];
		worldViewM.r[3] = XMLoadFloat4(&XMFLOAT4(0, 0, 0, 1));

		worldViewM = xSpin * worldViewM;
		worldViewM = worldViewM * ySpin;
		
		worldViewM.r[3] = temp;
		//ShowCursor(true);
	}
	// start camera movement
	XMFLOAT3 translation
	(
		speed * time * ((activeKeys[1] * keyEffect[1]) + (activeKeys[3] * keyEffect[3])),
		speed * time * ((activeKeys[5] * keyEffect[5]) + (activeKeys[4] * keyEffect[4])),
		speed * time * ((activeKeys[0] * keyEffect[0]) + (activeKeys[2] * keyEffect[2]))
	);
	if (activeKeys[9])
	{
		translation.x *= (activeKeys[9] * keyEffect[9]);
		translation.y *= (activeKeys[9] * keyEffect[9]);
		translation.z *= (activeKeys[9] * keyEffect[9]);
	}

	XMMATRIX translate = XMMatrixTranslation(translation.x, translation.y, translation.z);
	worldViewM = translate * worldViewM;
	viewM = XMMatrixInverse(0, worldViewM);

	// update skybox pos to camera pos
	XMVECTOR offset = worldViewM.r[3];
	/*
	XMFLOAT4 cameraPosition;
	XMStoreFloat4(&cameraPosition, offset);
	XMMATRIX skyboxT = XMMatrixTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);
	skyboxM = XMMatrixScaling((float)SKYBOX_SCALE, (float)SKYBOX_SCALE, (float)SKYBOX_SCALE) * skyboxT * XMMatrixIdentity();
	*/
	skyboxM.r[3] = offset;

	//update proj matrix with new FOV
	DXGI_SWAP_CHAIN_DESC current;
	swap->GetDesc(&current);
	
	projM = XMMatrixPerspectiveFovLH(XMConvertToRadians(currentFOV), (float)current.BufferDesc.Width / (float)current.BufferDesc.Height, 0.01f, 100.0f);

	prev = cursorPos;
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

	//D3D11_VIEWPORT vp;
	viewport.Width = float(current.BufferDesc.Width);
	viewport.Height = float(current.BufferDesc.Height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	context->RSSetViewports(1, &viewport);

	projM = XMMatrixPerspectiveFovLH(XMConvertToRadians(currentFOV), viewport.Width / viewport.Height, 0.1f, 100.0f);

	depthStencil->Release();
	depthStencilView->Release();

	// create new zbuffer
	D3D11_TEXTURE2D_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(depthDesc));
	depthDesc.Width = viewport.Width;
	depthDesc.Height = viewport.Height;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.CPUAccessFlags = NULL;
	depthDesc.MiscFlags = NULL;
	depthDesc.SampleDesc.Count = 4;
	depthDesc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
	device->CreateTexture2D(&depthDesc, NULL, &depthStencil);

	device->CreateDepthStencilView(depthStencil, 0, &depthStencilView);

	context->OMSetRenderTargets(1, &rtv, depthStencilView);

	return true;
}