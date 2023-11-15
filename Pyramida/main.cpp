//--------------------------------------------------------------------------------------
// Ìàòðèöû. Îñíîâàí íà ïðèìåðå èç DX SDK (c) Microsoft Corp.
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
// Ñòðóêòóðû в
//--------------------------------------------------------------------------------------

// Ñòðóêòóðà âåðøèíû
struct SimpleVertex
{
    XMFLOAT3 Pos;	// Êîîðäèíàòû òî÷êè â ïðîñòðàíñòâå
    XMFLOAT4 Color;	// Òåïåðü êàæäàÿ âåðøèíà áóäåò ñîäåðæàòü èíôîðìàöèþ î öâåòå
};


// Ñòðóêòóðà êîíñòàíòíîãî áóôåðà (ñîâïàäàåò ñî ñòðóêòóðîé â øåéäåðå)
struct ConstantBuffer
{
    
	XMMATRIX mWorld;		// Ìàòðèöà ìèðà
	XMMATRIX mView;			// Ìàòðèöà âèäà
	XMMATRIX mProjection;	// Ìàòðèöà ïðîåêöèè
};


//--------------------------------------------------------------------------------------
// Ãëîáàëüíûå ïåðåìåííûå
//--------------------------------------------------------------------------------------
HINSTANCE               g_hInst = NULL;
HWND                    g_hWnd = NULL;

D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = NULL;		// Óñòðîéñòâî (äëÿ ñîçäàíèÿ îáúåêòîâ)
ID3D11DeviceContext*    g_pImmediateContext = NULL;	// Êîíòåêñò (óñòðîéñòâî ðèñîâàíèÿ)
IDXGISwapChain*         g_pSwapChain = NULL;		// Öåïü ñâÿçè (áóôåðà ñ ýêðàíîì)
ID3D11RenderTargetView* g_pRenderTargetView = NULL;	// Îáúåêò çàäíåãî áóôåðà

ID3D11VertexShader*     g_pVertexShader = NULL;		// Âåðøèííûé øåéäåð
ID3D11PixelShader*      g_pPixelShader = NULL;		// Ïèêñåëüíûé øåéäåð
ID3D11InputLayout*      g_pVertexLayout = NULL;		// Îïèñàíèå ôîðìàòà âåðøèí
ID3D11Buffer*           g_pVertexBuffer = NULL;		// Áóôåð âåðøèí
ID3D11Buffer*           g_pIndexBuffer = NULL;		// Áóôåð èíäåêñîâ âåðøèí
ID3D11Buffer*           g_pConstantBuffer = NULL;	// Êîíñòàíòíûé áóôåð

XMMATRIX                g_World;					// Ìàòðèöà ìèðà
XMMATRIX                g_View;						// Ìàòðèöà âèäà
XMMATRIX                g_Projection;				// Ìàòðèöà ïðîåêöèè


//--------------------------------------------------------------------------------------
// Ïðåäâàðèòåëüíûå îáúÿâëåíèÿ ôóíêöèé
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );  // Ñîçäàíèå îêíà
HRESULT InitDevice();	// Èíèöèàëèçàöèÿ óñòðîéñòâ DirectX
HRESULT InitGeometry();	// Èíèöèàëèçàöèÿ øàáëîíà ââîäà è áóôåðà âåðøèí
HRESULT InitMatrixes();	// Èíèöèàëèçàöèÿ ìàòðèö
void SetMatrixes();		// Îáíîâëåíèå ìàòðèöû ìèðà
void Render();			// Ôóíêöèÿ ðèñîâàíèÿ
void CleanupDevice();	// Óäàëåíèå ñîçäàíííûõ óñòðîéñòâ DirectX
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );	  // Ôóíêöèÿ îêíà


//--------------------------------------------------------------------------------------
// Òî÷êà âõîäà â ïðîãðàììó. Èíèöèàëèçàöèÿ âñåõ îáúåêòîâ è âõîä â öèêë ñîîáùåíèé.
// Ñâîáîäíîå âðåìÿ èñïîëüçóåòñÿ äëÿ îòðèñîâêè ñöåíû.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

	// Ñîçäàíèå îêíà ïðèëîæåíèÿ
    if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

	// Ñîçäàíèå îáúåêòîâ DirectX
    if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

	// Ñîçäàíèå øåéäåðîâ è áóôåðà âåðøèí
    if( FAILED( InitGeometry() ) )
    {
        CleanupDevice();
        return 0;
    }

	// Èíèöèàëèçàöèÿ ìàòðèö
    if( FAILED( InitMatrixes() ) )
    {
        CleanupDevice();
        return 0;
    }
 

    // Ãëàâíûé öèêë ñîîáùåíèé
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
			SetMatrixes(); // Îáíîâèòü ìàòðèöó ìèðà
            Render();
        }
                    
    }
	// Îñâîáîæäàåì îáúåêòû DirectX
    CleanupDevice();

    return ( int )msg.wParam;
}


