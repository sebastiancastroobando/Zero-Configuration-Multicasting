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
### Test setup
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

### What is expected?


## Part 2 : LAN Relay
### Test Setup
To show LAN relay, we used the previously created apps and services in LAN-A and LAN-B. We moved some of them in the other LAN, and we created a relay to connect the two LANs. In detail:
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

### What is expected? 


## Questions 
1. **Question:** Suppose you are asked to extend the relay to work with more than 2 LANs, what is the major issue to be solved? 
**Answer:** Our implementation of the relay works by connecting the multicast channels of each LAN to their counter part on the other LAN. For example, if an app sends a discovery in channel A of LAN 1, then the relay will forward this message to the channel A of LAN 2. And vice versa for an app in LAN 2. As you may notice, the relay needs to have a receiver and a sender for each channel in each lan it connect. For this assignment, we we asked to connect 2 LANs, so we needed 8 multicast objects (4 senders and 4 receivers). 
This poses a memory limitation, as we need to create many multicast objects for each LAN we want to connect. This is not very scalable. We would need a more efficient way to connect the LANs which does not require creating two new multicast object for each LAN.

2. **Question:** How would you improve your design?
**Answer:** As we outline in the previous question, we are essentially relying in creating 2 multicast objects for each channel. The very first improvement of our design would need to be to find a way to connect the LANs without this object creation. Perhaps, 

## Contribution Disclosure
Each author has contributed equally to this assignment. 