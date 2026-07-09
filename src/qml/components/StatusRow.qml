import QtQuick

Row {
    id: statusRow

    property alias marker: markerText.text
    property alias markerColor: markerText.color
    property alias label: labelText.text
    property int contentWidth: 520

    spacing: 10

    Text {
        id: markerText
        width: 16
        color: Theme.ok
        font.pixelSize: 13
    }
    Text {
        id: labelText
        width: statusRow.contentWidth - 26
        color: Theme.text
        font.pixelSize: 13
        wrapMode: Text.WordWrap
    }
}
