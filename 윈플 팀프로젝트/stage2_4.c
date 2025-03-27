#include <windows.h>  
#include <tchar.h>    
#include <string.h>
#include <time.h>
#include <mmsystem.h>
#include "resource.h"
#pragma comment(lib,"winmm.lib")

//타이머 1: 플레이어 이동 타이머
//타이머 2: 시간 나타내는 타이머
//타이머 3: stage 2 탄막 간 시간 간격 타이머
//타이머 4: stage 2 탄막 패턴 타이머
//타이머 5: stage 2 경고 사각형 표시 타이머
#define marginX 16
#define marginY 39

#define RECT_SIZE 30           //플레이어 크기
#define MOVE_STEP 10           // ->,<- 이동속도
#define JUMP_HEIGHT 100        //점프 높이
#define JUMP_UP_SPEED 10       // 점프키 누르고 올라가는 속도
#define JUMP_DOWN_SPEED 10     // 점프하고 주인공이 내려가는 속도

#define MAX_HP 5                // 최대 HP량

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Final Survival";

typedef struct {
    int x;  // x 좌표
    int y;  // y 좌표
    int width;  // 가로
    int height;  // 세로

    int jumping;  // 점프 중인지 여부
    int jumpzero;  // 점프를 할때 변화량 예를들어 설명하면 주인공이 점프뛸때 jumpzero의 값이 커지다가 주인공이 땅에 내려갈때 작아지도록 만듬 

    int jumping_up;  // 점프 상승 중인지 여부
    int double_jump;  // 이단 점프 여부

    int move_left;  // 왼쪽 이동 여부
    int move_right;  // 오른쪽 이동 여부

    int current_jump;  // 현재 점프 높이
} Player;

// 플레이어 초기화
Player player = { 0, 0, RECT_SIZE, RECT_SIZE, 0, 0, 0, 0, 0, 0, 0 };

//시간 초기화
int playTime = 0;

int gamestart;   //게임 시작 여부
int paused;      //게임 퍼즈 여부
int gameover;   //게임 오버 여부

//스테이지 선택
int stage = 0;

// 게임 타이틀 그리기 함수
void game_Title(HWND hWnd, HDC hdc) {
    RECT rect;
    GetClientRect(hWnd, &rect); // 윈도우 화면 얻어오기

    // 사각형 그리기
    RECT titlerect = { rect.right / 2 - 250, 100, rect.right / 2 + 250, 200 };
    HBRUSH titlebrush = CreateSolidBrush(RGB(0, 255, 200));
    FillRect(hdc, &titlerect, titlebrush);
    DeleteObject(titlebrush);

    // 텍스트 그리기
    HFONT font = CreateFont(72, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial"));
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 0, 0));
    TextOut(hdc, rect.right / 2 - 200, 115, TEXT("Final Survival"), lstrlen(TEXT("Final Survival")));

    font = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial"));
    oldFont = (HFONT)SelectObject(hdc, font);
    SetTextColor(hdc, RGB(128, 128, 128)); // 회색 텍스트 색상
    TextOut(hdc, rect.right / 2 - 100, 450, TEXT("Press Enter to start"), lstrlen(TEXT("Press Enter to start")));

    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

// 퍼즈 화면 그리기 함수
void Paused(HWND hWnd, HDC hdc) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    Rectangle(hdc, rect.right / 2 - 250, 100, rect.right / 2 + 250, 200);

    HFONT font = CreateFont(72, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial"));
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 0, 0));
    TextOut(hdc, rect.right / 2 - 105, 115, TEXT("PAUSE"), lstrlen(TEXT("PAUSE")));

    font = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial"));
    oldFont = (HFONT)SelectObject(hdc, font);
    SetTextColor(hdc, RGB(128, 128, 128)); // 회색 텍스트 색상
    TextOut(hdc, rect.right / 2 - 100, 450, TEXT("Press P to unpause"), lstrlen(TEXT("Press P to unpause")));

    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

//게임 오버 화면 그리기 함수
void Gameover_Screen(HWND hWnd, HDC hdc) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    Rectangle(hdc, rect.right / 2 - 250, 100, rect.right / 2 + 250, 200);
    HFONT font = CreateFont(72, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial"));
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 0, 0));
    TextOut(hdc, rect.right / 2 - 150, 115, TEXT("Game over"), lstrlen(TEXT("Game over")));
    SelectObject(hdc, oldFont);
    DeleteObject(font);

    RECT ScoreRect = { rect.right / 2 - 75, 300,  rect.right / 2 + 75, 400 }; // 점수 출력될 부분
    TCHAR ScoreStr[35];
    wsprintf(ScoreStr, L"Score : %d", playTime);
    DrawText(hdc, ScoreStr, -1, &ScoreRect, DT_SINGLELINE | DT_LEFT);  //문자열의 길이 자동으로 계산
}

// 플레이어가 화면 밖으로 벗어나지 않게 해주는 함수
void Prevent_Leave_Screen(HWND hWnd) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    if (player.x < 0) player.x = 0;
    if (player.x + player.width > rect.right) player.x = rect.right - player.width;  // 화면 오른쪽 제한
    if (player.y + player.height > rect.bottom) player.y = rect.bottom - player.height;  // 화면 아래쪽 제한
}

// HP 그리기 함수
void Draw_HP(HDC hdc, int current_hp) {
    RECT hp[MAX_HP];

    //HP 그릴 위치 정의
    for (int i = 0;i < MAX_HP;i++) {
        hp[i].left = 10 + (40 * i);
        hp[i].top = 20;
        hp[i].right = 50 + (40 * i);
        hp[i].bottom = 40;
    }

    //현재 HP에 따라 HP 그리기
    for (int i = 0;i < current_hp;i++) {
        Rectangle(hdc, hp[i].left, hp[i].top, hp[i].right, hp[i].bottom);
    }
}

//시간(점수)그리기 함수
void Draw_timeRect(HDC hdc, int playTime) {
    RECT timeRect = { 10, 50, 100, 80 }; // 시간 출력될 부분
    TCHAR timeStr[20];

    wsprintf(timeStr, L"Score : %d", playTime);

    DrawText(hdc, timeStr, -1, &timeRect, DT_SINGLELINE | DT_LEFT);  //문자열의 길이 자동으로 계산
}


