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
#include "Multitexture_PS.csh"
#include "NormalMap_VS.csh"
#include "NormalMap_PS.csh"
#include "Master_VS.csh"
#include "Master_PS.csh"

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

	/////////////////////
	// GRAPHICS 2 CODE //
	/////////////////////

	ID3D11Device				*device;
	IDXGISwapChain				*swap;
	ID3D11RenderTargetView		*rtv;
	ID3D11DeviceContext			*context;
	D3D11_VIEWPORT				viewport;
	D3D11_VIEWPORT				viewport2;
	ID3D11InputLayout			*inputLayout;
	ID3D11VertexShader			*vs;
	ID3D11PixelShader			*ps;
	ID3D11Texture2D				*depthStencil;
	ID3D11DepthStencilView		*depthStencilView;


	// SKYBOX STUFF
	ID3D11PixelShader			*skyboxPS;
	ID3D11VertexShader			*skyboxVS;
	XMMATRIX					skyboxM;
	XMMATRIX					skyboxM2;
	ID3D11Buffer				*skyboxVBuffer;
	ID3D11Buffer				*skyboxIBuffer;
	ID3D11ShaderResourceView	*skyboxRV;
	ID3D11SamplerState			*skyboxSampler;
	ID3D11Buffer				*skyboxConstantBuffer;

	// WIP, used for more dyanmic updating
	struct MESH
	{
		bool						initialized = false;
		bool						normalMap = false;
		bool						specularMap = false;
		bool						multiTexture = false;

		bool						instanced = false;
		unsigned int				instanceCount;
		XMFLOAT4					instanceOffset;

		XMMATRIX					modifierMatrix = XMMatrixIdentity();
		XMMATRIX					localPos;
		MESH						*parentMesh = nullptr; //used for multiplying localPos

		ID3D11Buffer				*vBuffer;
		ID3D11Buffer				*iBuffer;
		unsigned int				numVertex;
		unsigned int				numIndex;

		ID3D11ShaderResourceView	*meshTexture[2];
		ID3D11ShaderResourceView	*meshNormalMap;
		ID3D11ShaderResourceView	*meshSpecularMap;
		ID3D11SamplerState			*meshSampler;

		XMMATRIX getTransformedMatrix()
		{
			if (parentMesh != nullptr)
			{
				return localPos * parentMesh->getTransformedMatrix();
			}
			else
			{
				return localPos;
			}
		}

		bool Shutdown()
		{
			if (!initialized)
				return false;
			vBuffer->Release();
			iBuffer->Release();
			meshTexture[0]->Release();
			meshSampler->Release();
			if (multiTexture)
				meshTexture[1]->Release();
			if (normalMap)
				meshNormalMap->Release();
			if (specularMap)
				meshSpecularMap->Release();

			return true;
		}
	};

	///////////////////
	// NEW MESH DATA //
	///////////////////
	ID3D11VertexShader			*masterVS;
	ID3D11PixelShader			*masterPS;
	ID3D11Buffer				*masterConstantBuffer;

	MESH						meshes[MESH_COUNT];
	unsigned int				meshIndex = 0;

	MESH						spaceMeshes[MESH_COUNT];
	unsigned int				spaceIndex = 0;

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
	XMMATRIX					instanceMatrices[INSTANCE_MESH_COUNT];

	///////////////////////
	// RENDER TO TEXTURE //
	///////////////////////
	D3D11_VIEWPORT				rtViewport;
	ID3D11RenderTargetView		*rtRTV;
	ID3D11ShaderResourceView	*rtSRV;
	ID3D11Texture2D				*renderTarget;

	ID3D11ShaderResourceView	*rtCubeSRV;
	ID3D11SamplerState			*rtCubeSampler;

	ID3D11Buffer				*rtVBuffer;
	ID3D11Buffer				*rtIBuffer;

	XMMATRIX					rtWorld;
	XMMATRIX					rtView;
	XMMATRIX					rtProj;

	/////////////////////
	// MULTI TEXTURING //
	/////////////////////
	ID3D11ShaderResourceView	*multiTextureRVs[2];
	ID3D11SamplerState			*multiSampler;
	ID3D11PixelShader			*multiPS;
	ID3D11Buffer				*mtVBuffer;
	ID3D11Buffer				*mtIBuffer;
	XMMATRIX					mtWorld;

	////////////////////
	// NORMAL MAPPING //
	////////////////////
	ID3D11ShaderResourceView	*normalTextureRVs[2];
	ID3D11SamplerState			*normalSampler;
	ID3D11VertexShader			*normalVS;
	ID3D11PixelShader			*normalPS;
	ID3D11Buffer				*normalVBuffer;
	ID3D11Buffer				*normalIBuffer;
	XMMATRIX					normalWorld;

	///////////////////////////
	// SPACE SCENE VARIABLES //
	///////////////////////////
	D3D11_VIEWPORT				spaceViewport;
	XMMATRIX					viewMSpace;
	XMMATRIX					projMSpace;
	XMMATRIX					spaceSkyboxM;

	bool						spaceActive = false; //TODO: make this true by default


	////////////////////
	// MISC VARIABLES //
	////////////////////
	XMMATRIX					viewM;
	XMMATRIX					viewM2;
	XMMATRIX					projM;

	XTime						timer;

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

	struct VS_MASTER_DATA
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
		XMFLOAT4 offset;
		XMFLOAT4 inmData;
		XMFLOAT4 camPos;
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
		XMFLOAT2 uv;
		XMFLOAT3 normal;
		XMFLOAT3 tangent;

		bool operator==(const SIMPLE_VERTEX that)
		{
			if (xyz.x == that.xyz.x && xyz.y == that.xyz.y && xyz.z == that.xyz.z)
			{
				if (uv.x == that.uv.x && uv.y == that.uv.y)
				{
					if (normal.x == that.normal.x && normal.y == that.normal.y && normal.z == that.normal.z)
					{
						if (tangent.x = that.tangent.x && tangent.y == that.tangent.y && that.tangent.z == tangent.z)
						{
							return true;
						}
					}
				}
			}
			return false;
		}
	};

	// BASIC MODEL CREATION FUNCTIONS //
	bool CalculateTangents(SIMPLE_VERTEX verts[], short indices[], unsigned int numVerts, unsigned int numInds);
	bool LoadBuffers(SIMPLE_VERTEX verts[], short indices[], unsigned int numVerts, unsigned int numInds, ID3D11Buffer **vertBuffer, ID3D11Buffer **indBuffer);
	bool LoadTexture(const wchar_t *texturePath, ID3D11ShaderResourceView **textureRV, ID3D11SamplerState **textureSampler);
	bool LoadMeshFromHeader(const OBJ_VERT verts[], const unsigned int indices[], unsigned int numVerts, unsigned int numInd, const wchar_t *texturePath);
	bool LoadOBJ(const char *filePath, const wchar_t *texturePath, ID3D11Buffer **vertBuffer, ID3D11Buffer **indBuffer, unsigned int *numberVerts, unsigned int *numberInds, ID3D11ShaderResourceView **textureRV, ID3D11SamplerState **textureSampler, bool updateIndex, unsigned int *index);
	bool CreateIndexedCube(float scale, const wchar_t *texturePath, ID3D11Buffer **vertBuffer, ID3D11Buffer **indBuffer, ID3D11ShaderResourceView **textureRV, ID3D11SamplerState **textureSampler, XMMATRIX *world, bool updateIndex, unsigned int *index);
	bool CreateInstancedCube(float scale, const wchar_t *texturePath, unsigned int count, XMFLOAT3 offset);

	// MESH CREATION FUNCTIONS //
	bool CreateMeshFromOBJ(MESH *meshArray, unsigned int meshInd, const char *filePath, const wchar_t *texturePath, MESH *parent);
	bool SetMeshMultitexture(MESH *mesh, bool multi, const wchar_t *texturePath);
	bool SetMeshNormalMap(MESH *mesh, bool normal, const wchar_t *texturePath);
	bool SetMeshSpecularMap(MESH *mesh, bool spec, const wchar_t *texturePath);
	bool SetMeshInstancing(MESH *mesh, bool instanced, XMFLOAT4 offset, unsigned int numInstances);

	// INITIALIZATION FUNCTIONS //
	bool InitializeTestMeshes();
	bool InitializeTestLights();

	bool InitializeSpaceMeshes();
	bool InitializeSpaceLights();

	// RENDER FUNCTIONS //
	bool RenderTestScene();
	bool RenderSpaceScene();
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

