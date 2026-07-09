import QtQuick

Rectangle {
    id: track

    property double value: -1

    height: 6
    radius: 3
    color: Theme.line

    onValueChanged: if (track.value >= 0) fill.x = 0

    Rectangle {
        id: fill
        height: parent.height
        radius: 3
        color: Theme.accent
        width: track.value >= 0 ? Math.max(6, track.width * track.value) : track.width * 0.3

        SequentialAnimation on x {
            running: track.value < 0
            loops: Animation.Infinite
            NumberAnimation { from: 0; to: track.width * 0.7; duration: 900; easing.type: Easing.InOutQuad }
            NumberAnimation { from: track.width * 0.7; to: 0; duration: 900; easing.type: Easing.InOutQuad }
        }
    }
}
