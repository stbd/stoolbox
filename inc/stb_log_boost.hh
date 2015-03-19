#ifndef BASE_LOGGER_HPP_
#define BASE_LOGGER_HPP_

#include <string>

#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/attributes/named_scope.hpp>

#define BASE_LOG_PREFIX << "(" << __FILE__ << " " << __LINE__ << "): "

#define LogTrace(logger) BOOST_LOG_SEV((logger.mBoostLogger), boost::log::trivial::trace)  BASE_LOG_PREFIX
#define LogDebug(logger) BOOST_LOG_SEV((logger.mBoostLogger), boost::log::trivial::debug)  BASE_LOG_PREFIX
#define LogInfo(logger) BOOST_LOG_SEV((logger.mBoostLogger), boost::log::trivial::info)    BASE_LOG_PREFIX
#define LogWarn(logger) BOOST_LOG_SEV((logger.mBoostLogger), boost::log::trivial::warning) BASE_LOG_PREFIX
#define LogError(logger) BOOST_LOG_SEV((logger.mBoostLogger), boost::log::trivial::error)  BASE_LOG_PREFIX
#define LogFatal(logger) BOOST_LOG_SEV((logger.mBoostLogger), boost::log::trivial::fatal)  BASE_LOG_PREFIX

#define LogScope(scope) BOOST_LOG_NAMED_SCOPE((scope))

namespace stb
{
    typedef boost::log::sources::severity_logger<boost::log::trivial::severity_level> BoostLogger;
    struct Log
    {
        BoostLogger mBoostLogger;
    };

    void initFileLogger(const char * filename, const unsigned logLevel);
    void initFileConsoleLogger(const char * filename, const unsigned logLevel);

    /*
     * Possible log levels go from 0 (all logs) to 6 (no logs at all, not even fatal)
     */
    void setLogLevel(const unsigned level);
}

#endif
