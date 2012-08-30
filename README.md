[![Build Status](https://secure.travis-ci.org/nickpascucci/S3P.png)](http://travis-ci.org/nickpascucci/S3P)

## libS3P and the Super Simple Streaming Protocol ##

S3P is an extremely minimal protocol for doing basic communications
over streaming links such as TCP or UART Serial connections. It
provides packetization and limited error detection functionality, but
not much more. The benefit of such limited functionality is that the
protocol is extremely easy to implement and to parse, and has low
overhead making it suitable for embedded applications.

### Technical Details ###

An S3P packet consists of a start marker, a series of data bytes, a
checksum, and a termination marker. In order to preserve the
uniqueness of the control values, any conflicting values in the body
of the packet are escaped.

An example:

    [0x56][0x01][0x02][0x03][0x06][0x65]
    ^0    ^1    ^2    ^3    ^4    ^5
    
    0: Start Byte
    1: Data 1
    2: Data 2
    3: Data 3
    4: Checksum
    5: Termination Marker

Escaping is performed by preceding the escaped value with 0x25, and
XOR'ing it with 0x20. Only two values need to be escaped in this way:
0x56 (Start) and 0x25 (Escape). In order to unescape, simply XOR the
value with 0x20 again.

Another example:

    [0x56][0x01][0x25][0x26][0x65]

becomes

    [0x56][0x01][0x25][0x05][0x26][0x65]
    ^0    ^1    ^2    ^3    ^4    ^5

    0: Start Byte
    1: Data 1
    2: Escape Byte
    3: Data 2
    4: Checksum
    5: Termination Marker

The checksum is calculated by summing the values of the data bytes,
but not the start or term bytes, and taking the value MOD 256.

### API ###

libS3P provides a reference implementation of the S3P protocol in C,
as well as Python bindings. The C interface provides two functions:
s3p\_build() and s3p\_read(). Their signatures are as follows:

```c
/**
   Build a new packet from the bytes in "data" into "out".

   S3P packets can encapsulate arbitrary data, but "out" must be at
   least S3P_OVERHEAD bytes longer than the data that needs to be
   packetized; and this can increase to twofold depending on the
   number of bytes that need escaping. In general, unless memory usage
   is a concern, allocate twice as much space as the data you wish to
   packetize.

   Params:
   data: a byte array of data to be packetized.
   dsize: the length of "data".
   out: a byte array which will hold the built packet.
   osize: the length of "out".
   psize: a pointer to an int which will contain the length of the
   built packet.
*/
S3P_ERR s3p_build(uint8_t const *data, size_t dsize, uint8_t *out, 
                  size_t osize, size_t *psize);

/**
   Read a packet from "in", and place unescaped data into "data".
    
   Breaking streams into packets is the responsibility of the
   caller. This function will only read the first packet in the
   "in" buffer; all other data will be ignored. This function
   guarantees that the length of the data retrieved from the
   packet will be strictly less than the size of the input packet.
      
   Params:
   in: byte array of raw, packetized data.
   isize: the length of "in".
   data: byte array which will contain the retrieved data.
   dsize: the length of "data".
   psize: pointer to an int which will contain the length of the
   retrieved data.
*/
S3P_ERR s3p_read(uint8_t const *in, size_t isize, uint8_t *data, 
                 size_t dsize, size_t *psize);
```

`S3P_ERR` is an enum of error conditions, including:

+ S3P\_SUCCESS
+ S3P\_BUF\_TOO\_SMALL
+ S3P\_PAYLOAD\_TOO\_LARGE
+ S3P\_CHECKSUM\_ERR
+ S3P\_PARSE\_FAILURE

The Python bindings are provided in the `s3p` module, and have the
following signatures:

```
build(...)
    s3p.build(str): Build an S3P packet from a string.
    
    This function wraps the C function s3p_build, which constructs
    Super Simple Streaming Protocol packets. S3P packets can
    encapsulate any binary data structured as a byte sequence.
    
    Arguments:
    str -- a byte sequence to be encapsulated.
    
    Returns:
    A new Python string containing the encapsulated data.
    
read(...)
    s3p.read(str): Read an S3P packet from a string.
    
    This function wraps the C function s3p_read, which extracts data
    from Super Simple Streaming Protocol Packets. S3P packets contain
    a checksum which allows for extremely basic error detection (but
    not correction).  s3p.read() will raise a ValueError if the
    checksum does not match the data in the packet.
    
    Arguments:
    str -- an S3P packet to read.
    
    Returns:
    A new Python string containing the packet's data.
    
    Raises:
    ValueError if the packet's checksum doesn't match the data.
    ValueError if the packet's formatting doesn't match the S3P 
    protocol.
```

These bindings work with general-purpose Python strings and can be
used to encapsulate arbitrary data.

## Improvements

+ C Functions for reading packets from streams and passing them to
user-provided callbacks would be nice. Portability (especially to
embedded systems) might be an issue.

All of the code in this library is provided free of charge and WITHOUT
WARRANTY under the LGPL license.
