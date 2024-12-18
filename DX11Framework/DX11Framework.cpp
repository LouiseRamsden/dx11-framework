#include "DX11Framework.h"
#include "GameObject.h"

#include <string>

//I dont like this Yaml Library very much at all
#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#pragma comment(lib, "yaml-cppd.lib")
//not one bit

//#define RETURNFAIL(x) if(FAILED(x)) return x;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

HRESULT DX11Framework::Initialise(HINSTANCE hInstance, int nShowCmd)
{
    HRESULT hr = S_OK;

    hr = CreateWindowHandle(hInstance, nShowCmd);
    if (FAILED(hr)) return E_FAIL;

    hr = CreateD3DDevice();
    if (FAILED(hr)) return E_FAIL;

    hr = CreateSwapChainAndFrameBuffer();
    if (FAILED(hr)) return E_FAIL;

    hr = InitShadersAndInputLayout();
    if (FAILED(hr)) return E_FAIL;

    hr = InitVertexIndexBuffers();
    if (FAILED(hr)) return E_FAIL;

    hr = InitPipelineVariables();
    if (FAILED(hr)) return E_FAIL;

    hr = InitRunTimeData();
    if (FAILED(hr)) return E_FAIL;

    return hr;
}

HRESULT DX11Framework::CreateWindowHandle(HINSTANCE hInstance, int nCmdShow)
{
    const wchar_t* windowName  = L"DX11Framework";

    WNDCLASSW wndClass;
    wndClass.style = 0;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = 0;
    wndClass.hIcon = 0;
    wndClass.hCursor = 0;
    wndClass.hbrBackground = 0;
    wndClass.lpszMenuName = 0;
    wndClass.lpszClassName = windowName;

    RegisterClassW(&wndClass);

    _windowHandle = CreateWindowExW(0, windowName, windowName, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 
        _WindowWidth, _WindowHeight, nullptr, nullptr, hInstance, nullptr);

    return S_OK;
}

HRESULT DX11Framework::CreateD3DDevice()
{
    HRESULT hr = S_OK;

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
    };

    ID3D11Device* baseDevice;
    ID3D11DeviceContext* baseDeviceContext;

    DWORD createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT | createDeviceFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &baseDevice, nullptr, &baseDeviceContext);
    if (FAILED(hr)) return hr;

    ///////////////////////////////////////////////////////////////////////////////////////////////

    hr = baseDevice->QueryInterface(__uuidof(ID3D11Device), reinterpret_cast<void**>(&_device));
    hr = baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext), reinterpret_cast<void**>(&_immediateContext));

    baseDevice->Release();
    baseDeviceContext->Release();

    ///////////////////////////////////////////////////////////////////////////////////////////////

    hr = _device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&_dxgiDevice));
    if (FAILED(hr)) return hr;

    IDXGIAdapter* dxgiAdapter;
    hr = _dxgiDevice->GetAdapter(&dxgiAdapter);
    hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&_dxgiFactory));
    dxgiAdapter->Release();

    return S_OK;
}

HRESULT DX11Framework::CreateSwapChainAndFrameBuffer()
{
    HRESULT hr = S_OK;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
    swapChainDesc.Width = 0; // Defer to WindowWidth
    swapChainDesc.Height = 0; // Defer to WindowHeight
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //FLIP* modes don't support sRGB backbuffer
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = 0;

    hr = _dxgiFactory->CreateSwapChainForHwnd(_device, _windowHandle, &swapChainDesc, nullptr, nullptr, &_swapChain);
    if (FAILED(hr)) return hr;

    ///////////////////////////////////////////////////////////////////////////////////////////////

    ID3D11Texture2D* frameBuffer = nullptr;

    hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&frameBuffer));
    if (FAILED(hr)) return hr;

    D3D11_RENDER_TARGET_VIEW_DESC framebufferDesc = {};
    framebufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; //sRGB render target enables hardware gamma correction
    framebufferDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

    hr = _device->CreateRenderTargetView(frameBuffer, &framebufferDesc, &_frameBufferView);

    D3D11_TEXTURE2D_DESC depthBufferDesc = {};
    frameBuffer->GetDesc(&depthBufferDesc);

    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    _device->CreateTexture2D(&depthBufferDesc, nullptr, &_depthStencilBuffer);
    _device->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);



    frameBuffer->Release();

    return hr;
}

