import ssl
import socket

SERVER_ADDRESS = 'raspberrypi.local'
SERVER_PORT = 8443

if __name__ == "__main__":
    context = ssl.create_default_context()
    # Load the CA certificate
    context.load_verify_locations("certs/rpi_cert.pem")
    
    ssl_connection = context.wrap_socket(socket.socket(socket.AF_INET, socket.SOCK_STREAM), server_hostname=SERVER_ADDRESS)
    
    ssl_connection.connect((SERVER_ADDRESS, SERVER_PORT))
    ssl_connection.sendall(b"Hello, server! This was encrypted.")