//--------------------------------------------------------------------------------------
// Ðåãèñòðàöèÿ êëàññà è ñîçäàíèå îêíà
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Ðåãèñòðàöèÿ êëàññà
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

    // Ñîçäàíèå îêíà
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
// Âûçûâàåòñÿ êàæäûé ðàç, êîãäà ïðèëîæåíèå ïîëó÷àåò ñèñòåìíîå ñîîáùåíèå
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
// Âñïîìîãàòåëüíàÿ ôóíêöèÿ äëÿ êîìïèëÿöèè øåéäåðîâ â D3DX11
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
// Ñîçäàíèå óñòðîéñòâà Direct3D (D3D Device), ñâÿçóþùåé öåïè (Swap Chain) è
// êîíòåêñòà óñòðîéñòâà (Immediate Context).
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect( g_hWnd, &rc );
    UINT width = rc.right - rc.left;	// ïîëó÷àåì øèðèíó
    UINT height = rc.bottom - rc.top;	// è âûñîòó îêíà

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

    // Òóò ìû ñîçäàåì ñïèñîê ïîääåðæèâàåìûõ âåðñèé DirectX
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

	// Ñåé÷àñ ìû ñîçäàäèì óñòðîéñòâà DirectX. Äëÿ íà÷àëà çàïîëíèì ñòðóêòóðó,
	// êîòîðàÿ îïèñûâàåò ñâîéñòâà ïåðåäíåãî áóôåðà è ïðèâÿçûâàåò åãî ê íàøåìó îêíó.
    DXGI_SWAP_CHAIN_DESC sd;			// Ñòðóêòóðà, îïèñûâàþùàÿ öåïü ñâÿçè (Swap Chain)
    ZeroMemory( &sd, sizeof( sd ) );	// î÷èùàåì åå
	sd.BufferCount = 1;					// ó íàñ îäèí áóôåð
    sd.BufferDesc.Width = width;		// øèðèíà áóôåðà
    sd.BufferDesc.Height = height;		// âûñîòà áóôåðà
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// ôîðìàò ïèêñåëÿ â áóôåðå
    sd.BufferDesc.RefreshRate.Numerator = 75;			// ÷àñòîòà îáíîâëåíèÿ ýêðàíà
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// íàçíà÷åíèå áóôåðà - çàäíèé áóôåð
    sd.OutputWindow = g_hWnd;							// ïðèâÿçûâàåì ê íàøåìó îêíó
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;						// íå ïîëíîýêðàííûé ðåæèì

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain( NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
                                            D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );
        if (SUCCEEDED(hr))  // Åñëè óñòðîéñòâà ñîçäàíû óñïåøíî, òî âûõîäèì èç öèêëà
            break;
    }
    if (FAILED(hr)) return hr;

    // Òåïåðü ñîçäàåì çàäíèé áóôåð. Îáðàòèòå âíèìàíèå, â SDK
    // RenderTargetOutput - ýòî ïåðåäíèé áóôåð, à RenderTargetView - çàäíèé.

	// Èçâëåêàåì îïèñàíèå çàäíåãî áóôåðà
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if (FAILED(hr)) return hr;

	// Ïî ïîëó÷åííîìó îïèñàíèþ ñîçäàåì ïîâåðõíîñòü ðèñîâàíèÿ
    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );
    pBackBuffer->Release();
    if (FAILED(hr)) return hr;

    // Ïîäêëþ÷àåì îáúåêò çàäíåãî áóôåðà ê êîíòåêñòó óñòðîéñòâà
    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, NULL );

    // Óñòàíîâêè âüþïîðòà (ìàñøòàá è ñèñòåìà êîîðäèíàò). Â ïðåäûäóùèõ âåðñèÿõ îí ñîçäàâàëñÿ
	// àâòîìàòè÷åñêè, åñëè íå áûë çàäàí ÿâíî.
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
// Ñîçäàíèå áóôåðà âåðøèí, øåéäåðîâ (shaders) è îïèñàíèÿ ôîðìàòà âåðøèí (input layout)
//--------------------------------------------------------------------------------------
HRESULT InitGeometry()
{
	HRESULT hr = S_OK;

	// Êîìïèëÿöèÿ âåðøèííîãî øåéäåðà èç ôàéëà
    ID3DBlob* pVSBlob = NULL; // Âñïîìîãàòåëüíûé îáúåêò - ïðîñòî ìåñòî â îïåðàòèâíîé ïàìÿòè
    hr = CompileShaderFromFile( (WCHAR*)L"pyramid.fx", "VS", "vs_4_0", &pVSBlob );
    if (FAILED(hr))
    {
        MessageBox( NULL, L"Íåâîçìîæíî ñêîìïèëèðîâàòü ôàéë FX. Ïîæàëóéñòà, çàïóñòèòå äàííóþ ïðîãðàììó èç ïàïêè, ñîäåðæàùåé ôàéë FX.", L"Îøèáêà", MB_OK );
        return hr;
    }

	// Ñîçäàíèå âåðøèííîãî øåéäåðà
	hr = g_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader );
    if (FAILED(hr))
	{	
		pVSBlob->Release();
        return hr;
	}

    // Îïðåäåëåíèå øàáëîíà âåðøèí
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE( layout );

    // Ñîçäàíèå øàáëîíà âåðøèí
	hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                                          pVSBlob->GetBufferSize(), &g_pVertexLayout );
	pVSBlob->Release();
	if (FAILED(hr)) return hr;

    // Ïîäêëþ÷åíèå øàáëîíà âåðøèí
    g_pImmediateContext->IASetInputLayout( g_pVertexLayout );

	// Êîìïèëÿöèÿ ïèêñåëüíîãî øåéäåðà èç ôàéëà
	ID3DBlob* pPSBlob = NULL;
    hr = CompileShaderFromFile( (WCHAR*)L"pyramid.fx", "PS", "ps_4_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL, L"Íåâîçìîæíî ñêîìïèëèðîâàòü ôàéë FX. Ïîæàëóéñòà, çàïóñòèòå äàííóþ ïðîãðàììó èç ïàïêè, ñîäåðæàùåé ôàéë FX.", L"Îøèáêà", MB_OK );
        return hr;
    }

	// Ñîçäàíèå ïèêñåëüíîãî øåéäåðà
	hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader );
	pPSBlob->Release();
	if (FAILED(hr)) return hr;

    // Ñîçäàíèå áóôåðà âåðøèí (ïÿòü óãëîâ ïèðàìèäû)
	SimpleVertex vertices[] =
    {	/* êîîðäèíàòû X, Y, Z				öâåò R, G, B, A					 */
        { XMFLOAT3(  0.0f,  1.5f,  0.0f ), XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) },
        { XMFLOAT3( -1.0f,  0.0f, -1.0f ), XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
        { XMFLOAT3(  1.0f,  0.0f, -1.0f ), XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f,  0.0f,  1.0f ), XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ) },
        { XMFLOAT3(  1.0f,  0.0f,  1.0f ), XMFLOAT4( 0.0f, 1.0f, 1.0f, 1.0f ) }
    };

	D3D11_BUFFER_DESC bd;	// Ñòðóêòóðà, îïèñûâàþùàÿ ñîçäàâàåìûé áóôåð
	ZeroMemory( &bd, sizeof(bd) );				// î÷èùàåì åå
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SimpleVertex ) * 5;	// ðàçìåð áóôåðà
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// òèï áóôåðà - áóôåð âåðøèí
	bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;	// Ñòðóêòóðà, ñîäåðæàùàÿ äàííûå áóôåðà
	ZeroMemory( &InitData, sizeof(InitData) );	// î÷èùàåì åå
    InitData.pSysMem = vertices;				// óêàçàòåëü íà íàøè 8 âåðøèí
	// Âûçîâ ìåòîäà g_pd3dDevice ñîçäàñò îáúåêò áóôåðà âåðøèí
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer );
	if (FAILED(hr)) return hr;

    // Ñîçäàíèå áóôåðà èíäåêñîâ:
	// Ñîçäàíèå ìàññèâà ñ äàííûìè
    WORD indices[] =
    {	// èíäåêñû ìàññèâà vertices[], ïî êîòîðûì ñòðîÿòñÿ òðåóãîëüíèêè
        0,2,1,	/* Òðåóãîëüíèê 1 = vertices[0], vertices[2], vertices[3] */
        0,3,4,	/* Òðåóãîëüíèê 2 = vertices[0], vertices[3], vertices[4] */
        0,1,3,	/* è ò. ä. */
        0,4,2,

        1,2,3,
        2,4,3,
    };
    bd.Usage = D3D11_USAGE_DEFAULT;		// Ñòðóêòóðà, îïèñûâàþùàÿ ñîçäàâàåìûé áóôåð
    bd.ByteWidth = sizeof( WORD ) * 18;	// äëÿ 6 òðåóãîëüíèêîâ íåîáõîäèìî 18 âåðøèí
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER; // òèï - áóôåð èíäåêñîâ
	bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;				// óêàçàòåëü íà íàø ìàññèâ èíäåêñîâ
	// Âûçîâ ìåòîäà g_pd3dDevice ñîçäàñò îáúåêò áóôåðà èíäåêñîâ
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pIndexBuffer );
	if (FAILED(hr)) return hr;

    // Óñòàíîâêà áóôåðà âåðøèí
    UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer, &stride, &offset );
    // Óñòàíîâêà áóôåðà èíäåêñîâ
    g_pImmediateContext->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
    // Óñòàíîâêà ñïîñîáà îòðèñîâêè âåðøèí â áóôåðå (â äàííîì ñëó÷àå - TRIANGLE LIST,
	// ò. å. òî÷êè 1-3 - ïåðâûé òðåóãîëüíèê, 4-6 - âòîðîé è ò. ä.
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// Ñîçäàíèå êîíñòàíòíîãî áóôåðà
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);		// ðàçìåð áóôåðà = ðàçìåðó ñòðóêòóðû
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	// òèï - êîíñòàíòíûé áóôåð
	bd.CPUAccessFlags = 0;
	// Âûçîâ ìåòîäà g_pd3dDevice ñîçäàñò îáúåêò êîíñòàíòíîãî áóôåðà
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pConstantBuffer );
	if (FAILED(hr)) return hr;

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Èíèöèàëèçàöèÿ ìàòðèö
//--------------------------------------------------------------------------------------
HRESULT InitMatrixes()
{
    RECT rc;
    GetClientRect( g_hWnd, &rc );
    UINT width = rc.right - rc.left;	// ïîëó÷àåì øèðèíó
    UINT height = rc.bottom - rc.top;	// è âûñîòó îêíà

	// Èíèöèàëèçàöèÿ ìàòðèöû ìèðà
	g_World = XMMatrixIdentity();

    // Èíèöèàëèçàöèÿ ìàòðèöû âèäà
	XMVECTOR Eye = XMVectorSet( 0.0f, 1.0f, -5.0f, 0.0f );	// Îòêóäà ñìîòðèì
	XMVECTOR At = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );	// Êóäà ñìîòðèì
	XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );	// Íàïðàâëåíèå âåðõà
	g_View = XMMatrixLookAtLH( Eye, At, Up );

    // Èíèöèàëèçàöèÿ ìàòðèöû ïðîåêöèè
	// Ïàðàìåòðû: 1) øèðèíà óãëà îáúåêòèâà 2) "êâàäðàòíîñòü" ïèêñåëÿ
	// 3) ñàìîå áëèæíåå âèäèìîå ðàññòîÿíèå 4) ñàìîå äàëüíåå âèäèìîå ðàññòîÿíèå
	g_Projection = XMMatrixPerspectiveFovLH( XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f );

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Îáíîâëåíèå ìàòðèö
//--------------------------------------------------------------------------------------
void SetMatrixes()
{
    // Îáíîâëåíèå ïåðåìåííîé-âðåìåíè
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

    // Âðàùàòü ìèð ïî îñè Y íà óãîë t (â ðàäèàíàõ)
	g_World = XMMatrixRotationY( t );

    // Îáíîâèòü êîíñòàíòíûé áóôåð
	// ñîçäàåì âðåìåííóþ ñòðóêòóðó è çàãðóæàåì â íåå ìàòðèöû
    ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose( g_World );
	cb.mView = XMMatrixTranspose( g_View );
	cb.mProjection = XMMatrixTranspose( g_Projection );
	// çàãðóæàåì âðåìåííóþ ñòðóêòóðó â êîíñòàíòíûé áóôåð g_pConstantBuffer
	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cb, 0, 0 );
}


//--------------------------------------------------------------------------------------
// Îñâîáîæäåíèå âñåõ ñîçäàííûõ îáúåêòîâ
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    // Ñíà÷àëà îòêëþ÷èì êîíòåêñò óñòðîéñòâà
    if( g_pImmediateContext ) g_pImmediateContext->ClearState();
	// Ïîòîì óäàëèì îáúåêòû
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
// Ðèñîâàíèå êàäðà
//--------------------------------------------------------------------------------------
void Render()
{
    // Î÷èñòèòü çàäíèé áóôåð
    float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // êðàñíûé, çåëåíûé, ñèíèé, àëüôà-êàíàë
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );

    // Íàðèñîâàòü ïèðàìèäêó
	g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
	g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
	g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pConstantBuffer );
    // Íàðèñîâàòü 18 èíäåêñèðîâàííûõ âåðøèí
	g_pImmediateContext->DrawIndexed( 18, 0, 0 );

    // Âûâåñòè â ïåðåäíèé áóôåð (íà ýêðàí) èíôîðìàöèþ, íàðèñîâàííóþ â çàäíåì áóôåðå.
    g_pSwapChain->Present( 0, 0 );
}

