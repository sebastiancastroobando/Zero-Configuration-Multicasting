# Custom Multicasting Library
This is a project for the course [Computer Networks (COMP 535)](https://www.mcgill.ca/study/2023-2024/courses/comp-535) at McGill Univeristy, Montreal. It implements a simple multicasting service using UDP and it defines a simple API for services and application to use. It follows a Zero Configuration Service (ZCS) model, where services can be discovered and used by applications without any configuration.

### Authors
- [Denis Aleksandrov](https://github.com/aleksandrov-denis)
- [Sebastian Castro Obando](https://github.com/sebastiancastroobando/)

### Description
Todo later...

### Usage
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
