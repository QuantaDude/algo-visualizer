#ifndef R_CELLS
#define R_CELLS 60

#endif // !R_CELLS
#ifndef C_CELLS
#define C_CELLS 80
#endif // !C_CELLS#define R_CELLS 160

struct IVector2 {
  int x, y;
};

static const char *lastKey;
const char *getKeyName();
void initCells(bool *);