//--------------------------------------------------------------
#define BAR_NUM 10
#define WARNING_TIME 2000 // 2초 전에 경고

typedef struct {    //탄막 구조체
    int x, y;       //탄막의 "중앙" 좌표
    int width, length;  //탄막의 가로, 세로 길이
    int shooted;      //탄막이 나갔는지 안나갔는지 여부 (나갔으면 1, 안나갔으면 0)
}Barrage;

Barrage weapon[BAR_NUM];   //탄막 생성
int weaponPattern;  //탄막 패턴
int invincible = 0; //무적 여부
int is_warning; //경고 사각형이 띄워졌는지 여부

// 무적 상태 해제 타이머 체크
void CALLBACK Invincible(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
    invincible = 0; // 무적 상태 해제
    KillTimer(hWnd, 10);
}

//충돌체크 (WM_TIMER에서 이 함수를 호출합니다)
void CollisionCheck(HWND hWnd, int* current_hp) {
    RECT playerRect = { player.x, player.y - player.jumpzero, player.x + player.width, player.y + player.height - player.jumpzero };
    for (int i = 0; i < 10; i++) {
        RECT weaponRect = { weapon[i].x - (weapon[i].width / 2), weapon[i].y - (weapon[i].length / 2), weapon[i].x + (weapon[i].width / 2), weapon[i].y + (weapon[i].length / 2) };
        if (playerRect.left < weaponRect.right &&
            playerRect.right > weaponRect.left &&
            playerRect.top < weaponRect.bottom &&
            playerRect.bottom > weaponRect.top) {
            if (!invincible) { // 무적 상태가 아닐 때만 HP 감소
                (*current_hp)--;
                if (*current_hp <= 0) {
                    gameover = 1;
                    gamestart = 0;
                    InvalidateRect(hWnd, NULL, TRUE);
                }
                invincible = 1; // 무적 상태로 설정
                SetTimer(hWnd, 10, 3000, Invincible); // 3초 후 무적 상태 해제 타이머 설정
            }
        }
    }
}

void DrawInvincibleStatus(HDC hdc) { //무적기능 표시
    if (invincible) {
        RECT invincibleRect = { 900, 20, 1000, 50 }; // 우측 상단에 위치
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 0, 0)); // 빨간색 텍스트
        DrawText(hdc, TEXT("무적 ON"), -1, &invincibleRect, DT_SINGLELINE | DT_LEFT);
    }
}

//탄막 초기화 함수
void AtkInit() {
    for (int i = 0;i < BAR_NUM;i++) {
        weapon[i].x = -100;
        weapon[i].y = -100;
        weapon[i].width = 0;
        weapon[i].length = 0;
        weapon[i].shooted = 0;
    }
    weaponPattern = 0;
    is_warning = 0;
}

//경고 사각형 그리기 함수
void DrawWarningRect(HDC hdc, int x, int y, int width, int height, COLORREF color) {
    HPEN pen = CreatePen(PS_DOT, 1, color);
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

    Rectangle(hdc, x, y, x + width, y + height);

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pen);
}

//1스테이지 경고 사각형 설정 함수
void Stage1_SetWarningRects(HWND hWnd, HDC hdc) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    if (is_warning == 1) {
        switch (weaponPattern + 1) {
        case 1: //2~4초에 등장할 위험 경고
            DrawWarningRect(hdc, rect.right - 120, rect.bottom - 120, rect.right, rect.bottom, RGB(255, 0, 0));
            break;
        case 2: //6~8초에 등장할 위험 경고
            break;
        case 3: //10~12초에 등장할 위험 경고
            DrawWarningRect(hdc, rect.right - 70, rect.bottom - 70, 70, 70, RGB(255, 0, 0));
            break;
        case 4: //14~16초에 등장할 위혐 경고
            DrawWarningRect(hdc, rect.left, rect.bottom - 70, 70, 70, RGB(255, 0, 0));
            break;
        case 5: //18~20초에 등장할 위험 경고
            DrawWarningRect(hdc, rect.right - 120, rect.bottom - 120, 120, 120, RGB(255, 0, 0));
            break;
        case 6: //22~24초에 등장할 위험 경고
            DrawWarningRect(hdc, rect.right - 120, rect.bottom - 120, 120, 120, RGB(255, 0, 0));
            break;
        }
    }
}

//3스테이지 경고 사각형 설정 함수
void Stage3_SetWarningRects(HWND hWnd, HDC hdc) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    if (is_warning == 1) {
        switch (weaponPattern + 1) {
        case 1: //2~4초에 등장할 위험 경고
            DrawWarningRect(hdc, 0, rect.bottom - 100, 100, 100, RGB(255, 0, 0));
            break;
        case 2: //6~8초에 등장할 위험 경고
            DrawWarningRect(hdc, rect.right - 100, rect.bottom - 100, 100, 100, RGB(255, 0, 0));
            break;
        case 3: //10~12초에 등장할 위험 경고
            DrawWarningRect(hdc, rect.left + 100, rect.top + 200, 30, 30, RGB(255, 0, 0));
            break;
        case 4: //14~16초에 등장할 위혐 경고
            DrawWarningRect(hdc, rect.right - 100, rect.top + 200, 30, 30, RGB(255, 0, 0));
            break;
        case 5: //18~20초에 등장할 위험 경고
            break;
        case 6: //22~24초에 등장할 위험 경고
            DrawWarningRect(hdc, 0, rect.top, rect.right - 630, rect.bottom, RGB(0, 0, 255));
            break;
        }
    }
}

//2스테이지 경고 사각형 설정 함수
void Stage2_SetWarningRects(HWND hWnd, HDC hdc) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    if (is_warning == 1) {
        switch (weaponPattern + 1) {
        case 1:
            DrawWarningRect(hdc, 0, rect.bottom - 80, rect.right, 60, RGB(255, 0, 0));
            DrawWarningRect(hdc, 0, rect.bottom - 280, rect.right, 60, RGB(255, 0, 0));
            DrawWarningRect(hdc, 0, rect.bottom - 480, rect.right, 60, RGB(255, 0, 0));
            break;
        case 2:
            DrawWarningRect(hdc, player.x + (player.width / 2) - 50, rect.top, 100, rect.bottom, RGB(255, 0, 0));
            break;
        case 3:
            DrawWarningRect(hdc, 0, rect.bottom - 300, 300, 300, RGB(0, 0, 255));
            break;
        case 5:
            DrawWarningRect(hdc, rect.right / 2 - 300, rect.bottom - 600, 600, 600, RGB(255, 0, 0));
            break;
        case 6:
            DrawWarningRect(hdc, 630, rect.top, rect.right - 630, rect.bottom, RGB(0, 0, 255));
            break;
        }
    }
}

