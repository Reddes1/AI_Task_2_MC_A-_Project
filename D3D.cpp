#include "D3D.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Forward hwnd on because we can get messages (e.g., WM_CREATE)
    // before CreateWindow returns, and thus before mhMainWnd is valid.
    return D3D12Application::Get()->MessageHandler(hwnd, msg, wParam, lParam);
}


D3D12Application* D3D12Application::m_App = nullptr;
void D3D12Application::Set4xMsaaState(bool value)
{
    if (m_Enable4xMsaa != value)
    {
        m_Enable4xMsaa = value;

        //Recreate swapchain and buffers with new multisample settings
        CreateSwapChain();
        OnResize();
    }
}

int D3D12Application::Run()
{
    MSG msg = { 0 };

    m_Timer.Reset();

    while (msg.message != WM_QUIT)
    {
        // If there are Window messages then process them.
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        // Otherwise, do animation/game stuff.
        else
        {
            m_Timer.Tick();

            if (!m_WinData.appPaused)
            {
                CalculateFrameStats();
                Update(m_Timer);
                Render(m_Timer);
            }
            else
            {
                Sleep(100);
            }
        }
    }

    return (int)msg.wParam;
}

bool D3D12Application::Initialize()
{
    if (!InitMainWindow())
        return false;

    if (!InitD3D())
        return false;

    // Do the initial resize code.
    OnResize();

    return true;
}

D3D12Application::D3D12Application(HINSTANCE hInstance)
{
    //Set window instance
    m_WinData.hAppInst = hInstance;

    // Only one D3DApp can be constructed.
    assert(m_App == nullptr);
    m_App = this;
}

D3D12Application::~D3D12Application()
{
    if (m_D3DDevice != nullptr)
        FlushCommandQueue();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12Application::GetCurrentBackBufferView() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
        m_RTVHeap->GetCPUDescriptorHandleForHeapStart(),
        m_CurrentBackBuffer,
        m_RTVDescriptorSize);
}

bool D3D12Application::InitMainWindow()
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_WinData.hAppInst;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = "MainWnd";

    if (!RegisterClass(&wc))
    {
        MessageBox(0, "RegisterClass Failed.", 0, 0);
        return false;
    }

    // Compute window rectangle dimensions based on requested client area dimensions.
    RECT R = { 0, 0, m_WinData.clientWidth, m_WinData.clientHeight };
    AdjustWindowRect(&R, WS_TILEDWINDOW, false);
    int width = R.right - R.left;
    int height = R.bottom - R.top;

    int offsetX = CW_USEDEFAULT, offsetY = CW_USEDEFAULT;
    offsetX = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    offsetY = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

    m_WinData.hMainWnd = CreateWindow("MainWnd", m_WinData.mainWndCaption.c_str(),
        m_WinData.winStyle, offsetX, offsetY, width, height, 0, 0, m_WinData.hAppInst, 0);
    if (!m_WinData.hMainWnd)
    {
        MessageBox(0, "CreateWindow Failed.", 0, 0);
        return false;
    }

    ShowWindow(m_WinData.hMainWnd, SW_SHOW);
    UpdateWindow(m_WinData.hMainWnd);

    return true;
}

void D3D12Application::ReleaseD3D(bool extraReporting)
{
    //check if full screen - not advisable to exit in full screen mode
    if (m_SwapChain)
    {
        BOOL fullscreen = false;
        HR(m_SwapChain->GetFullscreenState(&fullscreen, nullptr));
        if (fullscreen) //go for a window
            m_SwapChain->SetFullscreenState(false, nullptr);
    }

    if (extraReporting)
    {
        //ID3D12Debug* pd3dDebug;
        //HR(m_D3DDevice->QueryInterface(__uuidof(ID3D12Debug), reinterpret_cast<void**>(&pd3dDebug)));
        //HR(pd3dDebug->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY));
        //ReleaseCOM(pd3dDebug);
    }

    if (m_D3DDevice != nullptr)
        FlushCommandQueue();
}

