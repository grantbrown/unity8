/*
 * Copyright (C) 2016 Canonical, Ltd.
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

#include "MockController.h"

static QLightDM::MockController *m_instance = nullptr;

namespace QLightDM
{

MockController::MockController(QObject *parent)
    : QObject(parent)
    , m_fullSessions(
        {
            {"ubuntu", "Ubuntu"},
            {"ubuntu-2d", "Ubuntu 2D"},
            {"gnome", "GNOME"},
            {"gnome-classic", "GNOME Classic"},
            {"gnome-flashback-compiz", "GNOME Flashback (Compiz)"},
            {"gnome-flashback-metacity", "GNOME Flashback (Metacity)"},
            {"gnome-wayland", "GNOME on Wayland"},
            {"plasma", "Plasma"},
            {"kde", "KDE" },
            {"xterm", "Recovery Console"},
            {"", "Unknown?"}
        })
{
    m_userMode = qgetenv("LIBLIGHTDM_MOCK_MODE");
    if (m_userMode.isEmpty()) {
        m_userMode = "full";
    }
    m_sessionMode = "full";
    m_numSessions = numFullSessions();
}

MockController::~MockController()
{
    m_instance = nullptr;
}

MockController *MockController::instance()
{
    if (!m_instance) {
        m_instance = new MockController;
    }
    return m_instance;
}

QString MockController::selectUserHint() const
{
    return m_selectUserHint;
}

void MockController::setSelectUserHint(const QString &selectUserHint)
{
    if (m_selectUserHint != selectUserHint) {
        m_selectUserHint = selectUserHint;
        Q_EMIT selectUserHintChanged();
    }
}

QString MockController::userMode() const
{
    return m_userMode;
}

void MockController::setUserMode(const QString &userMode)
{
    if (m_userMode != userMode) {
        m_userMode = userMode;
        Q_EMIT userModeChanged();
    }
}

QString MockController::sessionMode() const
{
    return m_sessionMode;
}

void MockController::setSessionMode(const QString &sessionMode)
{
    if (m_sessionMode != sessionMode) {
        m_sessionMode = sessionMode;
        Q_EMIT sessionModeChanged();
    }
}

QString MockController::sessionName() const
{
    return QString("INVALID - Please read directly from the UserModel");
}

void MockController::setSessionName(const QString &sessionName)
{
        // Let the user model deal with this
        Q_EMIT sessionNameChanged(sessionName, m_currentUsername);
}

QString MockController::currentUsername() const
{
    return m_currentUsername;
}

void MockController::setCurrentUsername(const QString &username)
{
    if (m_currentUsername != username) {
        m_currentUsername = username;
        Q_EMIT currentUsernameChanged();
    }
}

const QList<MockController::SessionItem> &MockController::fullSessionItems() const
{
    return m_fullSessions;
}

int MockController::numFullSessions() const
{
    return m_fullSessions.size();
}

int MockController::numSessions() const
{
    return m_numSessions;
}

void MockController::setNumSessions(int numSessions)
{
    if (m_numSessions != numSessions) {
        m_numSessions = numSessions;
        Q_EMIT numSessionsChanged();
    }
}

}
