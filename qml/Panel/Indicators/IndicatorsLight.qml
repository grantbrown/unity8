/*
 * Copyright 2014 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *      Renato Araujo Oliveira Filho <renato@canonical.com>
 */

import QtQuick 2.0
import Powerd 0.1
import Lights 0.1

QtObject {
    id: root

    Component.onDestruction: Lights.state = Lights.Off

    // QtObject does not have children
    property var _connections: Connections {
        id: powerConnection

        target: Powerd
        onStatusChanged: {
            Lights.state = (Powerd.status === Powerd.Off) ? Lights.On : Lights.Off
        }
    }
}
