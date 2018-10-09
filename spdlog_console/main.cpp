#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <unistd.h>

class TraceMethod
{
  public:
    TraceMethod(const char *name,
                std::shared_ptr<spdlog::logger> logger) : m_name(name),
                                                          m_logger(logger)
    {
        m_logger->trace("{}() Enter", m_name);
    }

    ~TraceMethod()
    {
        m_logger->trace("{}() Leave", m_name);
    }

  private:
    std::string m_name;
    std::shared_ptr<spdlog::logger> m_logger;
};

class AppClass
{
  public:
    AppClass(const std::string &name) : m_thread([this]() { this->thread(); }), m_name(name),
                                        m_logger(spdlog::get("logger"))
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
        m_logger->trace("Thread {} starting", m_name);
        while (this->wait_for(std::chrono::seconds(5)))
        {
            anotherMethod();
            m_logger->info("Thread {} doing something", m_name);
        }
        m_logger->trace("Thread {} exiting", m_name);
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
    std::shared_ptr<spdlog::logger> m_logger;
};

void usage()
{
    std::cerr << "Usage:\n"
              << "spdlog_console [-l <logLevel>]\n";
}

int main(int argc, char **argv)
{
    int logLevel = spdlog::level::trace;
    int c;
    while ((c = getopt(argc, argv, "l:?")) != EOF)
    {
        switch (c)
        {
        case 'l':
            logLevel = std::stoi(optarg);
            break;
        case '?':
            usage();
            break;
        default:
            break;
        }
    }

    // Create the main logger named "logger" and configure it
    auto logger = spdlog::stdout_logger_mt("logger");
    // Log format:
    // 2018-10-08 21:08:31.633|020288|I|Thread Worker thread 3 doing something
    logger->set_pattern("%Y-%m-%d %H:%M:%S.%e|%t|%L|%v");
    // Set the log level for filtering
    spdlog::set_level(static_cast<spdlog::level::level_enum>(logLevel));

    logger->info("spdlog_console logging");

    std::vector<std::unique_ptr<AppClass>> threads;
    for (int i = 0; i < 10; i++)
    {
        threads.emplace_back(std::make_unique<AppClass>(std::string("Worker thread ") + std::to_string(i)));
    }

    logger->info("Main thread sleeping for 2 minutes");
    std::this_thread::sleep_for(std::chrono::minutes(2));

    logger->info("Exiting");
}