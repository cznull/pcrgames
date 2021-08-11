#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <gl/glew.h>
#include <opencv2/opencv.hpp>
#include <immintrin.h>

#ifdef _DEBUG
#pragma comment(lib,"opencv_world401d.lib")
#else
#pragma comment(lib,"opencv_world401.lib")
#endif 

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"glew32.lib")

#define MAX_LOADSTRING 100

struct target_t {
    float* data;
    float* alpha;
    int w, h;
};

struct snake_t {
    int region;
    int pos;
    int count;
    int point;
};

struct color_t {
    int r, g, b;
};

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

float inf = 1.0f / 0.0f;

ULONGLONG ts = 0;
HDC hdc1, hdc2;
HGLRC m_hrc;
int mx, my, cx, cy;
double ang1, ang2, len, cenx, ceny, cenz;
GLuint imgtex;

HWND bbhwnd;
HWND bbhwndmessage;
HDC bbhdc, bbhdcc;
HBITMAP hbitmap;
BITMAP bmp;
RECT bbrect;
unsigned char* data;

cv::Mat img, alpha;
target_t screen, flag, cursor;
target_t screenr, flagr, cursorr;

int start = 0;
uint64_t tc = 0, tl = 0;

int lastcursorx=0;
uint64_t lastcursort = 0, lastclicktime = 0;
double speed=0;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void draw(void) {

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClear(0x00004100);

    glBindTexture(GL_TEXTURE_2D, imgtex);
    glBegin(GL_QUADS);
    glColor3f(1.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0, 1.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(1.0, 1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0, -1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0, -1.0f, 0.0f);
    glEnd();

    SwapBuffers(hdc1);
}

int feverpoint = 0;
int colordistance(color_t a, color_t b) {
    return (a.r - b.r) * (a.r - b.r) + (a.g - b.g) * (a.g - b.g) + (a.b - b.b) * (a.b - b.b);
}

int colortosnaketype(uchar* data) {
    color_t c;
    c.b = data[0];
    c.g = data[1];
    c.r = data[2];

    if (colordistance(c, { 90,97,49 }) < 20) {
        return 1;
    }
    if (colordistance(c, { 66,77,165 }) < 20) {
        return 2;
    }
    if (colordistance(c, { 123,48,57 }) < 20) {
        return 3;
    }
    return 0;
}

int checksnakepoint(uchar* data, int x) {
    for (int i = 750; i < 765; i++) {
        color_t light;
        light.r = data[(i * bmp.bmWidth + x) * 4 + 2];
        light.g = data[(i * bmp.bmWidth + x) * 4 + 1];
        light.b = data[(i * bmp.bmWidth + x) * 4 + 0];
        if (light.r > 230 && light.g > 230 && light.b < 50) {
            return 1;
        }
    }
    return 0;
}

int getsnakeseq(uchar* data,std::vector<snake_t> &ss) {
    int lastlaftpos=-1;
    int lastrightpos = -1;

    for (int i = 100; i < 960; i++) {
        int type;
        type = colortosnaketype(data + (730 * bmp.bmWidth + 959 - i) * 4);
        if (type && (lastlaftpos < 0 || i - lastlaftpos>180)) {
            snake_t snake;
            snake.count = type;
            snake.pos = i;
            snake.region = 0;
            snake.point = 0;
            snake.point = checksnakepoint(data, 959 - i);
            ss.push_back(snake);
            lastlaftpos = i;
        }

        type = colortosnaketype(data + (730 * bmp.bmWidth + 960 + i) * 4);
        if (type && (lastlaftpos < 0 || i - lastlaftpos>180)) {
            snake_t snake;
            snake.count = type;
            snake.pos = i;
            snake.region = 1;
            snake.point = 0;
            snake.point = checksnakepoint(data, 960 + i);
            ss.push_back(snake);
            lastlaftpos = i;
        }
    }
    return 0;
}

int execclick(std::vector<snake_t>& ss) {
    tl = GetTickCount64();
    for (int i = 0; i < ss.size(); i++) {
        int clickpos = 65536 * 500 + 480 + 960 * ss[i].region;
        for (int j = 0; j < ss[i].count; j++) {
            PostMessage(bbhwndmessage, WM_LBUTTONDOWN, 1, clickpos);
            PostMessage(bbhwndmessage, WM_LBUTTONUP, 1, clickpos);
            Sleep(35);
            while ((tc = GetTickCount64()) < tl + 60);
            tl = tc;
            //printf("%d,", GetTickCount64() - ts);
        }
        feverpoint += ss[i].point;
        //printf("\n%d\n", feverpoint);

        if (feverpoint == 4) {
            tl = GetTickCount64();
            Sleep(500);
            while (GetTickCount64() - tl < 7000) {
                PostMessage(bbhwndmessage, WM_LBUTTONDOWN, 1, clickpos);
                PostMessage(bbhwndmessage, WM_LBUTTONUP, 1, clickpos);
                Sleep(10);
            }
            Sleep(1300);
            printf("resume\n");
            feverpoint = 0;
            break;
        }
    }
    return 0;
}

int game2(uchar* data) {
    std::vector<snake_t> ss;
    getsnakeseq(data, ss);
    execclick(ss);


    Sleep(350);
    return 0;
}

int main(int argc,char** argv){

    HINSTANCE hInstance = ::GetModuleHandle(NULL);
    int nCmdShow = SW_SHOW;

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PCRCLICK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PCRCLICK));

    MSG msg;

    ts = GetTickCount64();
    // Main message loop:
    for (;;) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else {
            if (bbhwnd) {
                //BitBlt(bbhdcc, 0, 0, bbrect.right - bbrect.left, bbrect.bottom - bbrect.top, bbhdc, 0, 0, SRCCOPY);
                PrintWindow(bbhwnd, bbhdcc, PW_CLIENTONLY);
                GetBitmapBits(hbitmap, bmp.bmHeight * bmp.bmWidthBytes, data);
                game2(data);
            }
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PCRCLICK));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PCRCLICK);
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

