#include "Game.h"

#include "DDSTextureLoader.h"

//////////////
/// Modes ////
//////////////

//Starting Block
#include "StartScreenMode.h"
#include "MainMenuMode.h"

//Game States

//Testing States
#include "MainGameMode.h"

using namespace DirectX;

Game* Game::m_Game = nullptr;
Game::Game(HINSTANCE hInstance)
	: D3D12Application(hInstance)
{
	assert(m_Game == nullptr);
	m_Game = this;
}

Game::~Game()
{
	if (m_D3DDevice != nullptr)
		FlushCommandQueue();

	m_AudioManager.SuspendAudioEngine();
}

bool Game::Initialize()
{
	//Call parent Init for window/d3d setup
	if (!D3D12Application::Initialize())
		return false;
	//Reset CL for init
	ThrowIfFailed(m_CommandList->Reset(m_DirectCommandListAllocator.Get(), nullptr));
	//Get the increment size of a descriptor for this heap type.
	//Hardware specific, so need to query
	m_CbvSrvUavDescriptorSize = m_D3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	////////////////////////////////////////////////////
	/// Run Initialisation Block of functions HERE!! ///
	////////////////////////////////////////////////////

	XTKSetup();
	BuildFrameResources();
	m_AssetManager.LoadClasses("data/ClassAndEquipment/ClassData.json");
	m_AssetManager.LoadEquipment("data/ClassAndEquipment/EquipmentData.json");
	m_AssetManager.LoadSkills("data/ClassAndEquipment/AbilityData.json");
	m_AudioManager.InitAudioEngine(m_WinData.hMainWnd);
	LoadInitialModes();

	//Init UI
	m_UIManager.SetupUIElementsViaFile(std::string("data/ui/UI_Configurations.json"));

	//Init Control Methods
	m_KBM.Initialise(m_WinData.hMainWnd, true, false);
	m_Gamepad.Initialise();

	//Execute init commands
	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsList[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsList), cmdsList);

	//Wait till init is complete
	FlushCommandQueue();

	//Set starting mode
	m_ModeManager.SwitchMode(ModeNames::START_SCREEN);

	//Give pass data a heap pointer
	m_PassData.heap = m_ResourceDescriptors.get();

	return true;
}

void Game::Update(const GameTimer& gt)
{
	//Cycle through the circular frame of frame resources
	m_CurrentFrameResourceIndex = (m_CurrentFrameResourceIndex + 1) % NUM_FRAME_RESOURCES;
	m_CurrentFrameResource2D = m_FrameResources2D[m_CurrentFrameResourceIndex].get();

	//Has the GPU finished proccessing the commands of the current frame resource?
	//If not, wait here till GPU has completed commands upto this fence point
	if (m_CurrentFrameResource2D->Fence != 0 && m_Fence->GetCompletedValue() < m_CurrentFrameResource2D->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFrameResource2D->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	//Update Controller States
	m_Gamepad.Update();
	//Update Current Mode
	m_ModeManager.Update(gt);
}

void Game::Render(const GameTimer& gt)
{
	//Run pre-frame setup
	PreFrame();

	ID3D12DescriptorHeap* heaps[] = { m_ResourceDescriptors->Heap() };
	m_CommandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);

	//Call render for current mode (NOTE: Modes are responsible for starting end ending Spritebatches calls themselves)
	m_ModeManager.Render(m_PassData);

	//Run post frame clean up, flip the buffer etc.
	PostFrame();
	CycleFrame();

	//Commit SB Data
	m_GraphicsMemory->Commit(m_CommandQueue.Get());
}

