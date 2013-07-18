# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
#
# Unity8 Autopilot Test Suite
# Copyright (C) 2012-2013 Canonical
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

from __future__ import absolute_import

from collections import namedtuple

from unity8.shell import with_lightdm_mock
from unity8.shell.tests import Unity8TestCase, _get_device_emulation_scenarios

from testtools.matchers import Equals
from autopilot.matchers import Eventually


SwipeCoords = namedtuple('SwipeCoords', 'start_x end_x start_y end_y')


class TestHud(Unity8TestCase):

    """Tests the Shell HUD."""

    scenarios = _get_device_emulation_scenarios()

    @with_lightdm_mock("single")
    def test_show_hud_button_appears(self):
        """Swiping up while an app is active must show the 'show hud' button."""
        self.launch_unity()
        self.main_window.get_greeter().unlock()
        window = self.main_window.get_qml_view()
        hud_show_button = self.main_window.get_hud_show_button()

        self._launch_test_app_from_app_screen()

        swipe_coords = self._get_hud_button_swipe_coords(
            window,
            hud_show_button
        )
        self.touch.press(swipe_coords.start_x, swipe_coords.start_y)
        self.addCleanup(self.touch.release)
        self.touch._finger_move(swipe_coords.end_x, swipe_coords.end_y)
        self.assertThat(hud_show_button.opacity, Eventually(Equals(1.0)))

    @with_lightdm_mock("single")
    def test_show_hud_appears(self):
        """Releasing the touch on the 'show hud' button must display the hud."""
        self.launch_unity()
        self.main_window.get_greeter().unlock()
        window = self.main_window.get_qml_view()
        hud_show_button = self.main_window.get_hud_show_button()
        hud = self.main_window.get_hud()

        self._launch_test_app_from_app_screen()

        swipe_coords = self._get_hud_button_swipe_coords(
            window,
            hud_show_button
        )
        self.touch.press(swipe_coords.start_x, swipe_coords.start_y)
        self.addCleanup(self._maybe_release_finger)
        self.touch._finger_move(swipe_coords.end_x, swipe_coords.end_y)

        self.assertThat(hud.shown, Eventually(Equals(False)))
        self.assertThat(hud_show_button.opacity, Eventually(Equals(1.0)))
        self.touch.release()
        self.assertThat(hud.shown, Eventually(Equals(True)))

    @with_lightdm_mock("single")
    def test_hide_hud_click(self):
        """Tapping the close button of the Hud must dismiss it."""
        self.launch_unity()
        self.main_window.get_greeter().unlock()
        hud = self.main_window.get_hud()

        self._launch_test_app_from_app_screen()
        hud.show()

        x, y = hud.get_close_button_coords()
        self.touch.tap(x, y)
        self.assertThat(hud.shown, Eventually(Equals(False)))

    @with_lightdm_mock("single")
    def test_hide_hud_dragging(self):
        """Once open the Hud must close if the upper bar is dragged and released
        downward.

        """
        self.launch_unity()
        self.main_window.get_greeter().unlock()
        hud = self.main_window.get_hud()
        window = self.main_window.get_qml_view()

        self._launch_test_app_from_app_screen()
        hud.show()

        start_x, start_y = hud.get_close_button_coords()
        end_x = start_x
        end_y = int(window.height / 2)

        self.touch.drag(start_x, start_y, end_x, end_y)
        self.assertThat(hud.shown, Eventually(Equals(False)))

    @with_lightdm_mock("single")
    def test_launcher_hides_hud(self):
        """Opening the Launcher while the Hud is active must close the Hud."""
        self.launch_unity()
        self.main_window.get_greeter().unlock()
        hud = self.main_window.get_hud()
        launcher = self.main_window.get_launcher()

        self._launch_test_app_from_app_screen()

        hud.show()
        launcher.show()

        self.assertThat(hud.shown, Eventually(Equals(False)))

    def _launch_test_app_from_app_screen(self):
        """Launches the camera app using the Dash UI.

        Because when testing on the desktop running
        self.launch_application() will launch the application on the desktop
        itself and not within the Unity8 UI.

        """
        dash = self.main_window.get_dash()
        icon = dash.get_application_icon('Camera')
        self.touch.tap_object(icon)

        # Ensure application is open
        bottombar = self.main_window.get_bottombar()
        self.assertThat(bottombar.applicationIsOnForeground,
                        Eventually(Equals(True)))

    # Because some tests are manually manipulating the finger, we want to
    # cleanup if the test fails, but we don't want to fail with an exception if
    # we don't.
    def _maybe_release_finger(self):
        """Only release the finger if it is in fact down."""
        if self.touch._touch_finger is not None:
            self.touch.release()

    # TODO: perhaps move this to the Hud emulator?
    def _get_hud_button_swipe_coords(self, main_view, hud_show_button):
        """Returns the coords both start and end x,y for swiping to make the
        'hud show' button appear.

        """
        start_x = int(main_view.x + (main_view.width / 2))
        end_x = start_x
        start_y = main_view.y + (main_view.height - 3)
        end_y = int(hud_show_button.y + (hud_show_button.height/2))

        return SwipeCoords(start_x, end_x, start_y, end_y)
