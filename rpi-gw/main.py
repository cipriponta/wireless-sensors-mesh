from bluepy.btle import Peripheral
import RPi.GPIO as GPIO
import time
import logging
import threading
import ssl
import socket
import json

FAN_PWN_PIN = 12
ESP32_GAS_BLE_MAC = "78:e3:6d:19:43:1a"
ESP32_TMP_BLE_MAC = "a8:42:e3:48:32:2e"
CHARACTERISTIC_NAME = "ffcd"
TMP_THRESHOLD = 30.0
GAS_THRESHOLD = 900

SERVER_ADDRESS = "raspberrypi.local"
SERVER_PORT = 8443
CERTFILE = "certs/rpi_cert.pem"
KEYFILE = "certs/rpi_key.pem"

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

def main_daemon_task(gas_sensor, tmp_sensor, tls_socket):
    while True:
        gas_sensor_value = gas_sensor.get_sensor_value()
        tmp_sensor_value = tmp_sensor.get_sensor_value()
        fan_state = False

        if gas_sensor_value > GAS_THRESHOLD or tmp_sensor_value > TMP_THRESHOLD:
            fan_state = True

        gpio_manage_fan(fan_state)

        logging.info("------------------------------------------------------------")
        logging.info("Gas Sensor Value: {0}".format(gas_sensor_value))
        logging.info("Temperature Sensor Value: {0}".format(tmp_sensor_value))
        logging.info("Fan State: {0}".format(fan_state))
        
        tls_socket.send_json( {
                "gas_sensor_value" : gas_sensor_value,
                "tmp_sensor_value" : tmp_sensor_value,
                "fan_state" : fan_state
            }
        )

        time.sleep(1)

class BleSensor:
    def __init__(self, ble_mac, characteristic_name, decode_value_method):
        logging.info("Initializing the {0} ble_sensor".format(ble_mac))
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

class TLSSocket:
    def __init__(self, certfile, keyfile, server_address, server_port):
        self._context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
        # Load the CA certificate and the corresponding private key
        self._context.load_cert_chain(certfile=certfile, keyfile=keyfile)

        self._server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
        self._server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._server_socket.bind((server_address, server_port))
        self._server_socket.listen(1)

        self._ssl_server_socket = self._context.wrap_socket(self._server_socket, server_side=True)
        self._ssl_connection = None

    def wait_for_client(self):
        logging.info("Waiting for a client to connect to the server...")
        self._ssl_connection, addr = self._ssl_server_socket.accept()
        logging.info("Connection received from {0}".format(addr))

    def send_json(self, data_dictionary):
        json_obj = json.dumps(data_dictionary)
        self._ssl_connection.sendall(bytes(json_obj, "utf-8"))

    def close(self):
        self._ssl_connection.close()
        self._server_socket.close()

if __name__ == "__main__":
    gas_sensor = None
    tmp_sensor = None
    tls_socket = None

    try:
        logging.basicConfig(level=logging.INFO, format="%(asctime)s: %(message)s", datefmt="%H:%M:%S")
        gpio_init()

        gas_sensor = BleSensor(ESP32_GAS_BLE_MAC, CHARACTERISTIC_NAME, decode_gas_value)
        tmp_sensor = BleSensor(ESP32_TMP_BLE_MAC, CHARACTERISTIC_NAME, decode_tmp_value)

        gas_sensor.start_sensor_daemon()
        tmp_sensor.start_sensor_daemon()

        tls_socket = TLSSocket(CERTFILE, KEYFILE, SERVER_ADDRESS, SERVER_PORT)
        tls_socket.wait_for_client()

        main_daemon = threading.Thread(target=main_daemon_task, args=(gas_sensor, tmp_sensor, tls_socket), daemon=True)
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
        if tls_socket is not None:
            tls_socket.close()
        gpio_clean()