//4스테이지 경고 사각형 설정 함수
void Stage4_SetWarningRects(HWND hWnd, HDC hdc) {
    RECT rect;
    GetClientRect(hWnd, &rect);

    if (is_warning == 1) {
        switch (weaponPattern + 1) {
        case 1:
            DrawWarningRect(hdc, rect.left, rect.top, rect.right, rect.right / BAR_NUM, RGB(255, 0, 0));
            break;

        case 2:
            DrawWarningRect(hdc, rect.right - 100, rect.bottom - 250, 70, 250, RGB(255, 0, 0));
            break;

        case 3:
        case 4:
            break;

        case 5:
            DrawWarningRect(hdc, rect.right / 2 - 100, rect.top + 30, 200, 250, RGB(255, 0, 0));
            break;
        case 6:
            break;
        }
    }
}

void CALLBACK Warning(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
    if (!paused) {
        if (is_warning) {
            is_warning = 0;
        }
        else {
            is_warning = 1;
        }
        InvalidateRect(hWnd, NULL, FALSE);
    }
}

//stage 1 탄막 렌더링 함수
void Stage1_AtkRender(HWND hWnd, HDC hdc, HBITMAP hBitmap1, HBITMAP hBitmap2, HBITMAP hBitmap3, HBITMAP hBitmap4) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    HDC memDC;
    memDC = CreateCompatibleDC(hdc);

    switch (weaponPattern) { //패턴에 따라 탄막의 모양과 개수 설정
    case 1: //4~8초에 그려지는 적과 탄막
    case 2: //8~12초에 그려지는 적과 탄막
        weapon[0].width = RECT_SIZE * 4;
        weapon[0].length = RECT_SIZE * 4;
        SelectObject(memDC, hBitmap1);  //겐지: 120*120 픽셀
        BitBlt(hdc, weapon[0].x - (weapon[0].width / 2), weapon[0].y - (weapon[0].length / 2), weapon[0].width, weapon[0].length, memDC, 0, 0, SRCCOPY);

        for (int i = 1;i < 7;i++) {
            weapon[i].width = 20;
            weapon[i].length = 20;
            SelectObject(memDC, hBitmap2);  //표창: 20*20 픽셀
            BitBlt(hdc, weapon[i].x - (weapon[i].width / 2), weapon[i].y - (weapon[i].length / 2), weapon[i].width, weapon[i].length, memDC, 0, 0, SRCCOPY);
        }
        break;
    case 3: //12~16초에 그려지는 적과 탄막
    case 4: //16~20초에 그려지는 적과 탄막
        weapon[7].width = 400;
        weapon[7].length = 70;
        SelectObject(memDC, hBitmap3);  //한조 궁: 400*70 픽셀
        BitBlt(hdc, weapon[7].x - (weapon[7].width / 2), weapon[7].y - (weapon[7].length / 2), weapon[7].width, weapon[7].length, memDC, 0, 0, SRCCOPY);
        break;
    case 5: //20~24초에 그려지는 적과 탄막
    case 6: //24~28초에 그려지는 적과 탄막
        weapon[8].width = 180;
        weapon[8].length = 120;
        SelectObject(memDC, hBitmap4);  //겐지 돌진: 180*120 픽셀
        BitBlt(hdc, weapon[8].x - (weapon[8].width / 2), weapon[8].y - (weapon[8].length / 2), weapon[8].width, weapon[8].length, memDC, 0, 0, SRCCOPY);
        break;
    }
    DeleteDC(memDC);
}

//stage 1 탄막 패턴 함수
void Stage1_Atk_Pattern(HWND hWnd) {
    RECT rect;
    GetClientRect(hWnd, &rect);

    switch (weaponPattern) {
    case 1: //4~8초의 적, 탄막 움직임
        if (weapon[0].shooted == 0) { //탄막이 아직 안나갔으면 탄막이 나가는 곳 설정(겐지)
            weapon[0].x = rect.right - (weapon[0].width / 2);
            weapon[0].y = rect.bottom - (weapon[0].length / 2);
            weapon[0].shooted = 1;    //탄막이 나감.
        }
        for (int i = 1;i < 4;i++) {
            if (weapon[i].shooted == 0) {   //표창
                weapon[i].x = weapon[0].x - (weapon[0].width / 2);
                weapon[i].y = weapon[0].y;
                weapon[i].shooted = 1;    //탄막이 나감.
            }
        }
        for (int i = 1;i < 4;i++) {
            weapon[i].x -= 30;
        }
        weapon[1].y -= 4;
        weapon[3].y += 4;
        break;
    case 2: //8~12초의 적, 탄막 움직임
        for (int i = 4;i < 7;i++) {
            if (weapon[i].shooted == 0) {   //표창
                weapon[i].x = weapon[0].x - (weapon[0].width / 2);
                weapon[i].y = rect.bottom - (weapon[0].length / 2);
                weapon[i].shooted = 1;    //탄막이 나감.
            }
        }
        for (int i = 4;i < 7;i++) {
            weapon[i].x -= 35;
        }
        weapon[4].y -= 4;
        weapon[5].y -= 8;
        break;
    case 3: //12~16초의 적, 탄막 움직임
        if (weapon[7].shooted == 0) {
            weapon[7].x = rect.right - (weapon[7].width / 2);
            weapon[7].y = rect.bottom - (weapon[7].length / 2);
            weapon[7].shooted = 1;
        }
        weapon[7].x -= 30;
        break;
    case 4: //16~20초의 적, 탄막 움직임
        if (weapon[7].shooted == 0) {
            weapon[7].x = rect.left + (weapon[7].width / 2);
            weapon[7].y = rect.bottom - (weapon[7].length / 2);
            weapon[7].shooted = 1;
        }
        weapon[7].x += 30;
        break;
    case 5: //20~24초의 적, 탄막 움직임
        if (weapon[8].shooted == 0) {
            weapon[8].x = rect.right;
            weapon[8].y = rect.bottom - (weapon[8].length / 2);
            weapon[8].shooted = 1;
        }
        weapon[8].x -= 40;
        break;
    case 6: //24~28초의 적, 탄막 움직임
        if (weapon[8].shooted == 0) {
            weapon[8].x = rect.right;
            weapon[8].y = rect.bottom - (weapon[8].length / 2);
            weapon[8].shooted = 1;
        }
        weapon[8].x -= 50;
        break;
    }
}