// New Functions
bool D3D12Application::InitD3D()
{

#if defined(DEBUG) || defined(_DEBUG) 
    // Enable the D3D12 debug layer.
    {
        ComPtr<ID3D12Debug> debugController;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
        debugController->EnableDebugLayer();
    }
#endif

    //Create DXGIFactory
    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_DXGIFactory)));

    // Try to create hardware device.
    HRESULT hardwareResult = D3D12CreateDevice(
        nullptr,             // default adapter
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&m_D3DDevice));

    // Fallback to WARP device.
    if (FAILED(hardwareResult))
    {
        ComPtr<IDXGIAdapter> pWarpAdapter;
        ThrowIfFailed(m_DXGIFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

        ThrowIfFailed(D3D12CreateDevice(
            pWarpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_D3DDevice)));
    }

    //Create Fence
    ThrowIfFailed(m_D3DDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(&m_Fence)));


    m_RTVDescriptorSize = m_D3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_DSVDescriptorSize = m_D3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    m_CbvSrvUavDescriptorSize = m_D3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Check 4X MSAA quality support for our back buffer format.
    // All Direct3D 11 capable devices support 4X MSAA for all render 
    // target formats, so we only need to check quality support.

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
    msQualityLevels.Format = m_BackBufferFormat;
    msQualityLevels.SampleCount = 4;
    msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msQualityLevels.NumQualityLevels = 0;
    ThrowIfFailed(m_D3DDevice->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
        &msQualityLevels,
        sizeof(msQualityLevels)));

    m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
    assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

#ifdef _DEBUG
    LogAdapters();
#endif

    CreateCommandObjects();
    CreateSwapChain();
    CreateRtvAndDsvDescriptorHeaps();

    return true;
}

void D3D12Application::LogAdapters()
{
    UINT i = 0;
    IDXGIAdapter* adapter = nullptr;
    std::vector<IDXGIAdapter*> adapterList;
    while (m_DXGIFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);

        std::wstring text = L"***Adapter: ";
        text += desc.Description;
        text += L"\n";

        OutputDebugStringW(text.c_str());

        adapterList.push_back(adapter);

        ++i;
    }

    for (size_t i = 0; i < adapterList.size(); ++i)
    {
        LogAdapterOutputs(adapterList[i]);
        ReleaseCOM(adapterList[i]);
    }
}

void D3D12Application::LogAdapterOutputs(IDXGIAdapter* adapter)
{
    UINT i = 0;
    IDXGIOutput* output = nullptr;
    while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_OUTPUT_DESC desc;
        output->GetDesc(&desc);

        std::wstring text = L"***Output: ";
        text += desc.DeviceName;
        text += L"\n";
        OutputDebugStringW(text.c_str());

        LogOutputDisplayModes(output, m_BackBufferFormat);

        ReleaseCOM(output);

        ++i;
    }
}

void D3D12Application::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
    UINT count = 0;
    UINT flags = 0;

    // Call with nullptr to get list count.
    output->GetDisplayModeList(format, flags, &count, nullptr);

    std::vector<DXGI_MODE_DESC> modeList(count);
    output->GetDisplayModeList(format, flags, &count, &modeList[0]);

    for (auto& x : modeList)
    {
        UINT n = x.RefreshRate.Numerator;
        UINT d = x.RefreshRate.Denominator;
        std::wstring text =
            L"Width = " + std::to_wstring(x.Width) + L" " +
            L"Height = " + std::to_wstring(x.Height) + L" " +
            L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
            L"\n";

        ::OutputDebugStringW(text.c_str());
    }
}

