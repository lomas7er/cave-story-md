/*
 * Number & Smoke effects, will be more than that eventually
 */

#define MAX_DAMAGE 4
#define MAX_SMOKE 8
#define MAX_MISC 8

enum { EFF_BONKL, EFF_BONKR, EFF_ZZZ, EFF_BOOST8, EFF_BOOST2 };

// Only send tiles for 1 damage string per frame
uint8_t dqueued;

// Initialize default (0, NULL) values to avoid weird glitches
void effects_init();
// Clear all effects
void effects_clear();
// Just clear the smoke
void effects_clear_smoke();
// Per frame update of effects
void effects_update();

// Creates a damage string (maximum of 4)
// Positive values will be white, negative will be red, zero is ignored
// Values must be limited to 3 digits
void effect_create_damage(int16_t num, int16_t x, int16_t y);
// Creates a single puff of smoke
void effect_create_smoke(int16_t x, int16_t y);

void effect_create_misc(uint8_t type, int16_t x, int16_t y);
