# TCP Protocol implementation
SimpleTcpProtocol is a simple implementation of a lightweight protocol over TCP.
This package is composed from the following classes:
* SimpleTcpProtocolServer - is an implementation of a tcp server.
* SimpleTcpProtocolClient - is an implementation of a tcp client.
* ProtocolException - is an exception that used both by the client and the server to indicate on error between themselves.

ProtocolException is exception that accept a string message.
If the server want to send exception to the client he raise this type of exception.


## SimpleTcpProtocolServer
SimpleTcpProtocolServer is a template class that allows for the registration of functions to requests.
by using the method addHandler, you can associate a function with a request code.
All the handle functions receives a metadata that was given to the server as parameter.
Usage is:

    int metadata = 5;
    SimpleTcpProtocolServer<int> server;
    server.addHandle(1, testFunction);
    server.setMetadata(&metadata);
    server.run();
    /** wait for input **/

In this example, the server handle requests, and when 
receiving a request of type 1 activate the testFunction with the metadata which is a pointer to the number 5.