/////////////////////////////////////////////
// Calculate tangents for given mesh data. 
// Used for normal mapping.		
/////////////////////////////////////////////
bool DEMO_APP::CalculateTangents(SIMPLE_VERTEX verts[], short indices[], unsigned int numVerts, unsigned int numInds)
{
	vector<XMFLOAT3> tempTangents;
	XMFLOAT3 tempTangent;
	float tangentU1, tangentU2, tangentV1, tangentV2;

	XMVECTOR edge1, edge2;
	float edgeX, edgeY, edgeZ;

	for (int i = 0; i < (numInds / 3); i++)
	{
		// calculate position edges for tangents
		edgeX = verts[indices[i * 3]].xyz.x - verts[indices[(i * 3) + 2]].xyz.x;
		edgeY = verts[indices[i * 3]].xyz.y - verts[indices[(i * 3) + 2]].xyz.y;
		edgeZ = verts[indices[i * 3]].xyz.z - verts[indices[(i * 3) + 2]].xyz.z;
		edge1 = XMVectorSet(edgeX, edgeY, edgeZ, 0);

		edgeX = verts[indices[i * 3]].xyz.x - verts[indices[(i * 3) + 1]].xyz.x;
		edgeY = verts[indices[i * 3]].xyz.y - verts[indices[(i * 3) + 1]].xyz.y;
		edgeZ = verts[indices[i * 3]].xyz.z - verts[indices[(i * 3) + 1]].xyz.z;
		edge2 = XMVectorSet(edgeX, edgeY, edgeZ, 0);

		// calculate texture coordinates for edges
		tangentU1 = verts[indices[i * 3]].uv.x - verts[indices[(i * 3) + 2]].uv.x;
		tangentV1 = verts[indices[i * 3]].uv.y - verts[indices[(i * 3) + 2]].uv.y;

		tangentU2 = verts[indices[i * 3]].uv.x - verts[indices[(i * 3) + 1]].uv.x;
		tangentV2 = verts[indices[i * 3]].uv.y - verts[indices[(i * 3) + 1]].uv.y;

		// find the tangent
		tempTangent.x = (tangentV1 * XMVectorGetX(edge1) - tangentV2 * XMVectorGetX(edge2)) * (1.0f / (tangentU1 * tangentV2 - tangentU2 * tangentV1));
		tempTangent.y = (tangentV1 * XMVectorGetY(edge1) - tangentV2 * XMVectorGetY(edge2)) * (1.0f / (tangentU1 * tangentV2 - tangentU2 * tangentV1));
		tempTangent.z = (tangentV1 * XMVectorGetZ(edge1) - tangentV2 * XMVectorGetZ(edge2)) * (1.0f / (tangentU1 * tangentV2 - tangentU2 * tangentV1));

		tempTangents.push_back(tempTangent);
	}

	float tangentX, tangentY, tangentZ;
	for (int i = 0; i < numVerts; i++)
	{
		XMVECTOR tangentSum = XMVectorSet(0, 0, 0, 0);
		int facesUsed = 0;
		for (int v = 0; (v < numInds / 3); v++)
		{
			if (indices[v * 3] == i || indices[(v * 3) + 1] == i || indices[(v * 3) + 2] == i)
			{
				tangentX = XMVectorGetX(tangentSum) + tempTangents[v].x;
				tangentY = XMVectorGetY(tangentSum) + tempTangents[v].y;
				tangentZ = XMVectorGetZ(tangentSum) + tempTangents[v].z;

				tangentSum = XMVectorSet(tangentX, tangentY, tangentZ, 0);
				facesUsed++;
			}
		}

		tangentSum = tangentSum / (float)facesUsed;
		tangentSum = XMVector3Normalize(tangentSum);

		XMFLOAT3 newTangent;
		newTangent.x = XMVectorGetX(tangentSum);
		newTangent.y = XMVectorGetY(tangentSum);
		newTangent.z = XMVectorGetZ(tangentSum);

		verts[i].tangent = newTangent;
	}

	return true;
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
// Option to update index.
// (for array systems for regular models / instanced models.)
//////////////////////////////////////////////////////////////////////
bool DEMO_APP::LoadOBJ(const char *filePath, const wchar_t *texturePath, ID3D11Buffer **vertBuffer, ID3D11Buffer **indBuffer, unsigned int *numberVerts, unsigned int *numberInds, ID3D11ShaderResourceView **textureRV, ID3D11SamplerState **textureSampler, bool updateIndex = false, unsigned int *index = nullptr)
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

	CalculateTangents(meshVerts, meshIndices, vertIndices.size(), vertIndices.size());

	LoadBuffers(meshVerts, meshIndices, vertIndices.size(), vertIndices.size(), vertBuffer, indBuffer);


	// load the model's texture
	LoadTexture(texturePath, textureRV, textureSampler);

	*numberVerts = vertIndices.size();
	*numberInds = vertIndices.size();

	if (updateIndex)
	{
		//update number arrays
		//numVertices[currentIndex] = normals.size();
		//numIndices[currentIndex] = vertIndices.size();
		*index += 1;
	}
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

////////////////////////////////////////////////////////////////
// Creates an indexed cube in the buffer arrays.
// Option to update index.
// (for array systems for regular models / instanced models.)
////////////////////////////////////////////////////////////////
bool DEMO_APP::CreateIndexedCube(float scale, const wchar_t *texturePath, ID3D11Buffer **vertBuffer, ID3D11Buffer **indBuffer, ID3D11ShaderResourceView **textureRV, ID3D11SamplerState **textureSampler, XMMATRIX *world, bool updateIndex = false, unsigned int *index = nullptr)
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

	CalculateTangents(cube, cubeInd, ARRAYSIZE(cube), ARRAYSIZE(cubeInd));

	LoadBuffers(cube, cubeInd, ARRAYSIZE(cube), ARRAYSIZE(cubeInd), vertBuffer, indBuffer);

	// load the model's texture
	LoadTexture(texturePath, textureRV, textureSampler);

	// create worldMatrix
	//worldMatrices[currentIndex] = XMMatrixScaling(scale, scale, scale) * XMMatrixIdentity();
	*world = XMMatrixScaling(scale, scale, scale) * (*world);
	
	// update currentIndex before exiting
	//currentIndex += 1;
	if (updateIndex)
	{
		//update number arrays
		numVertices[currentIndex] = ARRAYSIZE(cube);
		numIndices[currentIndex] = ARRAYSIZE(cubeInd);
		*index += 1;
	}
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

	instanceNumVertices[currentInstanceIndex] = ARRAYSIZE(cube);
	instanceNumIndices[currentInstanceIndex] = ARRAYSIZE(cubeInd);

	LoadBuffers(cube, cubeInd, ARRAYSIZE(cube), ARRAYSIZE(cubeInd), &instanceVertexBuffers[currentInstanceIndex], &instanceIndexBuffers[currentInstanceIndex]);

	LoadTexture(texturePath, &instanceTextureRVs[currentInstanceIndex], &instanceTextureSamplers[currentInstanceIndex]);

	instanceCount[currentInstanceIndex] = count;
	currentInstanceIndex++;

	return true;
}

