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

int cloak_lifes = 0;
int mail_lifes = 0;

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

ID2D1SolidColorBrush* LifeBrush{ nullptr };
ID2D1SolidColorBrush* HurtBrush{ nullptr };
ID2D1SolidColorBrush* CritBrush{ nullptr };

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
bool hero_killed = false;
float RIP_x{};
float RIP_y{};

bool cloak_on = false;
bool mail_on = false;

std::vector<dll::asset_ptr> vObstacles;
std::vector<dll::asset_ptr> vCrystals;
std::vector<dll::asset_ptr> vAssets;
std::vector<dll::creature_ptr> vEvils;

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

    if (!ClearHeap(&LifeBrush))LogError(L"Error releasing txtBrush !");
    if (!ClearHeap(&HurtBrush))LogError(L"Error releasing txtBrush !");
    if (!ClearHeap(&CritBrush))LogError(L"Error releasing txtBrush !");

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
BOOL CheckRecord()
{
    if (score < 1)return no_record;

    int result{};
    CheckFile(record_file, &result);
    if (result == FILE_NOT_EXIST)
    {
        std::wofstream rec(record_file);
        rec << score << std::endl;
        for (int i = 0; i < 16; i++)rec << static_cast<int>(current_player[i]) << std::endl;
        rec.close();
        return first_record;
    }
    else
    {
        std::wifstream check(record_file);
        check >> result;
        check.close();
    }

    if (result < score)
    {
        std::wofstream rec(record_file);
        rec << score << std::endl;
        for (int i = 0; i < 16; i++)rec << static_cast<int>(current_player[i]) << std::endl;
        rec.close();
        return record;
    }
    return no_record;
}
void GameOver()
{
    Draw->EndDraw();
    PlaySound(NULL, NULL, NULL);
    KillTimer(bHwnd, bTimer);

    switch (CheckRecord())
    {
    case no_record:
        Draw->BeginDraw();
        Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkViolet));
        if (bigTextFormat && txtBrush)
            Draw->DrawTextW(L"ООО ! УБИХА ТЕ !", 17, bigTextFormat, D2D1::RectF(30.0f, 80.0f, scr_width, scr_height), txtBrush);
        Draw->EndDraw();
        if (sound) PlaySound(L".\\res\\snd\\loose.wav", NULL, SND_SYNC);
        else Sleep(3000);
        break;

    case first_record:
        Draw->BeginDraw();
        Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkViolet));
        if (bigTextFormat && txtBrush)
            Draw->DrawTextW(L"ПЪРВИ РЕКОРД НА ИГРАТА !", 25, bigTextFormat, D2D1::RectF(30.0f, 80.0f, scr_width, scr_height), 
                txtBrush);
        Draw->EndDraw();
        if (sound) PlaySound(L".\\res\\snd\\record.wav", NULL, SND_SYNC);
        else Sleep(3000);
        break;

    case record:
        Draw->BeginDraw();
        Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkViolet));
        if (bigTextFormat && txtBrush)
            Draw->DrawTextW(L"НОВ СВЕТОВЕН РЕКОРД !", 22, bigTextFormat, D2D1::RectF(30.0f, 80.0f, scr_width, scr_height),
                txtBrush);
        Draw->EndDraw();
        if (sound) PlaySound(L".\\res\\snd\\record.wav", NULL, SND_SYNC);
        else Sleep(3000);
        break;
    }

    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}
