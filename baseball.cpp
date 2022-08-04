
#include <Windows.h>
#include <opencv2/opencv.hpp>
#include <immintrin.h>

#include "Eigen/Eigen"

#ifdef _DEBUG
#pragma comment(lib,"opencv_world454d.lib")
#else
#pragma comment(lib,"opencv_world454.lib")
#endif 

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

float inf = 1.0f / 0.0f;

ULONGLONG ts = 0;
HDC hdc1, hdc2;
HGLRC m_hrc;
int mx, my, cx, cy;
double ang1, ang2, len, cenx, ceny, cenz;

HWND bbhwnd;
HWND bbhwndmessage;
HDC bbhdc, bbhdcc;
HBITMAP hbitmap;
BITMAP bmp;
RECT bbrect;
unsigned char* data;
int* tempdata;
unsigned char* record;
int recordi = 0;

cv::Mat img, alpha;
target_t screen, flag, cursor;
target_t screenr, flagr, cursorr;
target_t corner1, corner2, corner3, corner4, cornerr;

int start = 0;
uint64_t tc = 0, tl = 0;

int lastcursorx = 0;
uint64_t lastcursort = 0, lastclicktime = 0;
double speed = 0;


int reduce(target_t src, target_t dst) {
    if (dst.alpha) {
        for (int i = 0; i < dst.h; i++) {
            for (int j = 0; j < dst.w; j++) {
                dst.data[(i * dst.w + j) * 3] = src.data[(i * 2 * src.w + j * 2) * 3] + src.data[(i * 2 * src.w + j * 2 + 1) * 3] + src.data[((i * 2 + 1) * src.w + j * 2) * 3] + src.data[((i * 2 + 1) * src.w + j * 2 + 1) * 3];
                dst.data[(i * dst.w + j) * 3 + 1] = src.data[(i * 2 * src.w + j * 2) * 3 + 1] + src.data[(i * 2 * src.w + j * 2 + 1) * 3 + 1] + src.data[((i * 2 + 1) * src.w + j * 2) * 3 + 1] + src.data[((i * 2 + 1) * src.w + j * 2 + 1) * 3 + 1];
                dst.data[(i * dst.w + j) * 3 + 2] = src.data[(i * 2 * src.w + j * 2) * 3 + 2] + src.data[(i * 2 * src.w + j * 2 + 1) * 3 + 2] + src.data[((i * 2 + 1) * src.w + j * 2) * 3 + 2] + src.data[((i * 2 + 1) * src.w + j * 2 + 1) * 3 + 2];

                dst.alpha[(i * dst.w + j) * 3] = src.alpha[(i * 2 * src.w + j * 2) * 3] + src.alpha[(i * 2 * src.w + j * 2 + 1) * 3] + src.alpha[((i * 2 + 1) * src.w + j * 2) * 3] + src.alpha[((i * 2 + 1) * src.w + j * 2 + 1) * 3];
                dst.alpha[(i * dst.w + j) * 3 + 1] = src.alpha[(i * 2 * src.w + j * 2) * 3 + 1] + src.alpha[(i * 2 * src.w + j * 2 + 1) * 3 + 1] + src.alpha[((i * 2 + 1) * src.w + j * 2) * 3 + 1] + src.alpha[((i * 2 + 1) * src.w + j * 2 + 1) * 3 + 1];
                dst.alpha[(i * dst.w + j) * 3 + 2] = src.alpha[(i * 2 * src.w + j * 2) * 3 + 2] + src.alpha[(i * 2 * src.w + j * 2 + 1) * 3 + 2] + src.alpha[((i * 2 + 1) * src.w + j * 2) * 3 + 2] + src.alpha[((i * 2 + 1) * src.w + j * 2 + 1) * 3 + 2];
            }
        }
    }
    else {
        for (int i = 0; i < dst.h; i++) {
            for (int j = 0; j < dst.w; j++) {
                dst.data[(i * dst.w + j) * 3] = src.data[(i * 2 * src.w + j * 2) * 3] + src.data[(i * 2 * src.w + j * 2 + 1) * 3] + src.data[((i * 2 + 1) * src.w + j * 2) * 3] + src.data[((i * 2 + 1) * src.w + j * 2 + 1) * 3];
                dst.data[(i * dst.w + j) * 3 + 1] = src.data[(i * 2 * src.w + j * 2) * 3 + 1] + src.data[(i * 2 * src.w + j * 2 + 1) * 3 + 1] + src.data[((i * 2 + 1) * src.w + j * 2) * 3 + 1] + src.data[((i * 2 + 1) * src.w + j * 2 + 1) * 3 + 1];
                dst.data[(i * dst.w + j) * 3 + 2] = src.data[(i * 2 * src.w + j * 2) * 3 + 2] + src.data[(i * 2 * src.w + j * 2 + 1) * 3 + 2] + src.data[((i * 2 + 1) * src.w + j * 2) * 3 + 2] + src.data[((i * 2 + 1) * src.w + j * 2 + 1) * 3 + 2];
            }
        }
    }
    return 0;
}

