## libS3P and the Super Simple Streaming Protocol ##

S3P is an extremely minimal protocol for doing basic communications
over streaming links such as TCP or UART Serial connections. It
provides packetization and limited error detection functionality, but
not much more. The benefit of such limited functionality is that the
protocol is extremely easy to implement and to parse, and has low
overhead making it suitable for embedded applications.

### Technical Details ###

An S3P packet consists of a start marker, a length byte, a series of
data bytes, and a checksum. In order to preserve the uniqueness of the
start marker value, any conflicting values in the body of the packet
are escaped.

An example:

    [0x56][0x03][0x01][0x02][0x03][0x06]
    ^0    ^1    ^2    ^3    ^4    ^5
    
    0: Start byte
    1: Length byte
    2: Data 1
    3: Data 2
    4: Data 3
    5: Checksum

Escaping is performed by preceding the escaped value with 0x25, and
XOR'ing it with 0x20. Only two values need to be escaped in this way:
0x56 (Start) and 0x25 (Escape). In order to unescape, simply XOR the
value with 0x20 again.

Another example:

    [0x56][0x02][0x01][0x25][0x26]

becomes

    [0x56][0x02][0x01][0x25][0x05][0x26]
    ^0    ^1    ^2    ^3    ^4    ^5

    0: Start byte
    1: Length byte
    2: Data 1
    3: Escape Byte
    4: Data 2
    5: Checksum

Because the length byte may take these values as well, it must be
escaped. The length byte represents the number of data bytes in the
packet *prior to escaping*. It does not take into account the start,
length, or checksum bytes.

The checksum is calculated by summing the values of the data bytes,
but not the start or length bytes, and taking the value MOD 256.

### API ###

libS3P provides a reference implementation, in C, of the S3P
protocol. It provides two functions: s3p\_build() and
s3p\_read(). Their signatures are as follows:

    /**
       Build a new packet from the bytes in "data" into "out".
    
       S3P packets are limited to 256 bytes of data; attempting to
       packetize more will result in a S3P_PAYLOAD_TOO_LARGE
       error. "out" must be at least S3P_OVERHEAD bytes longer than
       the data that needs to be packetized; and this can increase to
       twofold depending on the number of bytes that need escaping. In
       general, unless memory usage is a concern, allocate twice as
       much space as the data you wish to packetize.
    
       Params:
       data: a byte array of data to be packetized.
       dsize: the length of "data".
       out: a byte array which will hold the built packet.
       osize: the length of "out".
       psize: a pointer to an int which will contain the length of the
       built packet.
    */
    S3P_ERR s3p_build(uint8_t const *data, int dsize, uint8_t *out, int osize, 
                        int *psize);
    
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
    S3P_ERR s3p_read(uint8_t const *in, int isize, uint8_t *data, int dsize, 
                       int *psize);

`S3P_ERR` is an enum of error conditions, including:

+ S3P\_SUCCESS
+ S3P\_BUF\_TOO\_SMALL
+ S3P\_PAYLOAD\_TOO\_LARGE
+ S3P\_CHECKSUM\_ERR
+ S3P\_PARSE\_FAILURE

All of the code in this library is provided free of charge and WITHOUT
WARRANTY under the LGPL license.