void Game::PreFrame()
{

	//Get the Current Frames Allocator
	auto cmdListAlloc = m_CurrentFrameResource2D->CmdListAlloc;

	//Reuse the memory associated with command recording
	//We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(cmdListAlloc->Reset());

	//A CL can be reset after it has been added to the command queue via ExecutCommandList
	//Reusing the CL reuses memory
	ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), nullptr));

	//Indicate a state transition on the resource usage
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_SwapChainBuffer[m_CurrentBackBuffer].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	//Clear views
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHdl = {
		m_RTVHeap->GetCPUDescriptorHandleForHeapStart(),
		m_CurrentBackBuffer,
		m_RTVDescriptorSize };
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHdl(m_DSVHeap->GetCPUDescriptorHandleForHeapStart());

	//Clear the back and depth buffers
	m_CommandList->ClearRenderTargetView(rtvHdl, Colors::Black, 0, nullptr);
	m_CommandList->ClearDepthStencilView(dsvHdl, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//Specify the buffers being rendered to
	m_CommandList->OMSetRenderTargets(1, &rtvHdl, FALSE, &dsvHdl);

	//Set viewport and scissor rects
	m_CommandList->RSSetViewports(1, &m_ScreenViewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

}

void Game::PostFrame()
{
	//Indicate a state transition on the resource usage
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_SwapChainBuffer[m_CurrentBackBuffer].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT));

	//Done recording commands
	ThrowIfFailed(m_CommandList->Close());

	//Add CL to the queue for execution
	ID3D12CommandList* cmdsList[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsList), cmdsList);

	//Swap the back and front buffers
	ThrowIfFailed(m_SwapChain->Present(0, 0));
}

void Game::CycleFrame()
{
	//Swap current buffer target
	m_CurrentBackBuffer = (m_CurrentBackBuffer + 1) % SWAP_CHAIN_BUFFER_COUNT;

	//Advance the current fence value to mark the commands upto this point
	m_CurrentFrameResource2D->Fence = ++m_CurrentFence;

	/*
		Add an instruction to the CQ to set a new fence point.
		Because we are the GPU's timeline, the new fence point won't be set until
		the GPU finishes processing all the commands prior to this Signal().
	*/
	m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence);

}

void Game::XTKSetup()
{
	//Setup graphics memory (used in spritebatch)
	m_GraphicsMemory = std::make_unique<GraphicsMemory>(m_D3DDevice.Get());

	//Create a number of descriptors equal to the expected descriptor count
	m_ResourceDescriptors = std::make_unique<DescriptorHeap>(m_D3DDevice.Get(), RESOURCE_RESERVE_COUNT);

	//XTK Management of resources for textures and SpriteBatch
	ResourceUploadBatch resourceUpload(m_D3DDevice.Get());

	//Load Assets From Here
	resourceUpload.Begin();

	//Load all textures in here
	LoadTextures(resourceUpload);
	LoadFonts(resourceUpload);

	//Setup spritebatches
	SetupSpriteBatches(resourceUpload);

	//End of setup and loading
	auto uploadResourcesFinished = resourceUpload.End(m_CommandQueue.Get());
	uploadResourcesFinished.wait();

}

