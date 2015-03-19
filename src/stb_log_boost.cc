#include "stb_log_boost.hh"

#include <boost/make_shared.hpp>
#include <boost/log/core.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/expressions.hpp>
#include <fstream>
#include <stdexcept>

typedef boost::log::sinks::text_ostream_backend backend_t;
typedef boost::log::sinks::synchronous_sink<backend_t> frontend_t;

static void initLogger(const char * filename, bool printToConsole)
{
    std::string logfile(filename);
    if (logfile.empty()) {
        throw std::invalid_argument("Logger: filename was empty");
    }

    boost::shared_ptr<backend_t> backend = boost::make_shared<backend_t>();

    if (printToConsole) {
        boost::shared_ptr<std::ostream> stream(&std::clog, boost::null_deleter());
        backend->add_stream(stream);
    }

    backend->add_stream(boost::make_shared< std::ofstream >(logfile));

    boost::shared_ptr<frontend_t> frontend(new frontend_t(backend));
    frontend->set_formatter(
            boost::log::expressions::stream
            << boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp",  "%Y-%m-%d %H:%M:%S")
            << " "
            << boost::log::trivial::severity
            << " "
            << boost::log::expressions::smessage);

    boost::log::core::get()->add_global_attribute(
            boost::log::aux::default_attribute_names::timestamp(), boost::log::attributes::local_clock());

    boost::log::core::get()->add_sink(frontend);

    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
}

void stb::setLogLevel(const unsigned level)
{
    if (level == 0) {
        boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);
    } else if (level == 1) {
        boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
    } else if (level == 2) {
        boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
    } else if (level == 3) {
        boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::warning);
    } else if (level == 4) {
        boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::error);
    } else if (level == 5) {
        boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::fatal);
    } else if (level == 6){
        boost::log::core::get()->set_filter(boost::log::trivial::severity > boost::log::trivial::fatal);
    }
}

void stb::initFileLogger(const char * filename, const unsigned logLevel)
{
    initLogger(filename, false);
    setLogLevel(logLevel);
}

void stb::initFileConsoleLogger(const char * filename, const unsigned logLevel)
{
    initLogger(filename, true);
    setLogLevel(logLevel);
}