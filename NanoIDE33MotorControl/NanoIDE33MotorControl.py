"""
Arduino Nano 33 IoT BLEモータ制御クライアント（シンプル版）
PCからArduinoのモータをBluetooth LEで制御するためのPythonスクリプト
"""

import asyncio
from bleak import BleakClient, BleakScanner
import sys

# Arduinoで定義した同じUUID
MOTOR_SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214"
MOTOR_CHARACTERISTIC_UUID = "19B10002-E8F2-537E-4F6C-D104768A1214"

# Arduino Nano 33 IoTのデバイス名
DEVICE_NAME = "Arduino_Motor_Controller"

async def find_arduino():
    """Arduino モータコントローラーを探索する"""
    print(f"{DEVICE_NAME}を探しています...")
    
    devices = await BleakScanner.discover()
    
    for device in devices:
        if device.name == DEVICE_NAME:
            print(f"デバイスを発見しました: {device.name} [{device.address}]")
            return device.address
            
    return None

async def run():
    """メイン処理"""
    # デバイスアドレスを取得
    address = await find_arduino()
    
    if not address:
        print(f"{DEVICE_NAME}が見つかりませんでした。")
        print("Arduinoスケッチが実行されていることを確認してください。")
        return
    
    try:
        # デバイスに接続
        async with BleakClient(address) as client:
            print(f"{DEVICE_NAME}に接続しました")
            
            print("\n===== Arduino モータコントローラー =====")
            print("コマンド一覧:")
            print("  0: モータを停止する")
            print("  1: モータを正回転させる")
            print("  2: モータを負回転させる")
            print("  q: プログラムを終了する")
            
            while True:
                # ユーザー入力を受け取る
                command = input("\nコマンドを入力してください (0/1/2/q): ")
                
                if command.lower() == 'q':
                    print("モータを停止して終了します...")
                    # 終了前にモータを停止させる
                    await client.write_gatt_char(MOTOR_CHARACTERISTIC_UUID, bytearray([0]))
                    break
                    
                elif command == '0':
                    # モータ停止コマンドを送信
                    await client.write_gatt_char(MOTOR_CHARACTERISTIC_UUID, bytearray([0]))
                    print("モータ停止コマンドを送信しました")
                    
                elif command == '1':
                    # モータ正回転コマンドを送信
                    await client.write_gatt_char(MOTOR_CHARACTERISTIC_UUID, bytearray([1]))
                    print("モータ正回転コマンドを送信しました")
                    
                elif command == '2':
                    # モータ負回転コマンドを送信
                    await client.write_gatt_char(MOTOR_CHARACTERISTIC_UUID, bytearray([2]))
                    print("モータ負回転コマンドを送信しました")
                    
                else:
                    print("無効なコマンドです。0, 1, 2, または q を入力してください。")
                    
    except Exception as e:
        print(f"エラーが発生しました: {e}")
        print("デバイスが接続範囲内にあることを確認し、再度試してください。")

if __name__ == "__main__":
    # メインループ実行
    asyncio.run(run())