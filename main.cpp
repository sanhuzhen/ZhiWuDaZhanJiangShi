#include<stdio.h>
#include<time.h>
#include<windows.h>
#include<graphics.h>//图形库
#include "tools.h"//消除黑边
#include<math.h>
#include "vector2.h"
#include<mmsystem.h>

#pragma comment(lib, "winmm.lib")

#define WIDTH 900
#define HEIGHT 600
enum {
    WAN_DOU, XIANG_RI_KUI, ZHI_WU_COUNT
};//利用枚举实现数组数量灵活
IMAGE imaBg;//背景图片
IMAGE imaBar;//植物框图片
IMAGE imgCards[ZHI_WU_COUNT];//植物卡牌数组
IMAGE *imgZhiWu[ZHI_WU_COUNT][20];//各种植物的动图

int curX, curY;//当前选中的植物，在移动过程中的位置坐标
int curZhiWu;

struct zhiWu {
    int type;
    int frameIndex;//序列帧的序号
    int isAte;//是否被僵尸吃
    int deadTime;//死亡倒计时
    int timer;
    int x;
    int y;
};
struct zhiWu map[3][9];
//定义豌豆子弹
IMAGE imageNormalBullet;
IMAGE imageBlastBullet[4];
struct bullet {
    int x, y;
    bool isUsed;
    int row;
    int speed;
    int isBlasted;//是否发生爆炸
    int frameIndex;
};
struct bullet bullets[30];

//定义僵尸
IMAGE imageZM[22];
IMAGE imageZMDead[20];
IMAGE imageZMEat[21];
struct zm {
    int x, y;
    int row;
    int frameIndex;
    int isUsed;
    int speed;
    int blood;
    int dead;
    int isEating;//吃植物状态
};
struct zm zms[10];
//阳光的几种状态
enum {
    SUNSHINE_DOWN, SUNSHINE_GROUND, SUNSHINE_COLLECT, SUNSHINE_PRODUCE
};
//定义阳光
struct sunshineBoll {
    int x, y;//阳光在飘落过程中的坐标过程
    int frameIndex;//当前显示图片帧的序号
    int target_y;//飘落的y坐标停留地点
    int isUsed;//是否在使用
    int timer;//阳光消失计时器
    float xOff;
    float yOff;

    float t;//贝塞尔曲线时间点
    vector2 p1, p2, p3, p4;//对应贝塞尔曲线四个点
    vector2 pCur;//当前时刻阳光球的速度
    float speed;
    int status;//阳光球的状态
};
//运用池
struct sunshineBoll bolls[10];
//阳光值
int sunshine;

bool fileExist(const char *name);

void gameInit();

void updateWindow();

void userClick();

void updateGame();

void createSunshine();

void updateSunshine();

void collectSunshine(ExMessage *message);

void createZM();

void updateZM();

void shoot();

void updateBullets();

void collisionCheck();

void checkBulletsToZm();

void checkZhiWuToZm();

IMAGE imageSunshine[29];

//判断文件是否存在
bool fileExist(const char *name) {
    FILE *fp = fopen(name, "r");
    if (fp == NULL) return false;
    else {
        fclose(fp);
        return true;
    }
}

