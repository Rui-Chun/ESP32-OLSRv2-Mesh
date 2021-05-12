# TESTS - experimental deployment and outputs
## 1. Four nodes in a line.

Topology: 
**node N0(3c:61:05:4c:36:cc) - N1(3c:61:05:4c:3c:28) - N2(08:3a:f2:6e:05:94) - N3(08:3a:f2:6c:d3:bc)**

**Explaination for different fields:**
- 'Neighbors/TWO_HOPs/REMOTEs' shows all the nodes types recorded by the local node.
- 'Node id = #1' does not means N1. This is only the local identifier of each node. Please use mac addresses as global reference.
- 'link_status': 1 for LINK_SYMMETRIC, 0 for LINK_HEARD
- 'MPR status' gives (routing MPR status, flooding MPR status). The definition of value is as follow.
    ```
    typedef enum flooding_mpr_status_t {
        NOT_FLOODING = 0,                   // this node is not a flooding MPR
        FLOODING_TO,                    // this node is local node's flooding MPR
        FLOODING_FROM,                  // is flooding selector
        FLOODING_TO_FROM,               // is flooding MPR and selector
    } flooding_mpr_status_t;

    typedef enum routing_mpr_status_t {
        NOT_ROUTING = 0,
        ROUTING_TO,
        ROUTING_FROM,
        ROUTING_TO_FROM,
    } routing_mpr_status_t;
    ```

### Output from node #N1(3c:61:05:4c:3c:28)
```
Start printing topology info.
Neighbors :
    Node id = #1: 08:3a:f2:6e:05:94, link_status = 1, routing next_hop = #1
    MPR status = (3, 3), out_metric = 1, in_metric = 1 
    link_info : #0 oi_metric(1, 1), #2 oi_metric(1, 1), 
    Node id = #3: 3c:61:05:4c:36:cc, link_status = 1, routing next_hop = #3
    MPR status = (2, 2), out_metric = 1, in_metric = 1 
    link_info : #0 oi_metric(1, 1), 
TWO_HOPs :
    Node id = #2: 08:3a:f2:6c:d3:bc 
    routing next hop = #1, hop_num = 2, path_metric = 2 
    link_info : 
REMOTEs: 
Done printing topology info.
```

### Output from node #N0(3c:61:05:4c:36:cc)
```
Start printing topology info.
Neighbors :
    Node id = #1: 3c:61:05:4c:3c:28, link_status = 1, routing next_hop = #1
    MPR status = (1, 1), out_metric = 1, in_metric = 1 
    link_info : #2 oi_metric(1, 1), #0 oi_metric(1, 1), 
TWO_HOPs :
    Node id = #2: 08:3a:f2:6e:05:94 
    routing next hop = #1, hop_num = 2, path_metric = 2 
    link_info : #1 oi_metric(1, 1), #3 oi_metric(1, 1), 
REMOTEs: 
    Node id = #3: 08:3a:f2:6c:d3:bc 
    routing next hop = #1, hop_num = 3, path_metric = 3 
    link_info : 
Done printing topology info.
```

## 2. seven nodes as a mesh

**N0(3c:61:05:4c:36:cc) - N1(3c:61:05:4c:3c:28) - N2(08:3a:f2:6e:05:94) - N3(08:3a:f2:6c:d3:bc) - N4(c4:4f:33:7f:78:e9) - N5(3c:61:05:0c:38:e8) - N6(3c:61:05:0d:59:94)**

**Notes:** Please refer to the project report to view the topology. Also, the link_info fields shown here may not be up-to-date. Also, there is an unsolved bug of showing #0 multiple times, which I have not got time to fix. 

### Output from N1(3c:61:05:4c:3c:28)
```
Start printing topology info.
Neighbors : (N2, N0, N5, N4)
    Node id = #2: 08:3a:f2:6e:05:94, link_status = 1, routing next_hop = #2
    MPR status = (3, 3), out_metric = 1, in_metric = 1 
    link_info : #1 oi_metric(1, 1), #0 oi_metric(1, 1), #4 oi_metric(1, 1), #0 oi_metric(255, 1), 
    Node id = #3: 3c:61:05:4c:36:cc, link_status = 1, routing next_hop = #3
    MPR status = (2, 2), out_metric = 1, in_metric = 1 
    link_info : #0 oi_metric(1, 1), 
    Node id = #4: 3c:61:05:0c:38:e8, link_status = 1, routing next_hop = #4
    MPR status = (0, 0), out_metric = 1, in_metric = 1 
    link_info : #0 oi_metric(255, 1), #0 oi_metric(1, 1), #0 oi_metric(255, 1), #0 oi_metric(1, 1), #0 oi_metric(1, 1), 
    Node id = #5: c4:4f:33:7f:78:e9, link_status = 1, routing next_hop = #5
    MPR status = (3, 3), out_metric = 1, in_metric = 1 
    link_info : #0 oi_metric(255, 1), #6 oi_metric(1, 1), #0 oi_metric(1, 1), #0 oi_metric(255, 1), 
TWO_HOPs :
    Node id = #1: 08:3a:f2:6c:d3:bc 
    routing next hop = #2, hop_num = 2, path_metric = 2 
    link_info : #2 oi_metric(1, 1), 
    Node id = #6: 3c:61:05:0d:59:94 
    routing next hop = #5, hop_num = 2, path_metric = 2 
    link_info : #5 oi_metric(1, 1), 
REMOTEs: 
```

### Output from N0(3c:61:05:4c:36:cc)
```
Start printing topology info.
Neighbors : (N1)
        Node id = #1: 3c:61:05:4c:3c:28, link_status = 1, routing next_hop = #1
        MPR status = (1, 1), out_metric = 1, in_metric = 1 
        link_info : #0 oi_metric(1, 1), #2 oi_metric(1, 1), #3 oi_metric(1, 1), 
TWO_HOPs : (N2, N4, N5)
        Node id = #2: 08:3a:f2:6e:05:94 
        routing next hop = #1, hop_num = 2, path_metric = 2 
        link_info : #5 oi_metric(1, 1), #1 oi_metric(1, 1), #3 oi_metric(1, 1), 
        Node id = #3: c4:4f:33:7f:78:e9 
        routing next hop = #1, hop_num = 2, path_metric = 2 
        link_info : #4 oi_metric(1, 1), #2 oi_metric(1, 1), #6 oi_metric(1, 1), #1 oi_metric(1, 1), 
        Node id = #4: 3c:61:05:0c:38:e8 
        routing next hop = #1, hop_num = 2, path_metric = 2 
        link_info : 
REMOTEs: (N3, N6)
        Node id = #5: 08:3a:f2:6c:d3:bc 
        routing next hop = #1, hop_num = 3, path_metric = 3 
        link_info : #2 oi_metric(1, 1), 
        Node id = #6: 3c:61:05:0d:59:94 
        routing next hop = #1, hop_num = 3, path_metric = 3 
        link_info : #2 oi_metric(1, 1), 
Done printing topology info.
```
