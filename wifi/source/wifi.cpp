// Copyright (c) 2015-2017 Hypha

#include "hypha/plugins/wifi/wifi.h"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <Poco/ClassLibrary.h>
#include <QtCore/QProcess>

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

  std::stringstream ss(output.toStdString());
  std::istream_iterator<std::string> begin(ss);
  std::istream_iterator<std::string> end;
  std::vector<std::string> devices(begin, end);
  std::copy(devices.begin(), devices.end(),
            std::ostream_iterator<std::string>(std::cout, "\n"));

  boost::property_tree::ptree devices_node;

  for (auto &dev : devices) {
    boost::property_tree::ptree dev_node;
    dev_node.put("", dev);
    devices_node.push_back(std::make_pair("", dev_node));
  }

  boost::property_tree::ptree sendobject;
  sendobject.put("source", getId());
  sendobject.add_child("devices", devices_node);
  sendobject.put("devicetype", "wifi");

  std::stringstream ssso;
  boost::property_tree::write_json(ssso, sendobject);
  sendMessage(ssso.str());
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

PLUGIN_API POCO_BEGIN_MANIFEST(HyphaBasePlugin)
    POCO_EXPORT_CLASS(Wifi) POCO_END_MANIFEST
