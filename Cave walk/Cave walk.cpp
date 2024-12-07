#include "framework.h"
#include "Cave walk.h"
#include <mmsystem.h>
#include <d2d1.h>
#include <dwrite.h>
#include "ErrH.h"
#include "FCheck.h"
#include "D2BMPLOADER.h"
#include "gifresizer.h"
#include "dung_engine.h"
#include "chrono"

#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "d2d1.lib")
#pragma comment (lib, "dwrite.lib")
#pragma comment (lib, "errh.lib")
#pragma comment (lib, "fcheck.lib")
#pragma comment (lib, "d2bmploader.lib")
#pragma comment (lib, "gifresizer.lib")
#pragma comment (lib, "dung_engine.lib")

constexpr wchar_t bWinClassName[]{ L"OneCaveWalk" };

constexpr char tmp_file[]{ ".\\res\\data\\temp.dat" };
constexpr wchar_t Ltmp_file[]{ L".\\res\\data\\temp.dat" };
constexpr wchar_t help_file[]{ L".\\res\\data\\help.dat" };
constexpr wchar_t save_file[]{ L".\\res\\data\\save.dat" };
constexpr wchar_t record_file[]{ L".\\res\\data\\record.dat" };
constexpr wchar_t sound_file[]{ L".\\res\\snd\\main.wav" };

constexpr int mNew{ 1001 };
constexpr int mLvl{ 1002 };
constexpr int mExit{ 1003 };
constexpr int mSave{ 1004 };
constexpr int mLoad{ 1005 };
constexpr int mHoF{ 1006 };

constexpr int record{ 2001 };
constexpr int first_record{ 2002 };
constexpr int no_record{ 2003 };

////////////////////////////////////////////////////////////////

WNDCLASS bWinClass{};
HINSTANCE bIns{ nullptr };
HWND bHwnd{ nullptr };
HICON Icon{ nullptr };
HCURSOR mainCursor{ nullptr };
HCURSOR outCursor{ nullptr };
HMENU bBar{ nullptr };
HMENU bMain{ nullptr };
HMENU bStore{ nullptr };
HDC PaintDC{ nullptr };
MSG bMsg{};
BOOL bRet{ -1 };
PAINTSTRUCT bPaint{};
POINT cur_pos{ 0,0 };
UINT bTimer{ 0 };

D2D1_RECT_F b1Rect{ 10.0f, 5.0f, scr_width / 3 - 50.0f, 50.0f };
D2D1_RECT_F b2Rect{ scr_width / 3 + 10.0f, 5.0f, scr_width * 2 / 3 - 50.0f, 50.0f };
D2D1_RECT_F b3Rect{ scr_width * 2 / 3 + 10.0f, 5.0f, scr_width, 50.0f };

D2D1_RECT_F ViewMapRect{ -200.0f, -100.0f, map_width, map_height };

bool pause = false;
bool sound = true;
bool show_help = false;
bool in_client = true;
bool b1Hglt = false;
bool b2Hglt = false;
bool b3Hglt = false;
bool name_set = false;

wchar_t current_player[16] = L"ONE CAVEMAN";

int level = 1;
int score = 0;
int mins = 0;
int secs = 0;

int club_lifes = 100;
int axe_lifes = 100;
int sword_lifes = 100;

int cloak_lifes = 100;
int mail_lifes = 100;

int field_frame = 0;
int field_delay = 3;

dll::RANDiT RandGenerator{};

///////////////////////////////////////////////////////

ID2D1Factory* iFactory{ nullptr };
ID2D1HwndRenderTarget* Draw{ nullptr };

ID2D1RadialGradientBrush* bckgBrush{ nullptr };
ID2D1SolidColorBrush* txtBrush{ nullptr };
ID2D1SolidColorBrush* hgltBrush{ nullptr };
ID2D1SolidColorBrush* inactBrush{ nullptr };

IDWriteFactory* iWriteFactory{ nullptr };
IDWriteTextFormat* nrmTextFormat{ nullptr };
IDWriteTextFormat* midTextFormat{ nullptr };
IDWriteTextFormat* bigTextFormat{ nullptr };

ID2D1Bitmap* bmpAxe{ nullptr };
ID2D1Bitmap* bmpPotion{ nullptr };
ID2D1Bitmap* bmpBrick1{ nullptr };
ID2D1Bitmap* bmpBrick2{ nullptr };
ID2D1Bitmap* bmpCloak{ nullptr };
ID2D1Bitmap* bmpClub{ nullptr };
ID2D1Bitmap* bmpCrystal{ nullptr };
ID2D1Bitmap* bmpGold{ nullptr };
ID2D1Bitmap* bmpMail{ nullptr };
ID2D1Bitmap* bmpSword{ nullptr };
ID2D1Bitmap* bmpRIP{ nullptr };

ID2D1Bitmap* bmpHeroL[4]{ nullptr };
ID2D1Bitmap* bmpHeroR[4]{ nullptr };

ID2D1Bitmap* bmpClubHeroL[4]{ nullptr };
ID2D1Bitmap* bmpClubHeroR[4]{ nullptr };

ID2D1Bitmap* bmpAxeHeroL[4]{ nullptr };
ID2D1Bitmap* bmpAxeHeroR[4]{ nullptr };

ID2D1Bitmap* bmpSwordHeroL[4]{ nullptr };
ID2D1Bitmap* bmpSwordHeroR[4]{ nullptr };

ID2D1Bitmap* bmpEvil1L[16]{ nullptr };
ID2D1Bitmap* bmpEvil1R[16]{ nullptr };

