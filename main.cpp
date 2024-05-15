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
struct zhiwu{
    int type;
    int frameIndex;//序列帧的序号
};
struct zhiwu map[3][9];
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
    memset(map,0,sizeof(map));
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
    //渲染种植的植物
    for(int i =0;i<3;i++){
        for(int j=0;j<9;j++){
            if(map[i][j].type>0){//需要判断，否侧直接崩
                int x=256+81*j;
                int y=179+i*102;
                putimagePNG(x,y,imgZhiWu[map[i][j].type-1][map[i][j].frameIndex]);
            }
        }
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
            //确定种在几行几列
            if(msg.x>256&&msg.y>179&&msg.y<489){
                int row = (msg.y-179)/102;
                int col = (msg.x-256)/81;
                printf("%d %d\n",row,col);
                if(map[row][col].type==0){
                    map[row][col].type=curZhiWu;
                    map[row][col].frameIndex=0;
                }

            }
            curZhiWu = 0;
            status = 0;
        }
    }
}
//改变游戏相关数据
void updateGame(){
    for(int i=0;i<3;i++){
        for(int j=0;j<9;j++){
            if(map[i][j].type){
                map[i][j].frameIndex++;
                if(imgZhiWu[map[i][j].type-1][map[i][j].frameIndex]==NULL){
                    map[i][j].frameIndex=0;
                }
            }
        }
    }
}
//启动菜单
void startUI(){
    IMAGE imageStart,imageMenu1,imageMenu2;
    loadimage(&imageStart,_T("D:\\code\\clioncode\\untitled\\res\\menu.png"));
    loadimage(&imageMenu1,_T("D:\\code\\clioncode\\untitled\\res\\menu1.png"));
    loadimage(&imageMenu2,_T("D:\\code\\clioncode\\untitled\\res\\menu2.png"));
    int flag=0;
    while(1){
        BeginBatchDraw();
        putimage(0,0,&imageStart);
        putimagePNG(474,75,flag ? &imageMenu2:&imageMenu1);
        ExMessage msg;
        if(peekmessage(&msg)){
             if(msg.message==WM_MOUSEMOVE){
                if(msg.x>474&&msg.x<774&&msg.y>75&&msg.y<215) flag=1;
                else flag=0;
            }else if(msg.message==WM_LBUTTONUP&&msg.x>474&&msg.x<774&&msg.y>75&&msg.y<215){
                break;
            }
        }
        EndBatchDraw();
    }
}
int main() {
    gameInit();
    startUI();
    int timer=0;
    bool flag = true;
    while(1){
        userClick();
        timer+=getDelay();
        if(timer>25){
            flag= true;
            timer = 0;
        }
        if(flag){
            flag = false;
            updateWindow();
            updateGame();
        }
    }
    system("pause");
    return 0;
}