//加载游戏初始界面
void gameInit() {
    //加载图片
    loadimage(&imaBg, _T("D:\\code\\clioncode\\untitled\\res\\bg.jpg"));
    loadimage(&imaBar, _T("D:\\code\\clioncode\\untitled\\res\\bar5.png"));

    memset(imgZhiWu, 0, sizeof(imgZhiWu));
    memset(map, 0, sizeof(map));
    //初始化植物卡牌
    char name[64];
    for (int i = 0; i < ZHI_WU_COUNT; i++) {
        //生成植物卡牌文件名
        sprintf_s(name, sizeof(name), "D:\\code\\clioncode\\untitled\\res\\Cards\\card_%d.png", i + 1);
        loadimage(&imgCards[i], _T(name));

        for (int j = 0; j < 20; j++) {
            sprintf_s(name, sizeof(name), "D:\\code\\clioncode\\untitled\\res\\zhiWu\\%d\\%d.png", i, j + 1);
            //先判断文件是否存在
            if (fileExist(name)) {
                //分配内存
                imgZhiWu[i][j] = new IMAGE;
                loadimage(imgZhiWu[i][j], name);
            } else {
                break;
            }
        }
    }
    curZhiWu = 0;
    sunshine = 50;
    memset(bolls, 0, sizeof(bolls));
    for (int i = 0; i < 29; i++) {
        sprintf_s(name, sizeof(name), "D:\\code\\clioncode\\untitled\\res\\sunshine\\%d.png", i + 1);
        loadimage(&imageSunshine[i], _T(name));
    }
    //配置随机种子
    srand(time(NULL));
    //游戏图形框
    initgraph(WIDTH, HEIGHT, 1);
    //设置字体
    LOGFONT f;
    gettextstyle(&f);
    f.lfHeight = 30;
    f.lfWeight = 15;
    strcpy(f.lfFaceName, "Segoe UI Black");
    f.lfQuality = ANTIALIASED_QUALITY;//抗锯齿
    settextstyle(&f);
    setbkmode(TRANSPARENT);//设置字体背景
    setcolor(BLACK);
    //初始化僵尸
    memset(zms, 0, sizeof(zms));
    for (int i = 0; i < 22; i++) {
        sprintf_s(name, sizeof(name), "D:\\code\\clioncode\\untitled\\res\\zm\\%d.png", i + 1);
        loadimage(&imageZM[i], _T(name));
    }
    //僵尸被打倒
    for (int i = 0; i < 20; i++) {
        sprintf_s(name, sizeof(name), "D:\\code\\clioncode\\untitled\\res\\zm_dead\\%d.png", i + 1);
        loadimage(&imageZMDead[i], _T(name));
    }
    //僵尸吃植物
    for (int i = 0; i < 21; i++) {
        sprintf_s(name, sizeof(name), "D:\\code\\clioncode\\untitled\\res\\zm_eat\\%d.png", i + 1);
        loadimage(&imageZMEat[i], _T(name));
    }
    //初始化子弹
    loadimage(&imageNormalBullet, _T("D:\\code\\clioncode\\untitled\\res\\bullets\\bullet_normal.png"));
    memset(bullets, 0, sizeof(bullets));

    //初始化豌豆子弹
    loadimage(&imageBlastBullet[3], _T("D:\\code\\clioncode\\untitled\\res\\bullets\\bullet_blast.png"));
    for (int i = 0; i < 3; i++) {
        float k = 0.2 * (i + 1);
        loadimage(&imageBlastBullet[i], _T("D:\\code\\clioncode\\untitled\\res\\bullets\\bullet_blast.png"),
                  imageBlastBullet[3].getwidth() * k, imageBlastBullet[3].getheight() * k, true);
    }
}

//渲染图片
void updateWindow() {
    BeginBatchDraw();//开始双缓冲
    putimage(0, 0, &imaBg);
    putimagePNG(250, 0, &imaBar);
    int width = 338;
    for (int i = 0; i < ZHI_WU_COUNT; i++) {
        putimagePNG(width, 5, &imgCards[i]);
        width += 65;
    }
    //渲染种植的植物
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 9; j++) {
            if (map[i][j].type > 0) {//需要判断，否侧直接崩
//                map[i][j].x = 256 + 81 * j;
//                map[i][j].y = 193 + i * 102;
                putimagePNG(map[i][j].x, map[i][j].y, imgZhiWu[map[i][j].type - 1][map[i][j].frameIndex]);
            }
        }
    }
    //渲染拖动过程的植物
    if (curZhiWu) {
        IMAGE *img = imgZhiWu[curZhiWu - 1][0];
        putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);
    }
    //渲染阳光
    int bollMax = sizeof(bolls) / sizeof(bolls[0]);
    for (int i = 0; i < bollMax; i++) {
        if (bolls[i].isUsed || bolls[i].xOff) {
            putimagePNG(bolls[i].pCur.x, bolls[i].pCur.y, &imageSunshine[bolls[i].frameIndex]);
        }
    }
    //将阳光值输入
    char scoreText[8];
    sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
    outtextxy(285, 67, scoreText);

    //渲染僵尸
    int zmMax = sizeof(zms) / sizeof(zms[0]);
    for (int i = 0; i < zmMax; i++) {
        if (zms[i].isUsed) {
            if (zms[i].dead == 0) {
                if (zms[i].isEating == 0) {
                    putimagePNG(zms[i].x, zms[i].y, &imageZM[zms[i].frameIndex]);
                } else {
                    putimagePNG(zms[i].x, zms[i].y, &imageZMEat[zms[i].frameIndex]);
                }
            } else {
                putimagePNG(zms[i].x, zms[i].y, &imageZMDead[zms[i].frameIndex]);
            }

        }
    }
    //渲染子弹
    int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
    for (int i = 0; i < bulletMax; i++) {
        if (bullets[i].isUsed) {
            if (!bullets[i].isBlasted) {
                putimagePNG(bullets[i].x, bullets[i].y, &imageNormalBullet);
            } else {
                putimagePNG(bullets[i].x, bullets[i].y, &imageBlastBullet[bullets[i].frameIndex]);

            }
        }
    }
    EndBatchDraw();//结束双缓冲
}