HRESULT DX11Framework::InitShadersAndInputLayout()
{
    HRESULT hr = S_OK;
    ID3DBlob* errorBlob;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
    //Vertex Shaders
    ID3DBlob* vsBlob;

    hr =  D3DCompileFromFile(L"SimpleShaders.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_main", "vs_5_0", dwShaderFlags, 0, &vsBlob, &errorBlob);
    if (FAILED(hr))
    {
        MessageBoxA(_windowHandle, (char*)errorBlob->GetBufferPointer(), nullptr, ERROR);
        errorBlob->Release();
        return hr;
    }

    hr = _device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &_vertexShader);

    hr = D3DCompileFromFile(L"SkyBox.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_main", "vs_5_0", dwShaderFlags, 0, &vsBlob, &errorBlob);
    if (FAILED(hr))
    {
        MessageBoxA(_windowHandle, (char*)errorBlob->GetBufferPointer(), nullptr, ERROR);
        errorBlob->Release();
        return hr;
    }

    hr = _device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &_skyBoxVertexShader);

    hr = D3DCompileFromFile(L"Water.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_main", "vs_5_0", dwShaderFlags, 0, &vsBlob, &errorBlob);
    if (FAILED(hr))
    {
        MessageBoxA(_windowHandle, (char*)errorBlob->GetBufferPointer(), nullptr, ERROR);
        errorBlob->Release();
        return hr;
    }

    hr = _device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &_waterVertexShader);


    if (FAILED(hr)) return hr;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    //Geometry Shaders
    ID3DBlob* gsBlob;
    hr = D3DCompileFromFile(L"Water.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "GS_main", "gs_5_0", dwShaderFlags, 0, &gsBlob, &errorBlob);
    if (FAILED(hr))
    {
        MessageBoxA(_windowHandle, (char*)errorBlob->GetBufferPointer(), nullptr, ERROR);
        errorBlob->Release();
        return hr;
    }

    hr = _device->CreateGeometryShader(gsBlob->GetBufferPointer(), gsBlob->GetBufferSize(), nullptr, &_waterGeometryShader);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    //Pixel Shaders
    ID3DBlob* psBlob;

    hr = D3DCompileFromFile(L"SimpleShaders.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_main", "ps_5_0", dwShaderFlags, 0, &psBlob, &errorBlob);
    if (FAILED(hr))
    {
        MessageBoxA(_windowHandle, (char*)errorBlob->GetBufferPointer(), nullptr, ERROR);
        errorBlob->Release();
        return hr;
    }

    hr = _device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &_pixelShader);

    

    hr = D3DCompileFromFile(L"SkyBox.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_main", "ps_5_0", dwShaderFlags, 0, &psBlob, &errorBlob);
    if (FAILED(hr))
    {
        MessageBoxA(_windowHandle, (char*)errorBlob->GetBufferPointer(), nullptr, ERROR);
        errorBlob->Release();
        return hr;
    }
    
    hr = _device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &_skyBoxPixelShader);

    //Setup input elements
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    hr = _device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &_inputLayout);
    if (FAILED(hr)) return hr;


    //Release all the blobs
    vsBlob->Release();
    psBlob->Release();
    gsBlob->Release();

    return hr;
}

