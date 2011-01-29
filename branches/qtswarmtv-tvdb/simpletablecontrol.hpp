#ifndef SIMPLETABLECONTROL_HPP
#define SIMPLETABLECONTROL_HPP

#include <QObject>
#include <qtablewidget.h>

extern "C" {
#include <swarmtv.h>
}

class simpleTableControl : public QWidget
{
    Q_OBJECT
public:
    explicit simpleTableControl(QWidget *parent = 0);
    void setTable(QTableWidget *table);
    int updateTable();
    void initHeaders();

signals:

public slots:
    void cellDoubleClicked(int row, int column);
    void addSimpleButtonClicked();
    void editSimpleButtonClicked();
    void delSimpleButtonClicked();

private:
    int getRowSelected();
    void fillTable(lastdowned_container *container);

    QTableWidget *table;
};

#endif // SIMPLETABLECONTROL_HPP
