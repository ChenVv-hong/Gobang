#ifndef PLAYERITEM_H
#define PLAYERITEM_H

#include <QWidget>

namespace Ui {
class PlayerItem;
}

class PlayerItem : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerItem(QWidget *parent = nullptr);
    ~PlayerItem();
    void setData(QString num,QString uid, QString name, QString points);
private:
    Ui::PlayerItem *ui;
};

#endif // PLAYERITEM_H
