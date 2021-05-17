# Build using emscripten
make PLATFORM=PLATFORM_WEB -B PROJECT_NAME=evasion evasion

# Move build artifacts to public
mkdir -p public/game
mv build/evasion* public/game
mv public/game/evasion.html public/game/index.html

# Prepare manifest.json
cp src/manifest.json public/game/manifest.json

# npx serve -s public/game
