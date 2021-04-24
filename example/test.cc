#include "Bus.h"
#include "LinuxSocket.h"

#include <unistd.h>

using namespace kickcat;

int main()
{
    auto socket = std::make_shared<LinuxSocket>();
    Error err = socket->open("enp0s31f6");
    if (err)
    {
        err.what();
        return 1;
    }

    Bus bus(socket);

    err = bus.init();
    if (err)
    {
        err.what();
        return 1;
    }

    for (auto& slave : bus.slaves())
    {
        State state = bus.getCurrentState(slave);
        printf("Slave %d state is %s - %x\n", slave.address, toString(state), state);
    }

    bus.printSlavesInfo();

    uint8_t io_buffer[1024];
    err = bus.createMapping(io_buffer);
    if (err)
    {
        err.what();
        return 1;
    }

    bus.requestState(State::SAFE_OP);
    bus.waitForState(State::SAFE_OP, 10ms);
    for (auto& slave : bus.slaves())
    {
        State state = bus.getCurrentState(slave);
        printf("Slave %d state is %s - %x\n", slave.address, toString(state), state);
    }

    // do a round trip to let the bus switch to OP
    bus.sendProcessData();

    bus.requestState(State::OPERATIONAL);
    bus.waitForState(State::OPERATIONAL, 10ms);
    for (auto& slave : bus.slaves())
    {
        State state = bus.getCurrentState(slave);
        printf("Slave %d state is %s - %x\n", slave.address, toString(state), state);
    }

    auto& slave = bus.slaves().at(0);
    uint8_t* slave0_in = slave.mapping.at(0).client;
    if (slave.mapping.at(0).type == SyncManagerType::Output)
    {
        uint8_t* slave0_in = slave.mapping.at(1).client;
    }
    for (int32_t i = 0; i < 1000; ++i)
    {
        sleep(10ms);
        bus.sendProcessData();

        for (int32_t j = 0;  j < 32; ++j)
        {
            printf("%02x ", slave0_in[j]);
        }
        printf("\n");
    }


    return 0;
}
