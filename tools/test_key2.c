// Second keys (bind_key2_*), the mouse wheel's bindings and the "Keys: PC layout" preset, through the real bindings.c:
// a larecomp.ini round trip keeps them, clashing second keys are left out after the file is read, the preset never puts
// one key on two inputs, and "Restore default controls" clears the second keys.
// build (MSYS2 UCRT64, from the repository): clang -std=gnu11 -Iruntime -I$DEPS/include tools/test_key2.c -L$DEPS/lib -lSDL3
#include <SDL3/SDL.h>
#include "../runtime/hle/bindings.c"
#undef NDEBUG
#include <assert.h>
#include <stdio.h>

Settings g_settings;
bool g_settings_fresh;
void settings_save(void) {}
int input_pad_family(void) { return 0; }
int input_family_of_type(int type) { (void)type; return 1; }
int settings_on_steam_deck(void) { return 0; }
void rt_launcher_log(const char *fmt, ...) { (void)fmt; }

static void load_text(const char *text) {  // larecomp.ini lines through the loader, then the after-file check
    char buf[4096];
    snprintf(buf, sizeof buf, "%s", text);
    for (char *line = strtok(buf, "\n"); line; line = strtok(NULL, "\n")) {
        char *eq = strstr(line, " = ");
        if (!eq) continue;
        *eq = 0;
        bind_settings_load(line, eq + 3);
    }
    bind_settings_done();
}

static void reset_all(void) {
    bind_reset(BIND_DEV_KEY);
    bind_reset(BIND_DEV_PAD);
    bind_mouse_reset();
}

int main(void) {
    // 1. the PC layout: no key twice, recognised, and a reset clears it (second keys too)
    reset_all();
    bind_key_pc_preset();
    assert(bind_key_is_pc());
    for (int i = 0; i < BIND_KEY_COUNT; i++)
        for (int j = 0; j < BIND_KEY_COUNT; j++) {
            if (g_bind_key[i]) assert(i == j || g_bind_key[i] != g_bind_key[j]);
            if (g_bind_key2[i]) assert(g_bind_key2[i] != g_bind_key[j] && (i == j || g_bind_key2[i] != g_bind_key2[j]));
        }
    assert(g_bind_key[BIND_A] == SDL_SCANCODE_F && g_bind_key2[BIND_A] == SDL_SCANCODE_RETURN);
    assert(g_bind_mouse[MOUSEB_WHEEL_DOWN] == BIND_RIGHT && g_bind_mouse[MOUSEB_WHEEL_UP] == BIND_LEFT);
    assert(!bind_is_default(BIND_DEV_KEY));
    bind_reset(BIND_DEV_KEY);
    assert(bind_is_default(BIND_DEV_KEY) && !bind_key_is_pc());
    for (int i = 0; i < BIND_KEY_COUNT; i++) assert(!g_bind_key2[i]);

    // 2. a round trip through the file keeps the second keys and the wheel
    bind_mouse_reset();
    bind_key_pc_preset();
    char out[4096] = "";
    FILE *f = tmpfile();
    assert(f);
    bind_settings_save(f);
    rewind(f);
    size_t n = fread(out, 1, sizeof out - 1, f);
    out[n] = 0;
    fclose(f);
    assert(strstr(out, "bind_key2_a = Return\n") && strstr(out, "bind_key2_b = Backspace\n"));
    assert(strstr(out, "bind_mouse_wheel_up = left\n") && strstr(out, "bind_mouse_wheel_down = right\n"));
    reset_all();
    load_text(out);
    assert(bind_key_is_pc());

    // 3. a hand-edited file: a second key that is another input's key, the same key twice, or one the overlay keeps
    reset_all();
    load_text("bind_key_a = F\n"
              "bind_key2_b = F\n"          // (A's key)
              "bind_key2_x = Return\n"     // (Plus's default key)
              "bind_key2_y = Escape\n"     // (the overlay's)
              "bind_key2_l = H\n"
              "bind_key2_r = H\n"          // (L's second key already)
              "bind_key2_zl = nonsense\n"  // (no such key: ignored)
              "bind_mouse_wheel_up = zl\n"
              "bind_mouse_wheel_down = ls_up\n");  // (a stick direction: buttons only)
    assert(!g_bind_key2[BIND_B] && !g_bind_key2[BIND_X] && !g_bind_key2[BIND_Y] && !g_bind_key2[BIND_ZL]);
    assert(g_bind_key2[BIND_L] == SDL_SCANCODE_H && !g_bind_key2[BIND_R]);
    assert(g_bind_mouse[MOUSEB_WHEEL_UP] == BIND_ZL && g_bind_mouse[MOUSEB_WHEEL_DOWN] == -1);

    // 4. capturing a key in Configure inputs takes it off any second key
    reset_all();
    bind_key_pc_preset();
    assign(BIND_DEV_KEY, BIND_MINUS, SDL_SCANCODE_RETURN);  // (A's second key)
    assert(g_bind_key[BIND_MINUS] == SDL_SCANCODE_RETURN && !g_bind_key2[BIND_A]);

    puts("PASS");
    return 0;
}