///////////////////////////////////////////////////////////////////
// Create a MESH from OBJ file and insert into passed meshArray. 
///////////////////////////////////////////////////////////////////
bool DEMO_APP::CreateMeshFromOBJ(MESH meshArray[], unsigned int meshInd, const char *filePath, const wchar_t *texturePath, MESH *parent = nullptr)
{
	if (parent != nullptr)
	{
		meshArray[meshInd].parentMesh = parent;
	}
	LoadOBJ(filePath, texturePath, &meshArray[meshInd].vBuffer, &meshArray[meshInd].iBuffer, &meshArray[meshInd].numVertex, &meshArray[meshInd].numIndex, &meshArray[meshInd].meshTexture[0], &meshArray[meshInd].meshSampler);
	meshArray[meshInd].initialized = true;
	return true;
}

///////////////////////////////////
// Set passed MESH multitexture. 
///////////////////////////////////
bool DEMO_APP::SetMeshMultitexture(MESH *mesh, bool multi, const wchar_t *texturePath = nullptr)
{
	if (mesh->multiTexture)
	{
		mesh->meshTexture[1]->Release();
	}
	mesh->multiTexture = multi;
	if (multi)
		CreateDDSTextureFromFile(device, texturePath, nullptr, &mesh->meshTexture[1]);
	return true;
}

/////////////////////////////////
// Set passed MESH normal map.  
/////////////////////////////////
bool DEMO_APP::SetMeshNormalMap(MESH *mesh, bool normal, const wchar_t *texturePath = nullptr)
{
	if (mesh->normalMap)
		mesh->meshNormalMap->Release();
	mesh->normalMap = normal;
	if (normal)
		CreateDDSTextureFromFile(device, texturePath, nullptr, &mesh->meshNormalMap);
	return true;
}

///////////////////////////////////
// Set passed MESH specular map. 
///////////////////////////////////
bool DEMO_APP::SetMeshSpecularMap(MESH *mesh, bool spec, const wchar_t *texturePath = nullptr)
{
	if (mesh->specularMap)
		mesh->meshSpecularMap->Release();
	mesh->specularMap = spec;
	if (spec)
		CreateDDSTextureFromFile(device, texturePath, nullptr, &mesh->meshSpecularMap);
	return true;
}

/////////////////////////////////
// Set passed MESH instancing. 
/////////////////////////////////
bool DEMO_APP::SetMeshInstancing(MESH *mesh, bool instanced, XMFLOAT4 offset = XMFLOAT4(0, 0, 0, 0), unsigned int numInstances = 0)
{
	mesh->instanced = instanced;
	mesh->instanceCount = numInstances;
	mesh->instanceOffset = offset;

	return true;
}

