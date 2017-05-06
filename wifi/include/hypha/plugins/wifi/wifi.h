// Copyright (c) 2015-2017 Hypha

#ifndef WIFI_H
#define WIFI_H

#include <hypha/plugin/hyphasensor.h>
#include <string>

namespace hypha {
namespace plugin {
namespace wifi {
class Wifi : public HyphaSensor {
 public:
  void doWork() override;
  void setup() override;
  const std::string name() override { return "wifi"; }
  const std::string getTitle() override { return "Wifi"; }
  const std::string getVersion() override { return "0.1"; }
  const std::string getDescription() override {
    return "Plugin to scan for mac adresses in the network.";
  }
  const std::string getConfigDescription() override {
    return "{"
           "\"confdesc\":["
           "{\"name\":\"wifidevice\", "
           "\"type\":\"string\",\"value\":\"wlan0\",\"description\":\"The wifi "
           "device name. Find out with command 'ifconfig'.\"}"
           "]}";
  }
  void loadConfig(std::string json) override;
  std::string getConfig() override;
  HyphaBasePlugin *getInstance(std::string id) override;

  std::string communicate(std::string message) override;

 protected:
  std::string wifidevice = "wlan0";
};
}
}
}
#endif  // WIFI_H
