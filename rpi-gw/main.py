from bluepy.btle import Peripheral, Scanner
import time
import os
import subprocess

if __name__ == "__main__":
    device = Peripheral("78:e3:6d:19:43:1a")

    try:
        while True:
            characteristics = device.getCharacteristics()

            for characteristic in characteristics:
                print("Handle: {0}, UUID: {1}, UUID Name: {2}".format(
                    characteristic.handle, 
                    characteristic.uuid.binVal.hex(),
                    characteristic.uuid.getCommonName()), end="")
                
                if characteristic.supportsRead():
                    print(", Value: {0}".format(characteristic.read().hex()), end="")
                print()

            time.sleep(2)
            print()
    except Exception as exception:
        print("Exception:", exception)
    finally:
        device.disconnect()