ID2D1Bitmap* bmpEvil2[37]{ nullptr };

ID2D1Bitmap* bmpEvil3L[32]{ nullptr };
ID2D1Bitmap* bmpEvil3R[32]{ nullptr };

ID2D1Bitmap* bmpEvil4[29]{ nullptr };

ID2D1Bitmap* bmpEvil5[36]{ nullptr };

ID2D1Bitmap* bmpField[75]{ nullptr };
ID2D1Bitmap* bmpIntro[56]{ nullptr };

////////////////////////////////////////////

dll::creature_ptr Hero = nullptr;
bool move_hero = false;

std::vector<dll::asset_ptr> vObstacles;
std::vector<dll::asset_ptr>vCrystals;



///////////////////////////////////////////

template<typename T> concept HasRelease = requires(T check)
{
    check.Release();
};
template<HasRelease T> bool ClearHeap(T** var)
{
    if (*var)
    {
        (*var)->Release();
        (*var) = nullptr;
        return true;
    }
    return false;
}
void LogError(LPCWSTR what)
{
    std::wofstream log(L".\\res\\data\\error.log", std::ios::app);
    log << what << L" Time stamp: " << std::chrono::system_clock::now() << std::endl;
    log.close();
}
void ReleaseResources()
{
    if (!ClearHeap(&iFactory))LogError(L"Error releasing iFactory !");
    if (!ClearHeap(&Draw))LogError(L"Error releasing Draw !");
    if (!ClearHeap(&bckgBrush))LogError(L"Error releasing bckgBrush !");
    if (!ClearHeap(&txtBrush))LogError(L"Error releasing txtBrush !");
    if (!ClearHeap(&hgltBrush))LogError(L"Error releasing hgltBrush !");
    if (!ClearHeap(&inactBrush))LogError(L"Error releasing inactBrush !");

    if (!ClearHeap(&iWriteFactory))LogError(L"Error releasing iWriteFactory !");
    if (!ClearHeap(&nrmTextFormat))LogError(L"Error releasing nrmTextFormat !");
    if (!ClearHeap(&midTextFormat))LogError(L"Error releasing midTextFormat !");
    if (!ClearHeap(&bigTextFormat))LogError(L"Error releasing bigTextFormat !");

    if (!ClearHeap(&bmpAxe))LogError(L"Error releasing bmpAxe !");
    if (!ClearHeap(&bmpPotion))LogError(L"Error releasing bmpPotion !");
    if (!ClearHeap(&bmpBrick1))LogError(L"Error releasing bmpBrick1 !");
    if (!ClearHeap(&bmpBrick2))LogError(L"Error releasing bmpBrick2 !");
    if (!ClearHeap(&bmpCloak))LogError(L"Error releasing bmpCloak !");
    if (!ClearHeap(&bmpClub))LogError(L"Error releasing bmpClub !");
    if (!ClearHeap(&bmpCrystal))LogError(L"Error releasing bmpCrystal !");
    if (!ClearHeap(&bmpGold))LogError(L"Error releasing bmpGold !");
    if (!ClearHeap(&bmpMail))LogError(L"Error releasing bmpMail !");
    if (!ClearHeap(&bmpSword))LogError(L"Error releasing bmpSword !");
    if (!ClearHeap(&bmpRIP))LogError(L"Error releasing bmpRIP !");

    for (int i = 0; i < 4; i++)if (!ClearHeap(&bmpHeroL[i]))LogError(L"Error releasing HeroL !");
    for (int i = 0; i < 4; i++)if (!ClearHeap(&bmpHeroR[i]))LogError(L"Error releasing HeroR !");
    for (int i = 0; i < 4; i++)if (!ClearHeap(&bmpAxeHeroL[i]))LogError(L"Error releasing HeroAxeL !");
    for (int i = 0; i < 4; i++)if (!ClearHeap(&bmpAxeHeroR[i]))LogError(L"Error releasing HeroAxeR !");
    for (int i = 0; i < 4; i++)if (!ClearHeap(&bmpClubHeroL[i]))LogError(L"Error releasing HeroClubL !");
    for (int i = 0; i < 4; i++)if (!ClearHeap(&bmpClubHeroR[i]))LogError(L"Error releasing HeroClubR !");
    for (int i = 0; i < 4; i++)if (!ClearHeap(&bmpSwordHeroL[i]))LogError(L"Error releasing HeroSwordL !");
    for (int i = 0; i < 4; i++)if (!ClearHeap(&bmpSwordHeroR[i]))LogError(L"Error releasing HeroSwordR !");

    for (int i = 0; i < 16; i++)if (!ClearHeap(&bmpEvil1L[i]))LogError(L"Error releasing Evil1L !");
    for (int i = 0; i < 16; i++)if (!ClearHeap(&bmpEvil1R[i]))LogError(L"Error releasing Evil1R !");

    for (int i = 0; i < 37; i++)if (!ClearHeap(&bmpEvil2[i]))LogError(L"Error releasing Evil2L !");
    
    for (int i = 0; i < 32; i++)if (!ClearHeap(&bmpEvil3L[i]))LogError(L"Error releasing Evil3L !");
    for (int i = 0; i < 32; i++)if (!ClearHeap(&bmpEvil3R[i]))LogError(L"Error releasing Evil3R !");

    for (int i = 0; i < 29; i++)if (!ClearHeap(&bmpEvil4[i]))LogError(L"Error releasing Evil4L !");
    
    for (int i = 0; i < 36; i++)if (!ClearHeap(&bmpEvil5[i]))LogError(L"Error releasing Evil5L !");
    
    for (int i = 0; i < 75; i++)if (!ClearHeap(&bmpField[i]))LogError(L"Error releasing Field !");
    for (int i = 0; i < 56; i++)if (!ClearHeap(&bmpIntro[i]))LogError(L"Error releasing Intro !");
}
void ErrExit(int what)
{
    MessageBeep(MB_ICONERROR);
    MessageBox(NULL, ErrHandle(what), L"КРИТИЧНА ГРЕШКА !", MB_OK | MB_APPLMODAL | MB_ICONERROR);

    std::remove(tmp_file);
    ReleaseResources();
    exit(1);
}

