- Since the library doesn't return any "object", could we do gloabl static objects in the library and assume that each node using the library will be a process "isolated" from other nodes running from the library? 

- In the provided example, the command is run with two ports (source and destination). After reading the code, it is our understanding that destination is the port the node is listenning on, and source is the port where it would send from. Is this correct? Since we are working with multicasting, does this mean that all nodes, source = destination and also they all have the same port number? Leaving the filtering for us to implement. 

- 