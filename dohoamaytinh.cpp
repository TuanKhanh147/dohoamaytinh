/*
 =====================================================================
  FRUIT CATCH GAME  -  Game hung trai cay (C++ thuan + WinAPI/GDI)
 =====================================================================
  KHONG CAN CAI SFML HAY BAT KY THU VIEN NGOAI NAO.
  Chi dung Windows API co san trong MinGW (di kem Code::Blocks).

  Chuc nang:
   - Trang chu (Menu): Choi, Settings, Thoat
   - Settings: chinh am luong nhac nen & hieu ung (luu vao settings.cfg)
   - Chon Level: De / Trung binh / Kho
   - Gameplay: dieu khien gio bang phim <- -> (hoac A/D) hoac giu chuot
     trai va keo, hung trai cay, tranh bom, tinh diem, mang song,
     do kho tang dan theo thoi gian
   - Am thanh: tat ca hieu ung (hung trai, trung bom, rot trai) duoc
     TAO TU DONG bang code (sinh song sin thanh file WAV trong bo nho),
     phat bang PlaySound() -> khong can file .wav/.mp3 nao ca.
   - Nhac nen (TUY CHON): neu ban dat file assets/bgm.wav canh file
     .exe, game se tu phat lap lai. Khong co file van chay binh thuong.
   - Chu: dung font he thong cua Windows (Arial) -> khong can file font.

  ===================== HUONG DAN BUILD TRONG CODE::BLOCKS ===========
  1. File > New > Project > Win32 GUI project > Next > "Frame Based"
     hoac "Hello world" deu duoc -> dat ten project -> Finish.
     (Neu ban da co project console cu, xem README.md de doi sang GUI.)
  2. Xoa het code mau trong main.cpp cua project, dan toan bo noi dung
     file nay vao thay the.
  3. Vao Project > Build options > chon ten project (dong tren cung,
     KHONG chon rieng Debug/Release) > tab "Linker settings" >
     o "Link libraries" bam Add... > go: winmm  -> OK.
  4. Nhan F9 (Build and run). Xong!
 =====================================================================
*/
/*
 =====================================================================
  FRUIT CATCH GAME  -  Game hung trai cay (C++ thuan + WinAPI/GDI)
 =====================================================================
  KHONG CAN CAI SFML HAY BAT KY THU VIEN NGOAI NAO.
  Chi dung Windows API co san trong MinGW (di kem Code::Blocks).

  Chuc nang:
   - Trang chu (Menu): Choi, Settings, Thoat
   - Settings: chinh am luong nhac nen & hieu ung (luu vao settings.cfg)
   - Chon Level: De / Trung binh / Kho
   - Gameplay: dieu khien gio bang phim <- -> (hoac A/D) hoac giu chuot
     trai va keo, hung trai cay, tranh bom, tinh diem, mang song,
     do kho tang dan theo thoi gian
   - Am thanh: tat ca hieu ung (hung trai, trung bom, rot trai) duoc
     TAO TU DONG bang code (sinh song sin thanh file WAV trong bo nho),
     phat bang PlaySound() -> khong can file .wav/.mp3 nao ca.
   - Nhac nen (TUY CHON): neu ban dat file assets/bgm.wav canh file
     .exe, game se tu phat lap lai. Khong co file van chay binh thuong.
   - Chu: dung font he thong cua Windows (Arial) -> khong can file font.

  ===================== HUONG DAN BUILD TRONG CODE::BLOCKS ===========
  1. File > New > Project > Win32 GUI project > Next > "Frame Based"
     hoac "Hello world" deu duoc -> dat ten project -> Finish.
     (Neu ban da co project console cu, xem README.md de doi sang GUI.)
  2. Xoa het code mau trong main.cpp cua project, dan toan bo noi dung
     file nay vao thay the.
  3. Vao Project > Build options > chon ten project (dong tren cung,
     KHONG chon rieng Debug/Release) > tab "Linker settings" >
     o "Link libraries" bam Add... > go: winmm  -> OK.
  4. Nhan F9 (Build and run). Xong!
 =====================================================================
*/

#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <windowsx.h>   // GET_X_LPARAM, GET_Y_LPARAM
#include <mmsystem.h>   // PlaySound, mciSendString
#include <vector>
#include <string>
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <cstdint>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "msimg32.lib")  // can cho GradientFill

using namespace std;

