import QtQuick

Text {
    id: link

    signal clicked()

    color: Theme.muted
    font.pixelSize: 12
    font.underline: true

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: link.clicked()
    }
}
