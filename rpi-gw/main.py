from bluepy.btle import Peripheral
import RPi.GPIO as GPIO
import time
import logging
import threading

FAN_PWN_PIN = 12
ESP32_GAS_BLE_MAC = "78:e3:6d:19:43:1a"
ESP32_TMP_BLE_MAC = "a8:42:e3:48:32:2e"
CHARACTERISTIC_NAME = "ffcd"
TMP_THRESHOLD = 30.0
GAS_THRESHOLD = 900

def gpio_init():
    GPIO.setwarnings(False)
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(FAN_PWN_PIN, GPIO.OUT)

def gpio_clean():
    GPIO.cleanup()

def gpio_manage_fan(fan_state):
    if fan_state is True:
        GPIO.output(FAN_PWN_PIN, GPIO.HIGH)
    else:
        GPIO.output(FAN_PWN_PIN, GPIO.LOW)

def decode_gas_value(byte_array_input):
    return int(byte_array_input.decode("ascii"))

def decode_tmp_value(byte_array_input):
    return float(byte_array_input.decode("ascii")) / 10.0

def main_daemon_task(gas_sensor, tmp_sensor):
    while True:
        gas_sensor_value = gas_sensor.get_sensor_value()
        tmp_sensor_value = tmp_sensor.get_sensor_value()
        fan_state = False

        if gas_sensor_value > GAS_THRESHOLD or tmp_sensor_value > TMP_THRESHOLD:
            fan_state = True

        gpio_manage_fan(fan_state)

        logging.info("Gas Sensor Value: {0}".format(gas_sensor_value))
        logging.info("Temperature Sensor Value: {0}".format(tmp_sensor_value))
        logging.info("Fan State: {0}".format(fan_state))
        logging.info("------------------------------------------------------------")

        time.sleep(1)

class BleSensor:
    def __init__(self, ble_mac, characteristic_name, decode_value_method):
        self._peripheral = Peripheral(ble_mac)
        self._characteristic_name = characteristic_name
        self._value = 0
        self._daemon = None
        self._decode_value_method = decode_value_method
    
    def _read_characteristic(self):
        characteristics = self._peripheral.getCharacteristics()
        for characteristic in characteristics:
            if self._characteristic_name == characteristic.uuid.getCommonName() and characteristic.supportsRead():
                self._value = self._decode_value_method(characteristic.read())

    def _continuous_read_task(self):
        while True:
            self._read_characteristic()
            time.sleep(1)

    def start_sensor_daemon(self):
        self._daemon = threading.Thread(target=self._continuous_read_task, daemon=True)
        self._daemon.start()

    def get_sensor_value(self):
        return self._value
    
    def disconnect_sensor(self):
        self._peripheral.disconnect()

if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO, format="%(asctime)s: %(message)s", datefmt="%H:%M:%S")
    gpio_init()

    gas_sensor = None
    tmp_sensor = None

    try:
        gas_sensor = BleSensor(ESP32_GAS_BLE_MAC, CHARACTERISTIC_NAME, decode_gas_value)
        tmp_sensor = BleSensor(ESP32_TMP_BLE_MAC, CHARACTERISTIC_NAME, decode_tmp_value)

        gas_sensor.start_sensor_daemon()
        tmp_sensor.start_sensor_daemon()

        main_daemon = threading.Thread(target=main_daemon_task, args=(gas_sensor, tmp_sensor), daemon=True)
        main_daemon.start()

        # Daemons get killed if they get to the end of the program
        while True:
            pass
            
    except Exception as exception:
        logging.info("Exception:", exception)
    finally:
        if gas_sensor is not None:
            gas_sensor.disconnect_sensor()
        if tmp_sensor is not None:
            tmp_sensor.disconnect_sensor()
        gpio_clean()