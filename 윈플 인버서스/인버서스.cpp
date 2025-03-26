#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <time.h>
#include <cmath>

#define MARGIN_X 16          //중간고사 윈메인
#define MARGIN_Y 39          //중간고사 윈메인

#define BOARD_SIZE_EASY 10   //쉬움보드
#define BOARD_SIZE_NOMAL 15  //노말 보드
#define BOARD_SIZE_HARD 20   //하드 보드

#define TILE_SIZE 50         //검은색타일크기
#define PLAYER_SIZE TILE_SIZE //주인공 크기
#define ENEMY_SIZE 50        //적크기
#define BULLET_SIZE 25        // 총알크기
#define MOVE_INTERVAL 10      //적 이동주기
#define BULLET_INTERVAL 10    // 총알 이동주기
#define SCORE_INTERVAL 1000   //점수 1초당 오르는거
#define MAX_SCORE 50000       // 최대 점수
#define MAX_HP 5              //HP칸

#define MAX_BULLETS 6           //총 총알개수
#define RELOAD_INTERVAL 1000    //1초로 장전
#define RELOAD_TIMER_ID 6       //장탄수
#define RESPAWN_TIME 10         // 적소환 시간

#define MAX_ENEMIES 10          //최대 적
#define SPAWN_INTERVAL 3000     //새로운 적이 생성되는 주기
#define SHAKE_DURATION 500      //화면 흔들림 지속시간
#define SHAKE_INTENSITY 10      //흔들림 강도
#define SPAWN_ALERT_DURATION 2000   //적소환 알림 (파란색)
#define RESPAWN_DELAY 3000          //적이 죽고 새로운 적이 등장시간3초
#define IMMUNITY_DURATION 3000      // 부활후 3초무적
#define MAX_TRAIL 5                 //총알 그라데이션
#define COMBO_INTERVAL 3000        // 10초 이내에 적을 처치하면 콤보가 증가

int combo = 0;
DWORD lastKillTime = 0;
HINSTANCE g_hInst;
LPCTSTR lpszClass = L"윈플 인버서스 숙제";
LPCTSTR lpszWindowName = L"윈플 인버서스 숙제";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

typedef struct {
    int x;
    int y;
} Pos;

typedef struct {
    Pos pos;
    int dx;
    int dy;
    BOOL active;
    Pos trail[MAX_TRAIL];
    int trailLength;
} Bullet;

typedef struct {
    Pos pos;
    BOOL active;
    DWORD lastHitTime;
    int health;
    BOOL alert; 
} Enemy;

typedef struct {
    Pos pos;
    BOOL active;
} Obstacle;

Pos playerPos;
int playerSpeedX = 0;
int playerSpeedY = 0;
int** board;
Bullet bullets[MAX_BULLETS];
Enemy enemies[MAX_ENEMIES];
UINT_PTR playerTimer = 1;
UINT_PTR bulletTimer = 2;
UINT_PTR enemyMoveTimer = 3;
UINT_PTR scoreTimer = 4;
UINT_PTR enemySpawnTimer = 7;
UINT_PTR shakeTimer = 8;

int hp = MAX_HP;
int score = 0;
BOOL paused = FALSE;
int bulletCount = MAX_BULLETS;
UINT_PTR reloadTimer = 5;

#define OBSTACLE_COUNT_HARD 4
Obstacle obstacles[OBSTACLE_COUNT_HARD];

BOOL shaking = FALSE;
DWORD shakeStartTime = 0;
BOOL playerImmune = FALSE;
DWORD respawnStartTime[MAX_ENEMIES];

int currentBoardSize = BOARD_SIZE_HARD;
int obstacleCount = OBSTACLE_COUNT_HARD;
BOOL godMode = FALSE;
int specialBulletCount = 0;



// 보드 초기화
void InitBoard(int size) {
    if (board != NULL) {
        for (int i = 0; i < currentBoardSize; i++) {
            free(board[i]);
        }
        free(board);
    }
    board = (int**)malloc(size * sizeof(int*));
    for (int i = 0; i < size; i++) {
        board[i] = (int*)malloc(size * sizeof(int));
        for (int j = 0; j < size; j++) {
            board[i][j] = 0;
        }
    }
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int newY = playerPos.y / TILE_SIZE + dy;
            int newX = playerPos.x / TILE_SIZE + dx;
            if (newY >= 0 && newY < size && newX >= 0 && newX < size) {
                board[newY][newX] = 1;
            }
        }
    }
}

