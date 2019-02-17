//
// Created by yarten on 19-2-10.
//
#include <server/Acceptor.h>
#include <server/Maintainer.h>
#include <component/MemoryHub.h>

using namespace std;
using namespace lsm;


int main(int argc, char ** argv)
{
    string server_name(DefaultServerName);
    if(argc > 1)
        server_name = argv[1];

    Acceptor acceptor(server_name);
    Maintainer maintainer(server_name);

    acceptor.SetOnNewClient([&](const string & name, size_t id) {
        maintainer.AddClient(name);
    });

    acceptor.Run();

    while (true)
        this_thread::yield();

    return 0;
}