#pragma once


namespace kerf::accordion {

// Pure layout math for DropAccordion. No scene or GL dependency.
//
// Positions are along the accordion's axis, relative to its center, in the same
// units as length. The layout is CSS "space-around": count items over length
// each get an equal share (length / count) and sit centered in it.

// Center of slot index (of count) along the axis.
float slotOffset(int index, int count, float length);

// Which of count slots contains cursor, clamped to [0, count - 1].
int slotAt(float cursor, int count, float length);

} // namespace kerf::accordion
