#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <gl/GL.h>
#include <cmath>
#include <vector>
#include <algorithm>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

const float PI = 3.14159265f;

struct Building {
    float x, z, w, d, h;
    float r, g, b;
};

HDC dc = nullptr;
HGLRC rc = nullptr;

bool running = true;
bool keys[256] = {};

float px = 0.0f;
float py = 1.7f;
float pz = 8.0f;

float yaw = 0.0f;
float pitch = 0.0f;

int screenW = 1280;
int screenH = 720;

std::vector<Building> city;

void cube(float x, float y, float z,
          float w, float h, float d,
          float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);

    // On yuz
    glVertex3f(x-w/2, y,   z+d/2);
    glVertex3f(x+w/2, y,   z+d/2);
    glVertex3f(x+w/2, y+h, z+d/2);
    glVertex3f(x-w/2, y+h, z+d/2);

    // Arka yuz
    glVertex3f(x+w/2, y,   z-d/2);
    glVertex3f(x-w/2, y,   z-d/2);
    glVertex3f(x-w/2, y+h, z-d/2);
    glVertex3f(x+w/2, y+h, z-d/2);

    // Sol yuz
    glVertex3f(x-w/2, y,   z-d/2);
    glVertex3f(x-w/2, y,   z+d/2);
    glVertex3f(x-w/2, y+h, z+d/2);
    glVertex3f(x-w/2, y+h, z-d/2);

    // Sag yuz
    glVertex3f(x+w/2, y,   z+d/2);
    glVertex3f(x+w/2, y,   z-d/2);
    glVertex3f(x+w/2, y+h, z-d/2);
    glVertex3f(x+w/2, y+h, z+d/2);

    // Ust
    glVertex3f(x-w/2, y+h, z+d/2);
    glVertex3f(x+w/2, y+h, z+d/2);
    glVertex3f(x+w/2, y+h, z-d/2);
    glVertex3f(x-w/2, y+h, z-d/2);

    // Alt
    glVertex3f(x-w/2, y, z-d/2);
    glVertex3f(x+w/2, y, z-d/2);
    glVertex3f(x+w/2, y, z+d/2);
    glVertex3f(x-w/2, y, z+d/2);

    glEnd();
}

void initCity() {
    for (int z = -50; z <= 50; z += 12) {
        for (int x : {-18, -12, 12, 18}) {
            float h = 7.0f +
                float((x*x + z*z + 100) % 17);

            float shade = 0.35f +
                float((x-z+100) % 5) * 0.055f;

            city.push_back({
                float(x), float(z),
                7.0f, 8.0f, h,
                shade, shade+0.04f, shade+0.09f
            });
        }
    }
}

bool blocked(float x, float z) {
    for (const auto& b : city) {
        if (fabsf(x-b.x) < b.w/2 + 0.35f &&
            fabsf(z-b.z) < b.d/2 + 0.35f) {
            return true;
        }
    }
    return false;
}

void perspective(float fov, float aspect,
                 float nearZ, float farZ) {
    float top = nearZ * tanf(fov * PI / 360.0f);
    float right = top * aspect;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glFrustum(-right, right, -top, top, nearZ, farZ);

    glMatrixMode(GL_MODELVIEW);
}

void drawWorld() {
    // Zemin
    cube(0, -0.15f, 0, 200, 0.3f, 200,
         0.20f, 0.29f, 0.20f);

    // Asfalt
    cube(0, 0.015f, 0, 8, 0.04f, 200,
         0.16f, 0.17f, 0.18f);

    // Yol cizgileri
    for (int z = -95; z < 100; z += 8) {
        cube(0, 0.045f, float(z),
             0.12f, 0.015f, 3.5f,
             0.92f, 0.84f, 0.52f);
    }

    // Kaldirimlar
    cube(-5, 0.12f, 0, 1.8f, 0.20f, 200,
         0.48f, 0.48f, 0.46f);

    cube(5, 0.12f, 0, 1.8f, 0.20f, 200,
         0.48f, 0.48f, 0.46f);

    // Binalar
    for (const auto& b : city) {
        cube(b.x, 0.2f, b.z,
             b.w, b.h, b.d,
             b.r, b.g, b.b);

        // Cati
        cube(b.x, b.h + 0.2f, b.z,
             b.w + 0.15f, 0.18f, b.d + 0.15f,
             b.r*0.7f, b.g*0.7f, b.b*0.7f);

        // Pencereler
        for (float y = 1.5f; y < b.h - 0.5f; y += 2.2f) {
            for (float x = -b.w/2 + 1; x < b.w/2; x += 2.0f) {
                cube(b.x+x, y, b.z+b.d/2+0.012f,
                     0.9f, 1.0f, 0.025f,
                     0.30f, 0.55f, 0.68f);
            }
        }
    }

    // Park halindeki araba
    cube(-2.6f, 0.25f, 3.0f,
         1.8f, 0.75f, 3.4f,
         0.65f, 0.08f, 0.055f);

    // Arabanin camli ust bolumu
    cube(-2.6f, 1.0f, 3.0f,
         1.5f, 0.55f, 1.65f,
         0.18f, 0.30f, 0.37f);

    // Tekerlekler
    for (float x : {-3.5f, -1.7f}) {
        for (float z : {1.9f, 4.1f}) {
            cube(x, 0.28f, z,
                 0.35f, 0.48f, 0.65f,
                 0.035f, 0.035f, 0.04f);
        }
    }
}