BOOL CALLBACK quitfullscreenProc(HWND hwnd, LPARAM lParam)
{
    char szTitle[MAX_PATH] = { 0 };
    char szClass[MAX_PATH] = { 0 };
    int nMaxCount = MAX_PATH;

    LPSTR lpClassName = szClass;
    LPSTR lpWindowName = szTitle;

    GetWindowTextA(hwnd, lpWindowName, nMaxCount);
    GetClassNameA(hwnd, lpClassName, nMaxCount);
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    if (lpWindowName[0] == 'T') {
        bbhwndmessage = hwnd;
    }
    return true;
}

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
    case WM_CREATE: {
        int i, j;
        PIXELFORMATDESCRIPTOR pfd = {
            sizeof(PIXELFORMATDESCRIPTOR),
            1,
            PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_STEREO,
            PFD_TYPE_RGBA,
            24,
            0,0,0,0,0,0,0,0,
            0,
            0,0,0,0,
            32,
            0,0,
            PFD_MAIN_PLANE,
            0,0,0,0
        };
        hdc1 = GetDC(hWnd);
        hdc2 = GetDC(NULL);
        int uds = ::ChoosePixelFormat(hdc1, &pfd);
        ::SetPixelFormat(hdc1, uds, &pfd);
        m_hrc = ::wglCreateContext(hdc1);
        ::wglMakeCurrent(hdc1, m_hrc);
        //glewInit();
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_VERTEX_ARRAY);

        glGenTextures(1, &imgtex);
        glBindTexture(GL_TEXTURE_2D, imgtex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        bbhwnd = FindWindowA(NULL, "雷电模拟器");
        EnumChildWindows(bbhwnd, quitfullscreenProc, 0);
        bbhdc = GetDC(bbhwnd);
        bbhdcc = CreateCompatibleDC(bbhdc);
        GetClientRect(bbhwnd, &bbrect);
        hbitmap = CreateCompatibleBitmap(bbhdc, bbrect.right - bbrect.left, bbrect.bottom - bbrect.top);
        SelectObject(bbhdcc, hbitmap);
        //BitBlt(bbhdcc, 0, 0, bbrect.right - bbrect.left, bbrect.bottom - bbrect.top, bbhdc, 0, 0, SRCCOPY);
        PrintWindow(bbhwnd, bbhdcc, PW_CLIENTONLY);
        GetObject(hbitmap, sizeof(BITMAP), &bmp);
        //frame = new cv::Mat(bmp.bmHeight, bmp.bmWidth, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        data = (unsigned char*)malloc(bmp.bmHeight * bmp.bmWidth * 4);


        GetBitmapBits(hbitmap, bmp.bmHeight * bmp.bmWidthBytes, data);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmp.bmWidth, bmp.bmHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
        RECT ra, rb;
        GetClientRect(hWnd, &ra);
        GetWindowRect(hWnd, &rb);
        MoveWindow(hWnd, 1000, 0, bmp.bmWidth / 4 + rb.right - ra.right + ra.left - rb.left, bmp.bmHeight / 4 + rb.bottom - ra.bottom + ra.top - rb.top, true);

        break;
    }
    case WM_SIZE: {
        cx = lParam & 0xffff;
        cy = (lParam & 0xffff0000) >> 16;
        glViewport(0, 0, cx, cy);
        break;
    }
    case WM_LBUTTONDOWN: {
        for(int i=0;i<1000;i++){
            PostMessage(bbhwndmessage, WM_LBUTTONDOWN, 1, 10223939);
            PostMessage(bbhwndmessage, WM_LBUTTONUP, 1, 10223939);
        }
        break;
    }
    case WM_KEYDOWN: {
        break;
    }
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
