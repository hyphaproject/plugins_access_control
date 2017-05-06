// Copyright (c) 2015-2017 Hypha

#include "hypha/plugins/dooropener/dooropener.h"

#include <chrono>
#include <thread>

#include <Poco/ClassLibrary.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/Statement.h>
#include <Poco/DateTime.h>
#include <Poco/Timestamp.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <hypha/core/database/database.h>
#include <hypha/core/database/userdatabase.h>
#include <hypha/plugin/hyphabaseplugin.h>
#include <hypha/plugin/pluginloader.h>
#include <hypha/utils/logger.h>

using namespace hypha::utils;
using namespace hypha::database;
using namespace hypha::settings;
using namespace hypha::plugin;
using namespace hypha::plugin::dooropener;
using namespace hypha::plugin;

DoorOpener::DoorOpener() {}

DoorOpener::~DoorOpener() {}

void DoorOpener::doWork() {
  std::this_thread::sleep_for(std::chrono::seconds(1));
}

void DoorOpener::parse(std::string message) {
  boost::property_tree::ptree ptjson;
  std::stringstream ssjson(message);
  boost::property_tree::read_json(ssjson, ptjson);

  if (ptjson.get_optional<std::string>("device")) {
    std::string device = ptjson.get<std::string>("device");
    if (device == mastercard) {
      sendMessage("{\"state\":\"PROGRAMMING\"}");
    } else {
      std::string username = UserDatabase::instance()->getOwnerOfDevice(device);
      if (useFingerprint) {
        giveFeedback(username);
      } else {
        if (accessAllowed(username)) {
          openDoor(username);
        } else {
          sendMessage("{\"say\":\"Access denied at this time.\"}");
        }
      }
    }

  } else if (ptjson.get_optional<std::string>("authuser")) {
    std::string authuser = ptjson.get<std::string>("authuser");
    if (accessAllowed(authuser)) {
      openDoor(authuser);
    } else {
      sendMessage("{\"say\":\"Access denied at this time.\"}");
    }
  }
}

void DoorOpener::giveFeedback(std::string username) {
  std::string fullname = UserDatabase::instance()->getFirstname(username) +
                         " " + UserDatabase::instance()->getLastname(username);
  if (username.empty()) {
    sendMessage("{\"red\":true, \"door\":false}");
    sendMessage("{\"say\":\"Zugang verweigert.\"}");
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    sendMessage("{\"red\":false, \"door\":false}");
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    sendMessage("{\"red\":true, \"door\":false}");
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    sendMessage(
        "{\"red\":false,\"green\":false,\"yellow\":false, \"door\":false}");
  } else {
    sendMessage("{\"state\":\"SCANNING\"}");
    sendMessage("{\"say\":\"Guten Tag, " + fullname + ".\"}");
  }
}

void DoorOpener::loadConfig(std::string config) {
  boost::property_tree::ptree ptjson;
  std::stringstream ssjson(config);
  boost::property_tree::read_json(ssjson, ptjson);

  if (ptjson.get_optional<std::string>("mastercard")) {
    mastercard = ptjson.get<std::string>("mastercard");
  }
  if (ptjson.get_optional<bool>("fingerprint")) {
    useFingerprint = ptjson.get<bool>("fingerprint");
  }
  if (ptjson.get_optional<bool>("email")) {
    email = ptjson.get<bool>("email");
  }
}

std::string DoorOpener::getConfig() { return "{\"name\":\"dooropener\"}"; }

HyphaBasePlugin *DoorOpener::getInstance(std::string id) {
  DoorOpener *dooropener = new DoorOpener();
  dooropener->setId(id);
  return dooropener;
}

void DoorOpener::receiveMessage(std::string message) {
  boost::property_tree::ptree ptjson;
  std::stringstream ssjson(message);
  boost::property_tree::read_json(ssjson, ptjson);

  if (ptjson.get_optional<std::string>("say")) {
    sendMessage(message);
  } else {
    parse(message);
  }
}

std::string DoorOpener::communicate(std::string UNUSED(message)) { return ""; }

