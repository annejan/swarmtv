#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>

extern "C" {
#include <swarmtv.h>
}

#include <qstring.h>

#include "singleton.h"
#include "swarmtvstats.hpp"
#include "swarmtv.hpp"
#include "readablesize.hpp"


swarmTvStats::swarmTvStats()
{
}

QString *swarmTvStats::getStatsString()
{
    int rc=0;
    stats_struct stats;
    QString *statsstr = new QString();
    std::string sizestr;
    swarmTv *swarm = &Singleton<swarmTv>::Instance();

    // Get the statistics
    rc = rsstgetstats(swarm->getHandle(), &stats);

    // Convert size
    readableSize readsize(stats.dbsize);
    readsize.getSize(sizestr);

    // Create string
    statsstr->sprintf(
        "Statistics for SwarmTv %s\n"
        "================================\n"
        "Database version         : %d\n"
        "Number of sources        : %d\n"
        "Number of simple filters : %d\n"
        "Number of SQL filters    : %d\n"
        "Number of meta files     : %d\n"
        "Number of downloaded     : %d\n"
        "Database Size            : %s\n",
        stats.version, stats.database, stats.sources,
        stats.simples, stats.sqls, stats.metafile,
        stats.downloaded, sizestr.c_str()
        );

    return statsstr;
}
