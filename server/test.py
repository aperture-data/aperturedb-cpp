import sys
import socket
import struct
import ssl
import time

SERVER_IP_INTERCHANGE = "localhost"
SERVER_PORT_INTERCHANGE = 43444
TLS = True
PROTOCOL = 2 if TLS else 1

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

sent_len = struct.pack('@I', 1) # send size first
conn.send( sent_len )
protocol = struct.pack('@b', PROTOCOL)
conn.send(protocol)

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

response = struct.unpack('@b', response)[0]

if int(response) == 0:
    print("Error, server did not agree with protocol")
    exit(1)

# If TLS enabled, switch socket to TLS.
# In this case, we do not verify server's certificate.
if TLS:

    context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    context.check_hostname = False
    context.verify_mode = ssl.VerifyMode.CERT_NONE
    conn = context.wrap_socket(conn)

# Once connection is established (TLS or not), we just send
# messages to the server every 2 seconds.
while True:

    msg  = "holis, soy el cliente piton."
    if TLS:
        msg += " This is encrypted stuff, super secret."

    sent_len = struct.pack('@I', len(msg.encode())) # send size first
    conn.send(sent_len)
    conn.send(msg.encode())

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