int fliph(target_t& src, target_t& dst) {
    dst.w = src.w;
    dst.h = src.h;
    dst.data = (float*)malloc(dst.w * dst.h * 3 * sizeof(float));
    dst.alpha = (float*)malloc(dst.w * dst.h * 3 * sizeof(float));
    for (int i = 0; i < dst.h; i++) {
        for (int j = 0; j < dst.w; j++) {
            dst.data[(i * dst.w + j) * 3] = src.data[(i * src.w + (dst.w - j - 1)) * 3];
            dst.data[(i * dst.w + j) * 3 + 1] = src.data[(i * src.w + (dst.w - j - 1)) * 3 + 1];
            dst.data[(i * dst.w + j) * 3 + 2] = src.data[(i * src.w + (dst.w - j - 1)) * 3 + 2];
            dst.alpha[(i * dst.w + j) * 3] = src.alpha[(i * src.w + (dst.w - j - 1)) * 3];
            dst.alpha[(i * dst.w + j) * 3 + 1] = src.alpha[(i * src.w + (dst.w - j - 1)) * 3 + 1];
            dst.alpha[(i * dst.w + j) * 3 + 2] = src.alpha[(i * src.w + (dst.w - j - 1)) * 3 + 2];
        }
    }
    return 0;
}

int flipv(target_t& src, target_t& dst) {
    dst.w = src.w;
    dst.h = src.h;
    dst.data = (float*)malloc(dst.w * dst.h * 3 * sizeof(float));
    dst.alpha = (float*)malloc(dst.w * dst.h * 3 * sizeof(float));
    for (int i = 0; i < dst.h; i++) {
        for (int j = 0; j < dst.w; j++) {
            dst.data[(i * dst.w + j) * 3] = src.data[((dst.h - i - 1) * src.w + j) * 3];
            dst.data[(i * dst.w + j) * 3 + 1] = src.data[((dst.h - i - 1) * src.w + j) * 3 + 1];
            dst.data[(i * dst.w + j) * 3 + 2] = src.data[((dst.h - i - 1) * src.w + j) * 3 + 2];
            dst.alpha[(i * dst.w + j) * 3] = src.alpha[((dst.h - i - 1) * src.w + j) * 3];
            dst.alpha[(i * dst.w + j) * 3 + 1] = src.alpha[((dst.h - i - 1) * src.w + j) * 3 + 1];
            dst.alpha[(i * dst.w + j) * 3 + 2] = src.alpha[((dst.h - i - 1) * src.w + j) * 3 + 2];
        }
    }
    return 0;
}