//用户点击功能
void userClick() {
    //判断鼠标是否传来消息
    ExMessage msg;
    static int status = 0;//是否选择成功
    if (peekmessage(&msg)) {
        if (msg.message == WM_LBUTTONDOWN) {//判断是否是鼠标左键点击
            if (msg.x > 338 && msg.x < 338 + 65 * ZHI_WU_COUNT && msg.y > 5 && msg.y < 96) {
                //判断是第几张卡牌
                int index = (msg.x - 338) / 65;
                printf("%d\n", index);//打印，避免出错
                status = 1;
                curZhiWu = index + 1;
                curX = msg.x;
                curY = msg.y;
            } else {
                collectSunshine(&msg);
            }
        } else if (msg.message == WM_MOUSEMOVE && status == 1) {//判断是否鼠标移动
            curX = msg.x;
            curY = msg.y;
        } else if (msg.message == WM_LBUTTONUP) {//判断鼠标左键是否松开
            //确定种在几行几列
            if (msg.x > 256 && msg.y > 179 && msg.y < 489) {
                int row = (msg.y - 179) / 102;
                int col = (msg.x - 256) / 81;
                printf("%d %d\n", row, col);
                if (map[row][col].type == 0) {
                    map[row][col].type = curZhiWu;
                    map[row][col].frameIndex = 0;
                    map[row][col].x = 256 + 81 * col;
                    map[row][col].y = 193 + row * 102;
                }

            }
            curZhiWu = 0;
            status = 0;
        }
    }
}


//改变游戏相关数据
void updateGame() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 9; j++) {
            if (map[i][j].type) {
                map[i][j].frameIndex++;
                if (imgZhiWu[map[i][j].type - 1][map[i][j].frameIndex] == NULL) {
                    map[i][j].frameIndex = 0;
                }
            }
        }
    }
    createSunshine();
    updateSunshine();
    createZM();
    updateZM();
    shoot();
    updateBullets();
    collisionCheck();//碰撞检测
}

void collisionCheck() {
    checkBulletsToZm();//子弹与僵尸碰撞
    checkZhiWuToZm();//植物与僵尸碰撞
}

void checkZhiWuToZm() {
    int zmMax = sizeof(zms) / sizeof(zms[0]);
    for (int i = 0; i < zmMax; i++) {
        if (zms[i].dead) {
            continue;
        }
        //检测同行植物
        int row = zms[i].row;
        for (int j = 0; j < 9; j++) {
            if (map[row][j].type == 0) continue;
            int zhiWuX = 256 + j * 81;
            int x1 = zhiWuX + 10;
            int x2 = zhiWuX + 60;
            int x3 = zms[i].x + 80;
            if (x3 > x1 && x3 < x2) {
                if (map[row][j].isAte) {
                    map[row][j].deadTime++;
                    if (map[row][j].deadTime > 100) {
                        map[row][j].type = 0;
                        map[row][j].deadTime = 0;
                        zms[i].isEating = 0;
                        zms[i].frameIndex = 0;
                        zms[i].speed = 1;
                    }
                } else {
                    map[row][j].isAte = 1;
                    map[row][j].deadTime = 0;
                    zms[i].isEating = 1;
                    zms[i].speed = 0;
                    zms[i].frameIndex = 0;
                }

            }
        }
    }
}

