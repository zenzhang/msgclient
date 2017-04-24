//HttpServer.cpp

#include "client/Common.h"
#include "client/FileTask.h"
#include "client/ServerError.h"
#include "client/util.h"
#include "client/StreamSDK.h"

#include <framework/timer/AsioTimerManager.h>
#include <framework/timer/TickCounter.h>

#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::logger;
using namespace framework::system;

#include <boost/bind.hpp>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("FileTask", 0);

namespace streamsdk
{
    FileTask::FileTask()
    {
    }


    FileTask::FileTask(
        const std::string& own,
        const std::string& remote,
        const std::string& filename,
        const std::string& server,
        boost::uint32_t task_id
        )
        : own_name_(own)
        , remote_name_(remote)
        , filename_(filename)
        , server_(server)
        , task_id_(task_id)
    {

    }

    FileTask::~FileTask()
    {

    }
   
    void FileTask::start()
    {

    }

   

    void FileTask::on_timer()
    {

    }


    void FileTask::OnUdpRecv(
        protocol::Packet const & packet)
    {
        switch (packet.packet_action_)
        {
        case protocol::ZHelloPacket::Action:
        {
        }
        break;
        case protocol::ZStunPacket::Action:
        {
        }
        break;
        default:
            break;
        }
    }

} // namespace streamsdk


