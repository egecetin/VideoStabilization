#!/usr/bin/env python
ETH_P_ALL = 0x0003

import socket
import sys
import time

interface = "eth0"
packet = bytearray(str("I'm a dumb message.").encode("utf-8"))

with socket.socket(socket.AF_PACKET, socket.SOCK_RAW) as rs:
    rs.bind((interface, ETH_P_ALL))
    for i in range(0, 10):
        sentbytes = rs.send(packet)
        time.sleep(0.1)
print("Send all packets")