void update(float dt) {
    float speed = 6.0f * dt;

    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
        speed *= 2.0f;
    }

    float forward = 0.0f;
    float side = 0.0f;

    if (keys['W']) forward += 1.0f;
    if (keys['S']) forward -= 1.0f;
    if (keys['D']) side += 1.0f;
    if (keys['A']) side -= 1.0f;

    // Capraz hareketi normalize et
    float length = sqrtf(forward*forward + side*side);

    if (length > 0.0f) {
        forward /= length;
        side /= length;
    }

    float fx = sinf(yaw);
    float fz = -cosf(yaw);

    float sx = cosf(yaw);
    float sz = sinf(yaw);

    float dx = (fx*forward + sx*side)*speed;
    float dz = (fz*forward + sz*side)*speed;

    if (!blocked(px + dx, pz)) {
        px += dx;
    }

    if (!blocked(px, pz + dz)) {
        pz += dz;
    }

    // Kamera dikey acisi
    if (pitch < -1.45f) pitch = -1.45f;
    if (pitch >  1.45f) pitch =  1.45f;
}

void render() {
    glClearColor(0.43f, 0.62f, 0.80f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    glRotatef(-pitch * 180.0f / PI, 1, 0, 0);
    glRotatef(-yaw * 180.0f / PI, 0, 1, 0);

    glTranslatef(-px, -py, -pz);

    drawWorld();

    SwapBuffers(dc);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg,
                         WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_KEYDOWN:
        if (wp < 256) {
            keys[wp] = true;
        }

        if (wp == VK_ESCAPE) {
            PostQuitMessage(0);
        }
        return 0;

    case WM_KEYUP:
        if (wp < 256) {
            keys[wp] = false;
        }
        return 0;

    case WM_KILLFOCUS:
        for (int i = 0; i < 256; ++i) {
            keys[i] = false;
        }
        return 0;

    case WM_SIZE:
        screenW = LOWORD(lp);
        screenH = HIWORD(lp);

        if (screenW < 1) screenW = 1;
        if (screenH < 1) screenH = 1;

        if (rc && wglGetCurrentContext() == rc) {
            glViewport(0, 0, screenW, screenH);
            perspective(75.0f,
                float(screenW) / float(screenH),
                0.1f, 250.0f);
        }
        return 0;

    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE,
                   LPSTR, int) {
    WNDCLASSA wc = {};

    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = inst;
    wc.lpszClassName = "MiniCity3D";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if (!RegisterClassA(&wc)) {
        return 1;
    }

    HWND hwnd = CreateWindowA(
        "MiniCity3D",
        "MiniCity 3D - Kendi Oyunumuz",
        WS_OVERLAPPEDWINDOW,
        100, 80, screenW, screenH,
        nullptr, nullptr, inst, nullptr
    );

    if (!hwnd) return 1;

    dc = GetDC(hwnd);

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL |
                  PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pf = ChoosePixelFormat(dc, &pfd);

    if (pf == 0 || !SetPixelFormat(dc, pf, &pfd)) {
        ReleaseDC(hwnd, dc);
        DestroyWindow(hwnd);
        return 1;
    }

    rc = wglCreateContext(dc);

    if (!rc || !wglMakeCurrent(dc, rc)) {
        if (rc) wglDeleteContext(rc);
        ReleaseDC(hwnd, dc);
        DestroyWindow(hwnd);
        return 1;
    }

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glShadeModel(GL_FLAT);

    RECT cr;
    GetClientRect(hwnd, &cr);
    screenW = cr.right - cr.left;
    screenH = cr.bottom - cr.top;

    if (screenW < 1) screenW = 1;
    if (screenH < 1) screenH = 1;

    glViewport(0, 0, screenW, screenH);
    perspective(75.0f,
        float(screenW) / float(screenH),
        0.1f, 250.0f);

    initCity();

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    SetForegroundWindow(hwnd);

    RECT r;
    GetClientRect(hwnd, &r);

    POINT center = {
        (r.right-r.left)/2,
        (r.bottom-r.top)/2
    };

    ClientToScreen(hwnd, &center);
    SetCursorPos(center.x, center.y);

    LARGE_INTEGER freq, last, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&last);

    MSG msg = {};
    bool active = true;

    while (active) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                active = false;
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!active) break;

        QueryPerformanceCounter(&now);

        float dt = float(now.QuadPart - last.QuadPart)
                   / float(freq.QuadPart);

        last = now;

        if (dt > 0.05f) dt = 0.05f;
        if (dt < 0.0f) dt = 0.0f;

        if (GetForegroundWindow() == hwnd) {
            POINT m;
            GetCursorPos(&m);

            RECT r;
            GetClientRect(hwnd, &r);

            POINT c = {
                (r.right-r.left)/2,
                (r.bottom-r.top)/2
            };

            ClientToScreen(hwnd, &c);

            yaw += float(m.x-c.x) * 0.0025f;
            pitch -= float(m.y-c.y) * 0.0025f;

            if (pitch < -1.45f) pitch = -1.45f;
            if (pitch >  1.45f) pitch =  1.45f;

            SetCursorPos(c.x, c.y);
        }

        update(dt);
        render();
    }

    wglMakeCurrent(nullptr, nullptr);

    if (rc) {
        wglDeleteContext(rc);
        rc = nullptr;
    }

    ReleaseDC(hwnd, dc);
    DestroyWindow(hwnd);

    return 0;
}