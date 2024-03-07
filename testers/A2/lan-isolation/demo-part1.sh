#!/bin/bash

# Compile the programs
make

# Run the applications in the background and redirect their output to log files
./LAN-A_service1 > logs/LAN-A_service1_log.txt 2>&1 &
./LAN-A_service2 > logs/LAN-A_service2_log.txt 2>&1 &
./LAN-B_service1 > logs/LAN-B_service1_log.txt 2>&1 &
./LAN-B_service2 > logs/LAN-B_service2_log.txt 2>&1 &
./LAN-A_app1 > logs/LAN-A_app1_log.txt 2>&1 &
./LAN-A_app2 > logs/LAN-A_app2_log.txt 2>&1 &
./LAN-B_app1 > logs/LAN-B_app1_log.txt 2>&1 &
./LAN-B_app2 > logs/LAN-B_app2_log.txt 2>&1 &

# Wait for all background jobs to finish
wait

# Remove the executables
make clean
