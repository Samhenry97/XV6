//cow.h: header for work with copy-on-write implementation for fork()

void cowUp(uint page);
void cowDown(uint page);
int getCowRefs(uint page);
void cowSet(uint page, uint value);