/*
 * Copyright (C) 2012-2015 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Qt
#include <QCommandLineParser>
#include <QtQuick/QQuickView>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QLibrary>
#include <QDebug>
#include <csignal>
#include <libintl.h>

// local
#include <paths.h>
#include "MouseTouchAdaptor.h"
#include "ApplicationArguments.h"
#include "CachingNetworkManagerFactory.h"

// Ubuntu Gestures
#include <TouchRegistry.h>

int main(int argc, const char *argv[])
{
    bool isMirServer = false;
    if (qgetenv("QT_QPA_PLATFORM") == "ubuntumirclient") {
        setenv("QT_QPA_PLATFORM", "mirserver", 1 /* overwrite */);
        isMirServer = true;
    }

    QGuiApplication::setApplicationName("unity8");
    QGuiApplication *application;

    QCommandLineParser parser;
    parser.setApplicationDescription("Description: Unity 8 Shell");
    parser.addHelpOption();

    QCommandLineOption fullscreenOption("fullscreen",
        "Run in fullscreen");
    parser.addOption(fullscreenOption);

    QCommandLineOption framelessOption("frameless",
        "Run without window borders");
    parser.addOption(framelessOption);

    QCommandLineOption mousetouchOption("mousetouch",
        "Allow the mouse to provide touch input");
    parser.addOption(mousetouchOption);

    QCommandLineOption windowGeometryOption(QStringList() << "windowgeometry",
            "Specify the window geometry as [<width>x<height>]", "windowgeometry", "1");
    parser.addOption(windowGeometryOption);

    QCommandLineOption testabilityOption("testability",
        "DISCOURAGED: Please set QT_LOAD_TESTABILITY instead. \n \
Load the testability driver");
    parser.addOption(testabilityOption);

    QCommandLineOption modeOption("mode",
        "Whether to run greeter and/or shell [full-greeter, full-shell, greeter, shell]",
        "mode", "full-greeter");
    parser.addOption(modeOption);

    application = new QGuiApplication(argc, (char**)argv);

    // Treat args with single dashes the same as arguments with two dashes
    // Ex: -fullscreen == --fullscreen
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.process(*application);

    QString indicatorProfile = qgetenv("UNITY_INDICATOR_PROFILE");
    if (indicatorProfile.isEmpty()) {
        indicatorProfile = "phone";
    }

    ApplicationArguments qmlArgs;
    if (parser.isSet(windowGeometryOption) &&
        parser.value(windowGeometryOption).split('x').size() == 2)
    {
        QStringList geom = parser.value(windowGeometryOption).split('x');
        qmlArgs.setSize(geom.at(0).toInt(), geom.at(1).toInt());
    }

    // If an invalid option was specified, set it to the default
    // If no default was provided in the QCommandLineOption constructor, abort.
    QString shellMode;
    if(!parser.isSet(modeOption) ||
        (parser.value(modeOption) != "full-greeter" &&
         parser.value(modeOption) != "full-shell" &&
         parser.value(modeOption) != "greeter" &&
         parser.value(modeOption) != "shell"))
    {
        if (modeOption.defaultValues().first() != nullptr) {
            shellMode = modeOption.defaultValues().first();
            qWarning() << "Mode argument was not provided or was set to an illegal value. Using default value of --mode=" << shellMode;
        } else {
            qFatal("Shell mode argument was not provided and there is no default mode,");
        }

    } else {
        shellMode = parser.value(modeOption);
    }

    // The testability driver is only loaded by QApplication but not by QGuiApplication.
    // However, QApplication depends on QWidget which would add some unneeded overhead => Let's load the testability driver on our own.
    if (parser.isSet(testabilityOption) || getenv("QT_LOAD_TESTABILITY")) {
        QLibrary testLib(QLatin1String("qttestability"));
        if (testLib.load()) {
            typedef void (*TasInitialize)(void);
            TasInitialize initFunction = (TasInitialize)testLib.resolve("qt_testability_init");
            if (initFunction) {
                initFunction();
            } else {
                qCritical("Library qttestability resolve failed!");
            }
        } else {
            qCritical("Library qttestability load failed!");
        }
    }

    bindtextdomain("unity8", translationDirectory().toUtf8().data());
    textdomain("unity8");

    QQuickView* view = new QQuickView();
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->setColor("black");
    view->setTitle("Unity8 Shell");
    view->engine()->setBaseUrl(QUrl::fromLocalFile(::qmlDirectory()));
    view->rootContext()->setContextProperty("applicationArguments", &qmlArgs);
    view->rootContext()->setContextProperty("indicatorProfile", indicatorProfile);
    view->rootContext()->setContextProperty("shellMode", shellMode);
    if (parser.isSet(framelessOption)) {
        view->setFlags(Qt::FramelessWindowHint);
    }
    TouchRegistry touchRegistry;
    view->installEventFilter(&touchRegistry);

    // You will need this if you want to interact with touch-only components using a mouse
    // Needed only when manually testing on a desktop.
    MouseTouchAdaptor *mouseTouchAdaptor = 0;
    if (parser.isSet(mousetouchOption)) {
        mouseTouchAdaptor = MouseTouchAdaptor::instance();
    }

    QUrl source(::qmlDirectory()+"Shell.qml");
    prependImportPaths(view->engine(), ::overrideImportPaths());
    if (!isMirServer) {
        prependImportPaths(view->engine(), ::nonMirImportPaths());
    }
    appendImportPaths(view->engine(), ::fallbackImportPaths());

    CachingNetworkManagerFactory *managerFactory = new CachingNetworkManagerFactory();
    view->engine()->setNetworkAccessManagerFactory(managerFactory);

    view->setSource(source);
    QObject::connect(view->engine(), SIGNAL(quit()), application, SLOT(quit()));

    if (isMirServer || parser.isSet(fullscreenOption)) {
        view->showFullScreen();
    } else {
        view->show();
    }

    int result = application->exec();

    delete view;
    delete mouseTouchAdaptor;
    delete application;

    return result;
}