void GameOver()
{
    PlaySound(NULL, NULL, NULL);
    KillTimer(bHwnd, bTimer);


    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}
void InitGame()
{
    level = 1;
    score = 0;
    mins = 0;
    secs = 0;
    wcscpy_s(current_player, L"ONE_CAVEMAN");
    name_set = false;
    club_lifes = 100;
    axe_lifes = 100;
    sword_lifes = 100;
    cloak_lifes = 100;
    mail_lifes = 100;

    move_hero = false;

    ClearHeap(&Hero);
    Hero = dll::CreatureFactory(hero_flag, 80.0f, ground - 150.0f);
 
    if (!vObstacles.empty())
    for (int i = 0; i < vObstacles.size(); ++i)ClearHeap(&vObstacles[i]);
    vObstacles.clear();

    if (!vCrystals.empty())
        for (int i = 0; i < vCrystals.size(); ++i)ClearHeap(&vCrystals[i]);
    vCrystals.clear();
    
    for (float i = -200.0f; i <= map_width - 50.0f; i += 50.0f)
        vObstacles.push_back(dll::AssetFactory(stone_brick_flag, i, map_height - 50.0f));
    for (float i = -50.0f; i < map_height - 50.0f; i += 50.0f)
        vObstacles.push_back(dll::AssetFactory(stone_brick_flag, -200.0f, i));
    for (float i = -50.0f; i < map_height - 50.0f; i += 50.0f)
        vObstacles.push_back(dll::AssetFactory(stone_brick_flag, map_width - 50.0f, i));

    float current_crystal_x = 20.0f + static_cast<float>(RandGenerator(30, 50));
    float current_crystal_y = -40.0f;
    for (int i = 0; i < 9 + level; i++)
    {
        if (current_crystal_x <= map_width - 100.0f)
        {
            vCrystals.push_back(dll::AssetFactory(crystal_flag, current_crystal_x, current_crystal_y));
            current_crystal_x += static_cast<float>(RandGenerator(30, 50));
        }
        else
        {
            if (current_crystal_y <= ground - 60.0f)
            {
                vCrystals.push_back(dll::AssetFactory(crystal_flag, current_crystal_x, current_crystal_y));
                current_crystal_y += static_cast<float>(RandGenerator(50, 100));
            }
            else
            {
                vCrystals.push_back(dll::AssetFactory(crystal_flag, current_crystal_x, current_crystal_y));
                current_crystal_x -= static_cast<float>(RandGenerator(30, 50));
            }
        }
    }

    if (!vObstacles.empty() && !vCrystals.empty() && Hero)
    {
        float current_brick_x = 180.0f;
        float current_brick_y = 60.0f;

        for (int i = 0; i < 25; i++)
        {
            bool one_ok = false;
            dll::asset_ptr Dummy = nullptr;

            while (!one_ok)
            {
                one_ok = true;
                int atype = RandGenerator(0, 1);
                
                if (atype == 0)Dummy = dll::AssetFactory(red_brick_flag, current_brick_x, current_brick_y);
                else Dummy = dll::AssetFactory(stone_brick_flag, current_brick_x, current_brick_y);

                for (int i = 0; i < vObstacles.size(); ++i)
                {
                    if (!(Dummy->x > vObstacles[i]->ex || Dummy->ex<vObstacles[i]->x ||
                        Dummy->y>vObstacles[i]->ey || Dummy->ey < vObstacles[i]->y))
                    {
                        one_ok = false;
                        break;
                    }
                }
                for (int i = 0; i < vCrystals.size(); ++i)
                {
                    if (!(Dummy->x > vCrystals[i]->ex || Dummy->ex < vCrystals[i]->x ||
                        Dummy->y > vCrystals[i]->ey || Dummy->ey < vCrystals[i]->y))
                    {
                        one_ok = false;
                        break;
                    }
                }
                if (!(Dummy->x > Hero->ex || Dummy->ex < Hero->x ||
                    Dummy->y > Hero->ey || Dummy->ey < Hero->y))
                {
                    one_ok = false;
                    break;
                }
                
                current_brick_x += (float)(RandGenerator(0, 100));
                if (current_brick_x >= map_width - 100.0f)
                {
                    current_brick_x = 180.0f;
                    current_brick_y += (float)(RandGenerator(120, 150));
                }

                if (one_ok)vObstacles.push_back(Dummy);
                else Dummy->Release();            

            }
        }
    }

}