float geterror(target_t img, target_t target, int x, int y) {
    __m256 sum = _mm256_set1_ps(0.0f);
    for (int j = 0; j < target.h; j++) {
        for (int i = 0; i < target.w * 3; i += 8) {
            __m256 x1, x2;
            x1 = _mm256_loadu_ps(img.data + (j + y) * 3 * img.w + (i + x * 3));
            x2 = _mm256_loadu_ps(target.data + j * 3 * target.w + i);
            x1 = _mm256_sub_ps(x1, x2);
            x1 = _mm256_mul_ps(x1, x1);
            x2 = _mm256_loadu_ps(target.alpha + j * 3 * target.w + i);
            x1 = _mm256_mul_ps(x1, x2);
            sum = _mm256_add_ps(sum, x1);
        }
    }
    return sum.m256_f32[0] + sum.m256_f32[1] + sum.m256_f32[2] + sum.m256_f32[3] + sum.m256_f32[4] + sum.m256_f32[5] + sum.m256_f32[6] + sum.m256_f32[7];
}

int match(target_t img, target_t target, int x1, int x2, int y1, int y2, int& mx, int& my, float& value) {
    value = inf;
    for (int y = y1; y < y2; y++) {
        for (int x = x1; x < x2; x++) {
            float error = geterror(img, target, x, y);
            if (error < value) {
                value = error;
                mx = x;
                my = y;
            }
        }
    }
    return 0;
}

int loadimg(const char* fn, target_t& target, target_t& targetr) {
    img = cv::imread(fn);
    target.w = img.cols;
    target.h = img.rows;
    target.data = (float*)malloc(target.w * target.h * 3 * sizeof(float));
    target.alpha = (float*)malloc(target.w * target.h * 3 * sizeof(float));
    for (int i = 0; i < target.w * target.h; i++) {
        target.data[i * 3] = img.data[i * 3];
        target.data[i * 3 + 1] = img.data[i * 3 + 1];
        target.data[i * 3 + 2] = img.data[i * 3 + 2];
        target.alpha[i * 3] = 1.0;
        target.alpha[i * 3 + 1] = 1.0;
        target.alpha[i * 3 + 2] = 1.0;
    }
    targetr.w = target.w / 2;
    targetr.h = target.h / 2;
    targetr.data = (float*)malloc(targetr.w * targetr.h * 3 * sizeof(float));
    targetr.alpha = (float*)malloc(targetr.w * targetr.h * 3 * sizeof(float));
    reduce(target, targetr);
    return 0;
}

int loadimg(const char* fn, const char* fna, target_t& target, target_t& targetr) {
    img = cv::imread(fn);
    alpha = cv::imread(fna);
    target.w = img.cols;
    target.h = img.rows;
    target.data = (float*)malloc(target.w * target.h * 3 * sizeof(float));
    target.alpha = (float*)malloc(target.w * target.h * 3 * sizeof(float));
    for (int i = 0; i < target.w * target.h; i++) {
        target.data[i * 3] = img.data[i * 3];
        target.data[i * 3 + 1] = img.data[i * 3 + 1];
        target.data[i * 3 + 2] = img.data[i * 3 + 2];
        target.alpha[i * 3] = (1.0f / 255.0f) * alpha.data[i * 3];
        target.alpha[i * 3 + 1] = (1.0f / 255.0f) * alpha.data[i * 3];
        target.alpha[i * 3 + 2] = (1.0f / 255.0f) * alpha.data[i * 3];
    }
    targetr.w = target.w / 2;
    targetr.h = target.h / 2;
    targetr.data = (float*)malloc(targetr.w * targetr.h * 3 * sizeof(float));
    targetr.alpha = (float*)malloc(targetr.w * targetr.h * 3 * sizeof(float));
    reduce(target, targetr);
    return 0;
}

struct size_t3 {
    size_t x, y, z;
};

template <typename T>
struct area_t {
    size_t3 index;
    T value;
};

