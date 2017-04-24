#include "client/StreamSDK.h"
#include <framework/string/Slice.h>


#ifdef BIN
int main(int argc, char * argv[])
{
    bool b = streamsdk::the_streamsdk()->start_server();
    std::string cmd;

    if (b)
    {
        std::cout << "start client success" << std::endl;
    }
    else
    {
        return 0;
    }
    std::string message;
    streamsdk::the_streamsdk()->login("zenzhang", message);
    std::cout << "[login] message:" << message << std::endl;

	while (std::getline(std::cin, cmd))
	{
        std::vector<std::string> cmd_args;
        framework::string::slice<std::string>(cmd, std::inserter(cmd_args, cmd_args.end()), " ");
        if (cmd_args.size() < 1)
        {
            continue;
        }
        else if (cmd_args[0] == "logout")
        {
            streamsdk::the_streamsdk()->logout();
        }
        else if(cmd_args[0] == "exit")
        {
            break;
        }
	}
    streamsdk::the_streamsdk()->stop_server();
    return 0;
}
#endif
