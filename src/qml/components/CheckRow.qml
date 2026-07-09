import QtQuick

Item {
    id: checkRow

    property string label
    property bool checked: true
    signal toggled(bool checked)

    implicitWidth: checkContent.implicitWidth
    implicitHeight: checkContent.implicitHeight

    Row {
        id: checkContent
        spacing: 10

        Rectangle {
            width: 16
            height: 16
            anchors.verticalCenter: parent.verticalCenter
            radius: Theme.radiusSmall
            color: "transparent"
            border.color: checkRow.checked ? Theme.accent : Theme.line
            border.width: 1

            Rectangle {
                anchors.centerIn: parent
                width: 8
                height: 8
                radius: 1
                color: Theme.accent
                visible: checkRow.checked
            }
        }
        Text {
            text: checkRow.label
            anchors.verticalCenter: parent.verticalCenter
            color: Theme.text
            font.pixelSize: 13
        }
    }
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: checkRow.toggled(!checkRow.checked)
    }
}
