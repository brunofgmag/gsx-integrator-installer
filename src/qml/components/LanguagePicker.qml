import QtQuick
import QtQuick.Controls

Rectangle {
    id: picker

    property string current: "system"
    signal picked(string value)

    readonly property var options: [
        { value: "system", short: qsTr("Auto"), name: qsTr("System language") },
        { value: "en", short: "EN", name: "English" },
        { value: "pt_BR", short: "PT", name: "Português (Brasil)" }
    ]
    readonly property var currentOption: options.find(o => o.value === current) ?? options[0]

    function select(value) {
        picked(value)
        menu.close()
    }

    implicitWidth: buttonRow.implicitWidth + 20
    implicitHeight: 28
    radius: Theme.radius
    color: buttonArea.containsMouse || menu.visible ? Theme.panel : "transparent"
    border.color: picker.activeFocus ? Theme.accent : Theme.line
    border.width: 1

    activeFocusOnTab: true
    Accessible.role: Accessible.ComboBox
    Accessible.name: qsTr("Language")
    Keys.onSpacePressed: menu.visible ? menu.close() : menu.open()
    Keys.onReturnPressed: menu.visible ? menu.close() : menu.open()
    Keys.onDownPressed: menu.open()

    Row {
        id: buttonRow
        anchors.centerIn: parent
        spacing: 6

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: ""
            font.family: "Segoe MDL2 Assets"
            font.pixelSize: 13
            color: Theme.muted
        }
        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: picker.currentOption.short
            font.pixelSize: 12
            color: Theme.text
        }
        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: ""
            font.family: "Segoe MDL2 Assets"
            font.pixelSize: 9
            color: Theme.muted
            rotation: menu.visible ? 180 : 0
            Behavior on rotation { NumberAnimation { duration: 150; easing.type: Easing.OutQuart } }
        }
    }

    MouseArea {
        id: buttonArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: menu.visible ? menu.close() : menu.open()
    }

    Popup {
        id: menu

        x: picker.width - width
        y: picker.height + 6
        width: Math.max(190, implicitWidth)
        padding: 4
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

        background: Rectangle {
            color: Theme.panel
            border.color: Theme.line
            border.width: 1
            radius: Theme.radius
        }

        contentItem: Column {
            Repeater {
                model: picker.options

                Rectangle {
                    id: entry

                    required property var modelData
                    readonly property bool selected: picker.current === modelData.value

                    width: parent.width
                    implicitWidth: entryText.implicitWidth + 56
                    implicitHeight: 30
                    radius: Theme.radiusSmall
                    color: entryArea.containsMouse ? Theme.panel2 : "transparent"

                    Accessible.role: Accessible.MenuItem
                    Accessible.name: entry.modelData.name

                    Text {
                        id: entryText
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        text: entry.modelData.name
                        font.pixelSize: 12
                        color: entry.selected ? Theme.text : Theme.muted
                    }
                    Text {
                        anchors.right: parent.right
                        anchors.rightMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        visible: entry.selected
                        text: ""
                        font.family: "Segoe MDL2 Assets"
                        font.pixelSize: 11
                        color: Theme.accent
                    }
                    MouseArea {
                        id: entryArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: picker.select(entry.modelData.value)
                    }
                }
            }
        }
    }
}
