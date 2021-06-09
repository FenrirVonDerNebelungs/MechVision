// HolloNet_v0_2.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "HolloNet_v0_2.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_HOLLONETV02, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HOLLONETV02));

    MSG msg;
    /*run debug test*/
    CTargaImage targImg;
    targImg.Init();
    targImg.Open("IOFiles/RoadPics/RoadFull0.tga");
    Img inImg;
    inImg.init(&targImg);/*copys data into inImg*/
    targImg.Close();
    targImg.Release();

    /*
    Hex hex0;
    hex0.Init(&inImg);
    hex0.Run();
    HokHex hex1;
    hex1.Init((HexBase*)&hex0);
    hex1.Run();
    DrawHexImg hexDraw;
    hexDraw.Init((HexBase*)(&hex0));
    hexDraw.Run();
    Img* drawDebugImg = hexDraw.getHexedImg();
    CTargaImage outImg;
    outImg.Init();
    outImg.Open(drawDebugImg->getImg(), drawDebugImg->getWidth(), drawDebugImg->getHeight());
    outImg.Write("IOFiles/debug0Hex.tga");
    outImg.Close();
    outImg.Release();
    hexDraw.Release();
    hex1.Release();
    hex0.Release();
    */
    /*              */
    /*
    HexStack hstack;
    hstack.Init(&inImg);
    hstack.Update(&inImg);
    NNetTreeTest NNtree;
    NNtree.Init(&hstack);
    NNtree.run();
    NNetDraw nDraw;
    nDraw.Init(&hstack, &NNtree);
    nDraw.Run();

    CTargaImage outImg;
    outImg.Init();
    Img* ndrawImg = nDraw.getNNImg();
    outImg.Open(ndrawImg->getImg(), ndrawImg->getWidth(), ndrawImg->getHeight());
    outImg.Write("IOFiles/debugRoadPat0.tga");
    outImg.Close();
    outImg.Release();

    nDraw.Release();
    NNtree.Release();
    hstack.Release();
    */
    HexStack hstack;
    hstack.Init(&inImg);
    hstack.Update(&inImg);
    PatternScan patScan;
    patScan.init(&hstack);
    patScan.eval();
    LineFinder linefind;
    linefind.init(&hstack, &patScan);
    linefind.spawn();
    LaneLineFinder lanelinefind;
    lanelinefind.init(&linefind, &inImg);
    lanelinefind.spawn();
    PatternDraw patDraw;
    patDraw.init(&patScan, &linefind, &lanelinefind);
    patDraw.run();

    CTargaImage outImg;
    outImg.Init();
    Img* ndrawImg = patDraw.getImg();
    outImg.Open(ndrawImg->getImg(), ndrawImg->getWidth(), ndrawImg->getHeight());
    outImg.Write("IOFiles/debugLineFinder00.tga");
    outImg.Close();
    outImg.Release();


    patDraw.release();
    lanelinefind.release();
    linefind.release();
    patScan.release();
    hstack.Release();

    inImg.release();

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HOLLONETV02));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_HOLLONETV02);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