template <typename T>
int getonearea(T* data, size_t sizex, size_t sizey, size_t sizez, size_t x, size_t y, size_t z, std::vector<area_t<T>>& area) {
    area.push_back({ x,y,z,data[(z * sizey + y) * sizex + x] });
    data[(z * sizey + y) * sizex + x] = 0;
    int cur = 0;
    size_t3 p;
    while (cur < area.size()) {
        p = area[cur].index;
        if (p.x > 0 && data[(p.z * sizey + p.y) * sizex + p.x - 1]) {
            area.push_back({ p.x - 1, p.y,p.z, data[(p.z * sizey + p.y) * sizex + p.x - 1] });
            data[(p.z * sizey + p.y) * sizex + p.x - 1] = 0;
        }
        if (p.y > 0 && data[(p.z * sizey + p.y - 1) * sizex + p.x]) {
            area.push_back({ p.x, p.y - 1,p.z,data[(p.z * sizey + p.y - 1) * sizex + p.x] });
            data[(p.z * sizey + p.y - 1) * sizex + p.x] = 0;
        }
        if (p.z > 0 && data[((p.z - 1) * sizey + p.y) * sizex + p.x]) {
            area.push_back({ p.x, p.y,p.z - 1,data[((p.z - 1) * sizey + p.y) * sizex + p.x] });
            data[((p.z - 1) * sizey + p.y) * sizex + p.x] = 0;
        }
        if (p.x < sizex - 1 && data[(p.z * sizey + p.y) * sizex + p.x + 1]) {
            area.push_back({ p.x + 1, p.y,p.z,data[(p.z * sizey + p.y) * sizex + p.x + 1] });
            data[(p.z * sizey + p.y) * sizex + p.x + 1] = 0;
        }
        if (p.y < sizey - 1 && data[(p.z * sizey + p.y + 1) * sizex + p.x]) {
            area.push_back({ p.x, p.y + 1,p.z,data[(p.z * sizey + p.y + 1) * sizex + p.x] });
            data[(p.z * sizey + p.y + 1) * sizex + p.x] = 0;
        }
        if (p.z < sizez - 1 && data[((p.z + 1) * sizey + p.y) * sizex + p.x]) {
            area.push_back({ p.x, p.y,p.z + 1,data[((p.z + 1) * sizey + p.y) * sizex + p.x] });
            data[((p.z + 1) * sizey + p.y) * sizex + p.x] = 0;
        }
        cur++;
    }
    return 0;
}

template <typename T>
int getareas(T* data, size_t sizex, size_t sizey, size_t sizez, std::vector<std::vector<area_t<T>>>& areas) {
    for (size_t z = 0; z < sizez; z++) {
        for (size_t y = 0; y < sizey; y++) {
            for (size_t x = 0; x < sizex; x++) {
                if (data[(z * sizey + y) * sizex + x]) {
                    std::vector<area_t<T>> area;
                    getonearea(data, sizex, sizey, sizez, x, y, z, area);
                    areas.push_back(area);
                }
            }
        }
    }
    return 0;
}

struct peakadv_t {
    double area;//0
    double xc, yc, zc;//1
    double maxintensity;//4
    double totalintensity;//5
    double type;//6
    double xx, yy, zz, xy, xz, yz;//7
};

template <typename T>
void getpeakparaadv(std::vector<area_t<T>>& area, peakadv_t& peak) {
    peak.area = area.size();

    peak.maxintensity = area[0].value;
    peak.totalintensity = 0;
    peak.xc = 0;
    peak.yc = 0;
    peak.zc = 0;
    for (int i = 0; i < area.size(); i++) {
        peak.xc += area[i].index.x * area[i].value;
        peak.yc += area[i].index.y * area[i].value;
        peak.zc += area[i].index.z * area[i].value;
        peak.totalintensity += area[i].value;
        if (area[i].value > peak.maxintensity) {
            peak.maxintensity = area[i].value;
        }
    }

    double weight = 1.0 / peak.totalintensity;

    peak.xc *= weight;
    peak.yc *= weight;
    peak.zc *= weight;


    for (int i = 0; i < area.size(); i++) {
        double dx = area[i].index.x - peak.xc;
        double dy = area[i].index.y - peak.yc;
        double dz = area[i].index.z - peak.zc;
        peak.xx += dx * dx * area[i].value;
        peak.yy += dx * dy * area[i].value;
        peak.zz += dx * dz * area[i].value;
        peak.xy += dy * dy * area[i].value;
        peak.xz += dy * dz * area[i].value;
        peak.yz += dz * dz * area[i].value;
    }
    peak.xx *= weight;
    peak.yy *= weight;
    peak.zz *= weight;
    peak.xy *= weight;
    peak.xz *= weight;
    peak.yz *= weight;
}


