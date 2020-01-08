#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QQuickWidget>


class QML_Window : public QWidget
{
    Q_OBJECT

public:
    QML_Window(int width, int height,
        const COLORREF& background_color = RGB(0, 0, 0),
        int x = -1, int y = -1);
    ~QML_Window();

    void add_widget(const QUrl& qml_source);
    void show();

    Q_INVOKABLE void minimize();
    Q_INVOKABLE void toggle_maximize();
    Q_INVOKABLE void close();

    void setGeometry(int x, int y, int w, int h);
    HWND get_handle(){ return handle; }

protected:
    bool nativeEvent(const QByteArray& eventType, void *message, long *result) override;

private:
    QHBoxLayout *layout{nullptr};
    QQuickWidget *qml_widget{nullptr};
    HWND handle{nullptr};
    bool maximized{false};
};