const int WINDOW_W = 900;
const int WINDOW_H = 650;

// =====================================================================
//  TIEN ICH: sinh file WAV trong bo nho (khong can file .wav that)
// =====================================================================
static void pushStr(vector<BYTE>& v, const char* s) { for (int i = 0; i < 4; i++) v.push_back((BYTE)s[i]); }
static void push32(vector<BYTE>& v, uint32_t x) { v.push_back(x & 0xFF); v.push_back((x >> 8) & 0xFF); v.push_back((x >> 16) & 0xFF); v.push_back((x >> 24) & 0xFF); }
static void push16(vector<BYTE>& v, uint16_t x) { v.push_back(x & 0xFF); v.push_back((x >> 8) & 0xFF); }

// Tao 1 khoi song sin (dung de ghep thanh am thanh)
static void appendTone(
    vector<int16_t>& samples,
    double freq,
    double durationSec,
    double amplitude,
    bool fadeOut,
    int sampleRate
) {
    const double PI = 3.14159265358979323846;

    size_t n = (size_t)(durationSec * sampleRate);

    for (size_t i = 0; i < n; i++) {
        double t = (double)i / sampleRate;
        double env = fadeOut ? 1.0 - (double)i / n : 1.0;

        samples.push_back(
            (int16_t)(
                3000.0 * amplitude * env *
                sin(2.0 * PI * freq * t)
                )
        );
    }
}
// Ghep header WAV chuan tu du lieu mau (PCM 16-bit mono)
static vector<BYTE> buildWav(const vector<int16_t>& samples, int sampleRate = 44100) {
    vector<BYTE> wav;
    int dataSize = (int)samples.size() * 2;
    pushStr(wav, "RIFF"); push32(wav, 36 + dataSize); pushStr(wav, "WAVE");
    pushStr(wav, "fmt "); push32(wav, 16); push16(wav, 1); push16(wav, 1);
    push32(wav, sampleRate); push32(wav, sampleRate * 2); push16(wav, 2); push16(wav, 16);
    pushStr(wav, "data"); push32(wav, dataSize);
    const BYTE* raw = reinterpret_cast<const BYTE*>(samples.data());
    wav.insert(wav.end(), raw, raw + dataSize);
    return wav;
}

// Am thanh hung trai (2 not len cao, vui tai), trung bom (tram, keo dai), rot trai (ngan, thap)
static vector<BYTE> g_catchWav, g_missWav, g_bombWav;

static void buildAllSounds(double volume01) {
    vector<int16_t> s1;
    appendTone(s1, 880.0, 0.06, volume01, true, 44100);
    appendTone(s1, 1320.0, 0.09, volume01, true, 44100);
    g_catchWav = buildWav(s1);

    vector<int16_t> s2;
    appendTone(s2, 220.0, 0.13, volume01 * 0.8, true, 44100);
    g_missWav = buildWav(s2);

    vector<int16_t> s3;
    appendTone(s3, 90.0, 0.30, volume01, true, 44100);
    g_bombWav = buildWav(s3);
}

static void playCatch() { PlaySoundA((LPCSTR)g_catchWav.data(), NULL, SND_MEMORY | SND_ASYNC); }
static void playMiss() { PlaySoundA((LPCSTR)g_missWav.data(), NULL, SND_MEMORY | SND_ASYNC); }
static void playBomb() { PlaySoundA((LPCSTR)g_bombWav.data(), NULL, SND_MEMORY | SND_ASYNC); }

static bool g_musicPlaying = false;
static void startBackgroundMusic() {
    ifstream f("assets/bgm.wav");
    if (!f.good()) return; // khong co file thi bo qua, khong loi
    f.close();
    mciSendStringA("open \"assets/bgm.wav\" type waveaudio alias bgm", NULL, 0, NULL);
    mciSendStringA("play bgm repeat", NULL, 0, NULL);
    g_musicPlaying = true;
}
static void stopBackgroundMusic() {
    if (g_musicPlaying) { mciSendStringA("close bgm", NULL, 0, NULL); g_musicPlaying = false; }
}

// =====================================================================
//  SETTINGS
// =====================================================================
struct Settings {
    int musicVolume = 60; // 0..100 (chi de hien thi, nhac dung MCI mac dinh)
    int sfxVolume = 80;   // 0..100 -> anh huong truc tiep bien do am thanh sinh ra