// 장애물 초기화
void InitObstacles(int size) {
    srand((unsigned int)time(NULL));
    for (int i = 0; i < obstacleCount; i++) {
        int x, y;
        do {
            x = rand() % size;
            y = rand() % size;
        } while (board[y][x] == 1 || (x == playerPos.x / TILE_SIZE && y == playerPos.y / TILE_SIZE));
        obstacles[i].pos.x = x;
        obstacles[i].pos.y = y;
        obstacles[i].active = TRUE;
        board[y][x] = 2;
    }
}

// 적 초기화
void InitEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = FALSE;
        enemies[i].alert = FALSE;
    }
}

// 총알 초기화
void InitBullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = FALSE;
    }
}

// 보드 그리기
void DrawBoard(HDC hdc, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            HBRUSH brush;
            if (board[i][j] == 1)
                brush = CreateSolidBrush(RGB(255, 255, 255));
            else if (board[i][j] == 2)
                brush = CreateSolidBrush(RGB(255, 0, 0));
            else
                brush = CreateSolidBrush(RGB(0, 0, 0));

            RECT rect = { j * TILE_SIZE, i * TILE_SIZE, (j + 1) * TILE_SIZE, (i + 1) * TILE_SIZE };
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);
        }
    }
}

// 플레이어 그리기
void DrawPlayer(HDC hdc) {
    HBRUSH brush = CreateSolidBrush(RGB(128, 128, 128));
    RECT rect = {
        playerPos.x - (PLAYER_SIZE - TILE_SIZE) / 2,
        playerPos.y - (PLAYER_SIZE - TILE_SIZE) / 2,
        playerPos.x + TILE_SIZE + (PLAYER_SIZE - TILE_SIZE) / 2,
        playerPos.y + TILE_SIZE + (PLAYER_SIZE - TILE_SIZE) / 2
    };
    FillRect(hdc, &rect, brush);

    HBRUSH bulletBrush = CreateSolidBrush(RGB(255, 0, 0));
    int centerX = playerPos.x + TILE_SIZE / 2;
    int centerY = playerPos.y + TILE_SIZE / 2;
    int bulletRadius = 20;

    for (int i = 0; i < bulletCount; i++) {
        double angle = (i * 2 * 3.14159265358979323846) / bulletCount;
        int bulletX = centerX + (int)(bulletRadius * cos(angle));
        int bulletY = centerY - (int)(bulletRadius * sin(angle));
        Ellipse(hdc, bulletX - BULLET_SIZE / 2, bulletY - BULLET_SIZE / 2, bulletX + BULLET_SIZE / 2, bulletY + BULLET_SIZE / 2);
    }

    DeleteObject(bulletBrush);
    DeleteObject(brush);
}

// 적 그리기
void DrawEnemies(HDC hdc) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            HBRUSH brush = CreateSolidBrush(RGB(0, 255, 0));
            RECT rect = {
                enemies[i].pos.x * TILE_SIZE + (TILE_SIZE - ENEMY_SIZE) / 2,
                enemies[i].pos.y * TILE_SIZE + (TILE_SIZE - ENEMY_SIZE) / 2,
                enemies[i].pos.x * TILE_SIZE + (TILE_SIZE + ENEMY_SIZE) / 2,
                enemies[i].pos.y * TILE_SIZE + (TILE_SIZE + ENEMY_SIZE) / 2
            };
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);
        }
    }
}

// 총알 그리기
void DrawBullets(HDC hdc) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            HBRUSH brush;
            if (bullets[i].dx != 0 || bullets[i].dy != 0) {
                brush = CreateSolidBrush(RGB(255, 0, 0)); // 일반 총알은 빨간색
            }
            else {
                brush = CreateSolidBrush(RGB(255, 255, 0)); // 특수 총알은 노란색
            }
            RECT rect = {
                bullets[i].pos.x * TILE_SIZE + (TILE_SIZE - BULLET_SIZE) / 2,
                bullets[i].pos.y * TILE_SIZE + (TILE_SIZE - BULLET_SIZE) / 2,
                bullets[i].pos.x * TILE_SIZE + (TILE_SIZE + BULLET_SIZE) / 2,
                bullets[i].pos.y * TILE_SIZE + (TILE_SIZE + BULLET_SIZE) / 2
            };
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);
        }
    }
}

