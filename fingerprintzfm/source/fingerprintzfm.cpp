// Copyright (c) 2015-2016 Hypha

#include "hyphaplugins/fingerprintzfm/fingerprintzfm.h"

#include <Poco/ClassLibrary.h>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QProcess>
#include <QtCore/QVariant>
#include <QtSerialPort/QSerialPortInfo>

#include <hypha/core/database/database.h>
#include <hypha/plugin/hyphaplugin.h>
#include <hypha/utils/logger.h>

using namespace hypha::utils;
using namespace hypha::database;
using namespace hypha::plugin;
using namespace hypha::plugin::fingerprintzfm;

void FingerPrintZFM::doWork() {
  switch (state) {
    case SCANNING:
      state = IDLE;
      scanning();
      break;
    case PROGRAMMING:
      state = IDLE;
      programming();
      break;
    case IDLE:
    default:
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void FingerPrintZFM::setup() {
  storecount = 12;
  state = IDLE;
  createDatabase();
  QProcess process;
  process.execute("stty -F " + QString::fromStdString(usbPort) +
                  " 0:4:1cb1:0:3:1c:7f:15:4:0:0:0:11:13:1a:0:12:f:17:16:0:0:0:"
                  "0:0:0:0:0:0:0:0:0:0:0:0:0");
  fp = ZFMComm();
  fp.init(strdup(usbPort.c_str()));
}

std::string FingerPrintZFM::communicate(std::string message) {
  return "SUCCESS";
}

void FingerPrintZFM::loadConfig(std::string json) {
  usbPort = "/dev/ttyUSB0";
  QJsonDocument document =
      QJsonDocument::fromJson(QString::fromStdString(json).toUtf8());
  QJsonObject object = document.object();
  if (object.contains("usbport")) {
    usbPort = object.value("usbport").toString().toStdString();
  }
}

std::string FingerPrintZFM::getConfig() { return "{}"; }

hypha::plugin::HyphaPlugin *FingerPrintZFM::getInstance(std::string id) {
  FingerPrintZFM *instance = new FingerPrintZFM();
  instance->setId(id);
  return instance;
}

void FingerPrintZFM::receiveMessage(std::string message) {
  if (message.length() > 0) {
    QJsonDocument document = QJsonDocument::fromJson(message.c_str());
    QJsonObject object = document.object();
    if (object.contains("state")) {
      std::string state = object.value("state").toString().toStdString();
      if (state == "SCANNING") {
        this->state = SCANNING;
      } else if (state == "PROGRAMMING") {
        this->state = PROGRAMMING;
      }
    }
  }
}

std::string FingerPrintZFM::getUsername(std::string fingerprintid) {
  std::string retValue = "";
  Poco::Data::Statement statement = Database::instance()->getStatement();
  statement << "select username from fingerprint where fingerprintid = '" +
                   fingerprintid + "'",
      Poco::Data::Keywords::into(retValue);
  statement.execute();
  return retValue;
}

void FingerPrintZFM::createDatabase() {
  Poco::Data::Statement statement = Database::instance()->getStatement();
  statement << "CREATE TABLE IF NOT EXISTS fingerprint ("
               "`id` int(11) NOT NULL AUTO_INCREMENT,"
               "username varchar(32) NOT NULL,"
               "fingerprintid varchar(32) NOT NULL,"
               "PRIMARY KEY (id)"
               ") DEFAULT CHARSET=utf8;";
  statement.execute();
}

void FingerPrintZFM::storeFingerprint(std::string fingerprintid) {
  Poco::Data::Statement statement = Database::instance()->getStatement();
  std::string unknown = "unknown";
  statement << "insert into fingerprint(username, fingerprintid) values(?, ?);",
      Poco::Data::Keywords::use(unknown),
      Poco::Data::Keywords::use(fingerprintid);
  statement.execute();
}

void FingerPrintZFM::clearFingerprints() {
  bool ret = false;
  while (!ret) {
    ret = fp.scanFinger();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  fp.deleteAllFingerprints();
}

void FingerPrintZFM::programming() {
  sendMessage("{\"say\":\"Neuer Fingerprint wird einprogrammiert.\"}");
  bool ret = false;
  while (!ret) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ret = fp.scanFinger();
  }
  // store the image of the fingerprint to data-buffer 1 ...
  fp.storeImage(1);
  // and scan a second time (needed to create the fingerprint-template)
  // wait until a scan has been successfull
  ret = false;
  while (!ret) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ret = fp.scanFinger();
  }
  // store the image of the fingerprint to data-buffer 2 ...
  fp.storeImage(2);  // store in data-buffer 0
  sendMessage("{\"say\":\"store image.\"}");
  // ... and finally store the fingerprint with index on the device
  for (int i = 0; i < 255; ++i, storecount = (storecount + 1) % 255) {
    if (getUsername(getId() + std::to_string(storecount)).empty()) {
      if (fp.storeFingerprint(storecount)) {
        storeFingerprint(getId().append(std::to_string(storecount)));
        qDebug("Successfully stored your fingerprint !");
        sendMessage("{\"say\":\"Successfully stored your fingerprint !\"}");
        return;
      }
    }
  }
  sendMessage("{\"say\":\"Der Fingerprinter ist voll.\"}");
}

void FingerPrintZFM::scanning() {
  qDebug("scanning");
  for (int i = 0; i < 10; ++i) {
    bool ret = false;
    while (!ret) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      ret = fp.scanFinger();
    }
    fp.storeImage(1);
    // try to find the fingerprint on the device
    int tmp = fp.findFingerprint(1);
    if (tmp == -1) {
    } else {
      sendMessage("{\"authuser\":\"" +
                  getUsername(getId() + std::to_string(storecount)) + "\"}");
      return;
    }
  }
}

POCO_BEGIN_MANIFEST(HyphaPlugin)
POCO_EXPORT_CLASS(FingerPrintZFM)
POCO_END_MANIFEST
