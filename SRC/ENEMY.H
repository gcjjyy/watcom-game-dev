#ifndef ENEMY_H_INCLUDED
#define ENEMY_H_INCLUDED

/* Static enemy definitions for front-view command battles. */

#define ENEMY_DEF_COUNT      2
#define MAX_ENEMIES_PER_GRP  4
#define ENCOUNTER_GROUP_COUNT 3

typedef struct {
    const char *name;
    int hp;
    int mp;
    int atk;
    int def;
    int spd;
    int spr_idx;       /* unit sprite slot (2 = SPR_ENM1) */
    int exp_reward;
    int gold_reward;
} EnemyDef;

typedef struct {
    int count;
    int enemy_ids[MAX_ENEMIES_PER_GRP];
} EncounterGroup;

static const EnemyDef enemy_defs[ENEMY_DEF_COUNT] = {
    /* name      hp  mp  atk def spd spr  exp gold */
    { "Slime",   18,  0,  6,  2,  3,  2,   4,  3 },
    { "Goblin",  30,  0,  9,  4,  5,  2,   8,  7 }
};

static const EncounterGroup encounter_groups[ENCOUNTER_GROUP_COUNT] = {
    /* Group 0: 1x Slime */
    { 1, { 0, 0, 0, 0 } },
    /* Group 1: 2x Slime */
    { 2, { 0, 0, 0, 0 } },
    /* Group 2: 1x Goblin */
    { 1, { 1, 0, 0, 0 } }
};

#endif
