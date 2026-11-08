/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "tests/tests.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/framelimiter.h"
#include "graphics/paletteman.h"
#include "tests/console.h"
#include "tests/detection.h"

namespace Tests {

TestsEngine *g_engine;

TestsEngine::TestsEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																			 _gameDescription(gameDesc), _randomSource("Tests") {
	g_engine = this;
}

TestsEngine::~TestsEngine() {
	delete _screen;
}

uint32 TestsEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String TestsEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error TestsEngine::run() {
	// Initiating the config file
	Common::File configFile;
	if (!configFile.open("tests.conf"))
		error("Unable to load config file \"tests.conf\"");

	Common::String dimsRaw = configFile.readLine();

	size_t xPos = dimsRaw.findFirstOf('x');
	// implicit conversion from Uint64 to int
	int width, height;
	if (xPos != Common::String::npos) {
		width = dimsRaw.substr(0, xPos).asUint64();
		height = dimsRaw.substr(xPos + 1).asUint64();
	} else
		error("Unable to parse config file");

	// Initialize 320x200 paletted graphics mode
	initGraphics(width, height);
	_screen = new Graphics::Screen();

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	// Draw a series of boxes on screen as a sample
	for (int i = 0; i < 100; ++i)
		_screen->frameRect(Common::Rect(i, i, width - i, height - i), i);
	_screen->update();

	// Simple event handling loop
	byte pal[256 * 3] = {0};
	Common::Event e;
	int offset = 0;

	Graphics::FrameLimiter limiter(g_system, 60);
	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
		}

		// Cycle through a simple palette
		++offset;
		for (int i = 0; i < 256; ++i)
			pal[i * 3 + 1] = (i + offset) % 256;
		g_system->getPaletteManager()->setPalette(pal, 0, 256);
		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		limiter.delayBeforeSwap();
		_screen->update();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error TestsEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace Tests
