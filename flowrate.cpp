#include "flowrate.h"
#include "ui_flowrate.h"

FlowRate::FlowRate(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FlowRate)
{
    ui->setupUi(this);
}

FlowRate::~FlowRate()
{
    delete ui;
}
