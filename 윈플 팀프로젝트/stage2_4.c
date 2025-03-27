#include <windows.h>  
#include <tchar.h>    
#include <string.h>
#include <time.h>
#include <mmsystem.h>
#include "resource.h"
#pragma comment(lib,"winmm.lib")

//Ÿ�̸� 1: �÷��̾� �̵� Ÿ�̸�
//Ÿ�̸� 2: �ð� ��Ÿ���� Ÿ�̸�
//Ÿ�̸� 3: stage 2 ź�� �� �ð� ���� Ÿ�̸�
//Ÿ�̸� 4: stage 2 ź�� ���� Ÿ�̸�
//Ÿ�̸� 5: stage 2 ��� �簢�� ǥ�� Ÿ�̸�
#define marginX 16
#define marginY 39

#define RECT_SIZE 30           //�÷��̾� ũ��
#define MOVE_STEP 10           // ->,<- �̵��ӵ�
#define JUMP_HEIGHT 100        //���� ����
#define JUMP_UP_SPEED 10       // ����Ű ������ �ö󰡴� �ӵ�
#define JUMP_DOWN_SPEED 10     // �����ϰ� ���ΰ��� �������� �ӵ�

#define MAX_HP 5                // �ִ� HP��

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Final Survival";

typedef struct {
    int x;  // x ��ǥ
    int y;  // y ��ǥ
    int width;  // ����
    int height;  // ����

    int jumping;  // ���� ������ ����
    int jumpzero;  // ������ �Ҷ� ��ȭ�� ������� �����ϸ� ���ΰ��� �����۶� jumpzero�� ���� Ŀ���ٰ� ���ΰ��� ���� �������� �۾������� ���� 

    int jumping_up;  // ���� ��� ������ ����
    int double_jump;  // �̴� ���� ����

    int move_left;  // ���� �̵� ����
    int move_right;  // ������ �̵� ����

    int current_jump;  // ���� ���� ����
} Player;

// �÷��̾� �ʱ�ȭ
Player player = { 0, 0, RECT_SIZE, RECT_SIZE, 0, 0, 0, 0, 0, 0, 0 };

//�ð� �ʱ�ȭ
int playTime = 0;

int gamestart;   //���� ���� ����
int paused;      //���� ���� ����
int gameover;   //���� ���� ����

//�������� ����
int stage = 0;

// ���� Ÿ��Ʋ �׸��� �Լ�
void game_Title(HWND hWnd, HDC hdc) {
    RECT rect;
    GetClientRect(hWnd, &rect); // ������ ȭ�� ������

    // �簢�� �׸���
    RECT titlerect = { rect.right / 2 - 250, 100, rect.right / 2 + 250, 200 };
    HBRUSH titlebrush = CreateSolidBrush(RGB(0, 255, 200));
    FillRect(hdc, &titlerect, titlebrush);
    DeleteObject(titlebrush);

    // �ؽ�Ʈ �׸���
    HFONT font = CreateFont(72, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial"));
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 0, 0));
    TextOut(hdc, rect.right / 2 - 200, 115, TEXT("Final Survival"), lstrlen(TEXT("Final Survival")));

    font = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial"));
    oldFont = (HFONT)SelectObject(hdc, font);
    SetTextColor(hdc, RGB(128, 128, 128)); // ȸ�� �ؽ�Ʈ ����
    TextOut(hdc, rect.right / 2 - 100, 450, TEXT("Press Enter to start"), lstrlen(TEXT("Press Enter to start")));

    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

// ���� ȭ�� �׸��� �Լ�
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
    SetTextColor(hdc, RGB(128, 128, 128)); // ȸ�� �ؽ�Ʈ ����
    TextOut(hdc, rect.right / 2 - 100, 450, TEXT("Press P to unpause"), lstrlen(TEXT("Press P to unpause")));

    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

//���� ���� ȭ�� �׸��� �Լ�
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

    RECT ScoreRect = { rect.right / 2 - 75, 300,  rect.right / 2 + 75, 400 }; // ���� ��µ� �κ�
    TCHAR ScoreStr[35];
    wsprintf(ScoreStr, L"Score : %d", playTime);
    DrawText(hdc, ScoreStr, -1, &ScoreRect, DT_SINGLELINE | DT_LEFT);  //���ڿ��� ���� �ڵ����� ���
}