HRESULT DX11Framework::InitVertexIndexBuffers()
{
    HRESULT hr = S_OK;
    //Hardcoded Cube
    SimpleVertex cubeVertexData[] =
    {
        //Position                         //Normals                         //Texcoord
        { XMFLOAT3(-1.00f,  1.00f, -1.0f), XMFLOAT3(-1.00f,  1.00f, -1.0f)  ,XMFLOAT2(0.0f,0.0f),},
        { XMFLOAT3(1.00f,  1.00f, -1.0f),  XMFLOAT3(1.00f,  1.00f, -1.0f)   ,XMFLOAT2(1.0f,0.0f),},
        { XMFLOAT3(-1.00f, -1.00f, -1.0f), XMFLOAT3(-1.00f, -1.00f, -1.0f)  ,XMFLOAT2(0.0f,1.0f),},
        { XMFLOAT3(1.00f, -1.00f, -1.0f),  XMFLOAT3(1.00f, -1.00f, -1.0f)   ,XMFLOAT2(1.0f,1.0f)},

        { XMFLOAT3(-1.00f,  1.00f, 1.0f), XMFLOAT3(-1.00f,  1.00f, 1.0f)    ,XMFLOAT2(1.0f,0.0f)},
        { XMFLOAT3(1.00f,  1.00f, 1.0f),  XMFLOAT3(1.00f,  1.00f, 1.0f)     ,XMFLOAT2(0.0f,0.0f)},
        { XMFLOAT3(-1.00f, -1.00f, 1.0f), XMFLOAT3(-1.00f, -1.00f, 1.0f)    ,XMFLOAT2(1.0f,1.0f)},
        { XMFLOAT3(1.00f, -1.00f, 1.0f),  XMFLOAT3(1.00f, -1.00f, 1.0f)     ,XMFLOAT2(0.0f,1.0f)},
    };

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = sizeof(cubeVertexData);
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA cubevertexData = { cubeVertexData };

    hr = _device->CreateBuffer(&vertexBufferDesc, &cubevertexData, &_cubeVertexBuffer);
    if (FAILED(hr)) return hr;

    //Load Objects and call initializer
    _gameObject = new GameObject(nullptr, OBJLoader::Load("Models\\sphere.obj", _device), XMFLOAT3(0, -3.0f, 0), XMFLOAT3(30.0f, 7.0f, 30.0f), XMFLOAT3(0, 0, 0));
    _water = new GameObject(nullptr, OBJLoader::Load("Models\\plane.obj", _device), XMFLOAT3(0, 0, 0), XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(0, 0, 0));
    _skyBox = new GameObject(nullptr, OBJLoader::Load("Models\\skybox.obj", _device, true), XMFLOAT3(0, 0, 0), XMFLOAT3(512, 512, 512), XMFLOAT3(0, 0, 0));
    _teapot = new GameObject(nullptr, OBJLoader::Load("Models\\teapot.obj", _device), XMFLOAT3(0, 10, 0), XMFLOAT3(1, 1, 1), XMFLOAT3(0, 0, XMConvertToRadians(20)),
        [](float dt, GameObject* self) 
        {
            static float count = 0;
            count += dt;
            self->SetWorldMat(XMFLOAT3(0,10,0),XMFLOAT3(1,1,1),XMFLOAT3(0,count,0));
        } //Scriptable Objects using function pointers
    );
    return hr;
}

