NAME         := phbase
INTERFACE    := include

SOURCES      := src/allocation.c                                               \
                src/dynarray.c                                                 \
                src/ptrdex.c                                                   \
                src/test.c                                                     \
                src/utf8.c

TEST_SOURCES := bins/test.c                                                    \
                test/arena.c                                                   \
                test/core.c                                                    \
                test/dynarray.c                                                \
                test/extrusion.c                                               \
                test/ptrdex.c                                                  \
                test/utf8.c

EXTRA_CFLAGS := -std=c11
