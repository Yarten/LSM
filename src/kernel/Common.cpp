#include <kernel/Common.h>
#include <utils/String.h>

namespace lsm
{

    std::string GetPublicArea(const std::string &server)
    {
        return server + "_public_";
    }

    std::string GetPrivateArea(const std::string &server, size_t id)
    {
        return FormatOut("%s_private_%lu", server.c_str(), id);
    }

    std::string GetSwapArea(const std::string &server, const std::string &channel)
    {
        return server + "_swap_" + channel;
    }
}

