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

#include "pch.h"
#include "GameLevel.h"

//----------------------------------------------------------------------
using namespace Game;

bool GameLevel::Update(
    float /* time */,
    float /* elapsedTime */,
    float /* timeRemaining*/,
    std::vector<Game::GameObject^> objects
    )
{
    int left = 0;

    for (auto object = objects.begin(); object != objects.end(); object++)
    {
        if ((*object)->Active() && (*object)->Target())
        {
            if ((*object)->Hit())
            {
                (*object)->Active(false);
            }
            else
            {
                left++;
            }
        }
    }
    return (left == 0);
}

//----------------------------------------------------------------------

void GameLevel::SaveState(PersistentState^ /* state */)
{
}

//----------------------------------------------------------------------

void GameLevel::LoadState(PersistentState^ /* state */)
{
}

//----------------------------------------------------------------------

Platform::String^ GameLevel::Objective()
{
    return m_objective;
}

//----------------------------------------------------------------------

float GameLevel::TimeLimit()
{
    return m_timeLimit;
}

//----------------------------------------------------------------------