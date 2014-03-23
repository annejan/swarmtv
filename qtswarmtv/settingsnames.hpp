#ifndef SETTINGSNAMES_HPP
#define SETTINGSNAMES_HPP

// Tvdb API key
static const QString TVDB_API_CONFIG("config/tvdbapiconfig");

// Which signals to show in the icontray pop messages
static const QString TRAY_SHOW_DBUSSTART("trayicon/showdbusstart"); // show start of runcycle
static const QString TRAY_SHOW_DBUSSTOP("trayicon/showdbusstop"); // show end of run cycle
static const QString TRAY_SHOW_RSS("trayicon/showdbusrss"); // show downloading of RSS feeds
static const QString TRAY_SHOW_SIMPLE("trayicon/showdbussimple"); // show simple filter handling
static const QString TRAY_SHOW_SQL("trayicon/showdbussql"); // No sql support in frontend
static const QString TRAY_SHOW_DOWNED("trayicon/showdbusdowned"); // Show message when a download is performed


#endif // SETTINGSNAMES_HPP