void D3D12Application::CalculateFrameStats()
{
    // Code computes the average frames per second, and also the 
    // average time it takes to render one frame.  These stats 
    // are appended to the window caption bar.

    static int frameCnt = 0;
    static float timeElapsed = 0.0f;

    frameCnt++;

    // Compute averages over one second period.
    if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
    {
        float fps = (float)frameCnt; // fps = frameCnt / 1
        float mspf = 1000.0f / fps;

        std::string fpsStr = std::to_string(fps);
        std::string mspfStr = std::to_string(mspf);

        std::string windowText = m_WinData.mainWndCaption +
            "    fps: " + fpsStr +
            "   mspf: " + mspfStr;

        SetWindowTextA(m_WinData.hMainWnd, windowText.c_str());

        // Reset for next average.
        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}

void D3D12Application::CreateCommandObjects()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(m_D3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue)));

    ThrowIfFailed(m_D3DDevice->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(m_DirectCommandListAllocator.GetAddressOf())));

    ThrowIfFailed(m_D3DDevice->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_DirectCommandListAllocator.Get(), // Associated command allocator
        nullptr,                   // Initial PipelineStateObject
        IID_PPV_ARGS(m_CommandList.GetAddressOf())));

    // Start off in a closed state.  This is because the first time we refer 
    // to the command list we will Reset it, and it needs to be closed before
    // calling Reset.
    m_CommandList->Close();
}

void D3D12Application::CreateSwapChain()
{
    // Release the previous swapchain we will be recreating.
    m_SwapChain.Reset();

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = m_WinData.clientWidth;
    sd.BufferDesc.Height = m_WinData.clientHeight;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = m_BackBufferFormat;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count = m_Enable4xMsaa ? 4 : 1;
    sd.SampleDesc.Quality = m_Enable4xMsaa ? (m_4xMsaaQuality - 1) : 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
    sd.OutputWindow = m_WinData.hMainWnd;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    // Note: Swap chain uses queue to perform flush.
    ThrowIfFailed(m_DXGIFactory->CreateSwapChain(
        m_CommandQueue.Get(),
        &sd,
        m_SwapChain.GetAddressOf()));
}

void D3D12Application::CreateRtvAndDsvDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_D3DDevice->CreateDescriptorHeap(
        &rtvHeapDesc, IID_PPV_ARGS(m_RTVHeap.GetAddressOf())));


    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_D3DDevice->CreateDescriptorHeap(
        &dsvHeapDesc, IID_PPV_ARGS(m_DSVHeap.GetAddressOf())));
}

void D3D12Application::OnResize()
{
    assert(m_D3DDevice);
    assert(m_SwapChain);
    assert(m_DirectCommandListAllocator);

    // Flush before changing any resources.
    FlushCommandQueue();

    ThrowIfFailed(m_CommandList->Reset(m_DirectCommandListAllocator.Get(), nullptr));

    // Release the previous resources we will be recreating.
    for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
        m_SwapChainBuffer[i].Reset();
    m_DepthStencilBuffer.Reset();

    // Resize the swap chain.
    ThrowIfFailed(m_SwapChain->ResizeBuffers(
        SWAP_CHAIN_BUFFER_COUNT,
        m_WinData.clientWidth, m_WinData.clientHeight,
        m_BackBufferFormat,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

    m_CurrentBackBuffer = 0;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
    {
        ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i])));
        m_D3DDevice->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
        rtvHeapHandle.Offset(1, m_RTVDescriptorSize);
    }

    // Create the depth/stencil buffer and view.
    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = m_WinData.clientWidth;
    depthStencilDesc.Height = m_WinData.clientHeight;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;

    // Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
    // the depth buffer.  Therefore, because we need to create two views to the same resource:
    //   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
    //   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
    // we need to create the depth buffer resource with a typeless format.  
    depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

    depthStencilDesc.SampleDesc.Count = m_Enable4xMsaa ? 4 : 1;
    depthStencilDesc.SampleDesc.Quality = m_Enable4xMsaa ? (m_4xMsaaQuality - 1) : 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = m_DepthStencilFormat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;
    ThrowIfFailed(m_D3DDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear,
        IID_PPV_ARGS(m_DepthStencilBuffer.GetAddressOf())));

    // Create descriptor to mip level 0 of entire resource using the format of the resource.
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = m_DepthStencilFormat;
    dsvDesc.Texture2D.MipSlice = 0;
    m_D3DDevice->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &dsvDesc, GetDSVHeapStart());

    // Transition the resource from its initial state to be used as a depth buffer.
    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

    // Execute the resize commands.
    ThrowIfFailed(m_CommandList->Close());
    ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
    m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until resize is complete.
    FlushCommandQueue();

    // Update the viewport transform to cover the client area.
    m_ScreenViewport.TopLeftX = 0;
    m_ScreenViewport.TopLeftY = 0;
    m_ScreenViewport.Width = static_cast<float>(m_WinData.clientWidth);
    m_ScreenViewport.Height = static_cast<float>(m_WinData.clientHeight);
    m_ScreenViewport.MinDepth = 0.0f;
    m_ScreenViewport.MaxDepth = 1.0f;

    m_ScissorRect = { 0, 0, m_WinData.clientWidth, m_WinData.clientHeight };
}

