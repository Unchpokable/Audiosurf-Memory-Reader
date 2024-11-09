#include "pch.h"
#include "engine.h"
#include "load.h"

constexpr std::chrono::duration<long long, std::milli> g_deltaTime { 100 };

static NecromancyEngine* g_necromancyEngine = nullptr;
static Necromancy::Detours::Hook<Necromancy::DirectXEndScene>* g_endSceneHook = nullptr;
static Necromancy::Detours::Hook<Necromancy::Detours::Unstable>* g_trueCallChannelHook = nullptr;

static std::chrono::time_point<std::chrono::steady_clock> g_timePoint;

HRESULT __stdcall Necromancy::HkEndScene_DumpMemory(LPDIRECT3DDEVICE9 device) {
    auto now = std::chrono::high_resolution_clock::now();
    if(std::chrono::duration_cast<std::chrono::milliseconds>(now - g_timePoint) < g_deltaTime)
    {
        g_timePoint = now;
        return g_endSceneHook->original()(device);
    }

    // todo: dump memory

    return g_endSceneHook->original()(device);
}

HRESULT Necromancy::InitDirect3D() {
    IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);

    if(!pD3D)
        return E_FAIL;

    LPDIRECT3DDEVICE9 pDevice = nullptr;

    WNDCLASSEX windowClass;
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = DefWindowProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.hIcon = NULL;
    windowClass.hCursor = NULL;
    windowClass.hbrBackground = NULL;
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = L"Necromancy";
    windowClass.hIconSm = NULL;

    ::RegisterClassEx(&windowClass);

    HWND window = ::CreateWindow(windowClass.lpszClassName, L"Necromancy___", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, windowClass.hInstance, NULL);

    D3DPRESENT_PARAMETERS params;
    params.BackBufferWidth = 0;
    params.BackBufferHeight = 0;
    params.BackBufferFormat = D3DFMT_UNKNOWN;
    params.BackBufferCount = 0;
    params.MultiSampleType = D3DMULTISAMPLE_NONE;
    params.MultiSampleQuality = NULL;
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.hDeviceWindow = window;
    params.Windowed = 1;
    params.EnableAutoDepthStencil = 0;
    params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
    params.Flags = NULL;
    params.FullScreen_RefreshRateInHz = 0;
    params.PresentationInterval = 0;

    auto result = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, window, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT, &params, &pDevice);

    if(FAILED(result) || !pDevice)
    {
        ::DestroyWindow(window);
        ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
        return E_FAIL;
    }

    void** vTable = *reinterpret_cast<void***>(pDevice);
    ::DestroyWindow(window);
    ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

    auto endScene = reinterpret_cast<DirectXEndScene>(vTable[42]);
    if(g_endSceneHook != nullptr)
    {
        g_endSceneHook->detach();
        delete g_endSceneHook;
    }

    g_endSceneHook = new Detours::Hook(endScene, &HkEndScene_DumpMemory);
    if(auto status = g_endSceneHook->attach(); status == Status::DetourException)
    {
        throw RuntimeException("Critical exception during attaching endScene hook");
    }

    return 0;
}

void __fastcall Necromancy::HkTrueCallChannel(A3d_Channel* self, DWORD edx) {
    if(g_necromancyEngine->engineInterface() == nullptr)
    {
        g_necromancyEngine->setQ3DEngineInterface(self->engine);

        g_trueCallChannelHook->detach();
        return std::any_cast<Typedefs::TrueCallChannelFn>(g_trueCallChannelHook->unstableOriginal())(self);
    }

    return std::any_cast<Typedefs::TrueCallChannelFn>(g_trueCallChannelHook->unstableOriginal())(self);
}

void Necromancy::Setup() {
    g_necromancyEngine = new NecromancyEngine();

    g_trueCallChannelHook = new Detours::Hook<Detours::Unstable>();
    auto callChannelStatus = g_trueCallChannelHook->attach(g_necromancyEngine->functions().get<Typedefs::TrueCallChannelFn>(), &HkTrueCallChannel);

    if(callChannelStatus == Status::DetourException)
    {
        throw RuntimeException("Critical exception during attaching TrueCallChannel hook");
    }

    if(callChannelStatus == Status::InvalidHookMode)
    {
        throw LogicException("Trying to hook with unstable attach using non-unstable hook mode");
    }

    if(FAILED(InitDirect3D()))
    {
        throw RuntimeException("Unable to attach hook to DirectX EndScene");
    }
}
