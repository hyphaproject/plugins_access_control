// Copyright (c) 2015-2017 Hypha

#include "hypha/plugins/rfid/rfid.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <Poco/ClassLibrary.h>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QProcess>

using namespace hypha::plugin;
using namespace hypha::plugin::rfid;

RFID::RFID() : serialPort(ioService) {}

RFID::~RFID() {}

void RFID::doWork() {
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // read card reader data
  char buf[1024];

  if (read_until(fd, buf, '\n') == 0) {
  }

  QString uid(buf);
  uid = uid.remove(QRegExp("[ \\n\\t\\r]"));
  if (uid.isEmpty() || uid.length() < 5) return;
  // create json string with card id;
  QJsonDocument document_out;
  QJsonObject object_out;
  QJsonArray devices =
      QJsonArray::fromStringList(uid.split('\n', QString::SkipEmptyParts));
  object_out.insert("source", QJsonValue(QString::fromStdString(getId())));
  object_out.insert("devices", devices);
  object_out.insert("devicetype", QJsonValue("rfid"));
  document_out.setObject(object_out);
  qDebug(uid.toStdString().c_str());
  sendMessage(document_out.toJson().data());
  std::this_thread::sleep_for(std::chrono::seconds(2));
}

void RFID::setup() {
  serialPort.open("/dev/ttyACM0");
  serialPort.set_option(
      boost::asio::serial_port::parity(boost::asio::serial_port::parity::none));
  serialPort.set_option(boost::asio::serial_port::character_size(
      boost::asio::serial_port::character_size(8)));
  serialPort.set_option(boost::asio::serial_port::stop_bits(
      boost::asio::serial_port::stop_bits::one));
  serialPort.set_option(boost::asio::serial_port::flow_control(
      boost::asio::serial_port::flow_control::none));

  struct termios toptions;

  QProcess process;
  if (process.execute("stty -F /dev/ttyACM0 9600 -parenb -parodd cs8 -hupcl "
                      "-cstopb cread clocal -crtscts "
                      "-ignbrk -brkint -ignpar -parmrk inpck -istrip -inlcr "
                      "-igncr -icrnl -ixon -ixoff "
                      "-iuclc -ixany -imaxbel -iutf8 "
                      "-opost -olcuc -ocrnl -onlcr -onocr -onlret -ofill "
                      "-ofdel nl0 cr0 tab0 bs0 vt0 ff0 "
                      "-isig -icanon -iexten -echo -echoe -echok -echonl "
                      "-noflsh -xcase -tostop -echoprt "
                      "-echoctl -echoke ")) {
    qDebug("tty options set");
  }

  std::string s = "BEEP";
  serialPort.write_some(boost::asio::buffer(s));

  serialPort.close();

  fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY);
  std::this_thread::sleep_for(std::chrono::seconds(2));
  tcgetattr(fd, &toptions);
  cfsetispeed(&toptions, B9600);
  cfsetospeed(&toptions, B9600);
  toptions.c_cflag &= ~PARENB;
  toptions.c_cflag &= ~CSTOPB;
  toptions.c_cflag &= ~CSIZE;
  toptions.c_cflag |= CS8;
  toptions.c_lflag |= ICANON;
  tcsetattr(fd, TCSANOW, &toptions);
  beep();
  beep();
  beep();
}

int RFID::read_until(int fd, char *buf, char until) {
  char b[1];
  int i = 0;
  do {
    int n = read(fd, b, 1);  // read a char at a time
    if (n == -1) return -1;  // couldn't read
    if (n == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }
    buf[i] = b[0];
    i++;
  } while (b[0] != until);

  buf[i] = 0;  // null terminate the string
  return 0;
}

std::string RFID::communicate(std::string UNUSED(message)) {
  return getStatusMessage();
}

void RFID::loadConfig(std::string UNUSED(json)) {}

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

    QJsonDocument document = QJsonDocument::fromJson(message.c_str());
    QJsonObject object = document.object();

    if (object.contains("beep")) {
      beep();
    }

    if (object.contains("door")) {
      door = object.value("door").toBool();
      setDoor(door);
    }

    if (object.contains("red") || object.contains("green") ||
        object.contains("yellow")) {
      if (object.contains("red")) {
        red = object.value("red").toBool();
      }
      if (object.contains("green")) {
        green = object.value("green").toBool();
      }
      if (object.contains("yellow")) {
        yellow = object.value("yellow").toBool();
      }
      setRGY(red, green, yellow);
    }
  }
}

void RFID::beep() { write(fd, "BEEP\n", 5); }

void RFID::setDoor(bool open) {
  write(fd, open ? "DOOR_OPEN\n" : "DOOR_CLOSE\n", open ? 10 : 11);
}

void RFID::setRGY(bool red, bool green, bool yellow) {
  write(fd, red ? "RED_ON\n" : "RED_OFF\n", red ? 7 : 8);
  write(fd, green ? "GREEN_ON\n" : "GREEN_OFF\n", green ? 9 : 10);
  write(fd, yellow ? "YELLOW_ON\n" : "YELLOW_OFF\n", yellow ? 10 : 11);
}

PLUGIN_API POCO_BEGIN_MANIFEST(HyphaBasePlugin)
    POCO_EXPORT_CLASS(RFID) POCO_END_MANIFEST