// �÷��̾ ȭ�� ������ ����� �ʰ� ���ִ� �Լ�
void Prevent_Leave_Screen(HWND hWnd) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    if (player.x < 0) player.x = 0;
    if (player.x + player.width > rect.right) player.x = rect.right - player.width;  // ȭ�� ������ ����
    if (player.y + player.height > rect.bottom) player.y = rect.bottom - player.height;  // ȭ�� �Ʒ��� ����
}

// HP �׸��� �Լ�
void Draw_HP(HDC hdc, int current_hp) {
    RECT hp[MAX_HP];

    //HP �׸� ��ġ ����
    for (int i = 0;i < MAX_HP;i++) {
        hp[i].left = 10 + (40 * i);
        hp[i].top = 20;
        hp[i].right = 50 + (40 * i);
        hp[i].bottom = 40;
    }

    //���� HP�� ���� HP �׸���
    for (int i = 0;i < current_hp;i++) {
        Rectangle(hdc, hp[i].left, hp[i].top, hp[i].right, hp[i].bottom);
    }
}

//�ð�(����)�׸��� �Լ�
void Draw_timeRect(HDC hdc, int playTime) {
    RECT timeRect = { 10, 50, 100, 80 }; // �ð� ��µ� �κ�
    TCHAR timeStr[20];

    wsprintf(timeStr, L"Score : %d", playTime);

    DrawText(hdc, timeStr, -1, &timeRect, DT_SINGLELINE | DT_LEFT);  //���ڿ��� ���� �ڵ����� ���
}


//--------------------------------------------------------------
#define BAR_NUM 10
#define WARNING_TIME 2000 // 2�� ���� ���

typedef struct {    //ź�� ����ü
    int x, y;       //ź���� "�߾�" ��ǥ
    int width, length;  //ź���� ����, ���� ����
    int shooted;      //ź���� �������� �ȳ������� ���� (�������� 1, �ȳ������� 0)
}Barrage;

Barrage weapon[BAR_NUM];   //ź�� ����
int weaponPattern;  //ź�� ����
int invincible = 0; //���� ����
int is_warning; //��� �簢���� ��������� ����

// ���� ���� ���� Ÿ�̸� üũ
void CALLBACK Invincible(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
    invincible = 0; // ���� ���� ����
    KillTimer(hWnd, 10);
}

//�浹üũ (WM_TIMER���� �� �Լ��� ȣ���մϴ�)
void CollisionCheck(HWND hWnd, int* current_hp) {
    RECT playerRect = { player.x, player.y - player.jumpzero, player.x + player.width, player.y + player.height - player.jumpzero };
    for (int i = 0; i < 10; i++) {
        RECT weaponRect = { weapon[i].x - (weapon[i].width / 2), weapon[i].y - (weapon[i].length / 2), weapon[i].x + (weapon[i].width / 2), weapon[i].y + (weapon[i].length / 2) };
        if (playerRect.left < weaponRect.right &&
            playerRect.right > weaponRect.left &&
            playerRect.top < weaponRect.bottom &&
            playerRect.bottom > weaponRect.top) {
            if (!invincible) { // ���� ���°� �ƴ� ���� HP ����
                (*current_hp)--;
                if (*current_hp <= 0) {
                    gameover = 1;
                    gamestart = 0;
                    InvalidateRect(hWnd, NULL, TRUE);
                }
                invincible = 1; // ���� ���·� ����
                SetTimer(hWnd, 10, 3000, Invincible); // 3�� �� ���� ���� ���� Ÿ�̸� ����
            }
        }
    }
}

void DrawInvincibleStatus(HDC hdc) { //������� ǥ��
    if (invincible) {
        RECT invincibleRect = { 900, 20, 1000, 50 }; // ���� ��ܿ� ��ġ
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 0, 0)); // ������ �ؽ�Ʈ
        DrawText(hdc, TEXT("���� ON"), -1, &invincibleRect, DT_SINGLELINE | DT_LEFT);
    }
}

//ź�� �ʱ�ȭ �Լ�
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

//��� �簢�� �׸��� �Լ�
void DrawWarningRect(HDC hdc, int x, int y, int width, int height, COLORREF color) {
    HPEN pen = CreatePen(PS_DOT, 1, color);
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

    Rectangle(hdc, x, y, x + width, y + height);

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pen);
}