    void load() {
        ifstream f("settings.cfg");
        if (f) f >> musicVolume >> sfxVolume;
    }
    void save() {
        ofstream f("settings.cfg");
        f << musicVolume << " " << sfxVolume;
    }
};
static Settings g_settings;

// =====================================================================
//  BUTTON (dung RECT + text, ve bang GDI)
// =====================================================================
struct Button {
    RECT rect;
    string label;
    Button() {}
    Button(int x, int y, int w, int h, const string& text) {
        rect = { x, y, x + w, y + h };
        label = text;
    }
    bool hit(POINT p) const { return PtInRect(&rect, p); }
};

static POINT g_mouse = { -1, -1 };
static bool g_leftDown = false;

// =====================================================================
//  FRUIT / BASKET
// =====================================================================
enum class FruitType { APPLE, BANANA, GRAPE, ORANGE, BOMB };

struct Fruit {
    double x, y;
    double speed;
    FruitType type;
    int radius;
    Fruit(double x_, double y_, double speed_, FruitType t) : x(x_), y(y_), speed(speed_), type(t) {
        radius = (t == FruitType::BOMB) ? 18 : 20;
    }
    void update(double dt) { y += speed * dt; }
    RECT bounds() const { return { (LONG)(x - radius), (LONG)(y - radius), (LONG)(x + radius), (LONG)(y + radius) }; }
};

struct Basket {
    double x, y, w, h, speed;
    Basket() { w = 130; h = 34; x = WINDOW_W / 2.0 - w / 2.0; y = WINDOW_H - 70.0; speed = 550.0; }
    void update(double dt, bool left, bool right) {
        if (left) x -= speed * dt;
        if (right) x += speed * dt;
        if (x < 0) x = 0;
        if (x + w > WINDOW_W) x = WINDOW_W - w;
    }
    void followMouse(double mx) {
        x = mx - w / 2.0;
        if (x < 0) x = 0;
        if (x + w > WINDOW_W) x = WINDOW_W - w;
    }
    RECT bounds() const { return { (LONG)x, (LONG)y, (LONG)(x + w), (LONG)(y + h) }; }
};

static bool rectsIntersect(const RECT& a, const RECT& b) {
    RECT tmp;
    return IntersectRect(&tmp, &a, &b) != 0;
}

// =====================================================================
//  GAME STATE
// =====================================================================
enum class State { MENU, SETTINGS, LEVEL_SELECT, PLAYING, PAUSED, GAME_OVER };
static State g_state = State::MENU;

struct LevelConfig {
    string name;
    double fallSpeedMin, fallSpeedMax;
    double spawnInterval;
    double bombChance;
    int startLives;
};
static vector<LevelConfig> g_levels = {
    { "DE (Easy)",           180.0, 260.0, 1.10, 0.06, 5 },
    { "TRUNG BINH (Medium)", 240.0, 340.0, 0.85, 0.12, 4 },
    { "KHO (Hard)",          320.0, 460.0, 0.60, 0.20, 3 },
};

static int g_currentLevel = 0;
static Basket g_basket;
static vector<Fruit> g_fruits;
static int g_score = 0, g_lives = 5;
static double g_spawnTimer = 0, g_difficultyTimer = 0, g_speedMult = 1.0;

// Buttons cho tung man hinh
static Button btnPlay, btnSettings, btnExit;
static Button btnMusicUp, btnMusicDown, btnSfxUp, btnSfxDown, btnBackSettings;
static vector<Button> btnLevels;
static Button btnBackLevels;
static Button btnResume, btnQuitMenu;
static Button btnRetry, btnMenuFromOver;

// Fonts
static HFONT g_fontTitle, g_fontNormal, g_fontButton, g_fontSmall;

