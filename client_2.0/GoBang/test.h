#ifndef TEST_H
#define TEST_H

#include <QMainWindow>
#include <net.h>
#include <QMessageBox>
namespace Ui {
class test;
}

class test : public QMainWindow
{
    Q_OBJECT

public:
    explicit test(QWidget *parent = nullptr);
    ~test();

private slots:
    void pressButton();
protected:
    void closeEvent(QCloseEvent *event);

public:
    Ui::test *ui;
private:
    QMessageBox b;
};

#endif // TEST_H
