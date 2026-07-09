import QtQuick
import QtQuick.Window

Window {
    id: root

    required property var controller

    readonly property int contentWidth: 520

    visible: true
    title: qsTr("GSX Integrator Installer")
    color: Theme.bg

    onClosing: close => close.accepted = root.controller.state !== "installing"
                                         && root.controller.installerUpdateState !== "updating"

    readonly property int targetHeight: Math.max(420, content.implicitHeight + 64)

    width: contentWidth + 48
    minimumWidth: width
    maximumWidth: width

    onTargetHeightChanged: applyFixedHeight()
    Component.onCompleted: applyFixedHeight()

    function applyFixedHeight() {
        minimumHeight = 0
        maximumHeight = 16777215
        height = targetHeight
        minimumHeight = targetHeight
        maximumHeight = targetHeight
    }

    Binding {
        target: Theme
        property: "dark"
        value: Application.styleHints.colorScheme !== Qt.Light
    }

    Column {
        id: content
        x: 24
        y: 32
        width: root.contentWidth
        spacing: 20

        Item {
            width: parent.width
            implicitHeight: headerLeft.implicitHeight

            Row {
                id: headerLeft
                spacing: 14
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter

                Image {
                    source: "qrc:/icons/app-icon.png"
                    width: 48
                    height: 48
                    anchors.verticalCenter: parent.verticalCenter
                }
                Column {
                    spacing: 2
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        text: qsTr("GSX Integrator")
                        color: Theme.text
                        font.pixelSize: 20
                    }
                    Text {
                        text: qsTr("Installer · v%1").arg(root.controller.installerVersion)
                        color: Theme.muted
                        font.pixelSize: 12
                    }
                }
            }

            LanguagePicker {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                current: root.controller.language
                onPicked: value => root.controller.language = value
            }
        }

        Rectangle { width: parent.width; height: 1; color: Theme.line }

        UpdateBanner {
            width: parent.width
            controller: root.controller
        }

        Text {
            visible: root.controller.state === "checking"
            text: qsTr("Checking simulators and latest versions…")
            color: Theme.muted
            font.pixelSize: 13

            SequentialAnimation on opacity {
                running: visible
                loops: Animation.Infinite
                NumberAnimation { to: 0.4; duration: 600 }
                NumberAnimation { to: 1.0; duration: 600 }
            }
        }

        Column {
            visible: root.controller.state === "ready" || root.controller.state === "installing"
                     || root.controller.state === "done"
            width: parent.width
            spacing: 10

            StatusRow {
                marker: root.controller.clientNeedsInstall ? "›" : "✓"
                markerColor: root.controller.clientNeedsInstall ? Theme.accent : Theme.ok
                label: {
                    const latest = root.controller.clientLatest;
                    const installed = root.controller.clientInstalled;
                    if (!root.controller.clientNeedsInstall)
                        return qsTr("GSX Integrator client: up to date (%1)").arg(installed);
                    if (installed.length > 0)
                        return qsTr("GSX Integrator client: %1 → %2").arg(installed).arg(latest);
                    return qsTr("GSX Integrator client: install %1").arg(latest);
                }
            }

            Repeater {
                model: root.controller.sims

                StatusRow {
                    marker: modelData.needsInstall ? (modelData.selected ? "›" : "·") : "✓"
                    markerColor: modelData.needsInstall
                                 ? (modelData.selected ? Theme.accent : Theme.faint) : Theme.ok
                    label: {
                        let text;
                        if (!modelData.needsInstall)
                            text = qsTr("CommBus in %1: up to date (%2)")
                                .arg(modelData.label).arg(modelData.installedVersion);
                        else if (!modelData.selected)
                            text = qsTr("CommBus in %1: skipped").arg(modelData.label);
                        else if (modelData.installedVersion.length > 0)
                            text = qsTr("CommBus in %1: %2 → %3").arg(modelData.label)
                                .arg(modelData.installedVersion).arg(root.controller.commbusLatest);
                        else
                            text = qsTr("CommBus in %1: install %2")
                                .arg(modelData.label).arg(root.controller.commbusLatest);
                        if (modelData.running)
                            text += qsTr(" (simulator running)");
                        return text;
                    }
                }
            }

            StatusRow {
                visible: root.controller.sims.length === 0
                marker: "!"
                markerColor: Theme.amber
                label: qsTr("No MSFS installation detected. The CommBus module will be skipped.")
            }

            StatusRow {
                visible: root.controller.clientRunning
                marker: "!"
                markerColor: Theme.amber
                label: qsTr("GSX Integrator is currently running.")
            }

            StatusRow {
                visible: root.controller.anySimRunning
                marker: "!"
                markerColor: Theme.amber
                label: qsTr("A simulator is running. Close it before installing the CommBus module.")
            }
        }

        Text {
            visible: root.controller.errorText.length > 0
            width: parent.width
            text: root.controller.errorText
            color: Theme.red
            font.pixelSize: 13
            wrapMode: Text.WordWrap
        }

        Column {
            visible: root.controller.state === "ready"
            width: parent.width
            spacing: 10

            Text {
                text: qsTr("Shortcuts")
                color: Theme.muted
                font.pixelSize: 12
            }
            CheckRow {
                label: qsTr("Create a desktop shortcut")
                checked: root.controller.desktopShortcut
                onToggled: checked => root.controller.desktopShortcut = checked
            }
            CheckRow {
                label: qsTr("Add the client to the Start Menu")
                checked: root.controller.clientStartMenuShortcut
                onToggled: checked => root.controller.clientStartMenuShortcut = checked
            }
            CheckRow {
                label: qsTr("Add this installer to the Start Menu")
                checked: root.controller.setupShortcut
                onToggled: checked => root.controller.setupShortcut = checked
            }
        }

        Column {
            visible: root.controller.state === "ready"
            width: parent.width
            spacing: 10

            Text {
                text: qsTr("Auto-start with the simulator")
                color: Theme.muted
                font.pixelSize: 12
            }
            Text {
                width: parent.width
                text: qsTr("Launch GSX Integrator automatically when the simulator starts")
                color: Theme.faint
                font.pixelSize: 11
                wrapMode: Text.WordWrap
            }
            Repeater {
                model: root.controller.autoStartOptions

                RadioRow {
                    label: modelData.label
                    checked: root.controller.autoStartMode === modelData.value
                    onSelected: root.controller.autoStartMode = modelData.value
                }
            }
        }

        Column {
            visible: root.controller.state === "ready"
            width: parent.width
            spacing: 12

            LinkText {
                text: (advanced.visible ? "▾ " : "▸ ") + qsTr("Advanced options")
                font.underline: false
                onClicked: advanced.visible = !advanced.visible
            }

            Column {
                id: advanced
                visible: false
                width: parent.width
                spacing: 10
                leftPadding: 16

                Repeater {
                    model: root.controller.sims

                    CheckRow {
                        label: qsTr("Install CommBus module in %1").arg(modelData.label)
                        checked: modelData.selected
                        onToggled: checked => root.controller.setSimSelected(index, checked)
                    }
                }
                CheckRow {
                    label: qsTr("Force reinstall")
                    checked: root.controller.forceReinstall
                    onToggled: checked => root.controller.forceReinstall = checked
                }
            }
        }

        ActionButton {
            visible: root.controller.state === "ready"
            label: root.controller.actionLabel
            enabled: root.controller.anythingToDo && root.controller.installerUpdateState !== "updating"
            onClicked: root.controller.install()
        }

        Column {
            id: uninstallArea
            property bool confirming: false
            visible: root.controller.state === "ready" && root.controller.installed
            width: parent.width
            spacing: 8

            LinkText {
                visible: !uninstallArea.confirming
                text: qsTr("Uninstall GSX Integrator")
                color: Theme.red
                onClicked: uninstallArea.confirming = true
            }
            Text {
                visible: !uninstallArea.confirming
                width: parent.width
                text: qsTr("Removes the client and the CommBus module from all simulators")
                color: Theme.faint
                font.pixelSize: 11
                wrapMode: Text.WordWrap
            }

            Text {
                visible: uninstallArea.confirming
                width: parent.width
                text: qsTr("Remove GSX Integrator, the CommBus module and the auto-start entries from all simulators?")
                color: Theme.text
                font.pixelSize: 13
                wrapMode: Text.WordWrap
            }
            Row {
                visible: uninstallArea.confirming
                spacing: 10

                ActionButton {
                    label: qsTr("Uninstall")
                    onClicked: root.controller.uninstall()
                }
                ActionButton {
                    label: qsTr("Cancel")
                    primary: false
                    onClicked: uninstallArea.confirming = false
                }
            }
        }

        Column {
            visible: root.controller.state === "installing"
            width: parent.width
            spacing: 10

            ProgressTrack {
                width: parent.width
                color: Theme.panel2
                value: root.controller.progressValue
            }
            Text {
                text: root.controller.progressText
                color: Theme.muted
                font.pixelSize: 12
            }
        }

        Column {
            visible: root.controller.state === "done"
            width: parent.width
            spacing: 16

            Text {
                text: qsTr("✓ All set.")
                color: Theme.ok
                font.pixelSize: 15
            }
            Row {
                spacing: 10

                ActionButton {
                    label: qsTr("Open GSX Integrator")
                    onClicked: { root.controller.openClient(); Qt.quit() }
                }
                ActionButton {
                    label: qsTr("Close")
                    primary: false
                    onClicked: Qt.quit()
                }
            }
        }

        Column {
            visible: root.controller.state === "uninstalled"
            width: parent.width
            spacing: 16

            Text {
                text: qsTr("✓ GSX Integrator was removed.")
                color: Theme.ok
                font.pixelSize: 15
            }
            ActionButton {
                label: qsTr("Close")
                primary: false
                onClicked: Qt.quit()
            }
        }

        ActionButton {
            visible: root.controller.state === "error"
            label: qsTr("Try again")
            primary: false
            onClicked: root.controller.refresh()
        }
    }
}
