#include "sound.h"
#include "../emucore/emucore.h"

#define SND_DEBUG_CH1 1
#define SND_DEBUG_CH2 (1 << 1)
#define SND_DEBUG_CH3 (1 << 2)
#define SND_DEBUG_CH4 (1 << 3)
#define SND_DEBUG_CONTROLLER (1 << 4)

#define SND_DEBUG_OPEN (0)

#if DEBUG
#define snd_debug_assert(expr, msg) do { if (!(expr)) panic("debug_assert:%s, failing expr: %s in %s (%s:%d)", msg, #expr, __func__, __FILE__, __LINE__); } while(0)
#define snd_debug(bit, s, ...) do { if (SND_DEBUG_OPEN & (bit)) fprintf(stderr, "(debug) in %s (%s:%d):" s "\n", __func__, __FILE__, __LINE__, ##__VA_ARGS__); } while(0)
#else
#define snd_debug_assert(expr, msg) /**/
#define snd_debug(bit, s, ...) /**/
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define SOUND_CHANNEL_ENABLE(ctlr, chid) ((ctlr)->status_reg |= (1 << ((chid) - 1)))
#define SOUND_CHANNEL_DISABLE(ctlr, chid) ((ctlr)->status_reg &= ~(1 << ((chid) - 1)))
#define SOUND_CHANNEL_IS_ON(ctlr, chid) ((ctlr)->status_reg & (1 << ((chid) - 1)))

#define SOUND_VE_GETVOL(stt) ((stt)->ve_cur_volume / 15.0)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
static int sound_timer_tick(sound_timer_t *timer, int ncycs) {
    timer->tick -= ncycs;

    if (timer->tick <= 0) {
        timer->tick = timer->timebase + timer->tick;
        return 1;
    }

    return 0;
}

static void sound_timer_set_base(sound_timer_t *timer, int timebase) {
    timer->timebase = timebase;
}

static void sound_timer_reset(sound_timer_t *timer) {
    timer->tick = timer->timebase;
}
#pragma clang diagnostic pop

void sound_square_install_regs(sound_ctlr_t *state, void *target, snd_mmio_add_observer_t reg_func);
void sound_square_onsweep(sound_ctlr_t *state);

void sound_square_onclock_ch1(sound_ctlr_t *state, int ncyc);
int sound_square_getoutput_ch1(sound_ctlr_t *state);
void sound_square_onclock_ch2(sound_ctlr_t *state, int ncyc);
int sound_square_getoutput_ch2(sound_ctlr_t *state);

void sound_wave_install_regs(sound_ctlr_t *state, void *target, snd_mmio_add_observer_t reg_func);
void sound_wave_onclock(sound_ctlr_t *state, int ncyc);

void sound_noise_install_regs(sound_ctlr_t *state, void *target, snd_mmio_add_observer_t reg_func);
void sound_noise_onclock(sound_ctlr_t *state, int ncyc);

void sound_ve_configure(sound_ve_t *env, uint8_t regval);
void sound_ve_oninit(sound_ve_t *env);
void sound_ve_onclock(sound_ve_t *env);

#ifdef __cplusplus
}
#endif