int findedge(float* data, int n, float& x1, float& x2) {
    float r, g, b;
    int e, et;
    e = 0;
    et = 0;
    for (int i = 0; i < n; i++) {
        b = data[i * 3];
        g = data[i * 3 + 1];
        r = data[i * 3 + 2];
        if (r < 100 && g < 100 && b < 50) {
            if (!e) {
                et++;
            }
            e++;
        }
        else {
            if (e) {
                if (et == 1) {
                    x1 = i - 0.5f * e;
                }
                else if (et == 2) {
                    x2 = i - 0.5f * e;
                }
            }
            e = 0;
        }
    }
    return et;
}

int game3findball(target_t& screen, float& x, float& y) {
    const int xa = 820, xb = 1100, ya = 330, yb = 640;
    int mx, my;
    float error;

    int et = 0;
    float x1, x2;
    std::vector<float> x1v, x2v;
    for (int j = ya; j < yb; j++) {
        if (findedge(screen.data + j * screen.w * 3 + xa * 3, xb - xa, x1, x2) == 2) {
            x1v.emplace_back(x1);
            x2v.emplace_back(x2);
            et++;
        }
        else {
            if (et >= 4) {
                std::sort(x1v.begin(), x1v.end());
                std::sort(x2v.begin(), x2v.end());
                x = 0.5f * (x1v[x1v.size() / 2] + x2v[x2v.size() / 2]) + xa;
                y = j - 0.5f * et;
                return 1;
            }
            x1v.clear();
            x2v.clear();
            et = 0;
        }
    }
    //match(screen, corner1, xa, xb - 7, ya, yb - 7, mx, my, error);
    //printf("corner:%d,%d,%f\n", mx, my, error);
    //match(screen, corner2, xa, xb - 7, ya, yb - 7, mx, my, error);
    //printf("corner:%d,%d,%f\n", mx, my, error);
    //match(screen, corner3, xa, xb - 7, ya, yb - 7, mx, my, error);
    ///printf("corner:%d,%d,%f\n", mx, my, error);
    //match(screen, corner4, xa, xb - 7, ya, yb - 7, mx, my, error);
    //printf("corner:%d,%d,%f\n", mx, my, error);
    return 0;
}

int game3findfire(target_t& screen, float& x, float& y) {

    const int xa = 860, xb = 1060, ya = 300, yb = 500;

    for (int j = ya; j < yb; j++) {
        for (int i = xa; i < xb; i++) {
            tempdata[j * screen.w + i] = 0;
            float r, g, b;
            b = screen.data[(j * screen.w + i) * 3];
            g = screen.data[(j * screen.w + i) * 3 + 1];
            r = screen.data[(j * screen.w + i) * 3 + 2];
            if (r > 250.0f && g > 250.0f && b < 100.0f) {
                tempdata[j * screen.w + i] = 1;
            }
        }
    }

    std::vector<std::vector<area_t<int>>> area;
    getareas(tempdata, screen.w, screen.h, 1, area);

    int mi, mv;
    if (area.size()) {
        mv = area[0].size();
        mi = 0;
        for (int i = 0; i < area.size(); i++) {
            if (area[i].size() > mv) {
                mv = area[i].size();
                mi = i;
            }
        }
        peakadv_t peak;
        getpeakparaadv(area[mi], peak);
        //printf("fire:%f\n", peak.area);

        if (peak.area > 2000.0f) {
            x = peak.xc;
            y = peak.yc;
            return 1;
        }
    }
    return 0;
}