INT_PTR CALLBACK bDlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_INITDIALOG:
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)(Icon));
        return true;

    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;

        case IDOK:
            if (GetDlgItemTextW(hwnd, IDC_NAME, current_player, 16) < 1)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
                MessageBox(bHwnd, L"Наистина ли си забрави името ?", L"Забраватор !", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
                EndDialog(hwnd, IDCANCEL);
                break;
            }
            EndDialog(hwnd, IDOK);
            break;
        }
        break;
    }
    return (INT_PTR)(FALSE);
}
LRESULT CALLBACK bWinProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_CREATE:
        SetTimer(hwnd, bTimer, 1000, NULL);
        bBar = CreateMenu();
        bMain = CreateMenu();
        bStore = CreateMenu();
        AppendMenu(bBar, MF_POPUP, (UINT_PTR)bMain, L"Основно меню");
        AppendMenu(bBar, MF_POPUP, (UINT_PTR)bMain, L"Меню за данни");
        
        AppendMenu(bMain, MF_STRING, mNew, L"Нова игра");
        AppendMenu(bMain, MF_STRING, mLvl, L"Следващо ниво");
        AppendMenu(bMain, MF_SEPARATOR, NULL, NULL);
        AppendMenu(bMain, MF_STRING, mExit, L"Изход");
        
        AppendMenu(bStore, MF_STRING, mSave, L"Запази игра");
        AppendMenu(bStore, MF_STRING, mLoad, L"Зареди игра");
        AppendMenu(bStore, MF_SEPARATOR, NULL, NULL);
        AppendMenu(bStore, MF_STRING, mHoF, L"Зала на славата");
        SetMenu(hwnd, bBar);
        
        InitGame();
        break;

    case WM_CLOSE:
        pause = true;
        if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
        if (MessageBox(hwnd, L"Ако излезеш, ще загубиш тази игра !\n\nНаистина ли излизаш ?",
            L"Изход ?", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
        {
            pause = false;
            break;
        }
        GameOver();
        break;

    case WM_PAINT:
        PaintDC = BeginPaint(hwnd, &bPaint);
        FillRect(PaintDC, &bPaint.rcPaint, CreateSolidBrush(RGB(100, 100, 100)));
        EndPaint(hwnd, &bPaint);
        break;

    case WM_TIMER:
        if (pause) break;
        ++secs;
        mins = secs / 60;
        break;

    case WM_SETCURSOR:
        GetCursorPos(&cur_pos);
        ScreenToClient(hwnd, &cur_pos);
        if (LOWORD(lParam) == HTCLIENT)
        {
            if (!in_client)
            {
                pause = false;
                in_client = true;
            }

            if (cur_pos.y <= 50)
            {
                if (cur_pos.x >= b1Rect.left && cur_pos.x <= b1Rect.right)
                {
                    if (!b1Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = true;
                        b2Hglt = false;
                        b3Hglt = false;
                    }
                }
                if (cur_pos.x >= b2Rect.left && cur_pos.x <= b2Rect.right)
                {
                    if (!b2Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = true;
                        b3Hglt = false;
                    }
                }
                if (cur_pos.x >= b3Rect.left && cur_pos.x <= b3Rect.right)
                {
                    if (!b3Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = false;
                        b3Hglt = true;
                    }
                }

                SetCursor(outCursor);
                return true;
            }

            if (b1Hglt || b2Hglt || b3Hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b1Hglt = false;
                b2Hglt = false;
                b3Hglt = false;
            }
            SetCursor(mainCursor);
            return true;
        }
        else
        {
            if (in_client)
            {
                pause = true;
                in_client = false;
            }

            if (b1Hglt || b2Hglt || b3Hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b1Hglt = false;
                b2Hglt = false;
                b3Hglt = false;
            }
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return true;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case mNew:
            pause = true;
            if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"Ако рестартираш, ще загубиш тази игра !\n\nНаистина ли рестартираш ?",
                L"Рестарт ?", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
            {
                pause = false;
                break;
            }
            InitGame();
            break;


        case mExit:
            SendMessage(hwnd, WM_CLOSE, NULL, NULL);
            break;

        }
        break;


    case WM_LBUTTONDOWN:
        if (HIWORD(lParam) > 50)
        {
            if (Hero && !vObstacles.empty())
            {
                dll::PROT_CONTAINER ObstacleChecker(vObstacles.size());

                for (int i = 0; i < ObstacleChecker.size(); i++)
                {
                    dll::PROTON anObstacle(vObstacles[i]->x, vObstacles[i]->y,
                        50.0f, 50.0f);
                    ObstacleChecker.push_back(anObstacle);
                }

                Hero->Move((float)(level), ObstacleChecker, true, LOWORD(lParam), HIWORD(lParam));
                move_hero = true;
            }
        }
        break;

    case WM_KEYDOWN:

        switch (LOWORD(wParam))
        {
        case VK_LEFT:
        
            break;

        case VK_RIGHT:
            break;

        case VK_UP:
            break;

        case VK_DOWN:
            break;
        }

        break;

    default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
    }

    return (LRESULT)(FALSE);
}

