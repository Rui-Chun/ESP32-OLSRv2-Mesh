# ESPNOW-OLSR/MESH

NOTE: the default txpower is very low to limit the range of operation and make testing easier.

DONE:
1. always use broadcast mac address, do not reply to peer list.
2. use segmentation to create a larger packet. 
3. define HELLO message type.
4. define TC message type.
5. A timer to transmit message periodicaly.

These properties reflect that the MPR Set consists of a set of symmetric 1-hop neighbors that cover all the symmetric 2-hop neighbors and that they do so retaining a minimum distance route (1-hop, if present, or 2-hop) to each symmetric 2-hop neighbor.

Routers MAY make individual decisions as to whether to use link metrics for the calculation of flooding MPRs.

- What is different of flooding and routing MPR?

No much. they can be the same set. They serve different purposes and may be derived from different algorithms. From RFC, it looks like flooding MPR can just use 1 as metric for all links, while routing MPR needs to use metric.

Flooding MPR uses out-going link metrics, while routing MPR uses in-coming link metrics.

Done:
1. timer out and delete. done
2. node type switch. done
3. derive flooding and routing MPR
4. send out TC msg 
5. calculate routing set.

Metric support:
Link metrics are defined to be directional; the link metric from one router to another may be different from that on the reverse link. !

TODO:
- Add DATA msg routing support to make this protocol really useful.
- Support MPR willingness setting.
- Using the AP+STA feature of ESP32, we can add gateways in the mesh network.
- Using gateways to connect to Internet or connect with other WiFi devices.
- Integrate this protocol implementation with high-level protocol, like lwIP.

## TESTS

1. Four nodes in a line.

Topology: 
**node #(3c:61:05:4c:36:cc) - #(3c:61:05:4c:3c:28) - #(08:3a:f2:6e:05:94) - #(08:3a:f2:6c:d3:bc)**
Output from node #(3c:61:05:4c:3c:28)
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
Output from node #(3c:61:05:4c:36:cc)
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