import argparse

from PyQt5.QtCore import QThread, Qt, QFileSystemWatcher, QDirIterator
from PyQt5.QtWidgets import QMainWindow, QDockWidget, QTabWidget

from cetech.playground.logwidget import LogWidget
from cetech.playground.scripteditor import ScriptEditor
from cetech.shared.qtapi import QtConsoleAPI
from cetech.playground.assetbrowser import AssetBrowser
from cetech.playground.cetechproject import CetechProject
from cetech.playground.ui.mainwindow import Ui_MainWindow


class MainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()
        self.setupUi(self)

        self.centralwidget.hide()

        self.parser = argparse.ArgumentParser("playground")
        self.parser.add_argument("-d", "--data-dir", type=str, help="data dir")
        self.parser.add_argument("-a", "--console-address", type=str, help="console address", default='localhost')
        self.parser.add_argument("-p", "--console-port", type=int, help="console port", default=2222)
        self.args = self.parser.parse_args()

        self.data_dir = self.args.data_dir
        self.console_port = self.args.console_port
        self.console_address = self.args.console_address

        self.project = CetechProject()

        self.api = QtConsoleAPI(self.console_address, self.console_port)

        self.setTabPosition(Qt.AllDockWidgetAreas, QTabWidget.North)

        self.log_dock_widget = QDockWidget(self)
        self.log_dock_widget.hide()
        self.log_dock_widget.setWindowTitle("Log")
        self.log_widget = LogWidget(self.api)
        self.log_dock_widget.setWidget(self.log_widget)
        self.addDockWidget(Qt.BottomDockWidgetArea, self.log_dock_widget)

        self.assetb_widget = AssetBrowser()
        self.assetb_dock_widget = QDockWidget(self)
        self.assetb_dock_widget.hide()
        self.assetb_dock_widget.setWindowTitle("Asset browser")
        self.assetb_dock_widget.setFeatures(QDockWidget.AllDockWidgetFeatures)
        self.assetb_dock_widget.setWidget(self.assetb_widget)
        self.addDockWidget(Qt.LeftDockWidgetArea, self.assetb_dock_widget)

        self.script_editor_widget = ScriptEditor(project_manager=self.project, api=self.api)
        self.script_editor_dock_widget = QDockWidget(self)
        self.script_editor_dock_widget.setWindowTitle("Script editor")
        self.script_editor_dock_widget.hide()
        self.script_editor_dock_widget.setFeatures(QDockWidget.AllDockWidgetFeatures)
        self.script_editor_dock_widget.setWidget(self.script_editor_widget)
        self.addDockWidget(Qt.TopDockWidgetArea, self.script_editor_dock_widget)

        self.tabifyDockWidget(self.assetb_dock_widget, self.log_dock_widget)

        self.assetb_widget.asset_clicked.connect(self.open_asset)

    def open_asset(self, path, ext):
        if self.script_editor_widget.support_ext(ext):
            self.script_editor_widget.open_file(path)
            self.script_editor_dock_widget.show()
            self.script_editor_dock_widget.focusWidget()

    def open_project(self, name, dir):
        self.project.open_project(name, dir)

        self.project.run_cetech(compile=True, continu=True, daemon=True)
        self.api.start(QThread.LowPriority)

        self.assetb_widget.open_project(self.project.project_dir)
        self.assetb_dock_widget.show()
        self.log_dock_widget.show()

        self.watch_project_dir();

    def watch_project_dir(self):
        files = []
        it = QDirIterator(self.project.source_dir, QDirIterator.Subdirectories)
        while it.hasNext():
            files.append(it.next())

        self.file_watch = QFileSystemWatcher(self)
        self.file_watch.addPaths(files)
        self.file_watch.fileChanged.connect(self.file_changed)
        self.file_watch.directoryChanged.connect(self.dir_changed)

    def file_changed(self, path):
        self.api.lua_execute("Application.compile_all()")

    def dir_changed(self, path):
        self.watch_project_dir()

    def open_script_editor(self):
        self.script_editor_dock_widget.show()

    def closeEvent(self, evnt):
        self.api.disconnect()

        self.project.killall_process()

        self.statusbar.showMessage("Disconnecting ...")

        while self.api.connect:
            self.api.tick()

        self.statusbar.showMessage("Disconnected")

        evnt.accept()
