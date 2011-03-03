#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <swarmtv.h>

#include "swarmtv.hpp"

swarmTv::swarmTv()
{
    // Initialize the handle.
    handle = initrsstor();
}

swarmTv::~swarmTv()
{
    // Destroy the handle.
    freersstor(handle);
}

rsstor_handle *swarmTv::getHandle()
{
    // When the handle is requested, provide it.
    return handle;
}
