#include <stdlib.h>

extern "C" {
  #include <tvdb.h>
}

#include "thetvdb.hpp"

theTvdb::theTvdb()
{
  tvdb = NULL;
}

theTvdb::theTvdb(char *apiKey)
{
  tvdb = tvdb_init(apiKey);
}

theTvdb::~theTvdb()
{
  tvdb_uninit(tvdb);
}

void theTvdb::initKey(char *apiKey)
{
  if(tvdb == 0) {
    tvdb = tvdb_init(apiKey);
  }
}

htvdb_t theTvdb::getTvdb()
{
  return tvdb;
}
