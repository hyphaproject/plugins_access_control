// Copyright (c) 2015-2017 Hypha

#pragma once

#include <hypha/plugin/hyphahandler.h>
#include <string>

namespace hypha {
namespace plugin {
namespace dooropener {
class DoorOpener : public HyphaHandler {
 public:
  explicit DoorOpener();
  ~DoorOpener();
  static DoorOpener *instance();
  void doWork() override;
  const std::string name() override { return "dooropener"; }
  const std::string getTitle() override { return "DoorOpener"; }
  const std::string getVersion() override { return "0.2"; }
  const std::string getDescription() override {
    return "Handler to open doors.";
  }

  const std::string getConfigDescription() override { return "{}"; }
  void loadConfig(std::string config) override;
  std::string getConfig() override;

  void parse(std::string message);
  void giveFeedback(std::string username);

  HyphaBasePlugin *getInstance(std::string id) override;

  void receiveMessage(std::string message) override;
  std::string communicate(std::string message) override;

  void openDoor(std::string username);
  void closeDoor();

  bool accessAllowed(std::string user);
  void createDatabase();

 protected:
  std::string mastercard;
  bool useFingerprint = false;
  bool email = false;
};
}
}
}
