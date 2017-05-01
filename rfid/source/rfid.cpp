// Copyright (c) 2015-2017 Hypha

#include "hypha/plugins/rfid/rfid.h"

#include <Poco/ClassLibrary.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <hypha/utils/logger.h>

using namespace hypha::utils;
using namespace hypha::plugin;
using namespace hypha::plugin::rfid;

int baud_rate = 115200;

RFID::RFID() : serialPort(ioService) {}

RFID::~RFID() {
  if (serialPort.is_open()) {
    serialPort.close();
  }
}

void RFID::doWork() {
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  if (!serialPort.is_open()) return;

  try {
    // source: https://gist.github.com/yoggy/3323808
    int length = serialPort.read_some(
        boost::asio::buffer(read_buf_raw_, SERIAL_PORT_READ_BUF_SIZE));
    std::string readString(read_buf_raw_, length);

    boost::property_tree::ptree ptjson;
    std::stringstream ssjson(readString);
    boost::property_tree::read_json(ssjson, ptjson);

    if (ptjson.get_optional<std::string>("uid")) {
      beep();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      Logger::warning(readString);
      setRGY(false, true, false);
      std::this_thread::sleep_for(std::chrono::seconds(2));
      setRGY(false, false, false);
    }

  } catch (std::exception &e) {
    // skip end of file error.
    if (std::strcmp(e.what(), "read_some: End of file") > 0) {
      Logger::error("Error in RFID:");
      Logger::error(e.what());
    }
  }
}

void RFID::setup() {
  try {
    serialPort = boost::asio::serial_port(ioService);
    serialPort.open("/dev/ttyACM0");
    serialPort.set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
    serialPort.set_option(boost::asio::serial_port::parity(
        boost::asio::serial_port::parity::none));
    serialPort.set_option(boost::asio::serial_port::character_size(
        boost::asio::serial_port::character_size(8)));
    serialPort.set_option(boost::asio::serial_port::stop_bits(
        boost::asio::serial_port::stop_bits::one));
    serialPort.set_option(boost::asio::serial_port::flow_control(
        boost::asio::serial_port::flow_control::none));

    std::string s = "{ \"beep\":\"true\" }";
    serialPort.write_some(boost::asio::buffer(s));
  } catch (std::exception &e) {
    Logger::error(e.what());
  }
}

std::string RFID::communicate(std::string UNUSED(message)) {
  return getStatusMessage();
}

void RFID::loadConfig(std::string json) {
  boost::property_tree::ptree ptjson;
  std::stringstream ssjson(json);
  boost::property_tree::read_json(ssjson, ptjson);

  if (ptjson.get_optional<bool>("log")) {
    log = ptjson.get<bool>("log");
  }
}

std::string RFID::getConfig() { return "{}"; }

HyphaBasePlugin *RFID::getInstance(std::string id) {
  RFID *instance = new RFID();
  instance->setId(id);
  return instance;
}

void RFID::receiveMessage(std::string message) {
  if (message.length() > 0) {
    bool red = false;
    bool green = false;
    bool yellow = false;
    bool door = false;

    boost::property_tree::ptree ptjson;
    std::stringstream ssjson(message);
    boost::property_tree::read_json(ssjson, ptjson);

    if (ptjson.get_optional<bool>("beep")) {
      beep();
    }
    if (ptjson.get_optional<bool>("red")) {
      red = ptjson.get<bool>("red");
    }

    if (ptjson.get_optional<bool>("beep")) {
      door = ptjson.get<bool>("door");
      setDoor(door);
    }

    if (ptjson.get_optional<bool>("red") ||
        ptjson.get_optional<bool>("green") ||
        ptjson.get_optional<bool>("yellow")) {
      if (ptjson.get_optional<bool>("red")) {
        red = ptjson.get<bool>("red");
      }
      if (ptjson.get_optional<bool>("green")) {
        green = ptjson.get<bool>("green");
      }
      if (ptjson.get_optional<bool>("yellow")) {
        yellow = ptjson.get<bool>("yellow");
      }
      setRGY(red, green, yellow);
    }
  }
}

void RFID::beep() {
  boost::property_tree::ptree sendobject;
  sendobject.put("beep", "true");
  std::stringstream ssso;
  boost::property_tree::write_json(ssso, sendobject, false);
  std::string outStr(ssso.str() + "\n");
  serialPort.write_some(boost::asio::buffer(outStr));
}

void RFID::setRGY(bool red, bool green, bool yellow) {
  boost::property_tree::ptree sendobject;
  sendobject.put("red", red?"true":"false");
  sendobject.put("green", green?"true":"false");
  sendobject.put("yellow", yellow?"true":"false");
  std::stringstream ssso;
  boost::property_tree::write_json(ssso, sendobject, false);
  std::string outStr(ssso.str() + "\n");
  serialPort.write_some(boost::asio::buffer(outStr));
}

PLUGIN_API POCO_BEGIN_MANIFEST(HyphaBasePlugin)
    POCO_EXPORT_CLASS(RFID) POCO_END_MANIFEST
