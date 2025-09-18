#ifndef STARTMENU_H
#define STARTMENU_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class StartMenu : public QWidget
{
    Q_OBJECT

public:
    explicit StartMenu(QWidget *parent = nullptr);

signals:
    void beginDrawing();

private slots:
    void onBeginButtonClicked();

private:
    QPushButton *beginButton;
};

#endif // STARTMENU_H