void HallOfFame()
{
    int result = 0;
    CheckFile(record_file, &result);
    if (result == FILE_NOT_EXIST)
    {
        if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
        MessageBox(bHwnd, L"Все още няма рекорди на играта !\n\nПостарай се повече !", L"Липсва файл !",
            MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
        return;
    }

    std::wifstream rec(record_file);
    wchar_t txt[100] = L"НАЙ-ДОБЪР ИГРАЧ: ";
    wchar_t saved_player[16] = L"\0";
    wchar_t add[5] = L"\0";

    rec >> result;
    wsprintf(add, L"%d", result);
    for (int i = 0; i < 16; i++)
    {
        int letter = 0;
        rec >> letter;
        saved_player[i] = static_cast<wchar_t>(letter);
    }
    rec.close();
    wcscat_s(txt, saved_player);
    wcscat_s(txt, L"\n\nСВЕТОВЕН РЕКОРД: ");
    wcscat_s(txt, add);
    result = 0;
    for (int i = 0; i < 100; i++)
        if (txt[i] != '\0')result++;
        else break;

    if (Draw && txtBrush && midTextFormat)
    {
        Draw->BeginDraw();
        Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkOrange));
        Draw->DrawTextW(txt, result, midTextFormat, D2D1::RectF(50.0f, 100.0f, scr_width, scr_height), txtBrush);
        Draw->EndDraw();
    }

    if (sound)mciSendString(L"play .\\res\\snd\\showrec.wav", NULL, NULL, NULL);
    Sleep(3500);
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

    ViewMapRect.left = -200.0f;
    ViewMapRect.top = -100.0f;
    ViewMapRect.right = map_width;
    ViewMapRect.bottom = map_height;

    cloak_lifes = 0;
    mail_lifes = 0;
    cloak_on = false;
    mail_on = false;

    move_hero = false;

    ClearHeap(&Hero);
    Hero = dll::CreatureFactory(hero_flag, 80.0f, ground - 150.0f);
 
    if (!vObstacles.empty())
    for (int i = 0; i < vObstacles.size(); ++i)ClearHeap(&vObstacles[i]);
    vObstacles.clear();

    if (!vCrystals.empty())
        for (int i = 0; i < vCrystals.size(); ++i)ClearHeap(&vCrystals[i]);
    vCrystals.clear();

    if (!vAssets.empty())
        for (int i = 0; i < vAssets.size(); ++i)ClearHeap(&vAssets[i]);
    vAssets.clear();

    if (!vEvils.empty())
        for (int i = 0; i < vEvils.size(); ++i)ClearHeap(&vEvils[i]);
    vEvils.clear();
    
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
        current_crystal_x += static_cast<float>(RandGenerator(80, 200));
        if (current_crystal_x <= map_width - 100.0f)
            vCrystals.push_back(dll::AssetFactory(crystal_flag, current_crystal_x, current_crystal_y));
        else
        {
            if (current_crystal_x > map_width - 100.0f) current_crystal_x = map_width - 100.0f;
            if (current_crystal_y <= ground - 60.0f)
            {
                vCrystals.push_back(dll::AssetFactory(crystal_flag, current_crystal_x, current_crystal_y));
                current_crystal_y += static_cast<float>(RandGenerator(50, 100));
            }
            else
            {
                vCrystals.push_back(dll::AssetFactory(crystal_flag, current_crystal_x, current_crystal_y));
                current_crystal_x -= static_cast<float>(RandGenerator(80, 150));
            }
        }
    }

    if (!vObstacles.empty() && !vCrystals.empty() && Hero)
    {
        float current_brick_x = 100.0f;
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
                
                int temp_current_x = RandGenerator(50, 100);
                if (temp_current_x % 50 == 0)temp_current_x = 50;
                else temp_current_x += 60;
                current_brick_x += temp_current_x;
                if (current_brick_x >= map_width - 100.0f)
                {
                    current_brick_x = 100.0f;
                    current_brick_y += (float)(RandGenerator(120, 150));
                }

                if (one_ok)vObstacles.push_back(Dummy);
                else Dummy->Release();            

            }
        }
    }

    if (!vObstacles.empty() && !vCrystals.empty() && Hero)
    {
        for (int i = 0; i < 10; ++i)
        {
            bool is_ok = false;
            int choice = RandGenerator(0, 10);

            if (choice == 0)
            {
                while (!is_ok)
                {
                    is_ok = true;

                    dll::asset_ptr Dummy = dll::AssetFactory(gold_flag, (float)(RandGenerator(-100, (int)(map_width - 100))),
                        (float)(RandGenerator(-10, 300)));

                    for (int i = 0; i < vObstacles.size(); ++i)
                    {
                        if (!(Dummy->x > vObstacles[i]->ex || Dummy->ex<vObstacles[i]->x
                            || Dummy->y>vObstacles[i]->ey || Dummy->ey < vObstacles[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    for (int i = 0; i < vCrystals.size(); ++i)
                    {
                        if (!(Dummy->x > vCrystals[i]->ex || Dummy->ex < vCrystals[i]->x ||
                            Dummy->y > vCrystals[i]->ey || Dummy->ey < vCrystals[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    if (!(Dummy->x > Hero->ex || Dummy->ex < Hero->x ||
                        Dummy->y > Hero->ey || Dummy->ey < Hero->y))
                    {
                        is_ok = false;
                        break;
                    }

                    if (is_ok)vAssets.push_back(Dummy);
                }
            }
            if (choice == 1)
            {
                bool is_ok = false;

                while (!is_ok)
                {
                    is_ok = true;

                    dll::asset_ptr Dummy = dll::AssetFactory(club_flag, (float)(RandGenerator(-100, (int)(map_width - 100))),
                        (float)(RandGenerator(-10, 300)));

                    for (int i = 0; i < vObstacles.size(); ++i)
                    {
                        if (!(Dummy->x > vObstacles[i]->ex || Dummy->ex<vObstacles[i]->x
                            || Dummy->y>vObstacles[i]->ey || Dummy->ey < vObstacles[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    for (int i = 0; i < vCrystals.size(); ++i)
                    {
                        if (!(Dummy->x > vCrystals[i]->ex || Dummy->ex < vCrystals[i]->x ||
                            Dummy->y > vCrystals[i]->ey || Dummy->ey < vCrystals[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    if (!(Dummy->x > Hero->ex || Dummy->ex < Hero->x ||
                        Dummy->y > Hero->ey || Dummy->ey < Hero->y))
                    {
                        is_ok = false;
                        break;
                    }

                    if (is_ok)vAssets.push_back(Dummy);
                }
            }
            if (choice == 2)
            {
                bool is_ok = false;

                while (!is_ok)
                {
                    is_ok = true;

                    dll::asset_ptr Dummy = dll::AssetFactory(axe_flag, (float)(RandGenerator(-100, (int)(map_width - 100))),
                        (float)(RandGenerator(-10, 300)));

                    for (int i = 0; i < vObstacles.size(); ++i)
                    {
                        if (!(Dummy->x > vObstacles[i]->ex || Dummy->ex<vObstacles[i]->x
                            || Dummy->y>vObstacles[i]->ey || Dummy->ey < vObstacles[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    for (int i = 0; i < vCrystals.size(); ++i)
                    {
                        if (!(Dummy->x > vCrystals[i]->ex || Dummy->ex < vCrystals[i]->x ||
                            Dummy->y > vCrystals[i]->ey || Dummy->ey < vCrystals[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    if (!(Dummy->x > Hero->ex || Dummy->ex < Hero->x ||
                        Dummy->y > Hero->ey || Dummy->ey < Hero->y))
                    {
                        is_ok = false;
                        break;
                    }

                    if (is_ok)vAssets.push_back(Dummy);
                }
            }
            if (choice == 3)
            {
                bool is_ok = false;

                while (!is_ok)
                {
                    is_ok = true;

                    dll::asset_ptr Dummy = dll::AssetFactory(sword_flag, (float)(RandGenerator(-100, (int)(map_width - 100))),
                        (float)(RandGenerator(-10, 300)));

                    for (int i = 0; i < vObstacles.size(); ++i)
                    {
                        if (!(Dummy->x > vObstacles[i]->ex || Dummy->ex<vObstacles[i]->x
                            || Dummy->y>vObstacles[i]->ey || Dummy->ey < vObstacles[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    for (int i = 0; i < vCrystals.size(); ++i)
                    {
                        if (!(Dummy->x > vCrystals[i]->ex || Dummy->ex < vCrystals[i]->x ||
                            Dummy->y > vCrystals[i]->ey || Dummy->ey < vCrystals[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    if (!(Dummy->x > Hero->ex || Dummy->ex < Hero->x ||
                        Dummy->y > Hero->ey || Dummy->ey < Hero->y))
                    {
                        is_ok = false;
                        break;
                    }

                    if (is_ok)vAssets.push_back(Dummy);
                }
            }
            if (choice == 4)
            {
                bool is_ok = false;

                while (!is_ok)
                {
                    is_ok = true;

                    dll::asset_ptr Dummy = dll::AssetFactory(cloak_flag, (float)(RandGenerator(-100, (int)(map_width - 100))),
                        (float)(RandGenerator(-10, 300)));

                    for (int i = 0; i < vObstacles.size(); ++i)
                    {
                        if (!(Dummy->x > vObstacles[i]->ex || Dummy->ex<vObstacles[i]->x
                            || Dummy->y>vObstacles[i]->ey || Dummy->ey < vObstacles[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    for (int i = 0; i < vCrystals.size(); ++i)
                    {
                        if (!(Dummy->x > vCrystals[i]->ex || Dummy->ex < vCrystals[i]->x ||
                            Dummy->y > vCrystals[i]->ey || Dummy->ey < vCrystals[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    if (!(Dummy->x > Hero->ex || Dummy->ex < Hero->x ||
                        Dummy->y > Hero->ey || Dummy->ey < Hero->y))
                    {
                        is_ok = false;
                        break;
                    }

                    if (is_ok)vAssets.push_back(Dummy);
                }
            }
            if (choice == 5)
            {
                bool is_ok = false;

                while (!is_ok)
                {
                    is_ok = true;

                    dll::asset_ptr Dummy = dll::AssetFactory(mail_flag, (float)(RandGenerator(-100, (int)(map_width - 100))),
                        (float)(RandGenerator(-10, 300)));

                    for (int i = 0; i < vObstacles.size(); ++i)
                    {
                        if (!(Dummy->x > vObstacles[i]->ex || Dummy->ex<vObstacles[i]->x
                            || Dummy->y>vObstacles[i]->ey || Dummy->ey < vObstacles[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    for (int i = 0; i < vCrystals.size(); ++i)
                    {
                        if (!(Dummy->x > vCrystals[i]->ex || Dummy->ex < vCrystals[i]->x ||
                            Dummy->y > vCrystals[i]->ey || Dummy->ey < vCrystals[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    if (!(Dummy->x > Hero->ex || Dummy->ex < Hero->x ||
                        Dummy->y > Hero->ey || Dummy->ey < Hero->y))
                    {
                        is_ok = false;
                        break;
                    }

                    if (is_ok)vAssets.push_back(Dummy);

                }
            }
            if (choice == 6)
            {
                bool is_ok = false;

                while (!is_ok)
                {
                    is_ok = true;

                    dll::asset_ptr Dummy = dll::AssetFactory(potion_flag, (float)(RandGenerator(-100, (int)(map_width - 100))),
                        (float)(RandGenerator(-10, 300)));

                    for (int i = 0; i < vObstacles.size(); ++i)
                    {
                        if (!(Dummy->x > vObstacles[i]->ex || Dummy->ex<vObstacles[i]->x
                            || Dummy->y>vObstacles[i]->ey || Dummy->ey < vObstacles[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    for (int i = 0; i < vCrystals.size(); ++i)
                    {
                        if (!(Dummy->x > vCrystals[i]->ex || Dummy->ex < vCrystals[i]->x ||
                            Dummy->y > vCrystals[i]->ey || Dummy->ey < vCrystals[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    if (!(Dummy->x > Hero->ex || Dummy->ex < Hero->x ||
                        Dummy->y > Hero->ey || Dummy->ey < Hero->y))
                    {
                        is_ok = false;
                        break;
                    }

                    if (is_ok)vAssets.push_back(Dummy);
                }
            }
        }
    }

    if (!vObstacles.empty() && Hero)
    {
        for (int i = 0; i <= 5 + level; i++)
        {
            int atype = RandGenerator(0, 4);
            bool is_ok = false;

            while (!is_ok)
            {
                is_ok = true;

                dll::creature_ptr Dummy = nullptr;

                switch (atype)
                {
                case 0:
                    Dummy = dll::CreatureFactory(evil1_flag, (float)(RandGenerator(250, (int)(map_width)-80)),
                        (float)(RandGenerator(-50, (int)(ground)-80)));
                    break;

                case 1:
                    Dummy = dll::CreatureFactory(evil2_flag, (float)(RandGenerator(250, (int)(map_width)-80)),
                        (float)(RandGenerator(-50, (int)(ground)-80)));
                    break;

                case 2:
                    Dummy = dll::CreatureFactory(evil3_flag, (float)(RandGenerator(250, (int)(map_width)-80)),
                        (float)(RandGenerator(-50, (int)(ground)-80)));
                    break;

                case 3:
                    Dummy = dll::CreatureFactory(evil4_flag, (float)(RandGenerator(250, (int)(map_width)-80)),
                        (float)(RandGenerator(-50, (int)(ground)-80)));
                    break;

                case 4:
                    Dummy = dll::CreatureFactory(evil5_flag, (float)(RandGenerator(250, (int)(map_width)-80)),
                        (float)(RandGenerator(-50, (int)(ground)-80)));
                    break;
                }

                if (Dummy)
                {
                    if (!(Hero->x >= Dummy->ex || Hero->ex <= Dummy->x || Hero->y >= Dummy->ey || Hero->ey <= Dummy->y))
                    {
                        Dummy->Release();
                        is_ok = false;
                        break;
                    }

                    for (int i = 0; i < vObstacles.size(); i++)
                    {
                        if (!(Dummy->x >= vObstacles[i]->ex || Dummy->ex <= vObstacles[i]->x ||
                            Dummy->y >= vObstacles[i]->ey || Dummy->ey <= vObstacles[i]->y))
                        {
                            Dummy->Release();
                            is_ok = false;
                            break;
                        }
                    }

                    if (is_ok)
                    {
                        vEvils.push_back(Dummy);
                        dll::PROT_CONTAINER Obstacles(vObstacles.size());
                        if(Obstacles.is_valid())
                            for (int i = 0; i < vObstacles.size(); ++i)
                            {
                                dll::PROTON an_obstacle{ vObstacles[i]->x,vObstacles[i]->y,vObstacles[i]->GetWidth(),
                                vObstacles[i]->GetHeight() };
                                Obstacles.push_back(an_obstacle);
                            }
                        switch (RandGenerator(0, 1))
                        {
                        case 0:
                            vEvils.back()->Move((float)(level), Obstacles, true, 0, vEvils.back()->y);
                            break;

                        case 1:
                            vEvils.back()->Move((float)(level), Obstacles, true, map_width, vEvils.back()->y);
                            break;
                        }
                    }
                }
            }
        }
    }
}
void LevelUp()
{
    if (mins < 2)score += 50;
    else if (mins < 5)score += 20;

    if (Draw && bigTextFormat && txtBrush)
    {
        Draw->BeginDraw();
        Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkViolet));
        Draw->DrawTextW(L"НИВОТО ИЗЧИСТЕНО !", 19, bigTextFormat, D2D1::RectF(30.0f, 80.0f, scr_width, scr_height), txtBrush);
        if (mins < 2)
            Draw->DrawTextW(L"\n\nПОЛУЧИ БОНУС ЗА ВРЕМЕ !", 26, bigTextFormat, 
                D2D1::RectF(50.0f, 200.0f, scr_width, scr_height), txtBrush);
        Draw->EndDraw();
        if (sound)
        {
            PlaySound(NULL, NULL, NULL);
            PlaySound(L".\\res\\snd\\levelup.wav", NULL, SND_SYNC);
            PlaySound(sound_file, NULL, SND_SYNC | SND_LOOP);
        }
        else Sleep(3000);
    }

    ++level;
    
    mins = 0;
    secs = 0;

    move_hero = false;

    ViewMapRect.left = -200.0f;
    ViewMapRect.top = -100.0f;
    ViewMapRect.right = map_width;
    ViewMapRect.bottom = map_height;

    ClearHeap(&Hero);
    Hero = dll::CreatureFactory(hero_flag, 80.0f, ground - 150.0f);

    if (!vObstacles.empty())
        for (int i = 0; i < vObstacles.size(); ++i)ClearHeap(&vObstacles[i]);
    vObstacles.clear();

    if (!vCrystals.empty())
        for (int i = 0; i < vCrystals.size(); ++i)ClearHeap(&vCrystals[i]);
    vCrystals.clear();

    if (!vAssets.empty())
        for (int i = 0; i < vAssets.size(); ++i)ClearHeap(&vAssets[i]);
    vAssets.clear();

    if (!vEvils.empty())
        for (int i = 0; i < vEvils.size(); ++i)ClearHeap(&vEvils[i]);
    vEvils.clear();

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
        current_crystal_x += static_cast<float>(RandGenerator(80, 200));
        if (current_crystal_x <= map_width - 100.0f)
            vCrystals.push_back(dll::AssetFactory(crystal_flag, current_crystal_x, current_crystal_y));
        else
        {
            if (current_crystal_x > map_width - 100.0f) current_crystal_x = map_width - 100.0f;
            if (current_crystal_y <= ground - 60.0f)
            {
                vCrystals.push_back(dll::AssetFactory(crystal_flag, current_crystal_x, current_crystal_y));
                current_crystal_y += static_cast<float>(RandGenerator(50, 100));
            }
            else
            {
                vCrystals.push_back(dll::AssetFactory(crystal_flag, current_crystal_x, current_crystal_y));
                current_crystal_x -= static_cast<float>(RandGenerator(80, 150));
            }
        }
    }

    if (!vObstacles.empty() && !vCrystals.empty() && Hero)
    {
        float current_brick_x = 100.0f;
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

                int temp_current_x = RandGenerator(50, 100);
                if (temp_current_x % 50 == 0)temp_current_x = 50;
                else temp_current_x += 60;
                current_brick_x += temp_current_x;
                if (current_brick_x >= map_width - 100.0f)
                {
                    current_brick_x = 100.0f;
                    current_brick_y += (float)(RandGenerator(120, 150));
                }

                if (one_ok)vObstacles.push_back(Dummy);
                else Dummy->Release();

            }
        }
    }

    if (!vObstacles.empty() && !vCrystals.empty() && Hero)
    {
        for (int i = 0; i < 10; ++i)
        {
            bool is_ok = false;
            int choice = RandGenerator(0, 10);

            if (choice == 0)
            {
                while (!is_ok)
                {
                    is_ok = true;

                    dll::asset_ptr Dummy = dll::AssetFactory(gold_flag, (float)(RandGenerator(-100, (int)(map_width - 100))),
                        (float)(RandGenerator(-10, 300)));

                    for (int i = 0; i < vObstacles.size(); ++i)
                    {
                        if (!(Dummy->x > vObstacles[i]->ex || Dummy->ex<vObstacles[i]->x
                            || Dummy->y>vObstacles[i]->ey || Dummy->ey < vObstacles[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    for (int i = 0; i < vCrystals.size(); ++i)
                    {
                        if (!(Dummy->x > vCrystals[i]->ex || Dummy->ex < vCrystals[i]->x ||
                            Dummy->y > vCrystals[i]->ey || Dummy->ey < vCrystals[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    if (!(Dummy->x > Hero->ex || Dummy->ex < Hero->x ||
                        Dummy->y > Hero->ey || Dummy->ey < Hero->y))
                    {
                        is_ok = false;
                        break;
                    }

                    if (is_ok)vAssets.push_back(Dummy);
                }
            }
            if (choice == 1)
            {
                bool is_ok = false;

                while (!is_ok)
                {
                    is_ok = true;

                    dll::asset_ptr Dummy = dll::AssetFactory(club_flag, (float)(RandGenerator(-100, (int)(map_width - 100))),
                        (float)(RandGenerator(-10, 300)));

                    for (int i = 0; i < vObstacles.size(); ++i)
                    {
                        if (!(Dummy->x > vObstacles[i]->ex || Dummy->ex<vObstacles[i]->x
                            || Dummy->y>vObstacles[i]->ey || Dummy->ey < vObstacles[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    for (int i = 0; i < vCrystals.size(); ++i)
                    {
                        if (!(Dummy->x > vCrystals[i]->ex || Dummy->ex < vCrystals[i]->x ||
                            Dummy->y > vCrystals[i]->ey || Dummy->ey < vCrystals[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    if (!(Dummy->x > Hero->ex || Dummy->ex < Hero->x ||
                        Dummy->y > Hero->ey || Dummy->ey < Hero->y))
                    {
                        is_ok = false;
                        break;
                    }

                    if (is_ok)vAssets.push_back(Dummy);
                }
            }
            if (choice == 2)
            {
                bool is_ok = false;

                while (!is_ok)
                {
                    is_ok = true;

                    dll::asset_ptr Dummy = dll::AssetFactory(axe_flag, (float)(RandGenerator(-100, (int)(map_width - 100))),
                        (float)(RandGenerator(-10, 300)));

                    for (int i = 0; i < vObstacles.size(); ++i)
                    {
                        if (!(Dummy->x > vObstacles[i]->ex || Dummy->ex<vObstacles[i]->x
                            || Dummy->y>vObstacles[i]->ey || Dummy->ey < vObstacles[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    for (int i = 0; i < vCrystals.size(); ++i)
                    {
                        if (!(Dummy->x > vCrystals[i]->ex || Dummy->ex < vCrystals[i]->x ||
                            Dummy->y > vCrystals[i]->ey || Dummy->ey < vCrystals[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    if (!(Dummy->x > Hero->ex || Dummy->ex < Hero->x ||
                        Dummy->y > Hero->ey || Dummy->ey < Hero->y))
                    {
                        is_ok = false;
                        break;
                    }

                    if (is_ok)vAssets.push_back(Dummy);
                }
            }
            if (choice == 3)
            {
                bool is_ok = false;

                while (!is_ok)
                {
                    is_ok = true;

                    dll::asset_ptr Dummy = dll::AssetFactory(sword_flag, (float)(RandGenerator(-100, (int)(map_width - 100))),
                        (float)(RandGenerator(-10, 300)));

                    for (int i = 0; i < vObstacles.size(); ++i)
                    {
                        if (!(Dummy->x > vObstacles[i]->ex || Dummy->ex<vObstacles[i]->x
                            || Dummy->y>vObstacles[i]->ey || Dummy->ey < vObstacles[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    for (int i = 0; i < vCrystals.size(); ++i)
                    {
                        if (!(Dummy->x > vCrystals[i]->ex || Dummy->ex < vCrystals[i]->x ||
                            Dummy->y > vCrystals[i]->ey || Dummy->ey < vCrystals[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    if (!(Dummy->x > Hero->ex || Dummy->ex < Hero->x ||
                        Dummy->y > Hero->ey || Dummy->ey < Hero->y))
                    {
                        is_ok = false;
                        break;
                    }

                    if (is_ok)vAssets.push_back(Dummy);
                }
            }
            if (choice == 4)
            {
                bool is_ok = false;

                while (!is_ok)
                {
                    is_ok = true;

                    dll::asset_ptr Dummy = dll::AssetFactory(cloak_flag, (float)(RandGenerator(-100, (int)(map_width - 100))),
                        (float)(RandGenerator(-10, 300)));

                    for (int i = 0; i < vObstacles.size(); ++i)
                    {
                        if (!(Dummy->x > vObstacles[i]->ex || Dummy->ex<vObstacles[i]->x
                            || Dummy->y>vObstacles[i]->ey || Dummy->ey < vObstacles[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    for (int i = 0; i < vCrystals.size(); ++i)
                    {
                        if (!(Dummy->x > vCrystals[i]->ex || Dummy->ex < vCrystals[i]->x ||
                            Dummy->y > vCrystals[i]->ey || Dummy->ey < vCrystals[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    if (!(Dummy->x > Hero->ex || Dummy->ex < Hero->x ||
                        Dummy->y > Hero->ey || Dummy->ey < Hero->y))
                    {
                        is_ok = false;
                        break;
                    }

                    if (is_ok)vAssets.push_back(Dummy);
                }
            }
            if (choice == 5)
            {
                bool is_ok = false;

                while (!is_ok)
                {
                    is_ok = true;

                    dll::asset_ptr Dummy = dll::AssetFactory(mail_flag, (float)(RandGenerator(-100, (int)(map_width - 100))),
                        (float)(RandGenerator(-10, 300)));

                    for (int i = 0; i < vObstacles.size(); ++i)
                    {
                        if (!(Dummy->x > vObstacles[i]->ex || Dummy->ex<vObstacles[i]->x
                            || Dummy->y>vObstacles[i]->ey || Dummy->ey < vObstacles[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    for (int i = 0; i < vCrystals.size(); ++i)
                    {
                        if (!(Dummy->x > vCrystals[i]->ex || Dummy->ex < vCrystals[i]->x ||
                            Dummy->y > vCrystals[i]->ey || Dummy->ey < vCrystals[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    if (!(Dummy->x > Hero->ex || Dummy->ex < Hero->x ||
                        Dummy->y > Hero->ey || Dummy->ey < Hero->y))
                    {
                        is_ok = false;
                        break;
                    }

                    if (is_ok)vAssets.push_back(Dummy);

                }
            }
            if (choice == 6)
            {
                bool is_ok = false;

                while (!is_ok)
                {
                    is_ok = true;

                    dll::asset_ptr Dummy = dll::AssetFactory(potion_flag, (float)(RandGenerator(-100, (int)(map_width - 100))),
                        (float)(RandGenerator(-10, 300)));

                    for (int i = 0; i < vObstacles.size(); ++i)
                    {
                        if (!(Dummy->x > vObstacles[i]->ex || Dummy->ex<vObstacles[i]->x
                            || Dummy->y>vObstacles[i]->ey || Dummy->ey < vObstacles[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    for (int i = 0; i < vCrystals.size(); ++i)
                    {
                        if (!(Dummy->x > vCrystals[i]->ex || Dummy->ex < vCrystals[i]->x ||
                            Dummy->y > vCrystals[i]->ey || Dummy->ey < vCrystals[i]->y))
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    if (!(Dummy->x > Hero->ex || Dummy->ex < Hero->x ||
                        Dummy->y > Hero->ey || Dummy->ey < Hero->y))
                    {
                        is_ok = false;
                        break;
                    }

                    if (is_ok)vAssets.push_back(Dummy);
                }
            }
        }
    }

    if (!vObstacles.empty() && Hero)
    {
        for (int i = 0; i <= 5 + level; i++)
        {
            int atype = RandGenerator(0, 4);
            bool is_ok = false;

            while (!is_ok)
            {
                is_ok = true;

                dll::creature_ptr Dummy = nullptr;

                switch (atype)
                {
                case 0:
                    Dummy = dll::CreatureFactory(evil1_flag, (float)(RandGenerator(250, (int)(map_width)-80)),
                        (float)(RandGenerator(-50, (int)(ground)-80)));
                    break;

                case 1:
                    Dummy = dll::CreatureFactory(evil2_flag, (float)(RandGenerator(250, (int)(map_width)-80)),
                        (float)(RandGenerator(-50, (int)(ground)-80)));
                    break;

                case 2:
                    Dummy = dll::CreatureFactory(evil3_flag, (float)(RandGenerator(250, (int)(map_width)-80)),
                        (float)(RandGenerator(-50, (int)(ground)-80)));
                    break;

                case 3:
                    Dummy = dll::CreatureFactory(evil4_flag, (float)(RandGenerator(250, (int)(map_width)-80)),
                        (float)(RandGenerator(-50, (int)(ground)-80)));
                    break;

                case 4:
                    Dummy = dll::CreatureFactory(evil5_flag, (float)(RandGenerator(250, (int)(map_width)-80)),
                        (float)(RandGenerator(-50, (int)(ground)-80)));
                    break;
                }

                if (Dummy)
                {
                    if (!(Hero->x >= Dummy->ex || Hero->ex <= Dummy->x || Hero->y >= Dummy->ey || Hero->ey <= Dummy->y))
                    {
                        Dummy->Release();
                        is_ok = false;
                        break;
                    }

                    for (int i = 0; i < vObstacles.size(); i++)
                    {
                        if (!(Dummy->x >= vObstacles[i]->ex || Dummy->ex <= vObstacles[i]->x ||
                            Dummy->y >= vObstacles[i]->ey || Dummy->ey <= vObstacles[i]->y))
                        {
                            Dummy->Release();
                            is_ok = false;
                            break;
                        }
                    }

                    if (is_ok)
                    {
                        vEvils.push_back(Dummy);
                        dll::PROT_CONTAINER Obstacles(vObstacles.size());
                        if (Obstacles.is_valid())
                            for (int i = 0; i < vObstacles.size(); ++i)
                            {
                                dll::PROTON an_obstacle{ vObstacles[i]->x,vObstacles[i]->y,vObstacles[i]->GetWidth(),
                                vObstacles[i]->GetHeight() };
                                Obstacles.push_back(an_obstacle);
                            }
                        switch (RandGenerator(0, 1))
                        {
                        case 0:
                            vEvils.back()->Move((float)(level), Obstacles, true, 0, vEvils.back()->y);
                            break;

                        case 1:
                            vEvils.back()->Move((float)(level), Obstacles, true, map_width, vEvils.back()->y);
                            break;
                        }
                    }
                }
            }
        }
    }
}
void SaveGame()
{
    int result = 0;
    CheckFile(save_file, &result);
    if (result == FILE_EXIST)
    {
        if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
        if (MessageBox(bHwnd, L"Има записана игра, която ще загубиш !\n\nНаистина ли я презаписваш ?",
            L"Презапис ?", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)return;
    }

    std::wofstream save(save_file);

    save << level << std::endl;
    save << score << std::endl;
    save << secs << std::endl;
    save << sound << std::endl;
    save << cloak_on << std::endl;
    save << mail_on << std::endl;
    save << cloak_lifes << std::endl;
    save << mail_lifes << std::endl;
    save << club_lifes << std::endl;
    save << axe_lifes << std::endl;
    save << sword_lifes << std::endl;
    for (int i = 0; i < 16; i++)save << static_cast<int>(current_player[i]) << std::endl;
    save << name_set << std::endl;
    save << hero_killed << std::endl;

    save << ViewMapRect.left << std::endl;
    save << ViewMapRect.right << std::endl;
    save << ViewMapRect.top << std::endl;
    save << ViewMapRect.bottom << std::endl;

    save << vObstacles.size() << std::endl;
    if (!vObstacles.empty())
    {
        for (int i = 0; i < vObstacles.size(); ++i)
        {
            save << static_cast<int>(vObstacles[i]->GetType()) << std::endl;
            save << vObstacles[i]->x << std::endl;
            save << vObstacles[i]->y << std::endl;
        }
    }

    save << vAssets.size() << std::endl;
    if (!vAssets.empty())
    {
        for (int i = 0; i < vAssets.size(); ++i)
        {
            save << static_cast<int>(vAssets[i]->GetType()) << std::endl;
            save << vAssets[i]->x << std::endl;
            save << vAssets[i]->y << std::endl;
        }
    }

    save << vCrystals.size() << std::endl;
    if (!vCrystals.empty())
    {
        for (int i = 0; i < vCrystals.size(); ++i)
        {
            save << vCrystals[i]->x << std::endl;
            save << vCrystals[i]->y << std::endl;
        }
    }

    save << Hero->x << std::endl;
    save << Hero->y << std::endl;
    save << Hero->GetTypeFlag() << std::endl;
    save << Hero->lifes << std::endl;

    save << vEvils.size() << std::endl;
    if (!vEvils.empty())
    {
        for (int i = 0; i < vEvils.size(); ++i)
        {
            save << vEvils[i]->x << std::endl;
            save << vEvils[i]->y << std::endl;
            save << vEvils[i]->GetTypeFlag() << std::endl;
            save << vEvils[i]->lifes << std::endl;
        }
    }

    save.close();

    if (sound)mciSendString(L"play .\\res\\snd\\save.wav", NULL, NULL, NULL);
    MessageBox(bHwnd, L"Играта е запазена !", L"Запис !", MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
}
void LoadGame()
{
    int result = 0;
    CheckFile(save_file, &result);
    if (result == FILE_NOT_EXIST)
    {
        if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
        MessageBox(bHwnd, L"Все още няма записана игра !\n\nПостарай се повече !", L"Липсва файл !",
            MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
        return;
    }
    else
    {
        if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
        if (MessageBox(bHwnd, L"Настоящата игра ще бъде загубена !\n\nНаистина ли я презаписваш ?",
            L"Презапис ?", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)return;
    }

    mins = 0;
    secs = 0;

    move_hero = false;

    ViewMapRect.left = -200.0f;
    ViewMapRect.top = -100.0f;
    ViewMapRect.right = map_width;
    ViewMapRect.bottom = map_height;

    ClearHeap(&Hero);
    Hero = dll::CreatureFactory(hero_flag, 80.0f, ground - 150.0f);

    if (!vObstacles.empty())
        for (int i = 0; i < vObstacles.size(); ++i)ClearHeap(&vObstacles[i]);
    vObstacles.clear();

    if (!vCrystals.empty())
        for (int i = 0; i < vCrystals.size(); ++i)ClearHeap(&vCrystals[i]);
    vCrystals.clear();

    if (!vAssets.empty())
        for (int i = 0; i < vAssets.size(); ++i)ClearHeap(&vAssets[i]);
    vAssets.clear();

    if (!vEvils.empty())
        for (int i = 0; i < vEvils.size(); ++i)ClearHeap(&vEvils[i]);
    vEvils.clear();
    //////////////////////////////////////////////////////////////////////////

    std::wifstream save(save_file);

    save >> level;
    save >> score;
    save >> secs;
    save >> sound;
    save >> cloak_on;
    save >> mail_on;
    save >> cloak_lifes;
    save >> mail_lifes;
    save >> club_lifes;
    save >> axe_lifes;
    save >> sword_lifes;
    for (int i = 0; i < 16; i++)
    {
        int letter = 0;
        save >> letter;
        current_player[i] = static_cast<wchar_t>(letter);
    }
    save >> name_set;
    save >> hero_killed;

    float temp_viewrect_dim = 0;
    save >> temp_viewrect_dim;
    ViewMapRect.left = temp_viewrect_dim;
    save >> temp_viewrect_dim;
    ViewMapRect.right = temp_viewrect_dim;
    save >> temp_viewrect_dim;
    ViewMapRect.top = temp_viewrect_dim;
    save >> temp_viewrect_dim;
    ViewMapRect.bottom = temp_viewrect_dim;
    
    if (hero_killed)GameOver();

    save >> result;
    if (result > 0)
    {
        for (int i = 0; i < result; i++)
        {
            int16_t ttype = 0;
            float tx = 0;
            float ty = 0;

            save >> ttype;
            save >> tx;
            save >> ty;
            vObstacles.push_back(dll::AssetFactory(ttype, tx, ty));
        }
    }

    save >> result;
    if (result > 0)
    {
        for (int i = 0; i < result; i++)
        {
            int16_t ttype = 0;
            float tx = 0;
            float ty = 0;

            save >> ttype;
            save >> tx;
            save >> ty;
            vAssets.push_back(dll::AssetFactory(ttype, tx, ty));
        }
    }

    save >> result;
    if (result > 0)
    {
        for (int i = 0; i < result; i++)
        {
            float tx = 0;
            float ty = 0;

            save >> tx;
            save >> ty;
            vCrystals.push_back(dll::AssetFactory(crystal_flag, tx, ty));
        }
    }

    float hero_tx = 0;
    float hero_ty = 0;
    int hero_ttype = 0;
    int hero_tlifes = 0;

    save >> hero_tx;
    save >> hero_ty;
    save >> hero_ttype;
    save >> hero_tlifes;

    Hero = dll::CreatureFactory(hero_flag, hero_tx, hero_ty);
    Hero->Transform(static_cast<unsigned char>(hero_ttype));
    Hero->lifes = hero_tlifes;

    save >> result;
    if (result > 0)
    {
        for (int i = 0; i < result; i++)
        {
            float tx = 0;
            float ty = 0;
            int ttype = 0;
            int tlifes = 0;

            save >> tx;
            save >> ty;
            save >> ttype;
            save >> tlifes;

            vEvils.push_back(dll::CreatureFactory(static_cast<unsigned char>(ttype), tx, ty));
            vEvils.back()->lifes = tlifes;
        }
    }
  
    save.close();

    if (sound)mciSendString(L"play .\\res\\snd\\save.wav", NULL, NULL, NULL);
    MessageBox(bHwnd, L"Играта е заредена !", L"Зареждане !", MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
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
        AppendMenu(bBar, MF_POPUP, (UINT_PTR)bStore, L"Меню за данни");
        
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

        case mLvl:
            pause = true;
            if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"Ако прескочиш нивото, ще загубиш тази игра !\n\nНаистина ли прескачаш ниво ?",
                L"Прескачане на ниво ?", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
            {
                pause = false;
                break;
            }
            LevelUp();
            break;

        case mExit:
            SendMessage(hwnd, WM_CLOSE, NULL, NULL);
            break;

        case mSave:
            pause = true;
            SaveGame();
            pause = false;
            break;

        case mLoad:
            pause = true;
            LoadGame();
            pause = false;
            break;

        case mHoF:
            pause = true;
            HallOfFame();
            pause = false;
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
        else
        {
            if (LOWORD(lParam) >= b1Rect.left && LOWORD(lParam) <= b1Rect.right)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                if (DialogBox(bIns, MAKEINTRESOURCE(IDD_PLAYER), hwnd, &bDlgProc) == IDOK)name_set = true;
                break;
            }
            if (LOWORD(lParam) >= b2Rect.left && LOWORD(lParam) <= b2Rect.right)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                if (sound)
                {
                    sound = false;
                    PlaySound(NULL, NULL, NULL);
                    break;
                }
                else
                {
                    sound = true;
                    PlaySound(sound_file, NULL, SND_ASYNC | SND_LOOP);
                    break;
                }
            }
        }
        break;

    case WM_KEYDOWN:
        if (LOWORD(wParam) == VK_SPACE)move_hero = false;
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

                hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &LifeBrush);
                hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orange), &HurtBrush);
                hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkRed), &CritBrush);


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
                bool checker = Hero->Move((float)(level), ObstacleChecker);
                
                if (!checker)
                {
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
                            if (!vAssets.empty())
                                for (int i = 0; i < vAssets.size(); ++i)
                                {
                                    vAssets[i]->x -= (float)(level);
                                    vAssets[i]->SetEdges();
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
                            if (!vAssets.empty())
                                for (int i = 0; i < vAssets.size(); ++i)
                                {
                                    vAssets[i]->x += (float)(level);
                                    vAssets[i]->SetEdges();
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
                            if (!vAssets.empty())
                                for (int i = 0; i < vAssets.size(); ++i)
                                {
                                    vAssets[i]->y -= (float)(level);
                                    vAssets[i]->SetEdges();
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
                            if (!vAssets.empty())
                                for (int i = 0; i < vAssets.size(); ++i)
                                {
                                    vAssets[i]->y += (float)(level);
                                    vAssets[i]->SetEdges();
                                }
                        }
                    }
                }
                else move_hero = false;
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

        if (!vAssets.empty() && Hero)
        {
            for (std::vector<dll::asset_ptr>::iterator asset = vAssets.begin(); asset < vAssets.end(); asset++)
            {
                if (!(Hero->x > (*asset)->ex || Hero->ex<(*asset)->x || Hero->y>(*asset)->ey || Hero->ey < (*asset)->y))
                {
                    if ((*asset)->CheckFlag(gold_flag))score += 50 * level;
                    if ((*asset)->CheckFlag(potion_flag))Hero->lifes = 200;
                    if ((*asset)->CheckFlag(club_flag))
                    {
                        Hero->Transform(hero_club_flag);
                        club_lifes = 100;
                    }
                    if ((*asset)->CheckFlag(axe_flag))
                    {
                        Hero->Transform(hero_axe_flag);
                        club_lifes = 100;
                    }
                    if ((*asset)->CheckFlag(sword_flag))
                    {
                        Hero->Transform(hero_sword_flag);
                        club_lifes = 100;
                    }
                    if ((*asset)->CheckFlag(cloak_flag))
                    {
                        cloak_lifes = 100;
                        cloak_on = true;
                        mail_on = false;
                    }
                    if ((*asset)->CheckFlag(mail_flag))
                    {
                        mail_lifes = 200;
                        mail_on = true;
                        cloak_on = false;
                    }
                    (*asset)->Release();
                    vAssets.erase(asset);
                    if (sound)mciSendString(L"play .\\res\\snd\\takeasset.wav", NULL, NULL, NULL);
                    break;
                }
            }
        }

        if (!vEvils.empty() && Hero)
        {
            for (int i = 0; i < vEvils.size(); ++i)
            {
                if (vEvils[i]->Distance(POINT((LONG)(Hero->x), (LONG)(Hero->y)), 
                    POINT((LONG)(vEvils[i]->x), (LONG)(vEvils[i]->y))) <= 100.0f)
                {
                    if (!vObstacles.empty())
                    {
                        dll::PROT_CONTAINER Obstacles(vObstacles.size());
                        if (Obstacles.is_valid())
                            for (int i = 0; i < vObstacles.size(); ++i)
                            {
                                dll::PROTON an_obstacle{ vObstacles[i]->x,vObstacles[i]->y,vObstacles[i]->GetWidth(),
                                vObstacles[i]->GetHeight() };
                                Obstacles.push_back(an_obstacle);
                            }

                        vEvils[i]->Move((float)(level), Obstacles, true, Hero->x, Hero->y);
                    }
                }
                else
                {
                    if (!vObstacles.empty())
                    {
                        dll::PROT_CONTAINER Obstacles(vObstacles.size());
                        if (Obstacles.is_valid())
                            for (int i = 0; i < vObstacles.size(); ++i)
                            {
                                dll::PROTON an_obstacle{ vObstacles[i]->x,vObstacles[i]->y,vObstacles[i]->GetWidth(),
                                vObstacles[i]->GetHeight() };
                                Obstacles.push_back(an_obstacle);
                            }

                        vEvils[i]->Move((float)(level), Obstacles);
                    }
                }
            }
        }

        if (Hero && !vEvils.empty())
        {
            for (std::vector<dll::creature_ptr>::iterator evil = vEvils.begin(); evil < vEvils.end(); evil++)
            {
                if (!(Hero->x >= (*evil)->ex || Hero->ex <= (*evil)->x || Hero->y >= (*evil)->ey || Hero->ey <= (*evil)->y))
                {
                    int damage = (*evil)->Attack();
                    if (damage > 0)
                    {
                        if (cloak_on)
                        {
                            cloak_lifes--;
                            if (cloak_lifes <= 0) cloak_on = false;
                            if (sound)mciSendString(L"play .\\res\\snd\\blocked.wav", NULL, NULL, NULL);
                        }
                        else if (mail_on)
                        {
                            mail_lifes--;
                            if (mail_lifes <= 0) mail_on = false;
                            if (sound)mciSendString(L"play .\\res\\snd\\blocked.wav", NULL, NULL, NULL);
                        }
                        else
                        {
                            if (sound)mciSendString(L"play .\\res\\snd\\hero_hurt.wav", NULL, NULL, NULL);
                            Hero->lifes -= damage;
                        }
                    }
                    
                    damage = Hero->Attack();
                    if (damage > 0)
                    {
                        if (RandGenerator(0, 10) == 6)
                        {
                            if (Hero->CheckType(hero_club_flag))
                            {
                                --club_lifes;
                                if (club_lifes <= 0)
                                {
                                    Hero->Transform(hero_flag);
                                    if (sound)mciSendString(L"play .\\res\\snd\\broke.wav", NULL, NULL, NULL);
                                }
                            }
                            else if (Hero->CheckType(hero_axe_flag))
                            {
                                --axe_lifes;
                                if (axe_lifes <= 0)
                                {
                                    Hero->Transform(hero_flag);
                                    if (sound)mciSendString(L"play .\\res\\snd\\broke.wav", NULL, NULL, NULL);
                                }
                            }
                            else if (Hero->CheckType(hero_sword_flag))
                            {
                                --sword_lifes;
                                if (sword_lifes <= 0)
                                {
                                    Hero->Transform(hero_flag);
                                    if (sound)mciSendString(L"play .\\res\\snd\\broke.wav", NULL, NULL, NULL);
                                }
                            }
                        }
                        (*evil)->lifes -= damage;
                        if (RandGenerator(0, 20) == 6)
                        {
                            if (Hero->CheckType(hero_club_flag))
                            {
                                club_lifes--;
                                if (club_lifes <= 0)Hero->Transform(hero_flag);
                            }
                            else if (Hero->CheckType(hero_axe_flag))
                            {
                                axe_lifes--;
                                if (axe_lifes <= 0)Hero->Transform(hero_flag);
                            }
                            else if (Hero->CheckType(hero_sword_flag))
                            {
                                sword_lifes--;
                                if (sword_lifes <= 0)Hero->Transform(hero_flag);
                            }
                        }
                    }
                    if (Hero->lifes <= 0)
                    {
                        RIP_x = Hero->x;
                        RIP_y = Hero->y;
                        Hero->Release();
                        Hero = nullptr;
                        hero_killed = true;
                        break;
                    }
                    if ((*evil)->lifes <= 0)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\evilkilled.wav", NULL, NULL, NULL);
                        score += 10 + level;
                        (*evil)->Release();
                        vEvils.erase(evil);
                        break;
                    }
                }
            }
        }

        if (vCrystals.empty()) LevelUp();
        
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
        if (!vAssets.empty())
        {
            for (int i = 0; i < vAssets.size(); ++i)
            {
                if (vAssets[i]->CheckFlag(gold_flag))
                    Draw->DrawBitmap(bmpGold, D2D1::RectF(vAssets[i]->x, vAssets[i]->y, vAssets[i]->ex, vAssets[i]->ey));
                if (vAssets[i]->CheckFlag(club_flag))
                    Draw->DrawBitmap(bmpClub, D2D1::RectF(vAssets[i]->x, vAssets[i]->y, vAssets[i]->ex, vAssets[i]->ey));
                if (vAssets[i]->CheckFlag(axe_flag))
                    Draw->DrawBitmap(bmpAxe, D2D1::RectF(vAssets[i]->x, vAssets[i]->y, vAssets[i]->ex, vAssets[i]->ey));
                if (vAssets[i]->CheckFlag(sword_flag))
                    Draw->DrawBitmap(bmpSword, D2D1::RectF(vAssets[i]->x, vAssets[i]->y, vAssets[i]->ex, vAssets[i]->ey));
                if (vAssets[i]->CheckFlag(potion_flag))
                    Draw->DrawBitmap(bmpPotion, D2D1::RectF(vAssets[i]->x, vAssets[i]->y, vAssets[i]->ex, vAssets[i]->ey));
                if (vAssets[i]->CheckFlag(mail_flag))
                    Draw->DrawBitmap(bmpMail, D2D1::RectF(vAssets[i]->x, vAssets[i]->y, vAssets[i]->ex, vAssets[i]->ey));
                if (vAssets[i]->CheckFlag(cloak_flag))
                    Draw->DrawBitmap(bmpCloak, D2D1::RectF(vAssets[i]->x, vAssets[i]->y, vAssets[i]->ex, vAssets[i]->ey));
            }
        }
        if (!vEvils.empty())
        {
            for (int i = 0; i < vEvils.size(); i++)
            {
                if (vEvils[i]->CheckType(evil1_flag))
                {
                    if (vEvils[i]->dir == dirs::left)
                    {
                        int aframe = vEvils[i]->GetFrame();
                        Draw->DrawBitmap(bmpEvil1L[aframe], Resizer(bmpEvil1L[aframe], vEvils[i]->x, vEvils[i]->y));
                    }
                    else
                    {
                        int aframe = vEvils[i]->GetFrame();
                        Draw->DrawBitmap(bmpEvil1R[aframe], Resizer(bmpEvil1R[aframe], vEvils[i]->x, vEvils[i]->y));
                    }
                }
                if (vEvils[i]->CheckType(evil2_flag))
                {
                    int aframe = vEvils[i]->GetFrame();
                    Draw->DrawBitmap(bmpEvil2[aframe], Resizer(bmpEvil2[aframe], vEvils[i]->x, vEvils[i]->y));
                }
                if (vEvils[i]->CheckType(evil3_flag))
                {
                    if (vEvils[i]->dir == dirs::left)
                    {
                        int aframe = vEvils[i]->GetFrame();
                        Draw->DrawBitmap(bmpEvil3L[aframe], Resizer(bmpEvil3L[aframe], vEvils[i]->x, vEvils[i]->y));
                    }
                    else
                    {
                        int aframe = vEvils[i]->GetFrame();
                        Draw->DrawBitmap(bmpEvil3R[aframe], Resizer(bmpEvil3R[aframe], vEvils[i]->x, vEvils[i]->y));
                    }
                }
                if (vEvils[i]->CheckType(evil4_flag))
                {
                    int aframe = vEvils[i]->GetFrame();
                    Draw->DrawBitmap(bmpEvil4[aframe], Resizer(bmpEvil4[aframe], vEvils[i]->x, vEvils[i]->y));
                }
                if (vEvils[i]->CheckType(evil5_flag))
                {
                    int aframe = vEvils[i]->GetFrame();
                    Draw->DrawBitmap(bmpEvil5[aframe], Resizer(bmpEvil5[aframe], vEvils[i]->x, vEvils[i]->y));
                }
            }
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

        if (nrmTextFormat && hgltBrush && HurtBrush)
        {
            wchar_t stat_txt[150] = L"\0";
            wchar_t add[5] = L"\0";
            int stat_size = 0;

            wcscpy_s(stat_txt, current_player);
            wcscat_s(stat_txt, L", резултат: ");
            wsprintf(add, L"%d", score);
            wcscat_s(stat_txt, add);
            wcscat_s(stat_txt, L", ниво: ");
            wsprintf(add, L"%d", level);
            wcscat_s(stat_txt, add);
            wcscat_s(stat_txt, L", време: ");
            if (mins < 10)wcscat_s(stat_txt, L"0");
            wsprintf(add, L"%d", mins);
            wcscat_s(stat_txt, add);
            wcscat_s(stat_txt, L", : ");
            if (secs - mins * 60 < 10)wcscat_s(stat_txt, L"0");
            wsprintf(add, L"%d", secs - mins * 60);
            wcscat_s(stat_txt, add);
            
            for (int i = 0; i < 150; ++i)
            {
                if (stat_txt[i] != '\0')stat_size++;
                else break;
            }
            Draw->DrawTextW(stat_txt, stat_size, nrmTextFormat,
                D2D1::RectF(5.0f, ground + 5.0f, scr_width * 2 / 3, scr_height), HurtBrush);
            //////////////////////////////////////////////
            wchar_t armor_txt[30] = L"\0";
            wchar_t arm_add[5] = L"\0";
            int arm_size = 0;
            
            if (!cloak_on && !mail_on)
                Draw->DrawTextW(L"НЯМА БРОНЯ !", 13, nrmTextFormat,
                    D2D1::RectF(scr_width - scr_width / 3, ground + 5.0f, scr_width, scr_height), inactBrush);
            else
            {
                if (cloak_on)
                {
                    wcscpy_s(armor_txt, L"наметало: ");
                    wsprintf(arm_add, L"%d", cloak_lifes);
                    wcscat_s(armor_txt, arm_add);
                }
                else if (mail_on)
                {
                    wcscpy_s(armor_txt, L"броня: ");
                    wsprintf(arm_add, L"%d", mail_lifes);
                    wcscat_s(armor_txt, arm_add);
                }
                for (int i = 0; i < 30; i++)
                {
                    if (armor_txt[i] != '\0')++arm_size;
                    else break;
                }
                Draw->DrawTextW(armor_txt, arm_size, nrmTextFormat,
                    D2D1::RectF(scr_width - scr_width / 3, ground + 5.0f, scr_width, scr_height), HurtBrush);
            }
            
        }
        ///////////////////////////////////////////////////

        if (hero_killed)
        {
            Draw->DrawBitmap(bmpRIP, D2D1::RectF(RIP_x, RIP_y, RIP_x + 80.0f, RIP_y + 94.0f));
            Draw->EndDraw();
            PlaySound(NULL, NULL, NULL);
            if (sound)PlaySound(L".\\res\\snd\\killed.wav", NULL, SND_SYNC);
            else Sleep(2000);
            GameOver();
        }
        
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

            if (Hero->lifes > 120)
                Draw->DrawLine(D2D1::Point2F(Hero->x - 5.0f, Hero->ey), D2D1::Point2F(Hero->x + (float)(Hero->lifes / 5), 
                    Hero->ey), LifeBrush, 5.0f);
            else if (Hero->lifes > 60)
                Draw->DrawLine(D2D1::Point2F(Hero->x - 5.0f, Hero->ey), D2D1::Point2F(Hero->x + (float)(Hero->lifes / 5), 
                    Hero->ey), HurtBrush, 5.0f);
            else
                Draw->DrawLine(D2D1::Point2F(Hero->x - 5.0f, Hero->ey), D2D1::Point2F(Hero->x + (float)(Hero->lifes / 5), 
                    Hero->ey), CritBrush, 5.0f);
        }
        
        ////////////////////////////////////////////////
        Draw->EndDraw();

    }

    std::remove(tmp_file);
    
    ReleaseResources();

    return (int) bMsg.wParam;
}