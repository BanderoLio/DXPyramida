//--------------------------------------------------------------------------------------
// �������. ������� �� ������� �� DX SDK (c) Microsoft Corp.
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
//#include "resource.h"
#include <xnamath.h>

#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

//--------------------------------------------------------------------------------------
// ���������
//--------------------------------------------------------------------------------------

// ��������� �������
struct SimpleVertex
{
    XMFLOAT3 Pos;	// ���������� ����� � ������������
    XMFLOAT4 Color;	// ������ ������ ������� ����� ��������� ���������� � �����
};


// ��������� ������������ ������ (��������� �� ���������� � �������)
struct ConstantBuffer
{
    
	XMMATRIX mWorld;		// ������� ����
	XMMATRIX mView;			// ������� ����
	XMMATRIX mProjection;	// ������� ��������
};


//--------------------------------------------------------------------------------------
// ���������� ����������
//--------------------------------------------------------------------------------------
HINSTANCE               g_hInst = NULL;
HWND                    g_hWnd = NULL;

D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = NULL;		// ���������� (��� �������� ��������)
ID3D11DeviceContext*    g_pImmediateContext = NULL;	// �������� (���������� ���������)
IDXGISwapChain*         g_pSwapChain = NULL;		// ���� ����� (������ � �������)
ID3D11RenderTargetView* g_pRenderTargetView = NULL;	// ������ ������� ������

ID3D11VertexShader*     g_pVertexShader = NULL;		// ��������� ������
ID3D11PixelShader*      g_pPixelShader = NULL;		// ���������� ������
ID3D11InputLayout*      g_pVertexLayout = NULL;		// �������� ������� ������
ID3D11Buffer*           g_pVertexBuffer = NULL;		// ����� ������
ID3D11Buffer*           g_pIndexBuffer = NULL;		// ����� �������� ������
ID3D11Buffer*           g_pConstantBuffer = NULL;	// ����������� �����

XMMATRIX                g_World;					// ������� ����
XMMATRIX                g_View;						// ������� ����
XMMATRIX                g_Projection;				// ������� ��������


//--------------------------------------------------------------------------------------
// ��������������� ���������� �������
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );  // �������� ����
HRESULT InitDevice();	// ������������� ��������� DirectX
HRESULT InitGeometry();	// ������������� ������� ����� � ������ ������
HRESULT InitMatrixes();	// ������������� ������
void SetMatrixes();		// ���������� ������� ����
void Render();			// ������� ���������
void CleanupDevice();	// �������� ���������� ��������� DirectX
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );	  // ������� ����


//--------------------------------------------------------------------------------------
// ����� ����� � ���������. ������������� ���� �������� � ���� � ���� ���������.
// ��������� ����� ������������ ��� ��������� �����.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

	// �������� ���� ����������
    if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

	// �������� �������� DirectX
    if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

	// �������� �������� � ������ ������
    if( FAILED( InitGeometry() ) )
    {
        CleanupDevice();
        return 0;
    }

	// ������������� ������
    if( FAILED( InitMatrixes() ) )
    {
        CleanupDevice();
        return 0;
    }
 

    // ������� ���� ���������
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
			SetMatrixes(); // �������� ������� ����
            Render();
        }
                    
    }
	// ����������� ������� DirectX
    CleanupDevice();

    return ( int )msg.wParam;
}


//--------------------------------------------------------------------------------------
// ����������� ������ � �������� ����
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // ����������� ������
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    //wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_ICON1 );
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"PyramidClass";
    //wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_ICON1 );
    wcex.hIconSm = NULL;
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // �������� ����
    g_hInst = hInstance;
    RECT rc = { 0, 0, 400, 300 };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	g_hWnd = CreateWindow( L"PyramidClass", L"Pyramid", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );
    if( !g_hWnd )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// ���������� ������ ���, ����� ���������� �������� ��������� ���������
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}


//--------------------------------------------------------------------------------------
// ��������������� ������� ��� ���������� �������� � D3DX11
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}


