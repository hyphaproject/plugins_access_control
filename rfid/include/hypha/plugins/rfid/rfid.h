// Copyright (c) 2015-2017 Hypha
#ifndef RFID_H
#define RFID_H

#include <string>

#include <hypha/plugin/hyphaactor.h>
#include <hypha/plugin/hyphasensor.h>
#include <boost/asio.hpp>

#define SERIAL_PORT_READ_BUF_SIZE 512

namespace hypha {
namespace plugin {
namespace rfid {
class RFID : public HyphaSensor, HyphaActor {
 public:
  RFID();
  ~RFID();
  void doWork() override;
  void setup() override;
  const std::string name() override { return "rfid"; }
  const std::string getTitle() override { return "RFID"; }
  const std::string getVersion() override { return "0.2"; }
  const std::string getDescription() override {
    return "Plugin to detect rfid devices.";
  }
  const std::string getConfigDescription() override {
    return "{"
           "\"confdesc\":["
           "{\"name\":\"log\", "
           "\"type\":\"boolean\",\"value\":\"false\",\"description\":\"Log "
           "card detections: \"}"
           "]}";
  }
  void loadConfig(std::string json) override;
  std::string getConfig() override;
  HyphaBasePlugin *getInstance(std::string id) override;

  void receiveMessage(std::string message) override;
  std::string communicate(std::string message) override;

 private:
  boost::asio::io_service ioService;
  boost::asio::serial_port serialPort;

  /**
   * @brief read_buf_raw_ The buffer for async read on serial port
   */
  char read_buf_raw_[SERIAL_PORT_READ_BUF_SIZE];

  bool log = false;

  void setRGY(bool red, bool green, bool yellow);
  void setDoor(bool open);
  void beep();
};
}
}
}
#endif  // RFID_H
