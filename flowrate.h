#ifndef FLOWRATE_H
#define FLOWRATE_H

#include <QWidget>

namespace Ui {
class FlowRate;
}

class FlowRate : public QWidget
{
    Q_OBJECT

public:
    explicit FlowRate(QWidget *parent = nullptr);
    ~FlowRate();

private:
    Ui::FlowRate *ui;
};

#endif // FLOWRATE_H
