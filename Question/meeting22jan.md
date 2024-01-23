### Questions for prof
- Is the hearbeat meant to be implemented with the `zcs_post_ad` or with the multicast API.
- Does this assignment involve any multithreading? 
- Observer node? Can other nodes rely on the observer node. Isn't it the case that all nodes are observe nodes? 


### implimentations
- global registry of nodes
- we need a **node** object, with :
    - `name (string)`
    - `zcs_attribute_t (array)`
    - `number of attributes (int)`
    - `isObserver (bool)`
    - `isOnline (bool)`
