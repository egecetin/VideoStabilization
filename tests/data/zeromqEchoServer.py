#!/usr/bin/env python

import zmq
from optparse import OptionParser

address = ""
count = 1


def main():
    print("ZeroMQ server listening on %s" % address)
    context = zmq.Context()
    socket = context.socket(zmq.REP)
    socket.bind(address)

    for i in range(0, count):
        msg = socket.recv_multipart()
        print("Server received message: %s" % (msg))
        socket.send_multipart(msg)


if __name__ == "__main__":
    parser = OptionParser()
    parser.usage = "Creates an zeromq-server that will echo out"
    parser.add_option("-a", "--address")
    parser.add_option("-c", "--count", default=1)
    (options, args) = parser.parse_args()

    address = options.address
    count = int(options.count)
    main()
