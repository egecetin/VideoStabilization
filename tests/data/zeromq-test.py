#!/usr/bin/env python
import zmq
import struct

context = zmq.Context()
socketMng = context.socket(zmq.REQ)
socketMng.connect("tcp://127.0.0.1:8300")

msgArray = [
    # Ask version (success)
    [struct.pack("L", 1230128470)],
    # Ask version (fail)
    [struct.pack("L", 1230128470), b"dummy"],
    # Ask log level (info)
    [struct.pack("L", 1279741772), b"v"],
    # Ask log level (debug)
    [struct.pack("L", 1279741772), b"vv"],
    # Ask log level (trace)
    [struct.pack("L", 1279741772), b"vvv"],
    # Ask log level (fail)
    [struct.pack("L", 1279741772), b"v", b"dummy"],
    # Ask ping (success)
    [struct.pack("L", 1196312912)],
    # Ask ping (fail)
    [struct.pack("L", 1196312912), b"dummy"],
    # Ask status (success)
    [struct.pack("L", 1263027027)],
    # Ask status (fail)
    [struct.pack("L", 1263027027), b"dummy"],
]

for msg in msgArray:
    socketMng.send_multipart(msg)
    recvMsg = socketMng.recv_multipart()
    print("Server received message: %s" % (recvMsg))
