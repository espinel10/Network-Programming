import socket
import sys

print("Ingrese el mensaje")
entrada=input()


# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# Connect the socket to the port where the server is listening
server_address = ('localhost', 5020)
#print('connecting to {} port {}'.format(*server_address))
sock.connect(server_address)

try:

    # Send data buenas tardes
    entrada+=entrada+'                                                        '
    message = entrada.encode()
    #print('sending {!r}'.format(message))
    sock.sendall(message)

    # Look for the response
    amount_received = 0
    amount_expected = len(message)

    while amount_received < amount_expected:
        data = sock.recv(40)
        amount_received += len(data)
        #print('received {!r}'.format(data))

        print(data.decode("utf-8"))


finally:
    #print('closing socket')
    sock.close()