int game3move(float x, float y) {
    int clickpos = 65536 * (844 + int(y)) + 332 + int(x);
    PostMessage(bbhwndmessage, WM_MOUSEMOVE, 1, clickpos);
    //PostMessage(bbhwndmessage, WM_LBUTTONUP, 1, clickpos);
    return 0;
}

int game3attack(float x) {

    int clickpos = 65536 * 844 + 332 + int(x);
    //PostMessage(bbhwndmessage, WM_LBUTTONUP, 1, clickpos);
    clickpos = 65536 * 900 + 1500;
    //SendMessage(bbhwndmessage, WM_LBUTTONDOWN, 1, clickpos);
    //SendMessage(bbhwndmessage, WM_LBUTTONUP, 1, clickpos);


    PostMessage(bbhwndmessage, WM_KEYDOWN, 'Q', 4128769);
    //keybd_event('5', 0, 0, 0);
    return 0;
}

float getshift(std::vector<float>& xv, std::vector<float>& yv, float& yt,float &yshift) {

    if (xv[1] > 995.0f) {
        yt = 550.0f;
        return 1.4f * (xv[1] + xv[2] + xv[3] + xv[4] - 3858);
    }

    Eigen::MatrixXf ym, xm;

    int n = 5;
    if (yv.size() < 5) {
        n = yv.size();
    }

    ym.resize(n, 2);
    xm.resize(n, 1);
    for (int i = 0; i < n; i++) {
        xm(i) = xv[i];
        ym(i, 0) = 1.0;
        ym(i, 1) = yv[i];
        //ym(i, 2) = yv[i] * yv[i];
        //ym(i, 3) = yv[i] * yv[i] * yv[i];
    }
    Eigen::MatrixXf cm;
    cm = (ym.transpose() * ym).inverse() * (ym.transpose() * xm);

    float shift;
    shift = cm(0) + 833.0f * cm(1) - 959.0f;

    if (shift > 150.0f) {
        yshift = -20.0f;
        printf("shift:%f\n", shift);
        shift = 20.0f * (xv[5] - xv[4] - 10.0f);
    }
    else if (shift < -150.0f) {
        yshift = -20.0f;
        printf("shift:%f\n", shift);
        shift = 20.0f * (xv[5] - xv[4] + 10.0f);
    }

    return shift;
}

int ingame;
float lasty;
std::vector<float> xv, yv;
float yt;

