#include<stdio.h>
#include<graphics.h>//图形库
#include "tools.h"//消除黑边
#include<conio.h>
#define WIDTH 900
#define HEIGHT 600
enum {WAN_DOU, XIANG_RI_KUI,ZHI_WU_COUNT};//利用枚举实现数组数量灵活
IMAGE imaBg;//背景图片
IMAGE imaBar;//植物框图片
IMAGE imgCards[ZHI_WU_COUNT];
//加载游戏初始界面
void gameInit(){
    //加载图片
    loadimage(&imaBg,_T("D:\\code\\clioncode\\untitled\\res\\bg.jpg"));
    loadimage(&imaBar,_T("D:\\code\\clioncode\\untitled\\res\\bar5.png"));
    //初始化植物卡牌
    char name[64];
    for(int i=0;i<ZHI_WU_COUNT;i++){
        //生成植物卡牌文件名
        sprintf_s(name,sizeof(name),"D:\\code\\clioncode\\untitled\\res\\Cards\\card_%d.png",i+1);
        loadimage(&imgCards[i],_T(name));
    }
    //游戏图形框
    initgraph(WIDTH,HEIGHT);
}
void updateWindow(){
    //渲染图片
    putimage(0,0,&imaBg);
    putimagePNG(250,0,&imaBar);
    int width = 338;
    for(int i=0;i<ZHI_WU_COUNT;i++){
        putimagePNG(width,5,&imgCards[i]);
        width+=65;
    }
}
int main() {
    gameInit();
    updateWindow();
    system("pause");
    return 0;
}