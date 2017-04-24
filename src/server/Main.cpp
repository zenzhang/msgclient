#include "server/StreamSDK.h"
#include <framework/string/Slice.h>

int main(int argc, char * argv[])
{
    bool b = streamsdk::the_streamsdk()->start_server();
    std::string cmd;

    if (b)
    {
        std::cout << "start server success" << std::endl;
    }
    else
    {
        return 0;
    }


    while (1)
    {
#ifdef BOOST_WINDOWS_API
        Sleep(10 * 1000);
#else
        sleep(10);
#endif // BOOST_WINDOWS_API
        /*std::vector<std::string> cmd_args;
        framework::string::slice<std::string>(cmd, std::inserter(cmd_args, cmd_args.end()), " ");
        if (cmd_args.size() < 1)
        {
        continue;
        }
        else if(cmd_args[0] == "quit")
        {
        break;
        }*/
    }
    streamsdk::the_streamsdk()->stop_server();
    return 0;
}
