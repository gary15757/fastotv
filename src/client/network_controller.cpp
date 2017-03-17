/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of SiteOnYourDevice.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "network_controller.h"

#include <string>

#include "inih/ini.h"

#include <common/convert2string.h>
#include <common/file_system.h>
#include <common/logger.h>

#include "client/inner/inner_tcp_handler.h"
#include "client/inner/inner_tcp_server.h"

#include <common/application/application.h>

#include "server_config.h"

namespace {

int ini_handler_fasto(void* user, const char* section, const char* name, const char* value) {
  fasto::fastotv::client::TvConfig* pconfig =
      reinterpret_cast<fasto::fastotv::client::TvConfig*>(user);

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
  if (MATCH(SETTINGS_SECTION_LABEL, LOGIN_SETTING_LABEL)) {
    pconfig->login = value;
    return 1;
  } else if (MATCH(SETTINGS_SECTION_LABEL, PASSWORD_SETTING_LABEL)) {
    pconfig->password = value;
    return 1;
  } else {
    return 0;
  }
}
}  // namespace

namespace fasto {
namespace fastotv {
namespace client {

NetworkController::NetworkController(const std::string& config_path)
    : ILoopThreadController(),
      config_path_(common::file_system::realpath_from_filename(config_path)),
      config_() {
  readConfig();
}

void NetworkController::Start() {
  ILoopThreadController::Start();
}

void NetworkController::Stop() {
  ILoopThreadController::Stop();
}

NetworkController::~NetworkController() {
  saveConfig();
}

AuthInfo NetworkController::authInfo() const {
  return AuthInfo(config_.login, config_.password);
}

TvConfig NetworkController::config() const {
  return config_;
}

void NetworkController::SetConfig(const TvConfig& config) {
  config_ = config;
  client::inner::InnerTcpHandler* handler = static_cast<client::inner::InnerTcpHandler*>(handler_);
  if (handler) {
    handler->SetConfig(config);
  }
}

void NetworkController::RequestChannels() const {
  client::inner::InnerTcpHandler* handler = static_cast<client::inner::InnerTcpHandler*>(handler_);
  if (handler) {
    auto cb = [handler]() { handler->RequestChannels(); };
    ExecInLoopThread(cb);
  }
}

void NetworkController::saveConfig() {
  common::file_system::ascii_string_path configPath(config_path_);
  common::file_system::File configSave(configPath);
  if (!configSave.open("w")) {
    return;
  }

  configSave.write("[" SETTINGS_SECTION_LABEL "]\n");
  configSave.writeFormated(LOGIN_SETTING_LABEL "=%s\n", config_.login);
  configSave.writeFormated(PASSWORD_SETTING_LABEL "=%s\n", config_.password);
  configSave.close();
}

void NetworkController::readConfig() {
  const char* path = config_path_.c_str();
  TvConfig config;
  // default settings
  config.login = USER_SPECIFIC_DEFAULT_LOGIN;
  config.password = USER_SPECIFIC_DEFAULT_PASSWORD;

  // try to parse settings file
  if (ini_parse(path, ini_handler_fasto, &config) < 0) {
    INFO_LOG() << "Can't load config path: " << path << ", use default settings.";
  }

  SetConfig(config);
}

network::tcp::ITcpLoopObserver* NetworkController::CreateHandler() {
  client::inner::InnerTcpHandler* handler =
      new client::inner::InnerTcpHandler(g_service_host, config_);
  return handler;
}

network::tcp::ITcpLoop* NetworkController::CreateServer(network::tcp::ITcpLoopObserver* handler) {
  client::inner::InnerTcpServer* serv = new client::inner::InnerTcpServer(handler);
  serv->SetName("local_inner_server");
  return serv;
}

}  // namespace network
}  // namespace fastotv
}  // namespace fasto