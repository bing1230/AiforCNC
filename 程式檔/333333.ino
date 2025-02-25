#include <SD.h> // 引入SD卡函式庫
#define _TIMERINTERRUPT_LOGLEVEL_ 4
#include "RPi_Pico_TimerInterrupt.h"

#include <Button2.h>
#define BTN_PIN 20
Button2 button(BTN_PIN);    // 按鈕實例
// 使用 RPI_PICO_Timer 來設定 Timer 0
RPI_PICO_Timer ITimer(0);
bool isPressed = false;//按鈕按下了沒
/*     這邊放數據採集相關的變數          */
#define X_pin 26
#define Y_pin 27
#define Z_pin 28
int max_sample =20000;

int data_x[20000];
int data_y[20000];
int data_z[20000];
bool buffer_max = false; //判斷是不是兩萬筆滿了
// 中斷計數器
volatile unsigned long interruptCounter = 0;//計數器
unsigned long lastTapTime = 0;          // 上次單擊的時間
const int chipSelect = 5;  // 定義SD卡的CS引腳
int sampleCount = 0; // 採樣計數器
int fileIndex = 1;  // 檔案索引，從1開始命名
File dataFile;  // 用來儲存檔案的File物件
bool sdInitialized = false; // 用來檢查SD卡是否初始化成功
bool start_job = false;//拿來卡按鈕的
// 中斷處理函式
bool TimerHandler(struct repeating_timer *t) {
    if(buffer_max == false){//如果採樣還沒滿兩萬
      data_x[interruptCounter] = analogRead(X_pin);
      data_y[interruptCounter] = analogRead(Y_pin);
      data_z[interruptCounter] = analogRead(Z_pin);
      interruptCounter++;
    }
    if(interruptCounter>= max_sample){
      interruptCounter =0;
      buffer_max = true;
    }
    return true;
  }
void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT); // 設定按鈕為輸入
  // 嘗試初始化SD卡
  initializeSDCard();
  delay(1000); // 給序列埠初始化時間
  Serial.println("Starting TimerInterrupt at 20kHz");
  // 設定 Timer0 為 1000 微秒間隔（1ms = 1000Hz）
  //1s = 1000000us
  //一次週期是50/1000000s 
  //頻率是 1000000/50 Hz
  if (ITimer.attachInterruptInterval(50, TimerHandler)) {
    Serial.println("Timer setup successfully at 1000Hz");
    } 
  else {
    Serial.println("Failed to setup timer!");
    }
  ITimer.stopTimer(); // 啟動時不立即運行定時器，等待按鈕觸發
  // 設置按鈕事件
  button.setTapHandler([](Button2& b) {
    lastTapTime = millis();  // 更新最後點擊時間
    isPressed = true;//按鈕被按下去
    Serial.println("單點ㄌ");
  });
}

void loop() {
    button.loop();  // 處理按鈕事件
    if(isPressed  == true&& start_job == false){//判斷是不是被按鈕按下去了
      start_job = true;
      delay(100);
      Serial.println("開始記錄");
      ITimer.restartTimer();//啟動定時器
      buffer_max = false;//清除buffer滿了沒的狀態
    }
    if(buffer_max ==true)
    {
      ITimer.stopTimer();//關閉計時器
      Serial.println("開始存檔");
      SaveFile();
      buffer_max =false;//清掉 再來一次
    }
  }
// 創建新檔案並開啟  如果是第一次寫入數據，先寫入標題行
void SaveFile() {
  // 創建新檔案並開啟
  String filename = String(fileIndex) + ".csv"; // 檔案名稱: 1.csv, 2.csv, 3.csv, ...
  dataFile = SD.open(filename.c_str(), FILE_WRITE);
  
  if (dataFile) {
    // 如果是第一次寫入數據，先寫入標題行
    dataFile.println("time,x,y,z"); // 寫入標題
    Serial.println("開啟新檔案: " + filename);
  } else {
    Serial.println("無法開啟檔案進行寫入!");
  }
  for(int i=0;i<max_sample;i++){
    dataFile.print(i);
    dataFile.print(",");
    dataFile.print(data_x[i]);
    dataFile.print(",");
    dataFile.print(data_y[i]);
    dataFile.print(",");
    dataFile.println(data_z[i]);
  }
  fileIndex++;
  ITimer.restartTimer();//啟動定時器
  Serial.println("存檔ㄌ");

}
//SD卡初始化是否成功失敗提示 是否開始採樣
void initializeSDCard() {
  if (!SD.begin(chipSelect)) {
    sdInitialized = false;
    Serial.println("SD卡初始化失敗!");
  } else {
    sdInitialized = true;
    Serial.println("SD卡初始化成功!");
    Serial.println("準備好了，按下按鈕開始採樣!");
  }
}