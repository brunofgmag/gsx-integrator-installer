import QtQuick

Rectangle {
    id: banner

    required property var controller

    property bool dismissed: false

    visible: banner.controller.state === "ready"
             && banner.controller.installerUpdateAvailable && !banner.dismissed
    implicitHeight: bannerColumn.implicitHeight + 28
    color: Theme.panel2
    border.color: Theme.accent
    border.width: 1
    radius: Theme.radius

    Column {
        id: bannerColumn
        x: 14
        y: 14
        width: parent.width - 28
        spacing: 10

        Row {
            width: parent.width
            spacing: 8

            Text {
                id: bannerMarker
                text: "↑"
                color: Theme.accent
                font.pixelSize: 14
                width: 14
            }
            Text {
                width: parent.width - bannerMarker.width - parent.spacing
                text: qsTr("New installer version available: %1 → %2")
                    .arg(banner.controller.installerVersion).arg(banner.controller.installerLatest)
                color: Theme.text
                font.pixelSize: 13
                wrapMode: Text.WordWrap
            }
        }

        Row {
            visible: banner.controller.installerUpdateState === "idle"
            spacing: 10

            ActionButton {
                label: qsTr("Update and restart")
                onClicked: banner.controller.selfUpdate()
            }
            LinkText {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Not now")
                onClicked: banner.dismissed = true
            }
        }

        Column {
            visible: banner.controller.installerUpdateState === "updating"
            width: parent.width
            spacing: 10

            ProgressTrack {
                width: parent.width
                value: banner.controller.installerUpdateProgress
            }
            Text {
                text: qsTr("Updating the installer…")
                color: Theme.muted
                font.pixelSize: 12
            }
        }

        Column {
            visible: banner.controller.installerUpdateState === "error"
            width: parent.width
            spacing: 10

            Text {
                width: parent.width
                text: banner.controller.installerUpdateError
                color: Theme.red
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }
            ActionButton {
                label: qsTr("Try again")
                primary: false
                onClicked: banner.controller.selfUpdate()
            }
        }
    }
}
