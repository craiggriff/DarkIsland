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

// Level3:
// This class defines the third level of the game.  In this level each of the
// nine targets is moving along closed paths and can be hit
// in any order.

#include "GameLevel.h"
namespace Game
{
	ref class Level3 : public GameLevel
	{
	internal:
		Level3();
		virtual void Initialize(std::vector<Game::GameObject^> objects) override;
	};
}