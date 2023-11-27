import ssl
import socket

BUFF_SIZE = 1024
SERVER_ADDRESS = 'localhost'
SERVER_PORT = 8443

if __name__ == "__main__":
    context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
    # Load the CA certificate and the corresponding private key
    context.load_cert_chain(certfile="certs/localhost_cert.pem", keyfile="certs/localhost_key.pem")

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0) as server_socket:
        server_socket.bind((SERVER_ADDRESS, SERVER_PORT))
        server_socket.listen(1)
        with context.wrap_socket(server_socket, server_side=True) as ssl_server_socket:
            ssl_connection, _ = ssl_server_socket.accept()
            print(ssl_connection.read(BUFF_SIZE))