// 콤보 업데이트
void UpdateCombo() {
    DWORD currentTime = GetTickCount();
    if (currentTime - lastKillTime <= COMBO_INTERVAL) {
        combo++;
    }
    else {
        combo = 1;
    }
    lastKillTime = currentTime;
}

// UI 그리기
void DrawUI(HDC hdc) {
    TCHAR buffer[256];

    _stprintf_s(buffer, _countof(buffer), _T("점수: %d"), score);
    TextOut(hdc, 10, 50, buffer, _tcslen(buffer));

    _stprintf_s(buffer, _countof(buffer), _T("콤보: %d"), combo);
    TextOut(hdc, 10, 70, buffer, _tcslen(buffer));
}

// 체력 그리기
void DrawHP(HDC hdc, int current_hp) {
    RECT hp[MAX_HP];

    for (int i = 0; i < MAX_HP; i++) {
        hp[i].left = 10 + (40 * i);
        hp[i].top = 10;
        hp[i].right = 50 + (40 * i);
        hp[i].bottom = 30;
    }

    for (int i = 0; i < MAX_HP; i++) {
        HBRUSH brush;
        if (i < current_hp) {
            brush = CreateSolidBrush(RGB(255, 0, 0));
        }
        else {
            brush = CreateSolidBrush(RGB(128, 128, 128));
        }
        FillRect(hdc, &hp[i], brush);
        DeleteObject(brush);
    }

    if (current_hp == 0) {
        TCHAR message[256];
        _stprintf_s(message, _T(" 점수: %d"), score);
        MessageBox(NULL, message, _T("게임 종료"), MB_OK);
        PostQuitMessage(0);
    }
}

// 플레이어 이동
void MovePlayer() {
    int newX = playerPos.x + playerSpeedX;
    int newY = playerPos.y + playerSpeedY;

    if (newX >= 0 && newX <= (currentBoardSize * TILE_SIZE - PLAYER_SIZE) && newY >= 0 && newY <= (currentBoardSize * TILE_SIZE - PLAYER_SIZE)) {
        int leftTileX = newX / TILE_SIZE;
        int rightTileX = (newX + PLAYER_SIZE - 1) / TILE_SIZE;
        int topTileY = newY / TILE_SIZE;
        int bottomTileY = (newY + PLAYER_SIZE - 1) / TILE_SIZE;

        if (board[topTileY][leftTileX] != 0 && board[topTileY][leftTileX] != 2 &&
            board[topTileY][rightTileX] != 0 && board[topTileY][rightTileX] != 2 &&
            board[bottomTileY][leftTileX] != 0 && board[bottomTileY][leftTileX] != 2 &&
            board[bottomTileY][rightTileX] != 0 && board[bottomTileY][rightTileX] != 2) {
            playerPos.x = newX;
            playerPos.y = newY;
        }
    }
}

// 적 이동
void MoveEnemies() {
    if (!playerImmune) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                int oldX = enemies[i].pos.x;
                int oldY = enemies[i].pos.y;

                if (enemies[i].pos.x < playerPos.x / TILE_SIZE) enemies[i].pos.x++;
                else if (enemies[i].pos.x > playerPos.x / TILE_SIZE) enemies[i].pos.x--;

                if (enemies[i].pos.y < playerPos.y / TILE_SIZE) enemies[i].pos.y++;
                else if (enemies[i].pos.y > playerPos.y / TILE_SIZE) enemies[i].pos.y--;

                if (oldY >= 0 && oldY < currentBoardSize && oldX >= 0 && oldX < currentBoardSize) {
                    if (board[oldY][oldX] == 1) {
                        board[oldY][oldX] = 0;
                    }
                }
            }
        }
    }
}

