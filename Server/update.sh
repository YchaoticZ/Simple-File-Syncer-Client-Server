 #!/bin/bash
 echo Updating...
 g++ server.cpp -o server -I/opt/ssl/include/ -L/opt/ssl/lib/ -lcrypto -g -O0 -Wall
 echo Compiled!

