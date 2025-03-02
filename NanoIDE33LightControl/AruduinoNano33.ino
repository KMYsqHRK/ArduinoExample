/*
 * Arduino Nano 33 IoT - BLE デバッグ版 (修正版)
 * Bluetooth接続問題のトラブルシューティング用
 */

#include <ArduinoBLE.h>

const int LED_PIN = LED_BUILTIN;  // Arduino Nano 33 IoTの内蔵LED
const char* DEVICE_NAME = "Arduino_Nano33IoT_BLE";  // 設定するデバイス名

// BLEサービスとキャラクタリスティックを定義
BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLEByteCharacteristic ledCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

void setup() {
  // シリアル通信の初期化（速度を上げる）
  Serial.begin(115200);
  
  // 起動時の目印として内蔵LEDを一度点滅させる
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("=== Arduino Nano 33 IoT BLE デバッグ版 ===");
  Serial.println("起動中...");
  
  // 3秒待機（シリアルモニターを開く時間を確保）
  delay(3000);
  
  // BLEの初期化（複数回試行）
  int attempts = 0;
  const int maxAttempts = 5;
  
  while (attempts < maxAttempts) {
    Serial.print("BLEの初期化を試行中... (");
    Serial.print(attempts + 1);
    Serial.print("/");
    Serial.print(maxAttempts);
    Serial.print(") ");
    
    if (BLE.begin()) {
      Serial.println("成功!");
      break;
    } else {
      Serial.println("失敗");
      // エラー時は短く3回点滅
      for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
      }
      attempts++;
      delay(1000);
    }
  }
  
  if (attempts >= maxAttempts) {
    Serial.println("BLEの初期化に失敗しました。ハードウェアに問題がある可能性があります。");
    Serial.println("リセットボタンを押して再起動するか、または電源を入れ直してください。");
    // エラーを示すために点滅を続ける
    while (1) {
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(200);
    }
  }
  
  // デバイスのMACアドレスなど、詳細情報を表示
  Serial.print("BLEデバイスアドレス: ");
  Serial.println(BLE.address());
  
  // デバイス名を設定（ここでlocalNameメソッドを使用せず直接表示）
  Serial.print("設定するBLEデバイス名: ");
  Serial.println(DEVICE_NAME);
  BLE.setLocalName(DEVICE_NAME);
  
  Serial.println("BLEサービスを設定中...");
  BLE.setAdvertisedService(ledService);
  
  // キャラクタリスティックをサービスに追加
  ledService.addCharacteristic(ledCharacteristic);
  
  // サービスを追加
  BLE.addService(ledService);
  
  // キャラクタリスティックの初期値を設定
  ledCharacteristic.writeValue(0);
  
  // アドバタイズ設定を表示
  Serial.print("アドバタイズ間隔: ");
  Serial.println("ms");
  
  // アドバタイズ開始（間隔を短く設定）
  BLE.setAdvertisingInterval(100);
  
  Serial.println("アドバタイズを開始します...");
  if (BLE.advertise()) {
    Serial.println("BLEアドバタイズ開始成功!");
  } else {
    Serial.println("BLEアドバタイズ開始失敗...");
    // エラー時は長く2回点滅
    while (1) {
      digitalWrite(LED_PIN, HIGH);
      delay(500);
      digitalWrite(LED_PIN, LOW);
      delay(500);
    }
  }
  
  // 接続準備完了のサイン（ゆっくり2回点滅）
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(300);
    digitalWrite(LED_PIN, LOW);
    delay(300);
  }
  
  Serial.println("BLE接続の準備完了！");
  Serial.print("スマートフォンまたはPCからデバイス「");
  Serial.print(DEVICE_NAME);
  Serial.println("」を探してください。");
  Serial.println("接続待機中...");
}

void loop() {
  // 定期的にステータスを表示（1分ごと）
  static unsigned long lastStatusTime = 0;
  if (millis() - lastStatusTime > 60000) {
    Serial.print("BLE接続を待機中... デバイス名: ");
    Serial.println(DEVICE_NAME);
    lastStatusTime = millis();
    
    // アイドル中は1秒ごとに短く点滅させてアクティブなことを示す
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
  }
  
  // BLE接続を待機
  BLEDevice central = BLE.central();
  
  // 接続されたら
  if (central) {
    Serial.print("接続されました: ");
    Serial.println(central.address());
    
    // 接続中は点滅させて接続状態を示す
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    
    // 接続の開始時刻を記録
    unsigned long connectionStartTime = millis();
    
    // 接続されている間
    while (central.connected()) {
      // 定期的に接続状態を表示
      static unsigned long lastConnectionTime = 0;
      if (millis() - lastConnectionTime > 10000) {  // 10秒ごと
        Serial.print("接続中... 経過時間: ");
        Serial.print((millis() - connectionStartTime) / 1000);
        Serial.println("秒");
        lastConnectionTime = millis();
      }
      
      // キャラクタリスティックが書き込まれたら
      if (ledCharacteristic.written()) {
        // 値を読み取る
        int ledValue = ledCharacteristic.value();
        
        Serial.print("受信したコマンド: ");
        Serial.println(ledValue);
        
        // LEDを制御
        if (ledValue == 1) {
          digitalWrite(LED_PIN, HIGH);  // LED ON
          Serial.println("LED ON");
        } else {
          digitalWrite(LED_PIN, LOW);   // LED OFF
          Serial.println("LED OFF");
        }
      }
    }
    
    // 切断された
    Serial.print("切断されました: ");
    Serial.println(central.address());
  }
}