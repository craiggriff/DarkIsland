//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

struct PNTVertex
{
	/*
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 textureCoordinate;
	*/
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT4 tan;
	UINT color;
	XMFLOAT2 textureCoordinate;
};

static D3D11_INPUT_ELEMENT_DESC PNTVertexLayout[] =
{
	/*
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	*/
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

struct ConstantBufferNeverChanges
{
	XMFLOAT4 lightPosition[4];
	XMFLOAT4 lightColor;
};

struct ConstantBufferChangeOnResize
{
	XMFLOAT4X4 projection;
};

struct ConstantBufferChangesEveryFrame
{
	XMFLOAT4X4 view;
};

struct ConstantBufferChangesEveryPrim
{
	XMFLOAT4X4 worldMatrix;
	XMFLOAT4 meshColor;
	XMFLOAT4 diffuseColor;
	XMFLOAT4 specularColor;
	float specularPower;
};
