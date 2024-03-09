## TODO now
- [ ] should LAN-A (224.1.1.1 and 224.1.1.2) and LAN-B (224.1.1.3 and 224.1.1.4) be on the same port number? If so, is it normal that they can communicate with each other (even without the same channels)?
- [ ] When the name of the service is below 7 characters, there are some unexpected behaviours.
- [ ] Write test cases for part 2 (which is the relay part)
- [ ] Write shell script to run the tests

## Delivery
- [ ] put the zcs files (header and source) in this directory
- [ ] put relay file in this directory
- [ ] modify the makefile to include the new zcs files and relay file path
- [ ] complete README.md with instructions on how to run the tests

## Notes
- zcs library modifications:
    - In `init_app`, we are now ignoring the relay part of the packet being sent. 
    - Added verbose mode to the 
    - Now we only use two channels. This is done by creating a new global flag `listen_ad` which tells the `init_app` function to start listening to the ads.
    - Using mutex lock for sending data when two threads might try to use the msend object.
- relay file:
    - Wrote lots of comments, so it should be easy to understand. 

