// Copyright (c) 2015-2017 Hypha

#include "hypha/plugins/explorenfc/explorenfc.h"

#include <Poco/ClassLibrary.h>

#include <QtCore/QDebug>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QProcess>

using namespace hypha::plugin;
using namespace hypha::plugin::explorenfc;

void ExploreNFC::doWork() {
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  QProcess process;
  process.start("python", QStringList() << "../plugins/explorenfc.py");
  process.waitForFinished();
  QString output(process.readAllStandardOutput());
  QJsonDocument document;
  QJsonArray devices = QJsonArray::fromStringList(output.split("\n"));
  QJsonObject object;
  object.insert("source", QJsonValue(QString::fromStdString(getId())));
  object.insert("devices", devices);
  object.insert("devicetype", QJsonValue("rfid"));
  document.setObject(object);
  qDebug(output.toStdString().c_str());
  sendMessage(document.toJson().data());
}

void ExploreNFC::setup() {}

std::string ExploreNFC::communicate(std::string UNUSED(message)) {
  QProcess process;
  process.start("python", QStringList() << "../plugins/explorenfc.py");
  process.waitForFinished();
  QString output(process.readAllStandardOutput());
  qDebug(output.toStdString().c_str());

  // output to json string
  QJsonDocument document;
  QJsonArray devices = QJsonArray::fromStringList(output.split("\n"));
  QJsonObject object;
  object.insert("source", QJsonValue(QString::fromStdString(getId())));
  object.insert("devices", devices);
  object.insert("devicetype", QJsonValue("rfid"));
  document.setObject(object);

  return QString::fromUtf8(document.toJson()).toStdString();
}

void ExploreNFC::loadConfig(std::string UNUSED(json)) {}

std::string ExploreNFC::getConfig() { return "{}"; }

HyphaBasePlugin *ExploreNFC::getInstance(std::string id) {
  ExploreNFC *instance = new ExploreNFC();
  instance->setId(id);
  return instance;
}

void ExploreNFC::receiveMessage(std::string UNUSED(message)) {}

PLUGIN_API POCO_BEGIN_MANIFEST(HyphaBasePlugin)
    POCO_EXPORT_CLASS(ExploreNFC) POCO_END_MANIFEST