void CreateResources()
{
    int result = 0;
    CheckFile(Ltmp_file, &result);
    if (result == FILE_EXIST)ErrExit(eStarted);
    else
    {
        std::wofstream start(Ltmp_file);
        start << L"Game started at: " << std::chrono::system_clock::now();
        start.close();
    }

    int win_x = static_cast<int>(GetSystemMetrics(SM_CXSCREEN) / 2 - (int)(scr_width / 2));
    if (GetSystemMetrics(SM_CXSCREEN) < win_x + (int)(scr_width / 2) 
        || GetSystemMetrics(SM_CYSCREEN) < scr_height + 10)ErrExit(eScreen);

    Icon = (HICON)(LoadImage(NULL, L".\\res\\main.ico", IMAGE_ICON, 255, 255, LR_LOADFROMFILE));
    if (!Icon)ErrExit(eIcon);
    mainCursor = LoadCursorFromFileW(L".\\res\\main.ani");
    outCursor = LoadCursorFromFileW(L".\\res\\out.ani");
    if (!mainCursor || !outCursor)ErrExit(eCursor);

    bWinClass.lpszClassName = bWinClassName;
    bWinClass.hInstance = bIns;
    bWinClass.lpfnWndProc = &bWinProc;
    bWinClass.hbrBackground = CreateSolidBrush(RGB(100, 100, 100));
    bWinClass.hCursor = mainCursor;
    bWinClass.hIcon = Icon;
    bWinClass.style = CS_DROPSHADOW;

    if (!RegisterClass(&bWinClass))ErrExit(eClass);

    bHwnd = CreateWindow(bWinClassName, L"РАЗХОДКА В СТРАШНИТЕ ПОДЗЕМИЯ", WS_CAPTION | WS_SYSMENU, win_x, 10,
        (int)(scr_width), (int)(scr_height), NULL, NULL, bIns, NULL);
    if (!bHwnd)ErrExit(eWindow);
    else
    {
        ShowWindow(bHwnd, SW_SHOWDEFAULT);

        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
        if (hr != S_OK)
        {
            LogError(L"Error creating iFactory !");
            ErrExit(eD2D);
        }
        if (iFactory)
        {
            hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(bHwnd,
                D2D1::SizeU((UINT32)(scr_width), (UINT32)(scr_height))), &Draw);
            if (hr != S_OK)
            {
                LogError(L"Error creating hwnd Render Target !");
                ErrExit(eD2D);
            }

            if (Draw)
            {
                D2D1_GRADIENT_STOP gStops[2]{};
                ID2D1GradientStopCollection* gsColl = nullptr;

                gStops[0].position = 0;
                gStops[0].color = D2D1::ColorF(D2D1::ColorF::Orange);
                gStops[1].position = 1.0f;
                gStops[1].color = D2D1::ColorF(D2D1::ColorF::Brown);

                hr = Draw->CreateGradientStopCollection(gStops, 2, &gsColl);
                if (hr != S_OK)
                {
                    LogError(L"Error creating GradientStopCollection !");
                    ErrExit(eD2D);
                }
                if (gsColl)
                {
                    hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(scr_width / 2, 25.0f),
                        D2D1::Point2F(0, 0), scr_width / 2, 25.0f), gsColl, &bckgBrush);
                    if (hr != S_OK)
                    {
                        LogError(L"Error creating bckgBrush !");
                        ErrExit(eD2D);
                    }
                    ClearHeap(&gsColl);
                }

                hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), &txtBrush);
                hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &hgltBrush);
                hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGray), &inactBrush);

                if (hr != S_OK)
                {
                    LogError(L"Error creating Solid Color Brushes !");
                    ErrExit(eD2D);
                }

                bmpAxe = Load(L".\\res\\img\\assets\\axe.png", Draw);
                if (!bmpAxe)
                {
                    LogError(L"Error loading bmpAxe !");
                    ErrExit(eD2D);
                }

                bmpPotion = Load(L".\\res\\img\\assets\\potion.png", Draw);
                if (!bmpPotion)
                {
                    LogError(L"Error loading bmpPotion !");
                    ErrExit(eD2D);
                }

                bmpBrick1 = Load(L".\\res\\img\\assets\\brick.png", Draw);
                if (!bmpBrick1)
                {
                    LogError(L"Error loading bmpBrick1 !");
                    ErrExit(eD2D);
                }

                bmpBrick2 = Load(L".\\res\\img\\assets\\brick2.png", Draw);
                if (!bmpBrick2)
                {
                    LogError(L"Error loading bmpBrick2 !");
                    ErrExit(eD2D);
                }

                bmpCloak = Load(L".\\res\\img\\assets\\cloak.png", Draw);
                if (!bmpCloak)
                {
                    LogError(L"Error loading bmpCloak !");
                    ErrExit(eD2D);
                }

                bmpClub = Load(L".\\res\\img\\assets\\club.png", Draw);
                if (!bmpClub)
                {
                    LogError(L"Error loading bmpClub !");
                    ErrExit(eD2D);
                }

                bmpCrystal = Load(L".\\res\\img\\assets\\crystal.png", Draw);
                if (!bmpCrystal)
                {
                    LogError(L"Error loading bmpCrystla !");
                    ErrExit(eD2D);
                }

                bmpGold = Load(L".\\res\\img\\assets\\gold.png", Draw);
                if (!bmpGold)
                {
                    LogError(L"Error loading bmpGold !");
                    ErrExit(eD2D);
                }

                bmpMail = Load(L".\\res\\img\\assets\\mail.png", Draw);
                if (!bmpMail)
                {
                    LogError(L"Error loading bmpMail !");
                    ErrExit(eD2D);
                }

                bmpSword = Load(L".\\res\\img\\assets\\sword.png", Draw);
                if (!bmpSword)
                {
                    LogError(L"Error loading bmpSword !");
                    ErrExit(eD2D);
                }

                bmpRIP = Load(L".\\res\\img\\assets\\rip.png", Draw);
                if (!bmpRIP)
                {
                    LogError(L"Error loading bmpRIP !");
                    ErrExit(eD2D);
                }

                for (int i = 0; i < 4; i++)
                {
                    wchar_t name[75] = L".\\res\\img\\hero\\l\\";
                    wchar_t add[3] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpHeroL[i] = Load(name, Draw);

                    if (!bmpHeroL[i])
                    {
                        LogError(L"Error loading bmpHeroL !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 4; i++)
                {
                    wchar_t name[75] = L".\\res\\img\\hero\\r\\";
                    wchar_t add[3] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpHeroR[i] = Load(name, Draw);

                    if (!bmpHeroR[i])
                    {
                        LogError(L"Error loading bmpHeroR !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 4; i++)
                {
                    wchar_t name[75] = L".\\res\\img\\hero_axe\\l\\";
                    wchar_t add[3] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpAxeHeroL[i] = Load(name, Draw);

                    if (!bmpAxeHeroL[i])
                    {
                        LogError(L"Error loading bmpAxeHeroL !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 4; i++)
                {
                    wchar_t name[75] = L".\\res\\img\\hero_axe\\r\\";
                    wchar_t add[3] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpAxeHeroR[i] = Load(name, Draw);

                    if (!bmpAxeHeroR[i])
                    {
                        LogError(L"Error loading bmpAxeHeroR !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 4; i++)
                {
                    wchar_t name[75] = L".\\res\\img\\hero_club\\l\\";
                    wchar_t add[3] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpClubHeroL[i] = Load(name, Draw);

                    if (!bmpClubHeroL[i])
                    {
                        LogError(L"Error loading bmpClubHeroL !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 4; i++)
                {
                    wchar_t name[75] = L".\\res\\img\\hero_club\\r\\";
                    wchar_t add[3] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpClubHeroR[i] = Load(name, Draw);

                    if (!bmpClubHeroR[i])
                    {
                        LogError(L"Error loading bmpClubHeroR !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 4; i++)
                {
                    wchar_t name[75] = L".\\res\\img\\hero_sword\\l\\";
                    wchar_t add[3] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpSwordHeroL[i] = Load(name, Draw);

                    if (!bmpSwordHeroL[i])
                    {
                        LogError(L"Error loading bmpSwordHeroL !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 4; i++)
                {
                    wchar_t name[75] = L".\\res\\img\\hero_sword\\r\\";
                    wchar_t add[3] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpSwordHeroR[i] = Load(name, Draw);

                    if (!bmpSwordHeroR[i])
                    {
                        LogError(L"Error loading bmpSwordHeroR !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 16; i++)
                {
                    wchar_t name[75] = L".\\res\\img\\evil1\\l\\";
                    wchar_t add[3] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpEvil1L[i] = Load(name, Draw);

                    if (!bmpEvil1L[i])
                    {
                        LogError(L"Error loading bmpEvil1L !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 16; i++)
                {
                    wchar_t name[75] = L".\\res\\img\\evil1\\r\\";
                    wchar_t add[3] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpEvil1R[i] = Load(name, Draw);

                    if (!bmpEvil1R[i])
                    {
                        LogError(L"Error loading bmpEvil1R !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 37; i++)
                {
                    wchar_t name[75] = L".\\res\\img\\evil2\\";
                    wchar_t add[3] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpEvil2[i] = Load(name, Draw);

                    if (!bmpEvil2[i])
                    {
                        LogError(L"Error loading bmpEvil2 !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 32; i++)
                {
                    wchar_t name[75] = L".\\res\\img\\evil3\\l\\";
                    wchar_t add[3] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpEvil3L[i] = Load(name, Draw);

                    if (!bmpEvil3L[i])
                    {
                        LogError(L"Error loading bmpEvil3L !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 32; i++)
                {
                    wchar_t name[75] = L".\\res\\img\\evil3\\r\\";
                    wchar_t add[3] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpEvil3R[i] = Load(name, Draw);

                    if (!bmpEvil3R[i])
                    {
                        LogError(L"Error loading bmpEvil3R !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 29; i++)
                {
                    wchar_t name[75] = L".\\res\\img\\evil4\\";
                    wchar_t add[3] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpEvil4[i] = Load(name, Draw);

                    if (!bmpEvil4[i])
                    {
                        LogError(L"Error loading bmpEvil4 !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 36; i++)
                {
                    wchar_t name[75] = L".\\res\\img\\evil5\\";
                    wchar_t add[3] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpEvil5[i] = Load(name, Draw);

                    if (!bmpEvil5[i])
                    {
                        LogError(L"Error loading bmpEvil5 !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 75; i++)
                {
                    wchar_t name[75] = L".\\res\\img\\field\\";
                    wchar_t add[3] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpField[i] = Load(name, Draw);

                    if (!bmpField[i])
                    {
                        LogError(L"Error loading bmpField !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 56; i++)
                {
                    wchar_t name[75] = L".\\res\\img\\intro\\";
                    wchar_t add[3] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpIntro[i] = Load(name, Draw);

                    if (!bmpIntro[i])
                    {
                        LogError(L"Error loading bmpIntro !");
                        ErrExit(eD2D);
                    }
                }

            }
        }

        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), 
            reinterpret_cast<IUnknown**>(&iWriteFactory));
        if (hr != S_OK)
        {
            LogError(L"Error creating iWriteFactory !");
            ErrExit(eD2D);
        }

        if (iWriteFactory)
        {
            hr = iWriteFactory->CreateTextFormat(L"Segoe Print", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
                DWRITE_FONT_STRETCH_NORMAL, 18, L"", &nrmTextFormat);
            hr = iWriteFactory->CreateTextFormat(L"Segoe Print", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
                DWRITE_FONT_STRETCH_NORMAL, 32, L"", &midTextFormat);
            hr = iWriteFactory->CreateTextFormat(L"Segoe Print", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
                DWRITE_FONT_STRETCH_NORMAL, 72, L"", &bigTextFormat);

            if (hr != S_OK)
            {
                LogError(L"Error creating iWriteFactory Text Formats !");
                ErrExit(eD2D);
            }
        }
    }

    int intro_frame = 0;

    if (Draw && bigTextFormat && txtBrush)
    {
        for (int i = 0; i <= 90; ++i)
        {
            Draw->BeginDraw();
            Draw->DrawBitmap(bmpIntro[intro_frame], D2D1::RectF(0, 0, scr_width, scr_height));
            ++intro_frame;
            if (intro_frame > 55)intro_frame = 0;
            if (RandGenerator(0, 4) == 2)
            {
                Draw->DrawTextW(L"ГЕРОЙ В ПОДЗЕМИЯТА !\n\ndev. Daniel", 34, bigTextFormat, D2D1::RectF(10.0f, 40.0f, scr_width,
                    scr_height), txtBrush);
                mciSendString(L"play .\\res\\snd\\buzz.wav", NULL, NULL, NULL);
            }
            Draw->EndDraw();
            Sleep(50);
        }

        Draw->BeginDraw();
        Draw->DrawBitmap(bmpIntro[intro_frame], D2D1::RectF(0, 0, scr_width, scr_height));
        Draw->DrawTextW(L"ГЕРОЙ В ПОДЗЕМИЯТА !\n\ndev. Daniel", 34, bigTextFormat, D2D1::RectF(10.0f, 40.0f, scr_width,
            scr_height), txtBrush);
        mciSendString(L"play .\\res\\snd\\boom.wav", NULL, NULL, NULL);
        Draw->EndDraw();
        Sleep(2000);
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    bIns = hInstance;
    if (!bIns)
    {
        LogError(L"Cannot obtain Windows hInstance !");
        ErrExit(eClass);
    }

    CreateResources();

    while (bMsg.message != WM_QUIT)
    {
        if ((bRet = PeekMessage(&bMsg, bHwnd, NULL, NULL, PM_REMOVE)) != 0)
        {
            if (bRet == -1)ErrExit(eMsg);
            TranslateMessage(&bMsg);
            DispatchMessage(&bMsg);
        }

        if (pause)
        {
            if (show_help)continue;

            if (Draw && bigTextFormat && txtBrush)
            {
                Draw->BeginDraw();
                Draw->Clear(D2D1::ColorF(D2D1::ColorF::WhiteSmoke));
                Draw->DrawTextW(L"ПАУЗА", 6, bigTextFormat, D2D1::RectF(scr_width / 2 - 100.0f, scr_height / 2 - 80.0f, scr_width,
                    scr_height), txtBrush);
                Draw->EndDraw();
                continue;
            }

        }

        ////////////////////////////////////////////////////

        if (Hero && !vObstacles.empty())
        {
            dll::PROT_CONTAINER ObstacleChecker(vObstacles.size());

            for (int i = 0; i < ObstacleChecker.size(); i++)
            {
                dll::PROTON anObstacle(vObstacles[i]->x, vObstacles[i]->y,
                    50.0f, 50.0f);
                ObstacleChecker.push_back(anObstacle);
            }

            if (move_hero)
            {
                if (Hero->Move((float)(level), ObstacleChecker))move_hero = false;
            
                if (Hero->ex >= scr_width - 60.0f)
                {
                    if (ViewMapRect.right > scr_width)
                    {
                        ViewMapRect.left -= (float)(level);
                        ViewMapRect.right -= (float)(level);
                        if (!vObstacles.empty())
                            for (int i = 0; i < vObstacles.size(); ++i)
                            {
                                vObstacles[i]->x -= (float)(level);
                                vObstacles[i]->SetEdges();
                            }
                        if (!vCrystals.empty())
                            for (int i = 0; i < vCrystals.size(); ++i)
                            {
                                vCrystals[i]->x -= (float)(level);
                                vCrystals[i]->SetEdges();
                            }
                    }
                }
                if (Hero->x <= 60.0f)
                {
                    if (ViewMapRect.left < 0)
                    {
                        ViewMapRect.left += (float)(level);
                        ViewMapRect.right += (float)(level);
                        if (!vObstacles.empty())
                            for (int i = 0; i < vObstacles.size(); ++i)
                            {
                                vObstacles[i]->x += (float)(level);
                                vObstacles[i]->SetEdges();
                            }
                        if (!vCrystals.empty())
                            for (int i = 0; i < vCrystals.size(); ++i)
                            {
                                vCrystals[i]->x += (float)(level);
                                vCrystals[i]->SetEdges();
                            }
                    }
                }
            
                if (Hero->ey >= ground - 60.0f)
                {
                    if (ViewMapRect.bottom > scr_height)
                    {
                        ViewMapRect.bottom -= (float)(level);
                        ViewMapRect.top -= (float)(level);
                        if (!vObstacles.empty())
                            for (int i = 0; i < vObstacles.size(); ++i)
                            {
                                vObstacles[i]->y -= (float)(level);
                                vObstacles[i]->SetEdges();
                            }
                        if (!vCrystals.empty())
                            for (int i = 0; i < vCrystals.size(); ++i)
                            {
                                vCrystals[i]->y -= (float)(level);
                                vCrystals[i]->SetEdges();
                            }
                    }
                }
                if (Hero->y <= sky + 60.0f)
                {
                    if (ViewMapRect.top < 0)
                    {
                        ViewMapRect.top += (float)(level);
                        ViewMapRect.bottom += (float)(level);
                        if (!vObstacles.empty())
                            for (int i = 0; i < vObstacles.size(); ++i)
                            {
                                vObstacles[i]->y += (float)(level);
                                vObstacles[i]->SetEdges();
                            }
                        if (!vCrystals.empty())
                            for (int i = 0; i < vCrystals.size(); ++i)
                            {
                                vCrystals[i]->y += (float)(level);
                                vCrystals[i]->SetEdges();
                            }
                    }

                }
            }
        }

        if (Hero && !vCrystals.empty())
        {
            for (std::vector<dll::asset_ptr>::iterator crystal = vCrystals.begin(); crystal < vCrystals.end(); ++crystal)
            {
                if (!(Hero->x > (*crystal)->ex || Hero->ex<(*crystal)->x || Hero->y>(*crystal)->ey || Hero->ey < (*crystal)->y))
                {
                    score += 10 + level;
                    if (sound)mciSendString(L"play .\\res\\snd\\crystal.wav", NULL, NULL, NULL);
                    (*crystal)->Release();
                    vCrystals.erase(crystal);
                    break;
                }
            }
        }










        // DRAW THINGS ********************************

        Draw->BeginDraw();

        Draw->DrawBitmap(bmpField[field_frame], ViewMapRect);
        field_delay--;
        if (field_delay <= 0)
        {
            field_delay = 3;
            ++field_frame;
            if (field_frame > 55)field_frame = 0;
        }
        if (!vObstacles.empty())
        {
            for (int i = 0; i < vObstacles.size(); ++i)
            {
                if (vObstacles[i]->CheckFlag(stone_brick_flag))
                    Draw->DrawBitmap(bmpBrick2, D2D1::RectF(vObstacles[i]->x, vObstacles[i]->y,
                        vObstacles[i]->ex, vObstacles[i]->ey));
                if (vObstacles[i]->CheckFlag(red_brick_flag))
                    Draw->DrawBitmap(bmpBrick1, D2D1::RectF(vObstacles[i]->x, vObstacles[i]->y,
                        vObstacles[i]->ex, vObstacles[i]->ey));
            }
        }
        if (!vCrystals.empty())
        {
            for (int i = 0; i < vCrystals.size(); i++)
                Draw->DrawBitmap(bmpCrystal, D2D1::RectF(vCrystals[i]->x, vCrystals[i]->y, vCrystals[i]->ex, vCrystals[i]->ey));
        }



        ///////////////////////////////////////////////////
        if (bckgBrush)Draw->FillRectangle(D2D1::RectF(0, 0, scr_width, 50.0f), bckgBrush);
        if (txtBrush && hgltBrush && inactBrush && nrmTextFormat)
        {
            if (name_set) Draw->DrawTextW(L"ИМЕ НА БОЕЦ", 12, nrmTextFormat, b1Rect, inactBrush);
            else
            {
                if (b1Hglt) Draw->DrawTextW(L"ИМЕ НА БОЕЦ", 12, nrmTextFormat, b1Rect, hgltBrush);
                else Draw->DrawTextW(L"ИМЕ НА БОЕЦ", 12, nrmTextFormat, b1Rect, txtBrush);
            }
            if (b2Hglt) Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmTextFormat, b2Rect, hgltBrush);
            else Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmTextFormat, b2Rect, txtBrush);
            if (b3Hglt) Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmTextFormat, b3Rect, hgltBrush);
            else Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmTextFormat, b3Rect, txtBrush);
        }
        ///////////////////////////////////////////////////


        if (Hero)
        {
            switch (Hero->dir)
            {
            case dirs::left:
                if (Hero->CheckType(hero_flag))
                {
                    int aframe = Hero->GetFrame();
                    Draw->DrawBitmap(bmpHeroL[aframe], Resizer(bmpHeroL[aframe], Hero->x, Hero->y));
                }
                else if (Hero->CheckType(hero_club_flag))
                {
                    int aframe = Hero->GetFrame();
                    Draw->DrawBitmap(bmpClubHeroL[aframe], Resizer(bmpClubHeroL[aframe], Hero->x, Hero->y));
                }
                else if (Hero->CheckType(hero_axe_flag))
                {
                    int aframe = Hero->GetFrame();
                    Draw->DrawBitmap(bmpAxeHeroL[aframe], Resizer(bmpAxeHeroL[aframe], Hero->x, Hero->y));
                }
                else if (Hero->CheckType(hero_sword_flag))
                {
                    int aframe = Hero->GetFrame();
                    Draw->DrawBitmap(bmpSwordHeroL[aframe], Resizer(bmpSwordHeroL[aframe], Hero->x, Hero->y));
                }
                break;

            case dirs::right:
                if (Hero->CheckType(hero_flag))
                {
                    int aframe = Hero->GetFrame();
                    Draw->DrawBitmap(bmpHeroR[aframe], Resizer(bmpHeroR[aframe], Hero->x, Hero->y));
                }
                else if (Hero->CheckType(hero_club_flag))
                {
                    int aframe = Hero->GetFrame();
                    Draw->DrawBitmap(bmpClubHeroR[aframe], Resizer(bmpClubHeroR[aframe], Hero->x, Hero->y));
                }
                else if (Hero->CheckType(hero_axe_flag))
                {
                    int aframe = Hero->GetFrame();
                    Draw->DrawBitmap(bmpAxeHeroR[aframe], Resizer(bmpAxeHeroR[aframe], Hero->x, Hero->y));
                }
                else if (Hero->CheckType(hero_sword_flag))
                {
                    int aframe = Hero->GetFrame();
                    Draw->DrawBitmap(bmpSwordHeroR[aframe], Resizer(bmpSwordHeroR[aframe], Hero->x, Hero->y));
                }
                break;
            }
        }

        ////////////////////////////////////////////////
        Draw->EndDraw();

    }

    std::remove(tmp_file);
    
    ReleaseResources();

    return (int) bMsg.wParam;
}