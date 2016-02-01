// Tool to monitor when the volume of an ALSA mixer is changed and to execute
// commands to change the volume of your hifi amplifier (e.g., using infrared
// commands).
//
// https://github.com/martignlo/alsa-amp-mixer

#include <stdio.h>
#include <stdlib.h>

#include <alsa/asoundlib.h>

#define CHECK_OK(expr)                                                  \
  {                                                                     \
    int status = (expr);                                                \
    if (status != 0) {                                                  \
      fprintf(stderr, "Error (%s): %s\n", #expr, snd_strerror(status)); \
      abort();                                                          \
    }                                                                   \
  }

#define CHECK_NOTNULL(expr)                   \
  {                                           \
    if (expr == NULL) {                       \
      fprintf(stderr, "Error (%s)\n", #expr); \
      abort();                                \
    }                                         \
  }

#ifdef __DEBUG__
#define DEBUG(...)              \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n")
#else
#define DEBUG(...)
#endif

void mixer(const char *card, const char *on_volume_up,
           const char *on_volume_down) {
  snd_mixer_selem_id_t *snd_sid;
  snd_mixer_selem_id_alloca(&snd_sid);

  snd_mixer_selem_id_set_index(snd_sid, 0);
  snd_mixer_selem_id_set_name(snd_sid, "Master");

  snd_mixer_t *snd_handle = NULL;
  snd_mixer_elem_t *snd_elem = NULL;
  CHECK_OK(snd_mixer_open(&snd_handle, 0));
  CHECK_OK(snd_mixer_attach(snd_handle, card));
  CHECK_OK(snd_mixer_selem_register(snd_handle, NULL, NULL));
  CHECK_OK(snd_mixer_load(snd_handle));

  snd_elem = snd_mixer_find_selem(snd_handle, snd_sid);
  CHECK_NOTNULL(snd_elem);

  long volume_max = 0;
  long volume_min = 0;
  CHECK_OK(snd_mixer_selem_get_playback_volume_range(snd_elem, &volume_min,
                                                     &volume_max));
  // This is the default volume (50%). After each change, the volume is
  // automatically reset to the default.
  long volume_mid = volume_min + (volume_max - volume_min) / 2;

  DEBUG("Volume range %ld %ld. Default: %ld", volume_min, volume_max,
        volume_mid);

  // Set the volume to a default value.
  CHECK_OK(snd_mixer_selem_set_playback_volume(
      snd_elem, SND_MIXER_SCHN_FRONT_LEFT, volume_mid));
  CHECK_OK(snd_mixer_selem_set_playback_volume(
      snd_elem, SND_MIXER_SCHN_FRONT_RIGHT, volume_mid));

  while (1) {
    // Wait until something happens.
    CHECK_OK(snd_mixer_wait(snd_handle, -1));

    // Clears all pending mixer events.
    snd_mixer_handle_events(snd_handle);

    // Read the current volume.
    long volume = 0;
    CHECK_OK(snd_mixer_selem_get_playback_volume(
        snd_elem, SND_MIXER_SCHN_FRONT_LEFT, &volume));

    // Ignore events that don't cause the new volume to differ from the
    // default volume.
    if (volume_mid != volume) {
      const char *cmd = volume > volume_mid ? on_volume_up : on_volume_down;
      DEBUG("Volume: %ld. Executing command: '%s'", volume, cmd);
      CHECK_OK(system(cmd));
    }

    // Reset to the default value.
    CHECK_OK(snd_mixer_selem_set_playback_volume(
        snd_elem, SND_MIXER_SCHN_FRONT_LEFT, volume_mid));
    CHECK_OK(snd_mixer_selem_set_playback_volume(
        snd_elem, SND_MIXER_SCHN_FRONT_RIGHT, volume_mid));
  }
}

int main(int argc, const char **argv) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s card \"cmd_volume_up\" \"cmd_volume_down\"\n",
            argv[0]);
    exit(1);
  }

  mixer(argv[1], argv[2], argv[3]);
  return 0;
}