//stage 3 탄막 랜더링 함수
void Stage3_AtkRender(HWND hWnd, HDC hdc, HBITMAP hBitmap1, HBITMAP hBitmap2){
    RECT rect;
    GetClientRect(hWnd, &rect);
    HDC memDC;
    memDC = CreateCompatibleDC(hdc);

    switch (weaponPattern) {
    case 1:
    case 2:
        weapon[0].width = 100;
        weapon[0].length = 100;
        SelectObject(memDC, hBitmap1);
        BitBlt(hdc, weapon[0].x - 50, weapon[0].y - 50, weapon[0].width, weapon[0].length, memDC, 0, 0, SRCCOPY);
    case 3:
    case 4:
        for (int i = 0; i < 5;i++) {
            weapon[i].width = 30;
            weapon[i].length = 30;
            SelectObject(memDC, hBitmap2);
            BitBlt(hdc, weapon[i].x - 15, weapon[i].y - 15, weapon[i].width, weapon[i].length, memDC, 0, 0, SRCCOPY);
        }
        break;
    case 5:
        break;
    case 6:
        weapon[0].width = 600;
        weapon[0].length = 800;
        Rectangle(hdc, weapon[0].x - 300, weapon[0].y - (rect.bottom / 2), weapon[0].x + 300, weapon[0].y + (rect.bottom / 2));
        break;
    }
    DeleteDC(memDC);
}

//stage 3 탄막 패턴 함수
void Stage3_Atk_Pattern(HWND hWnd) {
    RECT rect;
    GetClientRect(hWnd, &rect);

    switch (weaponPattern) {
    case 1:
        if (weapon[0].shooted == 0) {
            weapon[0].x = rect.left;
            weapon[0].y = rect.bottom - 50;
            weapon[0].shooted = 1;
        }
        weapon[0].x += 30;
        break;
    case 2:
        if (weapon[0].shooted == 0) {
            weapon[0].x = rect.right;
            weapon[0].y = rect.bottom - 50;
            weapon[0].shooted = 1;
        }
        weapon[0].x -= 30;

        break;
    case 3:
        for (int i = 0; i < 5;i++) {
            if (weapon[i].shooted == 0) {
                weapon[i].x = rect.left + 100; 
                weapon[i].y = rect.top + 200;
                weapon[i].shooted = 1;
            }
            weapon[0].y += 15;

            weapon[1].x += 3;
            weapon[1].y += 12;

            weapon[2].x += 6;
            weapon[2].y += 9;

            weapon[3].x += 9;
            weapon[3].y += 6;

            weapon[4].x += 12;
            weapon[4].y += 3;

            weapon[5].y += 15;
        }
        break;
    case 4:
        for (int i = 0; i < 5;i++) {
            if (weapon[i].shooted == 0) {
                weapon[i].x = rect.right - 100;
                weapon[i].y = rect.top + 200;
                weapon[i].shooted = 1;
            }
            weapon[0].y += 15;

            weapon[1].x -= 3;
            weapon[1].y += 12;

            weapon[2].x -= 6;
            weapon[2].y += 9;

            weapon[3].x -= 9;
            weapon[3].y += 6;

            weapon[4].x -= 12;
            weapon[4].y += 3;

            weapon[5].y -= 15;
        }
        break;
    case 5:
        break;
    case 6:
        if (weapon[0].shooted == 0) {
            weapon[0].x = rect.right - 330;
            weapon[0].y = rect.bottom;
            weapon[0].shooted = 1;
        }
        if (weapon[0].y > rect.bottom / 2) {
            weapon[0].y -= 100;
        }
        else
            weapon[0].y = rect.bottom / 2;
        break;
    }
}

//stage 2 탄막 렌더링 함수
void Stage2_AtkRender(HWND hWnd, HDC hdc, HBITMAP hBitmap1, HBITMAP hBitmap2, HBITMAP hBitmap3) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    HDC memDC;
    memDC = CreateCompatibleDC(hdc);

    switch (weaponPattern) { //패턴에 따라 탄막의 모양과 개수 설정
    case 1:
        for (int i = 0;i < 3;i++) { //60x60 픽셀
            weapon[i].width = 60;
            weapon[i].length = 60;

            SelectObject(memDC, hBitmap1);
            //Ellipse(hdc, weapon[i].x - 30, weapon[i].y - 30, weapon[i].x + 30, weapon[i].y + 30);
            BitBlt(hdc, weapon[i].x - 30, weapon[i].y - 30, weapon[i].width, weapon[i].length, memDC, 0, 0, SRCCOPY);
        }
        break;
    case 2: //100,100픽셀
        weapon[0].width = 100;
        weapon[0].length = 100;

        SelectObject(memDC, hBitmap1);
        //Ellipse(hdc, weapon[0].x - 50, weapon[0].y - 50, weapon[0].x + 50, weapon[0].y + 50);
        BitBlt(hdc, weapon[0].x - 50, weapon[0].y - 50, weapon[0].width, weapon[0].length, memDC, 0, 0, SRCCOPY);
        break;
    case 3:
    case 5: // 600x600 픽셀
        weapon[1].width = 600;
        weapon[1].length = 600;

        SelectObject(memDC, hBitmap2);
        //Ellipse(hdc, weapon[1].x - 300, weapon[1].y - 300, weapon[1].x + 300, weapon[1].y + 300);
        BitBlt(hdc, weapon[1].x - 300, weapon[1].y - 300, weapon[1].width, weapon[1].length, memDC, 0, 0, SRCCOPY);
        break;
    case 4: // 30x30 픽셀
        for (int i = 1;i < BAR_NUM;i++) {
            weapon[i].width = 30;
            weapon[i].length = 30;
            Ellipse(hdc, weapon[i].x - 15, weapon[i].y - 15, weapon[i].x + 15, weapon[i].y + 15);
        }
        break;
    case 6: // 600x800 픽셀
        weapon[0].width = 600;
        weapon[0].length = 800;

        SelectObject(memDC, hBitmap3);
        //Rectangle(hdc, weapon[0].x - 300, weapon[0].y - (rect.bottom / 2), weapon[0].x + 300, weapon[0].y + (rect.bottom / 2));
        BitBlt(hdc, weapon[0].x - 300, weapon[0].y - (rect.bottom / 2), weapon[0].width, weapon[0].length, memDC, 0, 0, SRCCOPY);
        break;
    }
    DeleteDC(memDC);
}

