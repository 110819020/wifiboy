#include <wifiboypro.h>
#include <Arduino.h>
#include <esp32-hal.h>
#include "wb-sprite.h"

int score = 0;                  //分數
int startX = -40;                 //開始畫面動畫X軸
int masterX = 25;               //玩家X軸
int masterY = 200;              //玩家Y軸
const int beginX = 25;          //玩家基準X軸
const int beginY = 200;         //玩家基準Y軸
bool jumpIsPressed = false;     //是否跳躍
bool downIsPressed = false;     //是否蹲低
int maxHeight = 140;            //最大跳躍高度
bool up = true;                 //是否處於跳躍上升時
bool isFire = false;            //是否開火
unsigned long currentTime;      //現在時間
unsigned long bulletSpawnCD;    //開火冷卻
int bulletNo = 0;               //子彈編號
bool bulletAlive[5];            //子彈可生成
int bulletStatus[5];            //子彈狀態(設定、顯示、初始)
int bulletX[5];                 //子彈X軸
int bulletY[5];                 //子彈Y軸
unsigned long enemySpawnCD;     //敵人生成冷卻
int enemySpeed = 7;       //敵人相對移動速度
int enemyX[5];                  //敵人X軸
int enemyY[3] = {172, 216, 168};//敵人Y軸(空中、地上、大型)
bool enemyAlive[5];             //敵人是否存活
int randomEnemy[5];             //隨機敵人類型
int enemyNo;                    //敵人編號
int status = 0;                 //玩家形狀變化
int sceneStatus = 0;            //畫面狀態
int toturialStatus = 0;         //教學畫面切換
int RpressOnce = 0;             //R是否按一次
int LpressOnce = 0;             //L是否按一次
int BpressOnce = 0;             //B是否按一次

void blit_str256(const char *str, int x, int y)
{
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] >= '@' && str[i] <= ']')
            wbpro_blitBuf8(8 * (str[i] - '@'), 0, 240, x + i * 8, y, 8, 8, (uint8_t *)sprites);
        if (str[i] >= '!' && str[i] <= '>')
            wbpro_blitBuf8(8 * (str[i] - '!'), 8, 240, x + i * 8, y, 8, 8, (uint8_t *)sprites);
        if (str[i] == '?')
            wbpro_blitBuf8(8 * 14, 16, 240, x + i * 8, y, 8, 8, (uint8_t *)sprites);
        if (str[i] == 'c')
            wbpro_blitBuf8(8 * 13, 16, 240, x + i * 8, y, 8, 8, (uint8_t *)sprites);
        if (str[i] == 'w')
            wbpro_blitBuf8(7, 16, 240, x + i * 8, y, 26, 8, (uint8_t *)sprites);
        if (str[i] == 'x')
            wbpro_blitBuf8(42, 16, 240, x + i * 8, y, 61, 8, (uint8_t *)sprites);
    }
}

void blit_num256(uint16_t num, uint16_t x, uint16_t y, uint8_t color_mode)
{
    uint16_t d[5];

    d[0] = num / 10000;
    d[1] = (num - d[0] * 10000) / 1000;
    d[2] = (num - d[0] * 10000 - d[1] * 1000) / 100;
    d[3] = (num - d[0] * 10000 - d[1] * 1000 - d[2] * 100) / 10;
    d[4] = num - d[0] * 10000 - d[1] * 1000 - d[2] * 100 - d[3] * 10;

    for (int i = 0; i < 5; i++)
    {
        wbpro_blitBuf8(d[i] * 8 + 120, color_mode * 8, 240, x + i * 8, y, 8, 8, (uint8_t *)sprites); //將d[0]~d[4]逐個顯示並排列
    }
}

void setup()
{
    wbpro_init();
    wbpro_initBuf8();
    wbpro_fillScreen(wbYELLOW);
    for(int i = 0; i < 256; i++)
    {
        wbpro_setPal8(i, wbpro_color565(standardColour[i][0], standardColour[i][1], standardColour[i][2]));
    }
    // for(int i = 0; i < 100; i++)
    // {
    //     StarX[i] = random(0, 240);
    //     StarY[i] = random(0, 320);
    //     StarSpeed[i] = random(2, 5);
    // }
    pinMode(0, INPUT);
    pinMode(32, INPUT);
    pinMode(33, INPUT);
    pinMode(34, INPUT);
    pinMode(35, INPUT);
    pinMode(36, INPUT);
    pinMode(39, INPUT);
    pinMode(16, OUTPUT);
    //初始敵人類型
    for(int i = 0; i < 3; i++)
    {
        randomEnemy[i] = random(0, 3);
    }
    //初始敵人X軸
    for(int i = 0; i < 5; i++)
    {
        enemyX[i] = 300;
    }
}

