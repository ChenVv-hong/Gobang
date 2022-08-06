#ifndef MATCHWAIT_H
#define MATCHWAIT_H

#include <QWidget>

namespace Ui {
class matchwait;
}

class matchwait : public QWidget
{
    Q_OBJECT

public:
    explicit matchwait(QWidget *parent = nullptr);
    ~matchwait();

private:
    Ui::matchwait *ui;
};

#endif // MATCHWAIT_H
