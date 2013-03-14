package com.pascucci.s3p;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertTrue;

import org.junit.Test;
import org.junit.Before;
import org.junit.runner.RunWith;
import org.junit.runner.JUnitCore;
import org.junit.runners.JUnit4;

import java.util.Arrays;

/**
 * Unit tests for S3P's Java implementation.
 */
@RunWith(JUnit4.class)
public class S3PTranslatorTest {
  S3PTranslator translator;

  @Before
  public void setUp() {
    translator = new S3PTranslator();
  }

  @Test
  public void testEncodeNoEscaping() throws S3PException {
    byte[] data = {0x00, 0x01, 0x02, 0x03};
    byte[] expected = {0x56, 0x00, 0x01, 0x02, 0x03, 0x06, 0x65};
    translator.setRawBuffer(data);
    assertArrayEquals(expected, translator.getEncodedBuffer());
  }

  @Test
  public void testEncodeWithEscaping() throws S3PException {
    byte[] data = {0x25, 0x01, 0x56, 0x03};
    byte[] expected = {0x56, 0x25, 0x05, 0x01, 0x25, 0x76, 0x03, 0x7F, 0x65};
    translator.setRawBuffer(data);
    assertArrayEquals(expected, translator.getEncodedBuffer());
  }

  @Test(expected=IllegalArgumentException.class)
  public void testEncodeTooLong() throws S3PException {
    byte[] data = new byte[S3PTranslator.MAX_PACKET_SIZE + 5];
    translator.setRawBuffer(data);
  }

  @Test
  public void testDecodeNoEscaping() throws S3PException {
    byte[] data = {0x56, 0x00, 0x01, 0x02, 0x03, 0x06, 0x65};
    byte[] expected = {0x00, 0x01, 0x02, 0x03};
    translator.setEncodedBuffer(data);
    assertArrayEquals(expected, translator.getRawBuffer());
  }

  @Test
  public void testDecodeWithEscaping() throws S3PException {
    byte[] data = {0x56, 0x25, 0x05, 0x01, 0x25, 0x76, 0x03, 0x7F, 0x65};
    byte[] expected = {0x25, 0x01, 0x56, 0x03};
    translator.setEncodedBuffer(data);
    assertArrayEquals(expected, translator.getRawBuffer());
  }

  @Test(expected=S3PException.class)
  public void testDecodeMixedPacket() throws S3PException {
    byte[] data = {0x56, 0x25, 0x56, 0x01, 0x25, 0x76, 0x03, 0x7F, 0x65};
    translator.setEncodedBuffer(data);
  }

  @Test(expected=S3PException.class)
  public void testNoStartByte() throws S3PException {
    byte[] data = {0x00, 0x25, 0x56, 0x01, 0x25, 0x76, 0x03, 0x7F, 0x65};
    translator.setEncodedBuffer(data);
  }

  @Test(expected=S3PException.class)
  public void testBadChecksum() throws S3PException {
    byte[] data = {0x56, 0x01, 0x76, 0x03, 0x70, 0x65};
    translator.setEncodedBuffer(data);
  }

  @Test(expected=S3PException.class)
  public void testTruncatedPacket() throws S3PException {
    byte[] data = {0x56, 0x01, 0x76, 0x03, 0x70};
    translator.setEncodedBuffer(data);
  }

  @Test(expected=S3PException.class)
  public void testEmptyPacket() throws S3PException {
    byte[] data = {0x00};
    translator.setEncodedBuffer(data);
  }

  @Test(expected=S3PException.class)
  public void testIntegration() throws S3PException {
    byte[] data = {0x25, 0x01, 0x56, 0x03, 0x65};
    S3PTranslator translator1 = new S3PTranslator();
    S3PTranslator translator2 = new S3PTranslator();

    translator1.setRawBuffer(data);
    translator2.setEncodedBuffer(translator1.getEncodedBuffer());
    assertArrayEquals(translator2.getRawBuffer(), data);
  }

  private void printArray(byte[] a) {
    System.out.println("Encoded buffer: ");
    for (byte val : a) {
      System.out.print((int) val);
      System.out.print(",");
    }
  }
}