void loop()
{
    wbpro_clearBuf8();

    currentTime = millis();
    SceneCtrl();

    wbpro_blit8();
}

void DrawRoad()
{
    for(int i = 0; i < 240; i++)
    {
        wbpro_setBuf8(i+230*240, wbWHITE);
    }
}

void ActControl()
{
    //move down
    if(digitalRead(39) == 0)
    {
        downIsPressed = true;
        jumpIsPressed = false;
    }
    //move up
    if(digitalRead(36) == 0)
    {
        downIsPressed = false;
        jumpIsPressed = true;
        status = 1;
    }
    else if(digitalRead(39) == 1 && jumpIsPressed == false)
    {
        downIsPressed = false;
        masterY = beginY;
    }

    if(digitalRead(35) == 0)
    {
        isFire = true;
    }
    else
    {
        isFire = false;
    }
    
}

void MoveUpdate()
{
    const int maxHeight = 140;
    if(downIsPressed)
    {
        if(masterY+23 <= beginY+32)
        {
            masterY += 10;
        }
        else
        {
            status = 2;
        }
    }
    else if(jumpIsPressed)
    {  
        //上升階段
        if(masterY >= maxHeight && up)
        {
            masterY -= 8;
        }
        //下降階段
        else
        {   
            up=false;
            masterY += 8;
        }
        //落地
        if(masterY >= beginY)
        {
            masterY = beginY;
            jumpIsPressed = false;
            up=true;
        }
    }
    else
    {
        status = 0;
    }
}

//設定是否能發射
void Shoot()
{
    if(currentTime >= bulletSpawnCD && isFire)
    {
        bulletAlive[bulletNo] = true;
        bulletStatus[bulletNo] = 0;
        if(bulletNo < 4)
        {
            bulletNo += 1;
        }
        else
        {
            bulletNo = 0;
        }
        bulletSpawnCD = currentTime + 600;
    }
}

//畫出火焰
void DrawFire()
{
    for(int i  = 0; i < 5; i++)
    {
        if(bulletAlive[i])
        {
            switch (bulletStatus[i])
            {
            //設定生成位置
            case 0:
                bulletY[i] = masterY;
                bulletX[i] = masterX + 24;
                bulletStatus[i]++;
                break;
            //生成並前進
            case 1:
                bulletX[i] += 10;
                wbpro_blitBuf8(0, 0, 32, bulletX[i], bulletY[i], 32, 32, (uint8_t *) s_bullet);
                if(bulletX[i] > 240)
                {
                    bulletStatus[i]++;
                }
                break;
            //消失
            case 2:
                bulletX[i] = 400;
                bulletAlive[i] = false;
            }
        }
        else 
        {
            continue;
        }
    }
}


void InitEnemy()
{
    
    if(currentTime >= enemySpawnCD)
    {
        enemyAlive[enemyNo] = true;
        if(enemyNo < 5)
        {
            enemyNo += 1;
        }
        else
        {
            enemyNo = 0;
        }
        enemySpawnCD = currentTime + random(800, 1200);
    }
    for(int i = 0; i < 5; i++)
    {
        if(enemyAlive[i])
        {
            enemyX[i] -= enemySpeed;
            if(randomEnemy[i] == 0)
            {
                wbpro_blitBuf8(0, 0, 32, enemyX[i], enemyY[0], 32, 32, (uint8_t *) e_sky);
            }
            else if(randomEnemy[i] == 1)
            {
                wbpro_blitBuf8(0, 0, 32, enemyX[i], enemyY[1], 32, 32, (uint8_t *) e_ground);
            }
            else if(randomEnemy[i] == 2)
            {
                wbpro_blitBuf8(0, 0, 64, enemyX[i], enemyY[2], 64, 64, (uint8_t *) e_big);
            }

            if(enemyX[i] <= -32)
            {
                score++;
                enemyX[i] = 300;
                randomEnemy[i] = random(0, 3);
                enemyAlive[i] = false;
            }
        }
        else 
        {
            continue;
        }
    }
}