//--------------------------------------------------------------------------------------
// �������� ���������� Direct3D (D3D Device), ��������� ���� (Swap Chain) �
// ��������� ���������� (Immediate Context).
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect( g_hWnd, &rc );
    UINT width = rc.right - rc.left;	// �������� ������
    UINT height = rc.bottom - rc.top;	// � ������ ����

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE( driverTypes );

    // ��� �� ������� ������ �������������� ������ DirectX
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

	// ������ �� �������� ���������� DirectX. ��� ������ �������� ���������,
	// ������� ��������� �������� ��������� ������ � ����������� ��� � ������ ����.
    DXGI_SWAP_CHAIN_DESC sd;			// ���������, ����������� ���� ����� (Swap Chain)
    ZeroMemory( &sd, sizeof( sd ) );	// ������� ��
	sd.BufferCount = 1;					// � ��� ���� �����
    sd.BufferDesc.Width = width;		// ������ ������
    sd.BufferDesc.Height = height;		// ������ ������
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// ������ ������� � ������
    sd.BufferDesc.RefreshRate.Numerator = 75;			// ������� ���������� ������
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// ���������� ������ - ������ �����
    sd.OutputWindow = g_hWnd;							// ����������� � ������ ����
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;						// �� ������������� �����

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain( NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
                                            D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );
        if (SUCCEEDED(hr))  // ���� ���������� ������� �������, �� ������� �� �����
            break;
    }
    if (FAILED(hr)) return hr;

    // ������ ������� ������ �����. �������� ��������, � SDK
    // RenderTargetOutput - ��� �������� �����, � RenderTargetView - ������.

	// ��������� �������� ������� ������
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if (FAILED(hr)) return hr;

	// �� ����������� �������� ������� ����������� ���������
    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );
    pBackBuffer->Release();
    if (FAILED(hr)) return hr;

    // ���������� ������ ������� ������ � ��������� ����������
    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, NULL );

    // ��������� �������� (������� � ������� ���������). � ���������� ������� �� ����������
	// �������������, ���� �� ��� ����� ����.
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports( 1, &vp );

    return S_OK;
}

//--------------------------------------------------------------------------------------
// �������� ������ ������, �������� (shaders) � �������� ������� ������ (input layout)
//--------------------------------------------------------------------------------------
HRESULT InitGeometry()
{
	HRESULT hr = S_OK;

	// ���������� ���������� ������� �� �����
    ID3DBlob* pVSBlob = NULL; // ��������������� ������ - ������ ����� � ����������� ������
    hr = CompileShaderFromFile( (WCHAR*)L"pyramid.fx", "VS", "vs_4_0", &pVSBlob );
    if (FAILED(hr))
    {
        MessageBox( NULL, L"���������� �������������� ���� FX. ����������, ��������� ������ ��������� �� �����, ���������� ���� FX.", L"������", MB_OK );
        return hr;
    }

	// �������� ���������� �������
	hr = g_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader );
    if (FAILED(hr))
	{	
		pVSBlob->Release();
        return hr;
	}

    // ����������� ������� ������
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE( layout );

    // �������� ������� ������
	hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                                          pVSBlob->GetBufferSize(), &g_pVertexLayout );
	pVSBlob->Release();
	if (FAILED(hr)) return hr;

    // ����������� ������� ������
    g_pImmediateContext->IASetInputLayout( g_pVertexLayout );

	// ���������� ����������� ������� �� �����
	ID3DBlob* pPSBlob = NULL;
    hr = CompileShaderFromFile( (WCHAR*)L"pyramid.fx", "PS", "ps_4_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL, L"���������� �������������� ���� FX. ����������, ��������� ������ ��������� �� �����, ���������� ���� FX.", L"������", MB_OK );
        return hr;
    }

	// �������� ����������� �������
	hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader );
	pPSBlob->Release();
	if (FAILED(hr)) return hr;

    // �������� ������ ������ (���� ����� ��������)
	SimpleVertex vertices[] =
    {	/* ���������� X, Y, Z				���� R, G, B, A					 */
        { XMFLOAT3(  0.0f,  1.5f,  0.0f ), XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) },
        { XMFLOAT3( -1.0f,  0.0f, -1.0f ), XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
        { XMFLOAT3(  1.0f,  0.0f, -1.0f ), XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f,  0.0f,  1.0f ), XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ) },
        { XMFLOAT3(  1.0f,  0.0f,  1.0f ), XMFLOAT4( 0.0f, 1.0f, 1.0f, 1.0f ) }
    };

	D3D11_BUFFER_DESC bd;	// ���������, ����������� ����������� �����
	ZeroMemory( &bd, sizeof(bd) );				// ������� ��
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SimpleVertex ) * 5;	// ������ ������
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// ��� ������ - ����� ������
	bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;	// ���������, ���������� ������ ������
	ZeroMemory( &InitData, sizeof(InitData) );	// ������� ��
    InitData.pSysMem = vertices;				// ��������� �� ���� 8 ������
	// ����� ������ g_pd3dDevice ������� ������ ������ ������
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer );
	if (FAILED(hr)) return hr;

    // �������� ������ ��������:
	// �������� ������� � �������
    WORD indices[] =
    {	// ������� ������� vertices[], �� ������� �������� ������������
        0,2,1,	/* ����������� 1 = vertices[0], vertices[2], vertices[3] */
        0,3,4,	/* ����������� 2 = vertices[0], vertices[3], vertices[4] */
        0,1,3,	/* � �. �. */
        0,4,2,

        1,2,3,
        2,4,3,
    };
    bd.Usage = D3D11_USAGE_DEFAULT;		// ���������, ����������� ����������� �����
    bd.ByteWidth = sizeof( WORD ) * 18;	// ��� 6 ������������� ���������� 18 ������
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER; // ��� - ����� ��������
	bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;				// ��������� �� ��� ������ ��������
	// ����� ������ g_pd3dDevice ������� ������ ������ ��������
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pIndexBuffer );
	if (FAILED(hr)) return hr;

    // ��������� ������ ������
    UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer, &stride, &offset );
    // ��������� ������ ��������
    g_pImmediateContext->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
    // ��������� ������� ��������� ������ � ������ (� ������ ������ - TRIANGLE LIST,
	// �. �. ����� 1-3 - ������ �����������, 4-6 - ������ � �. �.
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// �������� ������������ ������
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);		// ������ ������ = ������� ���������
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	// ��� - ����������� �����
	bd.CPUAccessFlags = 0;
	// ����� ������ g_pd3dDevice ������� ������ ������������ ������
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pConstantBuffer );
	if (FAILED(hr)) return hr;

	return S_OK;
}


