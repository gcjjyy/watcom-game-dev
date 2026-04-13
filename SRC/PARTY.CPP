#include <string.h>
#include "PARTY.H"

static Party_Member _members[PARTY_SIZE];

static const Party_Member _defaults[PARTY_SIZE] = {
    /* name       hp  maxhp mp  maxmp atk def spd alive defending spr */
    { "Hero",     60, 60,  10, 10,   12, 8,  6,  1,    0,        0 },
    { "Mage",     35, 35,  30, 30,    6, 4,  7,  1,    0,        0 },
    { "Knight",   80, 80,   5,  5,   10, 14, 4,  1,    0,        0 },
    { "Cleric",   45, 45,  20, 20,    7, 6,  5,  1,    0,        0 }
};

void party_init(void) {
    memcpy(_members, _defaults, sizeof(_defaults));
}

Party_Member *party_get(int i) {
    if (i < 0 || i >= PARTY_SIZE) return 0;
    return &_members[i];
}

int party_alive_count(void) {
    int i, n = 0;
    for (i = 0; i < PARTY_SIZE; i++) {
        if (_members[i].alive && _members[i].hp > 0) n++;
    }
    return n;
}

void party_full_heal(void) {
    int i;
    for (i = 0; i < PARTY_SIZE; i++) {
        _members[i].hp = _members[i].max_hp;
        _members[i].mp = _members[i].max_mp;
        _members[i].alive = 1;
        _members[i].defending = 0;
    }
}

void party_clear_defend_flags(void) {
    int i;
    for (i = 0; i < PARTY_SIZE; i++) {
        _members[i].defending = 0;
    }
}
