#pragma once

#include <unistd.h>

#include <ctime>
#include <string>
#include <vector>
#include <unordered_map>

namespace dcs {

// Header at the beginning of an LRIT DCS file
struct FileHeader {
  std::string name;
  uint32_t length;
  std::string source;
  std::string type;
  std::string expansion;
  uint32_t headerCRC;
  uint32_t fileCRC;

  int readFrom(const char* buf, size_t len);
};

struct DCPData {
  // Keep information in the provided spec format as much as possible.  Formatting will be done in formatData for output
  struct blockData {
    uint8_t blockID;
    uint16_t blockLength;
    uint32_t sequence;

    uint16_t baudRate;
    uint8_t platform;
    bool parityErrors;
    bool missingEOT;

    bool addrCorrected;
    bool badAddr;
    bool invalidAddr;
    bool incompletePDT;
    bool timingError;
    bool unexpectedMessage;
    bool wrongChannel;

    uint32_t correctedAddr;
    
    std::string carrierStart;
    std::string carrierEnd;

    float signalStrength;
    float freqOffset;

    float phaseNoise;
    std::string phaseModQuality;
    float goodPhase;

    std::string spacePlatform;
    uint16_t channelNumber;

    std::string sourcePlatform;
    std::vector<uint8_t> sourceSecondary = { 0, 0 }; // Hack.  Change when this field is used by system.

    std::vector<uint8_t> DCPData;
    uint16_t DCPDataLength;
    uint16_t blockCRC;
  };

  std::unordered_map<std::string, std::string> spMap;
  std::vector<blockData> blocks;
  int readFrom(const char* buf, size_t len);
};

void initMap(std::unordered_map<std::string, std::string> &data);
std::string toPhaseModQuality(const uint16_t& data);
std::string toSpacePlatform(const uint16_t& data);
uint16_t toRate(const uint8_t& data);
std::string toDateTime(const std::vector<uint8_t>& data);
std::vector<char> formatData(const FileHeader &fh, const DCPData &dcp);

} // namespace dcs
