#!/bin/bash
echo Updating...
g++ client.cpp -o client -I/opt/ssl/include -L/opt/ssl/lib -lcrypto -g -O0 -Wall -Wextra
echo Compiled!
