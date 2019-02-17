#include <utils/String.h>
#include <boost/algorithm/string.hpp>

namespace lsm
{

    void Trim(std::string &s)
    {
        s = s.substr(0, s.find(char(0)));
        boost::trim(s);
    }
}