// 총알 발사
void Shoot(int dx, int dy) {
    if (bulletCount > 0) {
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].active) {
                bullets[i].pos = { playerPos.x / TILE_SIZE, playerPos.y / TILE_SIZE };
                bullets[i].dx = dx;
                bullets[i].dy = dy;
                bullets[i].active = TRUE;
                bulletCount--;
                break;
            }
        }
    }
}

// 폭발 그리기
void DrawExplosion(HDC hdc, int centerX, int centerY) {
    HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
    int radius = 30;
    Ellipse(hdc, centerX - radius, centerY - radius, centerX + radius, centerY + radius);
    Sleep(1);
    DeleteObject(brush);
}

// 폭발 제거
void ClearExplosion(HDC hdc, int centerX, int centerY) {
    HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int explosionX = centerX + dx * TILE_SIZE;
            int explosionY = centerY + dy * TILE_SIZE;
            Ellipse(hdc, explosionX, explosionY, explosionX + TILE_SIZE, explosionY + TILE_SIZE);
        }
    }
    DeleteObject(brush);
}

// 충돌 확인
BOOL CheckCollision(int x, int y) {
    if (x >= 0 && x < currentBoardSize && y >= 0 && y < currentBoardSize) {
        return (board[y][x] == 2);
    }
    return TRUE;
}

// 총알 경로 업데이트
void UpdateBulletTrail(Bullet* bullet) {
    if (bullet->trailLength < MAX_TRAIL) {
        bullet->trail[bullet->trailLength] = bullet->pos;
        bullet->trailLength++;
    }
    else {
        for (int i = 1; i < MAX_TRAIL; i++) {
            bullet->trail[i - 1] = bullet->trail[i];
        }
        bullet->trail[MAX_TRAIL - 1] = bullet->pos;
    }
}

// 총알 경로 그리기
void DrawBulletTrail(HDC hdc, Bullet* bullet) {
    for (int i = 0; i < bullet->trailLength; i++) {
        float t = (float)i / (bullet->trailLength - 1);

        int red = 255;
        int green = (int)(t * 255);
        int blue = 0;

        HBRUSH brush = CreateSolidBrush(RGB(red, green, blue));
        RECT rect = {
            bullet->trail[i].x * TILE_SIZE + (TILE_SIZE - BULLET_SIZE) / 2,
            bullet->trail[i].y * TILE_SIZE + (TILE_SIZE - BULLET_SIZE) / 2,
            bullet->trail[i].x * TILE_SIZE + (TILE_SIZE + BULLET_SIZE) / 2,
            bullet->trail[i].y * TILE_SIZE + (TILE_SIZE + BULLET_SIZE) / 2
        };
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
    }
}

// 총알 업데이트
void UpdateBullets(HDC hdc, HWND hWnd) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            UpdateBulletTrail(&bullets[i]); 

            int newX = bullets[i].pos.x + bullets[i].dx;
            int newY = bullets[i].pos.y + bullets[i].dy;

            if (CheckCollision(newX, newY)) {
                bullets[i].active = FALSE;
            }
            else {
                int stepX = (newX > bullets[i].pos.x) ? 1 : ((newX < bullets[i].pos.x) ? -1 : 0);
                int stepY = (newY > bullets[i].pos.y) ? 1 : ((newY < bullets[i].pos.y) ? -1 : 0);

                int currentX = bullets[i].pos.x;
                int currentY = bullets[i].pos.y;

                while (currentX != newX || currentY != newY) {
                    currentX += stepX;
                    currentY += stepY;

                    if (currentX >= 0 && currentX < currentBoardSize && currentY >= 0 && currentY < currentBoardSize) {
                        board[currentY][currentX] = 1;
                    }
                }

                bullets[i].pos.x = newX;
                bullets[i].pos.y = newY;

                DrawBulletTrail(hdc, &bullets[i]); 

                for (int j = 0; j < MAX_ENEMIES; j++) {
                    if (enemies[j].active && newX == enemies[j].pos.x && newY == enemies[j].pos.y) {
                        bullets[i].active = FALSE;
                        bulletCount++;
                        enemies[j].active = FALSE;
                        respawnStartTime[j] = GetTickCount(); 
                        enemies[j].lastHitTime = GetTickCount();

                        UpdateCombo(); 

                        // 적을 죽였을 때 특수 총알 획득
                        specialBulletCount++;

                        for (int dy = -1; dy <= 1; dy++) {
                            for (int dx = -1; dx <= 1; dx++) {
                                int updatedX = newX + dx;
                                int updatedY = newY + dy;
                                if (updatedX >= 0 && updatedX < currentBoardSize && updatedY >= 0 && updatedY < currentBoardSize) {
                                    board[updatedY][updatedX] = 1;
                                    DrawExplosion(hdc, updatedX * TILE_SIZE, updatedY * TILE_SIZE);

                                    for (int k = 0; k < MAX_ENEMIES; k++) {
                                        if (enemies[k].active && enemies[k].pos.x == updatedX && enemies[k].pos.y == updatedY) {
                                            enemies[k].active = FALSE;
                                            respawnStartTime[k] = GetTickCount(); 
                                        }
                                    }
                                }
                            }
                        }

                        shaking = TRUE;
                        shakeStartTime = GetTickCount();
                        SetTimer(hWnd, shakeTimer, 10, NULL);
                        break;
                    }
                }
            }
        }
    }
}