////////////////////////////////////////////////////
// Initializes meshes for original testing scene. 
////////////////////////////////////////////////////
bool DEMO_APP::InitializeTestMeshes()
{
	
	worldMatrices[currentIndex] = XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixIdentity() * 	XMMatrixTranslation(4.0f, 0.0f, 0.0f);
	//LoadMeshFromHeader(Barrel_data, Barrel_indicies, ARRAYSIZE(Barrel_data), ARRAYSIZE(Barrel_indicies), L"barrel.dds");
	LoadOBJ("Barrel.obj", L"barrel.dds", &vertexBuffers[currentIndex], &indexBuffers[currentIndex], &numVertices[currentIndex], &numIndices[currentIndex], &textureRVs[currentIndex], &textureSamplers[currentIndex], true, &currentIndex);
	worldMatrices[currentIndex] = XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixIdentity() * 	XMMatrixTranslation(3.0f, 2.0f, 0.0f);
	LoadOBJ("Barrel.obj", L"barrel.dds", &vertexBuffers[currentIndex], &indexBuffers[currentIndex], &numVertices[currentIndex], &numIndices[currentIndex], &textureRVs[currentIndex], &textureSamplers[currentIndex], true, &currentIndex);
	worldMatrices[currentIndex] = XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixIdentity() * 	XMMatrixTranslation(5.0f, 2.0f, 0.0f);
	LoadOBJ("Barrel.obj", L"barrel.dds", &vertexBuffers[currentIndex], &indexBuffers[currentIndex], &numVertices[currentIndex], &numIndices[currentIndex], &textureRVs[currentIndex], &textureSamplers[currentIndex], true, &currentIndex);
	worldMatrices[currentIndex] = XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixIdentity() * 	XMMatrixTranslation(4.0f, 4.0f, 0.0f);
	LoadOBJ("Barrel.obj", L"barrel.dds", &vertexBuffers[currentIndex], &indexBuffers[currentIndex], &numVertices[currentIndex], &numIndices[currentIndex], &textureRVs[currentIndex], &textureSamplers[currentIndex], true, &currentIndex);
	


	worldMatrices[currentIndex] = XMMatrixScaling(15.0f, 2.0f, 15.0f) * XMMatrixIdentity() * XMMatrixTranslation(0.0f, 9.0f, 0.0f);
	LoadMeshFromHeader(test_pyramid_data, test_pyramid_indicies, ARRAYSIZE(test_pyramid_data), ARRAYSIZE(test_pyramid_indicies), L"barrel.dds");

	worldMatrices[currentIndex] = XMMatrixScaling(0.3f, 0.3f, 0.3f) * XMMatrixIdentity() * XMMatrixTranslation(-5.0f, 0.0f, 0.0f);
	bool result = LoadOBJ("penguin.obj", L"peng.dds", &vertexBuffers[currentIndex], &indexBuffers[currentIndex], &numVertices[currentIndex], &numIndices[currentIndex], &textureRVs[currentIndex], &textureSamplers[currentIndex], true, &currentIndex);
	//worldMatrices[currentIndex] = XMMatrixScaling(0.3f, 0.3f, 0.3f) * XMMatrixIdentity() * XMMatrixTranslation(-7.0f, 0.0f, 0.0f);
	//LoadMeshFromHeader(penguin_data, penguin_indicies, ARRAYSIZE(penguin_data), ARRAYSIZE(penguin_indicies), L"peng.dds");

	instanceMatrices[currentInstanceIndex] = XMMatrixIdentity() * XMMatrixTranslation(0.0f, 3.0f, 0.0f);
	CreateInstancedCube(0.5f, L"crate1_diffuse.dds", 2500, XMFLOAT3(0, 0, 2));


	// render to texture take 2
	rtViewport.Width = 512;
	rtViewport.Height = 512;
	rtViewport.MinDepth = 0.0f;
	rtViewport.MaxDepth = 1.0f;
	rtViewport.TopLeftX = 0;
	rtViewport.TopLeftY = 0;

	D3D11_TEXTURE2D_DESC rtDesc;
	ZeroMemory(&rtDesc, sizeof(rtDesc));
	rtDesc.Width = 512;
	rtDesc.Height = 512;
	rtDesc.MipLevels = 1;
	rtDesc.ArraySize = 1;
	rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtDesc.SampleDesc.Count = 1;
	rtDesc.Usage = D3D11_USAGE_DEFAULT;
	rtDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	rtDesc.CPUAccessFlags = 0;
	rtDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	device->CreateTexture2D(&rtDesc, NULL, &renderTarget);

	D3D11_RENDER_TARGET_VIEW_DESC rtRTVDesc;
	rtRTVDesc.Format = rtDesc.Format;
	rtRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtRTVDesc.Texture2D.MipSlice = 0;
	device->CreateRenderTargetView(renderTarget, &rtRTVDesc, &rtRTV);

	D3D11_SHADER_RESOURCE_VIEW_DESC rtSRVDesc;
	rtSRVDesc.Format = rtDesc.Format;
	rtSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	rtSRVDesc.Texture2D.MostDetailedMip = 0;
	rtSRVDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(renderTarget, &rtSRVDesc, &rtSRV);

	rtWorld = XMMatrixIdentity() * XMMatrixTranslation(2, 0, 0);
	rtView = viewM;
	rtProj = XMMatrixPerspectiveFovLH(XMConvertToRadians(currentFOV), 512 / (FLOAT)512, 0.01f, 100.0f);

	CreateIndexedCube(1.0f, L"crate1_diffuse.dds", &rtVBuffer, &rtIBuffer, &rtCubeSRV, &rtCubeSampler, &rtWorld);


	// MULTI TEX CUBE //
	mtWorld = XMMatrixIdentity() * XMMatrixTranslation(4, 3.5f, 2);
	CreateIndexedCube(1.0f, L"stoneMultiTex.dds", &mtVBuffer, &mtIBuffer, &multiTextureRVs[0], &multiSampler, &mtWorld);
	CreateDDSTextureFromFile(device, L"dirtMultiTex.dds", nullptr, &multiTextureRVs[1]);

	// NORMAL MAP CUBE //

	normalWorld = XMMatrixIdentity() * XMMatrixTranslation(4, 3.5f, -2);
	CreateIndexedCube(1.0f, L"stoneMultiTex.dds", &normalVBuffer, &normalIBuffer, &normalTextureRVs[0], &normalSampler, &normalWorld);
	CreateDDSTextureFromFile(device, L"stoneNormal.dds", nullptr, &normalTextureRVs[1]);

	//////////////////////
	// NEW MESH TESTING //
	//meshes[meshIndex].modifierMatrix = XMMatrixIdentity() * XMMatrixRotationY(XMConvertToRadians(1.0f));
	
	meshes[meshIndex].localPos = XMMatrixIdentity() * XMMatrixTranslation(0, 1, -6);
	CreateIndexedCube(2.0f, L"stoneMultiTex.dds", &meshes[meshIndex].vBuffer, &meshes[meshIndex].iBuffer, &meshes[meshIndex].meshTexture[0], &meshes[meshIndex].meshSampler, &meshes[meshIndex].localPos);
	meshes[meshIndex].numVertex = 24;
	meshes[meshIndex].numIndex = 36;
	//SetMeshMultitexture(&meshes[meshIndex], true, L"dirtMultiTex.dds");
	SetMeshNormalMap(&meshes[meshIndex], true, L"stoneNormal.dds");
	SetMeshSpecularMap(&meshes[meshIndex], true, L"stoneSpecTEST.dds");
	//SetMeshInstancing(&meshes[meshIndex], true, XMFLOAT4(0, 0, -3.5f, 0), 10);

	meshes[meshIndex].initialized = true;
	meshIndex++;


	meshes[meshIndex].modifierMatrix = XMMatrixIdentity() * XMMatrixRotationY(XMConvertToRadians(-1.0f));
	meshes[meshIndex].localPos = XMMatrixIdentity() * XMMatrixTranslation(1, 0, 0);
	CreateIndexedCube(0.5f, L"stoneMultiTex.dds", &meshes[meshIndex].vBuffer, &meshes[meshIndex].iBuffer, &meshes[meshIndex].meshTexture[0], &meshes[meshIndex].meshSampler, &meshes[meshIndex].localPos);
	meshes[meshIndex].numVertex = 24;
	meshes[meshIndex].numIndex = 36;
	SetMeshMultitexture(&meshes[meshIndex], true, L"dirtMultiTex.dds");
	SetMeshNormalMap(&meshes[meshIndex], true, L"stoneNormal.dds");
	//meshes[meshIndex].parentMesh = &meshes[0];

	meshes[meshIndex].initialized = true;	
	meshIndex++;

	
	meshes[meshIndex].localPos = XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixIdentity() * XMMatrixTranslation(13.0f, 1.0f, -2.0f);
	CreateMeshFromOBJ(meshes, meshIndex, "spacestation.obj", L"spacestation_diffuse.dds");
	SetMeshSpecularMap(&meshes[meshIndex], true, L"spacestation_specular.dds");
	meshIndex++;	
	


	
	
	meshes[meshIndex].localPos = XMMatrixScaling(0.2f, 0.2f, 0.2f) * XMMatrixIdentity() * XMMatrixTranslation(-1.0f, 1.0f, -3.0f);
	CreateMeshFromOBJ(meshes, meshIndex, "asteroid.obj", L"asteroid_diffuse.dds");
	SetMeshSpecularMap(&meshes[meshIndex], true, L"asteroid_specular.dds");
	SetMeshNormalMap(&meshes[meshIndex], true, L"asteroid_normal.dds");
	meshIndex++;
	


	
	

	return true;
}

