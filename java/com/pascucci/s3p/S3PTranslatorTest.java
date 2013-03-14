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

  private static final byte START = S3PTranslator.S3P_START;
  private static final byte TERM = S3PTranslator.S3P_TERM;
  private static final byte ESCAPE = S3PTranslator.S3P_ESCAPE;
  private static final byte MASK = S3PTranslator.S3P_MASK;

  @Before
  public void setUp() {
    translator = new S3PTranslator();
  }

  @Test
  public void testEncodeNoEscaping() throws S3PException {
    byte[] data = {0x00, 0x01, 0x02, 0x03};
    byte[] expected = {START, 0x00, 0x01, 0x02, 0x03, 0x06, TERM};
    translator.setRawBuffer(data);
    assertArrayEquals(expected, translator.getEncodedBuffer());
  }

  @Test
  public void testEncodeWithEscaping() throws S3PException {
    byte[] data = {ESCAPE, 0x01, START, 0x03};
    byte[] expected = {START, ESCAPE, ESCAPE ^ MASK, 0x01, ESCAPE, START ^ MASK, 0x03, (byte) 0xBD,
        TERM};
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
    byte[] data = {START, 0x00, 0x01, 0x02, 0x03, 0x06, TERM};
    byte[] expected = {0x00, 0x01, 0x02, 0x03};
    translator.setEncodedBuffer(data);
    assertArrayEquals(expected, translator.getRawBuffer());
  }

  @Test
  public void testDecodeWithEscaping() throws S3PException {
    byte[] data = {START, ESCAPE, ESCAPE ^ MASK, 0x01, ESCAPE, START ^ MASK, 0x03, (byte) 0xBD,
        TERM};
    byte[] expected = {ESCAPE, 0x01, START, 0x03};
    translator.setEncodedBuffer(data);
    assertArrayEquals(expected, translator.getRawBuffer());
  }

  @Test(expected=S3PException.class)
  public void testDecodeMixedPacket() throws S3PException {
    byte[] data = {START, ESCAPE, START, 0x01, ESCAPE, 0x76, 0x03, 0x7F, TERM};
    translator.setEncodedBuffer(data);
  }

  @Test(expected=S3PException.class)
  public void testNoStartByte() throws S3PException {
    byte[] data = {0x00, ESCAPE, START, 0x01, ESCAPE, 0x76, 0x03, 0x7F, TERM};
    translator.setEncodedBuffer(data);
  }

  @Test(expected=S3PException.class)
  public void testBadChecksum() throws S3PException {
    byte[] data = {START, 0x01, 0x76, 0x03, 0x70, TERM};
    translator.setEncodedBuffer(data);
  }

  @Test(expected=S3PException.class)
  public void testTruncatedPacket() throws S3PException {
    byte[] data = {START, 0x01, 0x76, 0x03, 0x70};
    translator.setEncodedBuffer(data);
  }

  @Test(expected=S3PException.class)
  public void testEmptyPacket() throws S3PException {
    byte[] data = {0x00};
    translator.setEncodedBuffer(data);
  }

  @Test
  public void testIntegration() throws S3PException {
    byte[] data = {ESCAPE, 0x01, START, 0x03, TERM};
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
