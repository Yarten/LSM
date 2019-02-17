#include <LSM>
#include <utils/String.h>
#include <iostream>

using namespace lsm;
using namespace std;

int main(int argc, char ** argv)
{
    LSM client;
    
    client.SetOnReceive([](LSM & lsm, const string & channel, string msg) {
        Trim(msg);
        cout << "receive from [" << channel << "]: " << msg << endl;
    });
    
    client.SetOnError([](LSM & lsm, const string & channel) {
        cout << "error occurs on [" << channel << "]." << endl; 
    });
    
    if(not client.Connect())
    {
        cout << "Fail to connect to server" << endl;
        return 1;
    }

    if(not client.Open("test"))
    {
        cout << "Fail to open channel" << endl;
        return 2;
    }
    
    int n = 0;
    while (true)
    {
        string msg = FormatOut("%d", n++);
        cout << "send message: " << msg << endl;
        client.Send("test", msg);
        
        Time::Second(1).Sleep();
    }

    return 0;
}

