#include <SPI.h>
#include <SD.h>
#include <Button2.h>
#include "RPi_Pico_TimerInterrupt.h"

#define BTN_PIN 20
#define X_pin 26
#define Y_pin 27
#define Z_pin 28
#define SD_CS   5
#define SD_MISO 4
#define SD_MOSI 3
#define SD_SCK  2

const int max_sample = 20000;
int data_x[max_sample], data_y[max_sample], data_z[max_sample];

volatile unsigned long interruptCounter = 0;
bool buffer_max = false;
bool isPressed = false;
bool start_job = false;
bool waitForDelay = false;
int fileIndex = 1;
File dataFile;
bool sdInitialized = false;

RPI_PICO_Timer ITimer(0);
Button2 button(BTN_PIN);

// LED 陣列 (GPIO 11-16)
int led_pin[6] = {11, 12, 13, 14, 15, 16};
bool ledState = HIGH;
unsigned long lastBlinkTime = 0;
int blinkCount = 0;
bool ledBlinking = false;
unsigned long delayStartTime = 0;

bool TimerHandler(struct repeating_timer *t) {
    if (interruptCounter < max_sample) {
        data_x[interruptCounter] = analogRead(X_pin);
        data_y[interruptCounter] = analogRead(Y_pin);
        data_z[interruptCounter] = analogRead(Z_pin);
        interruptCounter++;
    }
    if (interruptCounter >= max_sample) {
        interruptCounter = 0;
        buffer_max = true;
        ITimer.stopTimer();
    }
    return true;
}

void setup() {
    // Serial 初始化（可選）
    Serial.begin(115200);

    // 設定 LED 腳位
    for (int i = 0; i < 6; i++) {
        pinMode(led_pin[i], OUTPUT);
        digitalWrite(led_pin[i], HIGH);
    }

    pinMode(BTN_PIN, INPUT_PULLUP);

    // 🟡 Debug: 初始化前亮 LED[1]
    digitalWrite(led_pin[1], LOW);

    // 初始化 SD 卡
    initializeSDCard();

    // 🔵 Debug: SD 卡初始化後 LED[2]
    digitalWrite(led_pin[2], LOW);

    delay(1000);  // 避免 SD 卡一開始不穩

    // 初始化 Timer
    if (ITimer.attachInterruptInterval(50, TimerHandler)) {
        Serial.println("Timer 啟動成功 (20kHz)");
    } else {
        Serial.println("Timer 啟動失敗");
    }

    ITimer.stopTimer();

    button.setTapHandler([](Button2 &b) {
        isPressed = true;
        Serial.println("按鈕被按下");
    });

    // 🟢 Debug: setup 完成 LED[3]
    digitalWrite(led_pin[3], LOW);
}

void loop() {
    button.loop();

    if (ledBlinking && blinkCount < 5) {
        if (millis() - lastBlinkTime >= 1000) {
            lastBlinkTime = millis();
            ledState = !ledState;
            digitalWrite(led_pin[0], ledState);
            blinkCount++;
            if (blinkCount >= 5) {
                ledBlinking = false;
                digitalWrite(led_pin[0], HIGH);
                waitForDelay = true;
                delayStartTime = millis();
                Serial.println("閃爍結束，等待 15 分鐘後開始記錄...");
            }
        }
    }

    if (waitForDelay) {
        if (millis() - delayStartTime >= 1000) {
            waitForDelay = false;
            Serial.println("15 分鐘等待結束，開始記錄!");
            buffer_max = false;
            ITimer.restartTimer();
        }
    }

    if (isPressed && !start_job) {
        start_job = true;
        delay(500);
        digitalWrite(led_pin[0], HIGH);
        Serial.println("開始記錄 (LED 閃爍 5 次後等待 1 分鐘)");

        ledBlinking = true;
        blinkCount = 0;
        lastBlinkTime = millis();
    }

    if (buffer_max) {
        Serial.println("開始存檔");
        SaveFile();
        buffer_max = false;
    }
}

void SaveFile() {
    if (!sdInitialized) {
        Serial.println("SD 卡未初始化!");
        return;
    }

    String filename = String(fileIndex) + ".csv";
    dataFile = SD.open(filename.c_str(), FILE_WRITE);
    if (dataFile) {
        dataFile.println("timestamp,x,y,z");
        for (int i = 0; i < max_sample; i++) {
            dataFile.print(i); dataFile.print(",");
            dataFile.print(data_x[i]); dataFile.print(",");
            dataFile.print(data_y[i]); dataFile.print(",");
            dataFile.println(data_z[i]);
        }
        dataFile.close();
        Serial.println("存檔完成: " + filename);
    } else {
        Serial.println("無法開啟檔案");
    }
    fileIndex++;
    ITimer.restartTimer();
}

void initializeSDCard() {
    SPI.setSCK(SD_SCK);
    SPI.setTX(SD_MOSI);
    SPI.setRX(SD_MISO);

    delay(500);  // 加一點穩定時間

    if (!SD.begin(SD_CS)) {
        sdInitialized = false;
        Serial.println("SD 卡初始化失敗!");
        return;
    }

    sdInitialized = true;
    Serial.println("SD 卡初始化成功! 等待按鈕開始...");
}
