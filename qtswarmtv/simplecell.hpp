#ifndef SIMPLECELL_HPP
#define SIMPLECELL_HPP

#include <QLabel>
#include <QWidget>

class simpleCell : public QLabel
{
    Q_OBJECT
public:
    explicit simpleCell(QWidget *parent = 0);
    simpleCell(const QString & text, QWidget * parent = 0, Qt::WindowFlags f = 0);
    int getSimpleId();
    void setSimpleId(int simpleId);

signals:

public slots:

private:
    int simpleId;
};

#endif // SIMPLECELL_HPP
