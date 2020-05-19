#include "MsgParser.h"
#include "ArduinoJson.hpp"
#include "log.h"

void MsgParser::parser(std::string json, size_t cap) {
    ArduinoJson::DynamicJsonDocument doc(cap);
    auto error = deserializeJson(doc, json);
    if (error) {
        LOGE("deserializeJson error: %s", json.c_str());
        return;
    }
    ID_t id = doc["id"];
    std::string type = doc["type"];
    std::string data = doc["data"];
    LOGD("id: %u, type: %s, data: %s", id, type.c_str(), data.c_str());

    if (type == "set_relay") {
        if (data == "on") {
            onRelay(true, id);
        } else if (data == "off") {
            onRelay(false, id);
        } else {
            LOGE("unknown data: %s", data.c_str());
        }
    }
    else if (type == "set_host_regex") {
        onHostRegex(data, id);
    }
    else if (type == "set_host_passwd") {
        onHostPasswd(data, id);
    }
    else {
        LOGE("unknown type: %s", type.c_str());
    }
}

std::string MsgParser::makeRsp(MsgParser::ID_t id, bool success) {
    ArduinoJson::DynamicJsonDocument doc(1024);
    doc["id"] = id;
    doc["type"] = Msg::Type::RSP;
    doc["success"] = success;
    std::string result;
    serializeJson(doc, result);
    return result;
}

std::string MsgParser::makeMsg(const std::string& type, const std::string& msg) {
    ArduinoJson::DynamicJsonDocument doc(1024);
    doc["id"] = _id++;
    doc["type"] = type;
    doc["data"] = msg;
    std::string result;
    serializeJson(doc, result);
    return result;
}

void MsgParser::test() {
    char jsonFromHost[] = R"({
    "id": 0
    ,"type":"set_relay"
    ,"data":"on"
    })";

    MsgParser msgParser;
    msgParser.setRelayCb([](bool relay, MsgParser::ID_t id) {
        LOG("RelayCb: %d, %u", relay, id);
        auto json = MsgParser::makeRsp(id);
        LOG("rsp: %s", json.c_str());
    });
    msgParser.parser(jsonFromHost);

    LOG("make msg: %s", msgParser.makeMsg(Msg::Type::MSG, "hello").c_str());
    LOG("make msg: %s", msgParser.makeMsg(Msg::Type::MSG, "world").c_str());
}