//1�������� ��� �簢�� ���� �Լ�
void Stage1_SetWarningRects(HWND hWnd, HDC hdc) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    if (is_warning == 1) {
        switch (weaponPattern + 1) {
        case 1: //2~4�ʿ� ������ ���� ���
            DrawWarningRect(hdc, rect.right - 120, rect.bottom - 120, rect.right, rect.bottom, RGB(255, 0, 0));
            break;
        case 2: //6~8�ʿ� ������ ���� ���
            break;
        case 3: //10~12�ʿ� ������ ���� ���
            DrawWarningRect(hdc, rect.right - 70, rect.bottom - 70, 70, 70, RGB(255, 0, 0));
            break;
        case 4: //14~16�ʿ� ������ ���� ���
            DrawWarningRect(hdc, rect.left, rect.bottom - 70, 70, 70, RGB(255, 0, 0));
            break;
        case 5: //18~20�ʿ� ������ ���� ���
            DrawWarningRect(hdc, rect.right - 120, rect.bottom - 120, 120, 120, RGB(255, 0, 0));
            break;
        case 6: //22~24�ʿ� ������ ���� ���
            DrawWarningRect(hdc, rect.right - 120, rect.bottom - 120, 120, 120, RGB(255, 0, 0));
            break;
        }
    }
}

//3�������� ��� �簢�� ���� �Լ�
void Stage3_SetWarningRects(HWND hWnd, HDC hdc) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    if (is_warning == 1) {
        switch (weaponPattern + 1) {
        case 1: //2~4�ʿ� ������ ���� ���
            DrawWarningRect(hdc, 0, rect.bottom - 100, 100, 100, RGB(255, 0, 0));
            break;
        case 2: //6~8�ʿ� ������ ���� ���
            DrawWarningRect(hdc, rect.right - 100, rect.bottom - 100, 100, 100, RGB(255, 0, 0));
            break;
        case 3: //10~12�ʿ� ������ ���� ���
            DrawWarningRect(hdc, rect.left + 100, rect.top + 200, 30, 30, RGB(255, 0, 0));
            break;
        case 4: //14~16�ʿ� ������ ���� ���
            DrawWarningRect(hdc, rect.right - 100, rect.top + 200, 30, 30, RGB(255, 0, 0));
            break;
        case 5: //18~20�ʿ� ������ ���� ���
            break;
        case 6: //22~24�ʿ� ������ ���� ���
            DrawWarningRect(hdc, 0, rect.top, rect.right - 630, rect.bottom, RGB(0, 0, 255));
            break;
        }
    }
}

//2�������� ��� �簢�� ���� �Լ�
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

//4�������� ��� �簢�� ���� �Լ�
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

//stage 1 ź�� ������ �Լ�
void Stage1_AtkRender(HWND hWnd, HDC hdc, HBITMAP hBitmap1, HBITMAP hBitmap2, HBITMAP hBitmap3, HBITMAP hBitmap4) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    HDC memDC;
    memDC = CreateCompatibleDC(hdc);

    switch (weaponPattern) { //���Ͽ� ���� ź���� ���� ���� ����
    case 1: //4~8�ʿ� �׷����� ���� ź��
    case 2: //8~12�ʿ� �׷����� ���� ź��
        weapon[0].width = RECT_SIZE * 4;
        weapon[0].length = RECT_SIZE * 4;
        SelectObject(memDC, hBitmap1);  //����: 120*120 �ȼ�
        BitBlt(hdc, weapon[0].x - (weapon[0].width / 2), weapon[0].y - (weapon[0].length / 2), weapon[0].width, weapon[0].length, memDC, 0, 0, SRCCOPY);

        for (int i = 1;i < 7;i++) {
            weapon[i].width = 20;
            weapon[i].length = 20;
            SelectObject(memDC, hBitmap2);  //ǥâ: 20*20 �ȼ�
            BitBlt(hdc, weapon[i].x - (weapon[i].width / 2), weapon[i].y - (weapon[i].length / 2), weapon[i].width, weapon[i].length, memDC, 0, 0, SRCCOPY);
        }
        break;
    case 3: //12~16�ʿ� �׷����� ���� ź��
    case 4: //16~20�ʿ� �׷����� ���� ź��
        weapon[7].width = 400;
        weapon[7].length = 70;
        SelectObject(memDC, hBitmap3);  //���� ��: 400*70 �ȼ�
        BitBlt(hdc, weapon[7].x - (weapon[7].width / 2), weapon[7].y - (weapon[7].length / 2), weapon[7].width, weapon[7].length, memDC, 0, 0, SRCCOPY);
        break;
    case 5: //20~24�ʿ� �׷����� ���� ź��
    case 6: //24~28�ʿ� �׷����� ���� ź��
        weapon[8].width = 180;
        weapon[8].length = 120;
        SelectObject(memDC, hBitmap4);  //���� ����: 180*120 �ȼ�
        BitBlt(hdc, weapon[8].x - (weapon[8].width / 2), weapon[8].y - (weapon[8].length / 2), weapon[8].width, weapon[8].length, memDC, 0, 0, SRCCOPY);
        break;
    }
    DeleteDC(memDC);
}

