#include <Windows.h>
#include <iostream>
#include <malloc.h>
#include <vector>
#include <gl/glew.h>
#include "resource/resource.h"

#define MAX_LOADSTRING 100
//#ifdef DEBUG

struct snake_t {
    int region;
    int pos;
    int feverPos;
    int count;
    int point;
};

struct color_t {
    int r, g, b;
};
typedef unsigned char uchar;
// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
//---------------------------香织的灵魂空手道场--毒蛇清理大师---------------------------------------
/**
 * 雷电模拟器窗口上面的边距和右边的边距 1642 935
 * 我的情况是窗口大小是1642*935，画面大小是1600*900
 * 那么边距分别是42 和 35
 */
constexpr int WINDOW_TOP_BORDER = 42;
constexpr int WINDOW_RIGHT_BORDER = 35;
/**
 * 检测蛇灵敏度，太高太低都会不能很好抓到蛇的数量，自己按情况调整
 */
constexpr int SENSITIVITY = 20;
/**
 * 检测蛇下面的黄圈的位置比例信息
 */
constexpr double HIGHPOS = 0.67666667;
constexpr double LOWPOS = 0.7311111;
/**
 * 开启debug模式的开关
 */
constexpr bool debug = false;
//-----------------以上参数可以自己调整，除此之外一些方法里的constexpr变量也可以调整--------------------
HDC hdc1;
HGLRC m_hrc;
GLuint imgtex;

HWND simulatorHandle; //雷电模拟器的窗口
HWND simulatorMsgWndHandle; //雷电模拟器 T开头的子窗口
HDC simulatorDc, memDc; //内存设备内容
HDC testDc; //debug模式下的窗口内容句柄
HBITMAP hbitmap;
BITMAP bmp;
RECT simulatorRect;

unsigned char* data;

