#include <qlabel.h>

#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>

extern "C" {
#include <swarmtv.h>
}

#include <iostream>

#include "singleton.h"
#include "swarmtv.hpp"
#include "simplecell.hpp"
#include "sourcetablecontroller.hpp"

sourcetablecontroller::sourcetablecontroller(QObject *parent) :
    QObject(parent)
{
}


void sourceTableControl::setTable(QTableWidget *table)
{
    // Store the object we are going to control
    this->table = table;
}
