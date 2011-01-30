#ifndef THETVDB_HPP
#define THETVDB_HPP

extern "C" {
  #include <tvdb.h>
}

class theTvdb
{
public:
    theTvdb();
    theTvdb(char *apiKey);
    ~theTvdb();

    void initKey(char *apiKey);
    htvdb_t getTvdb();
private:
    htvdb_t tvdb;
};

#endif // THETVDB_HPP
