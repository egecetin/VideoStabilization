#!/bin/bash

spawn telnet localhost 23000
sleep 1

foreach entry [list "Test Message\r" "Unknown Message\r" "help\r" "\b" "\t" "he\t\r" "enable log v\r" "enable log vv\r" \
                    "ping\r" "clear\r" "enable log vvv\r" "disable log\r" "disable log all\r" "version\r" "status\r" \
                    "\x1b\x5b\x41\r" "\x1b\x5b\x42\r" "\r" "quit\r"] {
    send $entry
    sleep 0.1
}

sleep 0.1
spawn telnet localhost 23000
spawn telnet localhost 23000
spawn telnet localhost 23000
spawn telnet localhost 23000
spawn telnet localhost 23000
spawn telnet localhost 23000
spawn telnet localhost 23000
sleep 1
