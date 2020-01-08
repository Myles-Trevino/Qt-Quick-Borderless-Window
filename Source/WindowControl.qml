import QtQuick 2.14


Rectangle
{
    id: windowControl
    anchors.rightMargin: 10
    anchors.verticalCenter: titleBar.verticalCenter
    width: 15
    height: width
    radius: width*.5
    color: "#AAAAAA"
    state: "DEFAULT"


    MouseArea
    {
        id: mouseArea
        anchors.fill: parent
        onEntered: windowControl.state = "HOVERED"
        onExited: windowControl.state = ""
        hoverEnabled: true
    }

    states:
        State
        {
            name: "HOVERED"
            PropertyChanges { target: windowControl; opacity: .7 }
        }

    transitions: 
        Transition
        {
            to: "*"
            NumberAnimation { target: windowControl; property: "opacity"; duration: 200 }
        }
}