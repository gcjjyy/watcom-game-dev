#ifndef SOUND_H_INCLUDED
#define SOUND_H_INCLUDED

#define SND_OK       0
#define SND_ERR     -1
#define SND_MAX_SIZE 65536UL

void snd_init(void);
void snd_close(void);

int  snd_detect(void);
int  snd_opl2_ready(void);  /* cached result of the last snd_detect() */
int  snd_load(const char *filename);

void snd_play(void);
void snd_stop(void);
int  snd_playing(void);

#endif
