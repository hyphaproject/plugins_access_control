// Copyright (c) 2015-2017 Hypha

#include "hypha/plugins/wifi/wifi.h"

#include <Poco/ClassLibrary.h>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QProcess>

#include <hypha/plugin/hyphabaseplugin.h>

using namespace hypha::plugin;
using namespace hypha::plugin::wifi;

void Wifi::doWork() {
  std::this_thread::sleep_for(std::chrono::seconds(1));
  qDebug("wifi ..");
  QProcess process;
  process.start("sh", QStringList()
                          << "-c"
                          << "arp-scan --localnet --interface=wlan0 | grep -o "
                             "-E '([[:xdigit:]]{1,2}:){5}[[:xdigit:]]{1,2}'");
  process.waitForFinished();
  QString output(process.readAllStandardOutput());

  // output to json string
  QJsonDocument document;
  QJsonArray devices = QJsonArray::fromStringList(output.split("\n"));
  QJsonObject object;
  object.insert("source", QJsonValue(QString::fromStdString(getId())));
  object.insert("devices", devices);
  object.insert("devicetype", QJsonValue("wifi"));
  document.setObject(object);
  sendMessage(document.toJson().data());
}

void Wifi::setup() {}

std::string Wifi::communicate(std::string UNUSED(message)) { return ""; }

void Wifi::loadConfig(std::string UNUSED(json)) {}

std::string Wifi::getConfig() { return "{}"; }

HyphaBasePlugin *Wifi::getInstance(std::string id) {
  Wifi *instance = new Wifi();
  instance->setId(id);
  return instance;
}

void Wifi::receiveMessage(std::string UNUSED(message)) {}

PLUGIN_API POCO_BEGIN_MANIFEST(HyphaBasePlugin)
POCO_EXPORT_CLASS(Wifi)
POCO_END_MANIFEST