// =====================================================================
//  KHOI TAO UI
// =====================================================================
static void buildButtons() {
    int cx = WINDOW_W / 2 - 110;
    btnPlay = Button(cx, 260, 220, 60, "CHOI NGAY");
    btnSettings = Button(cx, 340, 220, 60, "SETTINGS");
    btnExit = Button(cx, 420, 220, 60, "THOAT");

    int scx = WINDOW_W / 2;
    btnMusicDown = Button(scx + 60, 220, 50, 50, "-");
    btnMusicUp = Button(scx + 180, 220, 50, 50, "+");
    btnSfxDown = Button(scx + 60, 320, 50, 50, "-");
    btnSfxUp = Button(scx + 180, 320, 50, 50, "+");
    btnBackSettings = Button(scx - 110, 480, 220, 55, "QUAY LAI");

    int lcx = WINDOW_W / 2 - 130;
    const char* names[3] = { "DE (Easy)", "TRUNG BINH (Medium)", "KHO (Hard)" };
    btnLevels.clear();
    for (int i = 0; i < 3; i++) btnLevels.push_back(Button(lcx, 220 + i * 90, 260, 65, names[i]));
    btnBackLevels = Button(lcx, 220 + 3 * 90 + 10, 260, 55, "QUAY LAI");

    btnResume = Button(WINDOW_W / 2 - 110, 260, 220, 55, "TIEP TUC");
    btnQuitMenu = Button(WINDOW_W / 2 - 110, 330, 220, 55, "VE TRANG CHU");

    btnRetry = Button(WINDOW_W / 2 - 110, 360, 220, 55, "CHOI LAI");
    btnMenuFromOver = Button(WINDOW_W / 2 - 110, 430, 220, 55, "VE TRANG CHU");
}

// =====================================================================
//  GAMEPLAY LOGIC
// =====================================================================
static void startLevel(int idx) {
    g_currentLevel = idx;
    g_fruits.clear();
    g_score = 0;
    g_lives = g_levels[idx].startLives;
    g_spawnTimer = 0; g_difficultyTimer = 0; g_speedMult = 1.0;
    g_basket = Basket();
    g_state = State::PLAYING;
    startBackgroundMusic();
}

static void spawnFruit() {
    const LevelConfig& lc = g_levels[g_currentLevel];
    double x = 30 + (double)rand() / RAND_MAX * (WINDOW_W - 60);
    double speed = (lc.fallSpeedMin + (double)rand() / RAND_MAX * (lc.fallSpeedMax - lc.fallSpeedMin)) * g_speedMult;
    bool isBomb = ((double)rand() / RAND_MAX) < lc.bombChance;
    FruitType t;
    if (isBomb) t = FruitType::BOMB;
    else { int r = rand() % 4; t = (r == 0) ? FruitType::APPLE : (r == 1) ? FruitType::BANANA : (r == 2) ? FruitType::GRAPE : FruitType::ORANGE; }
    g_fruits.emplace_back(x, -30, speed, t);
}

static void updatePlaying(double dt) {
    bool left = (GetAsyncKeyState(VK_LEFT) & 0x8000) || (GetAsyncKeyState('A') & 0x8000);
    bool right = (GetAsyncKeyState(VK_RIGHT) & 0x8000) || (GetAsyncKeyState('D') & 0x8000);
    g_basket.update(dt, left, right);
    if (g_leftDown) g_basket.followMouse(g_mouse.x);

    g_difficultyTimer += dt;
    if (g_difficultyTimer >= 12.0) { g_difficultyTimer = 0; g_speedMult *= 1.08; }

    g_spawnTimer += dt;
    double interval = g_levels[g_currentLevel].spawnInterval / g_speedMult;
    if (g_spawnTimer >= interval) { g_spawnTimer = 0; spawnFruit(); }

    RECT basketRect = g_basket.bounds();
    for (size_t i = 0; i < g_fruits.size(); ) {
        g_fruits[i].update(dt);
        bool remove = false;
        RECT fr = g_fruits[i].bounds();
        if (rectsIntersect(fr, basketRect)) {
            if (g_fruits[i].type == FruitType::BOMB) { playBomb(); g_lives--; }
            else { playCatch(); g_score += 10; }
            remove = true;
        }
        else if (g_fruits[i].y - g_fruits[i].radius > WINDOW_H) {
            if (g_fruits[i].type != FruitType::BOMB) { playMiss(); g_lives--; }
            remove = true;
        }
        if (remove) g_fruits.erase(g_fruits.begin() + i); else i++;
    }

    if (g_lives <= 0) { g_state = State::GAME_OVER; stopBackgroundMusic(); }
}