// 플레이어 리스폰
void RespawnPlayer(HWND hWnd) {
    playerPos.x = currentBoardSize / 2 * TILE_SIZE;
    playerPos.y = currentBoardSize / 2 * TILE_SIZE;
    playerImmune = TRUE;
    SetTimer(hWnd, 200, IMMUNITY_DURATION, NULL); 
    InitBoard(currentBoardSize);
}

// 적 업데이트
void UpdateEnemies(HWND hWnd) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active && GetTickCount() - respawnStartTime[i] >= RESPAWN_DELAY) {
            enemies[i].alert = TRUE;
            SetTimer(hWnd, i + 100, SPAWN_ALERT_DURATION, NULL); 
        }
    }
}

// 플레이어 폭발
void ExplodePlayer(HWND hWnd, int centerX, int centerY) {
    HDC hdc = GetDC(hWnd);
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int explosionX = centerX + dx;
            int explosionY = centerY + dy;
            if (explosionX >= 0 && explosionX < currentBoardSize && explosionY >= 0 && explosionY < currentBoardSize) {
                DrawExplosion(hdc, explosionX * TILE_SIZE, explosionY * TILE_SIZE);
                if (board[explosionY][explosionX] == 1) {
                    board[explosionY][explosionX] = 0;
                }
            }
        }
    }
    shaking = TRUE;
    shakeStartTime = GetTickCount();
    SetTimer(hWnd, shakeTimer, 10, NULL);
    ReleaseDC(hWnd, hdc);
}

// 플레이어와 적 충돌 확인
void CheckPlayerEnemyColl(HWND hWnd) {
    if (godMode) {
        return; 
    }
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active &&
            abs(enemies[i].pos.x * TILE_SIZE - playerPos.x) < TILE_SIZE &&
            abs(enemies[i].pos.y * TILE_SIZE - playerPos.y) < TILE_SIZE) {
            if (!playerImmune) {
                ExplodePlayer(hWnd, playerPos.x / TILE_SIZE, playerPos.y / TILE_SIZE);
                hp--;
                if (hp > 0) {
                    RespawnPlayer(hWnd);
                }
                else {
                    
                }
                InvalidateRect(hWnd, NULL, FALSE);
                break;
            }
        }
    }
}

// 적 리스폰
void RespawnEnemies(HWND hWnd) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].active = TRUE;
            enemies[i].health = 1;
            srand((unsigned int)time(NULL) + i);
            enemies[i].pos.x = rand() % currentBoardSize;
            enemies[i].pos.y = rand() % currentBoardSize;
        }
    }
}

// 총알과 적 충돌 확인
BOOL CheckBulletCollision() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (enemies[j].active && bullets[i].pos.x == enemies[j].pos.x && bullets[i].pos.y == enemies[j].pos.y) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

// 적 맞추기
void HitEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (CheckBulletCollision()) {
            enemies[i].health--;
            if (enemies[i].health <= 0) {
                enemies[i].active = FALSE;
                respawnStartTime[i] = GetTickCount(); 
                enemies[i].lastHitTime = GetTickCount();
            }
            for (int j = 0; j < MAX_BULLETS; j++) {
                if (bullets[j].active && bullets[j].pos.x == enemies[i].pos.x && bullets[j].pos.y == enemies[i].pos.y) {
                    bullets[j].active = FALSE;
                    break;
                }
            }
        }
    }
}