void Game::LoadTextures(ResourceUploadBatch& resourceUpload)
{
	//Test Texture Loader


	///////////////
	/// Sprites ///
	///////////////

	m_AssetManager.SetupNewTexture(
		std::string("ASSASSIN_CLASS_SS_00"),
		std::wstring(L"data/sprites/units/Assassin_Class_Spritesheet.dds"),
		std::string("data/sprites/units/Assassin_Class_Spritesheet.json"),
		GameTextureDescriptors::ASSASSIN_CLASS_SS_00,
		m_D3DDevice.Get(),
		resourceUpload,
		m_ResourceDescriptors.get()
	);

	m_AssetManager.SetupNewTexture(
		std::string("BARBARIAN_CLASS_SS_00"),
		std::wstring(L"data/sprites/units/Barbarian_Class_Spritesheet.dds"),
		std::string("data/sprites/units/Barbarian_Class_Spritesheet.json"),
		GameTextureDescriptors::BARBARIAN_CLASS_SS_00,
		m_D3DDevice.Get(),
		resourceUpload,
		m_ResourceDescriptors.get()
	);

	m_AssetManager.SetupNewTexture(
		std::string("ARCHER_CLASS_SS_00"),
		std::wstring(L"data/sprites/units/Centaur_Class_Spritesheet.dds"),
		std::string("data/sprites/units/Centaur_Class_Spritesheet.json"),
		GameTextureDescriptors::CENTAUR_CLASS_SS_00,
		m_D3DDevice.Get(),
		resourceUpload,
		m_ResourceDescriptors.get()
	);

	m_AssetManager.SetupNewTexture(
		std::string("NECROMANCER_CLASS_SS_00"),
		std::wstring(L"data/sprites/units/Necromancer_Class_Spritesheet.dds"),
		std::string("data/sprites/units/Necromancer_Class_Spritesheet.json"),
		GameTextureDescriptors::NECROMANCER_CLASS_SS_00,
		m_D3DDevice.Get(),
		resourceUpload,
		m_ResourceDescriptors.get()
	);

	m_AssetManager.SetupNewTexture(
		std::string("SORCERESS_CLASS_SS_00"),
		std::wstring(L"data/sprites/units/Sorceress_Class_Spritesheet.dds"),
		std::string("data/sprites/units/Sorceress_Class_Spritesheet.json"),
		GameTextureDescriptors::SORCERESS_CLASS_SS_00,
		m_D3DDevice.Get(),
		resourceUpload,
		m_ResourceDescriptors.get()
	);

	m_AssetManager.SetupNewTexture(
		std::string("WARRIOR_CLASS_SS_00"),
		std::wstring(L"data/sprites/units/Warrior_Class_Spritesheet.dds"),
		std::string("data/sprites/units/Warrior_Class_Spritesheet.json"),
		GameTextureDescriptors::WARRIOR_CLASS_SS_00,
		m_D3DDevice.Get(),
		resourceUpload,
		m_ResourceDescriptors.get()
	);


	////////////////
	/// Tilemaps ///
	////////////////

	m_AssetManager.SetupNewTexture(
		std::string("Tilemap_00"),
		std::wstring(L"data/sprites/tilemaps/Tilemap_00.dds"),
		GameTextureDescriptors::TEST_TILEMAP_00,
		m_D3DDevice.Get(),
		resourceUpload,
		m_ResourceDescriptors.get()
	);

	///////////////////
	/// Backgrounds ///
	///////////////////

	m_AssetManager.SetupNewTexture(
		std::string("Main_Game_BG_00"),
		std::wstring(L"data/backgrounds/Main_Game_BG_00.dds"),
		GameTextureDescriptors::MAIN_GAME_BG_00,
		m_D3DDevice.Get(),
		resourceUpload,
		m_ResourceDescriptors.get()
	);

	m_AssetManager.SetupNewTexture(
		std::string("Main_Menu_BG_00"),
		std::wstring(L"data/backgrounds/Main_Menu_Background_00.dds"),
		GameTextureDescriptors::MAIN_MENU_BG_00,
		m_D3DDevice.Get(),
		resourceUpload,
		m_ResourceDescriptors.get()
	);

	m_AssetManager.SetupNewTexture(
		std::string("Start_Screen_BG_01"),
		std::wstring(L"data/backgrounds/Start_Screen_PH_01.dds"),
		GameTextureDescriptors::START_MENU_BG_00,
		m_D3DDevice.Get(),
		resourceUpload,
		m_ResourceDescriptors.get()
	);

	m_AssetManager.SetupNewTexture(
		std::string("Tutorial_00"),
		std::wstring(L"data/backgrounds/Tutorial_Atlas_00.dds"),
		std::string("data/backgrounds/Tutorial_Atlas_00.json"),
		GameTextureDescriptors::TUTORIAL_00,
		m_D3DDevice.Get(),
		resourceUpload,
		m_ResourceDescriptors.get()
	);


	//////////
	/// UI ///
	//////////

	m_AssetManager.SetupNewTexture(
		std::string("UI_Atlas_01"),
		std::wstring(L"data/ui/UI_Atlas_01.dds"),
		std::string("data/ui/UI_Atlas_01.json"),
		GameTextureDescriptors::UI_ATLAS_01,
		m_D3DDevice.Get(),
		resourceUpload,
		m_ResourceDescriptors.get()
	);

	/////////////////////
	/// Battle Scenes ///
	////////////////////
	m_AssetManager.SetupNewTexture(
		std::string("Battle_Scene_Bridge"),
		std::wstring(L"data/backgrounds/Battle_Scene_Bridge.dds"),
		GameTextureDescriptors::BATTLE_SCENE_BRIDGE,
		m_D3DDevice.Get(),
		resourceUpload,
		m_ResourceDescriptors.get()
	);

	m_AssetManager.SetupNewTexture(
		std::string("Battle_Scene_Grass"),
		std::wstring(L"data/backgrounds/Battle_Scene_Grass.dds"),
		GameTextureDescriptors::BATTLE_SCENE_GRASS,
		m_D3DDevice.Get(),
		resourceUpload,
		m_ResourceDescriptors.get()
	);

	m_AssetManager.SetupNewTexture(
		std::string("Battle_Scene_Sand"),
		std::wstring(L"data/backgrounds/Battle_Scene_Sand.dds"),
		GameTextureDescriptors::BATTLE_SCENE_SAND,
		m_D3DDevice.Get(),
		resourceUpload,
		m_ResourceDescriptors.get()
	);

}

