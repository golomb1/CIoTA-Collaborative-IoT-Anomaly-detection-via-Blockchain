# Networking Utils
This folder contains several networking related utils and their implementation.

Class | Description
----- | -----------
BroadcastServer | Interface for broadcast server.
MulticastServerDescriptor | Multicast server that implements the BroadcastServer interface. Support both sending and receiving multicast packets.
MessageFragmentationServer | Implementation of BroadcastServer that fragment messages into smaller packets to prevent packet losses.
SimpleProtocol | Interface for protocol client utils.
SimpleTcpProtocol | Implementation of SimpleProtocol over TCP sessions.
CompressionLibrary | Defines interface for compression tools.
