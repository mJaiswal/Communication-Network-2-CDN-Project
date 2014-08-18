Communication-Network-2-CDN-Project
===================================

A spring 2013 project for the course Communication Networks II on Content Distribution Network Protocol Prototyping.


Background
====================
This project will focus on the design and implementation of a simple "content routing"
protocol where the network service involves delivery of named content files. In contrast
to an IP network where packets are routed to a destination address (network port), a
content network goes one step further by directly fetching a content file (as identified by
a unique identifier) from the nearest location at which it is currently stored. Content may
be stored in network-attached devices such as client PCs or servers as in the current
Internet. The objective of the network design is to support a service by which a client
device can query the network with a content identifier and this query is automatically
routed to the nearest location of the content file, and the device storing the content then
replies by sending back the content file to the requesting device. 



Project Description
=====================
The goal of the project is to design and implement a routing protocol that supports
efficient delivery of files distributed in the nodes of a network while satisfying the
following requirements:

- Each content file will be specified by a unique content ID and can be generated at
arbitrary times at any host. The network should support the announcement or
discovery of new content files generated at a host such that other hosts can retrieve
this file when they ask for by its content ID.

- The same content file can be present at multiple hosts. In such a case, the network
must support the retrieval of content files from the best possible location.

- The content files on a host may get deleted at arbitrary times. Similarly hosts can
connect/disconnect from the network at arbitrary times. The network must support
these dynamic changes in the distribution of contents.


All the protocol aspects involved in the network will have to be covered such as: how
content is advertised by host machines, how routing of queries is performed in order to
reach content locations,how the reply containing a content file is routed back to the
requesting source, what ARQ scheme is used for reliability either on links or an 
end-to-end basis.
There are three types of entities to be considered in your design: 
- content files identified by globally unique content IDs, 
- hostbmachines identified by port addresses to which they are connected (“locators”) and
- routers which are connected by an arbitrary topology.



Requirements and Assumptions
==============================
The following set of requirements and assumptions have to be taken into consideration:

• The number of hosts connected to the network N < 255. Corresponding host address
space needed is 0-255. A static ARP table (MAC ID to network address) may be
assumed for simplicity.

• The number of content files is limited to K < 255 and each file has a preassigned
name. Corresponding content ID space needed is 0-255. A static content name
resolution table (content ID to content file name) can be assumed for simplicity.

• Content files are no bigger than 1500B units to be carried in a single packet.

• Location of contents might change over time (insertion and deletion of contents at
different locations).

• Links connecting network elements have a probability of loss equal to p.

• The network service to be provided has the basic primitive get(content_ID), and should 
be realized as a packet datagram type design rather than a connection oriented design 
(i.e. the content-ID is explicitly provided in the packet datagram and
used by the network to route the query).

• The actual protocol design approach is not specified and different teams may decide
to use routing based on content identifiers alone or with both content identifiers and
host addresses.

• Alternative designs will be compared in terms of control overhead costs and data path
forwarding efficiency. Protocol complexity should also be considered when making
design choices.

• No additional network elements such as directory servers or centralized gateways are
to be used.
