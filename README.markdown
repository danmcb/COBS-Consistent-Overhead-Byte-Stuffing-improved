

# COBS Encode and Decode Library

This is a fork of the COBS library available on Github here https://github.com/jacquesf/COBS-Consistent-Overhead-Byte-Stuffing

This implementation improves on the original in several ways:

1. a more comprehensive set of unit tests is provided (based on the [examples on the wikipedia](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing#Encoding_examples) page, with one additional test for good measure).
2. the expanded tests reveals a few bugs in the implementation by Jaques F which are fixed in my code (examples 7 and 8 in the wikipedia page).
3. tests are also added to check that the decoder returns 0 if the packet to be decoded is invalid (either because a "marker" byte points past the end of the frame or because a zero is detected before the end of the frame). This also turns out to not work in the other version, but does in mine.
4. Tests are also added to check that the encoder or decoder never write further in the output buffer than they should (whether the terminator is appended or not, see below).
5. A `#define COBS_ENCODE_ADD_TERMINATOR` is added which causes the encoder to append the trailing zero to the encoded packet - trivial but perhaps useful to some. The test suite also validates this option.

This repo keeps the Jaques F implementation in the file `old_cobs.c` and a trivial build script is provided which builds both versions and allows the test cases to be run on each. ( `COBS_ENCODE_ADD_TERMINATOR` should of course NOT be defined when testing the Jaques F version.)

## About COBS

[Consistent Overhead Byte Stuffing](http://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing) is an encoding that removes all 0 bytes from arbitrary binary data. The encoded data consists only of bytes with values from 0x01 to 0xFF. This is useful for preparing data for transmission over a serial link (RS-232 or RS-485 for example), as the 0 byte can be used to unambiguously indicate packet boundaries. COBS also has the advantage of adding very little overhead (at least 1 byte, plus up to an additional byte per 254 bytes of data). For messages smaller than 254 bytes, the overhead is constant.

This is an implementation of COBS for C. It is designed to be both efficient and robust. The code is implemented in modern C99. The decoder is designed to detect  malformed input data and report an error upon detection. A test suite is included to validate the encoder and decoder.

