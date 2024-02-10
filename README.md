# Custom Multicasting Library

## Authors
- [Denis Aleksandrov](https://github.com/aleksandrov-denis)
- [Sebastian Castro Obando](https://github.com/sebastiancastroobando/)

## Description
This is a project for the course *Computer Networks (COMP 535)* at McGill Univeristy, Montreal. The project is to implement a simple multicasting service using UDP. The service should be able to send and receive messages to and from other nodes in the network. 

## Usage
Add functions here... (let's do this later when we have time).


## To Do
- [x] `zcs_init()` calls `init_app()`, and this should be a thread
- [x] `zcs_log()` test it.
- [x] Double check `shutdown()`. In particular, which memory to free (app vs. service).
- [x] line 230 : In `init_app()`, we are not checking the name of the incoming node, rather the name of the current node.
- [x] listen and post AD, remove zcs_multicast_send() and use manual send instead.
- [x] query function
- [x] get attributes (check correcteness).
- [ ] better comments
- [ ] delete print statements.
