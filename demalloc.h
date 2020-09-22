#pragma once

typedef unsigned long SizeType;

struct Demalloc {
    void *memory;
    SizeType size;
};

void demallocInit(struct Demalloc *inst, void *memory, SizeType size);

void *demalloc(struct Demalloc *inst, SizeType size);
int defree(struct Demalloc *inst, void *pointer);