// 적 소환
void SpawnEnemy(HWND hWnd) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].pos.x = rand() % currentBoardSize;
            enemies[i].pos.y = rand() % currentBoardSize;
            enemies[i].alert = TRUE;
            SetTimer(hWnd, i + 100, SPAWN_ALERT_DURATION, NULL); 
            break;
        }
    }
}

// 화면 흔들기 적용
void ApplyShake(HWND hWnd) {
    if (shaking) {
        DWORD currentTime = GetTickCount();
        if (currentTime - shakeStartTime > SHAKE_DURATION) {
            shaking = FALSE;
            KillTimer(hWnd, shakeTimer);
            InvalidateRect(hWnd, NULL, TRUE);
        }
        else {
            int offsetX = (rand() % (2 * SHAKE_INTENSITY + 1)) - SHAKE_INTENSITY;
            int offsetY = (rand() % (2 * SHAKE_INTENSITY + 1)) - SHAKE_INTENSITY;
            SetWindowPos(hWnd, NULL, offsetX, offsetY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
    }
}

// 소환 알림 그리기
void DrawSpawnAlert(HDC hdc) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alert) {
            HPEN hPen = CreatePen(PS_SOLID, 3, RGB(0, 0, 255));
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

            RECT rect = {
                enemies[i].pos.x * TILE_SIZE,
                enemies[i].pos.y * TILE_SIZE,
                (enemies[i].pos.x + 1) * TILE_SIZE,
                (enemies[i].pos.y + 1) * TILE_SIZE
            };
            Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

            SelectObject(hdc, hOldPen);
            DeleteObject(hPen);
        }
    }
}

// 윈도우 크기 조정
void ResizeWindow(HWND hWnd) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    int width = currentBoardSize * TILE_SIZE + MARGIN_X;
    int height = currentBoardSize * TILE_SIZE + MARGIN_Y;
    SetWindowPos(hWnd, NULL, rect.left, rect.top, width, height, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
}

// 타이머 일시 정지
void PauseTimers(HWND hWnd) {
    KillTimer(hWnd, playerTimer);
    KillTimer(hWnd, bulletTimer);
    KillTimer(hWnd, enemyMoveTimer);
    KillTimer(hWnd, scoreTimer);
    KillTimer(hWnd, RELOAD_TIMER_ID);
    KillTimer(hWnd, enemySpawnTimer);
    KillTimer(hWnd, shakeTimer);
    for (int i = 0; i < MAX_ENEMIES; i++) {
        KillTimer(hWnd, i + 100);
    }
    KillTimer(hWnd, 200);
}

// 타이머 재개
void ResumeTimers(HWND hWnd) {
    SetTimer(hWnd, playerTimer, MOVE_INTERVAL, NULL);
    SetTimer(hWnd, bulletTimer, BULLET_INTERVAL, NULL);
    SetTimer(hWnd, enemyMoveTimer, MOVE_INTERVAL * 50, NULL);
    SetTimer(hWnd, scoreTimer, SCORE_INTERVAL, NULL);
    SetTimer(hWnd, RELOAD_TIMER_ID, RELOAD_INTERVAL, NULL);
    SetTimer(hWnd, enemySpawnTimer, SPAWN_INTERVAL, NULL);
    if (shaking) {
        SetTimer(hWnd, shakeTimer, 10, NULL);
    }
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alert) {
            SetTimer(hWnd, i + 100, SPAWN_ALERT_DURATION, NULL);
        }
    }
    if (playerImmune) {
        SetTimer(hWnd, 200, IMMUNITY_DURATION, NULL);
    }
}