//stage 2 탄막 패턴 함수
void Stage2_Atk_Pattern(HWND hWnd) {
    RECT rect;
    GetClientRect(hWnd, &rect);

    switch (weaponPattern) {
    case 1:
        if (weapon[0].shooted == 0) { //탄막이 아직 안나갔으면 탄막이 나가는 곳 설정
            weapon[0].x = rect.right;
            weapon[0].y = rect.bottom - 50;
            weapon[0].shooted = 1;    //탄막이 나감.
        }
        if (weapon[1].shooted == 0) {
            weapon[1].x = rect.right;
            weapon[1].y = rect.bottom - 250;
            weapon[1].shooted = 1;
        }
        if (weapon[2].shooted == 0) {
            weapon[2].x = rect.right;
            weapon[2].y = rect.bottom - 450;
            weapon[2].shooted = 1;
        }
        for (int i = 0;i < 3;i++) {
            weapon[i].x -= 30;
        }
        break;

    case 2:
        if (weapon[0].shooted == 0) {
            weapon[0].x = player.x + (player.width / 2);
            weapon[0].y = rect.top;
            weapon[0].shooted = 1;
        }
        weapon[0].y += 40;
        break;

    case 3:
        if (weapon[1].shooted == 0) {
            weapon[1].x = rect.left;
            weapon[1].y = rect.top;
            weapon[1].shooted = 1;
        }
        weapon[1].x += 20;
        weapon[1].y += 20;
        break;

    case 4:
        for (int i = 1; i < BAR_NUM; i++) {
            if (weapon[i].shooted == 0) {
                weapon[i].x = rand() % rect.right;
                weapon[i].y = rect.top;
                weapon[i].shooted = 1;
            }
            weapon[i].y += 30;
        }
        break;
    case 5:
        if (weapon[1].shooted == 0) {
            weapon[1].x = rect.right;
            weapon[1].y = rect.top;
            weapon[1].shooted = 1;
        }
        if (weapon[1].x > rect.right / 2) {
            weapon[1].x -= 20;
            weapon[1].y += 20;
        }
        else {
            weapon[1].y += 40;
        }
    case 6:
        if (weapon[0].shooted == 0) {
            weapon[0].x = rect.left + 330;
            weapon[0].y = rect.bottom;
            weapon[0].shooted = 1;
        }
        if (weapon[0].y > rect.bottom / 2) {
            weapon[0].y -= 100;
        }
        else
            weapon[0].y = rect.bottom / 2;
        break;
    }
}

//스테이지 4 렌더링에서 쓰이는 변수(애니메이션)
int skeletonshooting = 0;
int gastshooting = 0;

//stage 4 탄막 렌더링 함수
void Stage4_AtkRender(HWND hWnd, HDC hdc, HBITMAP hBitmap1, HBITMAP hBitmap2, HBITMAP hBitmap3, HBITMAP hBitmap4, HBITMAP hBitmap5, HBITMAP hBitmap6, HBITMAP hBitmap7, HBITMAP hBitmap8) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    HDC memDC;
    memDC = CreateCompatibleDC(hdc);

    switch (weaponPattern) { //패턴에 따라 탄막의 모양과 개수 설정
    case 1:
        for (int i = 0;i < BAR_NUM;i++) {
            weapon[i].width = rect.right / BAR_NUM;
            weapon[i].length = rect.right / BAR_NUM;
            if (i % 2 == 0) {
                SelectObject(memDC, hBitmap1);
            }
            else SelectObject(memDC, hBitmap2);
            /*Rectangle(hdc,
                weapon[i].x - (rect.right / BAR_NUM / 2), weapon[i].y - (rect.right / BAR_NUM / 2),
                weapon[i].x + (rect.right / BAR_NUM / 2), weapon[i].y + (rect.right / BAR_NUM / 2));*/
            BitBlt(hdc, weapon[i].x - (rect.right / BAR_NUM / 2), weapon[i].y - (rect.right / BAR_NUM / 2), weapon[i].width, weapon[i].length, memDC, 0, 0, SRCCOPY);
        }
        break;
    case 2:
    case 3:
        if (skeletonshooting == 0) {
            SelectObject(memDC, hBitmap3);
        }
        else
            SelectObject(memDC, hBitmap4);
        //Rectangle(hdc, rect.right - 100, rect.bottom - 250, rect.right - 30, rect.bottom);   //스켈레톤 70x250 픽셀
        BitBlt(hdc, rect.right - 100, rect.bottom - 250, 70, 250, memDC, 0, 0, SRCCOPY);
        for (int i = 0;i < BAR_NUM;i++) {   //화살 40x15 픽셀
            weapon[i].width = 40;
            weapon[i].length = 15;
            SelectObject(memDC, hBitmap5);
            //Rectangle(hdc, weapon[i].x, weapon[i].y, weapon[i].x + 40, weapon[i].y + 15);
            BitBlt(hdc, weapon[i].x - (weapon[i].width / 2), weapon[i].y - (weapon[i].length / 2), weapon[i].width, weapon[i].length, memDC, 0, 0, SRCCOPY);
        }
        break;
    case 4:
        break;
    case 5:
    case 6:
        if (gastshooting == 0) {
            SelectObject(memDC, hBitmap7);
        }
        else {
            SelectObject(memDC, hBitmap6);
        }
        //Rectangle(hdc, rect.right / 2 - 100, rect.top + 30, rect.right / 2 + 100, rect.top + 280);  //가스트 200x250 픽셀
        BitBlt(hdc, rect.right / 2 - 100, rect.top + 30, 200, 250, memDC, 0, 0, SRCCOPY);

       
        for (int i = 0; i < BAR_NUM; i++) { //파이어볼 50x50 픽셀
            weapon[i].width = 50;
            weapon[i].length = 50;
            SelectObject(memDC, hBitmap8);
            //Rectangle(hdc, weapon[i].x, weapon[i].y, weapon[i].x + 50, weapon[i].y + 50);
            BitBlt(hdc, weapon[i].x - (weapon[i].width / 2), weapon[i].y - (weapon[i].length / 2), weapon[i].width, weapon[i].length, memDC, 0, 0, SRCCOPY);
        }
        break;
    }
    DeleteDC(memDC);
}

