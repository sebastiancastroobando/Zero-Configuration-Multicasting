#!/bin/bash

# Run the applications in the background and redirect their output to log files
./app1_LAN1 > app1_log.txt 2>&1 &
./service1_LAN1 > service1_log.txt 2>&1 &
./app2_LAN2 > app2_log.txt 2>&1 &
./service2_LAN2 > service2_log.txt 2>&1 &

# Wait for all background jobs to finish
wait