//FileTask.h

#ifndef      _STREAMSDK_FILE_TASK_
#define      _STREAMSDK_FILE_TASK_

#include <protocol/UdpServer.h>

#include <framework/timer/Timer.h>

namespace streamsdk
{
    class FileTask
        : public boost::enable_shared_from_this<FileTask>
    {
    public:
        typedef boost::shared_ptr<FileTask> p;


        FileTask();

        FileTask(
            const std::string& own, 
            const std::string& remote,
            const std::string& filename,
            const std::string& server,
            boost::uint32_t task_id
                );

        void start();

        void on_timer();

        virtual ~FileTask();

        void OnUdpRecv(
            protocol::Packet const & packet);

        const std::string& get_remote_name()
        {
            return remote_name_;
        }

    private:
        std::string own_name_;
        std::string remote_name_;
        std::string filename_;
        std::string server_;
        boost::uint32_t task_id_;
    };

} // namespace streamsdk

#endif
