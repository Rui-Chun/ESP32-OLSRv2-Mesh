# ESPNOW-OLSR/MESH

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

TODO:
1. timer out and delete. done
2. node type switch. done
3. derive flooding and routing MPR
4. send out TC msg 
5. calculate routing set.

Metric support:
Link metrics are defined to be directional; the link metric from one router to another may be different from that on the reverse link. !