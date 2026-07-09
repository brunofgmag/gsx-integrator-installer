import QtQuick

Item {
    id: radioRow

    property string label
    property bool checked: false
    signal selected()

    implicitWidth: radioContent.implicitWidth
    implicitHeight: radioContent.implicitHeight

    Row {
        id: radioContent
        spacing: 10

        Rectangle {
            width: 16
            height: 16
            anchors.verticalCenter: parent.verticalCenter
            radius: 8
            color: "transparent"
            border.color: radioRow.checked ? Theme.accent : Theme.line
            border.width: 1

            Rectangle {
                anchors.centerIn: parent
                width: 8
                height: 8
                radius: 4
                color: Theme.accent
                visible: radioRow.checked
            }
        }
        Text {
            text: radioRow.label
            anchors.verticalCenter: parent.verticalCenter
            color: Theme.text
            font.pixelSize: 13
        }
    }
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: radioRow.selected()
    }
}