void Collider()
{
    for(int i = 0; i < 5; i++)
    {
        switch (randomEnemy[i])
        {
        case 0:
            if(((masterX+4 >= enemyX[i] && masterX+4 <= enemyX[i]+32) || (masterX + 28 >= enemyX[i] && masterX + 28 <= enemyX[i]+32)) && ((masterY >= enemyY[0] && masterY <= enemyY[0] + 32) || (masterY + 32 >= enemyY[0] && masterY + 32 <= enemyY[0] + 32)))
            {
                Clean();
            }
            break;
        
        case 1:
            if(((masterX+4 >= enemyX[i]+4 && masterX+4 <= enemyX[i]+28) || (masterX + 28 >= enemyX[i]+4 && masterX + 28 <= enemyX[i]+28)) && ((masterY >= enemyY[1] && masterY <= enemyY[1] + 32) || (masterY + 32 >= enemyY[1] && masterY + 32 <= enemyY[1] + 32)))
            {
                Clean();
            }
            break;

        case 2:
            if(((masterX >= enemyX[i]+12 && masterX <= enemyX[i]+52) || (masterX + 32 >= enemyX[i]+12 && masterX + 32 <= enemyX[i]+52)) && ((masterY >= enemyY[2] && masterY <= enemyY[2] + 64) || (masterY + 32 >= enemyY[2] && masterY + 32 <= enemyY[2] + 64)))
            {
                Clean();
            }
            break;
        }
    }
}

void ShootEnemy()
{
    digitalWrite(16, LOW);
    for(int i = 0; i < 5; i++)
    {
        if(randomEnemy[i] == 2)
        {  
            for(int j = 0; j < 5; j++)
            {
                if(((bulletX[j] >= enemyX[i] && bulletX[j] <= enemyX[i] + 64)||(bulletX[j] + 32 >= enemyX[i] && bulletX[j]+ 32 <= enemyX[i] + 64)) && ((bulletY[j] >= enemyY[2] && bulletY[j] <= enemyY[2] + 64)||(bulletY[j] + 32 >= enemyY[2] && bulletY[j] + 32 <= enemyY[2] + 64)))
                {
                    score++;
                    digitalWrite(16, HIGH);
                    enemyAlive[i] = false;
                    bulletStatus[j] = 2;
                    enemyX[i] = 300;
                    randomEnemy[i] = random(0, 3);
                }
            }
        }
        else
        {
            continue;
        }
        
    }
}
//清除並轉換螢幕
void Clean()
{
    enemyNo = 0;
    bulletNo = 0;
    for (int i = 0; i < 5; i++)
    {
        bulletStatus[i] = 2;
        enemyAlive[i] = false;
        enemyX[i] = 300;
        randomEnemy[i] = random(0, 3);
    }
    masterX = beginX;
    masterY = beginY;
    delay(300);
    sceneStatus++;
}

