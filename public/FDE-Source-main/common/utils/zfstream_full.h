#ifndef ZFSTREAM_H_
#define ZFSTREAM_H_

#include <ctime> // std::time
#include <fstream>
#include <iostream>
#include <string>

#include <boost/iostreams/char_traits.hpp> // EOF, WOULD_BLOCK
#include <boost/iostreams/concepts.hpp>    // filter
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/operations.hpp> // get, put

#include <boost/random/linear_congruential.hpp>

namespace FDU {

namespace io = boost::iostreams;

class rand_cipher
    : public io::dual_use_filter { // encrypt for output, decrypt for input
  static const unsigned long magic = 0x00D0CAFD, mask = 0x00F0FFFF;
  boost::minstd_rand rand_gen;
  bool header_done;

public:
  rand_cipher()
      : header_done(false), rand_gen(static_cast<unsigned int>(std::time(0))) {
    rand_gen();
  }

  template <typename Sink> bool put(Sink &dest, int c) { // encryption
    if (!header_done) {
      unsigned long header = magic | (rand_gen() & ~mask);
      if (io::write(dest, (char *)(&header), 4) != 4)
        return false;
      rand_gen.seed(header);
      header_done = true;
    }
    return io::put(dest, (c ^ rand_gen()) & 0xff);
  }

  template <typename Source> int get(Source &src) { // decryption
    if (!header_done) {
      unsigned long header;
      if (io::read(src, (char *)(&header), 4) != 4 || (header & mask) != magic)
        return EOF;
      rand_gen.seed(header);
      header_done = true;
    }
    switch (int c = io::get(src)) {
    case EOF:
    case io::WOULD_BLOCK:
      return c;
    default:
      return (c ^ rand_gen()) & 0xff;
    }
  }
};

class zofstream : public std::ostream {
  // static const std::ios_base::open_mode bin_out = std::ios_base::out |
  // std::ios_base::binary | std::ios_base::trunc;
  io::file_descriptor_sink outfile;
  io::filtering_ostreambuf outbuf;
  rand_cipher encoder;
  io::zlib_compressor compressor;

public:
  zofstream(const char *f)
      : outfile(f, std::ios_base::out | std::ios_base::binary |
                       std::ios_base::trunc),
        std::ostream(&outbuf) {
    outbuf.push(compressor);
    outbuf.push(encoder);
    outbuf.push(outfile);
  }
};

class zifstream : public std::istream {
  // std::ios_base::open_mode bin_in = std::ios_base::in |
  // std::ios_base::binary;
  io::file_descriptor_source infile;
  io::filtering_istreambuf inbuf;
  rand_cipher decoder;
  io::zlib_decompressor decompressor;

public:
  zifstream(const char *f)
      : infile(f, std::ios_base::in | std::ios_base::binary),
        std::istream(&inbuf) {
    inbuf.push(decompressor);
    inbuf.push(decoder);
    inbuf.push(infile);
  }
};

class ifstrm { // try open encrypted file
public:
  ifstrm(const char *f) : _pifs(0) { open(f); }
  ~ifstrm() { delete _pifs; }
  operator std::istream &() { return *_pifs; }

private:
  std::istream *_pifs;
  void open(const char *f) {
    _pifs = new zifstream(f);
    _pifs->peek(); // check header
    if (!*_pifs) {
      delete _pifs;
      _pifs = new std::ifstream(f);
    }
  }
};

} /*namespace FDU*/

#endif /*ZFSTREAM_H_*/
