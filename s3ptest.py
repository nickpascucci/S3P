#!/usr/bin/env python
# Unit tests for the S3P Python bindings.
# These tests only cover the Python interface, not the S3P implementation
# itself. For testing the implementation, compile and run s3ptest.c.

import s3p
import struct
import unittest

class TestS3PFunctions(unittest.TestCase):
    
    def setUp(self):
        pass
    
    def test_read_good_packet(self):
        packet = struct.pack("BBBBBBB", 
                             0x56, 0x25, 0x05, 0x02, 0x03, 0x2A, 0x65)
        data = s3p.read(packet)
        template = struct.pack("BBB", 0x25, 0x02, 0x03)
        self.assertEquals(data, template)

    def test_read_bad_checksum(self):
        packet = struct.pack("BBBBBBB", 
                             0x56, 0x25, 0x05, 0x02, 0x03, 0x05, 0x65)
        self.assertRaisesRegexp(ValueError, ".*checksum.*", s3p.read, packet)

    def test_read_bad_format(self):
        packet = struct.pack("BBBBBBB", 
                             0xAB, 0x25, 0x05, 0x02, 0x03, 0x05, 0x65)
        self.assertRaisesRegexp(ValueError, ".*parse.*", s3p.read, packet)

    def test_build_good(self):
        data = "The quick red fox jumped over the lazy dog."
        packet = s3p.build(data)
        out = s3p.read(packet)
        self.assertEquals(data, out)

    def test_build_oversize(self):
        data = "a" * 300
        self.assertRaisesRegexp(ValueError, "Payload too large.", 
                                s3p.build, data)

if __name__ == '__main__':
    print "Executing S3P Python Binding tests."
    unittest.main()
