#ifndef INFO_WINDOW_H
#define INFO_WINDOW_H

#include <QDialog>
#include <QLabel>

class InfoWindow : public QDialog
{
    Q_OBJECT

public:
    explicit InfoWindow(const QString &username, const QString &login_time, QWidget *parent = nullptr);
    ~InfoWindow();

private:
    void setup_ui();
    void apply_theme();

    QString username;
    QString login_time;
};

#endif // INFO_WINDOW_H