void D3D12Application::FlushCommandQueue()
{
    // Advance the fence value to mark commands up to this fence point.
    m_CurrentFence++;

    // Add an instruction to the command queue to set a new fence point.  Because we 
    // are on the GPU timeline, the new fence point won't be set until the GPU finishes
    // processing all the commands prior to this Signal().
    ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence));

    // Wait until the GPU has completed commands up to this fence point.
    if (m_Fence->GetCompletedValue() < m_CurrentFence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

        // Fire event when GPU hits current fence.  
        ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle));

        // Wait until the GPU hits current fence event is fired.
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}

void D3D12Application::SnapToRes16_9()
{
    //Setup common resolutions up to 1920x1080
    const XMINT2 res[] =
    {
        {640, 360},
    /*    {854, 480},*/
 /*       {1024, 576},*/
    /*    {1152, 648},*/
        {1280, 720},
        //{1366, 768},
        {1600, 900},
        {1920, 1080}
    };
    int size = sizeof(res) / sizeof(res[0]);
    //Solving for window X, Y is disregarded

    int& winX = m_WinData.clientWidth;

    //Solve for edge cases
    if (winX <= res[0].x)
    {
        m_WinData.clientWidth = res[0].x;
        m_WinData.clientHeight = res[0].y;
        return;
    }
    if (winX >= res[size-1].x)
    {
        m_WinData.clientWidth = res[size-1].x;
        m_WinData.clientHeight = res[size-1].y;
        return;
    }

    //Run search (Binary Search)
    int i = 0, j = size, mid = 0;
    while (i < j) 
    {
        mid = (i + j) / 2;

        //Check if midway point matches
        if (res[mid].x == winX)
        {
            m_WinData.clientWidth = res[mid].x;
            m_WinData.clientHeight = res[mid].y;
            return;
        }
        //Is the current res in the lower or upper half of the array of values
        if (winX < res[mid].x)
        {
            if (mid > 0 && winX > res[mid - 1].x)
            {

                int val = GetClosestInt(res[mid - 1].x, res[mid].x, winX);
                if (val == res[mid-1].x)
                {
                    m_WinData.clientWidth = res[mid-1].x;
                    m_WinData.clientHeight = res[mid-1].y;
                    return;
                }
                else
                {
                    m_WinData.clientWidth = res[mid].x;
                    m_WinData.clientHeight = res[mid].y;
                    return;
                }
            }
            j = mid;
        }
        else
        {
            if ((mid < size-1) && (winX < res[mid + 1].x))
            {
                int val = GetClosestInt(res[mid].x, res[mid + 1].x, winX);
                if (val == res[mid + 1].x)
                {
                    m_WinData.clientWidth = res[mid + 1].x;
                    m_WinData.clientHeight = res[mid + 1].y;
                    return;
                }
                else
                {
                    m_WinData.clientWidth = res[mid].x;
                    m_WinData.clientHeight = res[mid].y;
                    return;
                }
            }
            i = mid + 1;
        }

    }
    
    //Last possible value
    m_WinData.clientWidth = res[mid].x;
    m_WinData.clientHeight = res[mid].y;
    return;
    
}

int D3D12Application::GetClosestInt(int val1, int val2, int target)
{
    if (target - val1 >= val2 - target)
        return val2;
    else
        return val1;
}


