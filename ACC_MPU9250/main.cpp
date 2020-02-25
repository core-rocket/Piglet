/*
*加速度センサとBMP280による離床判定と頂点判定のテストプログラム
*LPC1768
*GY-91
*
//////新車書に記載した離床判定//////
A) フライトピンが抜けた
B) 加速度センサの3軸の合成値(5つおきに中央値をとったもの)が3[G]を超えた状態が連続で5回続いた
以上の条件のどちらか片方が満たされた時

フライトピンが抜けたかどうか判断するbool値。　FRIGHT_PIN
加速度センサが離床を検出したかを判断するbool値。　FLIGHT_ACC
///////////////////////////////
↓↓↓↓↓
//////審査書に記載した頂点判定//////
A) 気圧センサから得られる高度の値気圧の値(5つおきに中央値をとったもの)が連続して5回以上減少上昇した時
B) SIMによって得られた頂点到達時間(15秒)経過した時
C) 燃焼実験の計測で得られた燃焼完了時間（5秒）経過した時。
Cの条件が満たされ,かつAとBのいずれかが満たされた時

気圧センサによって頂点検知をしたbool値　TOP
///////////////////////////////

今回は実験のため、時間による離床判定および、頂点判定は行わない。
*/
#include "mbed.h"
#include "MPU9250.h"
#include "BME280.h"

DigitalOut myled(LED1);
Serial pc(USBTX,USBRX, 115200);
MPU9250 mpu = MPU9250(p9,p10);
BME280 sensor(p28,p27);

uint8_t whoami;
int16_t acc[3];
float ax, ay, az;
bool FLIGHT_ACC=false;
bool TOP=false;

void median(int LC,int RC,float BC[1][5]);

int main(){
    int pcount=0;
    int acount=0;
    float pres[5][5]={0};
    float zacc[5][5]={0};
    int proopc=0;
    int aroopc=0;
    int plastc=-1;
    int alastc=-1;
    wait(0.3);
    whoami = mpu.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
    pc.printf("I AM 0x%x\n\r", whoami); //0x71
    if (whoami == 0x71)
    {  
        pc.printf("MPU9250 is detected.\n\r");
        wait(0.3);
        mpu.resetMPU9250();
        mpu.initMPU9250(); 
        wait(0.3);
        myled = 1;
        pc.printf("Start.\n\r");
        mpu.getAres();
        wait(0.3);
   }
   else
   {
        pc.printf("Could not detect MPU9250.\n\r");
        while(1);
    }
//離床判定
    pc.printf("Start JUDGE.\n\r");
    while(FLIGHT_ACC!=true)
    {
        alastc++;
        if(aroopc==5)aroopc=0;
        pc.printf("Acc: %f, %f, %f\n\r", ax, ay, az);
        mpu.readAccelData(acc);
        zacc[aroopc][alastc]=acc[2]+acc[1]+acc[0] * 16.0 / 32768.0;//３軸の合成値
        if(alastc==4)
        {
            alastc=0;
            void median(int alastc,int aroopc,float zacc);
            if(aroopc!=0&&zacc[aroopc-1][2]>3.0)
            {
                acount++;
            }
            else
            {
                acount=0;
            }
            aroopc++;
        } 
        if(acount==5)
        {
            FLIGHT_ACC=true;
            pc.printf("FLIFHT_ACC success!!\n");
        }
    }
        
//頂点判定
    while(TOP!=true) 
    {
        plastc++;
        if(proopc==5)proopc=0;
        pc.printf("%04.2f hPa, count %d  \r\n", sensor.getPressure(),pcount);
        pres[proopc][plastc]=sensor.getPressure();
        if(plastc==4)
        {
            plastc=0;
            void median(int plastc,int proopc,float pres);
            if(proopc!=0&&pres[proopc-1][2]>pres[proopc][2])
            {
                pcount++;
            }
            else
            {
                pcount=0;
            }
            proopc++;
        } 
        if(pcount==5)
        {
            TOP=true;
            pc.printf("TOP_JUDGE success!!\n");
        }
    }
}
//バブルソート
void median(int LC, int RC, float BC[1][5])
{  
    int k;
    int m;
    float temp;
            for(k=0; k<5; k++)
            {
                for(m=5-1; m>LC; m--)
                {
                    if(BC[RC][m] < BC[RC][m-1])
                    {
                        temp = BC[RC][m];
                        BC[RC][m] = BC[RC][m-1];
                        BC[RC][m-1] = temp;
                    }
                }
            }
}