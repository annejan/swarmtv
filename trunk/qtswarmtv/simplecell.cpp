#include <QLabel>
#include "simplecell.hpp"

simpleCell::simpleCell(QWidget *parent) :
    QLabel(parent)
{
}

simpleCell::simpleCell ( const QString & text, QWidget * parent, Qt::WindowFlags f) :
    QLabel(text)
{
}

int simpleCell::getSimpleId()
{
    return simpleId;
}

void simpleCell::setSimpleId(int simpleId)
{
    this->simpleId = simpleId;
}
