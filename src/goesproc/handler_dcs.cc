#include "handler_dcs.h"

#include <regex>
#include <cstring>

#include "filename.h"
#include "string.h"
#include "time.h"

namespace {

  // LB - Remove if not needed.
//// Expect the file to be named like this:
////
////   16-TEXTdat_17348_201455.lrit
////
//bool goesrParseTextTime(const std::string& name, struct timespec& time) {
//  auto pos = name.find('_');
//  if (pos == std::string::npos) {
//    return false;
//  }
//
//  if (pos + 1 >= name.size()) {
//    return false;
//  }
//
//  const char* buf = name.c_str() + pos + 1;
//  const char* format = "%y%j_%H%M%S";
//  struct tm tm;
//  auto ptr = strptime(buf, format, &tm);
//
//  // strptime was successful if it returned a pointer to the next char
//  if (ptr == nullptr || ptr[0] != '.') {
//    return false;
//  }
//
//  time.tv_sec = mktime(&tm);
//  time.tv_nsec = 0;
//  return true;
//}

} // namespace

DCSTextHandler::DCSTextHandler(
  const Config::Handler& config,
  const std::shared_ptr<FileWriter>& fileWriter)
  : config_(config),
    fileWriter_(fileWriter) {
}

void DCSTextHandler::handle(std::shared_ptr<const lrit::File> f) {
  auto ph = f->getHeader<lrit::PrimaryHeader>();

  // Filter DCS
  if (ph.fileType != 130) {
    return;
  }

  // Filter DCS again if needed
  auto nlh = f->getHeader<lrit::NOAALRITHeader>();
  if (nlh.productID != 8) {
    return;
  }

  auto buf = f->read();
  const char* dcsData = buf.data();
  
  int rv;

  // Read DCS file header (container for multiple DCS payloads)
  dcs::FileHeader fh;
  rv = fh.readFrom(dcsData, buf.size());
  
  // Only process DCS data types.  As of January 11, 2021 this appears to be the only type of data available.
  if (fh.type != "DCSH") {
    return;
  }

  dcs::DCPData dcp;
  rv = dcp.readFrom(dcsData, fh.length);

  struct timespec time = {0, 0};

  rv = clock_gettime(CLOCK_REALTIME, &time);
  ASSERT(rv >= 0);

  // Skip if the time could not be determined
  if (time.tv_sec == 0) {
    return;
  }

  FilenameBuilder fb;
  fb.dir = config_.dir;
  fb.filename = removeSuffix(f->getHeader<lrit::AnnotationHeader>().text);
  fb.time = time;
  auto path = fb.build(config_.filename, "txt");
  const std::vector<char> fd = dcs::formatData(fh, dcp); // Format data to output to file
  fileWriter_->write(path, fd);
  if (config_.json) {
    fileWriter_->writeHeader(*f, path);
  }
}
