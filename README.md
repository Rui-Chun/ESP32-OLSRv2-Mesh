# ESPNOW-OLSR/MESH

TODO:
1. always use broadcast mac address, do not reply to peer list.
2. use segmentation to create a larger packet. 
3. define HELLO message type.
4. define TC message type.
5. A timer to transmit message periodicaly.

These properties reflect that the MPR Set consists of a set of symmetric 1-hop neighbors that cover all the symmetric 2-hop neighbors and that they do so retaining a minimum distance route (1-hop, if present, or 2-hop) to each symmetric 2-hop neighbor.

Routers MAY make individual decisions as to whether to use link metrics for the calculation of flooding MPRs.

- What is different of flooding and routing MPR?

No much. they can be the same set. They serve different purposes and may be derived from different algorithms. From RFC, it looks like flooding MPR can just use 1 as metric for all links, while routing MPR needs to use metric.

How to define a reasonable metric?
How to compute routing?

I can implement basics and discuss entensions.