# CppUtils
CppUtils package was meant to include all the utility functions and classes for this project.
They do not have a direct connection to CIoTA, since these functionalities are general and project independent.

## Buffers related functionalities
### MemBuffer
MemBuffer is a class that was meant to ease the use of using dynamic buffers.
It is a buffer which supports expansion and reduction of size and data dynamically at runtime.

Signature | description
--------- | -----------
void append(const void* ptr, size_t len) | Append data from ptr into this buffer. <br/> @param ptr to the data. <br/> @param len to read from ptr. <br/> @attention  ptr is copied and should be freed by the caller.
void reserveMore(size_t n) | Reserve n more bytes in this buffer. <br/> @param n - how many byte to reserve.
void appendValue(char value, size_t n) | Append a char value n times. <br/> @param value to append <br/> @param n - how many times to write the value.
void padValueTo(char value, size_t desiredLength) | Pad this buffer to the desired length with a given value. <br/> In a case where the length of the buffer is already equal or greater than the desired length nothing occurs. <br/> @param value to pad with. <br/> @param desiredLength of the buffer.
const char *data() const | return the underlying buffer.
size_t size() const;
void clear() | Removes all elements from the buffer (which are destroyed), leaving the container with a size of 0. <br/> A reallocation is not guaranteed to happen, and the vector capacity is not guaranteed to change due to calling this function. A typical alternative that forces a reallocation is to use swap: <br/> vector<char>().swap(x);
bool empty() const | Return true if this buffer is empty and false otherwise.
void swap(std::vector<char>& otherVector) | Swap two MemBuffers. <br/> @param otherVector buffer to swap with.
void swap(MemBuffer& other) | Swap two MemBuffers. <br/> @param other buffer to swap with.

    
MemBuffer's underlying implementation is an std vector of chars and therefore it is essential to understand the implications of that implementation.

Extending a buffer is an operation that requires allocating more memory, and therefore it is preferable to reduce the number of extending operations as much as possible.
For example, instead of the following code:
    
    MemBuffer buffer;
    buffer.appendValue('a', 5);
    buffer.appendValue('b', 5);
    buffer.appendValue('c', 5);

It is preferable to do the following:
    
    MemBuffer buffer;
    buffer.reserveMore(15);
    buffer.appendValue('a', 5);
    buffer.appendValue('b', 5);
    buffer.appendValue('c', 5);
    
    
### Cursor
A cursor is used to ease the handling of parsing byte buffers.
It is used to parse and read some bytes from a buffer while parsing them to a desirable type.
Simple usage:
    
    Cursor c(buffer, len);
    size_t mapperSize;
    c.readInto(&mapperSize, sizeof(size_t));
