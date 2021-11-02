import sys
import socket
import struct
import ssl
import time

SERVER_IP_INTERCHANGE = "localhost"
SERVER_PORT_INTERCHANGE = 43444
TLS = False
PROTOCOL = 2 if TLS else 1
PROTOCOL_VERSION = 1

dataNotUsed = []
conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
conn.setsockopt(socket.SOL_TCP, socket.TCP_NODELAY, 1)

# TCP_QUICKACK only supported in Linux 2.4.4+.
# We use startswith for checking the platform following Python's
# documentation:
# https://docs.python.org/dev/library/sys.html#sys.platform
if sys.platform.startswith('linux'):
    conn.setsockopt(socket.SOL_TCP, socket.TCP_QUICKACK, 1)

# Connect to Server
conn.connect((SERVER_IP_INTERCHANGE, SERVER_PORT_INTERCHANGE))

# Initial handshake with server, negotiate protocol.

sent_len = struct.pack('@I', 8) # send size first
conn.send( sent_len )
protocol_version = struct.pack('@II', PROTOCOL_VERSION, PROTOCOL)
print(len(protocol_version))
print(protocol_version)
conn.send(protocol_version)
# protocol = struct.pack('@B', PROTOCOL)
# print(len(protocol))
# print(len(protocol_version + protocol))
# conn.send(protocol_version + protocol)

# Recieve message from server indicating protocol to use, or error.
recv_len = conn.recv(4) # get message size
recv_len = struct.unpack('@I', recv_len)[0]
response = b''
while len(response) < recv_len:
    packet = conn.recv(recv_len - len(response))
    if not packet:
        print("Error receiving")
    response += packet

print("Server responded:", str(response))

version, protocol = struct.unpack('@II', response)

if int(protocol) == 0:
    print("Error, server did not agree with protocol")
    exit(1)

# If TLS enabled, switch socket to TLS.
# In this case, we do not verify server's certificate.
if TLS:

    print("switching to TLS")
    # conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # conn.setsockopt(socket.SOL_TCP, socket.TCP_NODELAY, 1)
    # conn.setsockopt(socket.SOL_TCP, socket.TCP_QUICKACK, 1)
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    context.check_hostname = False
    context.verify_mode = ssl.VerifyMode.CERT_NONE
    print("about to wrap socket...")
    # conn.connect((SERVER_IP_INTERCHANGE, SERVER_PORT_INTERCHANGE))
    conn = context.wrap_socket(conn)
    print("switching done!")

# Once connection is established (TLS or not), we just send
# messages to the server every 2 seconds.
while True:

    msg  = "holis, soy el cliente piton."
    if TLS:
        msg += " This is encrypted stuff, super secret."

    print("about to send the message")
    sent_len = struct.pack('@I', len(msg.encode())) # send size first
    print("sending lenght")
    conn.send(sent_len)
    print("sending message")
    conn.send(msg.encode())
    print("message sent")

    recv_len = conn.recv(4)
    recv_len = struct.unpack('@I', recv_len)[0]
    response = b''
    while len(response) < recv_len:
        packet = conn.recv(recv_len - len(response))
        if not packet:
            print("Error receiving")
        response += packet

    print("Server responded:", str(response))

    time.sleep(2)
