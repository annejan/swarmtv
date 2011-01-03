#ifndef SOURCETABLECONTROL_HPP
#define SOURCETABLECONTROL_HPP

#include <QObject>
#include <QWidget>
#include <qtablewidget.h>

extern "C" {
#include <swarmtv.h>
}

class sourceTableControl : public QWidget
{
    Q_OBJECT
public:
    explicit sourceTableControl(QWidget *parent = 0);
    void setTable(QTableWidget *table);
    int updateTable();
    void initHeaders();

signals:

public slots:
    void addButtonClicked(void);
    void delButtonClicked(void);
    void cellDoubleClicked(int row, int column);

private:
    void fillTable(source_container *container);

    QTableWidget *table;
};

#endif // SOURCETABLECONTROL_HPP
