#ifndef SCRNCAP_H_INCLUDED
#define SCRNCAP_H_INCLUDED

/* Save the current back-buffer (gfx_buffer) as an 8-bit indexed BMP
 * file under the given name. Uses the engine's game_palette for the
 * color table. Returns 0 on success, -1 on failure. */
int scrncap_save_bmp(const char *path);

#endif
