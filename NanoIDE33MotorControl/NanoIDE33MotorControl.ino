/*
 * Arduino Nano 33 IoT - Bluetooth LE モータ制御 (シンプル版)
 * Bluetooth接続でL293モータドライバを制御するスケッチ
 * モータコマンド: 0=停止, 1=正回転, 2=負回転
 */

#include <ArduinoBLE.h>

// ピン定義
const int LED_PIN = LED_BUILTIN;  // 内蔵LED (ステータス表示用)
const int MOTOR_PIN1 = 7;         // L293のIN1に接続 (D7)
const int MOTOR_PIN2 = 8;         // L293のIN2に接続 (D8)

// デバイス名設定
const char* DEVICE_NAME = "Arduino_Motor_Controller";

// BLEサービスとキャラクタリスティックを定義
BLEService motorService("19B10000-E8F2-537E-4F6C-D104768A1214");  // サービスUUID

// モータ制御用キャラクタリスティック (0=停止, 1=正回転, 2=負回転)
BLEByteCharacteristic motorCharacteristic("19B10002-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

void setup() {
  // シリアル通信の初期化（速度を上げる）
  Serial.begin(115200);
  
  // ピンを出力モードに設定
  pinMode(LED_PIN, OUTPUT);
  pinMode(MOTOR_PIN1, OUTPUT);
  pinMode(MOTOR_PIN2, OUTPUT);
  
  // 初期状態は全て停止
  digitalWrite(LED_PIN, LOW);    // LED消灯
  digitalWrite(MOTOR_PIN1, LOW); // モータ停止
  digitalWrite(MOTOR_PIN2, LOW); // モータ停止
  
  // 起動時の目印として内蔵LEDを一度点滅させる
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("=== Arduino Nano 33 IoT BLE モータ制御 ===");
  Serial.println("起動中...");
  
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
      // エラー時は短く点滅
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
  
  // デバイス名を設定
  Serial.print("設定するBLEデバイス名: ");
  Serial.println(DEVICE_NAME);
  BLE.setLocalName(DEVICE_NAME);
  
  Serial.println("BLEサービスを設定中...");
  BLE.setAdvertisedService(motorService);
  
  // キャラクタリスティックをサービスに追加
  motorService.addCharacteristic(motorCharacteristic);
  
  // サービスを追加
  BLE.addService(motorService);
  
  // キャラクタリスティックの初期値を設定
  motorCharacteristic.writeValue(0);  // モータ初期状態: 停止
  
  // アドバタイズ間隔を短く設定（検出されやすくする）
  BLE.setAdvertisingInterval(100);
  
  Serial.println("アドバタイズを開始します...");
  if (BLE.advertise()) {
    Serial.println("BLEアドバタイズ開始成功!");
  } else {
    Serial.println("BLEアドバタイズ開始失敗...");
    // エラー時は点滅
    while (1) {
      digitalWrite(LED_PIN, HIGH);
      delay(500);
      digitalWrite(LED_PIN, LOW);
      delay(500);
    }
  }
  
  // 準備完了のサイン（ゆっくり2回点滅）
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(300);
    digitalWrite(LED_PIN, LOW);
    delay(300);
  }
  
  Serial.println("BLE接続の準備完了！");
  Serial.println("Bluetooth操作の使用方法:");
  Serial.println("- モータコントロール: 0=停止, 1=正回転, 2=負回転");
  Serial.println("接続待機中...");
}

// モータの制御関数
void controlMotor(int command) {
  switch (command) {
    case 0:  // 停止
      digitalWrite(MOTOR_PIN1, LOW);
      digitalWrite(MOTOR_PIN2, LOW);
      Serial.println("モータ: 停止");
      break;
      
    case 1:  // 正回転
      digitalWrite(MOTOR_PIN1, HIGH);
      digitalWrite(MOTOR_PIN2, LOW);
      Serial.println("モータ: 正回転");
      break;
      
    case 2:  // 負回転
      digitalWrite(MOTOR_PIN1, LOW);
      digitalWrite(MOTOR_PIN2, HIGH);
      Serial.println("モータ: 負回転");
      break;
      
    default:  // 不明なコマンドの場合は停止
      digitalWrite(MOTOR_PIN1, LOW);
      digitalWrite(MOTOR_PIN2, LOW);
      Serial.println("モータ: 不明なコマンド - 停止");
      break;
  }
}

void loop() {
  // 定期的にステータスを表示（30秒ごと）
  static unsigned long lastStatusTime = 0;
  if (millis() - lastStatusTime > 30000) {
    Serial.print("BLE接続を待機中... デバイス名: ");
    Serial.println(DEVICE_NAME);
    lastStatusTime = millis();
    
    // アイドル中は短く点滅させてアクティブなことを示す
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
      // モータ制御用キャラクタリスティックが書き込まれたら
      if (motorCharacteristic.written()) {
        // 値を読み取る
        int motorValue = motorCharacteristic.value();
        
        Serial.print("モータ制御コマンド受信: ");
        Serial.println(motorValue);
        
        // モータを制御
        controlMotor(motorValue);
      }
    }
    
    // 切断された時はモータを停止させる
    controlMotor(0);
    
    // 切断された
    Serial.print("切断されました: ");
    Serial.println(central.address());
  }
}