///////////////////////////////////////////////////
// Initialize lights for original testing scene. 
///////////////////////////////////////////////////
bool DEMO_APP::InitializeTestLights()
{
	psData.directional[0].lightDirection = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	psData.directional[0].lightColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);

	psData.point[0].lightColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	psData.point[0].lightPos = XMFLOAT4(-0.55f, 0.0f, -3.5f, 1.0f);
	psData.point[0].lightRad = 2.5f;

	psData.spot[0].lightDirection = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	psData.spot[0].lightColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	psData.spot[0].lightPos = XMFLOAT4(-1.5f, 3.0f, 0.0f, 1.0f);
	psData.spot[0].lightRad = 100.0f;
	psData.spot[0].outerConeRatio = 0.96f;
	psData.spot[0].innerConeRatio = 0.98f;

	//psData.directional[0].lightColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	//psData.point[0].lightColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	//psData.spot[0].lightColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	return true;
}


bool DEMO_APP::RenderTestScene()
{
	double time = timer.TotalTime();
	UINT strides = sizeof(SIMPLE_VERTEX);
	UINT offsets = 0;

	D3D11_MAPPED_SUBRESOURCE vsSub;
	D3D11_MAPPED_SUBRESOURCE psSub;

	SKYBOX_VS_DATA skyVSData;
	skyVSData.proj = projM;

	VS_BUFFER_DATA vsData;
	vsData.proj = projM;

	VS_MASTER_DATA masterData;
	masterData.proj = projM;

	XMMATRIX spinM = XMMatrixRotationX(-timer.Delta() * 0.1f);
	XMMATRIX dirSpin = XMMatrixRotationY(-timer.Delta());
	XMMATRIX orbitM = XMMatrixRotationY(-time * 2.0f);
	XMMATRIX translateM = XMMatrixTranslation(-1.5f, 3.0f, 0.0f);

	XMVECTOR scale, rot, trans;
	XMVECTOR axis;
	float angle;

	///////////////////////////////////////////////
	// RENDER TO TEXTURE STUFF BEFORE MAIN SCENE //
	///////////////////////////////////////////////
	context->VSSetConstantBuffers(0, 1, &vertexConstantBuffer);
	context->PSSetConstantBuffers(0, 1, &pixelConstantBuffer);
	context->VSSetShader(vs, 0, 0);
	context->PSSetShader(ps, 0, 0);

	context->RSSetViewports(1, &rtViewport);
	context->OMSetRenderTargets(1, &rtRTV, NULL);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	FLOAT white[4] = { 1.0, 1.0f, 1.0f, 1.0f };
	context->ClearRenderTargetView(rtRTV, white);

	context->IASetVertexBuffers(0, 1, &rtVBuffer, &strides, &offsets);
	context->IASetIndexBuffer(rtIBuffer, DXGI_FORMAT_R16_UINT, 0);
	context->PSSetShaderResources(0, 1, &rtCubeSRV);
	context->PSSetSamplers(0, 1, &rtCubeSampler);

	VS_BUFFER_DATA vData;
	vData.world = XMMatrixTranslation(0, 1.0f, 0) * orbitM;
	vData.view = rtView;
	vData.proj = rtProj;
	context->Map(vertexConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &vsSub);
	memcpy(vsSub.pData, &vData, sizeof(vData));
	context->Unmap(vertexConstantBuffer, NULL);

	context->DrawIndexed(36, 0, 0);
	///////////////////////////
	// END RENDER TO TEXTURE //
	///////////////////////////

	///////////////////
	// LIGHT UPDATES //
	///////////////////
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
	if (psData.point[0].lightPos.y > 2.0f)
	{
		pLightMove = -1;
	}
	if (psData.point[0].lightPos.y < 0.0f)
	{
		pLightMove = 1;
	}
	psData.point[0].lightPos.y += timer.Delta() * pLightMove * 0.5f;
	///////////////////////
	// END LIGHT UPDATES //
	///////////////////////

	// MAIN SCENE 
	context->OMSetRenderTargets(1, &rtv, depthStencilView);
	for (int i = 0; i <= 1; i++)
	{
		if (i == 0)
		{
			context->RSSetViewports(1, &viewport);

			vsData.view = viewM;

			XMVECTOR camPos = skyboxM.r[3];
			XMFLOAT3 camPosF;
			XMStoreFloat3(&camPosF, camPos);
			skyVSData.cameraPos = camPosF;

			skyVSData.world = skyboxM;
			skyVSData.view = viewM;

			masterData.view = viewM;
			// set camera pos for specular data
			XMVECTOR currentCamPos = XMMatrixInverse(0, viewM).r[3];
			XMStoreFloat4(&masterData.camPos, currentCamPos);
		}
		else
		{
			context->RSSetViewports(1, &viewport2);

			vsData.view = viewM2;

			XMVECTOR camPos = skyboxM.r[3];
			XMFLOAT3 camPosF;
			XMStoreFloat3(&camPosF, camPos);
			skyVSData.cameraPos = camPosF;

			skyVSData.world = skyboxM2;
			skyVSData.view = viewM2;

			masterData.view = viewM2;
			// set camera pos for specular data
			XMVECTOR currentCamPos = XMMatrixInverse(0, viewM2).r[3];
			XMStoreFloat4(&masterData.camPos, currentCamPos);
		}

		// SKYBOX FIRST
		context->VSSetShader(skyboxVS, 0, 0);
		context->PSSetShader(skyboxPS, 0, 0);
		context->VSSetConstantBuffers(0, 1, &skyboxConstantBuffer);
		context->IASetVertexBuffers(0, 1, &skyboxVBuffer, &strides, &offsets);
		context->IASetIndexBuffer(skyboxIBuffer, DXGI_FORMAT_R16_UINT, 0);
		context->PSSetShaderResources(0, 1, &skyboxRV);
		context->PSSetSamplers(0, 1, &skyboxSampler);

		context->Map(skyboxConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &vsSub);
		memcpy(vsSub.pData, &skyVSData, sizeof(skyVSData));
		context->Unmap(skyboxConstantBuffer, NULL);

		context->DrawIndexed(36, 0, 0);

		// RE-CLEAR DEPTH BUFFER
		context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

		// RESET SHADERS 
		context->VSSetConstantBuffers(0, 1, &vertexConstantBuffer);
		context->PSSetConstantBuffers(0, 1, &pixelConstantBuffer);
		context->VSSetShader(vs, 0, 0);
		context->PSSetShader(ps, 0, 0);

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

			context->Map(vertexConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &vsSub);
			memcpy(vsSub.pData, &vsData, sizeof(vsData));
			context->Unmap(vertexConstantBuffer, NULL);

			context->DrawIndexed(numIndices[i], 0, 0);
		}

		// instancing stuff...
		for (int i = 0; i < currentInstanceIndex; i++)
		{
			context->IASetVertexBuffers(0, 1, &instanceVertexBuffers[i], &strides, &offsets);
			context->IASetIndexBuffer(instanceIndexBuffers[i], DXGI_FORMAT_R16_UINT, 0);
			context->PSSetShaderResources(0, 1, &instanceTextureRVs[i]);
			context->PSSetSamplers(0, 1, &instanceTextureSamplers[i]);

			vsData.world = instanceMatrices[i];

			context->Map(vertexConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &vsSub);
			memcpy(vsSub.pData, &vsData, sizeof(vsData));
			context->Unmap(vertexConstantBuffer, NULL);

			context->DrawIndexedInstanced(instanceNumIndices[i], instanceCount[i], 0, 0, 0);

		}
		// draw last cube that uses RTT
		context->IASetVertexBuffers(0, 1, &rtVBuffer, &strides, &offsets);
		context->IASetIndexBuffer(rtIBuffer, DXGI_FORMAT_R16_UINT, 0);
		context->PSSetShaderResources(0, 1, &rtSRV);
		context->PSSetSamplers(0, 1, &rtCubeSampler);

		vsData.world = rtWorld;

		context->Map(vertexConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &vsSub);
		memcpy(vsSub.pData, &vsData, sizeof(vsData));
		context->Unmap(vertexConstantBuffer, NULL);

		context->DrawIndexed(36, 0, 0);

		// MULTI TEXTURE TEST //
		context->PSSetShader(multiPS, 0, 0);
		context->IASetVertexBuffers(0, 1, &mtVBuffer, &strides, &offsets);
		context->IASetIndexBuffer(mtIBuffer, DXGI_FORMAT_R16_UINT, 0);
		context->PSSetShaderResources(0, 2, multiTextureRVs);
		context->PSSetSamplers(0, 1, &multiSampler);

		vsData.world = mtWorld;
		context->Map(vertexConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &vsSub);
		memcpy(vsSub.pData, &vsData, sizeof(vsData));
		context->Unmap(vertexConstantBuffer, NULL);

		context->DrawIndexed(36, 0, 0);
		// END MULTI TEXTURE //

		// NORMAL MAPPING TEST //
		context->VSSetShader(normalVS, 0, 0);
		context->PSSetShader(normalPS, 0, 0);
		context->IASetVertexBuffers(0, 1, &normalVBuffer, &strides, &offsets);
		context->IASetIndexBuffer(normalIBuffer, DXGI_FORMAT_R16_UINT, 0);
		context->PSSetShaderResources(0, 2, normalTextureRVs);
		context->PSSetSamplers(0, 1, &normalSampler);

		vsData.world = normalWorld;
		context->Map(vertexConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &vsSub);
		memcpy(vsSub.pData, &vsData, sizeof(vsData));
		context->Unmap(vertexConstantBuffer, NULL);

		context->DrawIndexed(36, 0, 0);
		// END NORMAL MAPPING //

		context->VSSetShader(masterVS, 0, 0);
		context->PSSetShader(masterPS, 0, 0);
		context->VSSetConstantBuffers(0, 1, &masterConstantBuffer);
		for (int i = 0; i < meshIndex; i++)
		{
			context->IASetVertexBuffers(0, 1, &meshes[i].vBuffer, &strides, &offsets);
			context->IASetIndexBuffer(meshes[i].iBuffer, DXGI_FORMAT_R16_UINT, 0);
			if (meshes[i].multiTexture)
			{
				context->PSSetShaderResources(0, 2, meshes[i].meshTexture);
			}
			else
			{
				context->PSSetShaderResources(0, 1, &meshes[i].meshTexture[0]);
			}
			if (meshes[i].normalMap)
				context->PSSetShaderResources(2, 1, &meshes[i].meshNormalMap);
			if (meshes[i].specularMap)
				context->PSSetShaderResources(3, 1, &meshes[i].meshSpecularMap);

			context->PSSetSamplers(0, 1, &meshes[i].meshSampler);

			meshes[i].localPos = (meshes[i].modifierMatrix) * meshes[i].localPos;
			XMMatrixDecompose(&scale, &rot, &trans, meshes[i].modifierMatrix);
			XMQuaternionToAxisAngle(&axis, &angle, rot);
			XMMATRIX translat = XMMatrixTranslationFromVector(trans);
			meshes[i].localPos = (XMMatrixRotationQuaternion(timer.Delta() * rot) * meshes[i].localPos);
			masterData.world = meshes[i].getTransformedMatrix();
			//masterData.world = meshes[i].localPos;

			masterData.offset = meshes[i].instanceOffset;
			masterData.inmData = XMFLOAT4((float)meshes[i].instanced, (float)meshes[i].normalMap, (float)meshes[i].multiTexture, (float)meshes[i].specularMap);

			context->Map(masterConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &vsSub);
			memcpy(vsSub.pData, &masterData, sizeof(masterData));
			context->Unmap(masterConstantBuffer, NULL);

			if (meshes[i].instanced)
			{
				context->DrawIndexedInstanced(meshes[i].numIndex, meshes[i].instanceCount, 0, 0, 0);
			}
			else
			{
				context->DrawIndexed(meshes[i].numIndex, 0, 0);
			}

		}
	}

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
	// SET VIEWPORT 1
	viewport.Width = BACKBUFFER_WIDTH*0.5f;
	viewport.Height = BACKBUFFER_HEIGHT;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	// SET VIEWPORT 2
	viewport2.Width = BACKBUFFER_WIDTH*0.5f;
	viewport2.Height = BACKBUFFER_HEIGHT;
	viewport2.MinDepth = 0.0f;
	viewport2.MaxDepth = 1.0f;
	viewport2.TopLeftX = BACKBUFFER_WIDTH*0.5f;
	viewport2.TopLeftY = 0;
	
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

	device->CreateVertexShader(Skybox_VS, sizeof(Skybox_VS), NULL, &skyboxVS);
	device->CreatePixelShader(Skybox_PS, sizeof(Skybox_PS), NULL, &skyboxPS);
	
	device->CreatePixelShader(Multitexture_PS, sizeof(Multitexture_PS), NULL, &multiPS);

	device->CreateVertexShader(NormalMap_VS, sizeof(NormalMap_VS), NULL, &normalVS);
	device->CreatePixelShader(NormalMap_PS, sizeof(NormalMap_PS), NULL, &normalPS);

	device->CreateVertexShader(Master_VS, sizeof(Master_VS), NULL, &masterVS);
	device->CreatePixelShader(Master_PS, sizeof(Master_PS), NULL, &masterPS);

	// CREATE INPUT LAYOUT
	D3D11_INPUT_ELEMENT_DESC vLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SV_INSTANCEID", 0, DXGI_FORMAT_R32_UINT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
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

	// MASTER CONSTANT BUFFER
	cbDesc.ByteWidth = sizeof(VS_MASTER_DATA);
	device->CreateBuffer(&cbDesc, NULL, &masterConstantBuffer);

	// SKYBOX
	skyboxM = XMMatrixIdentity();
	skyboxM2 = XMMatrixIdentity() * XMMatrixScaling((float)SKYBOX_SCALE, (float)SKYBOX_SCALE, (float)SKYBOX_SCALE);
	CreateIndexedCube((float)SKYBOX_SCALE, L"skyBox.dds", &skyboxVBuffer, &skyboxIBuffer, &skyboxRV, &skyboxSampler, &skyboxM);

	cbDesc.ByteWidth = sizeof(SKYBOX_VS_DATA);
	device->CreateBuffer(&cbDesc, NULL, &skyboxConstantBuffer);

	// MATRICES
	XMVECTOR eye = XMVectorSet(0.0f, 2.0f, -3.0f, 0.0f);
	XMVECTOR at = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	viewM = XMMatrixLookAtLH(eye, at, up);

	eye = XMVectorSet(0.0f, 2.0f, 7.0f, 0.0f);
	viewM2 = XMMatrixLookAtLH(eye, at, up);

	projM = XMMatrixPerspectiveFovLH(XMConvertToRadians(currentFOV), BACKBUFFER_WIDTH*0.5f / (FLOAT)BACKBUFFER_HEIGHT, 0.01f, 100.0f);

	// SPACE MATRICES
	eye = XMVectorSet(0.0f, 5.0f, -5.0f, 0.0f);
	at = XMVectorSet(0, 0, 0, 0);
	viewMSpace = XMMatrixLookAtLH(eye, at, up);

	projMSpace = XMMatrixPerspectiveFovLH(XMConvertToRadians(currentFOV), BACKBUFFER_WIDTH / (FLOAT)BACKBUFFER_HEIGHT, 0.01f, 100.0f);

	spaceSkyboxM = XMMatrixIdentity();
	
	// SPACE VIEWPORT
	spaceViewport.Width = BACKBUFFER_WIDTH;
	spaceViewport.Height = BACKBUFFER_HEIGHT;
	spaceViewport.MinDepth = 0.0f;
	spaceViewport.MaxDepth = 1.0f;
	spaceViewport.TopLeftX = 0;
	spaceViewport.TopLeftY = 0;

	// MESHES & LIGHTS
	InitializeTestLights();
	InitializeTestMeshes();
}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
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
	if (spaceActive)
	{

	}
	else
	{
		RenderTestScene();
	}
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

	// release RTT data
	rtRTV->Release();
	rtSRV->Release();
	rtIBuffer->Release();
	rtVBuffer->Release();
	renderTarget->Release();
	rtCubeSampler->Release();
	rtCubeSRV->Release();

	// release MT data
	multiPS->Release();
	multiSampler->Release();
	multiTextureRVs[0]->Release();
	multiTextureRVs[1]->Release();
	mtIBuffer->Release();
	mtVBuffer->Release();

	// release Normal Mapping data
	normalVS->Release();
	normalPS->Release();
	normalTextureRVs[0]->Release();
	normalTextureRVs[1]->Release();
	normalSampler->Release();
	normalIBuffer->Release();
	normalVBuffer->Release();

	// release MESH data
	for (int i = 0; i < meshIndex; i++)
	{
		meshes[i].Shutdown();
	}
	masterVS->Release();
	masterPS->Release();
	masterConstantBuffer->Release();
	

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

	// catch input for swapping viewports
	if (GetAsyncKeyState('1'))
	{
		spaceActive = true;
	}
	if (GetAsyncKeyState('2'))
	{
		spaceActive = false;
	}

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
	XMMATRIX worldViewM2 = XMMatrixInverse(0, viewM2);
	XMMATRIX worldSpaceView = XMMatrixInverse(0, viewMSpace);
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

		// new second viewport
		XMVECTOR temp2 = worldViewM2.r[3];
		worldViewM2.r[3] = XMLoadFloat4(&XMFLOAT4(0, 0, 0, 1));

		worldViewM2 = xSpin * worldViewM2;
		worldViewM2 = worldViewM2 * ySpin;

		worldViewM2.r[3] = temp2;
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
	if (spaceActive)
	{
		worldSpaceView = translate * worldSpaceView;
		worldSpaceView = XMMatrixInverse(0, worldSpaceView);

		spaceSkyboxM.r[3] = worldSpaceView.r[3];
	}
	else
	{
		worldViewM = translate * worldViewM;
		viewM = XMMatrixInverse(0, worldViewM);
		skyboxM.r[3] = worldViewM.r[3];

		// update second skybox matrix
		worldViewM2 = translate * worldViewM2;
		viewM2 = XMMatrixInverse(0, worldViewM2);
		skyboxM2.r[3] = worldViewM2.r[3];
	}

	//update proj matrix with new FOV
	DXGI_SWAP_CHAIN_DESC current;
	swap->GetDesc(&current);
	
	projM = XMMatrixPerspectiveFovLH(XMConvertToRadians(currentFOV), (float)current.BufferDesc.Width*0.5f / (float)current.BufferDesc.Height, 0.01f, 100.0f);

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

	viewport.Width = (float)current.BufferDesc.Width*0.5f;
	viewport.Height = (float)current.BufferDesc.Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	context->RSSetViewports(1, &viewport);

	viewport2.Width = (float)current.BufferDesc.Width*0.5f;
	viewport2.Height = (float)current.BufferDesc.Height;
	viewport2.MinDepth = 0.0f;
	viewport2.MaxDepth = 1.0f;
	viewport2.TopLeftX = (float)current.BufferDesc.Width*0.5f;
	viewport2.TopLeftY = 0;

	spaceViewport.Width = (float)current.BufferDesc.Width;
	spaceViewport.Height = (float)current.BufferDesc.Height;
	spaceViewport.MinDepth = 0.0f;
	spaceViewport.MaxDepth = 1.0f;
	spaceViewport.TopLeftX = 0;
	spaceViewport.TopLeftY = 0;



	projM = XMMatrixPerspectiveFovLH(XMConvertToRadians(currentFOV), viewport.Width * 0.5f / viewport.Height, 0.1f, 100.0f);
	projMSpace = XMMatrixPerspectiveFovLH(XMConvertToRadians(currentFOV), spaceViewport.Width / spaceViewport.Height, 0.1f, 100.0f);

	depthStencil->Release();
	depthStencilView->Release();

	// create new zbuffer
	D3D11_TEXTURE2D_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(depthDesc));
	depthDesc.Width = viewport.Width*2.0f;
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