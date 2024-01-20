# network-course-project-transparentMemory
Project for socket programming, ring topology to simute communication between nodes.

The way to use it is compile the code and execute 6 times in different console, one console represent one of the 6 nodes of the topology.

You must executed like:
./node0 127.0.0.1 own_udp_port next_node_udp_port own_tcp_port

where the port nodes are:

 node 0: port tcp 2000 port udp 2001\n
 node 1: port tcp 2002 port udp 2003
 node 2: port tcp 2004 port udp 2005
 node 3: port tcp 2006 port udp 2007
 node 4: port tcp 2008 port udp 2009
 node 5: port tcp 2010 port udp 2011

 Example: for simulate the node 2, you have to execute:
 ./node0 127.0.0.1 2005 2007 2004