void Game::SetupSpriteBatches(DirectX::ResourceUploadBatch& resourceUpload)
{
	//Setup Spritebatches for use (Using default settings)
	RenderTargetState rtState(m_BackBufferFormat, DXGI_FORMAT_UNKNOWN);
	SpriteBatchPipelineStateDescription pd(rtState);

	//Create Scene SpriteBatch
	m_PassData.sceneSB = std::make_unique<SpriteBatch>(
		m_D3DDevice.Get(),
		resourceUpload,
		pd
		);
	m_PassData.sceneSB->SetViewport(m_ScreenViewport);

	//Create UI SpriteBatch
	m_PassData.uiSB = std::make_unique<SpriteBatch>(
		m_D3DDevice.Get(),
		resourceUpload,
		pd
		);
	m_PassData.uiSB->SetViewport(m_ScreenViewport);
}

void Game::LoadFonts(DirectX::ResourceUploadBatch& resourceUpload)
{
	//Reserve space
	m_PassData.spritefonts.reserve(GameFonts::FONT_COUNT);

	m_PassData.spritefonts.push_back(std::make_unique<SpriteFont>(
		m_D3DDevice.Get(), resourceUpload,
		L"data/font/AlloyInk_10.spritefont",
		m_ResourceDescriptors->GetCpuHandle(GameTextureDescriptors::RESOURCE_COUNT + GameFonts::SF_ALLOYINK_10),
		m_ResourceDescriptors->GetGpuHandle(GameTextureDescriptors::RESOURCE_COUNT + GameFonts::SF_ALLOYINK_10)
		));

	m_PassData.spritefonts.push_back(std::make_unique<SpriteFont>(
		m_D3DDevice.Get(),
		resourceUpload,
		L"data/font/AlloyInk_12.spritefont",
		m_ResourceDescriptors->GetCpuHandle(GameTextureDescriptors::RESOURCE_COUNT + GameFonts::SF_ALLOYINK_12),
		m_ResourceDescriptors->GetGpuHandle(GameTextureDescriptors::RESOURCE_COUNT + GameFonts::SF_ALLOYINK_12)
		));

	m_PassData.spritefonts.push_back(std::make_unique<SpriteFont>(
		m_D3DDevice.Get(), resourceUpload,
		L"data/font/AlloyInk_14.spritefont",
		m_ResourceDescriptors->GetCpuHandle(GameTextureDescriptors::RESOURCE_COUNT + GameFonts::SF_ALLOYINK_14),
		m_ResourceDescriptors->GetGpuHandle(GameTextureDescriptors::RESOURCE_COUNT + GameFonts::SF_ALLOYINK_14)
		));

	m_PassData.spritefonts.push_back(std::make_unique<SpriteFont>(
		m_D3DDevice.Get(),
		resourceUpload,
		L"data/font/AlloyInk_16.spritefont",
		m_ResourceDescriptors->GetCpuHandle(GameTextureDescriptors::RESOURCE_COUNT + GameFonts::SF_ALLOYINK_16),
		m_ResourceDescriptors->GetGpuHandle(GameTextureDescriptors::RESOURCE_COUNT + GameFonts::SF_ALLOYINK_16)
		));

	m_PassData.spritefonts.push_back(std::make_unique<SpriteFont>(
		m_D3DDevice.Get(), resourceUpload,
		L"data/font/AlloyInk_18.spritefont",
		m_ResourceDescriptors->GetCpuHandle(GameTextureDescriptors::RESOURCE_COUNT + GameFonts::SF_ALLOYINK_18),
		m_ResourceDescriptors->GetGpuHandle(GameTextureDescriptors::RESOURCE_COUNT + GameFonts::SF_ALLOYINK_18)
		));

	m_PassData.spritefonts.push_back(std::make_unique<SpriteFont>(
		m_D3DDevice.Get(),
		resourceUpload,
		L"data/font/AlloyInk_20.spritefont",
		m_ResourceDescriptors->GetCpuHandle(GameTextureDescriptors::RESOURCE_COUNT + GameFonts::SF_ALLOYINK_20),
		m_ResourceDescriptors->GetGpuHandle(GameTextureDescriptors::RESOURCE_COUNT + GameFonts::SF_ALLOYINK_20)
		));

	m_PassData.spritefonts.push_back(std::make_unique<SpriteFont>(
		m_D3DDevice.Get(),
		resourceUpload,
		L"data/font/212_Keyboard_32.spritefont",
		m_ResourceDescriptors->GetCpuHandle(GameTextureDescriptors::RESOURCE_COUNT + GameFonts::SF_212_KEYBOARD_48),
		m_ResourceDescriptors->GetGpuHandle(GameTextureDescriptors::RESOURCE_COUNT + GameFonts::SF_212_KEYBOARD_48)
		));

}

