import socket
import threading
import time

# Set the IP address and port for the server to listen on
IP_ADDRESS = "127.0.0.1"
PORT = 8966


# Function to handle a client connection
def handle_client(client_socket):
    print(f"Accepted connection from {client_socket.getpeername()}")

    try:
        while True:
            # Send the response XML to the connected client
            response = "<?xml version=\"1.0\"?><protocol><cld v=\"1.0\"><msg t=\"welcome\"><id>12</id></msg></cld></protocol>"
            client_socket.send(response.encode("utf-8"))
            print("Sent...")

            # Sleep for 5 seconds before sending the next response
            time.sleep(5)

            # Receive data from the client
            data = client_socket.recv(1024)
            if not data:
                break

            # Process the received data (you can customize this part)
            received_data = data.decode("utf-8")
            print(f"Received data from {client_socket.getpeername()}: {received_data}")

    except (ConnectionResetError, ConnectionAbortedError):
        # Handle the case when the client disconnects
        print(f"Connection with {client_socket.getpeername()} closed")

    finally:
        # Close the client socket
        client_socket.close()


# Create a socket server
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

server_socket.bind((IP_ADDRESS, PORT))
server_socket.listen(1)  # Listen for one incoming connection

print(f"Server listening on {IP_ADDRESS}:{PORT}")

while True:
    client_socket, client_address = server_socket.accept()

    # Spawn a new thread to handle the client connection
    client_thread = threading.Thread(target=handle_client, args=(client_socket,))
    client_thread.start()
