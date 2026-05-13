/*   Ramen timer  Sleep付
     for Attiny402
*/
#include <avr/sleep.h>
const int INPUT_PIN = 5;// プッシュボタンpin
const int COMON_PIN = 4;// LEDのpin
const int LED_PIN[3] = {0,1,2};// LEDのpin
unsigned long Timers[3] = {178,238,298};//  3つのタイマー設定時間ms（-2秒くらいの値がちょうどいい）
const int BUZZ_PIN = 3;// 圧電ブザーのpin
unsigned long StartMills = 0;
unsigned long NowMills = 0;
byte Tmode;// タイマーの番号 -> LED_PIN[x],Timers[x]
bool Cmode;// タイマーのカウントダウンを開始できるかのフラグ
const unsigned long wokingTime = 10000;// 10秒でスリープ状態になる
unsigned long tm = 0;
unsigned long StartMillis = 0;
unsigned long NowMillis = 0;

ISR(PORTA_PORT_vect) {
  PORTA.INTFLAGS = PORT_INT0_bm; //clear the interrupt flag on PAn
}

void setup() {
  pinMode(INPUT_PIN,INPUT_PULLUP);
  pinMode(COMON_PIN,OUTPUT);
  for(int i =0; i<3; i++) {
    pinMode(LED_PIN[i],OUTPUT);
  }
  Tmode = 0;
  Cmode = false;
  LED_reset();
  tm = millis() + wokingTime;
}

void loop() {
  if(tm < millis()) {
    digitalWrite(LED_PIN[Tmode],LOW);
    goSleep();
  } 

  if(digitalRead(INPUT_PIN) == LOW) {// ボタンを押すとタイマーを切り替えて、カウントダウンを有効にする
    tone(BUZZ_PIN,1000,50);
    tm = millis() + wokingTime;
    Tmode++;
    Cmode = true;
    if(Tmode>2) Tmode=0;
    for(int i =0; i<3; i++){
      digitalWrite(LED_PIN[i],LOW);
    }
    digitalWrite(LED_PIN[Tmode],HIGH);
    delay(250);
    StartMills = millis() + 2000;// ボタンを押して2秒経つとカウントダウンが開始する
  }
    NowMills = millis();
    if(Cmode == true && NowMills > StartMills)
    {
      timers();    
    }
}

void timers() {// カウントダウンをする、設定した時間が経ったら音を出す
  tone(BUZZ_PIN,2000,50);
  digitalWrite(COMON_PIN,HIGH);
  for(int i =0; i<3; i++){
    digitalWrite(LED_PIN[i],HIGH);
  }
  digitalWrite(LED_PIN[Tmode],LOW);

  StartMills = millis() + Timers[Tmode] * 1000;
  while(NowMills < StartMills)
  {
    NowMills = millis();
    digitalWrite(LED_PIN[Tmode],HIGH);// カウントダウン中は、LEDをゆっくりと点滅させる
    delay(1000);
    digitalWrite(LED_PIN[Tmode],LOW);
    delay(1000);
    if(digitalRead(INPUT_PIN) == LOW) break;// ボタンを押すとカウントダウンを停止
  }
  for(int i=0; i<1000; i++){
   if(digitalRead(INPUT_PIN) == LOW) break;// ボタンを押すとブザー音を停止
   tone(BUZZ_PIN,2500,500);
   delay(1000);
  }
  while(true){
   if(digitalRead(INPUT_PIN) == LOW) break;// 動作停止、ボタンを押すまでループ
   if(tm < millis()) goSleep(); 
   delay(500);
  }
  LED_reset();
  delay(1000);
}

void LED_reset() {// LEDを消灯して、カウントダウンを無効にする
 tone(BUZZ_PIN,300,100);
 digitalWrite(COMON_PIN,LOW);
  for(int i =0; i<3; i++){
    digitalWrite(LED_PIN[i],LOW);
  }
  digitalWrite(LED_PIN[Tmode],HIGH);
  Cmode = false;
}

void goSleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //select the power down sleep mode
  noInterrupts(); //disable all interrupts
  sleep_enable();
  interrupts(); //enable all interrupts
  PORTA.PIN0CTRL = PORT_PULLUPEN_bm | PORT_ISC_LEVEL_gc; //pull up PAn, trigger on low level
  sleep_cpu(); //actually go to sleep here

  //the program will continue after waking up from here
  sleep_disable();
  PORTA.PIN0CTRL = PORT_PULLUPEN_bm; //pull up PAn, turn off the pin change interrupt
  tm = millis() + wokingTime;
  delay(500);
}