void Game::BuildFrameResources()
{
	for (int i(0); i < NUM_FRAME_RESOURCES; ++i)
	{
		m_FrameResources2D.push_back(std::make_unique<FrameResource2D>(m_D3DDevice.Get()));
	}
}

void Game::LoadInitialModes()
{
	m_ModeManager.AddMode(new StartScreenMode(ModeNames::START_SCREEN));
	m_ModeManager.AddMode(new MainMenuMode(ModeNames::MAIN_MENU));
	m_ModeManager.AddMode(new MainGameMode(ModeNames::MAIN_GAME_MODE));
}

LRESULT Game::MessageHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	/*
		Before handling any other messages, we wanna intercept here and check for WM_CHAR (Slow Inputs) and 
		WM_INPUT (Raw, Fast Inputs) and pass them to the appropriate function calls.
	*/
	switch (msg)
	{
	case WM_CHAR:
		m_ModeManager.ProcessKey(static_cast<char>(wParam));
		break;

	case WM_INPUT:
		m_KBM.MessageEvent((HRAWINPUT)lParam);
		break;
	}


	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_WinData.appPaused = true;
			m_Timer.Stop();
		}
		else
		{
			m_WinData.appPaused = false;
			m_Timer.Start();
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		m_WinData.clientWidth = LOWORD(lParam);
		m_WinData.clientHeight = HIWORD(lParam);
		SnapToRes16_9();
		if (m_D3DDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				m_WinData.appPaused = true;
				m_WinData.minimized = true;
				m_WinData.maximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				m_WinData.appPaused = false;
				m_WinData.minimized = false;
				m_WinData.maximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (m_WinData.minimized)
				{
					m_WinData.appPaused = false;
					m_WinData.minimized = false;
					OnResize();
				}

				// Restoring from maximized state?
				else if (m_WinData.maximized)
				{
					m_WinData.appPaused = false;
					m_WinData.maximized = false;
					OnResize();
				}
				else if (m_WinData.resizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{

					OnResize();
				}
			}
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		m_WinData.appPaused = true;
		m_WinData.resizing = true;
		m_Timer.Stop();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		m_WinData.appPaused = false;
		m_WinData.resizing = false;
		m_Timer.Start();
		SetWindowPos(m_WinData.hMainWnd, HWND_TOP, 0, 0, m_WinData.clientWidth, m_WinData.clientHeight, SWP_FRAMECHANGED | SWP_NOMOVE);
		OnResize();
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


