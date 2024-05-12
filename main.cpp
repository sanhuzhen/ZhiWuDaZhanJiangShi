#include<stdio.h>
#include<graphics.h>//图形库
#include "tools.h"//消除黑边
#define WIDTH 900
#define HEIGHT 600
enum {WAN_DOU, XIANG_RI_KUI,ZHI_WU_COUNT};//利用枚举实现数组数量灵活
IMAGE imaBg;//背景图片
IMAGE imaBar;//植物框图片
IMAGE imgCards[ZHI_WU_COUNT];//植物卡牌数组
IMAGE* imgZhiWu[ZHI_WU_COUNT][20];//各种植物的动图

int curX,curY;//当前选中的植物，在移动过程中的位置坐标
int curZhiWu;
//判断文件是否存在
bool fileExist(const char* name){
    FILE* fp = fopen(name,"r");
    if(fp==NULL) return false;
    else{
        fclose(fp);
        return true;
    }
}
//加载游戏初始界面
void gameInit(){
    //加载图片
    loadimage(&imaBg,_T("D:\\code\\clioncode\\untitled\\res\\bg.jpg"));
    loadimage(&imaBar,_T("D:\\code\\clioncode\\untitled\\res\\bar5.png"));

    memset(imgZhiWu,0,sizeof(imgZhiWu));
    //初始化植物卡牌
    char name[64];
    for(int i=0;i<ZHI_WU_COUNT;i++){
        //生成植物卡牌文件名
        sprintf_s(name,sizeof(name),"D:\\code\\clioncode\\untitled\\res\\Cards\\card_%d.png",i+1);
        loadimage(&imgCards[i],_T(name));

        for(int j=0;j<20;j++){
            sprintf_s(name,sizeof(name),"D:\\code\\clioncode\\untitled\\res\\zhiwu\\%d\\%d.png",i,j+1);
            //先判断文件是否存在
            if(fileExist(name)){
                //分配内存
                imgZhiWu[i][j] = new IMAGE;
                loadimage(imgZhiWu[i][j],name);
            }else{
                break;
            }
        }
    }
    curZhiWu=0;
    //游戏图形框
    initgraph(WIDTH,HEIGHT,1);
}
//渲染图片
void updateWindow(){
    BeginBatchDraw();//开始双缓冲
    putimage(0,0,&imaBg);
    putimagePNG(250,0,&imaBar);
    int width = 338;
    for(int i=0;i<ZHI_WU_COUNT;i++){
        putimagePNG(width,5,&imgCards[i]);
        width+=65;
    }
    //渲染拖动过程的植物
    if(curZhiWu){
        IMAGE* img = imgZhiWu[curZhiWu-1][0];
        putimagePNG(curX-img->getwidth()/2,curY-img->getheight()/2,img);
    }
    EndBatchDraw();//结束双缓冲
}
//用户点击功能
void userClick(){
    //判断鼠标是否传来消息
    ExMessage msg;
    static int status = 0;//是否选择成功
    if(peekmessage(&msg)){
        if(msg.message == WM_LBUTTONDOWN){//判断是否是鼠标左键点击
            if(msg.x>338&&msg.x<338+65*ZHI_WU_COUNT&&msg.y>5&&msg.y<96){
                //判断是第几张卡牌
                int index = (msg.x - 338)/65;
                printf("%d\n",index);//打印，避免出错
                status = 1;
                curZhiWu=index+1;
            }
        }else if(msg.message == WM_MOUSEMOVE&&status==1){//判断是否鼠标移动
            curX = msg.x;
            curY = msg.y;

        }else if(msg.message == WM_LBUTTONUP){//判断鼠标左键是否松开

        }
    }
}
int main() {
    gameInit();
    while(1){
        userClick();
        updateWindow();
    }
    system("pause");
    return 0;
}