//stage 1 ź�� ���� �Լ�
void Stage1_Atk_Pattern(HWND hWnd) {
    RECT rect;
    GetClientRect(hWnd, &rect);

    switch (weaponPattern) {
    case 1: //4~8���� ��, ź�� ������
        if (weapon[0].shooted == 0) { //ź���� ���� �ȳ������� ź���� ������ �� ����(����)
            weapon[0].x = rect.right - (weapon[0].width / 2);
            weapon[0].y = rect.bottom - (weapon[0].length / 2);
            weapon[0].shooted = 1;    //ź���� ����.
        }
        for (int i = 1;i < 4;i++) {
            if (weapon[i].shooted == 0) {   //ǥâ
                weapon[i].x = weapon[0].x - (weapon[0].width / 2);
                weapon[i].y = weapon[0].y;
                weapon[i].shooted = 1;    //ź���� ����.
            }
        }
        for (int i = 1;i < 4;i++) {
            weapon[i].x -= 30;
        }
        weapon[1].y -= 4;
        weapon[3].y += 4;
        break;
    case 2: //8~12���� ��, ź�� ������
        for (int i = 4;i < 7;i++) {
            if (weapon[i].shooted == 0) {   //ǥâ
                weapon[i].x = weapon[0].x - (weapon[0].width / 2);
                weapon[i].y = rect.bottom - (weapon[0].length / 2);
                weapon[i].shooted = 1;    //ź���� ����.
            }
        }
        for (int i = 4;i < 7;i++) {
            weapon[i].x -= 35;
        }
        weapon[4].y -= 4;
        weapon[5].y -= 8;
        break;
    case 3: //12~16���� ��, ź�� ������
        if (weapon[7].shooted == 0) {
            weapon[7].x = rect.right - (weapon[7].width / 2);
            weapon[7].y = rect.bottom - (weapon[7].length / 2);
            weapon[7].shooted = 1;
        }
        weapon[7].x -= 30;
        break;
    case 4: //16~20���� ��, ź�� ������
        if (weapon[7].shooted == 0) {
            weapon[7].x = rect.left + (weapon[7].width / 2);
            weapon[7].y = rect.bottom - (weapon[7].length / 2);
            weapon[7].shooted = 1;
        }
        weapon[7].x += 30;
        break;
    case 5: //20~24���� ��, ź�� ������
        if (weapon[8].shooted == 0) {
            weapon[8].x = rect.right;
            weapon[8].y = rect.bottom - (weapon[8].length / 2);
            weapon[8].shooted = 1;
        }
        weapon[8].x -= 40;
        break;
    case 6: //24~28���� ��, ź�� ������
        if (weapon[8].shooted == 0) {
            weapon[8].x = rect.right;
            weapon[8].y = rect.bottom - (weapon[8].length / 2);
            weapon[8].shooted = 1;
        }
        weapon[8].x -= 50;
        break;
    }
}

//stage 3 ź�� ������ �Լ�
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

//stage 3 ź�� ���� �Լ�
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

