#!/bin/bash

# LAN-A data
lana_app1_recv=$(cat logs/LAN-A_app1_log.txt | grep -c "Received ad")
lana_app2_recv=$(cat logs/LAN-A_app2_log.txt | grep -c "Received ad")
lana_serv1_send=$(cat logs/LAN-A_service1_log.txt | grep -c "Sending ad")
lana_serv2_send=$(cat logs/LAN-A_service2_log.txt | grep -c "Sending ad")

lana_app1_start=$(cat logs/LAN-A_app1_log.txt | grep -c "Starting")
lana_app1_shut=$(cat logs/LAN-A_app1_log.txt | grep -c "Shutting")

lana_app2_start=$(cat logs/LAN-A_app2_log.txt | grep -c "Starting")
lana_app2_shut=$(cat logs/LAN-A_app2_log.txt | grep -c "Shutting")

lana_serv1_start=$(cat logs/LAN-A_service1_log.txt | grep -c "Starting")
lana_serv1_shut=$(cat logs/LAN-A_service1_log.txt | grep -c "Shutting")

lana_serv2_start=$(cat logs/LAN-A_service2_log.txt | grep -c "Starting")
lana_serv2_shut=$(cat logs/LAN-A_service2_log.txt | grep -c "Shutting")

# LAN-B data
lanb_app1_recv=$(cat logs/LAN-B_app1_log.txt | grep -c "Received ad")
lanb_app2_recv=$(cat logs/LAN-B_app2_log.txt | grep -c "Received ad")
lanb_serv1_send=$(cat logs/LAN-B_service1_log.txt | grep -c "Sending ad")
lanb_serv2_send=$(cat logs/LAN-B_service2_log.txt | grep -c "Sending ad")

lanb_app1_start=$(cat logs/LAN-B_app1_log.txt | grep -c "Starting")
lanb_app1_shut=$(cat logs/LAN-B_app1_log.txt | grep -c "Shutting")

lanb_app2_start=$(cat logs/LAN-B_app2_log.txt | grep -c "Starting")
lanb_app2_shut=$(cat logs/LAN-B_app2_log.txt | grep -c "Shutting")

lanb_serv1_start=$(cat logs/LAN-B_service1_log.txt | grep -c "Starting")
lanb_serv1_shut=$(cat logs/LAN-B_service1_log.txt | grep -c "Shutting")

lanb_serv2_start=$(cat logs/LAN-B_service2_log.txt | grep -c "Starting")
lanb_serv2_shut=$(cat logs/LAN-B_service2_log.txt | grep -c "Shutting")

# hard check whether LAN-A logs contain LAN-B comms and vice versa
cat logs/LAN-A* | grep LAN-B
cat logs/LAN-B* | grep LAN-A

echo "there are $(cat logs/LAN-A_app* | grep -c ninja-blender) + $(cat logs/LAN-A* | grep -c SmartLightBulb) LAN-B occurances in LAN-A"

echo "there are $(cat logs/LAN-B* | grep -c xbox) + $(cat logs/LAN-B* | grep -c chromecast) LAN-A occurances in LAN-B"

# testing LAN-A recv and send ratio
if [[ $lana_app1_recv -gt $lana_serv1_send ]]; then
    echo "LAN-A app1 received more than sent by service1"
fi

if [[ $lana_app2_recv -gt $lana_serv2_send ]]; then
    echo "LAN-A app2 received more than sent by service2"
fi

# testing LAN-A starts
if [[ $lana_app1_start -ne 1 ]]; then
    echo "LAN-A app1 started $lana_app1_start times"
fi

if [[ $lana_app2_start -ne 1 ]]; then
    echo "LAN-A app2 started $lana_app2_start times"
fi

if [[ $lana_serv1_start -ne 1 ]]; then
    echo "LAN-A service1 started $lana_serv1_start times"
fi

if [[ $lana_serv2_start -ne 1 ]]; then
    echo "LAN-A service2 started $lana_serv2_start times"
fi

# testing LAN-A shutdowns
if [[ $lana_app1_shut -ne 1 ]]; then
    echo "LAN-A app1 shutdown $lana_app1_shut times"
fi

if [[ $lana_app2_shut -ne 1 ]]; then
    echo "LAN-A app2 shutdown $lana_app2_shut times"
fi

if [[ $lana_serv1_shut -ne 1 ]]; then
    echo "LAN-A service1 shutdown $lana_serv1_shut times"
fi

if [[ $lana_serv2_shut -ne 1 ]]; then
    echo "LAN-A service2 shutdowm $lana_serv2_shut times"
fi

# testing LAN-B send/recv ratio
if [[ $lanb_app1_recv -gt $lanb_serv1_send ]]; then
    echo "LAN-B app1 received more than sent by service1"
fi

if [[ $lanb_app2_recv -gt $lanb_serv2_send ]]; then
    echo "LAN-B app2 received more than sent by service2"
fi

# testing LAN-B starts
if [[ $lanb_app1_start -ne 1 ]]; then
    echo "LAN-B app1 started $lanb_app1_start times"
fi

if [[ $lanb_app2_start -ne 1 ]]; then
    echo "LAN-B app2 started $lanb_app2_start times"
fi

if [[ $lanb_serv1_start -ne 1 ]]; then
    echo "LAN-B service1 started $lanb_serv1_start times"
fi

if [[ $lanb_serv2_start -ne 1 ]]; then
    echo "LAN-B service2 started $lanb_serv2_start times"
fi

# testing LAN-B shutdowns
if [[ $lanb_app1_shut -ne 1 ]]; then
    echo "LAN-B app1 shutdown $lanb_app1_shut times"
fi

if [[ $lanb_app2_shut -ne 1 ]]; then
    echo "LAN-B app2 shutdown $lanb_app2_shut times"
fi

if [[ $lanb_serv1_shut -ne 1 ]]; then
    echo "LAN-B service1 shutdown $lanb_serv1_shut times"
fi

if [[ $lanb_serv2_shut -ne 1 ]]; then
    echo "LAN-B service2 shutdowm $lanb_serv2_shut times"
fi