int game3(uchar* data) {
    tc = GetTickCount64();

    for (int i = 300; i < bmp.bmHeight - 1; i++) {
        uchar* sl = data + i * bmp.bmWidth * 4;
        float* dl = screen.data + i * bmp.bmWidth * 3;
        for (int j = 800; j < bmp.bmWidth; j++) {
            dl[j * 3] = sl[j * 4];
            dl[j * 3 + 1] = sl[j * 4 + 1];
            dl[j * 3 + 2] = sl[j * 4 + 2];
        }
    }
    reduce(screen, screenr);


    int cursorx, cursory;
    float error;

    error = geterror(screenr, flagr, 738, 484);

    if (error < 1000000) {

        if (!ingame) {
            Sleep(1500);

            PostMessage(bbhwndmessage, WM_LBUTTONDOWN, 1, 65536 * 844 + 332);
            printf("down\n");
            ingame = 1;
            yt = 520.0f;

            xv.clear();
            yv.clear();
            return 0;
        }


        error = geterror(screenr, cursorr, 468, 157);
        if (error < 2.0e7f) {
            if (!xv.size()) {
                xv.emplace_back(952.0f);
                yv.emplace_back(330.0f);
                printf("fb:\n");
            }
            lasty = 330.0f;
        }
        else {
            float x, y;
            if (xv.size()) {
                if (game3findfire(screen, x, y)) {
                    printf("firepos:%f\n", x);
                    game3move((x - 950.5f) * 9.0f, 80.0f);
                    if (yv[4] > 376.5f) {
                        Sleep(90);
                    }
                    else {
                        Sleep(120);
                    }
                    game3attack(0.0f);
                    return 0;
                }

                //recordimage(data);
                game3findball(screen, x, y);

                if (y != yv[yv.size() - 1]) {
                    xv.emplace_back(x);
                    yv.emplace_back(y);
                    printf("pos:%f,%f\n", x, y);

                    float yshift = 0.0f;
                    if (yv.size() >= 4 && yv[3] > 460.0f) {
                        float shift = getshift(xv, yv, yt, yshift);
                        printf("%f\n", shift);
                        game3move(shift * 1.6f, 80.0f);
                    }
                    else if (y > 455.0f) {
                        float shift = getshift(xv, yv, yt, yshift);
                        printf("%f\n", shift);
                        game3move(shift * 1.5f, yshift);
                    }

                    if (y > yt) {
                        game3attack(0.0f);
                    }
                }
            }
        }

        //Sleep(8);
        //recordimage(data);

    }
    else {
        if (ingame) {
            PostMessage(bbhwndmessage, WM_LBUTTONUP, 1, 65536 * 844 + 332);
            ingame = 0;
            PostMessage(bbhwndmessage, WM_KEYUP, 'Q', 0);
        }
    }

    tl = tc;
    return 0;
}

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
        bbhwnd = bbhwndmessage;
    }
    return true;
}

int main(int argc, char** argv) {

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

    bbhwnd = FindWindowA(NULL, "雷电模拟器");
    printf("hwnd:%d\n", bbhwnd);
    EnumChildWindows(bbhwnd, quitfullscreenProc, 0);
    printf("chwnd:%d\n", bbhwnd);
    bbhdc = GetDC(bbhwnd);
    bbhdcc = CreateCompatibleDC(bbhdc);
    GetClientRect(bbhwnd, &bbrect);
    hbitmap = CreateCompatibleBitmap(bbhdc, bbrect.right - bbrect.left, bbrect.bottom - bbrect.top);
    SelectObject(bbhdcc, hbitmap);
    PrintWindow(bbhwnd, bbhdcc, PW_CLIENTONLY);
    GetObject(hbitmap, sizeof(BITMAP), &bmp);

    data = (unsigned char*)malloc(bmp.bmHeight * bmp.bmWidth * 4);
    tempdata = (int*)malloc(bmp.bmHeight * bmp.bmWidth * 4);

    record = (unsigned char*)malloc(1920 * 1080 * 4 * 256);
    recordi = 0;


    GetBitmapBits(hbitmap, bmp.bmHeight * bmp.bmWidthBytes, data);

    loadimg("./7.png", flag, flagr);
    loadimg("./8_1.png", cursor, cursorr);

    screen.w = bmp.bmWidth;
    screen.h = bmp.bmHeight;
    screen.data = (float*)malloc(screen.w * screen.h * 3 * sizeof(float));
    screen.alpha = nullptr;

    screenr.w = bmp.bmWidth / 2;
    screenr.h = bmp.bmHeight / 2;
    screenr.data = (float*)malloc(screenr.w * screenr.h * 3 * sizeof(float));
    screenr.alpha = nullptr;


    ts = GetTickCount64();
    for (;;) {
        BitBlt(bbhdcc, 0, 0, bbrect.right - bbrect.left, bbrect.bottom - bbrect.top, bbhdc, 0, 0, SRCCOPY);
        //PrintWindow(bbhwnd, bbhdcc, PW_RENDERFULLCONTENT);
        GetBitmapBits(hbitmap, bmp.bmHeight * bmp.bmWidthBytes, data);
        //game2(data);
        game3(data);
    }
    return 0;
}
