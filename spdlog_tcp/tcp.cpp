#include <spdlog/spdlog.h>
#include <spdlog/sinks/tcp_sink.h>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <iostream>
#include <memory>
#ifndef WIN32
#include <unistd.h>
#endif

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
    AppClass(const std::string &name) : m_thread([this]() { this->thread(); }), 
m_name(name),
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
              << "spdlog_tcp [-l <logLevel>][-i <ipAddr>][-p <port>]\n";
}

#ifdef WIN32
#include <string.h>
#include <stdio.h>

int     opterr = 1,             /* if error message should be printed */
  optind = 1,             /* index into parent argv vector */
  optopt,                 /* character checked for validity */
  optreset;               /* reset getopt */
char    *optarg;                /* argument associated with option */

#define BADCH   (int)'?'
#define BADARG  (int)':'
#define EMSG    ""

/*
* getopt --
*      Parse argc/argv argument vector.
*/
int
  getopt(int nargc, char * const nargv[], const char *ostr)
{
  static char *place = EMSG;              /* option letter processing */
  const char *oli;                        /* option letter list index */

  if (optreset || !*place) {              /* update scanning pointer */
    optreset = 0;
    if (optind >= nargc || *(place = nargv[optind]) != '-') {
      place = EMSG;
      return (-1);
    }
    if (place[1] && *++place == '-') {      /* found "--" */
      ++optind;
      place = EMSG;
      return (-1);
    }
  }                                       /* option letter okay? */
  if ((optopt = (int)*place++) == (int)':' ||
    !(oli = strchr(ostr, optopt))) {
      /*
      * if the user didn't specify '-' as an option,
      * assume it means -1.
      */
      if (optopt == (int)'-')
        return (-1);
      if (!*place)
        ++optind;
      if (opterr && *ostr != ':')
        (void)printf("illegal option -- %c\n", optopt);
      return (BADCH);
  }
  if (*++oli != ':') {                    /* don't need argument */
    optarg = NULL;
    if (!*place)
      ++optind;
  }
  else {                                  /* need an argument */
    if (*place)                     /* no white space */
      optarg = place;
    else if (nargc <= ++optind) {   /* no arg */
      place = EMSG;
      if (*ostr == ':')
        return (BADARG);
      if (opterr)
        (void)printf("option requires an argument -- %c\n", optopt);
      return (BADCH);
    }
    else                            /* white space */
      optarg = nargv[optind];
    place = EMSG;
    ++optind;
  }
  return (optopt);                        /* dump back option letter */
}
#endif

int main(int argc, char **argv)
{
    int logLevel = spdlog::level::trace;
    std::string ipAddr {"127.0.0.1"};
    uint16_t port = 9000;
    int c;
    while ((c = getopt(argc, argv, "l:i:p:?")) != EOF)
    {
        switch (c)
        {
        case 'l':
            logLevel = std::stoi(optarg);
            break;
        case 'i':
            ipAddr = optarg;
            break;
        case 'p':
            port = static_cast<uint16_t>(std::stoi(optarg));
            break;
        case '?':
            usage();
            exit(1);
            break;
        default:
            break;
        }
    }

    // Create the main logger named "logger" and configure it
    spdlog::sinks::tcp_sink_config cfg(ipAddr,port);
    auto sink = std::make_shared<spdlog::sinks::tcp_sink_mt>(cfg);
    auto logger = std::make_shared<spdlog::logger>("logger",sink);
    spdlog::register_logger(logger);
    // Log format:
    // 2018-10-08 21:08:31.633|020288|I|Thread Worker thread 3 doing something
    logger->set_pattern("%Y-%m-%d %H:%M:%S.%e|%t|%L|%v");
    // Set the log level for filtering
    spdlog::set_level(static_cast<spdlog::level::level_enum>(logLevel));

    logger->info("Begin spdlog_udp logging");

    std::vector<std::unique_ptr<AppClass>> threads;
    for (int i = 0; i < 10; i++)
    {
        threads.emplace_back(std::make_unique<AppClass>(std::string("Worker thread ") + std::to_string(i)));
    }

    logger->info("Main thread sleeping for 2 minutes");
    std::this_thread::sleep_for(std::chrono::minutes(2));

    logger->info("End spdlog_file logging");
}
