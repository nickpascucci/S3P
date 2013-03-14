package com.pascucci.s3p;

public class S3PTranslator {
  public static final int MAX_PACKET_SIZE = 256;
  public static final int S3P_START = 0x5B;
  public static final int S3P_TERM = 0x5D;
  public static final int S3P_ESCAPE = 0x5E;
  public static final int S3P_MASK = 0x20;

  byte[] raw = new byte[MAX_PACKET_SIZE];
  byte[] encoded = new byte[2*MAX_PACKET_SIZE];
  int rawSize = 0;
  int encodedSize = 0;

  public S3PTranslator setRawBuffer(byte[] buffer) throws S3PException {
    if (buffer.length > MAX_PACKET_SIZE) {
      throw new IllegalArgumentException("Buffer size must be less than " + MAX_PACKET_SIZE
                                         + " bytes");
    }

    this.raw = buffer;
    encode(buffer);
    return this;
  }

  public S3PTranslator setRawBuffer(int[] buffer) throws S3PException {
    byte[] convertedCopy = new byte[buffer.length];
    for (int i=0; i<buffer.length; i++) {
      convertedCopy[i] = intToByte(buffer[i]);
    }
    return setRawBuffer(convertedCopy);
  }

  public S3PTranslator setEncodedBuffer(byte[] buffer) throws S3PException {
    this.encoded = buffer;
    decode(buffer);
    return this;
  }

  public S3PTranslator setEncodedBuffer(int[] buffer) throws S3PException {
    byte[] convertedCopy = new byte[buffer.length];
    for (int i=0; i<buffer.length; i++) {
      convertedCopy[i] = intToByte(buffer[i]);
    }
    return setEncodedBuffer(convertedCopy);
  }

  public byte[] getRawBuffer() {
    byte[] copy = new byte[rawSize];
    System.arraycopy(raw, 0, copy, 0, rawSize);
    return copy;
  }

  public byte[] getEncodedBuffer() {
    byte[] copy = new byte[encodedSize];
    System.arraycopy(encoded, 0, copy, 0, encodedSize);
    return copy;
  }

  private void encode(byte[] buffer) {
    int checksum = 0;
    encoded[0] = intToByte(S3P_START);
    int nextEncodedSpot = 1;

    for (int i = 0; i<buffer.length; i++) {
      int dataByte = byteToInt(buffer[i]);
      checksum += dataByte;
      if (dataByte == S3P_START || dataByte == S3P_TERM || dataByte == S3P_ESCAPE) {
        dataByte = escape(dataByte);
        encoded[nextEncodedSpot] = intToByte(S3P_ESCAPE);
        nextEncodedSpot++;
      }
      encoded[nextEncodedSpot] = intToByte(dataByte);
      nextEncodedSpot++;
    }
    encoded[nextEncodedSpot] = intToByte(checksum % 256);
    encoded[nextEncodedSpot + 1] = intToByte(S3P_TERM);
    encodedSize = nextEncodedSpot + 2;
  }

  private void decode(byte[] buffer) throws S3PException {
    if (byteToInt(buffer[0]) != S3P_START) {
      throw new S3PException("The buffer does not start with the start byte");
    }
    int dataNext = 1;
    int dataRead = 0;
    int checksum = 0;
    while (true) {
      if (dataRead >= raw.length) {
        throw new S3PException("The packet is too large");
      }

      if (dataNext + 1 >= buffer.length) {
        throw new S3PException("Reached end of packet before S3P_TERM byte");
      }

      if (buffer[dataNext + 1] == intToByte(S3P_TERM)) {
        break;
      }

      int dataByte = byteToInt(buffer[dataNext]);
      if (dataByte == S3P_START) {
        throw new S3PException("Illegal START byte encountered during parsing");
      }

      if (dataByte == S3P_ESCAPE) {
        dataNext++;
        dataByte = buffer[dataNext];
        if (dataByte == S3P_START || dataByte == S3P_ESCAPE) {
           throw new S3PException("Illegal byte '" + dataByte + "' after ESCAPE byte");
        }
        dataByte = escape(dataByte);
      }

      raw[dataRead] = intToByte(dataByte);
      checksum += dataByte;
      dataRead++;
      dataNext++;
    }

    if (intToByte(checksum) != buffer[dataNext]) {
      throw new S3PException("Checksum mismatch: expected " + buffer[dataNext]
                             + " but got " + checksum);
    }
    rawSize = dataRead;
  }

  // Escape and unescape are mirrors of each other
  private int escape(int a) {
    return a ^ S3P_MASK;
  }

  private byte intToByte(int a) {
    return (byte) a;
  }

  private int byteToInt(byte a) {
    // Due to sign extension we must mask the 3 most significant bytes.
    return (int) a & 0x000000FF;
  }
}

class S3PException extends Exception {
  public S3PException(String message) {
    super(message);
  }
}
