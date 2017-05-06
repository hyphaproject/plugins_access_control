// Copyright (c) 2015-2017 Hypha

#ifndef FINGERPRINTZFM_H
#define FINGERPRINTZFM_H

#include <iostream>

#include <ZFMComm.h>
#include <boost/asio.hpp>

#include <hypha/plugin/hyphaactor.h>
#include <hypha/plugin/hyphasensor.h>

namespace hypha {
namespace plugin {
namespace fingerprintzfm {

class FingerPrintZFM : public HyphaSensor, public HyphaActor {
 public:
  enum State { IDLE, SCANNING, PROGRAMMING };
  void doWork() override;
  void setup() override;
  std::string communicate(std::string message) override;
  const std::string name() override { return "fingerprintzfm"; }
  const std::string getTitle() override { return "FingerprintZFM"; }
  const std::string getVersion() override { return "0.1"; }
  const std::string getDescription() override {
    return "Plugin to detect fingerprint with adafruit zfm.";
  }
  const std::string getConfigDescription() override {
    return "{"
           "\"confdesc\":["
           "{\"name\":\"usbport\", "
           "\"type\":\"string\",\"value\":\"/dev/"
           "ttyUSB0\",\"description\":\"Serial Port of fingerprint device.\"}"
           "]}";
  }
  void loadConfig(std::string json) override;
  std::string getConfig() override;
  HyphaBasePlugin *getInstance(std::string id) override;

  void receiveMessage(std::string message) override;

  std::string getUsername(std::string fingerprintid);
  void createDatabase();
  void storeFingerprint(std::string fingerprintid);

  void clearFingerprints();
  void programming();
  void scanning();

 private:
  std::string usbPort;
  ZFMComm fp;
  State state;
  int storecount;
};
}
}
}
#endif  // FINGERPRINTZFM_H