HRESULT DX11Framework::InitPipelineVariables()
{
    HRESULT hr = S_OK;

    //Input Assembler
    _immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _immediateContext->IASetInputLayout(_inputLayout);

    //Rasterizer
    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;

    hr = _device->CreateRasterizerState(&rasterizerDesc, &_fillState);

    rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    hr = _device->CreateRasterizerState(&rasterizerDesc, &_wireframeState);
    if (FAILED(hr)) return hr;

    
    _immediateContext->RSSetState(_fillState);
    //_immediateContext->RSSetState(_wireframeState);

    //Viewport Values
    _viewport = { 0.0f, 0.0f, (float)_WindowWidth, (float)_WindowHeight, 0.0f, 1.0f };
    _immediateContext->RSSetViewports(1, &_viewport);

    //Constant Buffer
    D3D11_BUFFER_DESC constantBufferDesc = {};
    constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = _device->CreateBuffer(&constantBufferDesc, nullptr, &_constantBuffer);
    if (FAILED(hr)) { return hr; }

    _immediateContext->VSSetConstantBuffers(0, 1, &_constantBuffer);
    _immediateContext->PSSetConstantBuffers(0, 1, &_constantBuffer);

    D3D11_SAMPLER_DESC bilinearSamplerdesc = {};
    bilinearSamplerdesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    bilinearSamplerdesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    bilinearSamplerdesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    bilinearSamplerdesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    bilinearSamplerdesc.MaxLOD = 1;
    bilinearSamplerdesc.MinLOD = 0;

    hr = _device->CreateSamplerState(&bilinearSamplerdesc, &_bilinearSamplerState);
    if (FAILED(hr)) return hr;

    D3D11_DEPTH_STENCIL_DESC dsDescSkybox = {};
    dsDescSkybox.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    dsDescSkybox.DepthEnable = true;
    dsDescSkybox.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

    _device->CreateDepthStencilState(&dsDescSkybox, &_depthStencilSkybox);

    D3D11_BLEND_DESC blendDesc = {};

    D3D11_RENDER_TARGET_BLEND_DESC rtbd = {};
    rtbd.BlendEnable = true;
    rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
    rtbd.DestBlend = D3D11_BLEND_BLEND_FACTOR;
    rtbd.BlendOp = D3D11_BLEND_OP_SUBTRACT;
    rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0] = rtbd;

    _device->CreateBlendState(&blendDesc, &_transparencyBlendState);

    return S_OK;
}

HRESULT DX11Framework::InitRunTimeData()
{
    HRESULT hr = S_OK;

    // Lighting Data Yaml
    YAML::Node nd = YAML::LoadFile("ResourceFiles\\LightingData.yaml");

    const YAML::Node& materials = nd["Materials"];
    
    const YAML::Node& ambientTypeM = materials[0];
    std::vector<float> col = ambientTypeM["rgba"].as<std::vector<float>>();
    _ambientMaterial = XMFLOAT4(col[0], col[1], col[2], col[3]);

    const YAML::Node& diffuseTypeM = materials[1];
    col = diffuseTypeM["rgba"].as<std::vector<float>>();
    _diffuseMaterial = XMFLOAT4(col[0], col[1], col[2], col[3]);
    
    const YAML::Node& specularTypeM = materials[2];
    col = specularTypeM["rgba"].as<std::vector<float>>();
    _specularMaterial = XMFLOAT4(col[0], col[1], col[2], col[3]);

    const YAML::Node& lighting = nd["Lighting"];

    const YAML::Node& ambientTypeL = lighting[0];
    col = ambientTypeL["rgba"].as<std::vector<float>>();
    _ambientLight = XMFLOAT4(col[0], col[1], col[2], col[3]);

    const YAML::Node& diffuseTypeL = lighting[1];
    col = diffuseTypeL["rgba"].as<std::vector<float>>();
    _diffuseLight = XMFLOAT4(col[0], col[1], col[2], col[3]);

    const YAML::Node& specularTypeL = lighting[2];
    col = specularTypeL["rgba"].as<std::vector<float>>();
    _specularLight = XMFLOAT4(col[0], col[1], col[2], col[3]);

    const YAML::Node& specularPowerType = lighting[3];
    col = specularPowerType["power"].as<std::vector<float>>();
    _specularPower = col[0];

    const YAML::Node& directionTypeL = lighting[4];
    std::vector<float> col3 = directionTypeL["xyz"].as<std::vector<float>>();
    _lightDir = XMFLOAT3(col3[0], col3[1], col3[2]);
    
    //Camera
    _camera = new Camera(_viewport);
    _staticCamera[0] = new Camera(_viewport,
        XMFLOAT3(0.0f, 10.0f, -20.0f),
        XMFLOAT3(0, 0, 1),
        XMFLOAT3(0, 1, 0),
        STATIC);
    _staticCamera[1] = new Camera(_viewport,
        XMFLOAT3(20.0f, 10.0f, 0.0f),
        XMFLOAT3(-1, 0, 0),
        XMFLOAT3(0, 1, 0),
        STATIC);
    _staticCamera[2] = new Camera(_viewport,
        XMFLOAT3(-20.0f, 10.0f, 0.0f),
        XMFLOAT3(1, 0, 0),
        XMFLOAT3(0, 1, 0),
        STATIC);
    //ShaderToggleVarInit
    _effectToggle = 0;

    //Textures
    hr = CreateDDSTextureFromFile(_device, L"Textures\\ManyTextures\\water.dds", nullptr, &_waterTexture);
    if (FAILED(hr)) return hr;
    hr = CreateDDSTextureFromFile(_device, L"Textures\\ManyTextures\\sand.dds", nullptr, &_sandTexture);
    if (FAILED(hr)) return hr;
    hr = CreateDDSTextureFromFile(_device, L"Textures\\skybox.dds", nullptr, &_skyboxTexture);
    if (FAILED(hr)) return hr;
    hr = CreateDDSTextureFromFile(_device, L"Textures\\teapot.dds", nullptr, &_teapotTexture);
    
    //gameObject Resource Settings
    _gameObject->SetShaderResource(_sandTexture);
    _teapot->SetShaderResource(_teapotTexture);
    _water->SetShaderResource(_waterTexture);
    _skyBox->SetShaderResource(_skyboxTexture);

    return S_OK;
}

