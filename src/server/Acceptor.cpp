#include <server/Acceptor.h>
#include <component/MemoryHub.h>
#include <utils/String.h>
#include <iostream>
using namespace std;

namespace lsm
{

    Acceptor::Acceptor(const string &name)
        : _name(name), _public_area_name(GetPublicArea(name)),
          _timeout(Time::Millisecond(30))
    {}

    Acceptor::~Acceptor()
    {
        _finished = true;
        if(_listener)
            _listener->join();

        MemoryHub::Instance()->Remove(_public_area_name);
    }

    void Acceptor::Run()
    {
        if(MemoryHub::Instance()->Find(_public_area_name))
        {
            cerr << "Acceptor found a public area with the same name [" << _public_area_name << "]. Will delete it." << endl;
        }

        _manager = MemoryHub::Instance()->Create(_public_area_name, 256);
        _cst = _manager->Construct<size_t>(HelloCST, 0);
        _amt = _manager->Construct<size_t>(HelloAMT, 0);
        _ack = _manager->Construct<size_t>(HelloACK, 0);

        _listener = New<thread>(std::bind(&Acceptor::_Listening, this));
    }

    void Acceptor::_Listening()
    {
        _finished = false;
        while(not _finished)
        {
            Time::Microsecond(100).Sleep();

            //region 抢注阶段，读取抢注号，若不存在则接受
            auto & cst = *_cst.Data();
            if(cst == 0) continue;

            string private_area_name = GetPrivateArea(_name, cst);
            if(MemoryHub::Instance()->Find(private_area_name))
            {
                cout << "Acceptor found a private area with the same name [" << private_area_name << "]. Will ignore new request." << endl;
                cst = 0;
                continue;
            }
            //endregion

            //region 准入阶段，将抢注号填入该字段，给client反馈
            auto & ack = *_ack.Data();
            ack = 0;
            *_amt.Data() = cst;
            //endregion

            //region 确认阶段，等待client确认，若不超时，则确认私有区域建立
            bool status = _timeout.Try([&]() { return ack == cst; });
            if(status)
            {
                cout << "Acceptor: new private area [" << private_area_name << "] built." << endl;
                if(_on_new_client)
                    _on_new_client(private_area_name, cst);
            }
            else
            {
                cout << "Acceptor: private area [" << private_area_name << "] acknowledge timeout. Drop this area." << endl;
            }
            //endregion

            cst = ack = 0;
        }
    }

    void Acceptor::SetOnNewClient(Acceptor::OnNewClient &&handler)
    {
        _on_new_client = handler;
    }

    void Acceptor::SetTimeout(const Time &time)
    {
        _timeout = time;
    }
}

