// Copyright (c) 2015-2016 Hypha

#ifndef WIFI_H
#define WIFI_H

#include <hypha/plugin/hyphasensor.h>
#include <string>

namespace hypha {
namespace plugin {
namespace wifi {
class Wifi : public HyphaSensor {
 public:
  void doWork();
  void setup();
  const std::string name() { return "wifi"; }
  const std::string getTitle() { return "Wifi"; }
  const std::string getVersion() { return "0.1"; }
  const std::string getDescription() {
    return "Plugin to scan for mac adresses in the network.";
  }
  const std::string getConfigDescription() { return "{}"; }
  void loadConfig(std::string json);
  std::string getConfig();
  HyphaBasePlugin *getInstance(std::string id);

  void receiveMessage(std::string message);
  std::string communicate(std::string message);
};
}
}
}
#endif  // WIFI_H
