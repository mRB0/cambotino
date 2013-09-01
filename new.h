#ifndef NEW_H_
#define NEW_H_
// Placement new is missing from the Arduino tools.
void * operator new (size_t size, void * ptr) { return ptr; }
#endif

