#ifndef TT_GAME_STATE_H
#define TT_GAME_STATE_H

typedef enum {
    GAME_CONTINUE,
    GAME_RETURN,
    GAME_NEXT
} GameStatus;

typedef struct {
    void (*init)(void);
    GameStatus (*update)(void); //returns a code to be interpreted by the owner
    void (*draw)(void);
} GameState;

#endif //TT_GAME_STATE_H