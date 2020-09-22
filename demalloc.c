typedef unsigned long SizeType;

struct Demalloc {
    void *memory;
    SizeType size;
};

enum PageInfoType {
    Freed,
    Used
};

struct PageInfo {
    enum PageInfoType type;
    struct PageInfo *next;
    SizeType size;
};

void demallocInit(struct Demalloc *inst, void *memory, SizeType size) {
    inst->memory = memory;
    inst->size = size;

    struct PageInfo *page = inst->memory;
    page->type = Freed;
    page->size = size - sizeof(struct PageInfo);
    page->next = inst->memory + size;
}

void *demalloc(struct Demalloc *inst, SizeType size) {
    void *pointer = inst->memory;

    int foundUsed = 0;
    struct PageInfo *lastUsed = 0;

    while (pointer < (inst->memory + inst->size)) {
        struct PageInfo *page = pointer;

        if (page->type == Used) {
            pointer = page->next;

            foundUsed = 1;
            lastUsed = page;

            continue;
        }

        if (page->type != Freed)
            return 0;

        while (page->size < size) {
            struct PageInfo *next = pointer + page->size + sizeof(struct PageInfo);

            if (page->type == Used)
                break;

            if (page->type != Freed)
                return 0;

            page->size += sizeof(struct PageInfo) + next->size;
        }

        if (page->size >= size)
            break;

        pointer += sizeof(struct PageInfo) + page->size;
    }

    struct PageInfo *selected = pointer;

    if (selected->size < size)
        return 0;

    if (selected->size > (size + sizeof(struct PageInfo))) {
        struct PageInfo *next = pointer + sizeof(struct PageInfo) + size;
        next->type = Freed;
        next->size = selected->size - size - sizeof(struct PageInfo);

        if (foundUsed)
            next->next = lastUsed;
        else
            next->next = inst->memory;

        selected->next = next;
        selected->size = size;
    } else {
        struct PageInfo *next = pointer + sizeof(struct PageInfo) + selected->size;

        if (next->type == Freed)
            selected->next = next;
        else
            selected->next = next->next;
    }

    if (foundUsed)
        lastUsed->next = selected->next;

    selected->type = Used;

    return pointer + sizeof(struct PageInfo);
}

int defree(struct Demalloc *inst, void *pointer) {
    struct PageInfo *page = pointer - sizeof(struct PageInfo);

    page->type = Freed;

    void *current = (void *)page + sizeof(struct PageInfo) + page->size;
    struct PageInfo *followingBlock = current;

    while (current < (inst->memory + inst->size)) {
        struct PageInfo *next = current;

        if (next->type == Freed)
            break;

        current += sizeof(struct PageInfo) + next->size;
    }

    // Allocated all of memory.
    if (current >= (inst->memory + inst->size))
        return 0;

    struct PageInfo *nextFreed = current;
    struct PageInfo *lastUsed = nextFreed->next;

    lastUsed->next = page;

    if (followingBlock != nextFreed) {
        nextFreed->next = followingBlock;
        followingBlock->next = nextFreed;
    }

    page->next = lastUsed;

    return 1;
}