void DoorOpener::openDoor(std::string username) {
  sendMessage("{\"say\":\"Door opened.\"}");
  sendMessage("{\"green\":true, \"beep\":600}");
  std::this_thread::sleep_for(std::chrono::milliseconds(400));
  sendMessage("{\"green\":false}");
  std::this_thread::sleep_for(std::chrono::milliseconds(400));
  sendMessage("{\"red\":false,\"green\":true,\"yellow\":false, \"door\":true}");
  sendMessage("{\"door\":true}");
  closeDoor();
  sendMessage("{\"mail\":\"" + username + " got access to door.\"}");
}

void DoorOpener::closeDoor() {
  std::this_thread::sleep_for(std::chrono::seconds(5));
  sendMessage("{\"green\":false, \"door\":false}");
  sendMessage("{\"say\":\"Door closed.\"}");
}

bool DoorOpener::accessAllowed(std::string user) {
  if (user.empty()) return false;
  Poco::Data::Statement statement = Database::instance()->getStatement();
  std::string q("select atworktime from dooropener_user where id = '" + id +
                "' and user = '" + user + "';");
  statement << q;
  statement.execute();
  Poco::Data::RecordSet rs(statement);
  bool more = rs.moveFirst();
  while (more) {
    if (rs[0].convert<bool>()) {
      q = "select type,start,end,starttime,endtime from iterativeaccount where "
          "username = '" +
          user + "';";
      statement << q;
      statement.execute();
      rs = Poco::Data::RecordSet(statement);
      more = rs.moveFirst();
      while (more) {
        Poco::DateTime now;
        std::string type = rs[0].convert<std::string>();
        Poco::DateTime start = rs[1].convert<Poco::DateTime>();
        Poco::DateTime end = rs[2].convert<Poco::DateTime>();
        Poco::DateTime starttime = rs[3].convert<Poco::DateTime>();
        Poco::DateTime endtime = rs[4].convert<Poco::DateTime>();

        Logger::warning(type);
        switch (now.dayOfWeek()) {
          case 0:
            if (type != "workingsun") return false;
          case 1:
            if (type != "workingmon") return false;
            break;
          case 2:
            if (type != "workingtue") return false;
            break;
          case 3:
            if (type != "workingwed") return false;
            break;
          case 4:
            if (type != "workingthu") return false;
            break;
          case 5:
            if (type != "workingfri") return false;
            break;
          case 6:
            if (type != "workingsat") return false;
            break;

            break;
          default:
            break;
        };
        if (start <= now && end >= now) {
          if (starttime.timestamp() <= now.timestamp() &&
              endtime.timestamp() >= now.timestamp()) {
            return true;
          }
        }
        more = rs.moveNext();
      }
    } else {
      return true;
    }
    more = rs.moveNext();
  }
  return false;
}

void DoorOpener::createDatabase() {
  try {
    Poco::Data::Statement statement = Database::instance()->getStatement();
    if (hypha::settings::DatabaseSettings::instance()->getDriver() ==
        "SQLite") {
      statement << "CREATE TABLE IF NOT EXISTS `dooropener_user` ("
                   "`id` varchar(32) PRIMARY KEY AUTOINCREMENT, "
                   "user varchar(32)  PRIMARY KEY NOT NULL, "
                   "atworktime INTEGER NOT NULL DEFAULT 0,"
                   ");";
    } else {
      statement << "CREATE TABLE IF NOT EXISTS `dooropener_user` ("
                   "id varchar(32) NOT NULL,"
                   "user varchar(128) NOT NULL,"
                   "atworktime bool NOT NULL DEFAULT 0,"
                   "PRIMARY KEY (id,user)"
                   ") DEFAULT CHARSET=utf8;";
    }
    statement.execute();
  } catch (std::exception &e) {
    hypha::utils::Logger::error(e.what());
  }
}

PLUGIN_API POCO_BEGIN_MANIFEST(HyphaHandler)
    POCO_EXPORT_CLASS(DoorOpener) POCO_END_MANIFEST
