#ifndef VARIANT_H
#define VARIANT_H

// Enumeration of rule sets, regardless of start position
typedef enum {
    NORMAL, // transposes with Fischer Random and No-Castle
    BUGHOUSE,
    CRAZYHOUSE,
    LOSERS,
    ANTICHESS,
    GIVEAWAY,
    DUNSANY,
    TWOKINGS,
    KRIEGSPIEL,
    ATOMIC,
    THREECHECK,
    KINGOFTHEHILL,
    XIANGQI,
    SHOGI,
    CAPABLANCA, // Transposes with Capablanca Random
    GOTHIC,
    FALCON,
    SHATRANJ,
    COURIER,
    KNIGHTMATE,
    BEROLINA,
    JANUS,
    CYLINDER,
    SUPER,
    GREAT,
    UNKNOWN
} variant_t;
#endif

