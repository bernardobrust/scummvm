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

int TestsEngine::parse_tilemap_file(Common::String s, int matrix[3][5]) {
	static const int rows = 3, columns = 5; // Just for testing

	const char *str = s.c_str();

	const char *p = str;
	char *end;
	int i, j;

	for (i = 0; i < rows; ++i) {
		for (j = 0; j < columns; ++j) {
			int64 value;

			while (Common::isSpace((byte)*p))
				++p;

			if (*p == '\0')
				return 0;

			errno = 0;
			value = strtol(p, &end, 10);

			if (p == end)
				return 0;

			matrix[i][j] = (int)value;
			p = end;
		}
	}

	while (Common::isSpace((unsigned char)*p))
		++p;

	return (*p == '\0');
}

Common::Error TestsEngine::run() {
	// Initiating the config file
	Common::File configFile;
	if (!configFile.open("tests.conf"))
		error("Unable to load config file \"tests.conf\"");

	// Initiating the tilemap
	Common::File level1_tilemap;
	if (!level1_tilemap.open("level1.dat"))
		error("Unable to load tilemap for level 1 (level1.dat)");

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

	Common::String s = level1_tilemap.readString();
	int tilemap[3][5];

	parse_tilemap_file(s, tilemap);

	// Simple event handling loop
	byte pal[256 * 3] = {0};
	Common::Event e;

	// black
	// red
	pal[1 * 3 + 0] = 255;
	// green
	pal[2 * 3 + 1] = 255;
	// blue
	pal[3 * 3 + 2] = 255;

	_screen->setPalette(pal, 0, 256);

	// Player
	int x{}, y{}, spd{4}, player_width = 10, player_height = 10, player_color{0};

	Graphics::FrameLimiter limiter(g_system, 60);
	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
			// Handle events
			switch (e.type) {
			case Common::EVENT_KEYDOWN:
				switch (e.kbd.keycode) {
				case Common::KEYCODE_UP:
					y -= spd;
					break;

				case Common::KEYCODE_DOWN:
					y += spd;
					break;

				case Common::KEYCODE_LEFT:
					x -= spd;
					break;

				case Common::KEYCODE_RIGHT:
					x += spd;
					break;

				case Common::KEYCODE_SPACE:
					player_color = (player_color + 1) % 4;
					break;

				default:
					break;
				}
				break;

			default:
				break;
			}
		}

		x = CLIP(x, 0, width - player_width);
		y = CLIP(y, 0, height - player_height);

		for (int row = 0; row < 3; ++row) {
			for (int col = 0; col < 5; ++col) {
				int type = tilemap[row][col];

				uint32 color = 0;

				switch (type) {
				case 1:
					color = 1;
					break;
				case 2:
					color = 2;
					break;
				case 3:
					color = 3;
					break;
				}

				Common::Rect rect(
					col * 64,
					row * 67,
					(col + 1) * 64,
					(row + 1) * 67);

				if (row == 2)
					rect.bottom = height;

				_screen->fillRect(rect, color);
			}
		}

		Common::Rect rect(
			x,
			y,
			x + player_width,
			y + player_height);

		_screen->fillRect(rect, player_color);

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