// 특수 총알 발사
void ShootSpecial() {
    if (specialBulletCount > 0) {
        specialBulletCount--;
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].active) {
                bullets[i].pos = { playerPos.x / TILE_SIZE, playerPos.y / TILE_SIZE };
                bullets[i].dx = 1;
                bullets[i].dy = 0;
                bullets[i].active = TRUE;
                break;
            }
        }
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].active) {
                bullets[i].pos = { playerPos.x / TILE_SIZE, playerPos.y / TILE_SIZE };
                bullets[i].dx = -1;
                bullets[i].dy = 0;
                bullets[i].active = TRUE;
                break;
            }
        }
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].active) {
                bullets[i].pos = { playerPos.x / TILE_SIZE, playerPos.y / TILE_SIZE };
                bullets[i].dx = 0;
                bullets[i].dy = 1;
                bullets[i].active = TRUE;
                break;
            }
        }
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].active) {
                bullets[i].pos = { playerPos.x / TILE_SIZE, playerPos.y / TILE_SIZE };
                bullets[i].dx = 0;
                bullets[i].dy = -1;
                bullets[i].active = TRUE;
                break;
            }
        }
    }
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpszCmdParam, _In_ int nCmdShow) {
    HWND hWnd;
    MSG Message;
    WNDCLASSEX WndClass;
    g_hInst = hInstance;

    WndClass.cbSize = sizeof(WndClass);
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    WndClass.lpfnWndProc = WndProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = hInstance;
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    WndClass.lpszMenuName = NULL;
    WndClass.lpszClassName = lpszClass;
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassEx(&WndClass);

    currentBoardSize = BOARD_SIZE_HARD;
    playerPos = { currentBoardSize / 2 * TILE_SIZE, currentBoardSize / 2 * TILE_SIZE };

    InitBoard(currentBoardSize);
    InitObstacles(currentBoardSize);
    InitEnemies();
    InitBullets();

    hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, currentBoardSize * TILE_SIZE + MARGIN_X, currentBoardSize * TILE_SIZE + MARGIN_Y, NULL, (HMENU)NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    SetTimer(hWnd, playerTimer, MOVE_INTERVAL, NULL);
    SetTimer(hWnd, bulletTimer, BULLET_INTERVAL, NULL);
    SetTimer(hWnd, enemyMoveTimer, MOVE_INTERVAL * 50, NULL);
    SetTimer(hWnd, scoreTimer, SCORE_INTERVAL, NULL);
    SetTimer(hWnd, enemySpawnTimer, SPAWN_INTERVAL, NULL);
    SetTimer(hWnd, RELOAD_TIMER_ID, RELOAD_INTERVAL, NULL);

    while (GetMessage(&Message, NULL, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
    return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
    static HDC hdcMem;
    static HBITMAP hbmMem;
    static HBITMAP hbmOld;
    static int cxClient, cyClient;

    HDC hdc;
    PAINTSTRUCT ps;

    switch (iMessage) {
    case WM_CREATE:
        hdc = GetDC(hWnd);
        cxClient = currentBoardSize * TILE_SIZE;
        cyClient = currentBoardSize * TILE_SIZE;
        hdcMem = CreateCompatibleDC(hdc);
        hbmMem = CreateCompatibleBitmap(hdc, cxClient, cyClient);
        hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
        ReleaseDC(hWnd, hdc);
        break;

    case WM_SIZE:
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);
        if (hdcMem != NULL) {
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            hbmMem = CreateCompatibleBitmap(GetDC(hWnd), cxClient, cyClient);
            hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
        }
        break;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        DrawBoard(hdcMem, currentBoardSize);
        DrawPlayer(hdcMem);
        DrawEnemies(hdcMem);
        DrawBullets(hdcMem);
        DrawSpawnAlert(hdcMem); 
        DrawUI(hdcMem);
        DrawHP(hdcMem, hp);

        BitBlt(hdc, 0, 0, cxClient, cyClient, hdcMem, 0, 0, SRCCOPY);
        EndPaint(hWnd, &ps);
        break;
    case WM_KEYDOWN:
        switch (wParam) {
        case 'W': playerSpeedY = -5; break;
        case 'S': playerSpeedY = 5; break;
        case 'A': playerSpeedX = -5; break;
        case 'D': playerSpeedX = 5; break;
        case VK_UP: Shoot(0, -1); break;
        case VK_DOWN: Shoot(0, 1); break;
        case VK_LEFT: Shoot(-1, 0); break;
        case VK_RIGHT: Shoot(1, 0); break;
        case 'P':
            if (paused) {
                paused = FALSE;
                ResumeTimers(hWnd);
            }
            else {
                paused = TRUE;
                PauseTimers(hWnd);
            }
            break;
        case 'Q':
            PostQuitMessage(0);
            break;
        case '1':
            currentBoardSize = BOARD_SIZE_EASY;
            obstacleCount = 0;
            playerPos = { currentBoardSize / 2 * TILE_SIZE, currentBoardSize / 2 * TILE_SIZE };
            InitBoard(currentBoardSize);
            ResizeWindow(hWnd);
            break;
        case '2':
            currentBoardSize = BOARD_SIZE_NOMAL;
            obstacleCount = 0;
            playerPos = { currentBoardSize / 2 * TILE_SIZE, currentBoardSize / 2 * TILE_SIZE };
            InitBoard(currentBoardSize);
            ResizeWindow(hWnd);
            break;
        case '3':
            currentBoardSize = BOARD_SIZE_HARD;
            obstacleCount = OBSTACLE_COUNT_HARD;
            playerPos = { currentBoardSize / 2 * TILE_SIZE, currentBoardSize / 2 * TILE_SIZE };
            InitBoard(currentBoardSize);
            if (obstacleCount > 0) {
                InitObstacles(currentBoardSize);
            }
            ResizeWindow(hWnd);
            break;
        case '4':
            godMode = !godMode; 
            break;
        case 'X':
            ShootSpecial();
            break;
        }
        InvalidateRect(hWnd, NULL, TRUE);
        break;


    case WM_KEYUP:
        switch (wParam) {
        case 'W': case 'S': playerSpeedY = 0; break;
        case 'A': case 'D': playerSpeedX = 0; break;
        }
        break;

    case WM_TIMER:
        if (wParam == playerTimer) {
            MovePlayer();
            CheckPlayerEnemyColl(hWnd); 
            InvalidateRect(hWnd, NULL, FALSE);
        }
        else if (wParam == bulletTimer) {
            hdc = GetDC(hWnd);
            UpdateBullets(hdc, hWnd);
            ReleaseDC(hWnd, hdc);
            HitEnemies();
            CheckPlayerEnemyColl(hWnd); 
            InvalidateRect(hWnd, NULL, FALSE);
        }
        else if (wParam == enemyMoveTimer) {
            MoveEnemies();
            CheckPlayerEnemyColl(hWnd); 
            InvalidateRect(hWnd, NULL, FALSE);
        }
        else if (wParam == scoreTimer) {
            if (score < MAX_SCORE) {
                score += 100;
            }
            InvalidateRect(hWnd, NULL, FALSE);
        }
        else if (wParam == RELOAD_TIMER_ID) {
            if (bulletCount < MAX_BULLETS) {
                bulletCount++;
                InvalidateRect(hWnd, NULL, FALSE);
            }
        }
        else if (wParam == enemySpawnTimer) {
            SpawnEnemy(hWnd); 
            InvalidateRect(hWnd, NULL, FALSE);
        }
        else if (wParam == shakeTimer) {
            ApplyShake(hWnd);
        }
        else if (wParam >= 100 && wParam < 110) { 
            int enemyIndex = wParam - 100;
            enemies[enemyIndex].alert = FALSE;
            enemies[enemyIndex].active = TRUE;
            respawnStartTime[enemyIndex] = GetTickCount(); 
            KillTimer(hWnd, wParam); 
            InvalidateRect(hWnd, NULL, FALSE);
        }
        else if (wParam == 200) { 
            playerImmune = FALSE;
            KillTimer(hWnd, wParam); 
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;

    case WM_DESTROY:
        KillTimer(hWnd, playerTimer);
        KillTimer(hWnd, bulletTimer);
        KillTimer(hWnd, enemyMoveTimer);
        KillTimer(hWnd, scoreTimer);
        KillTimer(hWnd, RELOAD_TIMER_ID);
        KillTimer(hWnd, enemySpawnTimer);
        KillTimer(hWnd, shakeTimer);
        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        for (int i = 0; i < currentBoardSize; i++) {
            free(board[i]);
        }
        free(board);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, iMessage, wParam, lParam);
}
