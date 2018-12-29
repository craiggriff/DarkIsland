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

#include "DirectXPage.g.h"
#include "GameMain.h"
#include "ProductItem.h"

// When submitting the app to the store, disable the Store Simulator by commenting out this definition.
#define USE_STORE_SIMULATOR 1

namespace Game
{
	/// <summary>
	/// A page that hosts a DirectX SwapChainPanel.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class DirectXPage sealed : Game::IGameUIControl
	{
	public:
		DirectXPage();

		void OnSuspending();
		void OnResuming();

		// IGameUIControl methods.
		virtual void SetGameLoading();
		virtual void SetGameStats(int maxLevel, int hitCount, int shotCount);
		virtual void SetGameOver(bool win, int maxLevel, int hitCount, int shotCount, int highScore);
		virtual void SetLevelStart(int level, Platform::String^ objective, float timeLimit, float bonusTime);
		virtual void SetPause(int level, int hitCount, int shotCount, float timeRemaining);
		virtual void ShowTooSmall();
		virtual void HideTooSmall();
		virtual void SetAction(GameInfoOverlayCommand action);
		virtual void HideGameInfoOverlay();
		virtual void ShowGameInfoOverlay();
		virtual void SetControlsVisible(bool bVisible);
		virtual void player_start();
		virtual void diff_from_camera();
		virtual void SetEditFilterItems();
		virtual void SetDat(int num, Platform::String^ objective);
		virtual void SetNonFocus();
	private:
		// Window event handlers.
		void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
		void OnWindowActivationChanged(
			_In_ Platform::Object^ sender,
			_In_ Windows::UI::Core::WindowActivatedEventArgs^ args
		);

		// DisplayInformation event handlers.
		void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnStereoEnabledChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);

		// Other event handlers.
		void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Object^ args);
		void OnSwapChainPanelSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
		void OnWindowSizeChanged(
			_In_ Windows::UI::Core::CoreWindow^ sender,
			_In_ Windows::UI::Core::WindowSizeChangedEventArgs^ args
		);

		// License handlers.
		void InitializeLicense();
		void InitializeLicenseCore();
		void OnLicenseChanged();
		void SetProductItems(
			Windows::ApplicationModel::Store::ListingInformation^ listing,
			Windows::ApplicationModel::Store::LicenseInformation^ license
		);
#ifdef USE_STORE_SIMULATOR
		void ResetLicenseFromFile();
#endif

		// Page code behind.
		void OnPlayButtonClicked(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args);
		void OnResetButtonClicked(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args);
		void OnBuyAppButtonTapped(Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ args);
		void OnBuySelectorClicked(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args);
		void OnChangeBackgroundButtonClicked(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args);
		void OnResetLicenseButtonClicked(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args);
		void OnGameInfoOverlayTapped(Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ args);
		void OnAppBarOpened(Object^ sender, Object^ args);
		void OnStoreReturnClicked(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args);
		void OnLoadStoreClicked(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args);

		void OptionalTrialUpgrade();
		void ShowStoreFlyout();

	public:
		virtual void SetCols(float ar, float ag, float ab, float dr, float dg, float db, float dx, float dy, float dz, float sr, float sg, float sb, float sp, bool bSnow, bool bRain, bool bFog, float sky_ambient, float sky_diffuse, float sky_brightness);

	private:
		// Resources used to render the DirectX content in the XAML page background.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		std::unique_ptr<GameMain> m_main;
		bool m_windowVisible;
		bool m_playActive;

		Windows::ApplicationModel::Store::LicenseInformation^ m_licenseInformation;
		Windows::ApplicationModel::Store::ListingInformation^ m_listingInformation;
		bool m_possiblePurchaseUpgrade;
#ifdef USE_STORE_SIMULATOR
		PersistentState^                                    m_licenseState;
		bool                                                m_isTrial;
#endif
		void sliderrr_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sliderrg_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sliderrb_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);

		void sdiffr_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sdiffg_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sdiffb_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sambr_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sambg_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sambb_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sspecr_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sspecg_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sspecb_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sspecpow_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);

		void sdiffx_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sdiffy_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sdiffz_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);

		void skybox_changed(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void togglee_snow(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void togglee_rain(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void togglee_fog(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void sambsky_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sdiffsky_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void sskybright_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void ButtonStuff_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void OnContinueTapped(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
		void OnNextLevelClicked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void togglee_fly(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void togglee_edasplay(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void itemtype_changed(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void deleteradius_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void Onfocus(Windows::UI::Xaml::Controls::Control^ sender, Windows::UI::Xaml::Controls::FocusEngagedEventArgs^ args);
		void ExitTapped(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
		void OnAppBarClosed(Platform::Object^ sender, Platform::Object^ e);
		void OnNextLevelClicked(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
		void OnCloseThisTapped(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
		void ExitTapped2(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
		void OnSettingsTapped(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
		void fxaa_change(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
		void Slider_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
	};
}