DX11Framework::~DX11Framework()
{
    if(_immediateContext)_immediateContext->Release();
    if(_device)_device->Release();
    if(_dxgiDevice)_dxgiDevice->Release();
    if(_dxgiFactory)_dxgiFactory->Release();
    if(_frameBufferView)_frameBufferView->Release();
    if(_swapChain)_swapChain->Release();

    if(_fillState)_fillState->Release();
    if (_wireframeState)_wireframeState->Release();
    if(_vertexShader)_vertexShader->Release();
    if(_inputLayout)_inputLayout->Release();
    if(_pixelShader)_pixelShader->Release();
    if(_constantBuffer)_constantBuffer->Release();

    if (_depthStencilBuffer)_depthStencilBuffer->Release();
    if (_depthStencilView)_depthStencilView->Release();
}


void DX11Framework::Update()
{  
    //Delta Time Calculation
    static ULONGLONG frameStart = GetTickCount64();

    ULONGLONG frameNow = GetTickCount64();
    float deltaTime = (frameNow - frameStart) / 1000.0f;
    frameStart = frameNow;

    //Simple Count
    static float simpleCount = 0.0f;
   
    simpleCount += deltaTime;
    
    _cbData.Count = simpleCount;
    _cbData.EffectToggle = _effectToggle;

    //Skybox Camera Follow
    XMFLOAT3 playerPosition;
    XMStoreFloat3(&playerPosition, XMMatrixInverse(nullptr,XMLoadFloat4x4(_camera->GetViewMatrix())).r[3]);
    
    _skyBox->SetWorldMat(playerPosition,
        XMFLOAT3(512, 512, 512),
        XMFLOAT3(0.0f, 0.0f, 0.0f));
    
    //Scroll Through Shader Effects
    static bool tempToggle = true;
    if ((GetAsyncKeyState(VK_F1) & 0x0001))
    {
            _effectToggle++; 
    }
    if ((GetAsyncKeyState(VK_F2) & 0x0001))
    {
            _effectToggle--;
    }
    //Wireframe Render State Toggle
    if ((GetAsyncKeyState(VK_F7) & 0x0001))
    {
        _immediateContext->RSSetState(tempToggle == true ? _wireframeState : _fillState);
        tempToggle = !tempToggle;
    }

    //Camera update
    _camera->Update(deltaTime);

    //Update Objects
    _gameObject->Update(deltaTime);
    _water->Update(deltaTime);
    _skyBox->Update(deltaTime);
    _teapot->Update(deltaTime);
}

