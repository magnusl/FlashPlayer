#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Commdlg.h>
#include <iostream>
#include <GL\glew.h>
#include <GL\wglew.h>

#include <swf\CPlayer.h>
#include <avm2\CMMapSource.h>
#include <swf\CGLRenderer.h>

bool CreateContext(HDC hDC, HGLRC & hRC)
{
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize  = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 32;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int nFormat = ChoosePixelFormat(hDC, &pfd);
    if (nFormat == 0) {
        return false;
    }

#ifdef _DEBUG
    AllocConsole();
#endif
    // create a temporary context so that we can call various glew/opengl functions before 
    // creating the actual context.
    SetPixelFormat(hDC, nFormat, &pfd);
    HGLRC hRCTmp = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRCTmp);

    // init glew
    glewInit();

    // we want a OpenGL 3.2 context
    int attribs[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 2,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        0
    };
    
    // glewIsSupported("WGL_ARB_create_context") seems to not work correctly.
    if (!strcmp(wglGetExtensionsStringARB(hDC), "WGL_ARB_create_context")) {
        return false;
    }

    // create a new context and destroy the old one.
    hRC = wglCreateContextAttribsARB(hDC,0, attribs);
    wglMakeCurrent(NULL,NULL);
    wglDeleteContext(hRCTmp);
    wglMakeCurrent(hDC, hRC);

    return true;
}

static swf::IRenderer * g_renderer = 0;

LRESULT CALLBACK WndProc(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_SIZE:
        {
            unsigned w = LOWORD(lParam);
            unsigned h = HIWORD(lParam);
            if (g_renderer) {
                g_renderer->resize(w, h);
            }
            break;
        }
    case WM_DESTROY:
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

HWND CreatePrimaryWindow(int w, int h)
{
    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));
    wc.hInstance = GetModuleHandle(0);
    wc.lpszClassName = L"glwnd";
    wc.lpfnWndProc = WndProc;
    wc.style = CS_OWNDC;

    RegisterClass(&wc);

    RECT rect;
    rect.left = 0;
    rect.right = w;
    rect.top = 0;
    rect.bottom = h;

    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hWnd = CreateWindow(L"glwnd", L"ShockWave Flash", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
        CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, 0,0, wc.hInstance, 0);

    return hWnd;
}

double PCFreq = 0.0;
__int64 CounterStart = 0;

void StartCounter()
{
    LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li)) {
    }
    PCFreq = double(li.QuadPart)/1000.0;

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}
double GetCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart-CounterStart)/PCFreq;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    HGLRC hRC;
    HDC hDC;

    wchar_t szFile[MAX_PATH] ;
    OPENFILENAME ofn;

    ZeroMemory( &ofn , sizeof( ofn));
    ofn.lStructSize = sizeof ( ofn );
    ofn.hwndOwner = NULL ;
    ofn.lpstrFile = szFile ;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof( szFile );
    ofn.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
    ofn.nFilterIndex =1;
    ofn.lpstrFileTitle = NULL ;
    ofn.nMaxFileTitle = 0 ;
    ofn.lpstrInitialDir=NULL ;
    ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST ;
    if (!GetOpenFileName( &ofn )) {
        return -1;
    }

    std::shared_ptr<avm2::io::CFileSource> source;
    try {
        source = std::make_shared<avm2::io::CMMapSource>(szFile);
    } catch(...) {
        MessageBox(0, L"Failed to open .swf file.", L"Open Error", MB_OK | MB_ICONEXCLAMATION);
        return -1;
    }

    std::shared_ptr<swf::CPlayer> player = swf::CPlayer::CreatePlayer(source);
    if (!player) {
        MessageBox(0, L"Failed to create a suitable player.", L"Unsupported file", MB_OK | MB_ICONEXCLAMATION);
        return -1;
    }

    size_t width, height;
    player->GetStageSize(width, height);

    // create a OpenGL context before we can call any OpenGL/GLEW functions.
    HWND hWnd = CreatePrimaryWindow(width, height);
    if (hWnd) {
        hDC = GetDC(hWnd);
        if (!hDC) {
            return -1;
        }
        if (!CreateContext(hDC, hRC)) {
            return -1;
        }
    } else {
        return 0;
    }

    ShowWindow(hWnd, SW_SHOW);
    //wglMakeCurrent(hDC, hRC);

    // create a OpenGL renderer and initialize it.
    swf::ogl::CGLRenderer renderer;
    if (!renderer.initialize(width, height)) {
        MessageBox(hWnd, L"Failed to initialize renderer", L"Init error", MB_OK | MB_ICONEXCLAMATION);
        return -1;
    }

    g_renderer = &renderer;

    if (player->Initialize() != avm2::Err_Ok) {
        MessageBox(hWnd, L"Failed to initialize player.", L"Init error", MB_OK | MB_ICONEXCLAMATION);
        return -1;
    }

    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(1.0, 1.0, 1.0, 0);
    glDisable(GL_DEPTH_TEST);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (player->Play(0, 1.0f) != avm2::Err_Ok) {
        MessageBox(hWnd, L"Failed to start playback", L"Playback error", MB_OK | MB_ICONEXCLAMATION);
        return -1;
    }

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    StartCounter();
    float last = GetCounter();

    MSG msg;
    while( 1 ) {
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        if (PeekMessage(&msg, 0,0,0,PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        float current = GetCounter();
        if ((current - last) > 42) {
            player->Advance((current-last)/1000);
            last = current;
            player->Draw(renderer);

            renderer.flush();
            SwapBuffers(hDC);
        } else {
            Sleep(5);
        }
        player->GC();
    }
    return 0;
}