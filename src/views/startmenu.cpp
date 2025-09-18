#include "../../headers/views/startmenu.h"

StartMenu::StartMenu(QWidget *parent)
    : QWidget(parent)
{
    beginButton = new QPushButton("Begin Drawing", this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(beginButton, 0, Qt::AlignHCenter);
    layout->addStretch();

    connect(beginButton, &QPushButton::clicked, this, &StartMenu::onBeginButtonClicked);
}

void StartMenu::onBeginButtonClicked()
{
    emit beginDrawing();
}