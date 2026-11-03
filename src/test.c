#include "mec/mec.h"
#include "mec/mem.h"
#include <stdio.h>

static const char *s_level_colors[] = {
  "\x1b[90m",   // TRACE: gray
  "\x1b[36m",   // DEBUG: cyan
  "\x1b[32m",   // INFO:  green
  "\x1b[33m",   // WARN:  yellow
  "\x1b[31m",   // ERROR: red
  "\x1b[1;41m", // FATAL: bold on red background
};
#define ANSI_RESET "\x1b[0m"

static void colored_stdout_sink (void *context $unused, LOG_Level level, STR_View msg)
{
  printf ("%s" SV_FMT ANSI_RESET "\n", s_level_colors[level], $sv_arg (msg));
}

static void context_init (MEC_Context *context, MEM_Allocator allocator)
{
  context->allocator = allocator;
  context->logger.level = LOG_LEVEL_TRACE;
  context->logger.sink_count = 0;
  log_add_sink (&context->logger, colored_stdout_sink, NULL);
}

// ECS Simple Test

#define WORLD_CAPACITY 16

#define VIEW_COUNT(v) ((v).count)
#define VIEW_DATA(v)  ((v).data)

typedef struct
{
  float32 x, y;
} Position;

typedef struct
{
  float32 x, y;
} Velocity;

typedef struct
{
  REG entities;
  SSET positions;
  SSET velocities;
} World;

static World world_new (MEM_Allocator alloc)
{
  World w;
  w.entities = reg_new (WORLD_CAPACITY, alloc);
  w.positions = $sset_new (Position, WORLD_CAPACITY, alloc);
  w.velocities = $sset_new (Velocity, WORLD_CAPACITY, alloc);
  $assert (w.entities && w.positions && w.velocities, "world allocation failed");
  return w;
}

static void world_free (World *w)
{
  sset_free (&w->velocities);
  sset_free (&w->positions);
  reg_free (&w->entities);
}

static REG_Handle entity_create (World *w)
{
  REG_Handle e = reg_register (w->entities);
  $assert (e != REG_INVALID_HANDLE, "entity registry full");
  return e;
}

static void entity_destroy (World *w, REG_Handle e)
{
  if (!reg_is_valid_handle (w->entities, e))
    return;

  sset_remove (w->positions, handle_index (e));
  sset_remove (w->velocities, handle_index (e));
  reg_unregister (w->entities, e);
}

static void add_position (World *w, REG_Handle e, float x, float y)
{
  Position p = { x, y };
  $sset_insert (w->positions, handle_index (e), &p);
}

static void add_velocity (World *w, REG_Handle e, float x, float y)
{
  Velocity v = { x, y };
  $sset_insert (w->velocities, handle_index (e), &v);
}

static Position *get_position (World *w, REG_Handle e)
{
  if (!reg_is_valid_handle (w->entities, e))
    return NULL;
  return $sset_get (Position, w->positions, handle_index (e));
}

static void system_move (World *w, float dt)
{
  MEM_View dense, keys;
  sset_dense (w->velocities, &dense, &keys);

  Velocity *vels = (Velocity *)VIEW_DATA (dense);
  uint32 *ids = (uint32 *)VIEW_DATA (keys);

  for (usize i = 0; i < VIEW_COUNT (dense); i++)
    {
      Position *p = $sset_get (Position, w->positions, ids[i]);
      if (p == NULL)
        continue;
      p->x += vels[i].x * dt;
      p->y += vels[i].y * dt;
    }
}

static void expect_position (World *w, REG_Handle e, float x, float y)
{
  Position *p = get_position (w, e);
  $assert (p != NULL, "entity should have a position");
  $assert (p->x == x && p->y == y, "position mismatch");
}

static void test_ecs (MEM_Allocator alloc)
{
  World w = world_new (alloc);

  REG_Handle a = entity_create (&w);
  REG_Handle b = entity_create (&w);
  REG_Handle c = entity_create (&w);
  $assert (reg_registered (w.entities) == 3, "3 entities registered");

  add_position (&w, a, 0, 0);
  add_velocity (&w, a, 2, 4);
  add_position (&w, b, 10, 10);
  add_velocity (&w, b, -2, 0);
  add_position (&w, c, 5, 5);

  system_move (&w, 0.5f);
  expect_position (&w, a, 1, 2);
  expect_position (&w, b, 9, 10);
  expect_position (&w, c, 5, 5);
  $info ("move: a, b moved, c stayed");

  entity_destroy (&w, b);
  $assert (!reg_is_valid_handle (w.entities, b), "b should be invalid");
  $assert (get_position (&w, b) == NULL, "b has no position");
  $assert (reg_registered (w.entities) == 2, "2 entities left");

  REG_Handle d = entity_create (&w);
  $assert (handle_index (d) == handle_index (b), "d reuses b's slot");
  $assert (handle_generation (d) != handle_generation (b), "d has a new generation");
  $assert (get_position (&w, b) == NULL, "stale handle must not see d's data");
  add_position (&w, d, 100, 100);

  system_move (&w, 0.5f);
  expect_position (&w, a, 2, 4);
  expect_position (&w, c, 5, 5);
  expect_position (&w, d, 100, 100);
  $info ("recycle: slot reused, stale handle rejected");

  world_free (&w);
  $assert (w.entities == NULL && w.positions == NULL && w.velocities == NULL, "world freed");

  $info ("ecs: all tests passed");
}

int main (void)
{
  context_init (&s_context, MEM_DEFAULT_ALLOCATOR);

  test_ecs (s_context.allocator);

  return 0;
}