// =====================================================================
//  XU LY CLICK
// =====================================================================
static void handleClick(POINT p) {
    switch (g_state) {
    case State::MENU:
        if (btnPlay.hit(p)) g_state = State::LEVEL_SELECT;
        else if (btnSettings.hit(p)) g_state = State::SETTINGS;
        else if (btnExit.hit(p)) PostQuitMessage(0);
        break;
    case State::SETTINGS:
        if (btnMusicUp.hit(p)) { g_settings.musicVolume = min(100, g_settings.musicVolume + 10); g_settings.save(); }
        if (btnMusicDown.hit(p)) { g_settings.musicVolume = max(0, g_settings.musicVolume - 10); g_settings.save(); }
        if (btnSfxUp.hit(p)) { g_settings.sfxVolume = min(100, g_settings.sfxVolume + 10); buildAllSounds(g_settings.sfxVolume / 100.0); g_settings.save(); playCatch(); }
        if (btnSfxDown.hit(p)) { g_settings.sfxVolume = max(0, g_settings.sfxVolume - 10); buildAllSounds(g_settings.sfxVolume / 100.0); g_settings.save(); playCatch(); }
        if (btnBackSettings.hit(p)) g_state = State::MENU;
        break;
    case State::LEVEL_SELECT:
        for (size_t i = 0; i < btnLevels.size(); i++) if (btnLevels[i].hit(p)) startLevel((int)i);
        if (btnBackLevels.hit(p)) g_state = State::MENU;
        break;
    case State::PAUSED:
        if (btnResume.hit(p)) g_state = State::PLAYING;
        if (btnQuitMenu.hit(p)) { g_state = State::MENU; stopBackgroundMusic(); }
        break;
    case State::GAME_OVER:
        if (btnRetry.hit(p)) startLevel(g_currentLevel);
        if (btnMenuFromOver.hit(p)) g_state = State::MENU;
        break;
    default: break;
    }
}

