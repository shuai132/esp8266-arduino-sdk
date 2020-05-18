#pragma once

#include <string>
#include <cstdint>
#ifdef SUPPORT_ATOMIC_FETCH
#include <atomic>
#endif

#include "MakeEvent.hpp"

namespace Msg {
namespace Type {
const static char* MSG = "msg";
const static char* RSP = "rsp";
}
}

class MsgParser {
public:
    using ID_t = uint32_t;
    /**
     {
        "id": number    消息id
        ,"type": string 可取"set_relay" "set_host_regex" "set_host_passwd"
        ,"data": string
     }
     */
    void parser(std::string json, size_t cap = 1024);
    MAKE_EVENT(Relay, bool, ID_t);
    MAKE_EVENT(HostRegex, std::string);
    MAKE_EVENT(HostPasswd, std::string);

    /**
     {
        "id": number     消息id
        ,"type": string  可取"rsp" 或其他自定义字符串
        ,"data": string
        ,"success": bool 仅当type为"rsp"时存在
     }
     */
    static std::string makeRsp(ID_t id, bool success = true);
    std::string makeMsg(const std::string& type, const std::string& msg);

private:
#ifdef SUPPORT_ATOMIC_FETCH
    std::atomic<ID_t> _id{0};
#else
    ID_t _id{0};
#endif

public:
    static void test();
};