//stage 2 ź�� ������ �Լ�
void Stage2_AtkRender(HWND hWnd, HDC hdc, HBITMAP hBitmap1, HBITMAP hBitmap2, HBITMAP hBitmap3) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    HDC memDC;
    memDC = CreateCompatibleDC(hdc);

    switch (weaponPattern) { //���Ͽ� ���� ź���� ���� ���� ����
    case 1:
        for (int i = 0;i < 3;i++) { //60x60 �ȼ�
            weapon[i].width = 60;
            weapon[i].length = 60;

            SelectObject(memDC, hBitmap1);
            //Ellipse(hdc, weapon[i].x - 30, weapon[i].y - 30, weapon[i].x + 30, weapon[i].y + 30);
            BitBlt(hdc, weapon[i].x - 30, weapon[i].y - 30, weapon[i].width, weapon[i].length, memDC, 0, 0, SRCCOPY);
        }
        break;
    case 2: //100,100�ȼ�
        weapon[0].width = 100;
        weapon[0].length = 100;

        SelectObject(memDC, hBitmap1);
        //Ellipse(hdc, weapon[0].x - 50, weapon[0].y - 50, weapon[0].x + 50, weapon[0].y + 50);
        BitBlt(hdc, weapon[0].x - 50, weapon[0].y - 50, weapon[0].width, weapon[0].length, memDC, 0, 0, SRCCOPY);
        break;
    case 3:
    case 5: // 600x600 �ȼ�
        weapon[1].width = 600;
        weapon[1].length = 600;

        SelectObject(memDC, hBitmap2);
        //Ellipse(hdc, weapon[1].x - 300, weapon[1].y - 300, weapon[1].x + 300, weapon[1].y + 300);
        BitBlt(hdc, weapon[1].x - 300, weapon[1].y - 300, weapon[1].width, weapon[1].length, memDC, 0, 0, SRCCOPY);
        break;
    case 4: // 30x30 �ȼ�
        for (int i = 1;i < BAR_NUM;i++) {
            weapon[i].width = 30;
            weapon[i].length = 30;
            Ellipse(hdc, weapon[i].x - 15, weapon[i].y - 15, weapon[i].x + 15, weapon[i].y + 15);
        }
        break;
    case 6: // 600x800 �ȼ�
        weapon[0].width = 600;
        weapon[0].length = 800;

        SelectObject(memDC, hBitmap3);
        //Rectangle(hdc, weapon[0].x - 300, weapon[0].y - (rect.bottom / 2), weapon[0].x + 300, weapon[0].y + (rect.bottom / 2));
        BitBlt(hdc, weapon[0].x - 300, weapon[0].y - (rect.bottom / 2), weapon[0].width, weapon[0].length, memDC, 0, 0, SRCCOPY);
        break;
    }
    DeleteDC(memDC);
}

