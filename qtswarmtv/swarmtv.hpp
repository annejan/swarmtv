#ifndef SWARMTV_H
#define SWARMTV_H


class swarmTv
{
public:
    swarmTv();
    ~swarmTv();
    rsstor_handle *getHandle();

private:
    rsstor_handle *handle;
};

#endif // SWARMTV_H
