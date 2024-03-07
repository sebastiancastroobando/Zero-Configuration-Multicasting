# Assignment 2

## Authors
- Denis Aleksandrov
- Sebastian Castro Obando

## Description
To run the first part of the assignment, run the following command:
```bash
./demo-part1.sh
```

To run the second part of the assignment, run the following command:
```bash
./demo-part2.sh
```
*We did not use Docker, because in the assignment description we read : "Get LAN-A and LAN-B running on the same machine or Docker containers".*

## Part 1 : LAN isolation
To show LAN isolation, we created two apps and two services in two different LANs. On LAN-A, apps and services are using the multicast addresses `224.1.1.1` and `224.1.1.2`. On LAN-B, apps and services are using the multicast addresses `224.1.1.3` and `224.1.1.4`. 

On LAN-A, we created the following apps and services:
- `LAN-A_app1` : App on LAN-A which wants to interact with a Chromecast service
- `LAN-A_app2` : App on LAN-A which wants to interact with a Xbox Cloud Gaming service
- `LAN-A_service1` : A Chromecast service on LAN-A
- `LAN-A_service2` : A Xbox Cloud Gaming service on LAN-A

On LAN-B, we created the following apps and services:
- `LAN-B_app1` : App on LAN-B which wants to interact with a Smart light bulb service
- `LAN-B_app2` : App on LAN-B which wants to interact with a Smart ninja blender service
- `LAN-B_service1` : A Smart light bulb service on LAN-B
- `LAN-B_service2` : A Smart ninja blender service on LAN-B

To show that the LANs are isolated, we created the script `demo-part1.sh` which runs all the listed apps and services. From the description above, we expect the following:
- 

## Part 2 : LAN Relay
TODO

## Questions 
1. Suppose you are asked to extend the relay to work with more than 2 LANs, what is the major issue to be solved? 

2. How would you improve your design?

## Contribution Disclosure
Each author has contributed equally to this assignment. 