import QtQuick 2.14
import QtQuick.Controls 2.14
import Laventh.QML_Window 1.0


Rectangle
{
    id: root
    color: "#EEEEEE"
    onWidthChanged: fadeIn()
    onHeightChanged: fadeIn()

    function fadeIn()
    {
        if(width === 0) return
        animateOpacity.restart()
    }

    NumberAnimation
    {
        id: animateOpacity
        target: content
        properties: "opacity"
        duration: 200
        from: 0
        to: 1
    }


    // Window content.
    Rectangle
    {
        id: content
        anchors.fill: parent
        color: "transparent"

        // Title bar.
        Rectangle
        {
            id: titleBar
            width: parent.width
            height: 30
            color: "#DDDDDD"
        }

        // Exit button.
        WindowControl
        {
            id: exitButton
            anchors.right: titleBar.right
            MouseArea { anchors.fill: parent; onClicked: QML_Window.close() }
        }

        // Maximize button.
        WindowControl
        {
            id: maximizeButton
            anchors.right: exitButton.left
            MouseArea
            { anchors.fill: parent; onClicked: QML_Window.toggle_maximize() }
        }

        // Minimize button.
        WindowControl
        {
            id: minimizeButton
            anchors.right: maximizeButton.left
            MouseArea { anchors.fill: parent; onClicked: QML_Window.minimize() }
        }

        // Text.
        Text
        {
            anchors.centerIn: parent
            text: "Borderless Window"
            font.pixelSize: 27
            color: "#AAAAAA"
        }
    }
}
