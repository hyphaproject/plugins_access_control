// Copyright (c) 2015-2017 Hypha
#ifndef RFID_H
#define RFID_H

#include <string>

#include <hypha/plugin/hyphasensor.h>
#include <boost/asio.hpp>

namespace hypha {
namespace plugin {
namespace rfid {
class RFID : public HyphaSensor {
 public:
  RFID();
  ~RFID();
  void doWork();
  void setup();
  const std::string name() { return "rfid"; }
  const std::string getTitle() { return "RFID"; }
  const std::string getVersion() { return "0.1"; }
  const std::string getDescription() { return "Plugin to detect rfid cards."; }
  const std::string getConfigDescription() { return "{}"; }
  void loadConfig(std::string json);
  std::string getConfig();
  HyphaBasePlugin *getInstance(std::string id);

  void receiveMessage(std::string message);
  std::string communicate(std::string message);

 private:
  boost::asio::io_service ioService;
  boost::asio::serial_port serialPort;
  int fd;
  void setRGY(bool red, bool green, bool yellow);
  void setDoor(bool open);
  void beep();
  int read_until(int fd, char *buf, char until);
};
}
}
}
#endif  // RFID_H
