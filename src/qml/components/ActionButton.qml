import QtQuick

Rectangle {
    id: button

    property string label
    property bool primary: true
    property bool enabled: true
    signal clicked()

    width: buttonText.implicitWidth + 48
    height: 40
    radius: Theme.radius
    color: !button.enabled ? Theme.panel2
                           : button.primary ? (buttonArea.containsMouse ? Qt.lighter(Theme.accent, 1.1)
                                                                        : Theme.accent)
                                            : (buttonArea.containsMouse ? Theme.panel : Theme.panel2)
    border.color: button.primary ? "transparent" : Theme.line
    border.width: button.primary ? 0 : 1

    Text {
        id: buttonText
        anchors.centerIn: parent
        text: button.label
        color: !button.enabled ? Theme.faint : button.primary ? Theme.accentText : Theme.text
        font.pixelSize: 14
    }
    MouseArea {
        id: buttonArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: button.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: if (button.enabled) button.clicked()
    }
}