void SceneCtrl()
{
    switch (sceneStatus)
    {

    //開始畫面
    case 0:
        if (digitalRead(34) == 1)           //判斷B放開
        {
            BpressOnce = 0;
        }
        if(digitalRead(0) == 0)
        {
            sceneStatus = 1;
        }
        else if(digitalRead(34) == 0 && BpressOnce == 0)
        {   
            BpressOnce = 1;
            toturialStatus = 0;
            sceneStatus = 3;
        }
        score = 0;
        masterX = beginX;
        masterY = beginY;
        blit_str256("JUMP GAME !", 75, 80);
        blit_str256("PRESS \"START\" TO PLAY", 35, 200);
        blit_str256("TOTURIAL (B)",10 ,300);
        startX += 10;
        if(startX % 40 < 20)
        {
            wbpro_blitBuf8(0, 0, 32, startX, 250, 32, 32, (uint8_t *) s_idle);
        }
        else
        {
            wbpro_blitBuf8(0, 0, 32, startX, 250, 32, 32, (uint8_t *) s_walk);
        }
        if(startX >= 240)
        {
            startX = -40;
        }
        break;

    //遊戲中
    case 1:
        DrawRoad();
        blit_str256("SCORE", 0, 0);
        blit_num256(score, 40, 0, 1);
        //設定速度
        if(score >= 90) enemySpeed = 13;
        else if(score >= 70) enemySpeed = 12;
        else if(score >= 55) enemySpeed = 11;
        else if(score >= 40) enemySpeed = 10;
        else if(score >= 25) enemySpeed = 9;
        else if(score >= 10) enemySpeed = 8;
        else enemySpeed = 7;
        ActControl();
        MoveUpdate();
        DrawFire();
        InitEnemy();
        Collider();
        ShootEnemy();
        //改變形狀
        switch (status)
        {
            case 0:
                Shoot();
                if (isFire)
                {
                    wbpro_blitBuf8(0, 0, 32, masterX, masterY, 32, 32, (uint8_t *) s_idle);
                }
                else
                {
                    wbpro_blitBuf8(0, 0, 32, masterX, masterY, 32, 32, (uint8_t *) s_walk);
                }
                break;
        
            case 1:
                wbpro_blitBuf8(0, 0, 32, masterX, masterY, 32, 32, (uint8_t *) s_jump);
                break;

            case 2:
                wbpro_blitBuf8(0, 9, 32, masterX, masterY, 32, 23, (uint8_t *) s_down);
                break;
        }
        break;

    //結束畫面
    case 2:
        if (digitalRead(34) == 1)           //判斷B放開
        {
            BpressOnce = 0;
        }
        if(digitalRead(35) == 0)
        {
            score = 0;
            sceneStatus = 1;
        }
        else if(digitalRead(34) == 0 && BpressOnce == 0)
        {
            BpressOnce = 1;
            startX = -40;
            sceneStatus = 0;
        }
        blit_str256("GAMEOVER", 87, 100);
        blit_str256("YOUR SCORE: ", 35, 150);
        blit_num256(score, 145, 150, 1);
        blit_str256("PRESS A TO RESTART", 45, 240);
        blit_str256("PRESS B TO HOME", 60, 260);
        break;
    
    //教學頁面
    case 3:
        if (digitalRead(34) == 1)           //判斷B放開
        {
            BpressOnce = 0;
        }
        if(digitalRead(34) == 0 && BpressOnce == 0)
        {
            BpressOnce = 1;
            sceneStatus = 0;
        }
        DrawRoad();
        blit_str256("HOME (B)", 10, 10);
        switch (toturialStatus)
        {

        //向上教學   
        case 0:
            if(digitalRead(33) == 1)            //判斷L放開
            {
                LpressOnce = 0;
            }
            if(digitalRead(32) == 0 && RpressOnce == 0)
            {
                RpressOnce = 1;
                toturialStatus = 1;
            }
            //跳躍教學實際操作
            if(digitalRead(36) == 0)
            {   
                jumpIsPressed = true;
            }
            else if(digitalRead(36) == 1 && jumpIsPressed == false)
            {
                masterY = beginY;
                wbpro_blitBuf8(0, 0, 32, masterX, masterY, 32, 32, (uint8_t *) s_walk);
            }
            //上升階段
            if(jumpIsPressed)
            {
                wbpro_blitBuf8(0, 0, 32, masterX, masterY, 32, 32, (uint8_t *) s_jump);
                if(masterY >= maxHeight && up)
                {
                    masterY -= 8;
                }
                //下降階段
                else
                {   
                    up=false;
                    masterY += 8;
                }
                //落地
                if(masterY >= beginY)
                {
                    masterY = beginY;
                    jumpIsPressed = false;
                    up=true;
                }
            }
            blit_str256("1. PRESS U TO JUMP", 35, 60);
            blit_str256("NEXT (R)", 155, 300);
            break;
        
        //向下教學
        case 1:
            if(digitalRead(32) == 1)            //判斷R放開
            {
                RpressOnce = 0;
            }
            if(digitalRead(33) == 1)            //判斷L放開
            {
                LpressOnce = 0;
            }
            if(digitalRead(32) == 0 && RpressOnce == 0)
            {
                RpressOnce = 1;
                toturialStatus = 2;
            }
            if(digitalRead(33) == 0 && LpressOnce == 0)
            {
                LpressOnce = 1;
                toturialStatus = 0;
            }
            //蹲低教學實際操作
            if(digitalRead(39) == 0)
            {
                if(masterY+23 <= beginY+32)
                {
                    masterY += 10;
                }
                else 
                {
                    wbpro_blitBuf8(0, 9, 32, masterX, masterY, 32, 23, (uint8_t *) s_down);
                }
            }
            else
            {
                masterY = beginY;
                wbpro_blitBuf8(0, 0, 32, masterX, masterY, 32, 32, (uint8_t *) s_walk);
            }
            blit_str256("2. PRESS D TO SQUAT DOWN", 10, 60);
            blit_str256("BACK (L)", 10, 300);
            blit_str256("NEXT (R)", 155, 300);
            break;

        //開火教學
        case 2:
            if(digitalRead(32) == 1)            //判斷R放開
            {
                RpressOnce = 0;
            }
            if(digitalRead(33) == 0 && LpressOnce == 0)
            {
                LpressOnce = 1;
                toturialStatus = 1;
            }
            //開火教學實際操作
            Shoot();
            DrawFire();
            if(digitalRead(35) == 0)
            {
                isFire = true;
            }
            else
            {
                isFire = false;
            }
            if (isFire)
            {
                wbpro_blitBuf8(0, 0, 32, masterX, masterY, 32, 32, (uint8_t *) s_idle);
            }
            else
            {
                wbpro_blitBuf8(0, 0, 32, masterX, masterY, 32, 32, (uint8_t *) s_walk);
            }
            blit_str256("3. PRESS A TO FIRE", 35, 60);
            blit_str256("BACK (L)", 10, 300);
            break;
        }
    }
}