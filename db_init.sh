#!/bin/bash

g++ --std=c++11 -Ddbmain ./Db_manager/Db_manager.cpp -o db_init -lsqlite3
./db_init
