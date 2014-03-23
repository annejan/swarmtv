#ifndef SOURCETABLECONTROLLER_HPP
#define SOURCETABLECONTROLLER_HPP

#include <QObject>

class sourcetablecontroller : public QObject
{
    Q_OBJECT
public:
    explicit sourcetablecontroller(QObject *parent = 0);
    void setTable(QTableWidget *table);
    int updateTable();

signals:

public slots:

private:

};

#endif // SOURCETABLECONTROLLER_HPP
