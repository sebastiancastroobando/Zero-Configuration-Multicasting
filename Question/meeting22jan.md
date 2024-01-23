### Questions for prof (or ed..)
- Is the hearbeat meant to be implemented with the `zcs_post_ad` or with the multicast API.
- Does this assignment involve any multithreading? 

#### Question for ed

---
**Title** :  Question regarding the observer nodes

In the `zcs_log()` function documentation, it mentions that an 'observing' node maintains UP and DOWN logs. Are these logs generated using the zcs_query() function, or is a separate logging mechanism required for this purpose?

Also, in page 5, there is mention of a 'local registry'. This is described as a local table of services based on notifications and heartbeats. Isn't this local registry also the log function? 

--- 

### implimentations
- global registry of nodes
- we need a **node** object, with :
    - `name (string)`
    - `zcs_attribute_t (array)`
    - `number of attributes (int)`
    - `isObserver (bool)`
    - `isOnline (bool)`
    - `localRegistry ("dictionary")`