void checkBulletsToZm() {
    int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
    int zmMax = sizeof(zms) / sizeof(zms[0]);
    for (int i = 0; i < bulletMax; i++) {
        if (bullets[i].isUsed == 0 || bullets[i].isBlasted) {
            continue;
        }
        for (int j = 0; j < zmMax; j++) {
            if (zms[j].isUsed == 0) {
                continue;
            }
            int x1 = zms[j].x + 80;
            int x2 = zms[j].x + 110;
            if (bullets[i].row == zms[j].row && bullets[i].x > x1 && bullets[i].x < x2 && !zms[j].dead) {
                bullets[i].isBlasted = 1;
                zms[j].blood -= 10;
                bullets[i].speed = 0;
                if (zms[j].blood <= 0) {
                    zms[j].dead = 1;
                    zms[j].speed = 0;
                    zms[j].frameIndex = 0;
                }
            }
        }
    }
}

//更新子弹的位置
void updateBullets() {
    int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
    for (int i = 0; i < bulletMax; i++) {
        if (bullets[i].isUsed) {
            bullets[i].x += bullets[i].speed;
            if (bullets[i].x > WIDTH) {
                bullets[i].isUsed = 0;
            }
            if (bullets[i].isBlasted) {
                bullets[i].frameIndex++;
                if (bullets[i].frameIndex >= 4) {
                    bullets[i].isUsed = 0;
                }
            }
        }
    }
}

//发射子弹
void shoot() {
    int lines[3] = {0};
    int zmMax = sizeof(zms) / sizeof(zms[0]);
    int dangerX = WIDTH - imageZM[0].getwidth();
    int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
    for (int i = 0; i < zmMax; i++) {
        if (zms[i].isUsed && zms[i].x < dangerX) {
            lines[zms[i].row] = 1;
//            printf("%d\n",zms[i].row);
        }
    }
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 9; j++) {
            if (map[i][j].type == WAN_DOU + 1 && lines[i]) {
                static int count = 0;
                count++;
                if (count > 20) {
                    count = 0;
                    int k;
                    for (k = 0; k < bulletMax && bullets[k].isUsed; k++);
                    if (k < bulletMax) {
                        bullets[k].isUsed = true;
                        bullets[k].row = i;
                        bullets[k].frameIndex = 0;
                        bullets[k].isBlasted = 0;
                        bullets[k].speed = 6;
                        int zwX = 256 + j * 81;
                        int zwY = 193 + i * 102;
                        bullets[k].x = zwX + imgZhiWu[0][0]->getwidth() - 10;
                        bullets[k].y = zwY + 5;
                        lines[i] = 0;
                    }
                }
            }
        }
    }
}

//更新僵尸的状态
void updateZM() {
    int zmMax = sizeof(zms) / sizeof(zms[0]);
    static int count = 0;
    count++;
    if (count > 2) {
        count = 0;
        for (int i = 0; i < zmMax; i++) {
            if (zms[i].isUsed) {
                zms[i].x -= zms[i].speed;
//            if (zms[i].x <= 170) {
//                printf("GAME OVER\n");
//                MessageBox(NULL, "over", "over", 0);//待优化
//                exit(0);
//            }

            }
        }
    }
    static int count2 = 0;
    count2++;
    if (count2 > 2) {
        count2 = 0;
        for (int i = 0; i < zmMax; i++) {
            if (zms[i].isUsed) {
                if (zms[i].dead) {
                    zms[i].frameIndex = zms[i].frameIndex + 1;
                    if (zms[i].frameIndex >= 20) {
                        zms[i].isUsed = 0;
                    }
                } else if (zms[i].isEating == 0) {
                    zms[i].frameIndex = (zms[i].frameIndex + 1) % 22;
                } else {
                    zms[i].frameIndex = (zms[i].frameIndex + 1) % 21;
                }

            }
        }
    }


}

