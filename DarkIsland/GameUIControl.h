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

namespace Game
{
	public enum class GameInfoOverlayCommand
	{
		None,
		TapToContinue,
		PleaseWait,
		PlayAgain,
	};

	public interface class IGameUIControl
	{
		void SetGameLoading();
		void SetGameStats(int maxLevel, int hitCount, int shotCount);
		void SetGameOver(bool win, int maxLevel, int hitCount, int shotCount, int highScore);
		void SetLevelStart(int level, Platform::String^ objective, float timeLimit, float bonusTime);
		void SetPause(int level, int hitCount, int shotCount, float timeRemaining);
		void ShowTooSmall();
		void HideTooSmall();
		void SetAction(GameInfoOverlayCommand action);
		void HideGameInfoOverlay();
		void ShowGameInfoOverlay();
		void SetControlsVisible(bool bVisible);
		void player_start();
		void diff_from_camera();
		void SetEditFilterItems();
		void SetDat(int num, Platform::String^ objective);
		void SetNonFocus();

		void SetCols(float ar, float ag, float ab, float dr, float dg, float db, float dx, float dy, float dz, float sr, float sg, float sb, float sp, bool bSnow, bool bRain, bool bFog, float sky_ambient, float sky_diffuse, float sky_brightness);
	};
};