//stage 2 ź�� ���� �Լ�
void Stage2_Atk_Pattern(HWND hWnd) {
    RECT rect;
    GetClientRect(hWnd, &rect);

    switch (weaponPattern) {
    case 1:
        if (weapon[0].shooted == 0) { //ź���� ���� �ȳ������� ź���� ������ �� ����
            weapon[0].x = rect.right;
            weapon[0].y = rect.bottom - 50;
            weapon[0].shooted = 1;    //ź���� ����.
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

//�������� 4 ���������� ���̴� ����(�ִϸ��̼�)
int skeletonshooting = 0;
int gastshooting = 0;

//stage 4 ź�� ������ �Լ�
void Stage4_AtkRender(HWND hWnd, HDC hdc, HBITMAP hBitmap1, HBITMAP hBitmap2, HBITMAP hBitmap3, HBITMAP hBitmap4, HBITMAP hBitmap5, HBITMAP hBitmap6, HBITMAP hBitmap7, HBITMAP hBitmap8) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    HDC memDC;
    memDC = CreateCompatibleDC(hdc);

    switch (weaponPattern) { //���Ͽ� ���� ź���� ���� ���� ����
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
        //Rectangle(hdc, rect.right - 100, rect.bottom - 250, rect.right - 30, rect.bottom);   //���̷��� 70x250 �ȼ�
        BitBlt(hdc, rect.right - 100, rect.bottom - 250, 70, 250, memDC, 0, 0, SRCCOPY);
        for (int i = 0;i < BAR_NUM;i++) {   //ȭ�� 40x15 �ȼ�
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
        //Rectangle(hdc, rect.right / 2 - 100, rect.top + 30, rect.right / 2 + 100, rect.top + 280);  //����Ʈ 200x250 �ȼ�
        BitBlt(hdc, rect.right / 2 - 100, rect.top + 30, 200, 250, memDC, 0, 0, SRCCOPY);

       
        for (int i = 0; i < BAR_NUM; i++) { //���̾ 50x50 �ȼ�
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

//========================�������� 4������ ���̴� Ÿ�̸ӿ� ���� ���� ����
int arrow, fireball;    //ź�� �ε���
int stage4_case2start, stage4_case5start = 0;   //�������� 4�� case2, case5 ���� ����(ź�� ���� �Լ��� 30ms���� �ݺ��Ǵ� ���������� �ƴ� ���������� ����)

//���̷����� �� �غ��ڼ� ���ϴ� Ÿ�̸� �Լ�(�ִϸ��̼�)
void CALLBACK stage4_readyforshoot(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
    if (!paused) {
        if (skeletonshooting == 0) {
            skeletonshooting = 1;
        }
        else skeletonshooting = 0;
    }
}

//stage 4 ���̷����� ȭ���� 1�ʸ��� ������ Ÿ�̸� �Լ�
void CALLBACK stage4_ShootArrow(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
    if (!paused) {
        arrow++;
        InvalidateRect(hWnd, NULL, FALSE);
    }
}

//����Ʈ�� ���̾�� �� �غ��ڼ� ���ϴ� Ÿ�̸� �Լ�(�ִϸ��̼�)
void CALLBACK stage4_readyforfire(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
    if (!paused) {
        if (gastshooting == 0) {
            gastshooting = 1;
        }
        else gastshooting = 0;
    }
}

//stage 4 ����Ʈ�� ���̾�� 1�ʸ��� ������ Ÿ�̸� �Լ�
void CALLBACK stage4_ShootFireball(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
    if (!paused) {
        fireball++;
        InvalidateRect(hWnd, NULL, FALSE);
    }
}
//========================�������� 4������ ���̴� Ÿ�̸ӿ� ���� ���� ����

//stage 4 ź�� ���� �Լ�
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
        if (stage4_case2start == 0) {   //case2�� ó�� ���۵Ǹ� ź�� �ε���, ó�� ���� ��ǥ, Ÿ�̸� ����
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
        //case 2�� 3���� ����� Ÿ�̸ӿ� ������ �ʱ�ȭ
        KillTimer(hWnd, 6);
        KillTimer(hWnd, 7);
        stage4_case2start = 0;
        arrow = 0;
        skeletonshooting = 0;
        break;
    case 5:
    case 6:
        if (stage4_case5start == 0) {   //case 5�� ó�� ���۵Ǹ� ź�� �ε���, ó�� ���� ��ǥ, Ÿ�̸� ����
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


//ź�� �̵� Ÿ�̸� �Լ�
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

//ź�� �� �ð�����
void CALLBACK Atk_Interval(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
    if (!paused) {
        if ((stage == 4 && weaponPattern == 2) ||
            (stage == 4 && weaponPattern == 5)) {
            //�̶��� shooted�� 0���� �ʱ�ȭ �Ƚ�Ŵ.(�̾�����)
        }
        else {
            for (int i = 0;i < BAR_NUM;i++) {
                weapon[i].shooted = 0;
                weapon[i].length = 0;
                weapon[i].width = 0;
            }
        }
        weaponPattern += 1;

        if (weaponPattern == 7) {   //�������� �ϳ��� �����ٸ�
            if (stage == 4) {//�������� 4 case 6���� ����� Ÿ�̸�, ���� �ʱ�ȭ
                KillTimer(hWnd, 8);
                KillTimer(hWnd, 9);
                stage4_case5start = 0;
                gastshooting = 0;
                fireball = 0;
            }
            KillTimer(hWnd, 5); //���� �簢�� ǥ�� Ÿ�̸� ����
            AtkInit();  //ź�� �ʱ�ȭ
            SetTimer(hWnd, 5, 2000, Warning);   //���� �簢�� ǥ�� Ÿ�̸� �ٽ� ����
           
            stage = rand() % 4 + 1; //�������� 1~4 ���� ����
        }
    }
}
//-----------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
void CALLBACK TimeMeasureProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);    //�ð� ���� �Լ�

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
    RECT rect;  //������ â rect

    static int current_hp;  //���� HP(�浹 üũ �� �� �浹�ϸ� �� ������ -1 �ϸ� ��)

    static HBITMAP hBitmap_ZedShuriken, hBitmap_meteor, hBitmap_lazer;  //2���� ��Ʈ��
    static HBITMAP hBitmap_stone, hBitmap_soil, hBitmap_skeleton, hBitmap_skeletonshoot, hBitmap_arrow, hBitmap_gast, hBitmap_gastshoot, hBitmap_fireball;  //4���� ��Ʈ��
    static HBITMAP hNinjaBitmap, hShurikenBitmap, hDragonStrikeBitmap, hNinjaRushBitmap;  //1���� ��Ʈ��
    static HBITMAP ParanoiaBitmap, JettBarrageBitmap;   //3���� ��Ʈ��

    switch (iMessage) {
    case WM_CREATE:
        GetClientRect(hWnd, &rect); // ������ ȭ�� ������
        player.x = rect.right / 2 - player.width / 2;  // �÷��̾� �ʱ� x ��ǥ 
        player.y = rect.bottom - player.height;  // �÷��̾� �ʱ� y ��ǥ

        current_hp = MAX_HP;    //�ʱ� HP
        gamestart = 0;          //���� ���� ����
        paused = 0;             //���� ���� ����
        gameover = 0;           //���� ���� ����
        srand((unsigned int)time(NULL));
        AtkInit();

        //ó���� ������ �������� ����
        //stage = rand() % 4 + 1; //�������� 1~4 ���� ����
        stage = 1;

        //�������� 2 ��Ʈ��
        hBitmap_ZedShuriken = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));
        hBitmap_meteor = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
        hBitmap_lazer = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP3));

        //�������� 4 ��Ʈ��
        hBitmap_stone = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP4));
        hBitmap_soil = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP5));
        hBitmap_skeleton = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP6));
        hBitmap_skeletonshoot = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP7));
        hBitmap_arrow = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP8));
        hBitmap_gast = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP9));
        hBitmap_gastshoot = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP10));
        hBitmap_fireball = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP11));

        //�������� 1 ��Ʈ��
        hNinjaBitmap = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP12));
        hShurikenBitmap = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP13));
        hDragonStrikeBitmap = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP14));
        hNinjaRushBitmap = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP15));

        //�������� 3 ��Ʈ��
        ParanoiaBitmap = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP16));
        JettBarrageBitmap = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP17));

        PlaySound(TEXT("GameSound.wav"), NULL, SND_ASYNC | SND_LOOP);
        break;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &rect); // ������ ȭ�� ������

        //���� ���۸�
        mdc = CreateCompatibleDC(hdc); //�޸� DC �����
        hBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom); //�޸� DC�� ������ ��Ʈ�� �����
        SelectObject(mdc, (HBITMAP)hBitmap); //�޸� DC�� ��Ʈ�� �����ϱ�
        HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 0)); //���� �귯�� ����(��� RGB���� ���� ���� ����)
        FillRect(mdc, &rect, bgBrush); //�޸� DC ȭ���� ����ֱ� ������ ȭ�� ������ �簢���� �׷� �������� �����ϱ�
        //-----------------------------------------�׸��� ����
        if (gameover == 0) {
            if (gamestart == 0) {   //���� ���� �������� ���� Ÿ��Ʋ ��Ÿ����
                game_Title(hWnd, mdc);
            }
            else{
                if (paused == 1) {  //����� ���� ȭ�� ����
                    Paused(hWnd, mdc);
                }
                else {             //��� �ƴϸ� �������� ��Ÿ����
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
                    //�÷��̾� �׸��� (player.jumpzero�� �� ������ �÷��̾ �ö� �� �Ǵ� ������ �� �簢���� ǥ���ϱ� ����) 
                    Rectangle(mdc, player.x, player.y - player.jumpzero, player.x + player.width, player.y + player.height - player.jumpzero);

                    //hpĭ �׸���
                    Draw_HP(mdc, current_hp);

                    //�ð�(����) ��Ÿ����
                    Draw_timeRect(mdc, playTime);
                    //���� ǥ��
                    DrawInvincibleStatus(mdc); 

                }
            }
        }
        else if (gameover == 1) {
            Gameover_Screen(hWnd, mdc);
            gamestart = 0;
        }
        
        //-----------------------------------------�׸��� ����
        //�������� �޸� DC�� ������ ȭ�� DC�� ����
        BitBlt(hdc, 0, 0, rect.right, rect.bottom, mdc, 0, 0, SRCCOPY);
        DeleteObject(bgBrush); //���� �귯�� ����
        DeleteDC(mdc); //������ �޸� DC ����
        DeleteObject(hBitmap);	//������ ��Ʈ�� ����
        EndPaint(hWnd, &ps);
        break;

    case WM_KEYDOWN:
        switch (wParam) {
        case VK_RETURN:
            if (gamestart == 0) {   //����Ű�� ������ ���� ����
                SetTimer(hWnd, 1, 30, NULL);                // �÷��̾� �̵� Ÿ�̸�
                SetTimer(hWnd, 2, 1000, TimeMeasureProc);   // TIME_MEASURE: �ð� ���� �Լ�

                SetTimer(hWnd, 3, 4000, Atk_Interval);      //ź�� ���� Ÿ�̸� ����
                SetTimer(hWnd, 4, 30, Atk_Move);            //ź�� �̵�
                SetTimer(hWnd, 5, 2000, Warning);           //��� �簢�� ǥ��/��ǥ��
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
                invincible = !invincible; // ���� ���� ���
            }
            break;
        case VK_LEFT:  // ���� Ű
            if (gamestart && !paused) {
                player.move_left = 1;
            }
            break;

        case VK_RIGHT:  // ������ Ű
            if (gamestart && !paused) {
                player.move_right = 1;
            }
            break;

        case VK_SPACE:  // �����̽���
            if (gamestart && !paused) {
                if (!player.jumping) {  // ���� ���� �ƴ� ��
                    player.jumping = 1;  // ����  ����  -> 1�� ���� �۶�
                    player.jumping_up = 1;  // ���� ���  ����
                    player.current_jump = 0;  // ���� ���� ���� �ʱ�ȭ
                }
                else if (player.jumping && !player.double_jump) {  // 2�� ���� ����
                    player.double_jump = 1;
                    player.jumping_up = 1;
                    player.current_jump = player.jumpzero;   //1���������� �� ���̸� �����ϸ鼭 2������ �ϱ�
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
        if (gamestart) {    //�浹 üũ
            CollisionCheck(hWnd, &current_hp);
        }

        if (player.move_left) {  // �������� �̵� ���� ��
            player.x -= MOVE_STEP;  // �������� �̵�
        }
        if (player.move_right) {  // ���������� �̵� ���� ��
            player.x += MOVE_STEP;  // ���������� �̵�
        }
        if (player.jumping) {  // ���� ���� ��
            if (player.jumping_up) {  // ���� ��� ���� ��
                player.jumpzero += JUMP_UP_SPEED;  // ���� zero ����
                if (player.jumpzero >= player.current_jump + JUMP_HEIGHT) {  // �ִ� ���� ���̿� �������� ��
                    player.jumping_up = 0;  // ���� ��� ����
                }
            }
            else {  // ���� �ϰ� ���� ��
                player.jumpzero -= JUMP_DOWN_SPEED;  // ���� ������ ����
                if (player.jumpzero <= 0) {  // �ٴڿ� �������� ��
                    player.jumpzero = 0;  // ���� ������ �ʱ�ȭ
                    player.jumping = 0;  // ���� ����
                    player.double_jump = 0;  // �̴� ���� �÷��� ����
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
        //stage2 ��Ʈ�� ����
        DeleteObject(hBitmap_ZedShuriken);
        DeleteObject(hBitmap_meteor);
        DeleteObject(hBitmap_lazer);

        //stage4 ��Ʈ�� ����
        DeleteObject(hBitmap_stone);
        DeleteObject(hBitmap_soil);
        DeleteObject(hBitmap_skeleton);
        DeleteObject(hBitmap_skeletonshoot);
        DeleteObject(hBitmap_arrow);
        DeleteObject(hBitmap_gast);
        DeleteObject(hBitmap_gastshoot);
        DeleteObject(hBitmap_fireball);

        //stage1 ��Ʈ�� ����
        DeleteObject(hNinjaBitmap);
        DeleteObject(hShurikenBitmap);
        DeleteObject(hDragonStrikeBitmap);
        DeleteObject(hNinjaRushBitmap);

        //stage3 ��Ʈ�� ����
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

//�ð� ���� �Լ�
void CALLBACK TimeMeasureProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
    if (gamestart && !paused) {
        playTime++; // �ð� ����
    }
}