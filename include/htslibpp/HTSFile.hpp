#ifndef __LIBHTSPP_HTS_FILE__
#define __LIBHTSPP_HTS_FILE__

extern "C" {
#include "htslib/sam.h"
}

#include <stdexcept>
#include <iostream>
#include "htslibpp/HTSRecord.hpp"
#include "htslibpp/HTSThreadPool.hpp"

enum class HTS_FILE_MODE : uint8_t {
  READ = 0,
  WRITE = 1,
  APPEND = 2,
  READ_WRITE = 3
};

std::string to_string(HTS_FILE_MODE m) {
  switch (m) {
  case HTS_FILE_MODE::READ:
    return "r";
  case HTS_FILE_MODE::WRITE:
    return "w";
  case HTS_FILE_MODE::APPEND:
    return "a";
  case HTS_FILE_MODE::READ_WRITE:
    return "rw";
  }
}

class HTSFile {
private:
  htsFile* fh_;
  bam_hdr_t* hdr_;
  bool valid_{false};

public:
  HTSFile() : fh_(nullptr), hdr_(nullptr) {}

  HTSFile(const std::string& fname, HTS_FILE_MODE m) {
    auto ms = to_string(m);
    fh_ = hts_open(fname.c_str(), ms.c_str());
    if (fh_ == NULL) {
      // determine how we want to deal with this
      std::string es = "HTSFile :: failed to open file " + fname;
      throw std::runtime_error(es);
    } else {
      hdr_ = sam_hdr_read(fh_);
      if (hdr_ == NULL) {
        std::string es = "HTSFile :: couldn't read header for file " + fname;
        throw std::runtime_error(es);
      }
      valid_ = true;
    }
  }

  /**
   * Move constructor
   */
  HTSFile(HTSFile&& other) {
    fh_ = other.fh_; other.fh_ = nullptr;
    hdr_ = other.hdr_; other.hdr_ = nullptr;
    valid_ = other.valid_; other.valid_ = false;
  }

  /**
   * Move assignment
   */ 
  HTSFile& operator=(HTSFile&& other) {
    if (this != &other) {
      fh_ = other.fh_;
      other.fh_ = nullptr;
      hdr_ = other.hdr_;
      other.hdr_ = nullptr;
      valid_ = other.valid_;
      other.valid_ = false;
    }
  }
  HTSFile(const char* fname, HTS_FILE_MODE m) : HTSFile(std::string(fname), m) {}

  ~HTSFile() {
    if (hdr_) { bam_hdr_destroy(hdr_); }
    if (fh_) { hts_close(fh_); }
  }

  void set_thread_pool(HTSThreadPool& p) {
    hts_set_opt(fh_, HTS_OPT_THREAD_POOL, &(p.get_pool()));
  }

  int get_next_record(HTSRecord& r) {
    return sam_read1(fh_, hdr_, &r.rec());
  }

};

#endif // __LIBHTSPP_HTS_FILE__
