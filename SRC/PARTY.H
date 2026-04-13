#ifndef PARTY_H_INCLUDED
#define PARTY_H_INCLUDED

#define PARTY_SIZE 4

typedef struct {
    char name[9];
    int hp, max_hp;
    int mp, max_mp;
    int atk, def, spd;
    int alive;
    int defending;   /* set by Defend, cleared at start of next actor's turn */
    int spr_idx;
} Party_Member;

void          party_init(void);
Party_Member *party_get(int i);
int           party_alive_count(void);
void          party_full_heal(void);
void          party_clear_defend_flags(void);

#endif