//========================스테이지 4에서만 쓰이는 타이머와 전역 변수 시작
int arrow, fireball;    //탄막 인덱스
int stage4_case2start, stage4_case5start = 0;   //스테이지 4의 case2, case5 시작 여부(탄막 패턴 함수는 30ms마다 반복되니 지역변수가 아닌 전역변수로 만듦)

//스켈레톤이 쏠 준비자세 취하는 타이머 함수(애니메이션)
void CALLBACK stage4_readyforshoot(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
    if (!paused) {
        if (skeletonshooting == 0) {
            skeletonshooting = 1;
        }
        else skeletonshooting = 0;
    }
}

//stage 4 스켈레톤이 화살을 1초마다 날리는 타이머 함수
void CALLBACK stage4_ShootArrow(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
    if (!paused) {
        arrow++;
        InvalidateRect(hWnd, NULL, FALSE);
    }
}

//가스트가 파이어볼을 쏠 준비자세 취하는 타이머 함수(애니메이션)
void CALLBACK stage4_readyforfire(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
    if (!paused) {
        if (gastshooting == 0) {
            gastshooting = 1;
        }
        else gastshooting = 0;
    }
}

//stage 4 가스트가 파이어볼을 1초마다 날리는 타이머 함수
void CALLBACK stage4_ShootFireball(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
    if (!paused) {
        fireball++;
        InvalidateRect(hWnd, NULL, FALSE);
    }
}
//========================스테이지 4에서만 쓰이는 타이머와 전역 변수 종료

//stage 4 탄막 패턴 함수
void Stage4_Atk_Pattern(HWND hWnd) {
    RECT rect;
    GetClientRect(hWnd, &rect);

    switch (weaponPattern) {
    case 1:
        for (int i = 0; i < BAR_NUM; i++) {
            if (weapon[i].shooted == 0) {
                weapon[i].x = (rect.right / BAR_NUM) * i + (rect.right / BAR_NUM / 2);
                weapon[i].y = rect.top + 30;
                weapon[i].shooted = 1;
            }
            for (int i = 0; i < 5;i++) {
                weapon[2 * i].y += 2;
                weapon[2 * i + 1].y += 3;
            }
        }
        break;
    case 2:
    case 3:
        if (stage4_case2start == 0) {   //case2가 처음 시작되면 탄막 인덱스, 처음 나갈 좌표, 타이머 설정
            arrow = 0;
            for (int i = 0;i < BAR_NUM;i++) {
                weapon[i].x = -100;
                weapon[i].y = -100;
                weapon[i].shooted = 0;
            }
            SetTimer(hWnd, 6, 1000, stage4_ShootArrow);
            SetTimer(hWnd, 7, 500, stage4_readyforshoot);
            stage4_case2start = 1;
        }
        for (int i = 0; i < arrow; i++) {
            if (weapon[i].shooted == 0) {
                weapon[i].x = rect.right - 100;
                weapon[i].y = rect.bottom - 125;
                weapon[i].shooted = 1;
            }
        }
        for (int i = 0; i < arrow; i++) {
            weapon[i].x -= 20;
            if (player.y + 5 < weapon[i].y) {
                weapon[i].y -= 7;
            }
            else if (player.y - 5 > weapon[i].y) {
                weapon[i].y += 7;
            }
        }
        break;
    case 4:
        //case 2와 3에서 사용한 타이머와 변수를 초기화
        KillTimer(hWnd, 6);
        KillTimer(hWnd, 7);
        stage4_case2start = 0;
        arrow = 0;
        skeletonshooting = 0;
        break;
    case 5:
    case 6:
        if (stage4_case5start == 0) {   //case 5가 처음 시작되면 탄막 인덱스, 처음 나갈 좌표, 타이머 설정
            fireball = 0;
            gastshooting = 0;
            for (int i = 0;i < BAR_NUM;i++) {
                weapon[i].x = -100;
                weapon[i].y = -100;
                weapon[i].shooted = 0;
            }
            SetTimer(hWnd, 8, 1000, stage4_ShootFireball);
            SetTimer(hWnd, 9, 500, stage4_readyforfire);
            stage4_case5start = 1;
        }
        for (int i = 0; i < fireball; i++) {
            if (weapon[i].shooted == 0) {
                weapon[i].x = rect.right / 2 - 60;
                weapon[i].y = rect.top + 175;
                weapon[i].shooted = 1;
            }
        }
        for (int i = 0; i < fireball; i++) {
            if (weapon[i].x + 10 < player.x) {
                weapon[i].x += 10;
            }
            else if (weapon[i].x - 10 > player.x) {
                weapon[i].x -= 10;
            }
            weapon[i].y += 10;
        }
        break;
    }
}


//탄막 이동 타이머 함수
void CALLBACK Atk_Move(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
    if (!paused) {
        if (stage == 1) {
            Stage1_Atk_Pattern(hWnd);
        }
        else if (stage == 2) {
            Stage2_Atk_Pattern(hWnd);
        }
        else if (stage == 3) {
            Stage3_Atk_Pattern(hWnd);
        }
        else if (stage == 4) {
            Stage4_Atk_Pattern(hWnd);
        }
        InvalidateRect(hWnd, NULL, FALSE);
    }
}

