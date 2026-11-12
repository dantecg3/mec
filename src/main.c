#include "mec/mec.h"

static const char *s_level_colors[] = {
  "\x1b[90m",   // TRACE: gray
  "\x1b[36m",   // DEBUG: cyan
  "\x1b[32m",   // INFO:  green
  "\x1b[33m",   // WARN:  yellow
  "\x1b[31m",   // ERROR: red
  "\x1b[1;41m", // FATAL: bold on red background
};
#define ANSI_RESET "\x1b[0m"

static void colored_stdout_sink (void *context $unused, LOG_Level level, const char *msg)
{
  printf ("%s%s" ANSI_RESET "\n", s_level_colors[level], msg);
}

void examples (void)
{
  // bitset example
  uint8 flags[$bitset (uint8, 10)] = { 0 };

  $bitset_set (flags, 2);
  if ($bitset_has (flags, 2))
    $debug ("flags: has flag 2");

  $bitset_clear (flags, 2);
  if (!$bitset_has (flags, 2))
    $debug ("flags: does not have flag 2");

  // COLOR example
  COLOR red = color_hex (0xFF0000FF);
  $debug ("red: [r: %u, g: %u, b: %u, a: %u]", red.color8.r, red.color8.g, red.color8.b, red.color8.a);

  COLORF redf = color_to_colorf (red);
  $debug ("redf: [r: %f, g: %f, b: %f, a: %f]", redf.r, redf.g, redf.b, redf.a);

  // ERROR example
  ERROR err = error (ERROR_OUT_OF_MEMORY, "some error");
  if (!error_is_null (err))
    $error ("something has failed: [0x%2X] %s", err.code, err.msg);

  ERROR err_null = error_null ();
  if (error_is_null (err_null))
    $debug ("nothing has failed");

  // RANGE example
  RANGE r1 = range (3, 15);
  $debug ("r1[3:15]: len: %zu, contains [5]: %s", range_length (r1), range_contains (r1, 5) ? "true" : "false");

  RANGE r2    = range (10, 20);
  RANGE inter = range_intersect (r1, r2);
  $debug ("r1[3:15], r2[10:20] -> intersection : [%zu:%zu]", inter.low, inter.high);

  // TIMER example
  TIMER frame = timer ();
  while (true)
    {
      float64 delta_time = timer_delta_milis (&frame);
      $debug ("delta: %f", delta_time);
      if (timer_since_start (&frame) >= 30e3)
        {
          $debug ("break after 30µs");
          break;
        }
    }

  // MEM example
  typedef struct
  {
    int32   id;
    float32 x, y;
  } Enemy;

  {
    $mem_scope_init (1 * MEM_KB, &s_context.temp_allocator);

    Enemy *enemies = $mem_new_array (Enemy, 10, s_context.temp_allocator);
    for (usize i = 0; i < 10; ++i)
      {
        enemies[i] = (Enemy){
          .id = (int32)i,
          .x  = (float32)(i * 10.0f),
          .y  = 5.0f
        };
      }

    RANGE    r            = range (5, 8);
    MEM_View enemies_view = $mem_view (Enemy, enemies, r);
    for (usize i = 0; i < range_length (r); ++i)
      {
        Enemy *enemy = $mem_view_at (Enemy, enemies_view, i);
        $debug ("%zu: [id: %i, x: %f, y: %f]", i, enemy->id, enemy->x, enemy->y);
      }
  }

  // STR example
  STR_View text = str_view ("  hola €uro 😀  ");
  $debug ("text: [" SV_FMT "], %zu bytes", SV_ARG (text), text.length);

  STR_View trimmed = str_view_trim (text);
  $debug ("trimmed: [" SV_FMT "]", SV_ARG (trimmed));

  usize euro = str_view_find_rune (trimmed, str_rune ("€"));
  $debug ("'€' found at byte %zu", euro);

  if (str_view_find_rune (trimmed, 'z') == STR_NOT_FOUND)
    $debug ("'z' not found");

  STR_View word = str_view_slice (trimmed, range (0, euro));
  $debug ("before the euro: [" SV_FMT "]", SV_ARG (word));

  STR_Buffer buf = str_new (trimmed, s_context.default_allocator);
  str_append (&buf, str_view (" ¡adiós!"));

  $debug ("buffer: [" SV_FMT "]", SV_ARG (str_view_from_buffer (buf)));
  $debug ("bytes: %zu, runes: %zu", str_length (buf), str_rune_count (buf));

  STR_Iter iter = str_iter (buf);
  STR_Rune rune;
  usize    non_ascii = 0;
  while (str_iter_next (&iter, &rune))
    {
      if (rune > 0x7F)
        ++non_ascii;
    }
  $debug ("non-ascii runes: %zu", non_ascii);

  STR_C c_str = str_c_new (str_view_from_buffer (buf), s_context.default_allocator);
  $debug ("c_str: %s", c_str);
  str_c_free (&c_str);

  str_free (&buf);

  // MAP
  MAP player_scores = map_new (sizeof (int), 8, s_context.default_allocator);

  int32 alice_score = 95;
  int32 bob_score   = 82;

  $map_put (player_scores, $map_hash_str (str_view ("alice")), &alice_score);
  $map_put (player_scores, $map_hash_str (str_view ("bob")), &bob_score);

  $debug ("map count after insertions: %zu", map_count (player_scores));

  int32 *found_alice = $map_get (int, player_scores, $map_hash_str (str_view ("alice")));
  if (found_alice != NULL)
    {
      $debug ("found score for 'alice': %i", *found_alice);
    }

  int *found_missing = $map_get (int, player_scores, $map_hash_str (str_view ("charlie")));
  if (found_missing == NULL)
    {
      $debug ("'charlie' not found as expected");
    }

  map_free (&player_scores);
  if (player_scores == NULL)
    {
      $debug ("map successfully freed and set to NULL");
    }

  // REG + SSET
  REG  ships = reg_new (4, s_context.default_allocator);
  SSET fuel  = $sset_new (float, 4, s_context.default_allocator);

  REG_Handle falcon = reg_register (ships);
  REG_Handle comet  = reg_register (ships);

  float32 tank = 42.5f;
  $sset_put (fuel, reg_handle_index (falcon), &tank);

  float32 *falcon_fuel = $sset_get (float32, fuel, reg_handle_index (falcon));
  float32 *comet_fuel  = $sset_get (float32, fuel, reg_handle_index (comet));
  $debug ("ships: %zu, falcon fuel: %.1f, comet has fuel: %s", reg_count (ships), falcon_fuel != NULL ? *falcon_fuel : 0.0f, comet_fuel != NULL ? "yes" : "no");

  sset_remove (fuel, reg_handle_index (falcon));
  reg_unregister (ships, falcon);

  REG_Handle nova = reg_register (ships);
  $debug ("nova reuses falcon's slot: %s, falcon handle still valid: %s", reg_handle_index (nova) == reg_handle_index (falcon) ? "yes" : "no", reg_is_valid_handle (ships, falcon) ? "yes" : "no");

  sset_free (&fuel);
  reg_free (&ships);
}

int main (void)
{
  mec_init_context ();
  log_add_sink (&s_context.logger, colored_stdout_sink, NULL);

  examples ();

  ERROR err = error_null ();
  while ($error_stack_pop (&err))
    $error (ERR_FMT " %s", err.code, err.msg);
}
