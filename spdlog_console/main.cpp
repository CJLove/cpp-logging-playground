#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <thread>
#include <chrono>

class AppClass {
public:
    AppClass(const std::string &name): m_thread([this]() { this->thread(); }), m_name(name),
        m_logger(spdlog::stdout_logger_mt(name))
    {}

    ~AppClass() {
        {
        std::lock_guard<std::mutex> l(m_mutex);
        m_stop = true;
        }
        m_cond.notify_one();
        m_thread.join();
    }

    void thread() {
        m_logger->trace("Thread {} starting",m_name);
        while (this->wait_for(std::chrono::seconds(5))) {
            m_logger->info("Thread {} doing something",m_name);
        }
        m_logger->trace("Thread {} exiting",m_name);
    }

    template<class Duration>
    bool wait_for(Duration duration) {
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


int main(int argc, char**argv) {
    auto logger = spdlog::stdout_logger_mt("main");
    logger->info("spdlog_console logging");

    std::vector<std::unique_ptr<AppClass>> threads;
    for (int i =0; i < 10; i++) {
        threads.emplace_back(std::make_unique<AppClass>(std::string("Worker thread ") + std::to_string(i)));
    }

    logger->info("Main thread sleeping for 2 minutes");
    std::this_thread::sleep_for(std::chrono::minutes(2));

    logger->info("Exiting");

}