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

#include <wrl.h>
#include <wrl/client.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#include <d2d1effects_2.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <DirectXMath.h>

#include <xaudio2.h>
#include <xaudio2fx.h>
#include <x3daudio.h>

#include <collection.h>

#include <mmreg.h>
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>

#include <stdio.h>
#include <vector>
#include <memory>
#include <random>

#include <ppltasks.h>
#include <agile.h>
#include <concrt.h>

#include <stdio.h>
#include <vector>
#include <memory>

//#include <ppltasks.h>
#include <agile.h>
// #include <concrt.h>
#include <BasicMath.h>

#include <concrt.h>
#include <pplinterface.h>

#include "BasicLoader.h"

#include "Common/DirectXSample.h"

#include "PerlinNoise.h"

#define NumberOfFaces 6

//#define M_PI 3.14159265358979323846

#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923
#define M_PI_4     0.785398163397448309616

#define FIN_DIST 0.0f

#define BIT(x) (1<<(x))
enum collisiontypes {
	COL_NOTHING = 0, //<Collide with nothing
	COL_RAY = 1, //<Collide with ray
	COL_TERRAIN = 4,
	COL_WHEEL = 8,
	COL_CARBODY = 16,

	COL_WHEELLIFT = 32,
	COL_OBJECTS = 64,
	COL_WALLS = 128,
	COL_CAAARBODY = 256,
	COL_CHAR = 512
};

enum physicsshapes {
	PHY_NOTHING = 0, //Physics nothing
	PHY_BOX = 1,
	PHY_CYLINDER = 2,
	PHY_SPHERE = 3,
	PHY_ELLIPSEOID = 4,
	PHY_CONVEXHULL = 5,
};

using namespace concurrency;
using namespace DirectX;
using namespace DX;
using namespace Microsoft::WRL;

using namespace std;