#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/global_logger_storage.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
using namespace logging::trivial;

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iostream>
#include <unistd.h>



class TraceMethod
{
  public:
    TraceMethod(const char *name,
                src::severity_logger< severity_level > &logger): m_name(name),
                                                          m_logger(logger)
    {
        BOOST_LOG_SEV(m_logger, trace) << m_name << "() Enter";
    }

    ~TraceMethod()
    {
        BOOST_LOG_SEV(m_logger, trace) << m_name << "() Leave";
    }

  private:
    std::string m_name;
    src::severity_logger< severity_level > &m_logger;    
};

class AppClass
{
  public:
    AppClass(const std::string &name) : m_thread([this]() { this->thread(); }), m_name(name)
                                        
    {
    }

    ~AppClass()
    {
        {
            std::lock_guard<std::mutex> l(m_mutex);
            m_stop = true;
        }
        m_cond.notify_one();
        m_thread.join();
    }

    void thread()
    {
        BOOST_LOG_SEV(m_logger, trace) << "Thread " << m_name << " starting";
        
        while (this->wait_for(std::chrono::seconds(5)))
        {
            anotherMethod();
            BOOST_LOG_SEV(m_logger, info) << "Thread " << m_name << " doing something";
        }
        BOOST_LOG_SEV(m_logger, trace) << "Thread " << m_name << " exiting";
    }

    void anotherMethod()
    {
        TraceMethod tm("anotherMethod", m_logger);
    }

    template <class Duration>
    bool wait_for(Duration duration)
    {
        std::unique_lock<std::mutex> l(m_mutex);
        return !m_cond.wait_for(l, duration, [this]() { return m_stop; });
    }

  private:
    std::condition_variable m_cond;
    std::mutex m_mutex;
    std::thread m_thread;
    bool m_stop = false;
    std::string m_name;
    src::severity_logger< severity_level > m_logger;
    
};

void usage()
{
    std::cerr << "Usage:\n"
              << "boost_file [-l <logLevel>][-f <logFile>][-s <size>]\n";
}

int main(int argc, char **argv)
{
    int logLevel = logging::trivial::trace;
    std::string logFile {"logfile_%N.txt"};
    int c;
    int size = 1024;
    while ((c = getopt(argc, argv, "l:f:s:?")) != EOF)
    {
        switch (c)
        {
        case 'l':
            logLevel = std::stoi(optarg);
            break;
        case 'f':
            logFile = optarg;
            break;
        case 's':
            size = std::stoi(optarg);
            break;
        case '?':
            usage();
            exit(1);
            break;
        default:
            break;
        }
    }

    logging::add_file_log(
        keywords::file_name = logFile,
        keywords::rotation_size = size,
        //keywords::format = "%TimeStamp%|%Severity%|%Message%"
        keywords::format = 
        (
            expr::stream
            << expr::format_date_time<boost::posix_time::ptime>("TimeStamp","%Y-%m-%d %H:%M%S.%f")
            << "|"
            << expr::attr<boost::log::attributes::current_thread_id::value_type >("ThreadID")
            << "|"
            << logging::trivial::severity
            << "|"
            << expr::smessage
        )    );

    logging::core::get()->set_filter(
        logging::trivial::severity >= logLevel
    );

    logging::add_common_attributes();

    src::severity_logger< severity_level > logger;

    BOOST_LOG_SEV(logger, info) << "Begin boost_file logging";
    
    std::vector<std::unique_ptr<AppClass>> threads;
    for (int i = 0; i < 10; i++)
    {
        threads.emplace_back(std::make_unique<AppClass>(std::string("Worker thread ") + std::to_string(i)));
    }

    BOOST_LOG_SEV(logger, info) << "Main thread sleeping for 2 minutes";
    
    std::this_thread::sleep_for(std::chrono::minutes(2));

    BOOST_LOG_SEV(logger, info) << "End boost_file logging";
    
}