//创建僵尸
void createZM() {
    static int zmFre = 400;
    static int count = 0;
    count++;
    if (count > zmFre) {
        count = 0;
        zmFre = 200 + rand() % 200;
        int i;
        int zmMax = sizeof(zms) / sizeof(zms[0]);
        for (i = 0; i < zmMax && zms[i].isUsed; i++);
        if (i < zmMax) {
            memset(&zms[i], 0, sizeof(zms[i]));
            zms[i].isUsed = 1;
            zms[i].x = WIDTH;
            zms[i].row = rand() % 3;
            zms[i].y = 26 + (1 + zms[i].row) * 100;
            zms[i].speed = 1;
            zms[i].blood = 100;
            zms[i].frameIndex = 0;
            zms[i].dead = 0;
            zms[i].isEating = 0;
        }
    }
}

//更新阳光状态
void updateSunshine() {
    int bollMax = sizeof(bolls) / sizeof(bolls[0]);
    for (int i = 0; i < bollMax; i++) {
        struct sunshineBoll *sun = &bolls[i];
        if (sun->isUsed) {
            sun->frameIndex = (sun->frameIndex + 1) % 29;
            if (sun->status == SUNSHINE_DOWN) {
                sun->t += sun->speed;
                sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
                if (sun->t >= 1) {
                    sun->status = SUNSHINE_GROUND;
                    sun->timer = 0;
                }
            } else if (sun->status == SUNSHINE_GROUND) {
                sun->timer++;
                if (sun->timer > 100) {
                    sun->isUsed = 0;
                    sun->timer = 0;
                }
            } else if (sun->status == SUNSHINE_COLLECT) {
                sun->t += sun->speed;
                sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
                if (sun->t > 1) {
                    sun->isUsed = 0;
                    sunshine += 25;
                    sun->t = 0;
                }
            } else {
                sun->t += sun->speed;
                sun->pCur = calcBezierPoint(sun->t, sun->p1, sun->p2, sun->p3, sun->p4);
                if (sun->t > 1) {
                    sun->status = SUNSHINE_GROUND;
                    sun->timer = 0;
                    sun->t = 0;
                }
            }

//            if (bolls[i].y >= bolls[i].target_y) {
//                bolls[i].timer++;
//                if (bolls[i].timer > 100) {
//                    bolls[i].isUsed = 0;
//                }
//            }
//        } else if (bolls[i].xOff) {
//            float dextY = 0;
//            float dextX = 262;
//            float angle = atan((bolls[i].y - dextY) / (bolls[i].x - dextX));
//            bolls[i].xOff = 10 * cos(angle);
//            bolls[i].yOff = 10 * sin(angle);
//            bolls[i].x -= bolls[i].xOff;
//            bolls[i].y -= bolls[i].yOff;
//            if (bolls[i].y <= 0 || bolls[i].x <= 262) {
//                bolls[i].isUsed = 0;
//                bolls[i].xOff = 0;
//                bolls[i].yOff = 0;
//                sunshine += 25;
        }
    }
}

//收集阳光
void collectSunshine(ExMessage *msg) {
    int count = sizeof(bolls) / sizeof(bolls[0]);
    int w = imageSunshine[0].getwidth();
    int h = imageSunshine[0].getheight();
    for (int i = 0; i < count; i++) {
        if (bolls[i].isUsed) {
            int x = bolls[i].pCur.x;
            int y = bolls[i].pCur.y;
            if (msg->x > x && msg->x < x + w && msg->y > y && msg->y < y + h) {
                PlaySound("open D:\\code\\clioncode\\untitled\\res\\an39o-vtxfb.wav",NULL,SND_FILENAME| SND_ASYNC);
//                bolls[i].isUsed = 0;
                bolls[i].status = SUNSHINE_COLLECT;
//                sunshine += 25;
                printf("%d\n", sunshine);
//                mciSendString("open D:\\code\\clioncode\\untitled\\res\\sunshine.mp3",0,0,0);
//                不用角度，用贝塞尔曲线
//                float dextY = 0;
//                float dextX = 262;
//                float angle = atan((bolls[i].y - dextY) / (bolls[i].x - dextX));
//                bolls[i].xOff = 10 * cos(angle);
//                bolls[i].yOff = 10 * sin(angle);
                bolls[i].p1 = bolls[i].pCur;//起点
                bolls[i].p4 = vector2(262, 0);//终点
                bolls[i].t = 0;
                float distance = dis(bolls[i].p1 - bolls[i].p4);
                float off = 8;
                bolls[i].speed = 1.0 / (distance / off);
            }
        }
    }
}