//탄막 간 시간간격
void CALLBACK Atk_Interval(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
    if (!paused) {
        if ((stage == 4 && weaponPattern == 2) ||
            (stage == 4 && weaponPattern == 5)) {
            //이때는 shooted를 0으로 초기화 안시킴.(이어지게)
        }
        else {
            for (int i = 0;i < BAR_NUM;i++) {
                weapon[i].shooted = 0;
                weapon[i].length = 0;
                weapon[i].width = 0;
            }
        }
        weaponPattern += 1;

        if (weaponPattern == 7) {   //스테이지 하나가 끝났다면
            if (stage == 4) {//스테이지 4 case 6에서 사용한 타이머, 변수 초기화
                KillTimer(hWnd, 8);
                KillTimer(hWnd, 9);
                stage4_case5start = 0;
                gastshooting = 0;
                fireball = 0;
            }
            KillTimer(hWnd, 5); //위험 사각형 표시 타이머 삭제
            AtkInit();  //탄막 초기화
            SetTimer(hWnd, 5, 2000, Warning);   //위험 사각형 표시 타이머 다시 생성
           
            stage = rand() % 4 + 1; //스테이지 1~4 랜덤 선택
        }
    }
}
//-----------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
void CALLBACK TimeMeasureProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);    //시간 측정 함수

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpszCmdParam, _In_ int nCmdShow) {
    HWND hWnd;
    MSG Message;
    WNDCLASSEX WndClass;

    g_hInst = hInstance;

    WndClass.cbSize = sizeof(WndClass);
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    WndClass.lpfnWndProc = (WNDPROC)WndProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = hInstance;
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hCursor = LoadCursor(NULL, IDI_APPLICATION);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClass.lpszMenuName = NULL;
    WndClass.lpszClassName = lpszClass;
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&WndClass);

    hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, 1024 + marginX, 768 + marginY, NULL, (HMENU)NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&Message, NULL, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    return (int)Message.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
    HDC hdc, mdc;
    PAINTSTRUCT ps;
    HBITMAP hBitmap;
    RECT rect;  //윈도우 창 rect

    static int current_hp;  //현재 HP(충돌 체크 할 때 충돌하면 이 변수를 -1 하면 됨)

    static HBITMAP hBitmap_ZedShuriken, hBitmap_meteor, hBitmap_lazer;  //2스테 비트맵
    static HBITMAP hBitmap_stone, hBitmap_soil, hBitmap_skeleton, hBitmap_skeletonshoot, hBitmap_arrow, hBitmap_gast, hBitmap_gastshoot, hBitmap_fireball;  //4스테 비트맵
    static HBITMAP hNinjaBitmap, hShurikenBitmap, hDragonStrikeBitmap, hNinjaRushBitmap;  //1스테 비트맵
    static HBITMAP ParanoiaBitmap, JettBarrageBitmap;   //3스테 비트맵

    switch (iMessage) {
    case WM_CREATE:
        GetClientRect(hWnd, &rect); // 윈도우 화면 얻어오기
        player.x = rect.right / 2 - player.width / 2;  // 플레이어 초기 x 좌표 
        player.y = rect.bottom - player.height;  // 플레이어 초기 y 좌표

        current_hp = MAX_HP;    //초기 HP
        gamestart = 0;          //게임 시작 안함
        paused = 0;             //게임 퍼즈 안함
        gameover = 0;           //게임 오버 안함
        srand((unsigned int)time(NULL));
        AtkInit();

        //처음에 나오는 스테이지 설정
        //stage = rand() % 4 + 1; //스테이지 1~4 랜덤 선택
        stage = 1;

        //스테이지 2 비트맵
        hBitmap_ZedShuriken = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));
        hBitmap_meteor = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
        hBitmap_lazer = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP3));

        //스테이지 4 비트맵
        hBitmap_stone = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP4));
        hBitmap_soil = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP5));
        hBitmap_skeleton = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP6));
        hBitmap_skeletonshoot = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP7));
        hBitmap_arrow = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP8));
        hBitmap_gast = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP9));
        hBitmap_gastshoot = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP10));
        hBitmap_fireball = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP11));

        //스테이지 1 비트맵
        hNinjaBitmap = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP12));
        hShurikenBitmap = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP13));
        hDragonStrikeBitmap = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP14));
        hNinjaRushBitmap = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP15));

        //스테이지 3 비트맵
        ParanoiaBitmap = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP16));
        JettBarrageBitmap = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP17));

        PlaySound(TEXT("GameSound.wav"), NULL, SND_ASYNC | SND_LOOP);
        break;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &rect); // 윈도우 화면 얻어오기

        //더블 버퍼링
        mdc = CreateCompatibleDC(hdc); //메모리 DC 만들기
        hBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom); //메모리 DC와 연결할 비트맵 만들기
        SelectObject(mdc, (HBITMAP)hBitmap); //메모리 DC와 비트맵 연결하기
        HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 0)); //배경색 브러시 생성(배경 RGB값에 따라 변경 가능)
        FillRect(mdc, &rect, bgBrush); //메모리 DC 화면이 비어있기 때문에 화면 가득히 사각형을 그려 배경색으로 설정하기
        //-----------------------------------------그리기 시작
        if (gameover == 0) {
            if (gamestart == 0) {   //게임 시작 안했으면 게임 타이틀 나타내기
                game_Title(hWnd, mdc);
            }
            else{
                if (paused == 1) {  //퍼즈면 퍼즈 화면 띄우기
                    Paused(hWnd, mdc);
                }
                else {             //퍼즈가 아니면 스테이지 나타내기
                    if (stage == 1) {
                        Stage1_SetWarningRects(hWnd, mdc);
                        Stage1_AtkRender(hWnd, mdc, hNinjaBitmap, hShurikenBitmap, hDragonStrikeBitmap, hNinjaRushBitmap);
                    }

                    else if (stage == 2) {
                        Stage2_SetWarningRects(hWnd, mdc);
                        Stage2_AtkRender(hWnd, mdc, hBitmap_ZedShuriken, hBitmap_meteor, hBitmap_lazer);
                    }

                    else if (stage == 3) {
                        Stage3_SetWarningRects(hWnd, mdc);
                        Stage3_AtkRender(hWnd, mdc, ParanoiaBitmap, JettBarrageBitmap);
                    }

                    else if (stage == 4) {
                        Stage4_SetWarningRects(hWnd, mdc);
                        Stage4_AtkRender(hWnd, mdc, hBitmap_stone, hBitmap_soil, hBitmap_skeleton, hBitmap_skeletonshoot, hBitmap_arrow, hBitmap_gast, hBitmap_gastshoot, hBitmap_fireball);
                    }
                    //플레이어 그리기 (player.jumpzero를 뺀 이유는 플레이어가 올라갈 때 또는 내려올 때 사각형을 표현하기 위해) 
                    Rectangle(mdc, player.x, player.y - player.jumpzero, player.x + player.width, player.y + player.height - player.jumpzero);

                    //hp칸 그리기
                    Draw_HP(mdc, current_hp);

                    //시간(점수) 나타내기
                    Draw_timeRect(mdc, playTime);
                    //무적 표시
                    DrawInvincibleStatus(mdc); 

                }
            }
        }
        else if (gameover == 1) {
            Gameover_Screen(hWnd, mdc);
            gamestart = 0;
        }
        
        //-----------------------------------------그리기 종료
        //마지막에 메모리 DC의 내용을 화면 DC로 복사
        BitBlt(hdc, 0, 0, rect.right, rect.bottom, mdc, 0, 0, SRCCOPY);
        DeleteObject(bgBrush); //배경색 브러시 삭제
        DeleteDC(mdc); //생성한 메모리 DC 삭제
        DeleteObject(hBitmap);	//생성한 비트맵 삭제
        EndPaint(hWnd, &ps);
        break;

    case WM_KEYDOWN:
        switch (wParam) {
        case VK_RETURN:
            if (gamestart == 0) {   //엔터키를 누르면 게임 시작
                SetTimer(hWnd, 1, 30, NULL);                // 플레이어 이동 타이머
                SetTimer(hWnd, 2, 1000, TimeMeasureProc);   // TIME_MEASURE: 시간 측정 함수

                SetTimer(hWnd, 3, 4000, Atk_Interval);      //탄막 생성 타이머 실행
                SetTimer(hWnd, 4, 30, Atk_Move);            //탄막 이동
                SetTimer(hWnd, 5, 2000, Warning);           //경고 사각형 표시/비표시
                gamestart = 1;
                InvalidateRect(hWnd, NULL, TRUE);
            }
            break;
        case 'P':
        case 'p':
            if (gamestart == 1) {
                paused = !paused;
                InvalidateRect(hWnd, NULL, TRUE);
            }
            break;
        case 'G':
        case 'g':
            if (gamestart && !paused) {
                invincible = !invincible; // 무적 상태 토글
            }
            break;
        case VK_LEFT:  // 왼쪽 키
            if (gamestart && !paused) {
                player.move_left = 1;
            }
            break;

        case VK_RIGHT:  // 오른쪽 키
            if (gamestart && !paused) {
                player.move_right = 1;
            }
            break;

        case VK_SPACE:  // 스페이스바
            if (gamestart && !paused) {
                if (!player.jumping) {  // 점프 중이 아닐 때
                    player.jumping = 1;  // 점프  설정  -> 1이 점프 뛸때
                    player.jumping_up = 1;  // 점프 상승  설정
                    player.current_jump = 0;  // 현재 점프 높이 초기화
                }
                else if (player.jumping && !player.double_jump) {  // 2단 점프 조건
                    player.double_jump = 1;
                    player.jumping_up = 1;
                    player.current_jump = player.jumpzero;   //1단점프에서 띈 높이를 유지하면서 2단점프 하기
                }
            }
            break;
        }
        break;

    case WM_KEYUP:
        switch (wParam) {
        case VK_LEFT:
            if (gamestart && !paused) {
                player.move_left = 0;
            }
            break;
        case VK_RIGHT:
            if (gamestart && !paused) {
                player.move_right = 0;
            }
            break;
        }
        break;

    case WM_TIMER:
        if (gamestart) {    //충돌 체크
            CollisionCheck(hWnd, &current_hp);
        }

        if (player.move_left) {  // 왼쪽으로 이동 중일 때
            player.x -= MOVE_STEP;  // 왼쪽으로 이동
        }
        if (player.move_right) {  // 오른쪽으로 이동 중일 때
            player.x += MOVE_STEP;  // 오른쪽으로 이동
        }
        if (player.jumping) {  // 점프 중일 때
            if (player.jumping_up) {  // 점프 상승 중일 때
                player.jumpzero += JUMP_UP_SPEED;  // 점프 zero 증가
                if (player.jumpzero >= player.current_jump + JUMP_HEIGHT) {  // 최대 점프 높이에 도달했을 때
                    player.jumping_up = 0;  // 점프 상승 종료
                }
            }
            else {  // 점프 하강 중일 때
                player.jumpzero -= JUMP_DOWN_SPEED;  // 점프 오프셋 감소
                if (player.jumpzero <= 0) {  // 바닥에 도달했을 때
                    player.jumpzero = 0;  // 점프 오프셋 초기화
                    player.jumping = 0;  // 점프 종료
                    player.double_jump = 0;  // 이단 점프 플래그 해제
                }
            }
        }
        Prevent_Leave_Screen(hWnd);
        InvalidateRect(hWnd, NULL, FALSE);
        break;

    case WM_DESTROY:
        KillTimer(hWnd, 1);
        KillTimer(hWnd, 2);

        KillTimer(hWnd, 3);
        KillTimer(hWnd, 4);
        KillTimer(hWnd, 5);
        //stage2 비트맵 삭제
        DeleteObject(hBitmap_ZedShuriken);
        DeleteObject(hBitmap_meteor);
        DeleteObject(hBitmap_lazer);

        //stage4 비트맵 삭제
        DeleteObject(hBitmap_stone);
        DeleteObject(hBitmap_soil);
        DeleteObject(hBitmap_skeleton);
        DeleteObject(hBitmap_skeletonshoot);
        DeleteObject(hBitmap_arrow);
        DeleteObject(hBitmap_gast);
        DeleteObject(hBitmap_gastshoot);
        DeleteObject(hBitmap_fireball);

        //stage1 비트맵 삭제
        DeleteObject(hNinjaBitmap);
        DeleteObject(hShurikenBitmap);
        DeleteObject(hDragonStrikeBitmap);
        DeleteObject(hNinjaRushBitmap);

        //stage3 비트맵 삭제
        DeleteObject(ParanoiaBitmap);
        DeleteObject(JettBarrageBitmap);

        PlaySound(NULL, 0, 0);

        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, iMessage, wParam, lParam);
    }

    return 0;
}

//시간 측정 함수
void CALLBACK TimeMeasureProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
    if (gamestart && !paused) {
        playTime++; // 시간 증가
    }
}