uint64_t tc = 0, tl = 0;


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
// Q:这个函数好像没用到，有什么用吗？ A：我也看不懂，我也不敢改
void draw() {

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

int feverPoint = 0;
int colorDistance(color_t a, color_t b) {
    return (a.r - b.r) * (a.r - b.r) + (a.g - b.g) * (a.g - b.g) + (a.b - b.b) * (a.b - b.b);
}

int colorToSnakeType(const uchar* colorData) {
    color_t c{};
    c.b = colorData[0];
    c.g = colorData[1];
    c.r = colorData[2];

    if (colorDistance(c, {90, 97, 49}) < SENSITIVITY) {
        return 1;
    }
    if (colorDistance(c, {66, 77, 165}) < SENSITIVITY) {
        return 2;
    }
    if (colorDistance(c, {123, 48, 57}) < SENSITIVITY) {
        return 3;
    }
    return 0;
}
/**
 * 检测蛇有没有黄色光环
 * @param colorData 位图数据
 * @param x 蛇的位置
 * @return
 */
int checkSnakePoint(const uchar* colorData, int x) {
    int startPos = static_cast<int>(HIGHPOS*(bmp.bmHeight - WINDOW_TOP_BORDER) + WINDOW_TOP_BORDER);
    int endPos = static_cast<int>(LOWPOS*(bmp.bmHeight - WINDOW_TOP_BORDER) + WINDOW_TOP_BORDER);
    for (int i = startPos; i < endPos; i++) {
        color_t light{};
        light.r = colorData[(i * bmp.bmWidth + x) * 4 + 2];
        light.g = colorData[(i * bmp.bmWidth + x) * 4 + 1];
        light.b = colorData[(i * bmp.bmWidth + x) * 4 + 0];
        if (light.r > 230 && light.g > 230 && light.b < 50) {
            return i;
        }
    }
    return -1;
}
/**
 * 获得蛇的序列
 * @param colorData 位图数据
 * @param 输出参数 ss 传递蛇的序列
 * @return
 */
int getSnakeSeq(uchar* colorData, std::vector<snake_t> &ss) {
    /**
     * 蛇的比例高度比例在0.5-0.68 之间 可以自行尝试以选择最优位置
     */
    constexpr double SNAKEPOSITION = 0.6755556;//0.642222;
    /**
     * 蛇的长度比例（比蛇全长偏小一些，因为只能检测到蛇的特定一部分）
     */
    constexpr double SNAKELENGTH = 0.11;
    /**
     * 香织大概就站在屏幕正中心，因为香织身上的彩饰带有的颜色会影响判断
     * 所以i的起始像素要在香织左右的像素外，以排除影响，可以自行适当调整
     */
    constexpr double KAORILENGTH = 0.0625;
    int lastPosition=-1;
    int snakeLength = static_cast<int>(SNAKELENGTH * (bmp.bmWidth - WINDOW_RIGHT_BORDER));//蛇的长度
    int kaoriLength = static_cast<int>(KAORILENGTH *(bmp.bmWidth - WINDOW_RIGHT_BORDER) );//香织长度
    int snakeVerticalPos = static_cast<int>((bmp.bmHeight - WINDOW_TOP_BORDER) * SNAKEPOSITION) + WINDOW_TOP_BORDER; //蛇的高度
    int midPos = (bmp.bmWidth - WINDOW_RIGHT_BORDER) / 2; //屏幕中位线
    for (int i = kaoriLength; i < midPos; i++) {
        int type;
        //搜索屏幕左侧的蛇
        type = colorToSnakeType(colorData + (snakeVerticalPos * bmp.bmWidth + midPos - i) * 4);
        if (type && (lastPosition < 0 || i - lastPosition > snakeLength)) {
            snake_t snake{};
            snake.count = type;
            snake.pos = i;
            snake.region = 0;
            snake.point = 0;
            snake.feverPos = checkSnakePoint(colorData, midPos - i);
            if(snake.feverPos>-1)
                snake.point =1;
            ss.push_back(snake);
            lastPosition = i;
        }
        //搜索屏幕右侧的蛇
        type = colorToSnakeType(colorData + (snakeVerticalPos * bmp.bmWidth + midPos + i) * 4);
        if (type && (lastPosition < 0 || i - lastPosition > snakeLength)) {
            snake_t snake{};
            snake.count = type;
            snake.pos = i;
            snake.region = 1;
            snake.point = 0;
            snake.feverPos = checkSnakePoint(colorData, midPos + i);
            if(snake.feverPos > -1)
                snake.point =1;
            ss.push_back(snake);
            lastPosition = i;
        }
    }

    for(snake_t s:ss){
        if(s.point)
            std::cout<<"*";
        switch (s.region) {
            case 0:
                std::cout<<"左";
                break;
            case 1:
                std::cout<<"右";
                break;
        }
        switch (s.count) {
            case 1:std::cout<<"绿 ";
                break;
            case 2:std::cout<<"蓝 ";
                break;
            case 3:std::cout<<"红 ";
                break;
        }
    }
    std::cout<<std::endl;

    if(!debug)return 0;
    //------------debug代码----------
    int startPos = static_cast<int>(HIGHPOS*(bmp.bmHeight - WINDOW_TOP_BORDER) + WINDOW_TOP_BORDER);
    int endPos = static_cast<int>(LOWPOS*(bmp.bmHeight - WINDOW_TOP_BORDER) + WINDOW_TOP_BORDER);
    int length = endPos - startPos;
    for(int i = 0;i<ss.size();i++){
        snake_t s = ss[i];
        int pos;
        if(s.region)pos = (bmp.bmWidth - WINDOW_RIGHT_BORDER) / 2 + s.pos;
        else pos = (bmp.bmWidth - WINDOW_RIGHT_BORDER) / 2 - s.pos;
        BitBlt(testDc,i*100,10,100,100,memDc,pos-50,snakeVerticalPos-50,SRCCOPY);

        BitBlt(testDc,i*100,110,50,length,memDc,pos-25,startPos,SRCCOPY);

        if(s.point){
            BitBlt(testDc,i*100,210,100,100,memDc,pos-50,s.feverPos-50,SRCCOPY);
        }
    }
    return 0;
}
/**
 * 执行点击操作，这里还会判断赛亚狗点数，触发连击模式
 * @param ss 蛇的序列
 * @return
 */
int execClick(std::vector<snake_t>& ss) {
    tl = GetTickCount64();
    DWORD LEFTPOS = MAKELONG(bmp.bmWidth/4,bmp.bmHeight/2), RIGHTPOS = MAKELONG(3*bmp.bmWidth/4,bmp.bmHeight/2);
    for (auto & s : ss) {
        DWORD clickPos = s.region == 0 ? LEFTPOS : RIGHTPOS;
        for (int j = 0; j < s.count; j++) {
            PostMessage(simulatorMsgWndHandle, WM_LBUTTONDOWN, 1, clickPos);
            PostMessage(simulatorMsgWndHandle, WM_LBUTTONUP, 1, clickPos);
            //控制打蛇的频率不能太快
            Sleep(35);
            while ((tc = GetTickCount64()) < tl + 60);
            tl = tc;
        }
        if(s.point)
            std::cout<<"***fever up***"<<std::endl;
        feverPoint += s.point;
        //开启连击模式
        if (feverPoint >= 4) {
            tl = GetTickCount64();
            Sleep(400);
            while (GetTickCount64() - tl < 6500) {
                PostMessage(simulatorMsgWndHandle, WM_LBUTTONDOWN, 1, clickPos);
                PostMessage(simulatorMsgWndHandle, WM_LBUTTONUP, 1, clickPos);
                Sleep(10);
            }
            Sleep(1080);
            std::cout<<"resume"<<std::endl;
            feverPoint = 0;
            break;
        }
    }
    return 0;
}

int game2(uchar* colorData) {
    std::vector<snake_t> ss;
    getSnakeSeq(colorData, ss);
    execClick(ss);
    /**
     * debug时看不出问题，一正式运行就莫名其妙点错位置的情况
     * 可能是图像变化太快，有各种特效干扰了判断
     * 例如debug时最后几秒屏幕边缘有红框，接近外侧的蛇会检测不到
     * 可以适当把睡眠时间加长，留足特效消失的时间
     */
    Sleep(360);

    return 0;
}
//程序入口点
int main(int argc,char** argv){
    srand(GetTickCount());
    HINSTANCE hInstance = ::GetModuleHandle(nullptr);
    int nCmdShow = SW_SHOW;

    // Initialize global strings 加载字符串存到变量中
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PCRCLICK, szWindowClass, MAX_LOADSTRING);
    //注册并初始化窗口
    MyRegisterClass(hInstance);
    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;

    // Main message loop: 消息循环
    for (;;) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            if (simulatorHandle) {
                if(!debug) {
                    PrintWindow(simulatorHandle, memDc, PW_CLIENTONLY);
                    GetBitmapBits(hbitmap, bmp.bmHeight * bmp.bmWidthBytes, data);
                    game2(data);
                }
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
    wcex.hIconSm        = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//              保存模块实例句柄，创建窗口
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
//寻找子窗口的回调函数 最终找到
BOOL CALLBACK quitfullscreenProc(HWND hwnd, LPARAM lParam)
{
    char szFindTitle[MAX_PATH] = {0 };
    char szFindClass[MAX_PATH] = {0 };
    int nMaxCount = MAX_PATH;

    LPSTR lpClassName = szFindClass;
    LPSTR lpWindowName = szFindTitle;

    GetWindowTextA(hwnd, lpWindowName, nMaxCount);
    GetClassNameA(hwnd, lpClassName, nMaxCount);
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    /* 这些是雷电模拟器的子窗口
     class Name:RenderWindow title:TheRender  我们要找的就是这个
     class Name:subWin title:sub
    */
    if (lpWindowName[0] == 'T') {
        simulatorMsgWndHandle = hwnd;
    }
    return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static std::vector<snake_t> ss;
    static int cx,cy;
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
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_CREATE: {
        //初始化opengl上下文
        // Q:这个有什么用吗？ A：我也看不懂，我也不敢改
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
        testDc = hdc1 = GetDC(hWnd); //窗口设备内容句柄
        int uds = ::ChoosePixelFormat(hdc1, &pfd);
        ::SetPixelFormat(hdc1, uds, &pfd);
        m_hrc = ::wglCreateContext(hdc1);
        ::wglMakeCurrent(hdc1, m_hrc);
        //glewInit();
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_VERTEX_ARRAY);
        //生成一个opengl的2d纹理
        glGenTextures(1, &imgtex);
        glBindTexture(GL_TEXTURE_2D, imgtex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        simulatorHandle = FindWindowW(nullptr, L"雷电模拟器");//因为此文件是UTF-8编码，所以用宽字节函数
        EnumChildWindows(simulatorHandle, quitfullscreenProc, 0);
        //用雷电模拟器的窗口 创建内存设备兼容内容 和 设备兼容位图并绑定
        simulatorDc = GetDC(simulatorHandle);
        memDc = CreateCompatibleDC(simulatorDc);
        GetClientRect(simulatorHandle, &simulatorRect);
        hbitmap = CreateCompatibleBitmap(simulatorDc, simulatorRect.right - simulatorRect.left, simulatorRect.bottom - simulatorRect.top);
        SelectObject(memDc, hbitmap);
        //        BitBlt(memDc, 0, 0, simulatorRect.right - simulatorRect.left, simulatorRect.bottom - simulatorRect.top, simulatorDc, 0, 0, SRCCOPY);
        //复制雷电的窗口内容到内存设备内容中，并填充opengl的纹理
        PrintWindow(simulatorHandle, memDc, PW_CLIENTONLY);
        GetObject(hbitmap, sizeof(BITMAP), &bmp);
        data = (unsigned char*)malloc(bmp.bmHeight * bmp.bmWidth * 4);
        GetBitmapBits(hbitmap, bmp.bmHeight * bmp.bmWidthBytes, data);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmp.bmWidth, bmp.bmHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
        //最后根据 位图大小移动窗口
        RECT ra, rb;
        GetClientRect(hWnd, &ra);
        GetWindowRect(hWnd, &rb);
        if(debug)
            MoveWindow(hWnd, 100, 0, 1000, 400, true);
        else MoveWindow(hWnd, 1000, 0, bmp.bmWidth / 4 + rb.right - ra.right + ra.left - rb.left, bmp.bmHeight / 4 + rb.bottom - ra.bottom + ra.top - rb.top, true);
        break;
    }
    case WM_SIZE: {
        cx = LOWORD(lParam);
        cy = HIWORD(lParam) ;
        glViewport(0, 0, cx, cy);
        break;
    }
    case WM_LBUTTONDOWN:
       if(debug) {
           execClick(ss);
           ss.clear();
           Sleep(350);
           PrintWindow(simulatorHandle, memDc, PW_CLIENTONLY);
           GetBitmapBits(hbitmap, bmp.bmHeight * bmp.bmWidthBytes, data);
           InvalidateRect(hWnd, nullptr, TRUE);
           Rectangle(testDc,0,0,1000,400);
           getSnakeSeq(data, ss);
           ValidateRect(hWnd, nullptr);
           break;
       }

    case WM_KEYDOWN: {
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        ReleaseDC(hWnd,testDc);
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
    default:
        break;
    }
    return (INT_PTR)FALSE;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------
//
//                                                                                                           `,:,:^
//                                                                                              '`'.      ,>??][\j<.
//                                                                                           ,<-[{\~   :}rYj[1)fOJXu\>.
//                                                                                         ,])+?[nLuj\tQQYYUJJUJJYUYYYt:
//                                                                                      .+fXCv|j/U0YUUJC00JXXXYXYu)+<+1)-"
//                                                                                     !xJUYXYUUJCJXYUUUULmQYYYYri;III<-})^
//                                                                                "l!"lYUXXXYYYYYXXYzf){(rCp0YYY(!ll!>_-_\~''..
//                                                                              ']1_}[{0LCJUYYXXYYYu~;;;;l[YqLUYn?___-]{1|-~~~<~~_<!:`
//                                                                              !\>>[u0QCCCCLQQCUXY{;lllll<]QOQJUx}?}1[+iIIlIIl>?(vOLYu(<` '^^^^^`'.
//                                                                             .{u()frCJYYYYUUJLZOC|>!lll<-?Cw0O0QJ\-!IIIlll!i<+__10O00QLx<:,"^",:;;I;:`
//                             ';~[(tfft)-!"                           .`":;I;ijCUUUUUUUnruf\nzUU0wX[_+__-_1mOzf)]+illli<<<_]{|fuzJQQQCJ\I.`^`.     .':!
//                          .>(vJJn(1(tnU0OJx]"                .^;i?)tnzYJCCJJZCXYYYXYc?])-\?i-)cUmwY/{]]})1]iI;;;IIli+_]fuXYUJUUULOOQLx~>+?]][]!'     .
//                         ixCCu[;.     ';]xLOY['           ,_\nzYUCCJJUUJJCCO0XYYYYYY]lx-i+\/t/j0ZpZOQLX)>I;Illllli~_?/YJUYYYYJJYvt1?+<<<<>>+})/(l.
//  ^"                    ?UCx~'    '>{/f/{-_\J0r:       .<jYCCCCCCCLLLLLCCCJZQXYYYYYY{>f{>~1vQJUJC0Zqwji;Illlllli<_[tXJYXYUJYn|?<i!i>>>>><<<_{{{f(~;
//^!_^                   ~JX?'    .-vCJXUC0OQXnQZn"    .+v00Q00OOOOOOOOO00000mOXYYYYYYf/UUvzUUXYYXXXXULY/+><~+__?])xYUYXYJUu)~lll!!i><+_?})(|///trn}<~^
//+_"                   `xnI     :jLJ\!^^,I<}rJOqm\. ."/ZmmmmmZZZZZZO0OOOZZmmmpQXXYYYXQ0YYYYYYYYYYYYYXYCOUvczzXXXXUJUYYJYt_lIllll!<_][}}}{1))1)|junv]I_l
//_;                    lx:     :vCnl .^:::"`.`i/QO11nXJJUUUUUUJJCCLLLCJUUUJCL0wOUXXXYmUXYYYYYYYYYYYYYYYJ0ZO00QQLLLLLQ0r<"^;llli+?]]]{\xzYJCYj1]]{/xz~">
//l                     ">      (0Y)|xvXYJCLCJn?~xZLJUYYYYYYYYYYYYYYYYUJJJUYYYYJ0mQJYUmYXYYYYYYYYYYYYYYYYLQQQQQQ0QQQOQ{ll;;lli+?]?[/cLOZZO0LQn-]-~><)xl"
//                            ^-YQJUUUUJQ0ZZmmZOLCYXXYXYYYYYYYYYYYYYYYYYYYYYYYYYYJZm0LwCXYYYYYYYYYYYYYYYYCQQQQQQQ0Ow0?I!!!!!~?]?}xCZZ0QLJUYXYx>><)UX/|{,
//                          :{cJLCXYJQOZQCUCQCYXXXXXXXXXYYYYYYYYYYYYYYYYYYYYYYYYYYYCOZZmYXYYYYYYYYYYYYYYULQQQQQQQOqw[!i!!!!_[]]jQ0CUYXzzzzzzYX_>>/LJQCC1
//                        "1XJUYYJCOZQJXXXCQUXXXXXXXXXXXXYYYYYYYYYYYYYYYYYYYYYYYYYYYYCQmmUXYYYYYYYYYYYYUCQQQQQQQQ0pni>>iii_}[[jr[+<<<><<<~~+_[~i>\QzXUCQ
//                       +cJYYYYCOZCYXXXXYLYXXXXXXXXXXXXXXXXYYYYYYYYYYYYYYYYYYXzYYYYYYYJ0wLUUYYYYYYYYYJCQQQQQQQQQ0p\!>>>><}{}/n_i<+_??][}{11{[]-_}JUcczY
//                     ,/JUYYYJOZCXXXXXXXYUXXXXXXXXXXXXXXXXXXYYYYYYYYYYYYYYYYYYcnXUUYXzXYJOOLCCJJJJJCLQQQQQQQQQQ0Zdf!>>>>+|)}jftnXYYYYYUJzxnYJYXXcYUcccc
//                    <cJYYYUQmQYXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXYYYYYYYzzYYXXXuncJJUzuXYY0mO0QQQQQQQQQQQQQQQ0Omqp0_>>>>+j\}rQJXzzzzcccccn/)fcXzzcccccc
//                  .{JUYYYJZZJXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXcuncYzXYYYXXJQJYYYYYCZmmmwwmwmmmZZmmwwqqqqqpc?--?}/r)1cQJXcccccccczzv/)\uXzccccc
//                 `/CYYYYLm0YYYYYXXXXXXXXXXXXXXXXXXXXXXXvzXcvvcccczXXXXXYYXYYYYYYYYYJOCYYYYYUL0000OmpmZmmZZOOZZO000ZX(}{{{tj(fX0LYccccccccczXcf)/uXzccc
//                ^jCYYYYQmCYYYYYYXXXXXXcXXXXXXXXXXzzzzXv-]xXzvxfuXXXXXYJYYYYYYYYYYYYYJmQYYYYYYJCQ00QOwOJUYYYYUL00000OQvt||/xnnncQQUzccccccXj))(}-]/cYzz
//               'tCYYYY0wJYJYYYYYYXXXXcvXXXXXXzXzvzzzXXx~i>{cYzvxzXXXXXUCYYYYYYYYYYYYYJq0YYYYYYYJQ0OQ0m0YYYYYYYJL0000Q0O0CUXcvvvz00JXczzzXu1_-1/t|-</Uv
//              .(CYYYU0wJYJUYYYYYYXXXXcvXXXXzzzcnrczzYJn~>>i~fYXczXXXXXXYXzYXXYUJCCLCCCOpOLJUYYYYCQZ0Q0ZZJYYYYYYYJLQ0QLCQ0000LUXzXQ0CXvvnf_i[vCXXCJ?>+<
//              }CYYYUQwCYJJYYYYYXXXXXYcnXzcczzzccvvzzULx+>>>ii{XXczXXXXXzYvc000QLCJJUYYY0CJQUXXXXYL0ZQQQmdLYYYYYYYYUCLLCJJL000QLJXzCOLn\]_-]+cCXXf-i+-_
//             ?JUYYYLwLYJCUYYXXXXXXzzJcfrnvczJXzzzzzzULn+>>>>>i-cXcYXzzzYQZJrcYczXzzzzzzXQrXCXzXXXJLwOQ00qwOJYXcvcXYYUYzunncXUUYYXzcYOC-_?])!(Qf~!_->ll
//            <UUYYYJZZYUCCYXXXXXXzzzXCXjjczzXQCzzXXXzC0u?>>>>>>i~uXzCXcXQJzU(_rYczzccccccUY1vLzzzzYJZw0QQwm0m0UYcnfjxvccnf\\//fjxnnnnc0|_]?+1r]!-->:^:l
//           !XUYYYYQqCYJLJXXXzcXzzzvXQJnxXzzYLzzXXXXzUOc[<>>>>>>i~uzXCzcvvvYc]>|cvzucvvvvvC\+nJcccXU0mmOQmwQ0ZZQUYzuf\\/t//////\//fjxxvZY{xCr<_[<IIII!~
//          lcJYYYYUOmYYL0JXXXzuvzcvnX0CzvzzzUJxXzzzXzzOU}+>>>>>>>i_ccYvzuvuuU(<i]xcx/zununXxi+xJcccYLZOq0wmQ0Q0ZZQUYUXnf\\\//////\\\/tfuc+\\i}[!llll>?}
//         >zJYYYYYUZ0YY0OUXXXXvjczzXXQLXzzzzXJ\YzzzzzcJZ{->>>>>>>>i{vX\uznnxYj~>i+fv?\Xxxxvxi>_vYvczQZQmOwO0000Q0ZZ0JYUJXuj///tfjrrjftt/xr)<{}!!!!i_[[[
//        -UUYYYYYYJmQXUmZUcccncucXXzXC0JzzXXUQfULJUUXcc0|-~>>>>>>>>>/z|[cuxxvj<i>>_/1>\XvvYjii!-zcczZOQ0ZZ0000000Q0OZ0LUUJJn|)\/tjrnxxxj(r\1)>iii~]}}}{
//      ')JUYYYYYYYCqCXJwZUnnzXXXzXXzzU0QJCQLCQu(cUXXzcvXr__>>>>>>>>i?u)_1Xxrc/<?][(uZqbakkhbw0c((vuUOQLC0Q000000ZO00Q0OZ0CUUv1l^"::,"`'  :jt+>>>+}{{{{{
//     ixJYYYYYYYXXLwCXJqmJvcXXXXXXXXzJOmLYXzcCx~~xzvfncuX{_~>>>>>>>>>t{+~jnrj]){|JkbQUJCLQ00QCLqqmYzQQLYYCQQ00000mZ000000ZQYXJYt>'       .\j<>>+}{{{{{{
//   I|zXYYYYUYznnJC0CYCpwJXXXXXXXXXXLCJ00YzzcYz[></cu1xvun?_<>>>>>>>>]_<>-xj?<+|00r{/XUQOmwppqJ1}cq0m0YXXJQQQQ000OmZ0Q000QQCXxjnvr{>,`.   !u{??}{{{{{{{
//^<|nuucYYYzuj/fYCcUQYJqqLYYXXXXXXXYXzYJOLXcccY)~ii}nu[(nv\_+>>>>>>>>i>>>>{?i>>{u)-__.`vmbbddqdq[^tk0vczzULQCUQ00QZwOQQQ0OQQ0QJx)+iI;:'    ]t{{{{{{{{{{
//jxrjrnvuxf/\/fzJcrXOYJwwmYYYXXXXzXUXzXUULJzvuUj?>ii<)f?+)j(_~>>>>>>>>>>>>ii>>><?~I>r}xJJqwwpq0qm>+zzcvczJLLJYJ0000mZO0QL0ZOL0OZZ0Ux)_!:^' .+\|())))())
//ttttt///tttjxcXujrYmUYZ0qCYJYXXXzzYUzzXYcfYcuvc}_~-]}(f|)}[+>>>>>>>>>>>>>>>>>>>>I,,xhwQmmLJQCvUb]>+|zuccJQCYYYC00QOqOOOQQQOc)1/xuczvr\[<:.  "+)fxucczz
//////jnvcunuvunrtfXY0QY0COZYULXzzzczUYccXU{[vznz\[\vCQQLCJv(~i>>>>>>>>>>>>>>>>>>>il::tOQcxjf/)({/><>{cuccJLUXYYUQ00QmwQ0mOQLLY(;                ',Ii<<<
//)\jcUXnjuXcnrt/jzYYJZUQQCm0X0CczzccXCzvcJf>~\nnQb#wJQmwwwZU/<i>>>>>>>>>>>>>>>>>>>>!I:~tcujtjvYj+~~~}vuccCCYXYYYJ00QOqJ\/Y0ZO00Cr+"
//!i<<!,:[rrft//rucYYYL0JZJQmLYqJcccccCLcvXY?i!]q8pt(xvCZZwpppL1i>>>>>>>>>>>>>>>>>>>>>>!l>)nr\1?+++_+}vncXQJXXYYYYL00QmZC):;-(xvXUUn1i'                :
//     l(f////tffnYYYYYQ0Q0L0mCQqCcccvzmQcvUf<>J8d)]?  _Xmdpbpww|i>>>>>>>>>>>>>>>>>>>>>>><<><<~+_]_[+)vucCQYXYYYYYJQ0QZw00X+   .`^^``'
//  .I1f///////tuYUYYYYYL0ZOL0mLOwLcvvvYq0zuU[_d&c-!)()ULwqmZp0QZ?i>>>>>>>>>>>>>>>>>>>>>><<<~~++_-+++jvuX0YXYYYYYYYL0000cLO0n>                     .`",,
//^>)jt//////fnXYXczYUYXcU0ZO0QYJQZ0zuunJmOYcn-Qhxi,+whZ0ZCzcYnxu[i>>>>>>>>>>>>>>>>>>>>>>><<<~~~~~~<_uncQUXYJYYYYYYJQ0001<rJO0x~'             `;>+][{)|\
//tft//////fxvvnrjnvnrjxvzU0ZQcuUZ0OZJunxJZ0QLXYmL+""<zOOYtt/1{vY]>>>>>>>~<>>>>>>>>>>>>>>>>>><<<<<<>|vcQYcXCCYYYYYYUQ000U, ,+\cCz)!'         <(__+<<[f\_
///////////tffffjjt/tjxnuzJYcvvzYZ000O0YvuULJCUx\vc{>;,<)xuuvvzv(~>>>>>><]<>>>>>>>>iiii>+_<>>>>>>>i-nuxXczYQCYYYYYYUL0Z0Z/    .:~)/(?!^.    ^x1i>>><]r[!
//t///ttfjrxxxrjjjxxnnnnuvuxxvzzYmQC0Q0O0LUJCUYXu\]__~>!l!~1({-<>>>>>>>>>i>>>>iii>~-}|j/}[f1>>>>>>+f\_{zcXCOCYYYYYYUL0mO00~        '":,.    ?x/><<<>+)j~
//jncXYYXzcvuuuvunxvvvunnnnuczXYCmJJ0000QQLJJUXYXnn(<i><~++~~~++~~<<>>>>>>ii><_]{)\/tt//1{fr1>>>>>]_iijYzU0ZJYYYYYYUL0XYOOX"               :t[\]><<>>_\f
//.'"I>+]{11){[-?)jnnnnnnucXYYnc0OYJ000LLQCJCUXcUJnun{<<~?_-_++++~<<>>>>>~[(tf/\\||(((((||(\f<>>>>ii><YYXCOZYYYYYYYUQZ\lXZZ\             ."{|__|_i>>>>_(
//           ^i1ruuvvvcXYYzn|~)YZUYLZOLYYCQLCCCUzYCvxvf-<____+++~<<>>>>>~vUvj\(|||||||||||(/f<>>>>>ii(mUYLmOYYYYYYYU0Q> :/CO)'         ^i?](j-<+)?;I>>>~
//        'l-)fj/|){->!!!I".'1UOCYCvYQYYYYLQQLLLLJXUYnnu|~><<<<<>>>>>>>>>)zx|(||||||||||||\t]i>>>>i~rmpCU0wCYYYYYYYCOwCz}:"~|},'":;li~+?]_~_j)~><{[>>>>>
//         ..  .           ;?1vCUCt-JJYYYYUQ0QQQLOwCJJUYUY(<ii>>>>>>>>>>>i_\f/\|||||||||\/(_i>>ii<}ummpQCOmYYYYYYYJQqppwu>;!+{){1}]][]_~<>>>-t}>>>-{?<>>
//                        .'"/JJu~.tQYYYYYYJQ000Q0pZOQ0ZwqpC/_>!ii>>>>>>>>i>_})|\\\\|({[?_>i>i<-{jrCmpbOQmZYYYJJYJQwdqpwOXr)}]-_++~~<>>>>>>>>_({<>><]{]+
//                         >cQu-' `vJYYYYYYYL0000QwwmwwwqpqqqOYj1-<iiiiiiii>iii><~++~<>iii>i~~[cZ00Z00qZQwQYYYCCJQmbqqqp0n]~~~<<>>>>>>>>>>>>>>~}(]<ii<?{
//                       ;|Yr+'   "zUYYYYYYYJ0000QZpwbpqqqqqqqqwwZCzr((fj(_iiiiiiiiiii>>ii!<nXvCOQQQQQOq0wCYYUQQQmwOOZmmw/i>>>>>>>>>>>>>>>>>>>><_})}-+~+
//                    ':+[+:      ,zUYUJYYYYUQ0OZQOkhpqwmZmZO000LLLQLQ0OwZCu(1}]-_-}~>>>_|/JZm0QQQQQ00QZZwJYYJO0mmQQ0OZZZC(+>>>>>>>>>>>>>>>>>>>>><+]1(()
//                   '^"'         ^cUYJCYYYYYC0Ow00dqZZOOQQCCL0LUYYYYYCLLQQOUXUYzvJOCn/fQZZLLLCUUUULLJYJQqJYYQOmmQQLCQQOX[{_>>>>>>>>>>>>>>>>>>>>>>><~_?[
//                                .jCYLJYYYYYJ00cO0qOQQQLJUYYYL0QYYYYYYJJLQL0000000QOOLQQUYUUYXXXYYYYXYXYZCYUZmZLQ0QC0w0Y}!i>>>>>>>>>>>>>>>>>>>>><<<~+++
//                                 <YU0JYYYYYJQZ\zZqZ00LJUYYYYJUCCYYYYYYYJJYYCQQLJCQCUUJUYYYXYYYYYYYYYYYXL0YCqQJLQZZQ0O{><>>>>>>>>>>>>>>>>>>>><<~~~++_+_
//                                 '/COUYYYYYJ0m()OqwmQCUYYYYYYYXYYYYYYYYYYYYYUJUXYUYXXXYYYYYYYYYYYYYYYYXUOYQZCLQQZmQOu>i>>>>>><<>>>>>>>>>><<~~~+++___}[
//-----------------------------------------------------------------------------------------------------------------------------------------------------