//创建阳光
void createSunshine() {
    static int count = 0;
    static int fre = 400;
    count++;
    int bollMax = sizeof(bolls) / sizeof(bolls[0]);
    if (count >= fre) {
        fre = 200 + rand() % 200;
        count = 0;
        //从阳光池中选一个阳光
        int i;
        for (i = 0; i < bollMax && bolls[i].isUsed; i++);
        if (i >= bollMax) return;
        bolls[i].isUsed = 1;
        bolls[i].frameIndex = 0;
        bolls[i].timer = 0;
        //使用贝塞尔曲线
//        bolls[i].x = 260 + rand() % 640;
//        bolls[i].y = 60;
//        bolls[i].target_y = 200 + (rand() % 4) * 90;
//        bolls[i].xOff = 0;
//        bolls[i].yOff = 0;
        bolls[i].status = SUNSHINE_DOWN;
        bolls[i].t = 0;
        bolls[i].p1 = vector2(260 + rand() % 640, 60);
        bolls[i].p4 = vector2(bolls[i].p1.x, 200 + (rand() % 4) * 90);
        int off = 2;
        float distance = bolls[i].p4.y - bolls[i].p1.y;
        bolls[i].speed = 1.0 / (distance - off);
    }
    //向日葵生产阳光
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 9; j++) {
            if (map[i][j].type == XIANG_RI_KUI + 1) {
                map[i][j].timer++;
                if (map[i][j].timer >= 200) {
                    map[i][j].timer = 0;
                    int k, w;
                    for (k = 0; k < bollMax && bolls[k].isUsed; k++);
                    if (k >= bollMax) return;
                    bolls[k].isUsed = 1;
                    bolls[k].p1 = vector2(map[i][j].x, map[i][j].y);
                    w = (25 + rand() % 50) * (rand() % 2 ? 1 : -1);
                    bolls[k].p4 = vector2(map[i][j].x + w, map[i][j].y + imgZhiWu[XIANG_RI_KUI][0]->getheight() -
                                                           imageSunshine[0].getheight());
                    bolls[k].p2 = vector2(bolls[k].p1.x + w * 0.3, bolls[k].p1.y - 100);
                    bolls[k].p3 = vector2(bolls[k].p1.x + w * 0.7, bolls[i].p1.y - 100);
                    bolls[k].status = SUNSHINE_PRODUCE;
                    bolls[k].speed = 0.05;
                    bolls[k].t = 0;
                }
            }
        }
    }
}

//启动菜单
void startUI() {
    IMAGE imageStart, imageMenu1, imageMenu2;
    loadimage(&imageStart, _T("D:\\code\\clioncode\\untitled\\res\\menu.png"));
    loadimage(&imageMenu1, _T("D:\\code\\clioncode\\untitled\\res\\menu1.png"));
    loadimage(&imageMenu2, _T("D:\\code\\clioncode\\untitled\\res\\menu2.png"));
    int flag = 0;
    while (1) {
        BeginBatchDraw();
        putimage(0, 0, &imageStart);
        putimagePNG(474, 75, flag ? &imageMenu2 : &imageMenu1);
        ExMessage msg;
        if (peekmessage(&msg)) {
            if (msg.message == WM_MOUSEMOVE) {
                if (msg.x > 474 && msg.x < 790 && msg.y > 75 && msg.y < 215) flag = 1;
                else flag = 0;
            } else if (msg.message == WM_LBUTTONUP && msg.x > 474 && msg.x < 790 && msg.y > 75 && msg.y < 215) {
                break;
            }
        }
        EndBatchDraw();
    }
}

int main() {
    gameInit();
    startUI();
    int timer = 0;
    bool flag = true;
    while (1) {
        userClick();
        timer += getDelay();
        if (timer > 40) {
            flag = true;
            timer = 0;
        }
        if (flag) {
            flag = false;
            updateWindow();
            updateGame();
        }
    }
    system("pause");
    return 0;
}