//--------------------------------------------------------------------------------------
// ������������� ������
//--------------------------------------------------------------------------------------
HRESULT InitMatrixes()
{
    RECT rc;
    GetClientRect( g_hWnd, &rc );
    UINT width = rc.right - rc.left;	// �������� ������
    UINT height = rc.bottom - rc.top;	// � ������ ����

	// ������������� ������� ����
	g_World = XMMatrixIdentity();

    // ������������� ������� ����
	XMVECTOR Eye = XMVectorSet( 0.0f, 1.0f, -5.0f, 0.0f );	// ������ �������
	XMVECTOR At = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );	// ���� �������
	XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );	// ����������� �����
	g_View = XMMatrixLookAtLH( Eye, At, Up );

    // ������������� ������� ��������
	// ���������: 1) ������ ���� ��������� 2) "������������" �������
	// 3) ����� ������� ������� ���������� 4) ����� ������� ������� ����������
	g_Projection = XMMatrixPerspectiveFovLH( XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f );

	return S_OK;
}


//--------------------------------------------------------------------------------------
// ���������� ������
//--------------------------------------------------------------------------------------
void SetMatrixes()
{
    // ���������� ����������-�������
    static float t = 0.0f;
    if( g_driverType == D3D_DRIVER_TYPE_REFERENCE )
    {
        t += ( float )XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();
        if( dwTimeStart == 0 )
            dwTimeStart = dwTimeCur;
        t = ( dwTimeCur - dwTimeStart ) / 1000.0f;
    }

    // ������� ��� �� ��� Y �� ���� t (� ��������)
	g_World = XMMatrixRotationY( t );

    // �������� ����������� �����
	// ������� ��������� ��������� � ��������� � ��� �������
    ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose( g_World );
	cb.mView = XMMatrixTranspose( g_View );
	cb.mProjection = XMMatrixTranspose( g_Projection );
	// ��������� ��������� ��������� � ����������� ����� g_pConstantBuffer
	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cb, 0, 0 );
}


//--------------------------------------------------------------------------------------
// ������������ ���� ��������� ��������
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    // ������� �������� �������� ����������
    if( g_pImmediateContext ) g_pImmediateContext->ClearState();
	// ����� ������ �������
    if( g_pConstantBuffer ) g_pConstantBuffer->Release();
    if( g_pVertexBuffer ) g_pVertexBuffer->Release();
    if( g_pIndexBuffer ) g_pIndexBuffer->Release();
    if( g_pVertexLayout ) g_pVertexLayout->Release();
    if( g_pVertexShader ) g_pVertexShader->Release();
    if( g_pPixelShader ) g_pPixelShader->Release();
    if( g_pRenderTargetView ) g_pRenderTargetView->Release();
    if( g_pSwapChain ) g_pSwapChain->Release();
    if( g_pImmediateContext ) g_pImmediateContext->Release();
    if( g_pd3dDevice ) g_pd3dDevice->Release();
}


//--------------------------------------------------------------------------------------
// ��������� �����
//--------------------------------------------------------------------------------------
void Render()
{
    // �������� ������ �����
    float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // �������, �������, �����, �����-�����
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );

    // ���������� ���������
	g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
	g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
	g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pConstantBuffer );
    // ���������� 18 ��������������� ������
	g_pImmediateContext->DrawIndexed( 18, 0, 0 );

    // ������� � �������� ����� (�� �����) ����������, ������������ � ������ ������.
    g_pSwapChain->Present( 0, 0 );
}

