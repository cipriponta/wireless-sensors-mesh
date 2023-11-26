from bluepy.btle import Peripheral
import time

ESP32_GAS_BLE_MAC = "78:e3:6d:19:43:1a"
ESP32_TMP_BLE_MAC = "a8:42:e3:48:32:2e"
CHARACTERISTIC_NAME = "ffcd"

def read_characteristics(device):
    characteristics = device.getCharacteristics()

    for characteristic in characteristics:
        if CHARACTERISTIC_NAME == characteristic.uuid.getCommonName() and characteristic.supportsRead():
            print("Value: {0}".format(characteristic.read().hex()))
            print()

if __name__ == "__main__":
    gas_sensor = Peripheral(ESP32_GAS_BLE_MAC)
    tmp_sensor = Peripheral(ESP32_TMP_BLE_MAC)

    try:
        while True:
            read_characteristics(gas_sensor)
            read_characteristics(tmp_sensor)
            time.sleep(1)
            
    except Exception as exception:
        print("Exception:", exception)
    finally:
        gas_sensor.disconnect()
        tmp_sensor.disconnect()