// =====================================================================
//  VE (RENDER) BANG GDI
// =====================================================================
static void fillRect(HDC hdc, RECT r, COLORREF c) {
    HBRUSH b = CreateSolidBrush(c);
    FillRect(hdc, &r, b);
    DeleteObject(b);
}
static void gradientBG(HDC hdc, COLORREF top, COLORREF bottom) {
    TRIVERTEX vt[2];
    vt[0].x = 0; vt[0].y = 0;
    vt[0].Red = GetRValue(top) << 8; vt[0].Green = GetGValue(top) << 8; vt[0].Blue = GetBValue(top) << 8; vt[0].Alpha = 0;
    vt[1].x = WINDOW_W; vt[1].y = WINDOW_H;
    vt[1].Red = GetRValue(bottom) << 8; vt[1].Green = GetGValue(bottom) << 8; vt[1].Blue = GetBValue(bottom) << 8; vt[1].Alpha = 0;
    GRADIENT_RECT gr = { 0, 1 };
    GradientFill(hdc, vt, 2, &gr, 1, GRADIENT_FILL_RECT_V);
}
static void drawTextCentered(HDC hdc, RECT r, const string& text, HFONT font, COLORREF color) {
    HFONT old = (HFONT)SelectObject(hdc, font);
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    DrawTextA(hdc, text.c_str(), -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, old);
}
static void drawTextAt(HDC hdc, int x, int y, const string& text, HFONT font, COLORREF color) {
    HFONT old = (HFONT)SelectObject(hdc, font);
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    TextOutA(hdc, x, y, text.c_str(), (int)text.size());
    SelectObject(hdc, old);
}
static void drawButton(HDC hdc, const Button& b) {
    bool hover = PtInRect(&b.rect, g_mouse);
    fillRect(hdc, b.rect, hover ? RGB(100, 160, 210) : RGB(70, 130, 180));
    FrameRect(hdc, &b.rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
    drawTextCentered(hdc, b.rect, b.label, g_fontButton, RGB(255, 255, 255));
}

static void renderMenu(HDC hdc) {
    gradientBG(hdc, RGB(135, 206, 235), RGB(70, 130, 180));
    RECT titleR = { 0, 110, WINDOW_W, 170 };
    drawTextCentered(hdc, titleR, "FRUIT CATCH GAME", g_fontTitle, RGB(255, 255, 255));
    RECT subR = { 0, 175, WINDOW_W, 205 };
    drawTextCentered(hdc, subR, "Hung trai cay - Tranh bom!", g_fontNormal, RGB(255, 255, 220));
    drawButton(hdc, btnPlay); drawButton(hdc, btnSettings); drawButton(hdc, btnExit);
}

static void renderSettings(HDC hdc) {
    gradientBG(hdc, RGB(100, 150, 200), RGB(60, 90, 140));
    RECT titleR = { 0, 80, WINDOW_W, 140 };
    drawTextCentered(hdc, titleR, "SETTINGS", g_fontTitle, RGB(255, 255, 255));
    drawTextAt(hdc, WINDOW_W / 2 - 150, 235, "Am nhac: " + to_string(g_settings.musicVolume) + "%", g_fontNormal, RGB(255, 255, 255));
    drawButton(hdc, btnMusicDown); drawButton(hdc, btnMusicUp);
    drawTextAt(hdc, WINDOW_W / 2 - 150, 335, "Hieu ung: " + to_string(g_settings.sfxVolume) + "%", g_fontNormal, RGB(255, 255, 255));
    drawButton(hdc, btnSfxDown); drawButton(hdc, btnSfxUp);
    drawButton(hdc, btnBackSettings);
}

static void renderLevelSelect(HDC hdc) {
    gradientBG(hdc, RGB(120, 190, 120), RGB(40, 100, 60));
    RECT titleR = { 0, 110, WINDOW_W, 170 };
    drawTextCentered(hdc, titleR, "CHON MAN CHOI", g_fontTitle, RGB(255, 255, 255));
    for (auto& b : btnLevels) drawButton(hdc, b);
    drawButton(hdc, btnBackLevels);
}

static void renderFruit(HDC hdc, const Fruit& f) {
    COLORREF c;
    switch (f.type) {
    case FruitType::APPLE:  c = RGB(220, 20, 60); break;
    case FruitType::BANANA: c = RGB(255, 220, 0); break;
    case FruitType::GRAPE:  c = RGB(140, 40, 180); break;
    case FruitType::ORANGE: c = RGB(255, 140, 0); break;
    default:                c = RGB(30, 30, 30); break;
    }
    HBRUSH br = CreateSolidBrush(c);
    HBRUSH old = (HBRUSH)SelectObject(hdc, br);
    Ellipse(hdc, (int)(f.x - f.radius), (int)(f.y - f.radius), (int)(f.x + f.radius), (int)(f.y + f.radius));
    SelectObject(hdc, old);
    DeleteObject(br);
}

static void renderPlaying(HDC hdc) {
    gradientBG(hdc, RGB(180, 230, 255), RGB(230, 250, 255));
    for (auto& f : g_fruits) renderFruit(hdc, f);

    RECT br = g_basket.bounds();
    fillRect(hdc, br, RGB(139, 90, 43));
    FrameRect(hdc, &br, (HBRUSH)GetStockObject(BLACK_BRUSH));

    drawTextAt(hdc, 20, 15, "Diem: " + to_string(g_score), g_fontNormal, RGB(20, 20, 20));
    drawTextAt(hdc, 20, 50, "Mang: " + to_string(max(g_lives, 0)), g_fontNormal, RGB(150, 0, 0));
    drawTextAt(hdc, WINDOW_W - 260, 15, g_levels[g_currentLevel].name, g_fontSmall, RGB(20, 20, 20));
    drawTextAt(hdc, WINDOW_W - 170, WINDOW_H - 30, "ESC = Tam dung", g_fontSmall, RGB(60, 60, 60));
}

static void renderPauseOverlay(HDC hdc) {
    // GDI co ban khong ho tro alpha-blend de lam nen mo trong suot,
    // nen ta dung mot nen dam mau phu kin man hinh cho de nhin chu.
    RECT full = { 0, 0, WINDOW_W, WINDOW_H };
    fillRect(hdc, full, RGB(15, 15, 25));
    RECT titleR = { 0, 140, WINDOW_W, 200 };
    drawTextCentered(hdc, titleR, "TAM DUNG", g_fontTitle, RGB(255, 255, 255));
    drawButton(hdc, btnResume);
    drawButton(hdc, btnQuitMenu);
}

static void renderGameOver(HDC hdc) {
    gradientBG(hdc, RGB(60, 20, 20), RGB(20, 5, 5));
    RECT r1 = { 0, 120, WINDOW_W, 180 };
    drawTextCentered(hdc, r1, "GAME OVER", g_fontTitle, RGB(255, 80, 80));
    RECT r2 = { 0, 210, WINDOW_W, 250 };
    drawTextCentered(hdc, r2, "Diem cuoi cung: " + to_string(g_score), g_fontNormal, RGB(255, 255, 255));
    RECT r3 = { 0, 260, WINDOW_W, 295 };
    drawTextCentered(hdc, r3, "Man: " + g_levels[g_currentLevel].name, g_fontSmall, RGB(220, 220, 220));
    drawButton(hdc, btnRetry);
    drawButton(hdc, btnMenuFromOver);
}

static void render(HDC hdc) {
    switch (g_state) {
    case State::MENU: renderMenu(hdc); break;
    case State::SETTINGS: renderSettings(hdc); break;
    case State::LEVEL_SELECT: renderLevelSelect(hdc); break;
    case State::PLAYING: renderPlaying(hdc); break;
    case State::PAUSED: renderPlaying(hdc); renderPauseOverlay(hdc); break;
    case State::GAME_OVER: renderGameOver(hdc); break;
    }
}

// =====================================================================
//  DOUBLE BUFFER
// =====================================================================
static HDC g_memDC = NULL;
static HBITMAP g_memBmp = NULL;

static void setupBackBuffer(HWND hwnd) {
    HDC hdc = GetDC(hwnd);
    g_memDC = CreateCompatibleDC(hdc);
    g_memBmp = CreateCompatibleBitmap(hdc, WINDOW_W, WINDOW_H);
    SelectObject(g_memDC, g_memBmp);
    ReleaseDC(hwnd, hdc);
}

// =====================================================================
//  WINDOW PROCEDURE
// =====================================================================
static ULONGLONG g_lastTick = 0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        srand((unsigned)time(nullptr));
        g_settings.load();
        buildAllSounds(g_settings.sfxVolume / 100.0);
        buildButtons();
        setupBackBuffer(hwnd);

        g_fontTitle = CreateFontA(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Arial");
        g_fontNormal = CreateFontA(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Arial");
        g_fontButton = CreateFontA(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Arial");
        g_fontSmall = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Arial");

        g_lastTick = GetTickCount64();
        SetTimer(hwnd, 1, 16, NULL); // ~60 FPS
        return 0;
    }
    case WM_TIMER: {
        ULONGLONG now = GetTickCount64();
        double dt = (now - g_lastTick) / 1000.0;
        g_lastTick = now;
        if (dt > 0.1) dt = 0.1; // tranh nhay frame qua lon
        if (g_state == State::PLAYING) updatePlaying(dt);
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }
    case WM_LBUTTONDOWN: {
        g_leftDown = true;
        POINT p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        handleClick(p);
        return 0;
    }
    case WM_LBUTTONUP:
        g_leftDown = false;
        return 0;
    case WM_MOUSEMOVE:
        g_mouse.x = GET_X_LPARAM(lParam);
        g_mouse.y = GET_Y_LPARAM(lParam);
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            if (g_state == State::PLAYING) g_state = State::PAUSED;
            else if (g_state == State::PAUSED) g_state = State::PLAYING;
        }
        return 0;
    case WM_ERASEBKGND:
        return 1; // chong nhap nhay, ta tu ve toan bo nen trong render()
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        render(g_memDC);
        BitBlt(hdc, 0, 0, WINDOW_W, WINDOW_H, g_memDC, 0, 0, SRCCOPY);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        KillTimer(hwnd, 1);
        stopBackgroundMusic();
        DeleteObject(g_fontTitle); DeleteObject(g_fontNormal);
        DeleteObject(g_fontButton); DeleteObject(g_fontSmall);
        DeleteDC(g_memDC); DeleteObject(g_memBmp);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

// =====================================================================
//  DIEM BAT DAU CHUONG TRINH
//  Them main() de Visual Studio co entry point va tao cua so WinAPI
// =====================================================================
int main() {
    HINSTANCE hInstance = GetModuleHandleA(NULL);

    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = "FruitCatchGameWindow";

    if (!RegisterClassExA(&wc)) {
        MessageBoxA(NULL, "Khong the dang ky lop cua so.", "Loi", MB_OK | MB_ICONERROR);
        return 1;
    }

    RECT rc = { 0, 0, WINDOW_W, WINDOW_H };
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);

    HWND hwnd = CreateWindowExA(
        0,
        wc.lpszClassName,
        "Fruit Catch Game",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rc.right - rc.left,
        rc.bottom - rc.top,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hwnd) {
        MessageBoxA(NULL, "Khong the tao cua so game.", "Loi", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return (int)msg.wParam;
}