void DX11Framework::Draw()
{    
    //ClearColor, Setup depth stencil
    float backgroundColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };  
    _immediateContext->OMSetRenderTargets(1, &_frameBufferView, _depthStencilView);
    _immediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
    _immediateContext->ClearRenderTargetView(_frameBufferView, backgroundColor);
   
   
    //Store data in constant buffer struct
    
    _cbData.DiffuseLight = _diffuseLight;
    _cbData.DiffuseMaterial = _diffuseMaterial;
    _cbData.AmbientLight = _ambientLight;
    _cbData.AmbientMaterial = _ambientMaterial;
    _cbData.LightDir = _lightDir;
    _cbData.SpecularLight = _specularLight;
    _cbData.SpecularMaterial = _specularMaterial;
    _cbData.SpecularPower = _specularPower;

    static uint8_t cameraToggle = 0;
    if ((GetAsyncKeyState(VK_F8) & 0x0001))
        cameraToggle++;
    if ((GetAsyncKeyState(VK_F9) & 0x0001))
        cameraToggle--;
    if (cameraToggle > 0 && cameraToggle < 4) 
    {
       
        _cbData.View = XMMatrixTranspose(XMLoadFloat4x4(_staticCamera[cameraToggle - 1]->GetViewMatrix()));
        _cbData.Projection = XMMatrixTranspose(XMLoadFloat4x4(_staticCamera[cameraToggle - 1]->GetProjectionMatrix()));
        XMStoreFloat3(&_cbData.CameraPosition, (XMLoadFloat4x4(_staticCamera[cameraToggle - 1]->GetViewMatrix()).r[3]));

    }
    else 
    {
        _cbData.View = XMMatrixTranspose(XMLoadFloat4x4(_camera->GetViewMatrix()));
        _cbData.Projection = XMMatrixTranspose(XMLoadFloat4x4(_camera->GetProjectionMatrix()));
        XMStoreFloat3(&_cbData.CameraPosition, (XMLoadFloat4x4(_camera->GetViewMatrix()).r[3]));

    }

    //Draw Regular Objects
    _immediateContext->OMSetBlendState(0, 0, 0xffffffff);
    _gameObject->Draw(_immediateContext, 
      &_cbData, 
      _constantBuffer, 
      _vertexShader, 
      _pixelShader, 
      _bilinearSamplerState,
      _waterGeometryShader);//to Recalculate Normals because it seemed like they werent correct for a flat disk
    _teapot->Draw(_immediateContext,
      &_cbData,
      _constantBuffer,
      _vertexShader,
      _pixelShader,
      _bilinearSamplerState);
    //Ensures Transparency is active for only the scenes that need it
    if (_effectToggle <= 1)
    {
        FLOAT blendFactor[4] = { 0.25f, 0.25f,0.25f,1.0f };
        _immediateContext->OMSetBlendState(_transparencyBlendState, blendFactor, 0xffffffff);
    }
    _water->Draw(_immediateContext,
      &_cbData,
      _constantBuffer,
      _waterVertexShader,
      _pixelShader,
      _bilinearSamplerState,
      _waterGeometryShader); //Optional Geometry Shader
    _immediateContext->OMSetBlendState(0, 0, 0xffffffff);

    //Skybox Draw
    _immediateContext->OMSetDepthStencilState(_depthStencilSkybox, 0);
    _skyBox->Draw(_immediateContext,
        &_cbData,
        _constantBuffer,
        _skyBoxVertexShader,
        _skyBoxPixelShader,
        _bilinearSamplerState);

    //Present Backbuffer to screen
    _swapChain->Present(0, 0);
}